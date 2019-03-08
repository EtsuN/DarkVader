#version 330

layout (location = 0) in vec3 position;                                           
layout (location = 1) in vec3 normal;                                              
layout (location = 2) in vec2 textures;                                                

//out vec3 WorldPos0;                                                                 
out vec3 modelPos;

//uniform mat4 projection;
//uniform mat4 modelview;                                           
//uniform mat4 model;                                                
                                                                                    
void main()                                                                         
{                                                   
    //gl_Position = projection * modelview * vec4(position, 1.0);	
	modelPos = position;
    //WorldPos0  = vec3(model *  vec4(position, 1.0));                                
}
