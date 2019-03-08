#define _CRT_SECURE_NO_WARNINGS
#include "Phantom.h"
#include "Window.h"

Phantom::Phantom(int type, vec3 initialPos, float anglex, float angley, float scale) {
	movingType = type;
	paralyzed = false;
	dead = false; 
	currentDist = 0;
	prevCenter = vec3(0);
	prevTurnPlace = initialPos;

	scaMat = glm::scale(mat4(1.0f), vec3(scale));
	traMat = translate(glm::mat4(1.0f), initialPos);
	//for debug:
	rotMat =  rotate(mat4(1.0f), anglex / 180.0f * pi<float>(), vec3(0.0f, 1.0f, 0.0f)) * rotate(mat4(1.0f), angley / 180.0f * pi<float>(), vec3(-1.0f, 0.0f, 0.0f));
	//rotMat = mat4(1.0f);
	updateToWorld(this);

	upperhalf = new OBJObject("head_s.obj", "white.ppm", mat4(1.f));
	lowerhalf = new OBJObject("head_s.obj", "white.ppm", mat4(1.f)); 

	radius = upperhalf->maxY - upperhalf->minY;
	upperhalf->toWorld = rotate(mat4(1.0f), 40.f / 180.0f * pi<float>(), vec3(-1.0f, 0.0f, 0.0f)) * translate(mat4(1.f), vec3(0, radius / 2.f, 0));
	lowerhalf->toWorld = rotate(mat4(1.0f), 140.f / 180.0f * pi<float>(), vec3(-1.0f, 0.0f, 0.0f)) * translate(mat4(1.f), vec3(0, radius / 2.f, 0));
	radius *= scale;
	
	upperhalfWorld = upperhalf->toWorld;
	lowerhalfWorld = lowerhalf->toWorld;

	currentAngle = 40.f;
	deltaAngle = 0.25f;

	explosion = nullptr;
	//set up for explosion
	vector<vec3> explosionVert;
	int startIndexOfLower;
	mat4 rotate0 = translate(mat4(1.f), vec3(0, radius / 2.f, 0));
	for (int i = 0; i < upperhalf->vertices.size(); i += 3) {
		if (upperhalf->vertices[i].y > upperhalf->minY) {
			explosionVert.push_back(upperhalf->vertices[i]);
			explosionVert.push_back(upperhalf->vertices[i + 1]);
			explosionVert.push_back(upperhalf->vertices[i + 2]);
		}
	}
	startIndexOfLower = explosionVert.size() / 3;
	mat4 rotate180 = rotate(mat4(1.0f), pi<float>(), vec3(-1.0f, 0.0f, 0.0f)) * translate(mat4(1.f), vec3(0, radius / 2.f, 0));
	for (int i = 0; i < lowerhalf->vertices.size(); i += 3) {
		if (upperhalf->vertices[i].y > upperhalf->minY) {
			explosionVert.push_back(lowerhalf->vertices[i]);
			explosionVert.push_back(lowerhalf->vertices[i + 1]);
			explosionVert.push_back(lowerhalf->vertices[i + 2]);
		}
	}
	explosion = new ParticleExplosion(vec3(0, 0, 0), explosionVert, startIndexOfLower);

	paralyzingSound = nullptr;
}

Phantom::~Phantom() {
	// Delete previously generated buffers. Note that forgetting to do this can waste GPU memory in a 
	// large project! This could crash the graphics driver due to memory leaks, or slow down application performance!
	delete(upperhalf);
	delete(lowerhalf);

	if (paralyzingSound) {
		paralyzingSound->stop();
		paralyzingSound->drop();
	}
}

void Phantom::draw(GLuint shaderProgram) {
	if (dead && explosion != nullptr) {
		explosion->draw(shaderProgram);
		return;
	}

	upperhalf->toWorld = toWorld * upperhalfWorld;
	lowerhalf->toWorld = toWorld * lowerhalfWorld;

	glUniform1i(glGetUniformLocation(shaderProgram, "objectType"), 0);

	upperhalf->draw(shaderProgram);
	lowerhalf->draw(shaderProgram);
}

