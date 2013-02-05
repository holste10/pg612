#include "ModelInterleavedArray.h"

#include "GameException.h"

#include <iostream>
#include <glm/gtc/matrix_transform.hpp>


ModelInterleavedArray::ModelInterleavedArray(std::string filename, bool invert) {
// 	min_dim = glm::vec3(std::numeric_limits<float>::min());
// 	max_dim = glm::vec3(std::numeric_limits<float>::max());
	std::vector<MyVertex> array_data;
	aiMatrix4x4 trafo;
	aiIdentityMatrix4(&trafo);

	scene = aiImportFile(filename.c_str(), aiProcessPreset_TargetRealtime_Quality);
	if(!scene) {
		std::string log = "Unable to load mesh from ";
		log.append(filename);
		THROW_EXCEPTION(log);
	}

	std::cout << sizeof(glm::vec3) << std::endl;

	loadRecursive(root, invert, array_data, scene, scene->mRootNode);

	min_dim = array_data[0].position;
	max_dim = array_data[0].position;
	for(unsigned int i = 1; i < array_data.size(); ++i) {
		if(min_dim.x < array_data[i].position.x)
			min_dim.x = array_data[i].position.x;
		if(min_dim.y < array_data[i].position.y)
			min_dim.y = array_data[i].position.y;
		if(min_dim.z < array_data[i].position.z)
			min_dim.z = array_data[i].position.z;
		if(max_dim.x > array_data[i].position.x)
			max_dim.x = array_data[i].position.x;
		if(max_dim.y > array_data[i].position.y) 
			max_dim.y = array_data[i].position.y;
		if(max_dim.z > array_data[i].position.z) 
			max_dim.z = array_data[i].position.z;
	}
	
 	
	glm::vec3 scale = min_dim - max_dim;
	//scale = 1.0f / scale;
	//scale = -scale;
	std::cout << scale.x << " " << scale.y << " " << scale.z << std::endl;

	glm::vec3 center = max_dim + min_dim;
	center /= 2;
	center = -center;
	std::cout << center.x << " " << center.y << " " << center.z << std::endl;

	//root.transform = glm::scale(root.transform, scale) * glm::translate(root.transform, center);

	root.transform = glm::scale(root.transform, 1.0f / scale);

	//root.transform = glm::scale(root.transform, glm::vec3(6.44f));
	root.transform = glm::translate(root.transform, center);

	n_vertices = array_data.size();
	
	std::shared_ptr<MyVertex> test;
	test.reset(new MyVertex(array_data[0]));

	if(fmod(static_cast<float>(n_vertices), 3.0f) < 0.000001f) {
		interleaved.reset(new GLUtils::VBO(array_data.data(), n_vertices * sizeof(MyVertex)));
	} else {
		THROW_EXCEPTION("The number of vertices in the mesh is wrong");
	}
}

ModelInterleavedArray::~ModelInterleavedArray() {

}

void ModelInterleavedArray::loadRecursive(MeshPart& part, bool invert, 
		std::vector<MyVertex>& array_data, const aiScene* scene, const aiNode* node) {
	
	aiMatrix4x4 m = node->mTransformation;
	for (int j=0; j<4; ++j)
		for (int i=0; i<4; ++i)
			part.transform[j][i] = m[i][j];

	for(unsigned int n=0; n < node->mNumMeshes; ++n) {
		const struct aiMesh* mesh = scene->mMeshes[node->mMeshes[n]];

		part.first = array_data.size()/3;
		part.count = mesh->mNumFaces*3;

		array_data.reserve(array_data.size() + part.count*3);

		//Add the vertices from file
		for (unsigned int t = 0; t < mesh->mNumFaces; ++t) {
			const struct aiFace* face = &mesh->mFaces[t];

			if(face->mNumIndices != 3)
				THROW_EXCEPTION("Only triangle meshes are supported");

			for(unsigned int i = 0; i < face->mNumIndices; i++) {
				int index = face->mIndices[i];

				MyVertex vertex;
				vertex.position.x = mesh->mVertices[index].x;
				vertex.position.y = mesh->mVertices[index].y;
				vertex.position.z = mesh->mVertices[index].z;
				vertex.normal.x = mesh->mNormals[index].x;
				vertex.normal.y = mesh->mNormals[index].y;
				vertex.normal.z = mesh->mNormals[index].z;
				array_data.push_back(vertex);
			}
		}
	}

	//Load children
	for(unsigned int n = 0; n < node->mNumChildren; ++n) {
		part.children.push_back(MeshPart());
		loadRecursive(part.children.back(), invert, array_data, scene, node->mChildren[n]);
	}
}