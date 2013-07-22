#ifndef SHADER_H
#define SHADER_H

#include "../vgl.h"

const GLchar* ReadShader(const char* filename);
GLuint compileShader(GLenum type, const GLchar *filename);
GLuint loadShaders();

#endif