void Phantom::drawWithGeom(GLuint shaderProgram, vec3 lightPos) {
	if (dead) {
		return;
	}

	upperhalf->toWorld = toWorld * upperhalfWorld;
	lowerhalf->toWorld = toWorld * lowerhalfWorld;

	glUniform1i(glGetUniformLocation(shaderProgram, "objectType"), 0);

	upperhalf->drawWithGeom(shaderProgram, lightPos);
	lowerhalf->drawWithGeom(shaderProgram, lightPos);
}

void Phantom::animate() {
	if (dead) {
		return;
	}
	else if (paralyzed) {
		if ((currentAngle >= 60.0f && deltaAngle > 0)
			|| (currentAngle <= 50.0f && deltaAngle < 0)) {
			deltaAngle *= -1.0f;
		}
		upperhalfWorld = rotate(mat4(1.0f), 3.0f * deltaAngle / 180.0f * pi<float>(), vec3(-1.0f, 0.0f, 0.0f)) * upperhalfWorld;
		lowerhalfWorld = rotate(mat4(1.0f), -3.0f * deltaAngle / 180.0f * pi<float>(), vec3(-1.0f, 0.0f, 0.0f)) * lowerhalfWorld;
		currentAngle += 3.0f * deltaAngle;
	}
	else {
		if ((currentAngle >= 40.0f && deltaAngle > 0) 
			|| (currentAngle <= 0.0f && deltaAngle < 0)) {
			deltaAngle *= -1.0f;
		}
		upperhalfWorld = rotate(mat4(1.0f), deltaAngle / 180.0f * pi<float>(), vec3(-1.0f, 0.0f, 0.0f)) * upperhalfWorld;
		lowerhalfWorld = rotate(mat4(1.0f), -deltaAngle / 180.0f * pi<float>(), vec3(-1.0f, 0.0f, 0.0f)) * lowerhalfWorld ;
		currentAngle += deltaAngle;
	}
	
}

