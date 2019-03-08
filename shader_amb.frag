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

out vec4 color;

void main()
{
	float ambientStrength = 0.4;
	vec3 ambient;

	switch (objectType) {
		case 0: //enemy
		case 3: //ground, or cube
			color = vec4(1.,1.,1.,1);
			if (objectType == 0) {
				color = texture(ourTexture, textureOutput);
			}

			ambient = ambientStrength * vec3(1,1,1); //lightColor = (1,1,1)
			
			vec3 fragPosition = vec3(model * vec4(positionOutput, 1));
			vec3 surfaceToLight = lightPos - fragPosition;
			float attenuation = min(1, 1/ 0.02 / (length(surfaceToLight)));
			color = vec4(attenuation * ambient * vec3(color), 1.0);
			
			/*if (objectType == 0) {
				color = vec4(color.x, color.y, color.z + 0.005f, 1.0f);
				//attenuation = attenuation * 0.02 / 0.05; 
				//if (max(dot(normalize(normalOutput), normalize(surfaceToLight) ), 0.0) * attenuation + color.z >= 1)
					//color = vec4(1,0,0,1);
			}*/
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
			if (color.x > 0.25) 
				color = vec4(vec3(color), 0.0f);
			break;
		case 5: //light
            color = vec4(1, 1, .8, 1);
			break;
		case 6: //particle
            color = particleColor;
			break;
		case 7: //attacks by phantom
            color = vec4(1, 0, 0, 1);
			break;
	}
}
