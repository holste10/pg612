#ifndef _GAMEMANAGER_H_
#define _GAMEMANAGER_H_

#include <memory>

#include <GL/glew.h>
#include <SDL.h>
#include <glm/glm.hpp>

#include "Timer.h"
#include "GLUtils/GLUtils.hpp"
#include "Model.h"
#include "ModelInterleavedArray.h"
#include "VirtualTrackball.h"

enum RenderMode {
	RENDERMODE_FLAT, 
	RENDERMODE_PHONG, 
	RENDERMODE_WIREFRAME, 
	RENDERMODE_HIDDENLINE
};


/**
 * This class handles the game logic and display.
 * Uses SDL as the display manager, and glm for 
 * vector and matrix computations
 */
class GameManager {
public:

	/**
	 * Constructor
	 */
	GameManager(char* argv);

	/**
	 * Destructor
	 */
	~GameManager();

	/**
	 * Initializes the game, including the OpenGL context
	 * and data required
	 */
	void init();

	/**
	 * The main loop of the game. Runs the SDL main loop
	 */
	void play();

	/**
	 * Quit function
	 */
	void quit();

	/**
	 * Function that handles rendering into the OpenGL context
	 */
	void render();

protected:
	/**
	 * Creates the OpenGL context using SDL
	 */
	void createOpenGLContext();

	/**
	 * Sets states for OpenGL that we want to keep persistent
	 * throughout the game
	 */
	void setOpenGLStates();

	/**
	 * Creates the matrices for the OpenGL transformations,
	 * perspective, etc.
	 */
	void createMatrices();

	/**
	 * Compiles, attaches, links, and sets uniforms for
	 * a simple OpenGL program
	 */
	void createSimpleProgram();

	/**
	 * Creates vertex array objects
	 */
	void createVAO();

	static const unsigned int window_width = 1200;
	static const unsigned int window_height = 900;

private:
	static void renderMeshRecursive(MeshPart& mesh, 
			const std::shared_ptr<GLUtils::Program>& program, 
			const glm::mat4& modelview, 
			const glm::mat4& transform,
			glm::vec3 color);

	glm::mat4 getNewViewMatrix();
	void renderWireframe(glm::vec3 color);
	void renderPhong(glm::vec3 color);
	void renderFlat(glm::vec3 color);
	void renderHiddenLine();
	void zoom(float factor);
	void ChangeToProgram(std::shared_ptr<GLUtils::Program>& program);

private:
	GLuint vao; //< Vertex array object
	//GLuint vertex_vbo; //< VBO for vertex data
	std::shared_ptr<GLUtils::VBO> vertices, normals;
	//GLuint program; //< OpenGL shader program
	std::shared_ptr<GLUtils::Program> phong_program;
	std::shared_ptr<GLUtils::Program> flat_program;
	std::shared_ptr<GLUtils::Program> active_program;

	std::shared_ptr<Model> model;
	std::shared_ptr<ModelInterleavedArray> modelInterleaved;

	Timer my_timer; //< Timer for machine independent motion

	glm::mat4 projection_matrix; //< OpenGL projection matrix
	glm::mat4 model_matrix; //< OpenGL model transformation matrix
	glm::mat4 view_matrix; //< OpenGL camera/view matrix
	glm::mat3 normal_matrix; //< OpenGL matrix to transfor normals
	glm::mat4 trackball_view_matrix; //< OpenGL camera matrix for the trackball
	
	VirtualTrackball trackball; //< Our virtual track ball for rotation.
	SDL_Window* main_window; //< Our window handle
	SDL_GLContext main_context; //< Our opengl context handle 
	float fov;

	RenderMode rendermode;
	glm::vec3 background_color;
	glm::vec3 model_color;
	std::string model_to_load;
};

#endif // _GAMEMANAGER_H_
