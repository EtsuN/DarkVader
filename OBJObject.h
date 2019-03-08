#ifndef OBJOBJECT_H
#define OBJOBJECT_H

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
#include <map>
#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>   // include math functions, such as sin, cos, M_PI
#include <iostream> // allow c++ style console printouts

using namespace glm;
using namespace std;

// Stores an edge by its vertices and force an order between them
struct Edge {
	uint a; //these are vertex labels
	uint b;
	Edge(uint _a, uint _b){
		assert(_a != _b);
		if (_a < _b) {
			a = _a;
			b = _b;
		}
		else {
			a = _b;
			b = _a;
		}
	}
	void Print()	{
		printf("Edge %d %d\n", a, b);
	}
	/*bool operator==(const Edge & other) const {
		return (this->a == other.a && this->b == other.b) || (this->a == other.b && this->b == other.a);
	}*/
};

struct Neighbors {
	uint n1; //these are indices
	uint n2;
	Neighbors()	{
		n1 = n2 = (uint)-1;
	}
	bool AddNeigbor(uint n)	{
		if (n1 == -1) {
			n1 = n;
		}
		else if (n2 == -1) {
			n2 = n;
		}
		else {
			printf("%d %d --- %d: there are two identical faces\n", n1, n2, n);
			return false;
		}
		return true;
	}
	uint GetOther(uint me) const 	{
		if (n1 == me) {
			return n2;
		}
		else if (n2 == me) {
			return n1;
		}
		else {
			printf("%d %d and me is %d, which is duplicated triangle vertex\n", n1, n2, me);
			return -1;
		}
	}
};

struct CompareEdges
{
	bool operator()(const Edge& Edge1, const Edge& Edge2) const	{
		if (Edge1.a < Edge2.a) {
			return true;
		}
		else if (Edge1.a == Edge2.a) {
			return (Edge1.b < Edge2.b);
		}
		else {
			return false;
		}
	}
};

struct CompareVectors
{
	bool operator()(const vec3 a, const vec3 b) const	{
		if (a.x < b.x) {
			return true;
		}
		else if (a.x == b.x) {
			if (a.y < b.y) {
				return true;
			}
			else if (a.y == b.y) {
				if (a.z < b.z) {
					return true;
				}
			}
		}
		return false;
	}
};

class OBJObject
{

public:
	std::vector<unsigned int> vertexIndices;
	std::vector<unsigned int> uvIndices;
	std::vector<unsigned int> normalIndices;
	std::vector<unsigned int> indices;
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> textures;
	std::vector<glm::vec3> normals;
	std::vector<Edge> edges;
	std::map<Edge, Neighbors, CompareEdges> mapNeighbors;
	std::map<vec3, int, CompareVectors> vertexLabels; //to give vertex unique labels
	std::vector<unsigned int> indicesWithAdj; //double size of indices

	OBJObject(const char*, const char*, mat4 M);
	~OBJObject();
	glm::mat4 toWorld;
	void parse(const char* filepath);
	//void reorder();
	void draw(GLuint);
	void position(vec3 position);
	void drawWithGeom(GLuint, vec3);

	float angle;
	float maxX, minX, maxY, minY, maxZ, minZ;
	float centerX, centerY, centerZ;
	//string ppmFile;
	bool isBound;
	bool isCurve;
	void changeScale(float);
    
    

	GLuint VBO0, VBO1, VBO2, VAO, EBO;
	//unsigned int textureID;
	GLuint texture[1];     // storage for one texture
	GLuint uProjection, uModelview, uSampler;

	void findAdjacencies();
	void centerToOrigin();
	void reorder();
	unsigned char* loadPPM(const char* filename, int& width, int& height);
	void loadTexture(const char*);
};

#endif
