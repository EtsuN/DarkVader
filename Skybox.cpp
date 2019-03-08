#define _CRT_SECURE_NO_WARNINGS
#include "Skybox.h"
#include "Window.h"

Skybox::Skybox()
{
	toWorld = glm::scale(glm::mat4(1.f), glm::vec3(400.f));
	ppmFile.push_back("starfield_rt.ppm");
	ppmFile.push_back("starfield_lf.ppm");
	ppmFile.push_back("starfield_up.ppm");
	ppmFile.push_back("starfield_dn.ppm");
	ppmFile.push_back("starfield_ft.ppm");
	ppmFile.push_back("starfield_bk.ppm");

	// Create array object and buffers. Remember to delete your buffers when the object is destroyed!
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	// Bind the Vertex Array Object (VAO) first, then bind the associated buffers to it.
	// Consider the VAO as a container for all your buffers.
	glBindVertexArray(VAO);

	// Now bind a VBO to it as a GL_ARRAY_BUFFER. The GL_ARRAY_BUFFER is an array containing relevant data to what
	// you want to draw, such as vertices, normals, colors, etc.
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// glBufferData populates the most recently bound buffer with data starting at the 3rd argument and ending after
	// the 2nd argument number of indices. How does OpenGL know how long an index spans? Go to glVertexAttribPointer.
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	// Enable the usage of layout location 0 (check the vertex shader to see what this is)
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0,// This first parameter x should be the same as the number passed into the line "layout (location = x)" in the vertex shader. In this case, it's 0. Valid values are 0 to GL_MAX_UNIFORM_LOCATIONS.
		3, // This second line tells us how any components there are per vertex. In this case, it's 3 (we have an x, y, and z component)
		GL_FLOAT, // What type these components are
		GL_FALSE, // GL_TRUE means the values should be normalized. GL_FALSE means they shouldn't
		3 * sizeof(GLfloat), // Offset between consecutive indices. Since each of our vertices have 3 floats, they should have the size of 3 floats in between
		(GLvoid*)0); // Offset of the first vertex's component. In our case it's 0 since we don't pad the vertices array with anything.

					 // We've sent the vertex data over to OpenGL, but there's still something missing.
					 // In what order should it draw those vertices? That's why we'll need a GL_ELEMENT_ARRAY_BUFFER for this.

	loadTexture();

	// Unbind the currently bound buffer so that we don't accidentally make unwanted changes to it.
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	// Unbind the VAO now so we don't accidentally tamper with it.
	// NOTE: You must NEVER unbind the element array buffer associated with a VAO!
	glBindVertexArray(0);
}

Skybox::~Skybox()
{
	// Delete previously generated buffers. Note that forgetting to do this can waste GPU memory in a 
	// large project! This could crash the graphics driver due to memory leaks, or slow down application performance!
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
}

void Skybox::draw(GLuint shaderProgram)
{
	glm::mat4 modelview = Window::V * toWorld;
	
	uProjection = glGetUniformLocation(shaderProgram, "projection");
	uModelview = glGetUniformLocation(shaderProgram, "modelview");
	glUniformMatrix4fv(uProjection, 1, GL_FALSE, &Window::P[0][0]);
	glUniformMatrix4fv(uModelview, 1, GL_FALSE, &modelview[0][0]);
	glUniform1i(glGetUniformLocation(shaderProgram, "objectType"), 1);

	glUniform1i(glGetUniformLocation(shaderProgram, "skybox"), 0);
	// Now draw the cube. We simply need to bind the VAO associated with it.
	glBindVertexArray(VAO);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	// Tell OpenGL to draw with triangles, using 36 indices, the type of the indices, and the offset to start from
	glDrawArrays(GL_TRIANGLES, 0, 36);
	// Unbind the VAO when we're done so we don't accidentally draw extra stuff or tamper with its bound buffers
	glBindVertexArray(0);
}

unsigned char* Skybox::loadPPM(const char* filename, int& width, int& height)
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
void Skybox::loadTexture()
{
	int twidth, theight;   // texture width/height [pixels]
	vector<unsigned char*> tdata;  // texture pixel data

	twidth = 512;
	theight = 512;

	// Load image file
	for (int i = 0; i < ppmFile.size(); i++) {
		tdata.push_back(loadPPM(ppmFile[i].c_str(), twidth, theight));
	}

	// Create ID for texture
	glGenTextures(1, &textureID);

	// Set this texture to be the one we are working with
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	// Generate the texture
	unsigned char *data;
	for (GLuint i = 0; i < tdata.size(); i++)
	{
		data = tdata[i];
		glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			0, GL_RGB, twidth, theight, 0, GL_RGB, GL_UNSIGNED_BYTE, data
		);
	}

	// Make sure no bytes are padded
	//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Set bi-linear filtering for both minification and magnification
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Use clamp to edge to hide skybox edges
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}
