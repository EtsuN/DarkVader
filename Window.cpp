#include "Window.h"
#include "Image2d.h"
#include "Skybox.h"
#include "Cube.h"
#include "Light.h"
#include "OBJObject.h"
#include "Phantom.h"
#include "PhantomAttack.h"
#include "Player.h"

using namespace glm;
using namespace std;
using namespace irrklang;

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
#define VERTEX_SHADER_PATH "shader.vert"
#define FRAGMENT_SHADER_PATH "shader.frag"
#define VERTEX_SHADER_NULL_PATH "shader_null.vert"
#define FRAGMENT_SHADER_NULL_PATH "shader_null.frag"
#define VERTEX_SHADER_DEF_PATH "shader_def.vert"
#define FRAGMENT_SHADER_DEF_PATH "shader_def.frag"
#define GEOMETRY_SHADER_DEF_PATH "shader_def.geom"
#define FRAGMENT_SHADER_AMB_PATH "shader_amb.frag"
#endif

const char* window_title = "DarkVader";
GLint shaderProgram;
GLint shaderProgramDeferred;
GLint shaderProgramNull;
GLint shaderProgramAmbient;


Skybox* skybox;
Light* light;

//stage
Cube* cube;
vector<Cube*> objects;
int stageArray[10][10];
int blockArray[40][40];

//image
Image2d* cursor;
Image2d* gameover;
Image2d* gameclear;
Image2d* youwin = nullptr;
Image2d* youlose = nullptr;

//phantom
vector<Phantom*> enemies;
PhantomAttack* allAttack;

int Window::width;
int Window::height;
int Window::frameCounter;
glm::vec3 Window::currVec;
glm::vec3 Window::prevVec;

ISoundEngine* engine;
ISoundSource* ssBGM;
ISoundSource* ssShooting;
ISoundSource* ssExplosion;
ISoundSource* ssParalyzation;
ISoundSource* ssAttack;
ISoundSource* ssDamage;
ISoundSource* ssStationary;
ISoundSource* ssReload;

bool showShadowVolume = false;
int bump = 0;
bool shooting = false;
bool lbutton_down = false;
bool first_lbutton_down = true;
double xpos = 0;
double ypos = 0;

//player
float playerRadius = 1.5f;
int playerLife;
int gameStatus; //0: PLAYING    1: GAME CLEAR    2: GAME OVER    3:
vector<Image2d*> hearts;
vector<Image2d*> bullets;

vector<int> moving; //0: stationary 1:left 2: right 3: up 4: down
int lastMove;
bool moved;
clock_t lastTime;
clock_t lastAttacked;
float immuneDuration = 1000;
float paralyzationDuration = 3000;

int lightMovingDir; //1:left 2: right 3: up 4: down
int lightMovingCount;
bool toggleMove = true;

glm::mat4 Window::P;
glm::mat4 Window::V;

std::vector<std::pair<glm::vec3, glm::vec3>> Window::planes;

// Default camera parameters
glm::vec3 cam_pos(0.0f, 5.0f, 20.0f);		// e  | Position of camera  0: (-3.0f, 5.0f, 20.f)1: (0.0f, 5.0f, 20.0f)    2: (75.0f, 5.0f, 75.0f)
glm::vec3 cam_look_at(0.0f, 5.0f, 0.0f);	// d  | This is where the camera looks at
glm::vec3 cam_up(0.0f, 1.0f, 0.0f);			// up | What orientation "up" is

//for multi player
Player* playerone = nullptr;
Player* playertwo = nullptr;
Player* currPlayer = nullptr;
vec3 cam_pos1(0.0f, 5.0f, 20.0f), cam_pos2(0.0f, 5.0f, 20.0f), cam_look_at1(0.0f, 5.0f, 0.0f), cam_look_at2(0.0f, 5.0f, 0.0f);
vector<int> turning; //for player one;		0: stationary 1:left 2: right 3: up 4: down

//type of the stage
int stageType = 0; // 2: cube   1: buildings   0/negative: debug
int playMode = 1; // 0: single player   1: multiple co-op   2: multi versus

