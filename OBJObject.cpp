#define _CRT_SECURE_NO_WARNINGS
#include "OBJObject.h"
#include "Window.h"

OBJObject::OBJObject(const char *filepath, const char *texture_file, mat4 M)
{
	isBound = false;
	isCurve = false;
	//TODO change to mat4(1) if everything is done
	toWorld = M; 
	parse(filepath);
	centerToOrigin();

	//do all transformation above this line
	int label = 0;
	for (vec3 v : vertices) {
		if (vertexLabels.find(v) == vertexLabels.end()) {
			vertexLabels[v] = label;
			label += 1;
		}
	}
	reorder();
	findAdjacencies();

	//if (indices.size() == vertices.size())
	//	printf("1\n");
	//printf("%d %d %d\n", vertices.size(), indices.size(), indicesWithAdj.size());

	// Create array object and buffers. Remember to delete your buffers when the object is destroyed!
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO0);
	glGenBuffers(1, &VBO1);
	glGenBuffers(1, &VBO2);
	glGenBuffers(1, &EBO);

	// Bind the Vertex Array Object (VAO) first, then bind the associated buffers to it.
	// Consider the VAO as a container for all your buffers.
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO0);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec3), &vertices[0], GL_STATIC_DRAW);
	// Enable the usage of layout location 0 (check the vertex shader to see what this is)
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);

	glBindBuffer(GL_ARRAY_BUFFER, VBO1);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(vec3), &normals[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);

	glBindBuffer(GL_ARRAY_BUFFER, VBO2);
	glBufferData(GL_ARRAY_BUFFER, textures.size() * sizeof(vec2), &textures[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);

	//use only vertex size if without geometry shader, otherwise use everything after vertex.size
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesWithAdj.size() * sizeof(unsigned int), &indicesWithAdj[0], GL_STATIC_DRAW);

	loadTexture(texture_file);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Unbind the currently bound buffer so that we don't accidentally make unwanted changes to it.
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	// Unbind the VAO now so we don't accidentally tamper with it.
	// NOTE: You must NEVER unbind the element array buffer associated with a VAO!
	glBindVertexArray(0);
}

void OBJObject::centerToOrigin()
{
	centerX = maxX / 2 + minX / 2;
	centerY = maxY / 2 + minY / 2;
	centerZ = maxZ / 2 + minZ / 2;
	for (glm::vec3 &vertice : vertices) {
		vertice.x -= centerX;
		vertice.y -= centerY;
		vertice.z -= centerZ;
	}
	maxX -= centerX;
	minX -= centerX;
	maxY -= centerY;
	minY -= centerY;
	maxZ -= centerZ;
	minZ -= centerZ;
}

OBJObject::~OBJObject()
{
	// Delete previously generated buffers. Note that forgetting to do this can waste GPU memory in a 
	// large project! This could crash the graphics driver due to memory leaks, or slow down application performance!
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO0);
	glDeleteBuffers(1, &VBO1);
	glDeleteBuffers(1, &VBO2);
	glDeleteBuffers(1, &EBO);
}

void OBJObject::parse(const char *filepath)
{
	FILE* file = fopen(filepath, "r");
	char str[80];
	float num;
	maxX = std::numeric_limits<float>::min();
	maxY = std::numeric_limits<float>::min();
	maxZ = std::numeric_limits<float>::min();
	minX = std::numeric_limits<float>::max();
	minY = std::numeric_limits<float>::max();
	minZ = std::numeric_limits<float>::max();

	while (!feof(file)) {
		fscanf(file, "%s", str);
		if (str[0] == 'v') {
			if (str[1] == 'n') {
				glm::vec3 normal(0, 0, 0);
				fscanf(file, "%f %f %f", &normal.x, &normal.y, &normal.z);
				if (feof(file))
					break;
				normals.push_back(normal);
			}
			else if (str[1] == 't') {
				glm::vec2 texture(0, 0);
				fscanf(file, "%f %f", &texture.x, &texture.y);
				if (feof(file))
					break;
				textures.push_back(texture);
			}
			else if (str[1] == '\0') {
				glm::vec3 vertice(0, 0, 0);
				fscanf(file, "%f %f %f", &vertice.x, &vertice.y, &vertice.z);
				if (feof(file))
					break;
				vertices.push_back(vertice);

				if (vertice.x > maxX)
					maxX = vertice.x;
				if (vertice.x < minX)
					minX = vertice.x;
				if (vertice.y > maxY)
					maxY = vertice.y;
				if (vertice.y < minY)
					minY = vertice.y;
				if (vertice.z > maxZ)
					maxZ = vertice.z;
				if (vertice.z < minZ)
					minZ = vertice.z;
			}
		}
		else if (str[0] == 'f' && str[1] == '\0') {
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			if (feof(file))
				break;
			vertexIndices.push_back(vertexIndex[0] - 1);
			vertexIndices.push_back(vertexIndex[1] - 1);
			vertexIndices.push_back(vertexIndex[2] - 1);
			uvIndices.push_back(uvIndex[0] - 1);
			uvIndices.push_back(uvIndex[1] - 1);
			uvIndices.push_back(uvIndex[2] - 1);
			normalIndices.push_back(normalIndex[0] - 1);
			normalIndices.push_back(normalIndex[1] - 1);
			normalIndices.push_back(normalIndex[2] - 1);
		}
	}
	fclose(file);
}

