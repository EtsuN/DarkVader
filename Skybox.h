#ifndef _SKYBOX_H_
#define _SKYBOX_H_

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
#include <vector>

using namespace std;

class Skybox
{
public:
	Skybox();
	~Skybox();

	glm::mat4 toWorld;
	vector<string> ppmFile;
	unsigned int textureID;

	void draw(GLuint); 
	unsigned char* loadPPM(const char* filename, int& width, int& height);
	void loadTexture();

	// These variables are needed for the shader program
	GLuint VBO, VAO, EBO;
	GLuint uProjection, uModelview;
};

// Define the coordinates and indices needed to draw the cube. Note that it is not necessary
// to use a 2-dimensional array, since the layout in memory is the same as a 1-dimensional array.
// This just looks nicer since it's easy to tell what coordinates/indices belong where.
const GLfloat vertices[36][3] = {
	// Back face
	{ -1.0f,  1.0f, -1.0f}, {	-1.0f, -1.0f, -1.0f}, {1.0f, -1.0f, -1.0f }, {1.0f, -1.0f, -1.0f}, {1.0f,  1.0f, -1.0f}, {-1.0f,  1.0f, -1.0f},

	// Left face
	{-1.0f, -1.0f,  1.0f}, {-1.0f, -1.0f, -1.0f}, {-1.0f,  1.0f, -1.0f}, {-1.0f,  1.0f, -1.0f}, {-1.0f,  1.0f,  1.0f}, {-1.0f, -1.0f,  1.0f},

	// Right face
	{1.0f, -1.0f, -1.0f }, {1.0f, -1.0f,  1.0f}, {1.0f,  1.0f,  1.0f}, {1.0f,  1.0f,  1.0f}, {1.0f,  1.0f, -1.0f}, {1.0f, -1.0f, -1.0f},

	// Front face
	{-1.0f, -1.0f,  1.0f }, {-1.0f,  1.0f,  1.0f}, {1.0f,  1.0f,  1.0f}, {1.0f,  1.0f,  1.0f}, {1.0f, -1.0f,  1.0f}, {-1.0f, -1.0f,  1.0f},

	// Top face
	{-1.0f,  1.0f, -1.0f}, {1.0f,  1.0f, -1.0f}, {1.0f,  1.0f,  1.0f}, {1.0f,  1.0f,  1.0f}, {-1.0f,  1.0f,  1.0f}, {-1.0f,  1.0f, -1.0f},

	// Bottom face
	{-1.0f, -1.0f, -1.0f }, {-1.0f, -1.0f,  1.0f}, {1.0f, -1.0f, -1.0f}, {1.0f, -1.0f, -1.0f}, {-1.0f, -1.0f,  1.0f}, {1.0f, -1.0f,  1.0f}
};

#endif