void Phantom::move(vec3 camPos) {
	if (paralyzed || dead) {
		return;
	}

	switch (movingType) {
	//trace player type
	case 0: //chasing
	case 2: {//stationary
		//direction in phantom's world
		vec3 currentDir = vec4(0, 0, 1.0f, 0);
		vec3 newDir = normalize(inverse(toWorld) * vec4(camPos, 1) - vec4(0, 0, 0, 1));

		//separate the rotation into two steps: rotation in xz-plane and that in yz-plane
		vec3 currentDirXOZ = normalize(vec3(currentDir.x, 0, currentDir.z));
		vec3 newDirXOZ = normalize(vec3(newDir.x, 0, newDir.z));
		float anglex = acos(dot(currentDirXOZ, newDirXOZ));

		vec3 currentDirOYZ = normalize(vec3(0, currentDir.y, currentDir.z));
		vec3 newDirOYZ = normalize(vec3(0, newDir.y, newDir.z));
		float angley = acos(dot(currentDirOYZ, newDirOYZ));

		//only allow certain degrees of change in angle 
		if (anglex > 0.2f / 180.0f * pi<float>()) {
			anglex = 0.2f / 180.0f * pi<float>();
		}
		if (angley > 0.2f / 180.0f * pi<float>()) {
			angley = 0.2f / 180.0f * pi<float>();
		}

		//negate the angle if necessary since cosine law doesn't take order of vectors into account
		if (cross(currentDirXOZ, newDirXOZ).y < 0) {
			anglex = -anglex;
		}
		if (cross(currentDirOYZ, newDirOYZ).x < 0) {
			angley = -angley;
		}

		//the if statements are to avoid errors in rotate method 
		//rotMat rotates in y first then x
		if (abs(anglex) > 0.00001f) {
			rotMat = rotate(mat4(1.0f), anglex, vec3(0, 1.0f, 0)) * rotMat;
		}
		if (abs(angley) > 0.00001f) {
			rotMat = rotMat * rotate(mat4(1.0f), angley, vec3(1.0f, 0, 0));
		}

		if (movingType == 2) {
			updateToWorld(this);
			return;
		}
	}
		break;
	case 1:
		break;
	case 4:
		return;
		break;
	case 3: //random perpendicular
		if (currentDist >= 20 || length(vec3(toWorld* vec4(0, 0, 0, 1)) - prevTurnPlace) <= speed/2) {
			currentDist = 0;
			vec3 currDir = normalize(toWorld * vec4(0, 0, 1.0f, 0)); //for case 2 and 1 when the direction is not perpendicular
			switch ((int)(rand() % 5)) {
			case 0: //proceed
				break;
			case 1: //turn left
			case 2: //turn left
				if (currDir.x == 0 || currDir.z == 0)
					rotMat = rotate(mat4(1.0f), 90.0f / 180.0f * pi<float>(), vec3(0, 1.0f, 0)) * rotMat;
				else {
					rotMat = rotate(mat4(1.0f),
						(currDir.z >= 0) ? atan((currDir.x) / abs(currDir.x)) : pi<float>() - atan((currDir.x) / abs(currDir.x))
							, vec3(0.0f, 1.0f, 0.0f));
					rotMat = rotate(mat4(1.0f), 45.0f / 180.0f * pi<float>(), vec3(0, 1.0f, 0)) * rotMat;
				}
				break;
			case 3: //turn right 
			case 4: //turn right 
				if (currDir.x == 0 || currDir.z == 0)
					rotMat = rotate(mat4(1.0f), -90.0f / 180.0f * pi<float>(), vec3(0, 1.0f, 0)) * rotMat;
				else {
					rotMat = rotate(mat4(1.0f),
						(currDir.z >= 0) ? atan((currDir.x) / abs(currDir.x)) : pi<float>() - atan((currDir.x) / abs(currDir.x))
						, vec3(0.0f, 1.0f, 0.0f));
					rotMat = rotate(mat4(1.0f), -45.0f / 180.0f * pi<float>(), vec3(0, 1.0f, 0)) * rotMat;
				}
				break;
			}
			printf("\n@@@TURN@@@\n");
			prevTurnPlace = toWorld * vec4(0, 0, 0, 1);
		}
		currentDist += speed;
		break;
	}

	prevCenter = toWorld * vec4(0, 0, 0, 1); //for block collision which happens when rotating while incidence

	updateToWorld(this);
	//debug speed: 0.1;		normal speed: 0.01
	vec3 translation = toWorld * vec4(0, 0, speed, 0);
	traMat = translate(traMat, translation);

	updateToWorld(this);
}

//collision within a cube
void Phantom::wallCollisionCheck(Cube* wall) {
	if (movingType == 2)
		return;

	vec3 minVertex = wall->minVertex;
	vec3 maxVertex = wall->maxVertex;
	vec3 centerInWorld = toWorld * vec4(0, 0, 0, 1);
	vec3 currentDirInWorld = normalize(toWorld * vec4(0, 0, 1.0f, 0));

	if ((currentDirInWorld.x > 0 && centerInWorld.x + radius > maxVertex.x)
		|| (currentDirInWorld.x < 0 && centerInWorld.x - radius < minVertex.x)) {
		float anglexz = (currentDirInWorld.z != 0) ?
			atan(currentDirInWorld.x / currentDirInWorld.z) : pi<float>() / 2.0f;
		anglexz *= -2.0f;

		/*
		if (currentDirInWorld.z < 0) {
			anglexz += pi<float>();
		}
		*/

		if (abs(anglexz) > 0.00001f) {
			rotMat = rotate(mat4(1.0f), anglexz, vec3(0, 1.0f, 0)) * rotMat;
		}
	}
	else if ((currentDirInWorld.z > 0 && centerInWorld.z + radius > maxVertex.z) 
		|| (currentDirInWorld.z < 0 && centerInWorld.z - radius < minVertex.z)) {
		float anglezx = (currentDirInWorld.x != 0) ?
			atan(currentDirInWorld.z / currentDirInWorld.x) : pi<float>() / 2.0f;
		anglezx *= 2.0f;
		/*
		if (currentDirInWorld.x < 0) {
		anglezx += pi<float>();
		}
		*/

		if (abs(anglezx) > 0.00001f) {
			rotMat = rotate(mat4(1.0f), anglezx, vec3(0, 1.0f, 0)) * rotMat;
		}
	}
	else if ((currentDirInWorld.y > 0 && centerInWorld.y + radius > maxVertex.y) 
		|| (currentDirInWorld.y < 0 && centerInWorld.y - radius < minVertex.y)) {
		float angley = 2.0f * asin(currentDirInWorld.y);
		if (abs(angley) > 0.00001f) {
			rotMat = rotMat * rotate(mat4(1.0f), angley, vec3(1.0f, 0, 0));
		}
	}
	else {
		return;
	}

	updateToWorld(this);
	vec3 translation = toWorld * vec4(0, 0, 2.0f * speed, 0);
	//printf("%f %f %f \n\n", translation.x, translation.y, translation.z);
	traMat = translate(traMat, translation);
	updateToWorld(this);

	currentDist += radius * 2;
}

