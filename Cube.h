#ifndef _CUBE_H_
#define _CUBE_H_

#define GLFW_INCLUDE_GLEXT
#ifdef __APPLE__
#define GLFW_INCLUDE_GLCOREARB
#else
#include <GL/glew.h>
#endif
#include <GLFW/glfw3.h>
// Use of degrees is deprecated. Use radians instead.
#ifndef GLM_FORCE_RADIANS
#define GLM_FORCE_RADIANS
#endif
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "OBJObject.h"

class Cube
{
public:
	Cube(glm::vec3 offset, glm::vec3 factor, bool isWall, bool visible = true);
	~Cube();

	glm::mat4 toWorld;

	void draw(GLuint);

	//for shadow volume
	OBJObject* cubeToBeDrawn;
	void drawWithGeom(GLuint, glm::vec3);

	glm::vec3 maxVertex;
	glm::vec3 minVertex;
	float xlength;
	float ylength; 
	float zlength;
	bool isVisible;

	// These variables are needed for the shader program
	GLuint VBO, VBO1, VAO, EBO;
	GLuint uProjection, uModelview, uObjectType;

	GLfloat vertices[36][3] = {
		/*
		// "Front" vertices
		//{-200.0, -200.0,  200.0}, {200.0, -200.0,  200.0}, {200.0,  -4.0,  200.0}, {-200.0,  -4.0,  200.0},
		{ -50.0,  0.0,  50.0 },{ 50.0,  0.0,  50.0 },{ 50.0, 30.0,  50.0 },{ -50.0, 30.0, 50.0 },
		// "Back" vertices
		//{-200.0, -200.0, -200.0}, {200.0, -200.0, -200.0}, {200.0,  -4.0, -200.0}, {-200.0,  -4.0, -200.0}
		{ -50.0,  0.0,  -50.0 },{ 50.0,  0.0,  -50.0 },{ 50.0, 30.0,  -50.0 },{ -50.0, 30.0, -50.0 }
		*/

		// Front face
		{ -50.0,  0.0,  50.0 },{ 50.0, 100.0,  50.0 },{ 50.0,  0.0,  50.0 },{ -50.0, 100.0, 50.0 },{ 50.0, 100.0,  50.0 },{ -50.0,  0.0,  50.0 },
		// Right face
		{ 50.0,  0.0,  50.0 },{ 50.0, 100.0,  -50.0 },{ 50.0,  0.0,  -50.0 },{ 50.0, 100.0,  50.0 },{ 50.0, 100.0,  -50.0 }, { 50.0,  0.0,  50.0 },
		// Back face
		{ -50.0, 100.0, -50.0 },{ 50.0,  0.0,  -50.0 },{ 50.0, 100.0,  -50.0 },{ -50.0,  0.0,  -50.0 },{ 50.0,  0.0,  -50.0 },{ -50.0, 100.0, -50.0 },
		// Left face
		{ -50.0,  0.0,  -50.0 },{ -50.0, 100.0, 50.0 },{ -50.0,  0.0,  50.0 },{ -50.0, 100.0, -50.0 },{ -50.0, 100.0, 50.0 },{ -50.0,  0.0,  -50.0 },
		// Bottom face
		{ -50.0,  0.0,  -50.0 },{ 50.0,  0.0,  50.0 },{ 50.0,  0.0,  -50.0 },{ -50.0,  0.0,  50.0 }, { 50.0,  0.0,  50.0 },{ -50.0,  0.0,  -50.0 },
		// Top face
		{ -50.0, 100.0, 50.0 },{ 50.0, 100.0,  -50.0 },{ 50.0, 100.0,  50.0 }, { -50.0, 100.0, -50.0 },{ 50.0, 100.0,  -50.0 },{ -50.0, 100.0, 50.0 }
	};

	GLfloat normals[36][3] = {
		/*
		// "Front" vertices
		{ 0.0, 1.0,  0.0 },{ 0.0, 1.0,  0.0 },{ 0.0,  -1.0,  0.0 },{ 0.0,  -1.0,  0.0 },
		// "Back" vertices
		{ 0.0, 1.0, 0.0 },{ 0.0, 1.0, 0.0 },{ 0.0,  -1.0, 0.0 },{ 0.0,  -1.0, 0.0 }
		*/

		// Front face
		{ 0.0, 0.0, -1.0 },{ 0.0, 0.0, -1.0 },{ 0.0, 0.0, -1.0 },{ 0.0, 0.0, -1.0 },{ 0.0, 0.0, -1.0 },{ 0.0, 0.0, -1.0 },
		// Right face
		{ -1.0,  0.0,  0.0 },{ -1.0,  0.0,  0.0 },{ -1.0,  0.0,  0.0 },{ -1.0,  0.0,  0.0 },{ -1.0,  0.0,  0.0 },{ -1.0,  0.0,  0.0 },
		// Back face
		{ 0.0, 0.0, 1.0 },{ 0.0, 0.0, 1.0 },{ 0.0, 0.0, 1.0 },{ 0.0, 0.0, 1.0 },{ 0.0, 0.0, 1.0 },{ 0.0, 0.0, 1.0 },
		// Left face
		{ 1.0,  0.0,  0.0 },{ 1.0,  0.0,  0.0 },{ 1.0,  0.0,  0.0 },{ 1.0,  0.0,  0.0 },{ 1.0,  0.0,  0.0 },{ 1.0,  0.0,  0.0 },
		// Bottom face
		{ 0.0,  1.0,  0.0 },{ 0.0,  1.0,  0.0 },{ 0.0,  1.0,  0.0 },{ 0.0,  1.0,  0.0 },{ 0.0,  1.0,  0.0 },{ 0.0,  1.0,  0.0 },
		// Top face
		{ 0.0, -1.0, 0.0 },{ 0.0, -1.0, 0.0 },{ 0.0, -1.0, 0.0 },{ 0.0, -1.0, 0.0 },{ 0.0, -1.0, 0.0 },{ 0.0, -1.0, 0.0 }

	};

	const GLuint indices[6][6] = {
		/*
		// Front face
		{0, 2, 1, 3, 2, 0},
		// Right face
		{1, 6, 5, 2, 6, 1},
		// Back face
		{7, 5, 6, 4, 5, 7},
		// Left face
		{4, 3, 0, 7, 3, 4},
		// Bottom face
		{4, 1, 5, 0, 1, 4},
		// Top face
		{3, 6, 2, 7, 6, 3}
		*/

		// Front face
		{0, 1, 2, 3, 4, 5},
		// Right face
		{6, 7, 8, 9, 10, 11},
		// Back face
		{12, 13, 14, 15, 16, 17},
		// Left face
		{18, 19, 20, 21, 22, 23},
		// Bottom face
		{24, 25, 26, 27, 28, 29},
		// Top face
		{30, 31, 32, 33, 34, 35}
	};
};

#endif