void Window::initialize_objects()
{
	srand(time(0));

	//########
	//shader
	shaderProgram = LoadShaders(VERTEX_SHADER_PATH, FRAGMENT_SHADER_PATH);
	shaderProgramDeferred = LoadShaders(VERTEX_SHADER_DEF_PATH, FRAGMENT_SHADER_DEF_PATH, GEOMETRY_SHADER_DEF_PATH);
	shaderProgramNull = LoadShaders(VERTEX_SHADER_NULL_PATH, FRAGMENT_SHADER_NULL_PATH);
	shaderProgramAmbient = LoadShaders(VERTEX_SHADER_PATH, FRAGMENT_SHADER_AMB_PATH);

	//########

	playerLife = 3;
	gameStatus = 0;

	lastMove = 0;
	moved = false;
	lightMovingDir = 3;
	lightMovingCount = 0;

	cursor = new Image2d("newcursor.ppm", 0.075, vec2(0), 2, 0.9);
	gameover = new Image2d("gameover.ppm", 1, vec2(0), 0, 0.25, true);
	gameclear = new Image2d("gameclear.ppm", 1, vec2(0), 0, 0.25, true);
	for (int i = 0; i < playerLife; i++) {
		hearts.push_back(new Image2d("newheart.ppm", 0.1, vec2( (1.6 * i + 0.8) * 0.1 - 1.0, 0.12 - 1.0), 2, 0.9));
	}

    skybox = new Skybox();
	Phantom* phantom;
	switch (stageType) {
	case 0: {
		cube = new Cube(vec3(0, 0, 0), vec3(2, 0.3, 2), true);

		Cube* object = new Cube(vec3(-2.5, 0, -2.5), vec3(0.05, 0.05, 0.05), false, true);// vec3(0, 0, 0), vec3(0.5, 0.3, 0.5), false); //TODO change true to false and debug(delete negating normals code and also blockcollisioncheck)
		object = new Cube(vec3(-2.5, 0, -12.5), vec3(0.05, 0.1, 0.05), false, true);
		object = new Cube(vec3(0, 0, 0), vec3(0.05, 0.05, 0.05), false, true);
		objects.push_back(object);

		phantom = new Phantom(4, vec3(7, 5, 0), 0.0f, 0.0f);
		enemies.push_back(phantom); 
		phantom = new Phantom(4, vec3(-7, 5, 0), 0.0f, 0.0f);
		enemies.push_back(phantom);
		phantom = new Phantom(1, vec3(15,25, 15), -135.f, -45.f);
		//enemies.push_back(phantom);
		phantom = new Phantom(1, vec3(0, 15, -5), 270.f, -90.f); 
		//enemies.push_back(phantom);

		float distance;
		vec2 bary;
		bool doesIntersect = glm::intersectRayTriangle(vec3(-1.527,5.00003,2.64)-vec3(0.98,0,0), -vec3(-0.0786,-0.0002,0.997),vec3(-2.5, 5, -25), vec3(-2.5, 5, 25), vec3(-2.5, 0, 25), bary, distance);
		printf("\n\n\ntest: %d %f\n\n", doesIntersect, distance);
		doesIntersect = glm::intersectRayTriangle(vec3(-1.527, 5.00003, 2.64) - vec3(0.98,0,0), -vec3(-0.0786, -0.0002, 0.997), vec3(-2.5, 5, -25), vec3(-2.5, 0, -25), vec3(-2.5, 0, 25), bary, distance);
		printf("\n\n\ntest: %d %f\n\n", doesIntersect, distance);

		light = new Light(vec3(-5, 28, 20)); 
	}
		break;
	case 2:
	case -2: {
		cube = new Cube(vec3(0, 0, 0), vec3(2, 0.3, 2), true);
		Cube* object;
		
		if (playMode == 0) {
			object = new Cube(vec3(0, 0, 0), vec3(0.75, 0.3, 0.75), false); //TODO change true to false and debug(delete negating normals code and also blockcollisioncheck)
			objects.push_back(object);

			phantom = new Phantom(2, vec3(50, (int)(rand() % 2) * 20 + 5, 50), 0.0f, 0.f);
			enemies.push_back(phantom);
			phantom = new Phantom(2, vec3(-50, (int)(rand() % 2) * 20 + 5, 50), 0.0f, 0.f);
			enemies.push_back(phantom);
			phantom = new Phantom(2, vec3(50, (int)(rand() % 2) * 20 + 5, -50), 0.0f, 0.f);
			enemies.push_back(phantom);
			phantom = new Phantom(2, vec3(-50, (int)(rand() % 2) * 20 + 5, -50), 0.0f, 0.f);
			enemies.push_back(phantom);
			phantom = new Phantom(0, vec3(-75, 5, 0), 0.f, 0.f);
			enemies.push_back(phantom);
			phantom = new Phantom(0, vec3(0, 25, -75), 0.f, 0.f);
			enemies.push_back(phantom);

			light = new Light(vec3(75, 28, 75)); // vec3(0, 28, 0)); // 
		}
		else if (playMode == 1) { //coop mode
			object = new Cube(vec3(0, 0, 0), vec3(0.75, 0.3, 0.75), false); //TODO change true to false and debug(delete negating normals code and also blockcollisioncheck)
			objects.push_back(object);

			phantom = new Phantom(2, vec3(50, (int)(rand() % 2) * 20 + 5, 50), 45.0f, 0.f);
			enemies.push_back(phantom);
			phantom = new Phantom(2, vec3(-50, (int)(rand() % 2) * 20 + 5, 50), -45.0f, 0.f);
			enemies.push_back(phantom);
			phantom = new Phantom(2, vec3(50, (int)(rand() % 2) * 20 + 5, -50), 135.0f, 0.f);
			enemies.push_back(phantom);
			phantom = new Phantom(2, vec3(-50, (int)(rand() % 2) * 20 + 5, -50), -135.0f, 0.f);
			enemies.push_back(phantom);
			phantom = new Phantom(0, vec3(-75, 5, -75), -180.f, 0.f);
			enemies.push_back(phantom);
			phantom = new Phantom(0, vec3(75, 5, 75), 0.f, 0.f);
			enemies.push_back(phantom);

			light = new Light(vec3(75, 28, 75)); // vec3(0, 28, 0)); // 
		}
		else if (playMode == 2) { //vs mode
			object = new Cube(vec3(-40, 0, -40), vec3(0.2, 0.3, 0.2), false); 
			objects.push_back(object);
			object = new Cube(vec3(-40, 0, 40), vec3(0.2, 0.3, 0.2), false);
			objects.push_back(object);
			object = new Cube(vec3(40, 0, -40), vec3(0.2, 0.3, 0.2), false);
			objects.push_back(object);
			object = new Cube(vec3(40, 0, 40), vec3(0.2, 0.3, 0.2), false);
			objects.push_back(object);
			
			light = new Light(vec3(0, 28, 0)); // vec3(0, 28, 0)); // 
		}

		cam_pos = vec3(75.0f, 5.0f, 75.0f);
	}
		break;
	case 1: 
	case -1: {
		procedurallyBuild();

		phantom = new Phantom(0, vec3(0, 5, 90), -180.0f, 0.f);
		enemies.push_back(phantom);
		phantom = new Phantom(0, vec3(0, 5, -90), 0.0f, 0.f);
		enemies.push_back(phantom);
		phantom = new Phantom(2, vec3(75, 5, 0), -90.0f, 0.f);
		enemies.push_back(phantom);
		phantom = new Phantom(2, vec3(-75, 5, 0), 90.0f, 0.f);
		enemies.push_back(phantom);
		
		int count = 0;
		while (count < 2) {
			int indexi = (int)rand() % 9;
			int indexj = (int)rand() % 9;
			if (indexi == 4 || indexj == 4 || blockArray[3 + 4 * indexi][3 + 4 * indexj] != 0)
				continue;

			phantom = new Phantom(3, vec3(-80.f+ 20.0f*indexi , 5, -80.f + 20.0f*indexj), 0.0f, 0.f);
			enemies.push_back(phantom);
			printf("\nrandom pos: %f %f\n", -80.f + 20.0f * indexi, -80.f + 20.f * indexj);
			count++;
		}

		cam_pos = vec3(0.0f, 5.0f, 20.0f);

		light = new Light(vec3(0, 50, 0));
	}
		break;
	}

	if (playMode > 0) {
		youwin = new Image2d("youwin.ppm", 1, vec2(0), 0, 0.25, true);
		youlose = new Image2d("youlose.ppm", 1, vec2(0), 0, 0.25, true);
		paralyzationDuration = 3500;
		switch (stageType) {
		case 0:
			playerone = new Player(vec3(-5, 5, 20), cam_look_at, playMode);
			playertwo = new Player(vec3(5, 5, 20), cam_look_at, playMode);
			break;
		case 2:
		case -2:
			playerone = new Player(vec3(75.0f, 5.0f, -75.0f), cam_look_at, playMode);
			playertwo = new Player(vec3(-75.0f, 5.0f, 75.0f), cam_look_at, playMode);
			break;
		case 1:
		case -1:
			playerone = new Player(cam_pos, cam_look_at, playMode);
			playertwo = new Player(cam_pos, cam_look_at, playMode);
			break;
		}
		if (playMode == 2) {
			for (int i = 0; i < playerone->bullet; i++) {
				bullets.push_back(new Image2d("bullet.ppm", 0.075, vec2((1.2 * i + 0.8) * 0.075 - 1.0, - 0.1 + 1.0), 2, 0.9));
			}
			immuneDuration = 3000;
		}
	}

	V = glm::lookAt(cam_pos, cam_look_at, cam_up);

	allAttack = new PhantomAttack(enemies);

	lastTime = clock();
	lastAttacked = lastTime;

	//#########
	//sound
	engine = createIrrKlangDevice(); 

	ssBGM = engine->addSoundSourceFromFile("toikioku_healing.mp3");
	ssBGM->setDefaultVolume(0.1f);
	engine->play2D(ssBGM, true);

	ssShooting = engine->addSoundSourceFromFile("attack1.mp3");
	ssShooting->setDefaultVolume(0.2f);

	ssExplosion= engine->addSoundSourceFromFile("attack3.mp3");
	ssExplosion->setDefaultVolume(0.5f);
	ssExplosion->setDefaultMinDistance(10.0f);

	ssParalyzation= engine->addSoundSourceFromFile("damage2.mp3");
	ssParalyzation->setDefaultVolume(0.5f);
	ssParalyzation->setDefaultMinDistance(10.0f);	
	engine->setListenerPosition(vec3df(cam_pos.x, cam_pos.y, cam_pos.z), vec3df(cam_look_at.x, cam_look_at.y, cam_look_at.z), vec3df(0, 0, 0), vec3df(cam_up.x, cam_up.y, cam_up.z));
	for (Phantom* enemy : enemies) {
		vec4 enemyPostion = enemy->toWorld * vec4(0, 0, 0, 1);
		enemy->paralyzingSound = engine->play3D(ssParalyzation, vec3df(enemyPostion.x, enemyPostion.y, enemyPostion.z), true, true, true);
	}

	ssAttack = engine->addSoundSourceFromFile("damage7.mp3");
	ssAttack->setDefaultVolume(0.3f);
	ssAttack->setDefaultMinDistance(10.0f);

	ssDamage = engine->addSoundSourceFromFile("kick2.mp3");
	ssDamage->setDefaultVolume(0.5f);

	ssStationary = engine->addSoundSourceFromFile("damage5.mp3"); //knocking_a_wall
	ssStationary->setDefaultVolume(0.1f);
	
	ssReload = engine->addSoundSourceFromFile("office_locker_C.mp3");
	ssReload->setDefaultVolume(0.2f);
}