//collision outside a cube
void Phantom::blockCollisionCheck(Cube* block) {
	if (movingType == 2)
		return;

	vec3 minVertex = block->minVertex;
	vec3 maxVertex = block->maxVertex;
	vec3 centerInWorld = toWorld * vec4(0, 0, 0, 1);
	vec3 currentDirInWorld = normalize(toWorld * vec4(0, 0, 1.0f, 0));
	
	if (centerInWorld.x - radius <= maxVertex.x && centerInWorld.x + radius >= minVertex.x
		&& centerInWorld.y - radius <= maxVertex.y && centerInWorld.y + radius >= minVertex.y
		&& centerInWorld.z - radius <= maxVertex.z && centerInWorld.z + radius >= minVertex.z) {
		vec3 prevCenterInWorld = prevCenter;//centerInWorld - speed*currentDirInWorld;  //etnakaha etsu added speed*
		float xlength = maxVertex.x - minVertex.x;
		float ylength = maxVertex.y - minVertex.y;
		float zlength = maxVertex.z - minVertex.z;
		
		if (!(prevCenterInWorld.x - radius <= maxVertex.x && prevCenterInWorld.x + radius >= minVertex.x
			&& prevCenterInWorld.y - radius <= maxVertex.y && prevCenterInWorld.y + radius >= minVertex.y
			&& prevCenterInWorld.z - radius <= maxVertex.z && prevCenterInWorld.z + radius >= minVertex.z)) {
			for (int i = 0; i < 36; i+=3) {
				vec3 first = vec3(block->vertices[i][0], block->vertices[i][1], block->vertices[i][2]);
				vec3 second = vec3(block->vertices[i+1][0], block->vertices[i+1][1], block->vertices[i+1][2]);
				vec3 third = vec3(block->vertices[i+2][0], block->vertices[i+2][1], block->vertices[i+2][2]);
			
				vec3 norm = normalize(vec3(block->normals[i][0], block->normals[i][1], block->normals[i][2])); //etnakaha etsu added normalize
				vec3 origin = centerInWorld - radius * norm; //change to positive if no change in normals by isWall

				float distance;
				vec2 bary;
				bool doesIntersect = glm::intersectRayTriangle(origin, -currentDirInWorld, first, second, third, bary, distance);

				if (doesIntersect && distance >= 0) { //negative distance means ray doesn't but line does intersect, infinitely negative distance means doesn't intersect 
					//reflect, or rotate
					vec3 newDirInWorld = normalize(reflect(currentDirInWorld, norm));
					
					//for debug
					/*
					printf("hello\n%f %f %f \n", -currentDirInWorld.x, -currentDirInWorld.y, -currentDirInWorld.z);
					printf("%f %f %f \n", origin.x, origin.y, origin.z);
					printf("%f \n", radius);
					printf("distance: %f \n\n", distance);
					printf("%f %f %f \n", newDirInWorld.x, newDirInWorld.y, newDirInWorld.z);
					printf("%d \n\n", i);
					*/

					rotMat = rotate(mat4(1.0f),
						(newDirInWorld.z >= 0) ? atan(newDirInWorld.x / newDirInWorld.z) : pi<float>() + atan(newDirInWorld.x / newDirInWorld.z)
						, vec3(0.0f, 1.0f, 0.0f)) * rotate(mat4(1.0f), -asin(newDirInWorld.y), vec3(1.0f, 0.0f, 0.0f));


					//translate
					prevCenter = toWorld * vec4(0, 0, 0, 1); //for block collision which happens when rotating while incidence

					updateToWorld(this);
					vec3 translation = toWorld * vec4(0, 0, 2.0f * speed, 0);
					//printf("%f %f %f \n\n", translation.x, translation.y, translation.z);
					traMat = translate(traMat, translation);
					updateToWorld(this);

					currentDist += radius * 2;

					printf("\n\n\nnormal reflect %f:\n", radius);
					return;
				}
			}

			//#############################################################################################
			//case when incidence onto edge/corner instead of face of the block

			int maxx = (prevCenterInWorld.x > maxVertex.x) * 2 - 1; // 1 or -1; maxx + minx = 0 means between maxx and minx
			int minx = (prevCenterInWorld.x > minVertex.x) * 2 - 1;
			int maxy = (prevCenterInWorld.y > maxVertex.y) * 2 - 1;
			int miny = (prevCenterInWorld.y > minVertex.y) * 2 - 1;
			int maxz = (prevCenterInWorld.z > maxVertex.z) * 2 - 1;
			int minz = (prevCenterInWorld.z > minVertex.z) * 2 - 1;

			int areaIndicator[3]; // 0:x  1:y  2:z
			areaIndicator[0] = (maxx + minx) / 2;
			areaIndicator[1] = (maxy + miny) / 2;
			areaIndicator[2] = (maxz + minz) / 2;

			vec3 minmaxVertex[2]; //0:min  1:max
			minmaxVertex[0] = minVertex;
			minmaxVertex[1] = maxVertex;
			if (abs(areaIndicator[0]) + abs(areaIndicator[1]) + abs(areaIndicator[2]) == 3) { //vertex corner
				vec3 corner = vec3(minmaxVertex[areaIndicator[0] > 0].x , minmaxVertex[areaIndicator[1] > 0].y , minmaxVertex[areaIndicator[2] > 0].z);
				vec3 norm = normalize(centerInWorld - corner);
				vec3 normOfCorner = vec3(minmaxVertex[areaIndicator[0] > 0].x - minmaxVertex[areaIndicator[0] <= 0].x, 
					minmaxVertex[areaIndicator[1] > 0].y - minmaxVertex[areaIndicator[1] <= 0].y, 
					minmaxVertex[areaIndicator[2] > 0].z - minmaxVertex[areaIndicator[2] <= 0].z);//for check
				if (acos(dot(normalize(normOfCorner), normalize(-currentDirInWorld))) > pi<float>() / 2) {
					printf("\n\n\nquityoo %f:\n", radius);
					printf("prev: %f %f %f \n", prevCenterInWorld.x, prevCenterInWorld.y, prevCenterInWorld.z);
					printf("now: %f %f %f \n", centerInWorld.x, centerInWorld.y, centerInWorld.z);
					return;
				}

				vec3 newDirInWorld = normalize(reflect(currentDirInWorld, norm));
				rotMat = rotate(mat4(1.0f),
					(newDirInWorld.z >= 0) ? atan(newDirInWorld.x / newDirInWorld.z) : pi<float>() + atan(newDirInWorld.x / newDirInWorld.z)
					, vec3(0.0f, 1.0f, 0.0f)) * rotate(mat4(1.0f), -asin(newDirInWorld.y), vec3(1.0f, 0.0f, 0.0f));
				//translate
				prevCenter = toWorld * vec4(0, 0, 0, 1); //for block collision which happens when rotating while incidence

				updateToWorld(this);
				vec3 translation = toWorld * vec4(0, 0, 2.0f * speed, 0);
				traMat = translate(traMat, translation);
				updateToWorld(this);
				printf("\n\n\nyoo %f: ", radius);
			}
			else if (abs(areaIndicator[0]) + abs(areaIndicator[1]) + abs(areaIndicator[2]) == 2) { //edge corner
				vec3 pointOnEdge = vec3(minmaxVertex[areaIndicator[0] > 0].x, minmaxVertex[areaIndicator[1] > 0].y, minmaxVertex[areaIndicator[2] > 0].z); //norm's origin on the edge
				for (int i = 0; i < 3; i++) {
					if (areaIndicator[i] == 0) {
						pointOnEdge[i] = centerInWorld[i]; //this is done in order to find the perpendicular norm on the edge
						break;
					}
				}
				vec3 norm = normalize(centerInWorld - pointOnEdge);


				vec3 normOfCorner = vec3(minmaxVertex[areaIndicator[0] >= 0].x - minmaxVertex[areaIndicator[0] <= 0].x,
					minmaxVertex[areaIndicator[1] >= 0].y - minmaxVertex[areaIndicator[1] <= 0].y,
					minmaxVertex[areaIndicator[2] >= 0].z - minmaxVertex[areaIndicator[2] <= 0].z);//for check
				if (acos(dot(normalize(normOfCorner), normalize(-currentDirInWorld))) > pi<float>() / 2) {
					printf("\n\n\nquityaa %f:\n", radius);
					printf("prev: %f %f %f \n", prevCenterInWorld.x, prevCenterInWorld.y, prevCenterInWorld.z);
					printf("now: %f %f %f \n", centerInWorld.x, centerInWorld.y, centerInWorld.z);
					printf("corner normal: %f %f %f \n", normOfCorner.x, normOfCorner.y, normOfCorner.z);
					printf("angle: %f\n", acos(dot(normalize(normOfCorner), normalize(-currentDirInWorld))));
					return;
				}

				vec3 newDirInWorld = normalize(reflect(currentDirInWorld, norm));
				rotMat = rotate(mat4(1.0f),
					(newDirInWorld.z >= 0) ? atan(newDirInWorld.x / newDirInWorld.z) : pi<float>() + atan(newDirInWorld.x / newDirInWorld.z)
					, vec3(0.0f, 1.0f, 0.0f)) * rotate(mat4(1.0f), -asin(newDirInWorld.y), vec3(1.0f, 0.0f, 0.0f));

				//translate
				prevCenter = toWorld * vec4(0, 0, 0, 1); //for block collision which happens when rotating while incidence

				updateToWorld(this);
				vec3 translation = toWorld * vec4(0, 0, 2.0f * speed, 0);
				traMat = translate(traMat, translation);
				updateToWorld(this);

				printf("\n\n\nyaa %f: ", radius);
			}
			else {
				printf("\n\n\nBUG\nBUG\nBUG");
			}
			/*
			for (int i = 0; i < 4; i++) {
				for (int j = 24; j < 36; j += 6) {
					vec3 first = vec3(block->vertices[j + i][0], block->vertices[j + i][1], block->vertices[j + i][2]);
					vec3 second = vec3(block->vertices[j + i + 2][0], block->vertices[j + i + 2][1], block->vertices[j + i + 2][2]);
				}
			}*/
			//printf("\n\n\nyaa %f: ", radius);
			printf("%f %f %f \n", prevCenterInWorld.x, prevCenterInWorld.y, prevCenterInWorld.z);
			printf("%f %f %f \n", centerInWorld.x, centerInWorld.y, centerInWorld.z);
			//printf("curdir: %f %f %f \n", currentDirInWorld.x, currentDirInWorld.y, currentDirInWorld.z);
			return;
		}
		printf("\n   tehe %f:\n", radius);
		/*printf("prev: %f %f %f \n", prevCenterInWorld.x, prevCenterInWorld.y, prevCenterInWorld.z);
		printf("now: %f %f %f \n", centerInWorld.x, centerInWorld.y, centerInWorld.z);
		printf("max: %f %f %f \n", maxVertex.x, maxVertex.y, maxVertex.z);
		printf("min: %f %f %f \n", minVertex.x, minVertex.y, minVertex.z);*/
	}
}

