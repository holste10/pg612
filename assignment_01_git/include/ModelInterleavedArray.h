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
#include "Texture2D.h"

struct VertexData {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 tex_coords;
};

enum VertexLayout {
	V_POSITION = 0,
	V_NORMAL = sizeof(glm::vec3),
	V_TEX_COORD = sizeof(glm::vec3) *2
};

struct TextureData {
	unsigned int id;
	unsigned int type;
};

class ModelInterleavedArray {
public:
	ModelInterleavedArray(std::string filename, bool invert = 0);
	~ModelInterleavedArray();

	inline MeshPart getMesh() { return root; }
	inline std::shared_ptr<GLUtils::VBO> getArray() {return interleaved;}
	inline std::shared_ptr<GLUtils::VBO> getIndices() {return indices;}

	void loadTexture(const std::string& filename);
	void bindTextures();
	

	inline unsigned int getIndeceSize() {return n_indices;}

private:
	static void loadRecursive(
		MeshPart& part, 
		bool invert, 
		std::vector<VertexData>& array_data,
		std::vector<unsigned int>& indices_data,
		std::vector<Texture2D>& textures, 
		const aiScene* scene,
		const aiNode* node);


	std::pair<glm::vec3, glm::vec3> getTranslateVectors(const std::vector<VertexData>& vertex_data);


private:
	const aiScene* scene;
	MeshPart root;

	std::shared_ptr<GLUtils::VBO> interleaved;
	std::shared_ptr<GLUtils::VBO> indices;
	std::vector<Texture2D> textures;

	glm::vec3 min_dim;
	glm::vec3 max_dim;

	unsigned int n_vertices;

	unsigned int n_indices;
};

#endif