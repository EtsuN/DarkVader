#define _CRT_SECURE_NO_WARNINGS
#include "PhantomAttack.h"
#include "Window.h"


PhantomAttack::PhantomAttack(vector<Phantom*> enemies) {

	clock_t currTime = clock();
	vertices.insert(vertices.begin(), enemies.size(), vec3(0, 0, 0));
	attacks.insert(attacks.begin(), enemies.size(), Attack(currTime));
	
	if (enemies.size() == 0) {
		vertices.push_back(vec3(0, 0, 0));
		attacks.push_back(Attack(currTime));
	}

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec3), &vertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

PhantomAttack::~PhantomAttack() {
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
}

void PhantomAttack::draw(GLuint shaderProgram) {
	glBindVertexArray(VAO);
	glPointSize(5.0f);
	glEnable(GL_POINT_SMOOTH);

	for (int i = 0; i < attacks.size(); i++) {
		if (!attacks[i].active)
			continue;

		Attack& att = attacks[i];
		glm::mat4 modelview = Window::V * att.toWorld;

		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, &Window::P[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "modelview"), 1, GL_FALSE, &modelview[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &att.toWorld[0][0]);

		glUniform4fv(glGetUniformLocation(shaderProgram, "particleColor"), 1, ((int)rand()%2 < 1) ? &vec4(1.0f)[0] : &vec4(0,0,0,1)[0]);
		glUniform1i(glGetUniformLocation(shaderProgram, "objectType"), 7);

		glDrawArrays(GL_POINTS, i, 1);
	}
	glBindVertexArray(0);
}

void PhantomAttack::updateAttacks(vector<Phantom*> enemies) {
	clock_t currTime = clock();
	for (int i = 0; i < enemies.size(); i++) {
		Attack& att = attacks[i];
		Phantom* enemy = enemies[i];

		if (att.active) { //translation while active
			att.toWorld = translate(mat4(1.0f), att.velocity) * att.toWorld;
			Window::processSingleAttack(i);
		}
		else if (currTime - att.lastAttackTime >= att.attackSpan && !enemy->paralyzed && !enemy->dead) { //inactive --> active
			att.active = true;
			att.velocity = 2.0f * enemy->speed * vec3(enemy->toWorld * vec4(0, 0, 1, 0));
			att.toWorld = translate(mat4(1.0f), att.velocity) * translate(mat4(1.0f) , vec3(enemy->toWorld * vec4(0, 0, 0, 1)));
			att.lastAttackTime = currTime;
			Window::playSound(0, i);
		}
	}
	for (int i = enemies.size(); i < attacks.size(); i++) {
		if (attacks[i].active){ //translation while active even though the phantom is dead
			attacks[i].toWorld = translate(mat4(1.0f), attacks[i].velocity) * attacks[i].toWorld;
			if (currTime - attacks[i].lastAttackTime >= 20000) { //just in case
				attacks[i].active = false;
				i--;
				continue;
			}
			Window::processSingleAttack(i);
		}
		else { //remove if inactive attack of a dead phantom
			attacks.erase(attacks.begin() + i);
			i--;
		}
	}
}