#include "GameManager.h"
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <assert.h>
#include <stdexcept>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using std::cerr;
using std::endl;
using GLUtils::VBO;
using GLUtils::Program;
using GLUtils::readFile;

GameManager::GameManager(char* argv) {
	my_timer.restart();
	rendermode = RENDERMODE_PHONG;
	background_color = glm::vec3(0.0f, 0.0f, 0.0f);
	model_color = glm::vec3(1.0f, 1.0f, 1.0f);
	model_to_load = argv;
	std::cout << argv << std::endl;
}

GameManager::~GameManager() {
}

void GameManager::createOpenGLContext() {
	//Set OpenGL major an minor versions
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

	// Set OpenGL attributes
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1); // Use double buffering
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16); // Use framebuffer with 16 bit depth buffer
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8); // Use framebuffer with 8 bit for red
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8); // Use framebuffer with 8 bit for green
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8); // Use framebuffer with 8 bit for blue
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8); // Use framebuffer with 8 bit for alpha

	// Initalize video
	main_window = SDL_CreateWindow("NITH - PG612 Example OpenGL Program", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		window_width, window_height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	if (!main_window) {
		THROW_EXCEPTION("SDL_CreateWindow failed");
	}

	//Create OpenGL context
	main_context = SDL_GL_CreateContext(main_window);
	trackball.setWindowSize(window_width, window_height);

	// Init glew
	// glewExperimental is required in openGL 3.3
	// to create forward compatible contexts 
	glewExperimental = GL_TRUE;
	GLenum glewErr = glewInit();
	if (glewErr != GLEW_OK) {
		std::stringstream err;
		err << "Error initializing GLEW: " << glewGetErrorString(glewErr);
		THROW_EXCEPTION(err.str());
	}

	// Unfortunately glewInit generates an OpenGL error, but does what it's
	// supposed to (setting function pointers for core functionality).
	// Lets do the ugly thing of swallowing the error....
	glGetError();
}

void GameManager::setOpenGLStates() {
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glClearColor(background_color.r, background_color.g, background_color.b, 1.0);
}

void GameManager::createMatrices() {
	fov = 45.0f;
	projection_matrix = glm::perspective(fov,	window_width / (float) window_height, 1.0f, 10.f);
	model_matrix = glm::scale(glm::mat4(1.0f), glm::vec3(3));
	view_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -5.0f));
}

void GameManager::createSimpleProgram() {
	// PHONG SHADING
	std::string fs_src = readFile("shaders/test.frag");
	std::string vs_src = readFile("shaders/test.vert");

	//Compile shaders, attach to program object, and link
	phong_program.reset(new Program(vs_src, fs_src));

	//Set uniforms for the program.
	phong_program->use();
	glUniformMatrix4fv(phong_program->getUniform("projection_matrix"), 1, 0, glm::value_ptr(projection_matrix));
	phong_program->disuse();


	// FLAT SHADING
	fs_src = readFile("shaders/flatshader.frag");
	vs_src = readFile("shaders/flatshader.vert");

	flat_program.reset(new Program(vs_src, fs_src));

	flat_program->use();
	glUniformMatrix4fv(flat_program->getUniform("projection_matrix"), 1, 0, glm::value_ptr(projection_matrix));
	flat_program->disuse();

	active_program = phong_program;
}

void GameManager::createVAO() {
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	CHECK_GL_ERROR();


	modelInterleaved.reset(new ModelInterleavedArray("models/toyplane.obj", false));
	modelInterleaved->getArray()->bind();

	phong_program->setAttributePointer("in_Position", 3 , GL_FLOAT, GL_FALSE, sizeof(MyVertex), 0);
	phong_program->setAttributePointer("in_Normal", 3, GL_FLOAT, GL_FALSE, sizeof(MyVertex), (void*)(sizeof(glm::vec3)));
	CHECK_GL_ERROR();

	flat_program->setAttributePointer("in_Position", 3 , GL_FLOAT, GL_FALSE, sizeof(MyVertex), 0);
	flat_program->setAttributePointer("in_Normal", 3, GL_FLOAT, GL_FALSE, sizeof(MyVertex), (void*)(sizeof(glm::vec3)));
	CHECK_GL_ERROR();

	modelInterleaved->getArray()->unbind();
	CHECK_GL_ERROR();
	
	
	/*
	std::stringstream ss;
	ss << "models/" << model_to_load;

	model.reset(new Model(ss.str(), false));
	model->getVertices()->bind();
	phong_program->setAttributePointer("in_Position", 3);
	flat_program->setAttributePointer("in_Position", 3);
	CHECK_GL_ERROR();

	model->getNormals()->bind();
	phong_program->setAttributePointer("in_Normal", 3);
	flat_program->setAttributePointer("in_Normal", 3);
	CHECK_GL_ERROR();
	*/

	//Unbind VBOs and VAO
	vertices->unbind(); //Unbinds both vertices and normals
	glBindVertexArray(0);
	CHECK_GL_ERROR();
}

void GameManager::init() {
	// Initialize SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		std::stringstream err;
		err << "Could not initialize SDL: " << SDL_GetError();
		THROW_EXCEPTION(err.str());
	}
	atexit( SDL_Quit);

	createOpenGLContext();
	setOpenGLStates();
	createMatrices();
	createSimpleProgram();
	createVAO();
}