void Window::procedurallyBuild() {
	for (int i = 0; i <= 9; i += 1) {
		for (int j = 0; j <= 9; j += 1) {
			if (i%5 == 0 && j%5 == 0)
				stageArray[i][j] = 0;
			else
				stageArray[i][j] = -1;
		}
	}

	int ioffset, joffset;
	for (int k = 0; k < 4; k++) {	
		ioffset = 0 + (k/2) * 5; 
		joffset = 0 + (k%2) * 5;
		
		//2x2 buildings
		for (int i = ioffset; i < ioffset + 4; i++) {
			for (int j = joffset; j < joffset + 4; j++) {
				if (stageArray[i][j] >= 0 || stageArray[i][j+1] >= 0 || (int)(rand()%2) == 0 ) {
					continue;
				}
				//int buildingShape = rand() %
				stageArray[i][j] = 2;
				stageArray[i+1][j] = 2;
				stageArray[i][j+1] = 2;
				stageArray[i+1][j+1] = 2;
			}
		}
		//1x1 buildings or nothing
		for (int i = ioffset; i < ioffset + 5; i++) {
			for (int j = joffset; j < joffset + 5; j++) {
				if (stageArray[i][j] >= 0) {
					continue;
				}
				if (rand()%8 >= 1)
					stageArray[i][j] = 1;
				else
					stageArray[i][j] = 0;
			}
		}
	}


	//rotate
	/*flip left rectangle then flip top rectangle
		xoxo	oxxo	oooo
		oooo	oooo	oxxo
		xoxo	oxxo	oxxo
		oooo	oooo	oooo
	*/
	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 5/2; j++) {
			int temp = stageArray[i][j];
			stageArray[i][j] = stageArray[i][4 - j];
			stageArray[i][4 - j] = temp;
		}
	}
	for (int i = 0; i < 5/2; i++) {
		for (int j = 0; j < 10; j++) {
			int temp = stageArray[i][j];
			stageArray[i][j] = stageArray[4 - i][j];
			stageArray[4 - i][j] = temp;
		}
	}

	//create objects
	cube = new Cube(vec3(0,0,0), vec3(2,1,2), true, false); //invisible wall
	objects.push_back(new Cube(vec3(0, -100, 0), vec3(2, 1, 2), false)); //ground

	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 10; j++) {
			if (stageArray[i][j] <= 0) {
				if (stageArray[i][j] == 0) {
					for (int x = 0; x < 4; x++)
						for (int y = 0; y < 4; y++)
							blockArray[4 * i + x][4 * j + y] = 0;
				}
				continue;
			}
			else if (stageArray[i][j] == 1) {
				float height = 20.0f + 10.0f * (int)(rand() % 2) + 10.0f * (int)(rand() % 3);
				objects.push_back(new Cube(vec3(-90 + 20.0f * j, 0, -90 + 20.0f * i), vec3(0.1, height/100.0f, 0.1), false));
				
				for (int x = 1; x < 3; x++)
					for (int y = 1; y < 3; y++)
						blockArray[4*i + x][4*j + y] = 1;
				continue;
			}

			int randVal = rand() % 5;
			float height = 10.0f + 5.0f * (int)(rand() % 4);

			if (randVal == 4) {
				objects.push_back(new Cube(vec3(-80 + 20.0f * j, 0, -80 + 20.0f * i), vec3(0.3, height / 100.0f, 0.3), false));
				for (int x = 1; x < 7; x++)
					for (int y = 1; y < 7; y++) 
						blockArray[4 * i + x][4 * j + y] = 1;
					
			}
			else {
				int origSize = objects.size();
				vector<Cube*> newObjects;

				objects.push_back(new Cube(vec3(-90 + 20.0f * j, 0, -90 + 20.0f * i), vec3(0.1, height / 100.0f, 0.1), false));//left top
				objects.push_back(new Cube(vec3(-70 + 20.0f * j, 0, -90 + 20.0f * i), vec3(0.1, height / 100.0f, 0.1), false));//right top
				objects.push_back(new Cube(vec3(-90 + 20.0f * j, 0, -70 + 20.0f * i), vec3(0.1, height / 100.0f, 0.1), false));//left bottom
				objects.push_back(new Cube(vec3(-70 + 20.0f * j, 0, -70 + 20.0f * i), vec3(0.1, height / 100.0f, 0.1), false));

				newObjects.push_back(new Cube(vec3(-80 + 20.0f * j, 0, -90 + 20.0f * i), vec3(0.1, height / 100.0f, 0.1), false));//top
				newObjects.push_back(new Cube(vec3(-90 + 20.0f * j, 0, -80 + 20.0f * i), vec3(0.1, height / 100.0f, 0.1), false));//left
				newObjects.push_back(new Cube(vec3(-70 + 20.0f * j, 0, -80 + 20.0f * i), vec3(0.1, height / 100.0f, 0.1), false));//right
				newObjects.push_back(new Cube(vec3(-80 + 20.0f * j, 0, -70 + 20.0f * i), vec3(0.1, height / 100.0f, 0.1), false));//bottom

				for (int x = 1; x < 7; x++)
					for (int y = 1; y < 7; y++)
						blockArray[4*i + x][4*j + y] = 1;
				for (int x = 3; x < 5; x++)
					for (int y = 3; y < 5; y++)
						blockArray[4 * i + x][4 * j + y] = 0;
				

				int randVal2 = rand() % 4;
				
				//delete two, first one based on one  1-4, second one new 1-4 if same ...(if 1-4) or two(56)
				for (int x = 0; x < newObjects.size(); x++) {
					if(x == randVal || x == randVal2) { //remove
						delete(newObjects[x]);

						blockArray[4*i + 1 + 2 * (x%2 + x/2)][4*j + 1 + 2 * (1 - x % 2 + x / 2)] = 0;
						blockArray[4 * i + 1 + 2 * (x % 2 + x / 2) + 1][4 * j + 1 + 2 * (1 - x % 2 + x / 2)] = 0;
						blockArray[4 * i + 1 + 2 * (x % 2 + x / 2)][4 * j + 1 + 2 * (1 - x % 2 + x / 2) + 1] = 0;
						blockArray[4 * i + 1 + 2 * (x % 2 + x / 2) + 1][4 * j + 1 + 2 * (1 - x % 2 + x / 2) + 1] = 0;
					}
					else { //add
						objects.push_back(newObjects[x]);
					}
				}
				if ((randVal + randVal2)%3 != 0 && randVal != randVal2 && (int)(rand()%3) == 0) {
					//x x
					//  x
					//xxx
					int idx = randVal + randVal2 + (randVal + randVal2) / 4 - 1;
					Cube* toDelete = objects[objects.size() - 6 + idx];
					objects.erase(objects.begin() + objects.size() - 6 + idx);
					delete(toDelete);

					blockArray[4 * i + 1 + 4 * (idx / 2)][4 * j + 1 + 4 * (idx % 2)] = 0;
					blockArray[4 * i + 1 + 4 * (idx / 2) + 1][4 * j + 1 + 4 * (idx % 2)] = 0;
					blockArray[4 * i + 1 + 4 * (idx / 2)][4 * j + 1 + 4 * (idx % 2) + 1] = 0;
					blockArray[4 * i + 1 + 4 * (idx / 2) + 1][4 * j + 1 + 4 * (idx % 2) + 1] = 0;
				}
			}
			stageArray[i][j] *= -1;
			stageArray[i+1][j] *= -1;
			stageArray[i][j+1] *= -1;
			stageArray[i+1][j+1] *= -1;
		}
	}

	//print
	for (int i = 0; i <= 9; i += 1) {
		for (int j = 0; j <= 9; j += 1) {
			printf("%d ", stageArray[i][j]);
		}
		printf("\n");
	}

	for (int i = 0; i < 40; i++) {
		for (int j = 0; j < 40; j++)
			printf("%d", blockArray[i][j]);
		printf("\n");
	}
}

// Treat this as a destructor func`tion. Delete dynamically allocated memory here.
void Window::clean_up()
{
    delete(skybox);
    delete(cube);
	delete(cursor);
	delete(gameover);
	delete(gameclear);
	for (Image2d* heart : hearts)
		delete(heart);
	delete(allAttack);

	glDeleteProgram(shaderProgram);
	glDeleteProgram(shaderProgramAmbient);
	glDeleteProgram(shaderProgramNull);
	glDeleteProgram(shaderProgramDeferred); 
	
	engine->removeAllSoundSources();
	engine->drop();

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

	if (playMode > 0) {
		width *= 2;
	}

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
	Window::width = (playMode > 0) ? width/2 : width;
	Window::height = height;
	// Set the viewport size. This is the only matrix that OpenGL maintains for us in modern OpenGL!
	glViewport(0, 0, width, height);

	if (height > 0)
	{
		P = glm::perspective(45.0f, (float)width / (float)height, 0.1f, 1000.0f);
        //get the 4 points of the camera perspective
		V = glm::lookAt(cam_pos, cam_look_at, cam_up);

		//update 2d images
		if (cursor)
			cursor->updateScale(width, height);
		if (gameover)
			gameover->updateScale(width, height);
		if (gameclear)
			gameclear->updateScale(width, height);
		if (youwin)
			youwin->updateScale(width, height);
		if (youlose)
			youlose->updateScale(width, height);
		if (!hearts.empty())
			for (int i = 0; i < hearts.size(); i++)
				hearts[i]->updateScale(width, height);
	}
}

void Window::moveEnemies() {
	for (int i = 0; i < enemies.size(); i++) {
		Phantom* enemy = enemies[i];

		enemy->animate();
		if (playMode > 0)
			cam_pos = (i % 2 == 0) ? playerone->cam_pos : playertwo->cam_pos;
		enemy->move(cam_pos);
		enemy->wallCollisionCheck(cube);
		for (Cube* object : objects)
			enemy->blockCollisionCheck(object);
		enemy->phantomCollisionCheck(enemies);

		//check collision with player
		if (playMode == 0) {
			if (length(vec3(enemy->toWorld * vec4(0, 0, 0, 1)) - cam_pos) <= playerRadius + enemy->radius) {
				processPlayerDamage();
			}
		}
		else {
			// player one
			if (length(vec3(enemy->toWorld * vec4(0, 0, 0, 1)) - playerone->cam_pos) <= playerone->playerRadius + enemy->radius) {
				lastAttacked = playerone->lastAttacked;
				gameStatus = playerone->gameStatus;
				playerLife = playerone->playerLife;
				processPlayerDamage();
				playerone->lastAttacked = lastAttacked;
				playerone->gameStatus = gameStatus;
				playerone->playerLife = playerLife;
				if (playerLife == 0) {
					playerone->moving.clear();
					turning.clear();
					if (playMode == 2) // if multi versus
						playertwo->gameStatus = 1; //since 1P is killed, 2P wins
				}
			}
			// player two
			if (length(vec3(enemy->toWorld * vec4(0, 0, 0, 1)) - playertwo->cam_pos) <= playertwo->playerRadius + enemy->radius) {
				lastAttacked = playertwo->lastAttacked;
				gameStatus = playertwo->gameStatus;
				playerLife = playertwo->playerLife;
				processPlayerDamage();
				playertwo->lastAttacked = lastAttacked;
				playertwo->gameStatus = gameStatus;
				playertwo->playerLife = playerLife;
				if (playerLife == 0) {
					playertwo->moving.clear();
					if (playMode == 2)
						playerone->gameStatus = 1; //since 2P is killed, 1P wins
				}
			}
		}
	}
}

