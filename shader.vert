#version 330 core
// NOTE: Do NOT use any version older than 330! Bad things will happen!

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texture;

// Uniform variables can be updated by fetching their location and passing values to that location
uniform mat4 projection;
uniform mat4 modelview;
uniform int objectType;
uniform mat4 model;
uniform vec3 cameraPos;
uniform vec3 lightPos;

out vec3 normalOutput;
out vec3 positionOutput;
out vec2 textureOutput;

void main()
{
	//Image 2d
	if (objectType == 4) {
		gl_Position =  vec4(position,1);
	}
	else {
		gl_Position = projection * modelview * vec4(position.x, position.y, position.z, 1.0);	
	}
    	
    //for lighting
    //take in the model matric instead

    normalOutput = mat3(transpose(inverse(model))) * normal;
    //normalOutput = normal;
    textureOutput = texture;
    positionOutput = position;
   
}