void GameManager::renderMeshRecursive(
				MeshPart& mesh, 
				const std::shared_ptr<Program>& program, 
				const glm::mat4& view_matrix, 
				const glm::mat4& model_matrix,
				glm::vec3 color) {

	//Create modelview matrix
	glm::mat4 meshpart_model_matrix = model_matrix * mesh.transform;
	glm::mat4 modelview_matrix = view_matrix * meshpart_model_matrix;
	glUniformMatrix4fv(program->getUniform("modelview_matrix"), 1, 0, glm::value_ptr(modelview_matrix));

	//Create normal matrix, the transpose of the inverse
	//3x3 leading submatrix of the modelview matrix
	glm::mat3 normal_matrix = glm::transpose(glm::inverse(glm::mat3(modelview_matrix)));
	glUniformMatrix3fv(program->getUniform("normal_matrix"), 1, 0, glm::value_ptr(normal_matrix));
	
	glUniform3f(program->getUniform("color"), color.r, color.g, color.b);

	glDrawArrays(GL_TRIANGLES, mesh.first, mesh.count);
	for (unsigned int i=0; i<mesh.children.size(); ++i)
		renderMeshRecursive(mesh.children.at(i), program, view_matrix, meshpart_model_matrix, color);
}

void GameManager::render() {
	//Clear screen, and set the correct program
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	active_program->use();

	//Render geometry
	glBindVertexArray(vao);
	switch(rendermode)
	{
	case RENDERMODE_WIREFRAME:
		renderWireframe(model_color);
		break;
	case RENDERMODE_HIDDENLINE:
		renderHiddenLine();
		break;
	case RENDERMODE_FLAT:
		renderFlat(model_color);
		break;
	case RENDERMODE_PHONG:
		renderPhong(model_color);
		break;
	}
	glBindVertexArray(0);
	CHECK_GL_ERROR();
}

void GameManager::play() {
	bool doExit = false;

	//SDL main loop
	while (!doExit) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {// poll for pending events
			switch (event.type) {
			case SDL_MOUSEBUTTONDOWN:
				trackball.rotateBegin(event.motion.x, event.motion.y);
				break;
			case SDL_MOUSEBUTTONUP:
				trackball.rotateEnd(event.motion.x, event.motion.y);
				break;
			case SDL_MOUSEMOTION:
				trackball_view_matrix = trackball.rotate(event.motion.x, event.motion.y);
				break;
			case SDL_MOUSEWHEEL:
				if(event.wheel.y > 0) {
					zoom(-5.0f);
				} else if(event.wheel.y < 0){
					zoom(5.0f);
				}
				break;
			case SDL_KEYDOWN:
				switch(event.key.keysym.sym)
				{
				case SDLK_ESCAPE:
					doExit = true;
					break;
				case SDLK_q:
					if(event.key.keysym.mod & KMOD_CTRL) doExit = true;
					break;
				case SDLK_1:
					rendermode = RENDERMODE_WIREFRAME;
					break;
				case SDLK_2:
					rendermode = RENDERMODE_HIDDENLINE;
					break;
				case SDLK_3:
					rendermode = RENDERMODE_FLAT;
					break;
				case SDLK_4:
					rendermode = RENDERMODE_PHONG;
					break;
				case SDLK_PAGEUP:
					zoom(5.0f);
					break;
				case SDLK_PAGEDOWN:
					zoom(-5.0f);
					break;
				}
				break;
			case SDL_QUIT: //e.g., user clicks the upper right x
				doExit = true;
				break;
			}
		}

		//Render, and swap front and back buffers
		render();
		SDL_GL_SwapWindow(main_window);
	}
	quit();
}

glm::mat4 GameManager::getNewViewMatrix()
{
	return view_matrix * trackball_view_matrix;
}

void GameManager::renderWireframe(glm::vec3 color) {
	ChangeToProgram(flat_program);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	renderMeshRecursive(modelInterleaved->getMesh(), active_program, getNewViewMatrix(), model_matrix, color);
}

void GameManager::renderPhong(glm::vec3 color) 
{
	ChangeToProgram(phong_program);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	renderMeshRecursive(modelInterleaved->getMesh(), active_program, getNewViewMatrix(), model_matrix, color);
}

void GameManager::renderFlat(glm::vec3 color) {
	ChangeToProgram(flat_program);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	renderMeshRecursive(modelInterleaved->getMesh(), active_program, getNewViewMatrix(), model_matrix, color);
}

void GameManager::renderHiddenLine() {
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1.0f, 1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	renderMeshRecursive(modelInterleaved->getMesh(), active_program, getNewViewMatrix(), model_matrix, background_color);
	glDisable(GL_POLYGON_OFFSET_FILL);
	glEnable(GL_POLYGON_OFFSET_LINE);
	glPolygonOffset(0.0f, 0.0f);
	renderWireframe(model_color);
	glDisable(GL_POLYGON_OFFSET_LINE);
}

void GameManager::zoom(float factor)
{
	float newFov = fov + factor;
	if(newFov < 170.0f && newFov >= 5)
		fov = newFov;

	projection_matrix = glm::perspective(fov, window_width / (float) window_height, 1.0f, 10.f);
	glUniformMatrix4fv(active_program->getUniform("projection_matrix"), 1, 0, glm::value_ptr(projection_matrix));
}

void GameManager::ChangeToProgram(std::shared_ptr<GLUtils::Program>& program)
{
	active_program = program;
	glUniformMatrix4fv(active_program->getUniform("projection_matrix"), 1, 0, glm::value_ptr(projection_matrix));
}

void GameManager::quit() {
	std::cout << "Bye bye..." << std::endl;
}
