//g++ *.cpp *.cc -o pmx -L./ -lglut -lGLU -lGLEW -lGL -ggdb -lSOIL -std=c++11
#include <iostream>
#include <fstream>
#include <sstream>

#include "vgl.h"
//#include "vmath.h"

#include <GL/freeglut_ext.h>

#include "SOIL/SOIL.h"

#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include "src/texthandle.h"
#include "src/pmx.h"
#include "src/vmd.h"
#include "src/motioncontroller.h"
#include "src/shader.h"
#include "src/pmxvLogger.h"

#define NO_STRIDE 0

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define CONST_TEST_FRAME_NUMBER 1470

using namespace std;

enum VAO_IDs { Vertices, BoneVertices, NumVAOs };
enum Buffer_IDs { VertexArrayBuffer, VertexIndexBuffer, NumBuffers };
enum Attrib_IDs { vPosition, vUV, vNormal, vBoneIndices, vBoneWeights, vWeightFormula };

GLuint VAOs[NumVAOs];
GLuint Buffers[NumBuffers];

GLuint MVP_loc;

PMXInfo pmxInfo;
VMDInfo vmdInfo;
VMDMotionController *motionController;

vector<GLuint> textures;

void loadTextures(PMXInfo &pmxInfo, vector<GLuint> &textures)
{	
	for(int i=0; i<pmxInfo.texture_continuing_datasets; ++i)
	{
		cout<<"Loading "<<pmxInfo.texturePaths[i]<<"...";
		if(pmxInfo.texturePaths[i].substr(pmxInfo.texturePaths[i].size()-3)=="png")
		{
			GLuint texture;
			int width, height;
			unsigned char* image;
			string loc=pmxInfo.texturePaths[i];
			
			ifstream test(loc);
			if(!test.is_open())
			{
				cerr<<"Texture file could not be found: "<<loc<<endl;
				exit(EXIT_FAILURE);
			}
			test.close();
			
			glActiveTexture( GL_TEXTURE0 );
			glGenTextures( 1, &texture );
			glBindTexture( GL_TEXTURE_2D, texture );
				image = SOIL_load_image( loc.c_str(), &width, &height, 0, SOIL_LOAD_RGBA );
			glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image );
			SOIL_free_image_data( image );
			//glUniform1i( glGetUniformLocation( program, "texKitten" ), 0 );

			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			
			if(texture==0)
			{
				cerr<<"Texture failed to load: "<<pmxInfo.texturePaths[i]<<endl;
				printf( "SOIL loading error: '%s'\n", SOIL_last_result() );
				exit(EXIT_FAILURE);
			}
			
			cout<<"done"<<endl;
			
			textures.push_back(texture);
		}
		else
		{
			GLuint texture;
			int width, height;
			unsigned char* image;
			string loc=pmxInfo.texturePaths[i];
			
			ifstream test(loc);
			if(!test.is_open())
			{
				cerr<<"Texture file could not be found: "<<loc<<endl;
				//exit(EXIT_FAILURE);
			}
			test.close();
			
			glActiveTexture(GL_TEXTURE0);
			glGenTextures( 1, &texture );
			glBindTexture( GL_TEXTURE_2D, texture );
			image = SOIL_load_image( loc.c_str(), &width, &height, 0, SOIL_LOAD_RGB );
			glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image );
			SOIL_free_image_data( image );
			
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
 
			if(texture == 0)
			{
				cerr<<"Texture failed to load: "<<pmxInfo.texturePaths[i]<<endl;
				printf( "SOIL loading error: '%s'\n", SOIL_last_result() );
				exit(EXIT_FAILURE);
			}
			
			textures.push_back(texture);
			
			cout<<"done"<<endl;
		}
	}
}



struct VertexData
{
	glm::vec4 position;
	glm::vec2 UV;
	glm::vec3 normal;

	GLfloat weightFormula;

	GLfloat boneIndex1;
	GLfloat boneIndex2;
	GLfloat boneIndex3;
	GLfloat boneIndex4;

	GLfloat weight1;
	GLfloat weight2;
	GLfloat weight3;
	GLfloat weight4;

	string str()
	{	
		std::stringstream ss;
		ss << position.x << " " << position.y << " " << position.z << " " << position.w << endl;
		ss << UV.x << " " << UV.y << endl;
		ss << normal.x << " " << normal.y << " " << normal.z << endl;
		ss << boneIndex1 << " " << boneIndex2 << " " << boneIndex3 << " " << boneIndex4 << endl;
		ss << weight1 << " " << weight2 << " " << weight3 << " " << weight4 << endl;

		return ss.str();
	}
};
     