void Phantom::phantomCollisionCheck(vector<Phantom*> others) {
	if (movingType >= 2)
		return;

	vec3 thisCenter = toWorld * vec4(0, 0, 0, 1.0f);
	for (Phantom* other : others) {
		if (other == this) {
			continue;
		}
		vec3 otherCenter = other->toWorld * vec4(0, 0, 0, 1.0f);
		if (abs(distance(thisCenter, otherCenter)) < radius + other->radius) {

			vec3 thisCurrentDir = normalize(toWorld * vec4(0, 0, 1.0f, 0));
			vec3 norm = normalize(thisCenter - otherCenter);

			//reflect this
			vec3 thisNewDir = normalize(reflect(thisCurrentDir, norm));

			printf("%f %f %f \n\n", thisCurrentDir.x, thisCurrentDir.y, thisCurrentDir.z);
			printf("%f %f %f \n\n", thisNewDir.x, thisNewDir.y, thisNewDir.z);

			rotMat = rotate(mat4(1.0f), 
				(thisNewDir.z >= 0) ? atan(thisNewDir.x/thisNewDir.z) : pi<float>() + atan(thisNewDir.x / thisNewDir.z)
				, vec3(0.0f, 1.0f, 0.0f)) * rotate(mat4(1.0f), -asin(thisNewDir.y), vec3(1.0f, 0.0f, 0.0f));

			updateToWorld(this);
			vec3 thisTranslation = toWorld * vec4(0, 0, 2*speed, 0);
			traMat = translate(traMat, thisTranslation);
			updateToWorld(this);

			//reflect other
			if (other->movingType >= 2)
				continue;
			vec3 otherCurrentDir = normalize(other->toWorld * vec4(0, 0, 1.0f, 0));
			vec3 otherNewDir = normalize(reflect(otherCurrentDir, -norm));
			other->rotMat = rotate(mat4(1.0f),
				(otherNewDir.z >= 0) ? atan(otherNewDir.x / otherNewDir.z) : pi<float>() + atan(otherNewDir.x / otherNewDir.z)
				, vec3(0.0f, 1.0f, 0.0f)) * rotate(mat4(1.0f), -asin(otherNewDir.y), vec3(1.0f, 0.0f, 0.0f));

			updateToWorld(other);
			vec3 otherTranslation = other->toWorld * vec4(0, 0, 2*speed, 0);
			other->traMat = translate(other->traMat, otherTranslation);
			updateToWorld(other);
		}
	}
}