void OBJObject::reorder() {
	std::vector<glm::vec3> out_vertices;
	std::vector<glm::vec2> out_uvs;
	std::vector<glm::vec3> out_normals;
	for (unsigned int v = 0; v < vertexIndices.size(); v += 3)
	{
		// For each vertex of the triangle
		for (unsigned int i = 0; i < 3; i += 1)
		{
			unsigned int vertexIndex = vertexIndices[v + i];
			glm::vec3 vertex = vertices[vertexIndex];

			unsigned int uvIndex = uvIndices[v + i];
			glm::vec2 uv = textures[uvIndex];

			unsigned int normalIndex = normalIndices[v + i];
			glm::vec3 normal = normals[normalIndex];

			out_vertices.push_back(vertex);
			out_uvs.push_back(uv);
			out_normals.push_back(normal);
			indices.push_back(v + i);
			indicesWithAdj.push_back(v + i);
		}
	}
	vertices = out_vertices;
	textures = out_uvs;
	normals = out_normals;
}

void OBJObject::draw(GLuint shaderProgram)
{
	// Calculate the combination of the model and view (camera inverse) matrices
	glm::mat4 modelview = Window::V * toWorld;
	//modelview = scale(modelview, vec3(factor, factor, factor));

	uProjection = glGetUniformLocation(shaderProgram, "projection");
	uModelview = glGetUniformLocation(shaderProgram, "modelview");
	glUniformMatrix4fv(uProjection, 1, GL_FALSE, &Window::P[0][0]);
	glUniformMatrix4fv(uModelview, 1, GL_FALSE, &modelview[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &toWorld[0][0]);
	//glUniform1i(glGetUniformLocation(shaderProgram, "objectType"), 0);
	
	GLuint uSampler = glGetUniformLocation(shaderProgram, "ourTexture");
	glUniform1i(uSampler, 2);
	glActiveTexture(GL_TEXTURE0 + 2); //and follows 2,4,6

	glBindTexture(GL_TEXTURE_2D, texture[0]);

	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, vertices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

}

void OBJObject::drawWithGeom(GLuint shaderProgram, vec3 lightPos)
{
	// Calculate the combination of the model and view (camera inverse) matrices
	glm::mat4 modelview = Window::V * toWorld;
	//modelview = scale(modelview, vec3(factor, factor, factor));

	uProjection = glGetUniformLocation(shaderProgram, "projection");
	uModelview = glGetUniformLocation(shaderProgram, "modelview");
	glUniformMatrix4fv(uProjection, 1, GL_FALSE, &Window::P[0][0]);
	glUniformMatrix4fv(uModelview, 1, GL_FALSE, &modelview[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &toWorld[0][0]);

	vec3 lightPosInEnemyWorld = inverse(toWorld) * vec4(lightPos, 1.0);
	glUniform3fv(glGetUniformLocation(shaderProgram, "lightPos"), 1, &(lightPosInEnemyWorld[0]));

	/*
	GLuint uSampler = glGetUniformLocation(shaderProgram, "ourTexture");
	glUniform1i(uSampler, 2);
	glActiveTexture(GL_TEXTURE0 + 2); //and follows 2,4,6
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	*/

	glLineWidth(1.0f);
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES_ADJACENCY, indicesWithAdj.size() - indices.size(), GL_UNSIGNED_INT, (void*)(indices.size() * sizeof(GLuint)));
	//glDrawRangeElements(GL_TRIANGLES_ADJACENCY, indices.size(), indicesWithAdj.size(), indicesWithAdj.size() - indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

}

void OBJObject::findAdjacencies()
{
	// find neighbors
	for (uint i = 0; i < indices.size(); i += 3) {
		// If a position vector is duplicated in the VB we fetch the 
		// index of the first occurrence.
		/*
		for (uint j = 0; j < 3; j++) {
			uint Index = i + j;
			aiVector3D& v = paiMesh->mVertices[Index];

			if (m_posMap.find(v) == m_posMap.end()) {
				m_posMap[v] = Index;
			}
			else {
				Index = m_posMap[v];
			}

			Unique.Indices[j] = Index;
		}
		*/
		Edge e1(vertexLabels[vertices[i]], vertexLabels[vertices[i+1]]);
		Edge e2(vertexLabels[vertices[i+1]], vertexLabels[vertices[i+2]]);
		Edge e3(vertexLabels[vertices[i+2]], vertexLabels[vertices[i]]);
		/*
		if (mapNeighbors.find(e1) == mapNeighbors.end())
			mapNeighbors[e1] = Neighbors();
		if (mapNeighbors.find(e2) == mapNeighbors.end())
			mapNeighbors[e2] = Neighbors(); 
		if (mapNeighbors.find(e3) == mapNeighbors.end())
			mapNeighbors[e3] = Neighbors(); 
		*/
		bool check = mapNeighbors[e1].AddNeigbor(i+2); //indices are all unique, should check vectices
		if (!check) { //meaning this face, or three vertices, has already been seen and added to indicesWithAdj
			/*printf("      %f %f %f \n      %f %f %f \n      %f %f %f \n", vertices[i].x, vertices[i].y, vertices[i].z, vertices[i + 1].x, vertices[i + 1].y, vertices[i + 1].z, vertices[i + 2].x, vertices[i + 2].y, vertices[i + 2].z);
			//vertices.erase(vertices.begin() + i, vertices.begin() + i + 3);
			//normals.erase(normals.begin() + i, normals.begin() + i + 3);
			//textures.erase(textures.begin() + i, textures.begin() + i + 3);
			//indices.erase(indices.end() - 3, indices.end());
			//i -= 3;*/
			continue;
		}
		mapNeighbors[e2].AddNeigbor(i);
		/*
		printf("%f %f %f\n", e1.a.x, e1.a.y, e1.a.z);
		printf("%f %f %f\n", e1.b.x, e1.b.y, e1.b.z);
		printf("%f %f %f\n", e2.a.x, e2.a.y, e2.a.z);
		printf("%f %f %f\n", e2.b.x, e2.b.y, e2.b.z);
		if (e1 == e2)
		assert(e1 == e2);
		*/
		mapNeighbors[e3].AddNeigbor(i+1);

		/*
		printf("%f %f %f\n", mapNeighbors[e1].n1, mapNeighbors[e1].n1, mapNeighbors[e1].n1);
		printf("%f %f %f\n", mapNeighbors[e2].n1, mapNeighbors[e2].n1, mapNeighbors[e2].n1);
		printf("%f %f %f\n", mapNeighbors[e3].n1, mapNeighbors[e3].n1, mapNeighbors[e3].n1);
		*/
		
	}

	// build the index buffer with the adjacency info
	for (uint i = 0; i < indices.size(); i+=3) {
		for (uint j = 0; j < 3; j++) {
			Edge e(vertexLabels[vertices[i+j]], vertexLabels[vertices[i + (j + 1) % 3]]);
			if (mapNeighbors.find(e) == mapNeighbors.end())
				printf("map not populated correctly");
			Neighbors n = mapNeighbors[e];
			uint OppositeIndex = n.GetOther(i + (j + 2) % 3);
			if (OppositeIndex == -1) {
				j = 3;
				//indicesWithAdj.erase(indicesWithAdj.end() - j*2, indicesWithAdj.end());
				continue;
			}

			indicesWithAdj.push_back(i + j);
			indicesWithAdj.push_back(OppositeIndex);
		}
		//printf("\n\nindex\n");
	}
}

unsigned char* OBJObject::loadPPM(const char* filename, int& width, int& height)
{
	const int BUFSIZE = 128;
	FILE* fp;
	unsigned int read;
	unsigned char* rawData;
	char buf[3][BUFSIZE];
	char* retval_fgets;
	size_t retval_sscanf;

	if ((fp = fopen(filename, "rb")) == NULL)
	{
		std::cerr << "error reading ppm file, could not locate " << filename << std::endl;
		width = 0;
		height = 0;
		return NULL;
	}

	// Read magic number:
	retval_fgets = fgets(buf[0], BUFSIZE, fp);

	// Read width and height:
	do
	{
		retval_fgets = fgets(buf[0], BUFSIZE, fp);
	} while (buf[0][0] == '#');
	retval_sscanf = sscanf(buf[0], "%s %s", buf[1], buf[2]);
	width = atoi(buf[1]);
	height = atoi(buf[2]);

	// Read maxval:
	do
	{
		retval_fgets = fgets(buf[0], BUFSIZE, fp);
	} while (buf[0][0] == '#');

	// Read image data:
	rawData = new unsigned char[width * height * 3];
	read = fread(rawData, width * height * 3, 1, fp);
	fclose(fp);
	if (read != 1)
	{
		std::cerr << "error parsing ppm file, incomplete data" << std::endl;
		delete[] rawData;
		width = 0;
		height = 0;
		return NULL;
	}

	return rawData;
}

// load image file into texture object
void OBJObject::loadTexture(const char* ppmFile)
{
	int twidth, theight;   // texture width/height [pixels]
	unsigned char* tdata;  // texture pixel data

	twidth = 512;
	theight = 512;

	// Load image file
	tdata = loadPPM(ppmFile, twidth, theight);
	if (tdata == NULL) { 
		printf("cannot open texture file for OBJObject");
		return; 
	}

	// Create ID for texture
	glGenTextures(1, &texture[0]);

	// Set this texture to be the one we are working with
	glBindTexture(GL_TEXTURE_2D, texture[0]);

	// Generate the texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, twidth, theight, 0, GL_RGB, GL_UNSIGNED_BYTE, tdata);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);//GL_LINEAR_MIPMAP_LINEAR);//
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void OBJObject::position(vec3 position)
{
    float maxX = vertices[0].x;
    float minX = vertices[0].x;
    float maxY = vertices[0].y;
    float minY = vertices[0].y;
    float maxZ = vertices[0].z;
    float minZ = vertices[0].z;
    
    for(int i = 0; i < vertices.size(); i++){
        //Traverse (loop over) the vertices of each of your models and look at each dimension (x,y,z) independently
        if(vertices[i].x > maxX){
            maxX = vertices[i].x;
        }
        if(vertices[i].x < minX){
            minX = vertices[i].x;
        }
        if(vertices[i].y > maxY){
            maxY = vertices[i].y;
        }
        if(vertices[i].y < minY){
            minY = vertices[i].y;
        }
        if(vertices[i].z > maxZ){
            maxZ = vertices[i].z;
        }
        if(vertices[i].z < minZ){
            minZ = vertices[i].z;
        }
    }
    float shiftXposition = position.x - ((maxX + minX)/2);
    float shiftYposition = position.y - ((maxY + minY)/2);
    float shiftZposition = position.z - ((maxZ + minZ)/2);
    
    for(int i = 0; i < vertices.size(); i++){
        vertices[i].x = vertices[i].x + shiftXposition;
        vertices[i].y = vertices[i].y + shiftYposition;
        vertices[i].z = vertices[i].z + shiftZposition;
    }
    //printf("calling the position method");
    glBindBuffer(GL_ARRAY_BUFFER, VBO0);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float) * 3, vertices.data(), GL_STATIC_DRAW);
    // Enable the usage of layout location 0 (check the vertex shader to see what this is)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,// This first parameter x should be the same as the number passed into the line "layout (location = x)" in the vertex shader. In this case, it's 0. Valid values are 0 to GL_MAX_UNIFORM_LOCATIONS.
                          3, // This second line tells us how any components there are per vertex. In this case, it's 3 (we have an x, y, and z component)
                          GL_FLOAT, // What type these components are
                          GL_TRUE, // GL_TRUE means the values should be normalized. GL_FALSE means they shouldn't
                          3 * sizeof(GLfloat), // Offset between consecutive indices. Since each of our vertices have 3 floats, they should have the size of 3 floats in between
                          (GLvoid*)0); // Offset of the first vertex's component. In our case it's 0 since we don't pad the vertices array with anything.
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    
}

void OBJObject::changeScale(float scalingVal)
{
	toWorld = scale(toWorld, vec3(scalingVal, scalingVal, scalingVal));
}