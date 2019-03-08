#define _CRT_SECURE_NO_WARNINGS
#include "ParticleExplosion.h"
#include "Window.h"

ParticleExplosion::ParticleExplosion(vec3 explosionCenter, std::vector<vec3> inputVertices, int startIndexOfLower) : startIndexOfLower(startIndexOfLower) {

	for (int i = 0; i < inputVertices.size(); i+=3) {
		particles.push_back(Particle(inputVertices[i] - explosionCenter));
		vertices.push_back(inputVertices[i]);
		vertices.push_back(inputVertices[i + 1]);
		vertices.push_back(inputVertices[i + 2]);
	}

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
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec3), &vertices[0], GL_DYNAMIC_DRAW);
	// Enable the usage of layout location 0 (check the vertex shader to see what this is)
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	activeNum = particles.size();
}

ParticleExplosion::~ParticleExplosion()
{
	// Delete previously generated buffers. Note that forgetting to do this can waste GPU memory in a 
	// large project! This could crash the graphics driver due to memory leaks, or slow down application performance!
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
}

void ParticleExplosion::updateParticleToWorld(mat4 phantomToWorld, mat4 upperToWorld, mat4 lowerToWorld) {
	for (int i = 0; i < particles.size(); i++) {
		Particle& p = particles[i];
		if (i < startIndexOfLower) {
			if ((upperToWorld * vec4(vertices[i * 3], 1)).y >= 0) {
				p.toWorld = phantomToWorld * upperToWorld * p.toWorld;
			}
			else {
				p.color.a = 0.0f;
			}
		}
		else {
			if ((lowerToWorld * vec4(vertices[i * 3], 1)).y <= 0) {
				p.toWorld = phantomToWorld * lowerToWorld * p.toWorld;
			}
			else {
				p.color.a = 0.0f;
			}
		}
	}
	prevTime = clock();
}

bool ParticleExplosion::update() {
	float deltaTime = 0.001f * (clock() - prevTime);
	prevTime = clock();
	// Update all particles
	for (GLuint i = 0; i < particles.size(); ++i)
	{
		Particle &p = particles[i];
		if (p.color.a > 0.0f) {	// particle is alive, thus update
			p.color.a -= deltaTime * 1.0;
			p.velocity *= 0.7f;
			p.toWorld = p.toWorld * translate(mat4(1.0f), p.velocity);
		}
		else if (p.active) {
			p.active = false;
			if (--activeNum <= 0) {
				return true;
			}
		}
	}
	return false;
}

// Render all particles
void ParticleExplosion::draw(GLuint shaderProgram)
{
	glDisable(GL_CULL_FACE);
	// Use additive blending to give it a 'glow' effect
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	glBindVertexArray(VAO);

	glm::mat4 modelview;
	for (int i = 0; i < particles.size(); ++i)
	{
		Particle p = particles[i]; //no need to be reference
		if (p.color.a > 0.0f) {
			if ((int)(rand() % 2) != 0)
				continue;
			modelview = Window::V * p.toWorld; 

			glUniform1i(glGetUniformLocation(shaderProgram, "objectType"), 6);
			glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &p.toWorld[0][0]);
			glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, &Window::P[0][0]);
			glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "modelview"), 1, GL_FALSE, &modelview[0][0]);

			glUniform4fv(glGetUniformLocation(shaderProgram, "particleColor"), 1, &p.color[0]);

			//glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)(3 * i));
			glDrawArrays(GL_TRIANGLES, 3 * i, 3);
			//glDrawArrays(GL_TRIANGLES, 0, 6);
		}
	}
	glBindVertexArray(0);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	// Don't forget to reset to default blending mode
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_BLEND);
}