void moveLight() {
	switch (stageType) {
	case 0: {
	}
		break;
	case 2: { //cube
		if (playMode == 0) {
			clock_t curTime = clock();
			float deltaTime = curTime - lastTime;
			if (deltaTime > 7000) {
				light->pos = vec3((int)(rand() % 2) * 150.0f - 75.0f, light->pos.y, (int)(rand() % 2) * 150.0f - 75.0f);
				lastTime = curTime;
			}
		}
		else if (playMode == 1) {
			clock_t curTime = clock();
			float deltaTime = curTime - lastTime;
			if (deltaTime > 8000) {
				light->pos = vec3((int)(rand() % 2) * 150.0f - 75.0f, light->pos.y, (int)(rand() % 2) * 150.0f - 75.0f);
				lastTime = curTime;
			}
		}
		else if (playMode == 2) {
			if (lightMovingCount % 800 == 0) {
				lightMovingDir = (int)(rand() % 20) / 5 + 1;
				lightMovingCount = 0;
				//printf("dir: %d\n", lightMovingDir);
			}
			switch (lightMovingDir) {
			case 1: //left
				if (light->pos.x <= -80) {
					lightMovingCount = 0;
					return;
				}
				light->pos += vec3(-0.1, 0, 0);
				break;
			case 2: //right
				if (light->pos.x >= 80) {
					lightMovingCount = 0;
					return;
				}
				light->pos += vec3(0.1, 0, 0);
				break;
			case 3: //up
				if (light->pos.z <= -80) {
					lightMovingCount = 0;
					return;
				}
				light->pos += vec3(0, 0, -0.1);
				break;
			case 4: //down
				if (light->pos.z >= 80) {
					lightMovingCount = 0;
					return;
				}
				light->pos += vec3(0, 0, 0.1);
				break;
			}
			lightMovingCount += 1;
		}
	}
		break;
	case 1: { //city
		if (lightMovingCount % 800 == 0){//(fmod(light->pos.x, 20.0f) <= 0.001 || fmod(light->pos.x, 20.0f) >= 19.999) && (fmod(light->pos.z, 20.0f) <= 0.001 || fmod(light->pos.z, 20.0f) <= 19.999)) {
			/*int newDir = (int)rand() % 4 + 1; 
			if ((lightMovingDir + newDir) % 4 == 3)
				lightMovingDir = (int)rand() % 4 + 1;
			else
				lightMovingDir = newDir;*/
			int newDir = (int)(rand() % 20) / 4 + 1;// (int)rand() % 5 + 1;
			if (newDir != 5)
				lightMovingDir = newDir;
			lightMovingCount = 0;
		}
		switch (lightMovingDir) {
		case 1: //left
			if (light->pos.x <= -120) {
				lightMovingCount = 0;
				return;
			}
			light->pos += vec3(-0.05, 0, 0);
			break;
		case 2: //right
			if (light->pos.x >= 120) {
				lightMovingCount = 0;
				return;
			}
			light->pos += vec3(0.05, 0, 0);
			break;
		case 3: //up
			if (light->pos.z <= -120) {
				lightMovingCount = 0;
				return;
			}
			light->pos += vec3(0, 0, -0.05);
			break;
		case 4: //down
			if (light->pos.z >= 120) {
				lightMovingCount = 0;
				return;
			}
			light->pos += vec3(0, 0, 0.05);
			break;
		}
		lightMovingCount += 1;
	}
		break;
	}
}

//only for 1P when multi player mode
void turnCamera1P() {
	cam_pos = playerone->cam_pos;
	cam_look_at = playerone->cam_look_at;

	playerone->updateToWorld();
	vec3 yaxis = playerone->toWorld * vec4(0, 1, 0, 0);
	vec3 xaxis = playerone->toWorld * vec4(-1, 0, 0, 0);
	switch (turning.back()) {
	// 0: stationary 1 : left 2 : right 3 : up 4 : down
	case 0:
		return;
		break;
	case 1:
		Window::rotateCamera(0.4f / 180.0f * pi<float>(), yaxis);
		break;
	case 2:
		Window::rotateCamera(-0.4f / 180.0f * pi<float>(), yaxis);
		break;
	case 3:
		Window::rotateCamera(0.4f / 180.0f * pi<float>(), xaxis);
		break;
	case 4:
		Window::rotateCamera(-0.4f / 180.0f * pi<float>(), xaxis);
		break;
	}

	vec3 view_dir = normalize(cam_look_at - playerone->cam_pos);
	if (abs(dot(view_dir, cam_up)) > 0.9) {
		return;
	}

	playerone->cam_look_at = cam_look_at;
}