GLuint shaderProgram;
void init(PMXInfo &pmxInfo, VMDInfo &vmdInfo)
{	
	shaderProgram=loadShaders();

	//Load Textures
	loadTextures(pmxInfo,textures);
	
	#ifdef MODELDUMP
	ofstream modeldump("modeldump.txt");
	modeldump << "indices:" << endl;
	#endif
           
	//GLushort vertexIndices[pmxInfo.face_continuing_datasets][3];
	GLushort *vertexIndices= (GLushort*) malloc(pmxInfo.face_continuing_datasets*sizeof(GLushort)*3);
	for(int i=0; i<pmxInfo.faces.size(); ++i) //faces.size()
	{
		int j=i*3;
		vertexIndices[j]=pmxInfo.faces[i]->points[0];
		vertexIndices[j+1]=pmxInfo.faces[i]->points[1];
		vertexIndices[j+2]=pmxInfo.faces[i]->points[2];
		
		#ifdef MODELDUMP
		modeldump << vertexIndices[j] << " " << vertexIndices[j+1] << " " << vertexIndices[j+2] << endl;
		#endif
	}
	
	#ifdef MODELDUMP
	modeldump << "vertices:" << endl;
	#endif

	VertexData *vertexData = (VertexData*)malloc(pmxInfo.vertex_continuing_datasets*sizeof(VertexData));
	for(int i=0; i<pmxInfo.vertex_continuing_datasets; ++i)
	{
		vertexData[i].position.x=pmxInfo.vertices[i]->pos[0];
		vertexData[i].position.y=pmxInfo.vertices[i]->pos[1];
		vertexData[i].position.z=pmxInfo.vertices[i]->pos[2];
		vertexData[i].position.w=1.0;

		vertexData[i].UV.x=pmxInfo.vertices[i]->UV[0];
		vertexData[i].UV.y=pmxInfo.vertices[i]->UV[1];

		vertexData[i].normal.x=pmxInfo.vertices[i]->normal[0];
		vertexData[i].normal.y=pmxInfo.vertices[i]->normal[1];
		vertexData[i].normal.z=pmxInfo.vertices[i]->normal[2];

		vertexData[i].weightFormula=pmxInfo.vertices[i]->weight_transform_formula;

		vertexData[i].boneIndex1=pmxInfo.vertices[i]->boneIndex1;
		vertexData[i].boneIndex2=pmxInfo.vertices[i]->boneIndex2;
		vertexData[i].boneIndex3=pmxInfo.vertices[i]->boneIndex3;
		vertexData[i].boneIndex4=pmxInfo.vertices[i]->boneIndex4;

		vertexData[i].weight1=pmxInfo.vertices[i]->weight1;
		vertexData[i].weight2=pmxInfo.vertices[i]->weight2;
		vertexData[i].weight3=pmxInfo.vertices[i]->weight3;
		vertexData[i].weight4=pmxInfo.vertices[i]->weight4;

		#ifdef MODELDUMP
		modeldump << vertexData[i].str();
		#endif
                   
		//cout<<vertexData[i].position.x<<" "<<vertexData[i].position.y<<" "<<vertexData[i].position.z<<" "<<vertexData[i].position.w<<endl;
		//cout<<vertexData[i].UV.x<<" "<<vertexData[i].UV.y<<endl;

		/*if(pmxInfo.vertices[i]->weight_transform_formula>2)
		{
			cerr<<"SDEF and QDEF not supported yet"<<endl;
			exit(EXIT_FAILURE);
		}*/
	}
	
	#ifdef MODELDUMP
	modeldump.close();
	#endif
	
	
	//Generate all Buffers
	glGenBuffers(NumBuffers,Buffers);
	
	//init Element Buffer Object
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Buffers[VertexIndexBuffer]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, pmxInfo.face_continuing_datasets*sizeof(GLushort)*3, vertexIndices, GL_STATIC_DRAW);
	
	
	
	//Init Vertex Array Buffer Object
	glGenVertexArrays(NumVAOs, VAOs);
	glBindVertexArray(VAOs[Vertices]);
	
	glBindBuffer(GL_ARRAY_BUFFER, Buffers[VertexArrayBuffer]);
	glBufferData(GL_ARRAY_BUFFER, pmxInfo.vertex_continuing_datasets*sizeof(VertexData), vertexData, GL_STATIC_DRAW);
	
	//Intialize Vertex Attribute Pointer	
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, sizeof(VertexData), BUFFER_OFFSET(0)); //4=number of components updated per vertex
	glEnableVertexAttribArray(vPosition);
	
	glVertexAttribPointer(vUV, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), BUFFER_OFFSET(sizeof(GLfloat)*4));
	glEnableVertexAttribArray(vUV);
	
	glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), BUFFER_OFFSET(sizeof(GLfloat)*6));
	glEnableVertexAttribArray(vNormal);
	
	glVertexAttribPointer(vWeightFormula, 1, GL_FLOAT, GL_FALSE, sizeof(VertexData), BUFFER_OFFSET(sizeof(GLfloat)*9));
	glEnableVertexAttribArray(vWeightFormula);
	
	glVertexAttribPointer(vBoneIndices, 4, GL_FLOAT, GL_FALSE, sizeof(VertexData), BUFFER_OFFSET(sizeof(GLfloat)*10));
	glEnableVertexAttribArray(vBoneIndices);
	
	glVertexAttribPointer(vBoneWeights, 4, GL_FLOAT, GL_FALSE, sizeof(VertexData), BUFFER_OFFSET(sizeof(GLfloat)*14));
	glEnableVertexAttribArray(vBoneWeights);
	
	free(vertexData);
	free(vertexIndices);
	

	MVP_loc = glGetUniformLocation(shaderProgram, "MVP");
	
	motionController=new VMDMotionController(pmxInfo,vmdInfo,shaderProgram);
	motionController->updateBoneMatrix();
    
    glDisable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glPointSize(5.0);
	//glClearDepth(1.0f);
	
	//glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
}


