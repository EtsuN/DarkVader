#include "Window.h"
#include "Image2d.h"
#include "Skybox.hpp"
#include "Cube.h"
#include "Light.h"
#include "OBJObject.h"
#include "NormalOBJObject.hpp"


using namespace glm;
using namespace std;

const char* window_title = "GLFW Starter Project";
GLint shaderProgram;
GLint shaderProgramDeferred;
GLint shaderProgramNull;
GLint shaderProgramAmbient;


Skybox * skybox;
Cube * cube;

Image2d* cursor;
Image2d* gun;
Light* light;

NormalOBJObject* enemy2;

vector<OBJObject*> enemies;



// On some systems you need to change this to the absolute path
#ifdef __APPLE__
#define VERTEX_SHADER_PATH "shader.vert"
#define FRAGMENT_SHADER_PATH "shader.frag"
#define VERTEX_SHADER_NULL_PATH "shader_null.vert"
#define FRAGMENT_SHADER_NULL_PATH "shader_null.frag"
#define VERTEX_SHADER_DEF_PATH "shader_def.vert"
#define FRAGMENT_SHADER_DEF_PATH "shader_def.frag"
#define GEOMETRY_SHADER_DEF_PATH "shader_def.geom"
#define FRAGMENT_SHADER_AMB_PATH "shader_amb.frag"
#else
#define VERTEX_SHADER_PATH "../../cse167final/finalproject/shader.vert"
#define FRAGMENT_SHADER_PATH "../../cse167final/finalproject/shader.frag"
#define VERTEX_SHADER_NULL_PATH "../../cse167final/finalproject/shader_null.vert"
#define FRAGMENT_SHADER_NULL_PATH "../../cse167final/finalproject/shader_null.frag"
#define VERTEX_SHADER_DEF_PATH "../../cse167final/finalproject/shader_def.vert"
#define FRAGMENT_SHADER_DEF_PATH "../../cse167final/finalproject/shader_def.frag"
#define GEOMETRY_SHADER_DEF_PATH "../../cse167final/finalproject/shader_def.geom"
#define FRAGMENT_SHADER_AMB_PATH "../../cse167final/finalproject/shader_amb.frag"
#endif


// Default camera parameters
glm::vec3 cam_pos(0.0f, 0.0f, 20.0f);		// e  | Position of camera
glm::vec3 cam_look_at(0.0f, 0.0f, 0.0f);	// d  | This is where the camera looks at
glm::vec3 cam_up(0.0f, 1.0f, 0.0f);			// up | What orientation "up" is

int Window::width;
int Window::height;
int Window::frameCounter;
glm::vec3 Window::currVec;
glm::vec3 Window::prevVec;


bool showShadowVolume = false;
bool showShadow = true;
int bump = 0;
bool shooting = false;
bool lbutton_down = false;
bool first_lbutton_down = true;
double xpos = 0;
double ypos = 0;
vector<int> moving; //0: stationary 1:left 2: right 3: up 4: down
vector<int> enemyDir;
clock_t lastTime;


glm::mat4 Window::P;
glm::mat4 Window::V;

std::vector<std::pair<glm::vec3, glm::vec3>> Window::planes;

