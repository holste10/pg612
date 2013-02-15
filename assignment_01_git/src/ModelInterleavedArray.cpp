#include "ModelInterleavedArray.h"
#include "GameException.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

ModelInterleavedArray::ModelInterleavedArray(std::string filename, bool invert) {
	std::cout << "Loading model: " << filename << "... Please Wait..." << std::endl;
	std::vector<VertexData> array_data;
	std::vector<unsigned int> indices_data;
	aiMatrix4x4 trafo;
	aiIdentityMatrix4(&trafo);

	scene = aiImportFile(filename.c_str(), aiProcessPreset_TargetRealtime_Quality);
	if(!scene) {
		std::string log = "Unable to load mesh from ";
		log.append(filename);
		THROW_EXCEPTION(log);
	}

	loadRecursive(root, invert, array_data, indices_data, textures, scene, scene->mRootNode);

	// Scale first, Translate center second!
	std::pair<glm::vec3, glm::vec3> translateVectors = getTranslateVectors(array_data);
	root.transform = glm::scale(root.transform, translateVectors.first);
	root.transform = glm::translate(root.transform, translateVectors.second);

	n_vertices = array_data.size();
	n_indices = indices_data.size();
	
	if(fmod(static_cast<float>(n_indices), 3.0f) < 0.000001f) {
		interleaved.reset(new GLUtils::VBO(array_data.data(), n_vertices * sizeof(VertexData), GL_ARRAY_BUFFER));
		indices.reset(new GLUtils::VBO(indices_data.data(), n_indices * sizeof(unsigned int), GL_ELEMENT_ARRAY_BUFFER));
		std::cout << "Model Loaded Successfully" << std::endl;
	} else {
		THROW_EXCEPTION("The number of vertices in the mesh is wrong");
	}
}

ModelInterleavedArray::~ModelInterleavedArray() {

}

void ModelInterleavedArray::loadRecursive(
	MeshPart& part, 
	bool invert, 
	std::vector<VertexData>& array_data, 
	std::vector<unsigned int>& indices_data,
	std::vector<Texture2D>& textures,
	const aiScene* scene, 
	const aiNode* node) {
	

	aiMatrix4x4 m = node->mTransformation;
	for (int j=0; j<4; ++j)
		for (int i=0; i<4; ++i)
			part.transform[j][i] = m[i][j];

	for(unsigned int n=0; n < node->mNumMeshes; ++n) {
		const struct aiMesh* mesh = scene->mMeshes[node->mMeshes[n]];

		part.first = indices_data.size();
		part.count = mesh->mNumFaces*3;
		part.vertexCount = array_data.size();

		indices_data.reserve(indices_data.size() + part.count*3);

		for(unsigned int i = 0; i < mesh->mNumVertices; i++) {
			VertexData tmp;
			tmp.position.x = mesh->mVertices[i].x;
			tmp.position.y = mesh->mVertices[i].y;
			tmp.position.z = mesh->mVertices[i].z;

			if(mesh->HasNormals()) {
				tmp.normal.x = mesh->mNormals[i].x;
				tmp.normal.y = mesh->mNormals[i].y;
				tmp.normal.z = mesh->mNormals[i].z;
			}
			
			if(mesh->HasTextureCoords(0)) {
				tmp.tex_coords.x = mesh->mTextureCoords[0][i].x;
				tmp.tex_coords.y = mesh->mTextureCoords[0][i].y;
			}
			array_data.push_back(tmp);
		}
		
		for (unsigned int t = 0; t < mesh->mNumFaces; ++t) {
			const struct aiFace* face = &mesh->mFaces[t];

			if(face->mNumIndices != 3)
				THROW_EXCEPTION("Only triangle meshes are supported");

			for(unsigned int i = 0; i < face->mNumIndices; i++) 
				indices_data.push_back(face->mIndices[i]);			
		}
	
		if(scene->HasMaterials()) {
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			for(unsigned int i = 0; i < material->GetTextureCount(aiTextureType_DIFFUSE); i++) {
				aiString str;
				material->GetTexture(aiTextureType_DIFFUSE, i, &str);
				std::stringstream ss;
				ss << "models/" << str.C_Str();
				Texture2D tex = Texture2D(ss.str());
				textures.push_back(tex);
			}
		} else {
			textures.push_back(Texture2D());
		}
	}

	//Load children
	for(unsigned int n = 0; n < node->mNumChildren; ++n) {
		part.children.push_back(MeshPart());
		loadRecursive(part.children.back(), invert, array_data, indices_data, textures, scene, node->mChildren[n]);
	}
}

std::pair<glm::vec3, glm::vec3> ModelInterleavedArray::getTranslateVectors(const std::vector<VertexData>& array_data) {
	min_dim = glm::vec3(std::numeric_limits<float>::min());
	max_dim = glm::vec3(std::numeric_limits<float>::max());
	for(unsigned int i = 0; i < array_data.size(); i++) {
		float x = array_data[i].position.x;
		float y = array_data[i].position.y;
		float z = array_data[i].position.z;

		if(min_dim.x < x)
			min_dim.x = x;
		if(min_dim.y < y)
			min_dim.y = y;
		if(min_dim.z < z)
			min_dim.z = z;
		if(max_dim.x > x)
			max_dim.x = x;
		if(max_dim.y > y) 
			max_dim.y = y;
		if(max_dim.z > z) 
			max_dim.z = z;
	}

	glm::vec3 displacement = min_dim - max_dim;
	float scalefactor = 0.0f;
	if(displacement.x > displacement.y)
		scalefactor = displacement.x;
	else
		scalefactor = displacement.y;
	if(displacement.z > scalefactor)
		scalefactor = displacement.z;
	
	glm::vec3 scale = glm::vec3(1.0f / scalefactor);
	glm::vec3 center = glm::vec3((max_dim + min_dim)) /= -2;
	return std::make_pair(scale, center);
}

void ModelInterleavedArray::bindTextures()
{
	if(textures.size() > 0)
		textures[0].bind();
}