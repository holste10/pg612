#ifndef _MODEL_INTERLEAVED_H__
#define  _MODEL_INTERLEAVED_H__

#include <memory>
#include <string>
#include <vector>

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "GLUtils/VBO.hpp"
#include "GLUtils/Program.hpp"
#include "Model.h"

struct MyVertex {
	glm::vec3 position;
	glm::vec3 normal;
	float padding[2]; // just to get the size 32 bit total.
};

class ModelInterleavedArray {
public:
	ModelInterleavedArray(std::string filename, bool invert = 0);
	~ModelInterleavedArray();

	inline MeshPart getMesh() { return root; }
	inline std::shared_ptr<GLUtils::VBO> getArray() {return interleaved;}

private:
	static void loadRecursive(MeshPart& part, bool invert, 
		std::vector<MyVertex>& array_data, const aiScene* scene, const aiNode* node);

	const aiScene* scene;
	MeshPart root;

	std::shared_ptr<GLUtils::VBO> interleaved;
	
	glm::vec3 min_dim;
	glm::vec3 max_dim;

	unsigned int n_vertices;
};

#endif