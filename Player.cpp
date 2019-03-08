#define _CRT_SECURE_NO_WARNINGS
#include "Player.h"
#include "Window.h"

Player::Player(vec3 campos, vec3 camlookat, int mode) : cam_pos(campos), cam_look_at(camlookat), playMode(mode) { 
	//moving
	lastMove = 0;
	moved = false;
	shooting = false;
	playerRadius = 1.5f;
	playerLife = 3;
	gameStatus = 0;
	lastAttacked = clock();

	//for versus mode
	bullet = 5;
	reloading = false;
	immune = false;
	paralyzed = false;
	reloadTime = clock();
	if (playMode == 2)
		playerRadius = 2.5f;

	//for rendering
	upperhalf = new OBJObject("head_s.obj", "eyes.ppm", mat4(1.f));
	lowerhalf = new OBJObject("head_s.obj", "grey.ppm", mat4(1.f));
	float origRadius = upperhalf->maxY - upperhalf->minY;
	upperhalf->toWorld = rotate(mat4(1.0f), 40.f / 180.0f * pi<float>(), vec3(-1.0f, 0.0f, 0.0f)) * translate(mat4(1.f), vec3(0, origRadius / 2.f, 0))
		*rotate(mat4(1.0f), pi<float>(), vec3(0.0f, 1.0f, 0.0f));
		//*rotate(mat4(1.0f), 60.f / 180.0f * pi<float>(), vec3(0.0f, 1.0f, 0.0f)); //last rotate is to avoid texture, stencil bug caused by a single triangle
	lowerhalf->toWorld = rotate(mat4(1.0f), 140.f / 180.0f * pi<float>(), vec3(-1.0f, 0.0f, 0.0f)) * translate(mat4(1.f), vec3(0, origRadius / 2.f, 0));
		//*rotate(mat4(1.0f), -120.f / 180.0f * pi<float>(), vec3(0.0f, 1.0f, 0.0f)); //last rotate is to avoid texture, stencil bug caused by a single triangle
	upperhalfWorld = upperhalf->toWorld;
	lowerhalfWorld = lowerhalf->toWorld;

	scaMat = glm::scale(mat4(1.0f), vec3(playerRadius/origRadius));
	updateToWorld();

	//currentAngle = 40.f;
}


Player::~Player() {
	// Delete previously generated buffers. Note that forgetting to do this can waste GPU memory in a 
	// large project! This could crash the graphics driver due to memory leaks, or slow down application performance!
	delete(upperhalf);
	delete(lowerhalf);
}

void Player::draw(GLuint shaderProgram, bool forceDraw) {
	if (playMode == 2 && !forceDraw) {
		if (immune)
			return;
		if (!(paralyzed || reloading))
			return;
	}
	updateToWorld();
	upperhalf->toWorld = toWorld * upperhalfWorld;
	lowerhalf->toWorld = toWorld * lowerhalfWorld;

	glUniform1i(glGetUniformLocation(shaderProgram, "objectType"), 0);
	upperhalf->draw(shaderProgram);
	lowerhalf->draw(shaderProgram);
}

void Player::drawWithGeom(GLuint shaderProgram, vec3 lightPos, bool forceDraw) {
	if (playMode == 2 && !forceDraw && immune) {
		return;
	}
	updateToWorld();
	upperhalf->toWorld = toWorld * upperhalfWorld;
	lowerhalf->toWorld = toWorld * lowerhalfWorld;

	glUniform1i(glGetUniformLocation(shaderProgram, "objectType"), 0);
	upperhalf->drawWithGeom(shaderProgram, lightPos);
	lowerhalf->drawWithGeom(shaderProgram, lightPos);
}

void Player::processRIP() {
	clock_t currTime = clock();
	if (reloading) {
		if (currTime - reloadTime >= 1500) {
			reloading = false;
			bullet = 2;
		}
	}
	if (immune) {
		if (currTime - lastAttacked >= 3000) {
			immune = false;
		}
	}
	if (paralyzed) {
		if (currTime - paralyzedTime >= 2000 || immune) {
			paralyzed = false;
		}
	}
}

int Player::processShooting() {
	if (!reloading) {
		if (bullet == 0) {
			reloading = true;
			reloadTime = clock();
			return 2; //need reload
		}
		else {
			bullet--;
			return 1; //can shoot
		}
	}
	return 0; //cannot shoot
}

void Player::updateToWorld() {
	traMat = translate(glm::mat4(1.0f), cam_pos);
	vec3 currDir = normalize(cam_look_at - cam_pos);
	rotMat = rotate(mat4(1.0f),
		(currDir.z >= 0) ? atan(currDir.x / currDir.z) : pi<float>() + atan(currDir.x / currDir.z),
		vec3(0.0f, 1.0f, 0.0f)) * rotate(mat4(1.0f), -asin(currDir.y), vec3(1.0f, 0.0f, 0.0f));
	toWorld = traMat * rotMat * scaMat;
}