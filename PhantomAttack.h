#ifndef _PHANTOMATTACK_H_
#define _PHANTOMATTACK_H_

#define GLM_ENABLE_EXPERIMENTAL

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#include <OpenGL/glext.h>
#include <OpenGL/gl.h> // Remove this line in future projects
#else
#include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>
#include <glm/gtx/intersect.hpp>
#include <vector>
#include <time.h>
#include "Phantom.h" 
#include "Cube.h"

using namespace glm;

struct Attack {
	mat4 toWorld;
	vec3 velocity;
	bool active;
	clock_t lastAttackTime;
	float attackSpan; //fixed

	Attack(clock_t time, float span = 8000) : active(false), lastAttackTime(time), toWorld(mat4(1.0f)), velocity(vec3(0)) {
		GLfloat random = (rand() % 20) * 200.0f - 2000.0f;
		attackSpan = span + random;
	}
};

class PhantomAttack {

public:
	PhantomAttack(vector<Phantom*>);
	~PhantomAttack();


	mat4 toWorld;
	vector<vec3> vertices;
	vector<Attack> attacks;

	GLuint VBO, VAO;

	void draw(GLuint);
	void updateAttacks(vector<Phantom*>);
};

#endif