void Window::idle_callback()
{
	if (playMode == 0) {
		if (!moving.empty()) 
			moveCamera();
		else 
			lastMove = 0;
	}
	else {
		 //player one
		if (!turning.empty()) 
			turnCamera1P();
		if (!playerone->moving.empty()) {
			currPlayer = playerone;
			moving = playerone->moving;
			cam_pos = playerone->cam_pos;
			cam_look_at = playerone->cam_look_at;
			playerRadius = playerone->playerRadius;
			moveCamera();
		}
		else {
			playerone->lastMove = 0;
		}
		//player two
		if (!playertwo->moving.empty()) { 
			currPlayer = playertwo;
			moving = playertwo->moving;
			cam_pos = playertwo->cam_pos;
			cam_look_at = playertwo->cam_look_at;
			playerRadius = playertwo->playerRadius;
			moveCamera();	
		}
		else {
			playertwo->lastMove = 0;
		}
	}

	// setting for sound
	if (playMode == 0)
		engine->setListenerPosition(vec3df(cam_pos.x, cam_pos.y, cam_pos.z), vec3df(cam_look_at.x, cam_look_at.y, cam_look_at.z), vec3df(0, 0, 0), vec3df(cam_up.x, cam_up.y, cam_up.z));
	else {
		vec3 temp = playerone->cam_pos + playertwo->cam_pos;
		vec3df listener_pos = vec3df(temp.x / 2, temp.y / 2, temp.z / 2);
		temp = playerone->cam_look_at + playertwo->cam_look_at;
		vec3df listener_look_at = vec3df(temp.x / 2, temp.y / 2, temp.z / 2);
		engine->setListenerPosition(listener_pos, listener_look_at, vec3df(0, 0, 0), vec3df(cam_up.x, cam_up.y, cam_up.z));
	}

	//clock_t curTime = clock();
	if (toggleMove && playMode < 2)
		moveEnemies();

	moveLight();

	if (playMode < 2) {
		processParalyzationAndDeath();
		allAttack->updateAttacks(enemies);
	}
	//versus mode
	else if (playMode == 2) {
		playerone->processRIP();
		playertwo->processRIP();
	}
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
		view_dir = -normalize(view_dir - vec3(0, view_dir.y, 0));
		break;
	}

	float speed = 0.2f;

	//check whether ran into wall, only checks x and z
	vec3 new_cam_pos = cam_pos + speed * view_dir;
	vec3 new_cam_look_at = cam_look_at + speed * view_dir;
	bool needToCheckWall = true;
	vec3 maxVertex, minVertex;
	
	if (stageType * stageType != 1) { //if not city
		for (Cube* block : objects) {
			maxVertex = block->maxVertex;
			minVertex = block->minVertex;
			if (new_cam_pos.x - playerRadius < maxVertex.x && new_cam_pos.x + playerRadius > minVertex.x
						&& new_cam_pos.z - playerRadius < maxVertex.z && new_cam_pos.z + playerRadius > minVertex.z) { // hits block
				new_cam_pos = cam_pos + vec3(0, 0, speed * view_dir.z);
				new_cam_look_at = cam_look_at + vec3(0, 0, speed * view_dir.z);

				if (new_cam_pos.x - playerRadius < maxVertex.x && new_cam_pos.x + playerRadius > minVertex.x
							&& new_cam_pos.z - playerRadius < maxVertex.z && new_cam_pos.z + playerRadius > minVertex.z) { // hits block when only move in z dir
					new_cam_pos = cam_pos + vec3(speed * view_dir.x, 0, 0);
					new_cam_look_at = cam_look_at + vec3(speed * view_dir.x, 0, 0);

					if (new_cam_pos.x - playerRadius < maxVertex.x && new_cam_pos.x + playerRadius > minVertex.x
								&& new_cam_pos.z - playerRadius < maxVertex.z && new_cam_pos.z + playerRadius > minVertex.z) { // hits block when only move in x dir
						new_cam_pos = cam_pos;
						new_cam_look_at = cam_look_at; // don't move
					}
				}
				needToCheckWall = false;
				break;
			}
		}
	}
	else { //city
		if (blockArray[(int)(new_cam_pos.z + 100 - playerRadius) / 5][(int)(new_cam_pos.x + 100 - playerRadius) / 5] == 1 || 
			blockArray[(int)(new_cam_pos.z + 100 - playerRadius) / 5][(int)(new_cam_pos.x + 100 + playerRadius) / 5] == 1 ||
			blockArray[(int)(new_cam_pos.z + 100 + playerRadius) / 5][(int)(new_cam_pos.x + 100 - playerRadius) / 5] == 1 ||
			blockArray[(int)(new_cam_pos.z + 100 + playerRadius) / 5][(int)(new_cam_pos.x + 100 + playerRadius) / 5] == 1) { // hits block
				
			new_cam_pos = cam_pos + vec3(speed * view_dir.x, 0, 0);
			new_cam_look_at = cam_look_at + vec3(speed * view_dir.x, 0, 0);
			

			if (blockArray[(int)(new_cam_pos.z + 100 - playerRadius) / 5][(int)(new_cam_pos.x + 100 - playerRadius) / 5] == 1 ||
				blockArray[(int)(new_cam_pos.z + 100 - playerRadius) / 5][(int)(new_cam_pos.x + 100 + playerRadius) / 5] == 1 ||
				blockArray[(int)(new_cam_pos.z + 100 + playerRadius) / 5][(int)(new_cam_pos.x + 100 - playerRadius) / 5] == 1 ||
				blockArray[(int)(new_cam_pos.z + 100 + playerRadius) / 5][(int)(new_cam_pos.x + 100 + playerRadius) / 5] == 1) { // hits block when only move in x dir

				new_cam_pos = cam_pos + vec3(0, 0, speed * view_dir.z);
				new_cam_look_at = cam_look_at + vec3(0, 0, speed * view_dir.z);
			
				if (blockArray[(int)(new_cam_pos.z + 100 - playerRadius) / 5][(int)(new_cam_pos.x + 100 - playerRadius) / 5] == 1 ||
					blockArray[(int)(new_cam_pos.z + 100 - playerRadius) / 5][(int)(new_cam_pos.x + 100 + playerRadius) / 5] == 1 ||
					blockArray[(int)(new_cam_pos.z + 100 + playerRadius) / 5][(int)(new_cam_pos.x + 100 - playerRadius) / 5] == 1 ||
					blockArray[(int)(new_cam_pos.z + 100 + playerRadius) / 5][(int)(new_cam_pos.x + 100 + playerRadius) / 5] == 1) { // hits block when only move in z dir

					new_cam_pos = cam_pos;
					new_cam_look_at = cam_look_at; // don't move
				}	
			}
			needToCheckWall = false;
		}
	}

	Cube* wall = cube;
	if (needToCheckWall) {
		maxVertex = wall->maxVertex;
		minVertex = wall->minVertex;
		if (new_cam_pos.x + playerRadius > maxVertex.x || new_cam_pos.x - playerRadius < minVertex.x
			|| new_cam_pos.z + playerRadius > maxVertex.z || new_cam_pos.z - playerRadius < minVertex.z) { // hits wall

			new_cam_pos = cam_pos + vec3(0, 0, speed * view_dir.z);
			new_cam_look_at = cam_look_at + vec3(0, 0, speed * view_dir.z);

			if (new_cam_pos.z + playerRadius > maxVertex.z || new_cam_pos.z - playerRadius < minVertex.z) { // hits wall when only move in z dir
				new_cam_pos = cam_pos + vec3(speed * view_dir.x, 0, 0);
				new_cam_look_at = cam_look_at + vec3(speed * view_dir.x, 0, 0);

				if (new_cam_pos.x + playerRadius > maxVertex.x || new_cam_pos.x - playerRadius < minVertex.x) { // hits wall when only move in x dir
					new_cam_pos = cam_pos;
					new_cam_look_at = cam_look_at; // don't move
				}
			}
		}
	}
	
	//case when players hit each other
	if (playMode > 0) { 
		currPlayer->cam_pos = new_cam_pos;
		if (length(playerone->cam_pos - playertwo->cam_pos) <= playerone->playerRadius + playertwo->playerRadius) {
			new_cam_pos = cam_pos;
			new_cam_look_at = cam_look_at;
		}
		currPlayer->cam_pos = cam_pos;
	}

	if (length(new_cam_pos - cam_pos) == 0) {
		if (playMode == 0) { //single player
			if (moved || lastMove != moving.back()) {
				engine->play2D(ssStationary);
			}
			moved = false;
		}
		else {
			if (currPlayer->moved || currPlayer->lastMove != moving.back()) {
				engine->play2D(ssStationary);
			}
			currPlayer->moved = false;
		}
	}
	else {
		if (playMode == 0) { //single player
			moved = true;
		}
		else {
			currPlayer->moved = true;
		}
	}

	cam_pos = new_cam_pos;
	cam_look_at = new_cam_look_at;
	Window::V = glm::lookAt(cam_pos, cam_look_at, cam_up);
	lastMove = moving.back();

	if (playMode > 0) {
		currPlayer->cam_pos = new_cam_pos;
		currPlayer->cam_look_at = new_cam_look_at;
		currPlayer->lastMove = moving.back();
	}
}
/*
void processShadowShooting() {
	//set up
	glEnable(GL_STENCIL_TEST);
	glClear(GL_STENCIL_BUFFER_BIT);

	glDrawBuffer(GL_NONE); //or GL_BACK
	glDepthMask(GL_FALSE);
	glEnable(GL_DEPTH_CLAMP);
	glDisable(GL_CULL_FACE);

	glStencilFunc(GL_ALWAYS, 0, 0xff);
	glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
	glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);

	glUseProgram(shaderProgramDeferred);

	int pixelStencil;
	for (Phantom* enemy : enemies) {
		//render into stencil
		vec3 lightPosInEnemyWorld = inverse(enemy->toWorld) * vec4(light->pos, 1.0);
		glUniform3fv(glGetUniformLocation(shaderProgramDeferred, "lightPos"), 1, &(lightPosInEnemyWorld[0]));
		enemy->drawWithGeom(shaderProgramDeferred, light->pos);

		glReadPixels((playMode > 0) ? Window::width * 3 / 2 : Window::width / 2, Window::height / 2, 1, 1, GL_STENCIL_INDEX, GL_INT, &pixelStencil);
		printf("shadow's stencil: %d\n", pixelStencil);

		if (pixelStencil >= 1) { //shooted shadow!!
			printf("\nshooted paralyzed!!!\n");
			enemy->paralyze();

			//playsound
			if (enemy->paralyzingSound) {
				enemy->paralyzingSound->setIsPaused(false);
			}
		}

		glClear(GL_STENCIL_BUFFER_BIT);
	}

	// Restore local stuff
	glUniform3fv(glGetUniformLocation(shaderProgramDeferred, "lightPos"), 1, &(light->pos[0]));
	glUseProgram(shaderProgram);

	glDepthMask(GL_TRUE);
	glDisable(GL_DEPTH_CLAMP);
	glEnable(GL_CULL_FACE);

	glDisable(GL_STENCIL_TEST);
	glDrawBuffer(GL_FRONT_AND_BACK); //or GL_BACK
}
*/
void Window::processShooting() {
	vec4 pixelColor;
	int pixelStencil;
	if (playMode == 0 || currPlayer == playerone) {
		glReadPixels(Window::width / 2, Window::height / 2, 1, 1, GL_RGBA, GL_FLOAT, &pixelColor[0]);
		glReadPixels(Window::width / 2, Window::height / 2, 1, 1, GL_STENCIL_INDEX, GL_INT, &pixelStencil);
	}
	else if (playMode > 0 && currPlayer == playertwo) {
		glReadPixels(Window::width * 3 / 2 , Window::height / 2, 1, 1, GL_RGBA, GL_FLOAT, &pixelColor[0]);
		glReadPixels(Window::width * 3 / 2, Window::height / 2, 1, 1, GL_STENCIL_INDEX, GL_INT, &pixelStencil);
	}
	printf("\npixel color: %f %f %f %f\n", pixelColor.x, pixelColor.y, pixelColor.z, pixelColor.w);
	//printf("pixel stencil: %d\n", pixelStencil);
	//printf("pixel equal: %d\n", pixelColor.x == pixelColor.z);

	vec3 viewDir = cam_look_at - cam_pos;
	
	//set up
	glEnable(GL_STENCIL_TEST);
	glClear(GL_STENCIL_BUFFER_BIT);

	glDrawBuffer(GL_NONE); //or GL_BACK
	glDepthMask(GL_FALSE);
	glEnable(GL_DEPTH_CLAMP);
	glDisable(GL_CULL_FACE);

	glStencilFunc(GL_ALWAYS, 0, 0xff);
	glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
	glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);

	Phantom* hit = nullptr;

	//phantom/enemy/object shooting
	glUseProgram(shaderProgram);
	if (playMode == 0 || (playMode == 1 && currPlayer == playerone)) {
		for (Phantom* enemy : enemies) {
			//render into stencil

			enemy->draw(shaderProgram);

			glReadPixels(Window::width / 2, Window::height / 2, 1, 1, GL_STENCIL_INDEX, GL_INT, &pixelStencil);
			printf("phantom's stencil: %d\n", pixelStencil);
			
			glClear(GL_STENCIL_BUFFER_BIT);
			if (pixelStencil >= 1) { //shooted shadow!! shooted->paralyzed) {
				if (!enemy->paralyzed) {
					hit = enemy;
					break;
				}
				
				enemy->die();
				printf("shooted dead!!!\n");

				//playsound
				vec4 shootedPostion = enemy->toWorld * vec4(0, 0, 0, 1);
				//engine->setListenerPosition(vec3df(cam_pos.x, cam_pos.y, cam_pos.z), vec3df(cam_look_at.x, cam_look_at.y, cam_look_at.z), vec3df(0, 0, 0), vec3df(cam_up.x, cam_up.y, cam_up.z));
				engine->play3D(ssExplosion, vec3df(shootedPostion.x, shootedPostion.y, shootedPostion.z));

				break;
			}
		}
	}

	//shadow shooting
	glUseProgram(shaderProgramDeferred);
	if (playMode == 0 || (playMode == 1 && currPlayer == playertwo)) {
		for (Phantom* enemy : enemies) {
			//render into stencil
			vec3 lightPosInEnemyWorld = inverse(enemy->toWorld) * vec4(light->pos, 1.0);
			glUniform3fv(glGetUniformLocation(shaderProgramDeferred, "lightPos"), 1, &(lightPosInEnemyWorld[0]));
			enemy->drawWithGeom(shaderProgramDeferred, light->pos);

			glReadPixels((playMode > 0) ? Window::width * 3 / 2 : Window::width / 2, Window::height / 2, 1, 1, GL_STENCIL_INDEX, GL_INT, &pixelStencil);
			printf("shadow's stencil: %d\n", pixelStencil);
			glClear(GL_STENCIL_BUFFER_BIT);
			if (pixelStencil >= 1) { //shooted shadow!!
				if (hit && hit == enemy) { // to avoid killing enemy by shooting its shade instead of its shadow
					continue;
				}
				printf("shooted paralyzed!!!\n");
				enemy->paralyze();

				//playsound
				if (enemy->paralyzingSound) {
					enemy->paralyzingSound->setIsPaused(false);
				}
			}
		}
	}

	//versus mode
	if (playMode == 2) {
		Player* otherPlayer = (currPlayer == playerone) ? playertwo : playerone;
		bool hit = false;

		//enemy shooting
		glUseProgram(shaderProgram);
		glDepthMask(GL_TRUE);

		glDisable(GL_STENCIL_TEST);
		otherPlayer->draw(shaderProgram, true);

		glEnable(GL_STENCIL_TEST);
		glDepthMask(GL_FALSE);
		otherPlayer->draw(shaderProgram, true);
		glReadPixels((currPlayer == playerone) ? Window::width / 2 : Window::width * 3 / 2, Window::height / 2, 1, 1, GL_STENCIL_INDEX, GL_INT, &pixelStencil);
		printf("phantom's stencil: %d\n", pixelStencil);
		if (pixelStencil >= 1) {
			printf("hit\n");
			hit = true;
			lastAttacked = otherPlayer->lastAttacked;
			gameStatus = otherPlayer->gameStatus;
			playerLife = otherPlayer->playerLife;
			processPlayerDamage();
			otherPlayer->lastAttacked = lastAttacked;
			otherPlayer->gameStatus = gameStatus;
			otherPlayer->playerLife = playerLife;
			if (playerLife == 0) {
				otherPlayer->moving.clear();
				turning.clear();
				currPlayer->gameStatus = 1; //since other is killed, curr wins
			}
			else if (!otherPlayer->immune) {
				otherPlayer->immune = true;
			}
		}
		glClear(GL_STENCIL_BUFFER_BIT);

		//shadow shooting
		glUseProgram(shaderProgramDeferred);
		otherPlayer->drawWithGeom(shaderProgramDeferred, light->pos);
		glReadPixels((currPlayer == playerone) ? Window::width / 2 : Window::width * 3 / 2, Window::height / 2, 1, 1, GL_STENCIL_INDEX, GL_INT, &pixelStencil);
		printf("shadow's stencil: %d\n", pixelStencil);
		if (pixelStencil >= 1 && !otherPlayer->immune && !hit) { //shooted shadow!!
			otherPlayer->paralyzed = true;
			otherPlayer->paralyzedTime = clock();
			printf("paralyzed\n");
		}
		glClear(GL_STENCIL_BUFFER_BIT);
	}

	// Restore local stuff
	glUseProgram(shaderProgramDeferred);
	glUniform3fv(glGetUniformLocation(shaderProgramDeferred, "lightPos"), 1, &(light->pos[0]));
	glUseProgram(shaderProgram);

	glDepthMask(GL_TRUE);
	glDisable(GL_DEPTH_CLAMP);
	glEnable(GL_CULL_FACE);

	glDisable(GL_STENCIL_TEST);
	glDrawBuffer(GL_FRONT_AND_BACK); //or GL_BACK

	/*
	if (pixelStencil <= 0 || pixelColor.x != pixelColor.z) { //if not shadow or if shadow on enemy
	//########
	//phantom shooting
	//########
		if (playMode == 1 && currPlayer != playerone) //multi co-op, since playerone is the terminator (responsible for killing) if not playerone then terminate
			return;

		if (pixelColor.x != pixelColor.z) { //is enemy
			Phantom* shooted = nullptr;
			float minDistance = 1000.0f; // TODO this might not be max enough for initial min value
			viewDir = normalize(viewDir);
			for (Phantom* enemy : enemies) {
				if (enemy->dead) 
					continue;
				vec3 camToEnemy = vec3(enemy->toWorld * vec4(0, 0, 0, 1)) - cam_pos;
				float distanceThruViewDir = dot(viewDir, camToEnemy);
				if (glm::pow(enemy->radius, 2.0) >= glm::pow(length(camToEnemy), 2.0) - glm::pow(distanceThruViewDir, 2.0)) { // a^2 = c^2 - b^2
					if (minDistance > distanceThruViewDir) {
						minDistance = distanceThruViewDir;
						shooted = enemy;
					}
				}
			}
			if (shooted != nullptr && shooted->paralyzed) {
				shooted->die();
				printf("\nshooted dead!!!\n");

				//playsound
				vec4 shootedPostion = shooted->toWorld * vec4(0, 0, 0, 1);
				//engine->setListenerPosition(vec3df(cam_pos.x, cam_pos.y, cam_pos.z), vec3df(cam_look_at.x, cam_look_at.y, cam_look_at.z), vec3df(0, 0, 0), vec3df(cam_up.x, cam_up.y, cam_up.z));
				engine->play3D(ssExplosion, vec3df(shootedPostion.x, shootedPostion.y, shootedPostion.z));
			}
		}
		return;
	}
	else if (pixelStencil > 0) { //is shadow
	//########
	//shadow shooting
	//########
		if (playMode == 1 && currPlayer != playertwo) //multi co-op, since playertwo is the paralyzer (responsible for shooting the shadow) if not playertwo then terminate
			return;

		processShadowShooting();
	}
	*/	
}

