#include "Texture2D.h"
#include <iostream>

Texture2D::Texture2D() {
	createWhiteImage();
	createGLTexture();
}

Texture2D::Texture2D(const std::string& filename) {
	readImageFile(filename);
	createGLTexture();
}

void Texture2D::bind() {
	glBindTexture(GL_TEXTURE_2D, texture_name);
}

void Texture2D::createWhiteImage() {
	image.reset(new Image());

	image->widht = 16;
	image->height = 16;
	image->components = 4;

	unsigned int mem_size = image->widht * image->height * image->components;;
	unsigned char color_value = 255;

	for(unsigned int i = 0; i < mem_size; i++) 
		image->data.push_back(color_value);	
}

void Texture2D::readImageFile(const std::string& filename) {
	image.reset(new Image());

	ILuint image_name;
	ilGenImages(1, &image_name);
	ilBindImage(image_name);

	if(!ilLoadImage(filename.c_str())) {
		ILenum error;
		while((error = ilGetError()) != IL_NO_ERROR) {
			std::cout << error << " " << iluErrorString(error) << " " << filename << std::endl;
		}
		ilDeleteImages(1, &image_name);	
		createWhiteImage();
	} else {
		image->widht = ilGetInteger(IL_IMAGE_WIDTH);
		image->height = ilGetInteger(IL_IMAGE_HEIGHT);
		image->components = 4;
		int memory_needed = image->widht * image->height * image->components;
		image->data.resize(memory_needed);

		ilCopyPixels(0, 0, 0, image->widht, image->height, 1, IL_RGBA, IL_UNSIGNED_BYTE, &image->data[0]);
		ilDeleteImages(1, &image_name);
	}
}

void Texture2D::createGLTexture() {
	glGenTextures(1, &texture_name);
	glBindTexture(GL_TEXTURE_2D, texture_name);

// 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
// 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->widht, image->height,
				 0, GL_RGBA, GL_UNSIGNED_BYTE, &image->data[0]);
}