#ifndef PHANTOM_H
#define PHANTOM_H

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
#include <irrKlang.h>
#include "OBJObject.h"
#include "Cube.h"
#include "ParticleExplosion.h"

using namespace glm;

class Phantom {
public:
	float speed = 0.05f; //divisible by 10
	float omega = 0.2f; //angular speed

	OBJObject* upperhalf;
	OBJObject* lowerhalf;
	ParticleExplosion* explosion;

	glm::mat4 toWorld;
	glm::mat4 upperhalfWorld;
	glm::mat4 lowerhalfWorld;
	glm::mat4 traMat; //translational matrix
	glm::mat4 rotMat; //rotational matrix
	glm::mat4 scaMat; //rotational matrix

	float currentAngle;
	float deltaAngle;
	float radius;
	float currentDist;
	int movingType;
	bool paralyzed;
	bool dead;
	clock_t paralyzedTime;
	clock_t diedTime;
	vec3 prevCenter;
	vec3 prevTurnPlace;

	irrklang::ISound* paralyzingSound;

	Phantom(int type, glm::vec3 initialPos, float anglex, float angley, float scale = 2.0f);
	~Phantom();

	void draw(GLuint);
	void drawWithGeom(GLuint, vec3);

	void updateToWorld(Phantom*);

	void animate();
	void move(vec3);
	void wallCollisionCheck(Cube* wall);
	void blockCollisionCheck(Cube* block);
	void phantomCollisionCheck(vector<Phantom*> others);
	void paralyze();
	bool die();
};

#endif