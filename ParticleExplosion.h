#ifndef _PARTICLEEXPLOSION_H_
#define _PARTICLEEXPLOSION_H_


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
#include <iostream>
#include <vector>
#include <time.h>

using namespace glm;

struct Particle {
	mat4 toWorld;
	vec3 velocity;
	glm::vec4 color;
	bool active;

	Particle(vec3 pos) : active(true) {
		GLfloat random = (rand() % 100) / 100.0f;
		GLfloat rColor = 0.5 + ((rand() % 100) / 100.0f);
		color = glm::vec4(rColor, rColor, rColor, 1.0f);
		velocity = pos * random;
		toWorld = mat4(1.0f);
	}
};

class ParticleExplosion {

public:
	ParticleExplosion(vec3 explosionOrigin, std::vector<vec3> vertices, int startIndexOfLower);
	~ParticleExplosion();
	bool update();
	void draw(GLuint);
	void updateParticleToWorld(mat4 phantomToWorld, mat4 upperToWorld, mat4 lowerToWorld);

private:
	// State
	std::vector<Particle> particles;
	std::vector<glm::vec3> vertices;
	clock_t prevTime;
	GLuint VAO, VBO;

	int activeNum;
	int startIndexOfLower;

};

#endif