void  Window::processParalyzationAndDeath() {
	clock_t currentTime = clock();
	Phantom* enemy;
	int shootedEnemy = -1;
	for (int i = 0; i < enemies.size(); i++) {
		enemy = enemies[i];
		if (enemy->paralyzed && currentTime - enemy->paralyzedTime >= paralyzationDuration) {
			enemy->paralyzed = false;

			//stopsound
			enemy->paralyzingSound->setIsPaused(true);
		}
		if (enemy->dead && enemy->die()) { //TODO change paralyzationDuration
			allAttack->attacks.push_back(allAttack->attacks[i]);
			allAttack->attacks.erase(allAttack->attacks.begin() + i);

			enemies.erase(enemies.begin() + i);
			delete(enemy);
			printf("\n curr size of enemies: %d\n", enemies.size());

			if (enemies.empty()) {
				gameStatus = 1;
				moving.clear();
				if (playMode > 0) {
					playerone->gameStatus = 1;
					playertwo->gameStatus = 1;
					playerone->moving.clear();
					playertwo->moving.clear();
				}
				printf("\n\n\nCLEAR\nCLEAR\nCLEAR\nCLEAR\nCLEAR\nCLEAR\nCLEAR\nCLEAR\n");
			}
		}
	}
}

void  Window::processSingleAttack(int attackIndex) {
	Attack& att = allAttack->attacks[attackIndex];
	vec3 attackPosition = att.toWorld * vec4(0,0,0,1);

	//player check
	if (playMode == 0) {
		if (length(attackPosition - cam_pos) <= playerRadius) {
			processPlayerDamage();
			att.active = false;
			return;
		}
	}
	else {
		if (length(attackPosition - playerone->cam_pos) <= playerone->playerRadius) {
			lastAttacked = playerone->lastAttacked;
			gameStatus = playerone->gameStatus;
			playerLife = playerone->playerLife;
			processPlayerDamage();
			playerone->lastAttacked = lastAttacked;
			playerone->gameStatus = gameStatus;
			playerone->playerLife = playerLife;
			if (playerLife == 0) {
				playerone->moving.clear();
				turning.clear();
				if (playMode == 2) // if multi versus
					playertwo->gameStatus = 1; //since 1P is killed, 2P wins
				else if (playMode == 1)
					playertwo->gameStatus = 2; //dies together
			}

			att.active = false;
			return;
		}
		if (length(attackPosition - playertwo->cam_pos) <= playertwo->playerRadius) {
			lastAttacked = playertwo->lastAttacked;
			gameStatus = playertwo->gameStatus;
			playerLife = playertwo->playerLife;
			processPlayerDamage();
			playertwo->lastAttacked = lastAttacked;
			playertwo->gameStatus = gameStatus;
			playertwo->playerLife = playerLife;
			if (playerLife == 0) {
				playertwo->moving.clear();
				if (playMode == 2)
					playerone->gameStatus = 1; //since 2P is killed, 1P wins
				else if (playMode == 1)
					playerone->gameStatus = 2; //dies together
			}

			att.active = false;
			return;
		}
	}

	//block check
	vec3 maxVertex, minVertex;
	for (Cube* block : objects) {
		maxVertex = block->maxVertex;
		minVertex = block->minVertex;
		if (attackPosition.x < maxVertex.x && attackPosition.x > minVertex.x
			&& attackPosition.y < maxVertex.y && attackPosition.y > minVertex.y
			&& attackPosition.z < maxVertex.z && attackPosition.z > minVertex.z) { // hits block

			att.active = false;
			return;
		}
	}

	//wall check
	Cube* wall = cube;
	maxVertex = wall->maxVertex;
	minVertex = wall->minVertex;
	if (attackPosition.x > maxVertex.x || attackPosition.x < minVertex.x
		|| attackPosition.y > maxVertex.y || attackPosition.y < minVertex.y
		|| attackPosition.z > maxVertex.z || attackPosition.z < minVertex.z) { // hits wall

		att.active = false;
		return;
	}
}