//Spherical coordinate system (http://en.wikipedia.org/wiki/Spherical_coordinate_system)
float theta=3.0f*M_PI/2.0f;
float zenith=0.0;
float radius=10.0f;
float t=radius; //distances to center after rotating up/down

float theta2=0.0f;
float radius2=20.0f;
//glm::vec3 cameraPosition(0.0f, 16.0f, radius*sin(theta));
//glm::vec3 cameraPosition(0.0f, 0.0f, radius*sin(theta));
glm::vec3 cameraPosition(0.0f, 0.0f, radius*sin(theta));
glm::vec3 cameraTarget(0.0f,0.0f,0.0f);


glm::vec3 modelTranslate(0.0f,-16.0f,0.0f);

void drawModel(PMXInfo &pmxInfo)
{	
	glBindVertexArray(VAOs[Vertices]);
	glBindBuffer(GL_ARRAY_BUFFER, Buffers[VertexArrayBuffer]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Buffers[VertexIndexBuffer]);
	
	glm::mat4 Projection = glm::perspective(45.0f, 16.0f/9.0f, 0.1f, 100.0f);
	// Camera matrix
	glm::mat4 View       = glm::lookAt(
		glm::vec3(cameraPosition.x,cameraPosition.y,cameraPosition.z), // Camera is at (4,3,3), in World Space
		cameraTarget, // and looks at the origin
		glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
	);
	View= glm::rotate(0.0f,0.0f,0.0f,1.0f)* View;
	// Model matrix : an identity matrix (model will be at the origin)
	glm::mat4 Model = glm::rotate(0.0f, 0.0f, 0.0f, 1.0f) * glm::translate(modelTranslate.x, modelTranslate.y, modelTranslate.z);
	// Our ModelViewProjection : multiplication of our 3 matrices
	glm::mat4 MVP = Projection * View * Model;
	
	glUniformMatrix4fv(MVP_loc, 1, GL_FALSE, &MVP[0][0]);
	
	motionController->updateBoneMatrix();
	
	GLuint ambientColorLoc=glGetUniformLocation(shaderProgram, "ambientColor");
	GLuint diffuseColorLoc=glGetUniformLocation(shaderProgram, "diffuseColor");
	GLuint specularColorLoc=glGetUniformLocation(shaderProgram, "specularColor");
	
	GLuint shininessLoc=glGetUniformLocation(shaderProgram, "shininess");
	
	glm::vec3 halfVector=glm::normalize(cameraPosition - cameraTarget);
	GLuint halfVectorLoc=glGetUniformLocation(shaderProgram, "halfVector");
    
	int faceCount=0;
	for(int m=0; m<pmxInfo.material_continuing_datasets; ++m) //pmxInfo.material_continuing_datasets
	{
		//cout<<"Texture Index: "<<pmxInfo.materials[m]->textureIndex<<endl;
		//cout<<"Toon Index: "<<pmxInfo.materials[m]->toonTextureIndex<<endl;
		//cout<<"Sphere Index: "<<pmxInfo.materials[m]->sphereIndex<<endl;
		//cout<<"Face count: "<<faceCount<<endl;
		//cout<<"HasFaceNum: "<<pmxInfo.materials[m]->hasFaceNum<<endl;
		
		//cout<<pmxInfo.materials[m]->shininess<<endl;
		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D,textures[pmxInfo.materials[m]->textureIndex]);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D,textures[pmxInfo.materials[m]->toonTextureIndex]);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D,textures[pmxInfo.materials[m]->sphereIndex]);
		
		
		/*glUniform3fv(ambientColorLoc,1,&pmxInfo.materials[m]->ambient);
		glUniform3fv(specularColorLoc,1,pmxInfo.materials[m]->specular);
		glUniform1f(shininessLoc,glm::normalize(pmxInfo.materials[m]->shininess));
		glUniform4fv(diffuseColorLoc,1,pmxInfo.materials[m]->diffuse);
		glUniform3f(halfVectorLoc,halfVector.x,halfVector.y,halfVector.z);*/
		
		//cout<<pmxInfo.materials[m]->specular[0]<<endl;
		
		
		
		glDrawElements(GL_TRIANGLES, (pmxInfo.materials[m]->hasFaceNum), GL_UNSIGNED_SHORT, BUFFER_OFFSET(sizeof(short)*faceCount));
		
		faceCount+=pmxInfo.materials[m]->hasFaceNum;
	}
}


