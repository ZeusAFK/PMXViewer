#include "shader.h"

#include <stdio.h>
#include <iostream>
#include <fstream>

using namespace std;

const GLchar* ReadShader(const char* filename)
{
#ifdef _WIN32
	FILE* infile;
	fopen_s( &infile, filename, "rb" );
#else
    FILE* infile = fopen( filename, "rb" );
#endif //_WIN32

    if ( !infile ) {
#ifdef _DEBUG
        std::cerr << "Unable to open file '" << filename << "'" << std::endl;
#endif /* DEBUG */
        return NULL;
    }

    fseek( infile, 0, SEEK_END );
    int len = ftell( infile );
    fseek( infile, 0, SEEK_SET );

    GLchar* source = new GLchar[len+1];

    fread( source, 1, len, infile );
    fclose( infile );

    source[len] = 0;

    return const_cast<const GLchar*>(source);
}

GLuint compileShader(GLenum type, const GLchar *filename)
{
	const GLchar* source=ReadShader(filename);
	if(source==NULL)
	{
		cerr<<"FATAL ERROR: shader file could not be read"<<endl;
		exit(EXIT_FAILURE);
	}
	
	//cout<<"Source: "<<source<<endl;
	
	GLuint shader=glCreateShader(type);
	
	glShaderSource(shader, 1, &source, NULL);
	
	glCompileShader(shader);
	GLint compileStatus=-1;
	glGetShaderiv(shader,  GL_COMPILE_STATUS,  &compileStatus);
	cout<<"Compile Status: "<<compileStatus<<endl;
	if(compileStatus==GL_FALSE)
	{
		char *shaderLog=(char*) malloc(sizeof(char)*500);
		glGetShaderInfoLog(shader, sizeof(char)*500, NULL, shaderLog);
		
		cout<<endl<<shaderLog<<endl;
		
		exit(EXIT_FAILURE);
	}
	
	return shader;
}


GLuint loadShaders()
{
	GLuint vertShader=compileShader(GL_VERTEX_SHADER,"shaders/triangles.vert");
	GLuint fragShader=compileShader(GL_FRAGMENT_SHADER,"shaders/triangles.frag");

	GLuint program=glCreateProgram();
	glAttachShader(program,vertShader);
	glAttachShader(program,fragShader);
	
	glLinkProgram(program);
	
	GLint linkStatus=-1;
	glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
	cout<<"Link Status: "<<linkStatus<<endl;
	
	if(linkStatus==GL_FALSE)
	{
		char *linkerLog=(char*) malloc(sizeof(char)*500);
		glGetProgramInfoLog(program, sizeof(char)*500, NULL, linkerLog);
		
		cout<<endl<<linkerLog<<endl;
		
		//exit(EXIT_FAILURE);
	}
	
	
	glUseProgram(program);
	
	return program;
}
