#ifndef _PLAYER_H_
#define _PLAYER_H_


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
#include "OBJObject.h"

using namespace glm;

class Player {

public:
	Player(vec3 campos, vec3 camlookat, int mode);
	~Player();

	vec3 cam_pos, cam_look_at;

	std::vector<int> moving; //0: stationary 1:left 2: right 3: up 4: down
	int lastMove;
	bool moved;
	bool shooting;
	float playerRadius;
	int playerLife;
	int gameStatus; //0: PLAYING    1: GAME CLEAR    2: GAME OVER    3:
	clock_t lastAttacked;
	int playMode;

	//for versus mode
	bool immune; //disable shadow while immune
	int bullet;
	bool paralyzed;
	bool reloading;
	clock_t reloadTime;
	clock_t paralyzedTime;
	void processRIP(); //reload immune paralyzation
	int processShooting();

	//for rendering
	//float omega = 0.2f; //angular speed

	void draw(GLuint, bool forceDraw = false);
	void drawWithGeom(GLuint, vec3, bool forceDraw = false);
	void updateToWorld();

	OBJObject* upperhalf;
	OBJObject* lowerhalf;
	
	glm::mat4 toWorld;
	glm::mat4 upperhalfWorld;
	glm::mat4 lowerhalfWorld; 
	glm::mat4 traMat; //translational matrix
	glm::mat4 rotMat; //rotational matrix
	glm::mat4 scaMat; //rotational matrix

};

#endif