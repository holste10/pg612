#include "VirtualTrackball.h"
#include <cmath>
#include <iostream>

glm::mat4 quatToMat4(glm::quat q) {
	float length = sqrt(pow(q.x, 2) + pow(q.y, 2) + pow(q.z, 2) + pow(q.w, 2));
	float s = 2 / length;

	glm::mat4 mat4(
		1-(s*(pow(q.y,2) + pow(q.z,2))), s*(q.x*q.y - q.w*q.z),           s*(q.x*q.z + q.w*q.y),             0,
		s*(q.x*q.y + q.w*q.z),           1-(s*(pow(q.x,2) + pow(q.z,2))), s*(q.y*q.z - q.w*q.x),             0,
		s*(q.x*q.z - q.w*q.y),           s*(q.y*q.z + q.w*q.x),           1-(s*(pow(q.x, 2) + pow(q.y, 2))), 0,
		0,                               0,                               0,                                 1
	);

	return mat4;
}

VirtualTrackball::VirtualTrackball() {
	quat_old.w = 1.0;
	quat_old.x = 0.0;
	quat_old.y = 0.0;
	quat_old.z = 0.0;
	rotating = false;
	debug = false;
}

VirtualTrackball::~VirtualTrackball() {}

void VirtualTrackball::rotateBegin(int x, int y) {
	rotating = true;
	point_on_sphere_begin = getClosestPointOnUnitSphere(x, y);
}

void VirtualTrackball::rotateEnd(int x, int y) {
	rotating = false;
	quat_old = quat_new;
}

glm::mat4 VirtualTrackball::rotate(int x, int y) {
	//If not rotating, simply return the old rotation matrix
	if (!rotating) return quatToMat4(quat_old);

	glm::vec3 point_on_sphere_end; //Current point on unit sphere
	glm::vec3 axis_of_rotation; //axis of rotation
	float theta = 0.0f; //angle of rotation

	point_on_sphere_end = getClosestPointOnUnitSphere(x, y);

	point_on_sphere_begin = glm::normalize(point_on_sphere_begin);
	point_on_sphere_end = glm::normalize(point_on_sphere_end);

	theta = glm::degrees(glm::acos(glm::dot(point_on_sphere_end, point_on_sphere_begin)));
	axis_of_rotation = glm::cross(point_on_sphere_end, point_on_sphere_begin);
	quat_new = glm::rotate(quat_old, theta, axis_of_rotation);
	
	if(debug) {
		std::cout << "Angle: " << theta << std::endl;
		std::cout << "Axis: " << axis_of_rotation.x << " " << axis_of_rotation.y << " " << axis_of_rotation.z << std::endl;
	}
	return quatToMat4(quat_new);
}

void VirtualTrackball::setWindowSize(int w, int h) {
	this->w = w;
	this->h = h;
}

glm::vec2 VirtualTrackball::getNormalizedWindowCoordinates(int x, int y) {
	glm::vec2 coord = glm::vec2(x, y);
	coord.x = coord.x / w - radius;
	coord.y = radius - coord.y / h;

	if(debug)
		std::cout << "Window coords: " << coord.x << " " << coord.y << std::endl;
	return coord;
}

glm::vec3 VirtualTrackball::getClosestPointOnUnitSphere(int x, int y) {
	glm::vec2 normalized_coords;
	glm::vec3 point_on_sphere;
	float k;

	normalized_coords = getNormalizedWindowCoordinates(x, y);

	k = sqrt(pow(normalized_coords.x, 2) + pow(normalized_coords.y, 2));	
	if(k >= radius) {
		point_on_sphere.x = normalized_coords.x / 2;
		point_on_sphere.y = normalized_coords.y / 2;
		point_on_sphere.z = 0.0f;
	} else {
		point_on_sphere.x = 2 * normalized_coords.x;
		point_on_sphere.y = 2 * normalized_coords.y;
		point_on_sphere.z = sqrt(1 - 4 * pow(k, 2));
	}

	if(debug)
		std::cout << "Point on sphere: " << point_on_sphere.x << ", " << point_on_sphere.y << ", " << point_on_sphere.z << std::endl;
	
	return point_on_sphere;
}

const float VirtualTrackball::radius = 0.5f;