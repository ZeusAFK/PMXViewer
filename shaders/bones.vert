#version 420 core

uniform mat4 MVP;

//Vertex Properties
layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec2 vUV;
layout(location = 2) in vec3 vNormal;

out vec2 UV;
out vec3 normal;

void main()
{
	//gl_Position=vPosition;
	
	//gl_Position = (model_matrix * vPosition);
	
	//gl_Position = projection_matrix * (model_matrix * vPosition);
	
	gl_Position = MVP * vPosition;
	
	UV = vUV;
	normal = vNormal;
}