using namespace std;
void Window::initialize_objects()
{
    skybox = new Skybox();
    cube = new Cube();
	cursor = new Image2d("cursor.ppm", 0.075, 0);
	//gun = new Image2d("Koala.ppm", 1, 0);
	light = new Light(vec3(0, 5, 0));


	enemy2 = new NormalOBJObject("barrel.obj", "barrel.ppm");
	enemy2->changeScale(.2f);
	enemy2->position(vec3(-10,-4, 10));

	OBJObject* enemy = new OBJObject("eyeball_s.obj", "silver.ppm", translate(mat4(1.f), vec3(5, 0, -5))*scale(mat4(1.f), vec3(10, 10, 10))); //TODO change the filename to the real ones
	enemies.push_back(enemy);
	enemy = new OBJObject("body_s.obj", "silver.ppm", translate(mat4(1.f), vec3(-5, 2, 0))*scale(mat4(1.f), vec3(1, 1, 1))); //TODO change the filename to the real ones
	enemies.push_back(enemy);
	enemyDir.push_back(1);
	enemyDir.push_back(1);

	lastTime = clock();

	shaderProgram = LoadShaders(VERTEX_SHADER_PATH, FRAGMENT_SHADER_PATH);
	shaderProgramDeferred = LoadShaders(VERTEX_SHADER_DEF_PATH, FRAGMENT_SHADER_DEF_PATH, GEOMETRY_SHADER_DEF_PATH);
	shaderProgramNull = LoadShaders(VERTEX_SHADER_NULL_PATH, FRAGMENT_SHADER_NULL_PATH);
	shaderProgramAmbient = LoadShaders(VERTEX_SHADER_PATH, FRAGMENT_SHADER_AMB_PATH);
}

void Window::zoomIn(){
    //get
    printf("ZOOMING IN");
    glm::vec3 vector = cam_look_at - cam_pos;
    
    cam_pos = cam_pos + normalize(vector);
    //cam_look_at = cam_pos + normalize(vector);
    cam_look_at = cam_look_at + normalize(vector);
    V = glm::lookAt(cam_pos, cam_look_at, cam_up);
}

void Window::zoomOut(){
    glm::vec3 vector = cam_look_at - cam_pos;
    
    cam_pos = cam_pos - normalize(vector);
    cam_look_at = cam_look_at - normalize(vector);
    V = glm::lookAt(cam_pos, cam_look_at, cam_up);
}

// Treat this as a destructor func`tion. Delete dynamically allocated memory here.
void Window::clean_up()
{
    delete(skybox);
    delete(cube);
	delete(cursor);
	delete(gun);
	glDeleteProgram(shaderProgram);

	glDeleteProgram(shaderProgramDeferred);

}

GLFWwindow* Window::create_window(int width, int height)
{
	// Initialize GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		return NULL;
	}

	// 4x antialiasing
	glfwWindowHint(GLFW_SAMPLES, 4);

#ifdef __APPLE__ // Because Apple hates comforming to standards
	// Ensure that minimum OpenGL version is 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// Enable forward compatibility and allow a modern OpenGL context
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// Create the GLFW window
	GLFWwindow* window = glfwCreateWindow(width, height, window_title, NULL, NULL);

	// Check if the window could not be created
	if (!window)
	{
		fprintf(stderr, "Failed to open GLFW window.\n");
		fprintf(stderr, "Either GLFW is not installed or your graphics card does not support modern OpenGL.\n");
		glfwTerminate();
		return NULL;
	}

	// Make the context of the window
	glfwMakeContextCurrent(window);
  
	// Set swap interval to 1
	glfwSwapInterval(1);

	// Get the width and height of the framebuffer to properly resize the window
	glfwGetFramebufferSize(window, &width, &height);
	// Call the resize callback to make sure things get drawn immediately
	Window::resize_callback(window, width, height);

	return window;
}

void Window::resize_callback(GLFWwindow* window, int width, int height)
{
#ifdef __APPLE__
	glfwGetFramebufferSize(window, &width, &height); // In case your Mac has a retina display
#endif
	Window::width = width;
	Window::height = height;
	// Set the viewport size. This is the only matrix that OpenGL maintains for us in modern OpenGL!
	glViewport(0, 0, width, height);

	if (height > 0)
	{
		P = glm::perspective(45.0f, (float)width / (float)height, 0.1f, 1000.0f);
        //get the 4 points of the camera perspective
		V = glm::lookAt(cam_pos, cam_look_at, cam_up);

		//update 2d images
		if (cursor) {
			cursor->updateScale(width, height);
		}
	}
}

