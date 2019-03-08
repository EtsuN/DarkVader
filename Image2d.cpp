#define _CRT_SECURE_NO_WARNINGS
#include "Image2d.h"
#include "Window.h"

using namespace glm;
using namespace std;

// height and width = 2.0 always even after resize 

Image2d::Image2d(const char* texture_file, float factor, vec2 offset, int minType, float thres, bool fillx) : minifyType(minType), threshold(thres), fillx(fillx)
{
	toWorld = mat4(1.f);

	curOffset = offset;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 2; j++) {
			vertices[i][j] *= factor;
			if (!fillx && j == 0) { // don't fill x
				vertices[i][j] *= 1.0f * Window::height / Window::width;
			}
			else if (fillx && j == 1) { // fill x
				vertices[i][j] *= 1.0f * Window::width / Window::height;
			}
			vertices[i][j] += offset[j];
		}
	}
	prevWindow = vec2(Window::width, Window::height);

	// Create array object and buffers. Remember to delete your buffers when the object is destroyed!
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &VBO2);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0,// This first parameter x should be the same as the number passed into the line "layout (location = x)" in the vertex shader. In this case, it's 0. Valid values are 0 to GL_MAX_UNIFORM_LOCATIONS.
		3, // This second line tells us how any components there are per vertex. In this case, it's 3 (we have an x, y, and z component)
		GL_FLOAT, // What type these components are
		GL_FALSE, // GL_TRUE means the values should be normalized. GL_FALSE means they shouldn't
		3 * sizeof(GLfloat), // Offset between consecutive indices. Since each of our vertices have 3 floats, they should have the size of 3 floats in between
		(GLvoid*)0); // Offset of the first vertex's component. In our case it's 0 since we don't pad the vertices array with anything.

	glBindBuffer(GL_ARRAY_BUFFER, VBO2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(textures), textures, GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2,// This first parameter x should be the same as the number passed into the line "layout (location = x)" in the vertex shader. In this case, it's 0. Valid values are 0 to GL_MAX_UNIFORM_LOCATIONS.
		2, // This second line tells us how any components there are per vertex. In this case, it's 3 (we have an x, y, and z component)
		GL_FLOAT, // What type these components are
		GL_FALSE, // GL_TRUE means the values should be normalized. GL_FALSE means they shouldn't
		2 * sizeof(GLfloat), // Offset between consecutive indices. Since each of our vertices have 3 floats, they should have the size of 3 floats in between
		(GLvoid*)0); // Offset of the first vertex's component. In our case it's 0 since we don't pad the vertices array with anything.

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	loadTexture(texture_file);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

Image2d::~Image2d()
{
	// Delete previously generated buffers. Note that forgetting to do this can waste GPU memory in a 
	// large project! This could crash the graphics driver due to memory leaks, or slow down application performance!
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &VBO2);
	glDeleteBuffers(1, &EBO);
}

void Image2d::draw(GLuint shaderProgram)
{
	uObjectType = glGetUniformLocation(shaderProgram, "objectType");
	glUniform1i(uObjectType, 4);
	glUniform1f(glGetUniformLocation(shaderProgram, "threshold"), threshold);

	GLuint uSampler = glGetUniformLocation(shaderProgram, "ourTexture");
	glUniform1i(uSampler, 2);
	glActiveTexture(GL_TEXTURE0 + 2); //and follows 2,4,6

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glBindVertexArray(VAO);
	// Tell OpenGL to draw with triangles, using 36 indices, the type of the indices, and the offset to start from
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	// Unbind the VAO when we're done so we don't accidentally draw extra stuff or tamper with its bound buffers
	glBindVertexArray(0);

	glDisable(GL_BLEND);
}

unsigned char* Image2d::loadPPM(const char* filename, int& width, int& height)
{
	const int BUFSIZE = 128;
	FILE* fp;
	unsigned int read;
	unsigned char* rawData;
	char buf[3][BUFSIZE];
	char* retval_fgets;
	size_t retval_sscanf;

	if ((fp = fopen(filename, "rb")) == NULL)
	{
		std::cerr << "error reading ppm file, could not locate " << filename << std::endl;
		width = 0;
		height = 0;
		return NULL;
	}

	// Read magic number:
	retval_fgets = fgets(buf[0], BUFSIZE, fp);

	// Read width and height:
	do
	{
		retval_fgets = fgets(buf[0], BUFSIZE, fp);
	} while (buf[0][0] == '#');
	retval_sscanf = sscanf(buf[0], "%s %s", buf[1], buf[2]);
	width = atoi(buf[1]);
	height = atoi(buf[2]);

	// Read maxval:
	do
	{
		retval_fgets = fgets(buf[0], BUFSIZE, fp);
	} while (buf[0][0] == '#');

	// Read image data:
	rawData = new unsigned char[width * height * 3];
	read = fread(rawData, width * height * 3, 1, fp);
	fclose(fp);
	if (read != 1)
	{
		std::cerr << "error parsing ppm file, incomplete data" << std::endl;
		delete[] rawData;
		width = 0;
		height = 0;
		return NULL;
	}

	return rawData;
}

// load image file into texture object
void Image2d::loadTexture(const char* ppmFile)
{
	int twidth, theight;   // texture width/height [pixels]
	unsigned char* tdata;  // texture pixel data

	twidth = 512;
	theight = 512;

	// Load image file
	tdata = loadPPM(ppmFile, twidth, theight);
	if (tdata == NULL) return;

	// Create ID for texture
	glGenTextures(1, &texture[0]);

	// Set this texture to be the one we are working with
	glBindTexture(GL_TEXTURE_2D, texture[0]);

	// Generate the texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, twidth, theight, 0, GL_RGB, GL_UNSIGNED_BYTE, tdata);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Set bi-linear filtering for both minification and magnification
	switch (minifyType) {
	case 1:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);//GL_LINEAR_MIPMAP_NEAREST);//GL_LINEAR_MIPMAP_LINEAR);//
		break;
	case 0:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);//GL_LINEAR_MIPMAP_LINEAR);//
		break;
	case 2:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);//
		break;
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void Image2d::updateScale(float width, float height, vec2 newCurOffset) {
	for (int i = 0; i < 4; i++) {
		vertices[i][0] -= curOffset[0];
		vertices[i][1] -= curOffset[1];
		if (!fillx) {
			vertices[i][0] *= 1.0f * Window::height / Window::width / prevWindow.y * prevWindow.x; 
		}
		else {
			vertices[i][1] *= 1.0f / Window::height * Window::width * prevWindow.y / prevWindow.x;
		}
		vertices[i][0] += curOffset[0];
		vertices[i][1] += curOffset[1];
	}
	prevWindow = vec2(Window::width, Window::height);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
