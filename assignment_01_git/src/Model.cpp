#include "Model.h"

#include "GameException.h"

#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

Model::Model(std::string filename, bool invert) {
	min_dim = glm::vec3(std::numeric_limits<float>::min());
	max_dim = glm::vec3(std::numeric_limits<float>::max());
	std::vector<float> vertex_data, normal_data;
	aiMatrix4x4 trafo;
	aiIdentityMatrix4(&trafo);

	scene = aiImportFile(filename.c_str(), aiProcessPreset_TargetRealtime_Quality);// | aiProcess_FlipWindingOrder);
	if (!scene) {
		std::string log = "Unable to load mesh from ";
		log.append(filename);
		THROW_EXCEPTION(log);
	}

	//Load the model recursively into data
	loadRecursive(root, invert, vertex_data, normal_data, scene, scene->mRootNode);
	
	// Scale first, Translate center second!
	std::pair<glm::vec3, glm::vec3> translateVectors = getTranslateVectors(vertex_data);
	root.transform = glm::scale(root.transform, translateVectors.first);
	root.transform = glm::translate(root.transform, translateVectors.second);

	n_vertices = vertex_data.size();

	//Create the VBOs from the data.
	if (fmod(static_cast<float>(n_vertices), 3.0f) < 0.000001f) {
		vertices.reset(new GLUtils::VBO(vertex_data.data(), n_vertices*sizeof(float), GL_ARRAY_BUFFER));
		normals.reset(new GLUtils::VBO(normal_data.data(), n_vertices*sizeof(float), GL_ARRAY_BUFFER));
	}
	else
		THROW_EXCEPTION("The number of vertices in the mesh is wrong");
}

Model::~Model() {

}

void Model::loadRecursive(MeshPart& part, bool invert,
			std::vector<float>& vertex_data, std::vector<float>& normal_data, const aiScene* scene, const aiNode* node) {
	//update transform matrix. notice that we also transpose it
	aiMatrix4x4 m = node->mTransformation;
	for (int j=0; j<4; ++j)
		for (int i=0; i<4; ++i)
			part.transform[j][i] = m[i][j];

	// draw all meshes assigned to this node
	for (unsigned int n=0; n < node->mNumMeshes; ++n) {
		const struct aiMesh* mesh = scene->mMeshes[node->mMeshes[n]];

		//apply_material(scene->mMaterials[mesh->mMaterialIndex]);

		part.first = vertex_data.size()/3;
		part.count = mesh->mNumFaces*3;

		//Allocate data
		vertex_data.reserve(vertex_data.size() + part.count*3);
		normal_data.reserve(normal_data.size() + part.count*3);

		//Add the vertices from file
		for (unsigned int t = 0; t < mesh->mNumFaces; ++t) {
			const struct aiFace* face = &mesh->mFaces[t];

			if(face->mNumIndices != 3)
				THROW_EXCEPTION("Only triangle meshes are supported");

			for(unsigned int i = 0; i < face->mNumIndices; i++) {
				int index = face->mIndices[i];
				vertex_data.push_back(mesh->mVertices[index].x);
				vertex_data.push_back(mesh->mVertices[index].y);
				vertex_data.push_back(mesh->mVertices[index].z);
				normal_data.push_back(mesh->mNormals[index].x);
				normal_data.push_back(mesh->mNormals[index].y);
				normal_data.push_back(mesh->mNormals[index].z);
			}
		}
	}

	// load all children
	for (unsigned int n = 0; n < node->mNumChildren; ++n) {
		part.children.push_back(MeshPart());
		loadRecursive(part.children.back(), invert, vertex_data, normal_data, scene, node->mChildren[n]);
	}
}

std::pair<glm::vec3, glm::vec3> Model::getTranslateVectors(const std::vector<float>& vertex_data)
{
	for(unsigned int i = 0; i < vertex_data.size(); i += 3) {
		if(min_dim.x < vertex_data[i])
			min_dim.x = vertex_data[i];
		if(min_dim.y < vertex_data[i + 1])
			min_dim.y = vertex_data[i + 1];
		if(min_dim.z < vertex_data[i + 2])
			min_dim.z = vertex_data[i + 2];
		if(max_dim.x > vertex_data[i])
			max_dim.x = vertex_data[i];
		if(max_dim.y > vertex_data[i + 1]) 
			max_dim.y = vertex_data[i + 1];
		if(max_dim.z > vertex_data[i + 2]) 
			max_dim.z = vertex_data[i + 2];
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

	glm::vec3 center = max_dim + min_dim;
	center /= 2;
	center = -center;

	return std::make_pair(scale, center);
}