void Window::processPlayerDamage() {
	clock_t currTime = clock();
	//return; //etnakaha
	if (currTime - lastAttacked >= immuneDuration && 	gameStatus == 0) {
		//playsound
		if (--playerLife >= 0 && gameStatus == 0) {
			engine->play2D(ssDamage);
			//Image2d* toDelete = hearts.back();
			//hearts.pop_back();
			//delete(toDelete);
			if (playerLife == 0) {
				gameStatus = 2;
				moving.clear();
				printf("\nDEAD\nDEAD\nDEAD\nDEAD\nDEAD\nDEAD\nDEAD\nDEAD\nDEAD");
				return; //not sure etnakaha
			}
		}
		lastAttacked = currTime;
	}
}

void Window::display_callback(GLFWwindow* window) {
	glClear(GL_COLOR_BUFFER_BIT);

	if (playMode == 0) {
		draw();
	}
	else {
		glViewport(0, 0, Window::width, Window::height);
		cam_pos = playerone->cam_pos;
		cam_look_at = playerone->cam_look_at;
		playerRadius = playerone->playerRadius;
		playerLife = playerone->playerLife;
		shooting = playerone->shooting;
		gameStatus = playerone->gameStatus;
		currPlayer = playerone;
		P = glm::perspective(45.0f, (float)width / (float)height, 0.1f, 1000.0f);
		V = glm::lookAt(cam_pos, cam_look_at, cam_up);
		draw();
		playerone->shooting = shooting;

		glViewport(Window::width, 0, Window::width, Window::height);
		cam_pos = playertwo->cam_pos;
		cam_look_at = playertwo->cam_look_at;
		playerRadius = playertwo->playerRadius;
		playerLife = playertwo->playerLife;
		shooting = playertwo->shooting;
		gameStatus = playertwo->gameStatus;
		currPlayer = playertwo;
		P = glm::perspective(45.0f, (float)width / (float)height, 0.1f, 1000.0f);
		V = glm::lookAt(cam_pos, cam_look_at, cam_up);
		draw();
		playertwo->shooting = shooting;
		//glViewport(0, 0, Window::width*2, Window::height);
	}

	// draw is done
	glfwPollEvents();
	glfwSwapBuffers(window);
}

void Window::draw() {
	// for depth test
	glDepthMask(GL_TRUE);
	// Clear the color and depth buffers
	glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

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

// BELOW: draw the remaining
	// Use the shader of programID
	glUseProgram(shaderProgram);
	glDepthMask(GL_TRUE); //enable writing to depth buffer

	skybox->draw(shaderProgram);
	light->draw(shaderProgram);
	for (Phantom* enemy : enemies) {
		if (enemy->dead)
			enemy->draw(shaderProgram);
	}

	if (shooting) {
		if (playMode < 2) {
			//playsound
			engine->play2D(ssShooting, false);
			processShooting();
			shooting = false;
		}
		else {
			int shootingStatus = currPlayer->processShooting();
			if (shootingStatus == 1) {
				engine->play2D(ssShooting, false);
				processShooting();
			}
			else if (shootingStatus == 2) {
				engine->play2D(ssReload, false);
			}
		}
		shooting = false;
	}

	if (showShadowVolume) {
		glUseProgram(shaderProgramDeferred);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDisable(GL_CULL_FACE);

		for (Cube* object : objects) {
			vec3 lightPosInObjectWorld = inverse(object->toWorld) * vec4(light->pos, 1.0);
			object->drawWithGeom(shaderProgramDeferred, light->pos);
		}
		if (playMode > 0) {
			if (currPlayer == playerone)
				playertwo->drawWithGeom(shaderProgramDeferred, light->pos);
			else
				playerone->drawWithGeom(shaderProgramDeferred, light->pos);
		}
		for (Phantom* enemy : enemies) {
			enemy->drawWithGeom(shaderProgramDeferred, light->pos);
		}

		glUniform3fv(glGetUniformLocation(shaderProgramDeferred, "lightPos"), 1, &(light->pos[0])); //reset lightPos
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glUseProgram(shaderProgram);
	}

	//draw 2D stuffs on the screen, should be drawn at last
	//also the attacks should be after shooting
	allAttack->draw(shaderProgram);
	switch (gameStatus) {
	case 0: //playing
		cursor->draw(shaderProgram); 
		for (int i = 0; i < playerLife; i++) {
			hearts[i]->draw(shaderProgram);
		}
		if (playMode == 2) {
			for (int i = 0; i < currPlayer->bullet; i++) {
				bullets[i]->draw(shaderProgram);
			}
		}
		break;
	case 1: //game clear
		if (playMode == 2) //versus mode
			youwin->draw(shaderProgram);
		else
			gameclear->draw(shaderProgram);
		for (int i = 0; i < playerLife; i++) {
			hearts[i]->draw(shaderProgram);
		}
		if (playMode == 2) {
			for (int i = 0; i < currPlayer->bullet; i++) {
				bullets[i]->draw(shaderProgram);
			}
		}
		break;
	case 2: //game over
		if (playMode == 2) //versus mode
			youlose->draw(shaderProgram);
		else
			gameover->draw(shaderProgram);
		break;
	}

}

void Window::drawAmbientShadow() {
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);
	glUseProgram(shaderProgramAmbient);
	glUniform3fv(glGetUniformLocation(shaderProgramAmbient, "lightPos"), 1, &(light->pos[0]));
	cube->draw(shaderProgramAmbient);
	for (Cube* object : objects) {
		object->draw(shaderProgramAmbient);
	}

	for (Phantom* enemy : enemies) {
		if ((playMode == 0 && !enemy->dead) || (playMode > 0 && enemy->paralyzed)) //etnakaha etsu --> !enemy->dead
			enemy->draw(shaderProgramAmbient);
	}

	if (playMode > 0) {
		playerone->draw(shaderProgramAmbient);
		playertwo->draw(shaderProgramAmbient);
	}

	//glUniform3fv(glGetUniformLocation(shaderProgramAmbient, "lightPos"), 1, &(light->pos[0]));
	//enemy2->draw(shaderProgramAmbient);
	glDisable(GL_BLEND);
}

// draw shadow onto objects
void Window::drawObjectWithShadow() {
	glDrawBuffer(GL_FRONT_AND_BACK); //or GL_BACK
	// Draw only if the corresponding stencil value is zero
	glStencilFunc(GL_EQUAL, 0x0, 0xFF);
	// prevent update to the stencil buffer
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	// Use the shader of programID
	glUseProgram(shaderProgram);
	cube->draw(shaderProgram);
	for (Cube* object : objects) {
		object->draw(shaderProgram);
	}
	for (Phantom* enemy : enemies) {
		if (playMode == 0 || enemy->paralyzed) {
			enemy->draw(shaderProgram);
		}
	}
	if (playMode > 0) {
		playerone->draw(shaderProgram);
		playertwo->draw(shaderProgram);
	}
	//glUniform3fv(glGetUniformLocation(shaderProgram, "lightPos"), 1, &(light->pos[0]));
	//enemy2->draw(shaderProgram);
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
	
	//draw the ones with shadow volume
	for (Phantom* enemy : enemies) {
		// set light position
		vec3 lightPosInEnemyWorld = inverse(enemy->toWorld) * vec4(light->pos, 1.0);
		//glUniform3fv(glGetUniformLocation(shaderProgramDeferred, "lightPos"), 1, &(lightPosInEnemyWorld[0]));

		// render
		enemy->drawWithGeom(shaderProgramDeferred, light->pos);
	}
	
	if (playMode > 0) {
		playerone->drawWithGeom(shaderProgramDeferred, light->pos);
		playertwo->drawWithGeom(shaderProgramDeferred, light->pos);
	}

	for (Cube* object : objects) {
		vec3 lightPosInObjectWorld = inverse(object->toWorld) * vec4(light->pos, 1.0);
		//glUniform3fv(glGetUniformLocation(shaderProgramDeferred, "lightPos"), 1, &(lightPosInObjectWorld[0]));
		object->drawWithGeom(shaderProgramDeferred, light->pos);
	}

	//draw the others without shadow volume but can have shadows on them
	glUniform3fv(glGetUniformLocation(shaderProgramDeferred, "lightPos"), 1, &(light->pos[0])); //reset lightPos
	////enemy2->draw(shaderProgramDeferred);
	//cube->draw(shaderProgramDeferred);
	//object->draw(shaderProgramDeferred);

	// Restore local stuff
	glDisable(GL_DEPTH_CLAMP);
	glEnable(GL_CULL_FACE);
}

void Window::drawIntoDepth() {
	glDrawBuffer(GL_NONE);

	glUseProgram(shaderProgramNull);
	cube->draw(shaderProgramNull);

	for (Cube* object : objects) {
		object->draw(shaderProgramNull);
	}
	for (Phantom* enemy : enemies) {
		if ((playMode == 0 && !enemy->dead) || (playMode > 0 && enemy->paralyzed))
			enemy->draw(shaderProgramNull);
	}
	if (playMode > 0) {
		playerone->draw(shaderProgramNull);
		playertwo->draw(shaderProgramNull);
	}
}

