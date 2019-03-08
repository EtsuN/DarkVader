#ifndef LIGHT_H
#define LIGHT_H

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#include <OpenGL/glext.h>
#include <OpenGL/gl.h> // Remove this line in future projects
#else
#include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

using namespace glm;

class Light
{
private:
std::vector<unsigned int> indices;
std::vector<glm::vec3> vertices;
std::vector<glm::vec3> normals;


public:
	Light(glm::vec3); 
	~Light();

	void parse(const char *filepath);
	void draw(GLuint);

	float angle;
	float maxX, minX, maxY, minY, maxZ, minZ;
	glm::mat4 toWorld;

	void rotate(float, vec3);
	void changeScale(float scalingVal);
	void centerToOrigin();
	void adjustLightPos(float scale, float angle, vec3 axis);

	GLuint VBO, VBO1, VAO, EBO;
	GLuint uProjection, uModelview;
	float factor;

	int mode;
	vec3 color;
	vec3 pos;
	vec3 dir;
	float cutoff;
	float exp;

	mat4 R;

	float centerX;
	float centerY;
	float centerZ;
};

#endif