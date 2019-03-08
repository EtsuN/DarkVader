#define _CRT_SECURE_NO_WARNINGS
#include "Light.h"
#include "Window.h"
#include <glm/gtx/rotate_vector.hpp>

Light::Light(vec3 lightPosition) 
{
	toWorld = translate(glm::mat4(1.0f), lightPosition);
	parse("sphere.obj");

	pos = lightPosition;

	centerToOrigin();

	// Create array object and buffers. Remember to delete your buffers when the object is destroyed!
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &VBO1);
	glGenBuffers(1, &EBO);

	// Bind the Vertex Array Object (VAO) first, then bind the associated buffers to it.
	// Consider the VAO as a container for all your buffers.
	glBindVertexArray(VAO);

	// Now bind a VBO to it as a GL_ARRAY_BUFFER. The GL_ARRAY_BUFFER is an array containing relevant data to what
	// you want to draw, such as vertices, normals, colors, etc.
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// glBufferData populates the most recently bound buffer with data starting at the 3rd argument and ending after
	// the 2nd argument number of indices. How does OpenGL know how long an index spans? Go to glVertexAttribPointer.
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec3), &vertices[0], GL_STATIC_DRAW);
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
	glBindBuffer(GL_ARRAY_BUFFER, VBO1);
	// glBufferData populates the most recently bound buffer with data starting at the 3rd argument and ending after
	// the 2nd argument number of indices. How does OpenGL know how long an index spans? Go to glVertexAttribPointer.
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(vec3), &normals[0], GL_STATIC_DRAW);
	// Enable the usage of layout location 0 (check the vertex shader to see what this is)
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1,// This first parameter x should be the same as the number passed into the line "layout (location = x)" in the vertex shader. In this case, it's 0. Valid values are 0 to GL_MAX_UNIFORM_LOCATIONS.
		3, // This second line tells us how any components there are per vertex. In this case, it's 3 (we have an x, y, and z component)
		GL_FLOAT, // What type these components are
		GL_FALSE, // GL_TRUE means the values should be normalized. GL_FALSE means they shouldn't
		3 * sizeof(GLfloat), // Offset between consecutive indices. Since each of our vertices have 3 floats, they should have the size of 3 floats in between
		(GLvoid*)0); // Offset of the first vertex's component. In our case it's 0 since we don't pad the vertices array with anything.

	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	// Unbind the currently bound buffer so that we don't accidentally make unwanted changes to it.
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	// Unbind the VAO now so we don't accidentally tamper with it.
	// NOTE: You must NEVER unbind the element array buffer associated with a VAO!
	glBindVertexArray(0);

}

Light::~Light()
{
	// Delete previously generated buffers. Note that forgetting to do this can waste GPU memory in a 
	// large project! This could crash the graphics driver due to memory leaks, or slow down application performance!
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &VBO1);
	glDeleteBuffers(1, &EBO);
}

void Light::parse(const char *filepath)
{
	//TODO parse the OBJ file
	// Populate the face indices, vertices, and normals vectors with the OBJ Object data

	FILE* file = fopen(filepath, "r");
	char str[80];
	float num;
	maxX = std::numeric_limits<float>::min();
	maxY = std::numeric_limits<float>::min();
	maxZ = std::numeric_limits<float>::min();
	minX = std::numeric_limits<float>::max();
	minY = std::numeric_limits<float>::max();
	minZ = std::numeric_limits<float>::max();
	
	while (!feof(file)) {
		fscanf(file, "%s", str);
		if (str[0] == 'v') {
			if (str[1] == 'n') {
				glm::vec3 normal(0, 0, 0);
				fscanf(file, "%f %f %f", &normal.x, &normal.y, &normal.z);
				normals.push_back(normal);
			}
			else if (str[1] == '\0') {
				glm::vec3 vertice(0, 0, 0);
				fscanf(file, "%f %f %f", &vertice.x, &vertice.y, &vertice.z);
				vertices.push_back(vertice);
				
				if (vertice.x > maxX)
					maxX = vertice.x;
				if (vertice.x < minX)
					minX = vertice.x;
				if (vertice.y > maxY)
					maxY = vertice.y;
				if (vertice.y < minY)
					minY = vertice.y;
				if (vertice.z > maxZ)
					maxZ = vertice.z;
				if (vertice.z < minZ)
					minZ = vertice.z;
			}
		}
		else if (str[0] == 'f' && str[1] == '\0') {
			unsigned int vi[3], ni[3];
			fscanf(file, "%d//%d %d//%d %d//%d", &vi[0], &ni[0], &vi[1], &ni[1], &vi[2], &ni[2]);

			indices.push_back(vi[0] - 1);
			indices.push_back(vi[1] - 1);
			indices.push_back(vi[2] - 1);
			indices.push_back(ni[0] - 1);
			indices.push_back(ni[1] - 1);
			indices.push_back(ni[2] - 1);
		}
	}
	fclose(file);
}

void Light::centerToOrigin()
{
	centerX = maxX / 2 + minX / 2;
	centerY = maxY / 2 + minY / 2;
	centerZ = maxZ / 2 + minZ / 2;

	for (glm::vec3 &vertice : vertices) {
		vertice.x = vertice.x - centerX ;
		vertice.y = vertice.y - centerY;
		vertice.z = vertice.z - centerZ;
	}
}

void Light::draw(GLuint shaderProgram)
{
	// Calculate the combination of the model and view (camera inverse) matrices
	toWorld = translate(glm::mat4(1.0f), pos);
	glm::mat4 modelview = Window::V * toWorld;

	uProjection = glGetUniformLocation(shaderProgram, "projection");
	uModelview = glGetUniformLocation(shaderProgram, "modelview");
	glUniformMatrix4fv(uProjection, 1, GL_FALSE, &Window::P[0][0]);
	glUniformMatrix4fv(uModelview, 1, GL_FALSE, &modelview[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &toWorld[0][0]);
	glUniform1i(glGetUniformLocation(shaderProgram, "objectType"), 5);
	glUniform3fv(glGetUniformLocation(shaderProgram, "lightPos"), 1, &pos[0]);

	glBindVertexArray(VAO);
	// Tell OpenGL to draw with triangles, using 36 indices, the type of the indices, and the offset to start from
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	// Unbind the VAO when we're done so we don't accidentally draw extra stuff or tamper with its bound buffers
	glBindVertexArray(0);
}

void Light::rotate(float angle, vec3 axis)
{
	toWorld = glm::rotate(toWorld, angle, vec3(inverse(toWorld) * vec4(axis, 0)));
}

void Light::adjustLightPos(float scale, float angle, vec3 axis) {
	pos = glm::rotate(pos, angle, vec3(inverse(toWorld) * vec4(axis, 0) ) );
}