void moveEnemies() {
	for (int i = 0; i < enemies.size(); i++) {
		OBJObject* enemy = enemies[i];
		if (i % 2 == 1) {
			if ((enemy->toWorld * vec4(0, 0, 0, 1)).x < -13 || (enemy->toWorld * vec4(0, 0, 0, 1)).x > 0) {
				enemyDir[i] *= -1;
			}
			enemy->toWorld = translate(enemy->toWorld, vec3(0.03f * enemyDir[i], 0, 0.03f * enemyDir[i]));
		}
		else {
			if ((enemy->toWorld * vec4(0, 0, 0, 1)).y < -2 || (enemy->toWorld * vec4(0, 0, 0, 1)).y > 10) {
				enemyDir[i] *= -1;
			}
			enemy->toWorld = translate(enemy->toWorld, vec3(0, 0.002f * enemyDir[i], 0.002f * enemyDir[i]));
		}
	}
}

void moveLight() {
	clock_t curTime = clock();
	float deltaTime = curTime - lastTime;
	if (deltaTime > 3000) {
		if (light->pos.y == 5)
			light->pos = vec3(0, 6, 10);
		else if (light->pos.y == 6)
			light->pos = vec3(-2, 3, -5);
		else if (light->pos.y == 3)
			light->pos = vec3(0, 5, 0);
		lastTime = curTime;
	}
}

void Window::idle_callback()
{
	if (!moving.empty())
		moveCamera();
	
	clock_t curTime = clock();
	moveEnemies();
	moveLight();

}

void Window::moveCamera() {
	vec3 view_dir = cam_look_at - cam_pos;
	switch (moving.back()) {
	case 0:
		return;
		break;
	case 1:
		view_dir = cross(vec3(0, 1, 0), normalize(view_dir - vec3(0, view_dir.y, 0)));
		break;
	case 2:
		view_dir = cross(normalize(view_dir - vec3(0, view_dir.y, 0)), vec3(0, 1, 0));
		break;
	case 3:
		view_dir = normalize(view_dir - vec3(0, view_dir.y, 0));
		break;
	case 4:
		view_dir = - normalize(view_dir - vec3(0, view_dir.y, 0));
		break;
	}
	
	cam_look_at += 0.05f * view_dir;
	cam_pos += 0.05f * view_dir;
	Window::V = glm::lookAt(cam_pos, cam_look_at, cam_up);
}

void Window::processShooting() {
	vec3 pixelColor;
	glReadPixels(Window::width / 2, Window::height / 2, 1, 1, GL_RGB, GL_FLOAT, &pixelColor[0]); //only outputs black due to cursor
	printf("pixel color: %f %f %f\n", pixelColor.x, pixelColor.y, pixelColor.z);
	int pixelStencil;
	glReadPixels(Window::width / 2, Window::height / 2, 1, 1, GL_STENCIL_INDEX, GL_INT, &pixelStencil); //only outputs black due to cursor
	printf("pixel stencil: %d\n", pixelStencil);


	float groundLevel = cube->vertices[3][1];
	vec3 viewDir = cam_look_at - cam_pos;
	vec3 pixelPos = viewDir * (abs(viewDir.y - groundLevel) / abs(viewDir.y));
	vec3 lightToPixel = normalize(pixelPos - light->pos);

	printf("shooted: none\n");
	if (pixelStencil == 1) { // meaning is shadow
		OBJObject* enemy;
		int shootedEnemy = -1;
		float maxCosine = -2;
		for (int i = 0; i < enemies.size(); i++) {
			enemy = enemies[i];
			vec3 lightToEnemy = vec3(enemy->toWorld * vec4(0, 0, 0, 1)) - light->pos;
			lightToEnemy = normalize(lightToEnemy);
			float cosineVal = dot(lightToPixel, lightToEnemy);
			if (cosineVal > maxCosine) {
				maxCosine = cosineVal;
				shootedEnemy = i;
			}
		}
		printf("shooted: %d\n", shootedEnemy);
		OBJObject* shooted = enemies[shootedEnemy];
		enemies.erase(enemies.begin() + shootedEnemy);
		delete(shooted);
	}
}

