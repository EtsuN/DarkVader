#version 330 core

layout (triangles_adjacency) in;
layout (triangle_strip, max_vertices = 18) out;

in vec3 modelPos[];

uniform vec3 lightPos;
uniform mat4 projection;
uniform mat4 modelview;     
uniform int objectType;

float EPSILON = 0.01; //0.0001

// Emit a quad using a triangle strip
void EmitQuad(vec3 StartVertex, vec3 EndVertex)
{
    // Vertex #1: the starting vertex (just a tiny bit below the original edge)
    vec3 LightDir = normalize(StartVertex - lightPos); 
    gl_Position = projection * modelview * vec4((StartVertex + LightDir * EPSILON), 1.0);
    EmitVertex();

    // Vertex #2: the starting vertex projected to infinity
    gl_Position = projection * modelview * vec4(LightDir, 0.0);
    EmitVertex();

    // Vertex #3: the ending vertex (just a tiny bit below the original edge)
    LightDir = normalize(EndVertex - lightPos);
    gl_Position = projection * modelview * vec4((EndVertex + LightDir * EPSILON), 1.0);
    EmitVertex();

    // Vertex #4: the ending vertex projected to infinity
    gl_Position = projection * modelview * vec4(LightDir , 0.0);
    EmitVertex();

    EndPrimitive(); 
}

void main()
{
	if (objectType == 3) { 
		EPSILON = 0.0001; //0.0001
	}

    vec3 e1 = modelPos[2] - modelPos[0];
    vec3 e2 = modelPos[4] - modelPos[0];
    vec3 e3 = modelPos[1] - modelPos[0];
    vec3 e4 = modelPos[3] - modelPos[2];
    vec3 e5 = modelPos[4] - modelPos[2];
    vec3 e6 = modelPos[5] - modelPos[0];

    vec3 Normal = normalize(cross(e1,e2));
    vec3 LightDir = normalize(lightPos - modelPos[0]);

    if (dot(Normal, LightDir) > 0) {
        Normal = cross(e3,e1);

        if (dot(Normal, LightDir) <= 0) {
            vec3 StartVertex = modelPos[0];
            vec3 EndVertex = modelPos[2];
            EmitQuad(StartVertex, EndVertex);
        }

        Normal = cross(e4,e5);
        LightDir = lightPos - modelPos[2];

        if (dot(Normal, LightDir) <= 0) {
            vec3 StartVertex = modelPos[2];
            vec3 EndVertex = modelPos[4];
            EmitQuad(StartVertex, EndVertex);
        }

        Normal = cross(e2,e6);
        LightDir = lightPos - modelPos[4];

        if (dot(Normal, LightDir) <= 0) {
            vec3 StartVertex = modelPos[4];
            vec3 EndVertex = modelPos[0];
            EmitQuad(StartVertex, EndVertex);
        }

        // render the back cap
        LightDir = modelPos[0] - lightPos;
        gl_Position = projection * modelview * vec4(LightDir, 0.0);
        EmitVertex();

        LightDir = modelPos[4] - lightPos;
        gl_Position = projection * modelview * vec4(LightDir, 0.0);
        EmitVertex();

        LightDir = modelPos[2] - lightPos;
        gl_Position = projection * modelview * vec4(LightDir, 0.0);
        EmitVertex();

        EndPrimitive();
    }
	else {
	
        // render the front cap
        LightDir = (normalize(modelPos[0] - lightPos));
        gl_Position = projection * modelview * vec4((modelPos[0] + LightDir * EPSILON), 1.0);
        EmitVertex();

        LightDir = (normalize(modelPos[4] - lightPos));
        gl_Position = projection * modelview * vec4((modelPos[4] + LightDir * EPSILON), 1.0);
        EmitVertex();

        LightDir = (normalize(modelPos[2] - lightPos));
        gl_Position = projection * modelview * vec4((modelPos[2] + LightDir * EPSILON), 1.0);
        EmitVertex();

        EndPrimitive();
 
	}
}