void timer(int id)
{
	motionController->advanceTime();
	glutPostRedisplay();
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	drawModel(pmxInfo);
	
	glutSwapBuffers();
	glutTimerFunc(16, timer, 0);
}

void resize(int w, int h)
{
	glViewport(0, 0, (GLsizei) w, (GLsizei) h);
	//cout<<"AR changed"<<endl;
}

void key(unsigned char key, int x, int y)
{
	if (key == 'q') exit(0);
}

void processSpecialKeys(int key, int xx, int yy) {

	float fraction = 1.0f;

	switch (key) {
		case GLUT_KEY_LEFT :
			break;
		case GLUT_KEY_RIGHT :
			break;
		case GLUT_KEY_UP :
			modelTranslate.y-=1.0;
			break;
		case GLUT_KEY_DOWN :
			modelTranslate.y+=1.0;
			break;
	}
}

int prevX=0,prevY=0;
void mouse(int button, int dir, int x, int y)
{
	switch(button)
	{
		case 3: //mouse scroll up
			radius-=1.0f;
			t=radius*cos(zenith);
			cameraPosition.y=radius*sin(zenith);
	
			cameraPosition.x=t*cos(theta);
			cameraPosition.z=t*sin(theta);
		break;
		
		case 4: //mouse scroll down
			radius+=1.0f;
			t=radius*cos(zenith);
			cameraPosition.y=radius*sin(zenith);
	
			cameraPosition.x=t*cos(theta);
			cameraPosition.z=t*sin(theta);
		break;
		
		case GLUT_LEFT_BUTTON:
			prevX=x;
			prevY=y;
		break;
	}
}
void mouseMotion(int x, int y)
{
	int dx=x-prevX,dy=y-prevY;
	
	
	/*rotX+=dx;
	rotY+=dy;
	rotZ+=dx+dy;*/
	theta+=dx/250.0f;
	zenith+=dy/250.0f;
	
	if(zenith>=M_PI/2) zenith=M_PI/2-0.0001f;
	if(zenith<=-M_PI/2)zenith=-M_PI/2+0.0001f;
	
	//cout<<theta<<" "<<zenith<<endl;
	//cout<<dx<<" "<<dy<<endl;
	
	t=radius*cos(zenith);
	cameraPosition.y=radius*sin(zenith);
	
	cameraPosition.x=t*cos(theta);
	cameraPosition.z=t*sin(theta);
	
	
	/*zenith+=dx/250.0f;
	cameraPosition.x=radius*cos(theta);
	cameraPosition.z=radius*sin(theta);*/
	
	/*theta2+=dy/250.0f;
	cameraPosition.y=radius2*sin(theta2);
	cameraPosition.z=radius2*cos(theta2);*/
	
	
	//cameraPosition.z=radius*sin(theta)+radius2*cos(theta2);
	
	prevX=x, prevY=y;
}



int main(int argc, char** argv)
{
	log("Starting PMXViewer v0.1");
	pmxInfo=readPMX("data/model/gumiv3/","GUMI_V3.pmx");
	vmdInfo=readVMD("data/motion/Masked bitcH/Masked bitcH.vmd");
	log("Model loaded");
	
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	//glutInitWindowSize(1024, 576);
	glutInitWindowSize(1920, 1080);
	glutInitContextVersion(3,2);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow(argv[0]);
	
	
	glewExperimental = GL_TRUE; 
	if(glewInit())
	{
		cerr<<"Unable to initialize GLEW...exiting"<<endl;
		exit(EXIT_FAILURE);
	}
	
	init(pmxInfo, vmdInfo);
	
	glutDisplayFunc(display);
	glutReshapeFunc(resize);
	glutKeyboardFunc(key);
	glutSpecialFunc(processSpecialKeys);
	
	glutMouseFunc(mouse);
	glutMotionFunc(mouseMotion);
	//glutIdleFunc(idle);
	
	glutMainLoop();
	
	delete pmxvLogger::get();
	return 0;
}