void Window::display_callback(GLFWwindow* window)
{
	if (showShadow) {
		// for depth test
		glDepthMask(GL_TRUE);
		// Clear the color and depth buffers
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);


		//draw scene into depth buffer
		drawIntoDepth();

		//draw shadow volume into stencil buffer
		glEnable(GL_STENCIL_TEST);
		drawIntoStencil();

		// draw shadow
		drawObjectWithShadow();
		glDisable(GL_STENCIL_TEST);

		// draw ambient color to the shadows
		drawAmbientShadow();
	}
	else {

		// for depth test
		glDepthMask(GL_TRUE);
		// Clear the color and depth buffers
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		//draw scene into depth buffer
		drawIntoDepth();
		//draw shadow volume into stencil buffer
		glEnable(GL_STENCIL_TEST);

		// draw shadow
		drawObjectWithShadow();
		glDisable(GL_STENCIL_TEST);
		drawAmbientShadow();
		// draw ambient color to the shadows
		glUseProgram(shaderProgram);
	}

// BELOW: draw the remaining
	// Use the shader of programID
	glUseProgram(shaderProgram);
	skybox->draw(shaderProgram);
	light->draw(shaderProgram);

	//the below is kind of cheating to overcome the shadow error TODO: delete this and fix the bug
	for (OBJObject* enemy : enemies) {
		enemy->draw(shaderProgram); //change to shaderprogram
	}
	glUniform3fv(glGetUniformLocation(shaderProgram, "lightPos"), 1, &(light->pos[0]));
    enemy2->draw(shaderProgram);

	if (showShadowVolume && showShadow) {
		for (OBJObject* enemy : enemies) {
			glUseProgram(shaderProgramDeferred);
			vec3 lightPosInEnemyWorld = inverse(enemy->toWorld) * vec4(light->pos, 1.0);
			glUniform3fv(glGetUniformLocation(shaderProgramDeferred, "lightPos"), 1, &(lightPosInEnemyWorld[0]));
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glDisable(GL_CULL_FACE);
			enemy->drawWithGeom(shaderProgramDeferred);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			glUseProgram(shaderProgram);
		}
	}
	if (shooting) {
		processShooting();
		shooting = false;
	}

	//draw 2D stuffs on the screen
	cursor->draw(shaderProgram); //should be drawn at last
// ABOVE: draw is done

	glfwPollEvents();
	glfwSwapBuffers(window);
}

void Window::drawAmbientShadow() {
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);
	glUseProgram(shaderProgramAmbient);
	cube->draw(shaderProgramAmbient);
	for (OBJObject* enemy : enemies) {
		enemy->draw(shaderProgramAmbient);
	}
	//glUniform3fv(glGetUniformLocation(shaderProgramAmbient, "lightPos"), 1, &(light->pos[0]));
	enemy2->draw(shaderProgramAmbient);
	glDisable(GL_BLEND);
}

void Window::drawObjectWithShadow() {
	glDrawBuffer(GL_FRONT_AND_BACK); //or GL_BACK
	// Draw only if the corresponding stencil value is zero
	glStencilFunc(GL_EQUAL, 0x0, 0xFF);
	// prevent update to the stencil buffer
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	// Use the shader of programID
	glUseProgram(shaderProgram);
	cube->draw(shaderProgram);
	for (OBJObject* enemy : enemies) {
		enemy->draw(shaderProgram);
	}
	glUniform3fv(glGetUniformLocation(shaderProgram, "lightPos"), 1, &(light->pos[0]));
	enemy2->draw(shaderProgram);
}

