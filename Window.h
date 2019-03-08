#define GLM_ENABLE_EXPERIMENTAL

#ifndef _WINDOW_H_
#define _WINDOW_H_

#include <iostream>

#define GLFW_INCLUDE_GLEXT
#ifdef __APPLE__
#define GLFW_INCLUDE_GLCOREARB
#else
#include <GL/glew.h>
#endif
#include <GLFW/glfw3.h>
#include "shader.h"
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <vector>
#include <algorithm>
#include <ctime>
#include <time.h>
#include <irrKlang.h>

class Window
{
public:
    static glm::vec3 currVec;
    static glm::vec3 prevVec;
    static int frameCounter;
    static float spherePosition;
    static int spherePosition2;
	static int width;
	static int height;
    static int countNotRendered;
    static int spotLightWidth;
    static int spotEdge;

	static glm::mat4 P; // P for projection
	static glm::mat4 V; // V for view
    static std::vector<std::pair<glm::vec3, glm::vec3>> planes; //for the planes

	static void initialize_objects();
	static void clean_up();
	static GLFWwindow* create_window(int width, int height);
	static void resize_callback(GLFWwindow* window, int width, int height);
	static void idle_callback();
    static void zoomIn();
    static void zoomOut();
    static void planesMaker();
    static GLboolean isVisible(glm::vec3 x, float r);
	static void display_callback(GLFWwindow*);
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
	static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);

	static void rotateCamera(float angle, glm::vec3 axis);
	static glm::vec3 trackBallMapping(double xpos, double ypos);
	static void moveCamera();
	static void moveEnemies(); 

	static void drawIntoDepth();
	static void drawIntoStencil();
	static void drawObjectWithShadow();
	static void drawAmbientShadow();
	static void draw();

	static void processShooting();
	static void processParalyzationAndDeath();
	static void processSingleAttack(int attackIndex);
	static void processPlayerDamage();

	static void procedurallyBuild();

	static void playSound(int type, int enemyIndex);
    
};

#endif
