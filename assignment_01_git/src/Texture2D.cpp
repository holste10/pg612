#include "Texture2D.h"
#include <iostream>

Texture2D::Texture2D(const std::string& filename) {
	readImageFile(filename);
	createGLTexture();
}

void Texture2D::bind() {
	glBindTexture(GL_TEXTURE_2D, texture_name);
}

void Texture2D::readImageFile(const std::string& filename) {
	image.reset(new Image());

	ILuint image_name;
	ilGenImages(1, &image_name);
	ilBindImage(image_name);

	if(!ilLoadImage(filename.c_str())) {
		ILenum error;
		while((error = ilGetError()) != IL_NO_ERROR) {
			std::cout << error << iluErrorString(error) << std::endl;
		}
		ilDeleteImages(1, &image_name);	
	}

	image->widht = ilGetInteger(IL_IMAGE_WIDTH);
	image->height = ilGetInteger(IL_IMAGE_HEIGHT);
	image->components = 4;
	int memory_needed = image->widht * image->height * image->components;
	image->data.resize(memory_needed);

	ilCopyPixels(0, 0, 0, image->widht, image->height, 1, IL_RGBA, IL_UNSIGNED_BYTE, &image->data[0]);
	ilDeleteImages(1, &image_name);
}

void Texture2D::createGLTexture() {
	glGenTextures(1, &texture_name);
	glBindTexture(GL_TEXTURE_2D, texture_name);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->widht, image->height,
				 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, &image->data[0]);
}