// one way to implement shooter is to call this function and drawIntoDepth separately for each invidual enemy and color the shadow with uniform of the enemy's index
void Window::drawIntoStencil() {
	//set up
	glDepthMask(GL_FALSE);
	glEnable(GL_DEPTH_CLAMP);
	glDisable(GL_CULL_FACE);

	// We need the stencil test to be enabled but we want it
	// to succeed always. Only the depth test matters.
	glStencilFunc(GL_ALWAYS, 0, 0xff);
	glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
	glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);

	// Use the shader of programID with geometry shadr --> deferred shading
	glUseProgram(shaderProgramDeferred);

	for (OBJObject* enemy : enemies) {
		// set light position
		vec3 lightPosInEnemyWorld = inverse(enemy->toWorld) * vec4(light->pos, 1.0);
		glUniform3fv(glGetUniformLocation(shaderProgramDeferred, "lightPos"), 1, &(lightPosInEnemyWorld[0]));

		// render
		enemy->drawWithGeom(shaderProgramDeferred);
	}
	//glUniform3fv(glGetUniformLocation(shaderProgramDeferred, "lightPos"), 1, &(light->pos[0]));
	enemy2->draw(shaderProgramDeferred);
	cube->draw(shaderProgramDeferred);

	// Restore local stuff
	glDisable(GL_DEPTH_CLAMP);
	glEnable(GL_CULL_FACE);
}

void Window::drawIntoDepth() {
	glDrawBuffer(GL_NONE);

	glUseProgram(shaderProgramNull);
	cube->draw(shaderProgramNull);
	for (OBJObject* enemy : enemies) {
		enemy->draw(shaderProgramNull);
	}
}

void Window::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		// Check if escape was pressed
		if (key == GLFW_KEY_ESCAPE)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}		
		else if (key == GLFW_KEY_A)
		{
			//left
			moving.push_back(1);
		}
		else if (key == GLFW_KEY_D)
		{
			//right
			moving.push_back(2);
		}
		else if (key == GLFW_KEY_W)
		{
			//forward
			moving.push_back(3);
		}
		else if (key == GLFW_KEY_S)
		{
			//backward
			moving.push_back(4);
		}
		else if (key == GLFW_KEY_0)
		{
			showShadowVolume = !showShadowVolume;
		}
		else if (key == GLFW_KEY_9)
		{
			bump = (bump + 1) % 2;
			glUniform1i(glGetUniformLocation(shaderProgram, "bump"), bump);
		}
		else if (key == GLFW_KEY_8)
		{
			showShadow = !showShadow;
		}
		else if (key == GLFW_KEY_SPACE)
		{
			shooting = true;
		}
		else if (key == GLFW_KEY_Z) //for debug
		{
			cam_look_at += vec3(0, 0.1f, 0);
			cam_pos += vec3(0, 0.1f, 0);
			Window::V = glm::lookAt(cam_pos, cam_look_at, cam_up);
		}
		else if (key == GLFW_KEY_X) //for debug
		{
			cam_look_at -= vec3(0, 0.1f, 0);
			cam_pos -= vec3(0, 0.1f, 0);
			Window::V = glm::lookAt(cam_pos, cam_look_at, cam_up);
		}
		//light movements
		else if (key == GLFW_KEY_I) //for debug
		{
			light->pos += vec3(0, 0, -0.5f);
		}
		else if (key == GLFW_KEY_K) //for debug
		{
			light->pos += vec3(0, 0, 0.5f);
		}
		else if (key == GLFW_KEY_J) //for debug
		{
			light->pos += vec3(0.5f, 0, 0);
		}
		else if (key == GLFW_KEY_L) //for debug
		{
			light->pos += vec3(-0.5f, 0, 0);
		}
		else if (key == GLFW_KEY_N) //for debug
		{
			light->pos += vec3(0, 0.5f, 0);
		}
		else if (key == GLFW_KEY_M) //for debug
		{
			light->pos += vec3(0, -0.5f, 0);
		}
	}
	else if (action == GLFW_RELEASE) {
		if (key == GLFW_KEY_A)
		{
			moving.erase(std::remove(moving.begin(), moving.end(), 1), moving.end());
		}
		else if (key == GLFW_KEY_D)
		{
			moving.erase(std::remove(moving.begin(), moving.end(), 2), moving.end());
		}
		else if (key == GLFW_KEY_W)
		{
			moving.erase(std::remove(moving.begin(), moving.end(), 3), moving.end());
		}
		else if (key == GLFW_KEY_S)
		{
			moving.erase(std::remove(moving.begin(), moving.end(), 4), moving.end());
		}
	}
}

