#ifndef _TEXTURE_2D__
#define _TEXTURE_2D__

#include <string>
#include <vector>
#include <IL/il.h>
#include <IL/ilu.h>

#include <GL/glew.h>

struct Image {
	std::vector<char> data;
	unsigned int components;
	unsigned long widht;
	unsigned long height;
};

class Texture2D {
public:
	Texture2D(const std::string& filename);
	void bind();

	GLuint texture_name;

private:
	void readImageFile(const std::string& filename);
	void createGLTexture();
	std::shared_ptr<Image> image;
};

#endif