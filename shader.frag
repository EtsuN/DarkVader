#version 330 core
// This is a sample fragment shader.

// Inputs to the fragment shader are the outputs of the same name from the vertex shader.
// Note that you do not have access to the vertex shader's default output, gl_Position.
in float sampleExtraOutput;
in vec3 normalOutput;
in vec3 positionOutput;
in vec2 textureOutput;

uniform samplerCube skybox;
uniform sampler2D ourTexture; //takes in an uniform
uniform int objectType;
uniform vec3 cameraPos;
uniform vec3 lightPos;
uniform mat4 model;
uniform vec4 particleColor;
uniform float threshold;

out vec4 color;

void main()
{

	vec3 ambient;
	vec3 lightDir;
	float diff;
	vec3 diffuse;
	switch (objectType) {
		case 0: //enemy
		case 3: //cube
			if (objectType == 0) {
				color = texture(ourTexture, textureOutput);
			}
			else {
				color = vec4(1,1,1,1);
			}

			vec3 fragPos = vec3(model * vec4(positionOutput, 1.0));
			vec3 norm = normalize(normalOutput);
			lightDir = normalize(lightPos - fragPos);
			diff = max(dot(norm, lightDir), 0.0);
			diffuse = diff * vec3(1, 1, 1); //lightColor = (1,1,1)

			vec3 surfaceToLight = lightPos - fragPos;
			float attenuation = min(1, 1/ 0.05 / (length(surfaceToLight)));

			//float ambientStrength = 0.1;
			//ambient = ambientStrength * vec3(1,1,1); //lightColor = (1,1,1)
			color = vec4(attenuation * (diffuse) * vec3(color), 1.0); //+ambient
			
			//if (objectType == 0) 
				//color = vec4(color.x - 0.005f,  color.y - 0.005f, color.z, 1.0f);

			break;
		case 1: //skybox
		    color = texture(skybox, positionOutput);
			break;
		case 2: //environmental mapping
			vec3 I = normalize(positionOutput - cameraPos);
			vec3 R = reflect(I, normalize(normalOutput));
			color = vec4(texture(skybox, R).rgb, 1.0);
        //color = vec4(normalOutput.x, normalOutput.y, normalOutput.z, 1.0);
			break;
		case 4: //2d image
            color = texture( ourTexture, textureOutput );
			if (color.z > threshold) 
				color = vec4(vec3(color), 0.0f);
			break;
		case 5: //light
            color = vec4(1, 1, 1, 1);
			break;
		case 6: //particle
            color = particleColor;
			break;
		case 7: //attacks by phantom
            color = particleColor;
			break;

	}
}