/*
void Window::mouse_callback(GLFWwindow* window,double xpos, double ypos){
    
    if(lbutton_down) {
        float d;
        //get rotation axis and rotation angle, cross poduct and dot product
        //from the <past, origin> and <current, origin>
        //how do i get the x position
        Window::currVec.x = (2.0*xpos - width)/width;
        Window::currVec.y = (height - 2.0*ypos)/height;
        Window::currVec.z = 0.0;
        //the z value is incorrect
        d = glm::length(currVec);
        d = (d<1.0) ? d : 1.0;
        currVec.z = sqrtf(1.001 - d*d);
        glm::normalize(currVec);
        if(!first_lbutton_down){
            glm::vec3 crossVec = glm::cross(Window::prevVec, Window::currVec);
            float angle = glm::dot(Window::prevVec, Window::currVec);
            //make sure to reset the rotation
            angle = angle * .02;
            
            //we are rotating the cam_up and cam_pos
            cam_up = normalize(glm::rotate(glm::mat4(1.0f), -angle, crossVec) * glm::vec4(cam_up,1));
            cam_pos =  glm::vec3(glm::rotate(glm::mat4(1.0f), -angle, crossVec) * glm::vec4(cam_pos,1));
            
            //multiply cam pos
            
            //inverses the camera matrix to make the view matrix
            V = glm::lookAt(cam_pos, cam_look_at, cam_up);
        }
        //is there a copy constuctor
        Window::prevVec.x = Window::currVec.x;
        Window::prevVec.y = Window::currVec.y;
        Window::prevVec.z = Window::currVec.z;
        first_lbutton_down = false;
    }
    
}
*/

void Window::cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (xpos < 0 || xpos >= width || ypos < 0 || ypos >= height)
		return;

	vec3 direction;
	float pixel_diff;
	float rot_angle;
	vec3 currVec = trackBallMapping(xpos, ypos);
	if (lbutton_down)
	{

		direction = currVec - prevVec;
		float velocity = length(direction);
		if (velocity > 0.0001) // If little movement - do nothing.
		{
			vec3 rotAxis;
			rotAxis = cross(prevVec, currVec);
			//TODO: put the rotAxis to world space
			rotAxis = inverse(V) * vec4(rotAxis, 0.f);
			rot_angle = acos(dot(prevVec, currVec));
			rotateCamera(rot_angle, rotAxis);
		}

	}
	prevVec = currVec;
}

vec3 Window::trackBallMapping(double xpos, double ypos)
{
	vec3 v;
	float d;
	v.x = (2.0*xpos - width) / width;
	v.y = (height - 2.0*ypos) / height;
	v.z = 0.0f;
	d = glm::length(v);
	d = (d < 1.0) ? d : 1.0;
	v.z = sqrtf(1.001 - d * d);
	v = normalize(v); // Still need to normalize, since we only capped d, not v.
	return v;
}

void Window::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if(GLFW_PRESS == action){
            lbutton_down = true;
            first_lbutton_down = true;
        }
		else if (GLFW_RELEASE == action) {
			lbutton_down = false;
		}
	}
}

void Window::rotateCamera(float angle, vec3 axis) {

	vec3 view_dir = cam_look_at - cam_pos;
	mat4 rotMat = rotate(angle, axis);
	view_dir = vec3(rotMat * vec4(view_dir, 0.f));

	//avoid pointing exactly at the same direction as cam_up
	if (abs(view_dir.y) > 5.0f * (abs(view_dir.x) + abs(view_dir.z))) {
		return;
	}

	cam_look_at = view_dir + cam_pos;
	V = glm::lookAt(cam_pos, cam_look_at, cam_up);
}