void Phantom::updateToWorld(Phantom* phantom) {
	phantom->toWorld = phantom->traMat * phantom->rotMat * phantom->scaMat;
}

void Phantom::paralyze() {
	paralyzed = true;
	paralyzedTime = clock();
}

// return true when fully dead and hence should be deleted
bool Phantom::die() {
	if (!dead) {
		paralyzed = false;
		paralyzingSound->stop();
		paralyzingSound->drop();
		paralyzingSound = 0;

		dead = true;
		diedTime = clock();
		explosion->updateParticleToWorld(toWorld, upperhalfWorld, lowerhalfWorld);
	}
	else {
		/* shrinking animation
		float factor = scaMat[0][0] - 0.05f;
		if (factor <= 0) {
			return true;
		}
		scaMat = scale(mat4(1.0f), vec3(factor));
		updateToWorld(this);
		*/
		if (explosion->update()) {
			delete(explosion);
			return true;
		}
	}
	return false;
}

//printf("%f %f %f \n\n", translation.x, translation.y, translation.z);

/*
		printf("%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n\n",
			&toWorld[0][0], &toWorld[0][1], &toWorld[0][2], &toWorld[0][3],
			&toWorld[1][0], &toWorld[1][1], &toWorld[1][2], &toWorld[1][3],
			&toWorld[2][0], &toWorld[2][1], &toWorld[2][2], &toWorld[2][3],
			&toWorld[3][0], &toWorld[3][1], &toWorld[3][2], &toWorld[3][3]);
*/