void Window::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) 	{
		// Check if escape was pressed
		if (key == GLFW_KEY_ESCAPE) {
			glfwSetWindowShouldClose(window, GL_TRUE);
		}		
		else if (playMode == 0) { //single player
			if (gameStatus == 0) { //if playing
				if (key == GLFW_KEY_A) {
					//left
					moving.push_back(1);
				}
				else if (key == GLFW_KEY_D) {
					//right
					moving.push_back(2);
				}
				else if (key == GLFW_KEY_W) {
					//forward
					moving.push_back(3);
				}
				else if (key == GLFW_KEY_S) {
					//backward
					moving.push_back(4);
				}
				else if (key == GLFW_KEY_SPACE) {
					shooting = true;
				}
			}
		}
		else { //multi player
			if (playerone->gameStatus == 0) { 	//player one
				//moving
				if (key == GLFW_KEY_A) {
					//left
					playerone->moving.push_back(1);
				}
				else if (key == GLFW_KEY_D) {
					//right
					playerone->moving.push_back(2);
				}
				else if (key == GLFW_KEY_W) {
					//forward
					playerone->moving.push_back(3);
				}
				else if (key == GLFW_KEY_S) {
					//backward
					playerone->moving.push_back(4);
				}
				//turning
				else if (key == GLFW_KEY_X) {
					//left
					turning.push_back(1);
				}
				else if (key == GLFW_KEY_V) {
					//right
					turning.push_back(2);
				}
				else if (key == GLFW_KEY_F) {
					//forward
					turning.push_back(3);
				}
				else if (key == GLFW_KEY_C) {
					//backward
					turning.push_back(4);
				}
				else if (key == GLFW_KEY_SPACE) {
					playerone->shooting = true;
				}
			}
			if (playertwo->gameStatus == 0) {				//player two
				if (key == GLFW_KEY_J) {
					//left
					playertwo->moving.push_back(1);
				}
				else if (key == GLFW_KEY_L) {
					//right
					playertwo->moving.push_back(2);
				}
				else if (key == GLFW_KEY_I) {
					//forward
					playertwo->moving.push_back(3);
				}
				else if (key == GLFW_KEY_K) {
					//backward
					playertwo->moving.push_back(4);
				}
				else if (key == GLFW_KEY_M) {
					playertwo->shooting = true;
				}
				else if (key == GLFW_KEY_O) { //for debug
					playertwo->cam_look_at += vec3(0, 1.f, 0);
					playertwo->cam_pos += vec3(0, 1.f, 0);
				}
				else if (key == GLFW_KEY_P) { //for debug 
					playertwo->cam_look_at -= vec3(0, 1.f, 0);
					playertwo->cam_pos -= vec3(0, 1.f, 0);
				}
			}
		}

		if (key == GLFW_KEY_0) {
			showShadowVolume = !showShadowVolume;
		}
		else if (key == GLFW_KEY_3) {
			toggleMove = !toggleMove;
		}
		else if (key == GLFW_KEY_9) {
			bump = (bump + 1) % 2;
			glUniform1i(glGetUniformLocation(shaderProgram, "bump"), bump);
		}
		else if (key == GLFW_KEY_Q) { //for debug
			cam_look_at += vec3(0, 1.f, 0);
			cam_pos += vec3(0, 1.f, 0);
			Window::V = glm::lookAt(cam_pos, cam_look_at, cam_up);
		}
		else if (key == GLFW_KEY_E) { //for debug 
			cam_look_at -= vec3(0, 1.f, 0);
			cam_pos -= vec3(0, 1.f, 0);
			Window::V = glm::lookAt(cam_pos, cam_look_at, cam_up);
		}
		//light movements
		else if (key == GLFW_KEY_Y) { //for debug
			light->pos += vec3(0, 0, -5.f);
		}
		else if (key == GLFW_KEY_H) { //for debug
			light->pos += vec3(0, 0, 5.f);
		}
		else if (key == GLFW_KEY_U) { //for debug
			light->pos += vec3(5.f, 0, 0);
		}
		else if (key == GLFW_KEY_T) { //for debug
			light->pos += vec3(-5.f, 0, 0);
		}
		else if (key == GLFW_KEY_R) { //for debug
			light->pos += vec3(0, 5.f, 0);
		}
		else if (key == GLFW_KEY_G) { //for debug
			light->pos += vec3(0, -5.f, 0);
		}
	}
	else if (action == GLFW_RELEASE) {
		if (playMode == 0) { //single player
			if (key == GLFW_KEY_A) {
				vector<int>::iterator iter = std::find(moving.begin(), moving.end(), 1);
				if (iter != moving.end())
					moving.erase(iter);
			}
			else if (key == GLFW_KEY_D) {
				vector<int>::iterator iter = std::find(moving.begin(), moving.end(), 2);
				if (iter != moving.end())
					moving.erase(iter);
			}
			else if (key == GLFW_KEY_W) {
				vector<int>::iterator iter = std::find(moving.begin(), moving.end(), 3);
				if (iter != moving.end())
					moving.erase(iter);
			}
			else if (key == GLFW_KEY_S) {
				vector<int>::iterator iter = std::find(moving.begin(), moving.end(), 4);
				if (iter != moving.end())
					moving.erase(iter);
			}
		}
		else { //multi player
			//player one
			if (key == GLFW_KEY_A) {
				vector<int>::iterator iter = std::find(playerone->moving.begin(), playerone->moving.end(), 1);
				if (iter != playerone->moving.end())
					playerone->moving.erase(iter);
			}
			else if (key == GLFW_KEY_D) {
				vector<int>::iterator iter = std::find(playerone->moving.begin(), playerone->moving.end(), 2);
				if (iter != playerone->moving.end())
					playerone->moving.erase(iter);
			}
			else if (key == GLFW_KEY_W) {
				vector<int>::iterator iter = std::find(playerone->moving.begin(), playerone->moving.end(), 3);
				if (iter != playerone->moving.end())
					playerone->moving.erase(iter);
			}
			else if (key == GLFW_KEY_S) {
				vector<int>::iterator iter = std::find(playerone->moving.begin(), playerone->moving.end(), 4);
				if (iter != playerone->moving.end())
					playerone->moving.erase(iter);
			}
			else if (key == GLFW_KEY_X) {
				vector<int>::iterator iter = std::find(turning.begin(), turning.end(), 1);
				if (iter != turning.end())
					turning.erase(iter);
			}
			else if (key == GLFW_KEY_V) {
				vector<int>::iterator iter = std::find(turning.begin(), turning.end(), 2);
				if (iter != turning.end())
					turning.erase(iter);
			}
			else if (key == GLFW_KEY_F) {
				vector<int>::iterator iter = std::find(turning.begin(), turning.end(), 3);
				if (iter != turning.end())
					turning.erase(iter);
			}
			else if (key == GLFW_KEY_C) {
				vector<int>::iterator iter = std::find(turning.begin(), turning.end(), 4);
				if (iter != turning.end())
					turning.erase(iter);
			}
			// player two
			else if (key == GLFW_KEY_J) {
				vector<int>::iterator iter = std::find(playertwo->moving.begin(), playertwo->moving.end(), 1);
				if (iter != playertwo->moving.end())
					playertwo->moving.erase(iter);
			}
			else if (key == GLFW_KEY_L) {
				vector<int>::iterator iter = std::find(playertwo->moving.begin(), playertwo->moving.end(), 2);
				if (iter != playertwo->moving.end())
					playertwo->moving.erase(iter);
			}
			else if (key == GLFW_KEY_I) {
				vector<int>::iterator iter = std::find(playertwo->moving.begin(), playertwo->moving.end(), 3);
				if (iter != playertwo->moving.end())
					playertwo->moving.erase(iter);
			}
			else if (key == GLFW_KEY_K) {
				vector<int>::iterator iter = std::find(playertwo->moving.begin(), playertwo->moving.end(), 4);
				if (iter != playertwo->moving.end())
					playertwo->moving.erase(iter);
			}
		}
	}
}

void Window::playSound(int type, int enemyIndex = 0) {
	switch (type) {
	case 0: //attack
		vec4 attackPosition = enemies[enemyIndex]->toWorld * vec4(0, 0, 0, 1);
		//engine->setListenerPosition(vec3df(cam_pos.x, cam_pos.y, cam_pos.z), vec3df(cam_look_at.x, cam_look_at.y, cam_look_at.z), vec3df(0, 0, 0), vec3df(cam_up.x, cam_up.y, cam_up.z));
		engine->play3D(ssAttack, vec3df(attackPosition.x, attackPosition.y, attackPosition.z));
		break;
	}
}

void Window::cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (playMode == 0) {
		if (xpos < 0 || xpos >= width || ypos < 0 || ypos >= height)
			return;
	}
	else {
		if (xpos < width || xpos >= 2*width || ypos < 0 || ypos >= height)
			return;
		cam_pos = playertwo->cam_pos;
		cam_look_at = playertwo->cam_look_at;
		V = glm::lookAt(cam_pos, cam_look_at, cam_up);
		xpos -= width;
	}

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
			if (playMode > 0) {
				playertwo->cam_look_at = cam_look_at;
			}
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

void Window::zoomIn() {
	//get
	printf("ZOOMING IN");
	glm::vec3 vector = cam_look_at - cam_pos;

	cam_pos = cam_pos + normalize(vector);
	//cam_look_at = cam_pos + normalize(vector);
	cam_look_at = cam_look_at + normalize(vector);
	V = glm::lookAt(cam_pos, cam_look_at, cam_up);
}

void Window::zoomOut() {
	glm::vec3 vector = cam_look_at - cam_pos;

	cam_pos = cam_pos - normalize(vector);
	cam_look_at = cam_look_at - normalize(vector);
	V = glm::lookAt(cam_pos, cam_look_at, cam_up);
}


