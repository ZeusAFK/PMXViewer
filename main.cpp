//g++ *.cpp *.cc -o pmx -L./ -lglut -lGLU -lGLEW -lGL -ggdb -lSOIL -std=c++11
#include <iostream>
#include <fstream>

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
#include "src/shader.h"

#define NO_STRIDE 0

using namespace std;

enum VAO_IDs { Vertices, BoneVertices, NumVAOs };
enum Buffer_IDs { VertexArrayBuffer, VertexIndexBuffer, BoneBuffer, NumBuffers };
enum Attrib_IDs { vPosition, vUV, vNormal, vBoneIndices, vBoneWeights, vWeightFormula };

GLuint VAOs[NumVAOs];
GLuint Buffers[NumBuffers];

const GLuint NumVertices=6;

GLuint MVP_loc;

GLuint boneOffset_loc;
GLuint boneQuaternion_loc;

PMXInfo pmxInfo;
VMDInfo vmdInfo;



vector<GLuint> textures;

void loadTextures(PMXInfo &pmxInfo)
{	
	for(int i=0; i<pmxInfo.texture_continuing_datasets; ++i)
	{
		cout<<"Loading "<<pmxInfo.texturePaths[i]<<"...";
		if(pmxInfo.texturePaths[i].substr(pmxInfo.texturePaths[i].size()-3)=="png")
		{
			GLuint texture;
			int width, height;
			unsigned char* image;
			string loc="gumiv3/"+pmxInfo.texturePaths[i];
			
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
			string loc="gumiv3/"+pmxInfo.texturePaths[i];
			
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


void quaternionToMat4(glm::mat4 &m, glm::vec3 &p, glm::quat &q)
{
	//cout<<"Vector: "<<p[0]<<" "<<p[1]<<" "<<p[2]<<endl;
	
	m[3][0] = p[0];
	m[3][1] = p[1];
	m[3][2] = p[2];
	
	float x2 = q[0] + q[0];
	float y2 = q[1] + q[1];
	float z2 = q[2] + q[2];
	{
		float xx2 = q[0] * x2;
		float yy2 = q[1] + y2;
		float zz2 = q[2] + z2;

		m[0][0] = 1.0f - yy2 - zz2;
		m[1][1] = 1.0f - xx2 - zz2;
		m[2][2] = 1.0f - xx2 - yy2;
	}
	{
		float yz2 = q[1] * z2;
		float wx2 = q[3] * x2;
		m[1][2] = yz2 - wx2;
		m[2][1] = yz2 + wx2;
	}
	{
		float xy2 = q[0] * y2;
		float wz2 = q[3] * z2;
		m[0][1] = xy2 - wz2;
		m[1][0] = xy2 + wz2;
	}
	{
		float xz2 = q[0] * z2;
		float wy2 = q[3] * y2;
		m[2][0] = xz2 - wy2;
		m[0][2] = xz2 + wy2;
	}
	
	/*cout<<"Quat Matrix:"<<endl;
	cout<<m[0][0]<<" "<<m[0][1]<<" "<<m[0][2]<<" "<<m[0][3]<<endl
	<<m[1][0]<<" "<<m[1][1]<<" "<<m[1][2]<<" "<<m[1][3]<<endl
	<<m[2][0]<<" "<<m[2][1]<<" "<<m[2][2]<<" "<<m[2][3]<<endl
	<<m[3][0]<<" "<<m[3][1]<<" "<<m[3][2]<<" "<<m[3][3]<<endl<<endl;*/
	
}


glm::mat4 setBoneToFrame(PMXBone *b, BoneFrame &bf)
{
	//glm::mat4 invBindPose=glm::inverse(bindPose);
	//*****************LOOK HERE*****************/
	
	
	cout<<"Refreshing relative position matrix for: "<<b->name<<endl;
	cout<<bf.position.x<<" "<<bf.position.y<<" "<<bf.position.z<<endl;
	cout<<bf.quaternion.x<<" "<<bf.quaternion.y<<" "<<bf.quaternion.z<<" "<<bf.quaternion.w<<endl<<endl;
				
	glm::mat4 aniMatrix=glm::toMat4(bf.quaternion);
	aniMatrix[3][0]=bf.position.x;
	aniMatrix[3][1]=bf.position.y;
	aniMatrix[3][2]=bf.position.z;
	
	/*aniMatrix[0][3]=b->position.x;
	aniMatrix[1][3]=b->position.y;
	aniMatrix[2][3]=b->position.z;*/
	
	glm::mat4 invAniMatrix=glm::inverse(aniMatrix);
	
	glm::mat4 L=aniMatrix; //setBoneToFrame(pmxInfo.bones[bone->parentBoneIndex],bf);
	
	/*L[0][3]=-b->position.x;
	L[1][3]=-b->position.y;
	L[2][3]=-b->position.z;*/
	
	
	if(b->parentBoneIndex==-1)
	{
		b->relativeForm=L;
		
		return L;
	}
	else
	{
		glm::mat4 result=L; //setBoneToFrame(pmxInfo.bones[b->parentBoneIndex],bf)*
	
		//cout<<"Result:"<<endl;
		cout<<result[0][0]<<" "<<result[0][1]<<" "<<result[0][2]<<" "<<result[0][3]<<endl
		<<result[1][0]<<" "<<result[1][1]<<" "<<result[1][2]<<" "<<result[1][3]<<endl
		<<result[2][0]<<" "<<result[2][1]<<" "<<result[2][2]<<" "<<result[2][3]<<endl
		<<result[3][0]<<" "<<result[3][1]<<" "<<result[3][2]<<" "<<result[3][3]<<endl<<endl;
		
		b->relativeForm=L;
	
		return result;
	
		//bone->relativeForm=result;
	}
}

BoneFrame *getBoneFrame(int frame, string boneName)
{
	for(int i=0; i<vmdInfo.boneCount; ++i)
	{
		if(vmdInfo.boneFrames[i].name==boneName && vmdInfo.boneFrames[i].frame==frame) return &vmdInfo.boneFrames[i];
	}
	
	//cerr<<"No bone found: "<<boneName<<endl;
	
	return NULL;
}

void setModelToKeyFrame(glm::mat4 Bone[], GLuint &shaderProgram, PMXInfo &pmxInfo, VMDInfo &vmdInfo)
{
	//*****************LOOK HERE*****************/
	
	cout<<"START BONE READ"<<endl;
	
	cout<<"Bone vector size: "<<pmxInfo.bone_continuing_datasets<<endl;
	
	PMXBone *parentBone=pmxInfo.parentBone;
	int targetFrame=0;
	
	int boneFrameCount=0;
	for(int i=0; i<vmdInfo.boneCount; ++i)
	{
		if(vmdInfo.boneFrames[i].frame>boneFrameCount) boneFrameCount=vmdInfo.boneFrames[i].frame;
	}
	cout<<"VMD Frame Count: "<<boneFrameCount<<endl;
	

	glm::mat4 bindPose[pmxInfo.bone_continuing_datasets];
	glm::mat4 invBindPose[pmxInfo.bone_continuing_datasets];
	
	glm::mat4 skinMatrix[pmxInfo.bone_continuing_datasets];
	//for(size_t i=0; i<pmxInfo.bone_continuing_datasets; i++)
	{
		int i=32;
		PMXBone *b = pmxInfo.bones[i];
		
		BoneFrame *bf=getBoneFrame(targetFrame,b->name);
		
		if(b->parentBoneIndex==-1)
		{
			b->absoluteForm = b->relativeForm;
		}
		else
		{
			
			BoneFrame *bf=getBoneFrame(targetFrame,b->name);
			if(bf!=NULL)
			{
				
				glm::mat4 aniMatrix=glm::toMat4(bf->quaternion);
				aniMatrix[3][0]=bf->position.x;
				aniMatrix[3][1]=bf->position.y;
				aniMatrix[3][2]=bf->position.z;
		
				glm::mat4 invAniMatrix=glm::inverse(aniMatrix);
				b->absoluteForm = pmxInfo.bones[b->parentBoneIndex]->absoluteForm * (setBoneToFrame(b,*bf));
				//b->absoluteForm = 
			}
			else
			{
				b->absoluteForm = pmxInfo.bones[b->parentBoneIndex]->absoluteForm * b->relativeForm;
			}
		}
		
		//bindPose[i] = (pmxInfo.bones[i]->absoluteForm);
		//invBindPose[i] = glm::inverse(bindPose[i]);
		
		/*cout<<invBindPose[i][0][0]<<" "<<invBindPose[i][0][1]<<" "<<invBindPose[i][0][2]<<" "<<invBindPose[i][0][3]<<endl
		<<invBindPose[i][1][0]<<" "<<invBindPose[i][1][1]<<" "<<invBindPose[i][1][2]<<" "<<invBindPose[i][1][3]<<endl
		<<invBindPose[i][2][0]<<" "<<invBindPose[i][2][1]<<" "<<invBindPose[i][2][2]<<" "<<invBindPose[i][2][3]<<endl
		<<invBindPose[i][3][0]<<" "<<invBindPose[i][3][1]<<" "<<invBindPose[i][3][2]<<" "<<invBindPose[i][3][3]<<endl<<endl;*/
		
		if(bf!=NULL)
		{
		
			Bone[i] = b->absoluteForm * invBindPose[i];
		}
		else
		{
			Bone[i] = b->absoluteForm * invBindPose[i];
		}
		
		cout<<"Final Bone: "<<endl;
		cout<<Bone[i][0][0]<<" "<<Bone[i][0][1]<<" "<<Bone[i][0][2]<<" "<<Bone[i][0][3]<<endl
		<<Bone[i][1][0]<<" "<<Bone[i][1][1]<<" "<<Bone[i][1][2]<<" "<<Bone[i][1][3]<<endl
		<<Bone[i][2][0]<<" "<<Bone[i][2][1]<<" "<<Bone[i][2][2]<<" "<<Bone[i][2][3]<<endl
		<<Bone[i][3][0]<<" "<<Bone[i][3][1]<<" "<<Bone[i][3][2]<<" "<<Bone[i][3][3]<<endl<<endl;
	}
	
	GLuint Bones_loc=glGetUniformLocation(shaderProgram,"Bones");
	glUniformMatrix4fv(Bones_loc, pmxInfo.bone_continuing_datasets, GL_FALSE, &Bone[0][0][0]);
	
	
	
	cout<<"END BONE READ"<<endl;
	//exit(EXIT_SUCCESS);
	//Transform the vertices
}

GLuint shaderProgram;
void init(PMXInfo &pmxInfo, VMDInfo &vmdInfo)
{
	shaderProgram=loadShaders();
	
	//Load Textures
	loadTextures(pmxInfo);
	
	GLushort vertexIndices[pmxInfo.face_continuing_datasets][3];
	for(int i=0; i<pmxInfo.faces.size(); ++i) //faces.size()
	{
		//cout<<i<<endl;
		vertexIndices[i][0]=pmxInfo.faces[i]->points[0];
		vertexIndices[i][1]=pmxInfo.faces[i]->points[1];
		vertexIndices[i][2]=pmxInfo.faces[i]->points[2];
	}
	
	GLfloat vertexData[pmxInfo.vertex_continuing_datasets][18];
	for(int i=0; i<pmxInfo.vertex_continuing_datasets; ++i)
	{
		vertexData[i][0]=pmxInfo.vertices[i]->pos[0];
		vertexData[i][1]=pmxInfo.vertices[i]->pos[1];
		vertexData[i][2]=pmxInfo.vertices[i]->pos[2];
		vertexData[i][3]=1.0;
		
		vertexData[i][4]=pmxInfo.vertices[i]->UV[0];
		vertexData[i][5]=pmxInfo.vertices[i]->UV[1];
		
		vertexData[i][6]=pmxInfo.vertices[i]->normal[0];
		vertexData[i][7]=pmxInfo.vertices[i]->normal[1];
		vertexData[i][8]=pmxInfo.vertices[i]->normal[2];
		
		vertexData[i][9]=pmxInfo.vertices[i]->boneIndex1;
		vertexData[i][10]=pmxInfo.vertices[i]->boneIndex2;
		vertexData[i][11]=pmxInfo.vertices[i]->boneIndex3;
		vertexData[i][12]=pmxInfo.vertices[i]->boneIndex4;
		
		vertexData[i][13]=pmxInfo.vertices[i]->weight1;
		vertexData[i][14]=pmxInfo.vertices[i]->weight2;
		vertexData[i][15]=pmxInfo.vertices[i]->weight3;
		vertexData[i][16]=pmxInfo.vertices[i]->weight4;
		
		vertexData[i][16]=pmxInfo.vertices[i]->weight_transform_formula;
		
		//cout<<pmxInfo.vertices[i]->boneIndex1<<" "<<pmxInfo.vertices[i]->pos[1]<<" "<<pmxInfo.bones[pmxInfo.vertices[i]->boneIndex1]->position.y<<endl;
		
		/*if(pmxInfo.vertices[i]->weight_transform_formula==3)
		{
			cerr<<"SDEF not supported yet"<<endl;
			exit(EXIT_FAILURE);
		}*/
		
		//cout<<pmxInfo.vertices[i]->weight1<<" "<<pmxInfo.vertices[i]->weight2<<" "<<pmxInfo.vertices[i]->weight3<<" "<<pmxInfo.vertices[i]->weight4<<endl;
		
		//cout<<pmxInfo.vertices[i]->boneIndex1<<" "<<pmxInfo.vertices[i]->boneIndex2<<" "<<pmxInfo.vertices[i]->boneIndex3<<" "<<pmxInfo.vertices[i]->boneIndex4<<endl;
		
		//cout<<pmxInfo.vertices[i]->pos[0]<<" "<<pmxInfo.vertices[i]->pos[1]<<" "<<pmxInfo.vertices[i]->pos[2]<<endl;
	}
	
	//Generate all Buffers
	glGenBuffers(NumBuffers,Buffers);
	
	//init Element Buffer Object
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Buffers[VertexIndexBuffer]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(vertexIndices), vertexIndices, GL_STATIC_DRAW);
	
	
	//Init Vertex Array Buffer Object
	glGenVertexArrays(NumVAOs, VAOs);
	glBindVertexArray(VAOs[Vertices]);
	
	glBindBuffer(GL_ARRAY_BUFFER, Buffers[VertexArrayBuffer]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);
	
	//Intialize Vertex Attribute Pointer	
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, sizeof(vertexData[0]), BUFFER_OFFSET(0)); //4=number of components updated per vertex
	glEnableVertexAttribArray(vPosition);
	
	glVertexAttribPointer(vUV, 2, GL_FLOAT, GL_FALSE, sizeof(vertexData[0]), BUFFER_OFFSET(sizeof(float)*4));
	glEnableVertexAttribArray(vUV);
	
	glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, sizeof(vertexData[0]), BUFFER_OFFSET(sizeof(float)*6));
	glEnableVertexAttribArray(vNormal);
	
	glVertexAttribPointer(vBoneIndices, 4, GL_FLOAT, GL_FALSE, sizeof(vertexData[0]), BUFFER_OFFSET(sizeof(float)*9));
	glEnableVertexAttribArray(vBoneIndices);
	
	glVertexAttribPointer(vBoneWeights, 4, GL_FLOAT, GL_FALSE, sizeof(vertexData[0]), BUFFER_OFFSET(sizeof(float)*13));
	glEnableVertexAttribArray(vBoneWeights);
	
	glVertexAttribPointer(vWeightFormula, 1, GL_FLOAT, GL_FALSE, sizeof(vertexData[0]), BUFFER_OFFSET(sizeof(float)*17));
	glEnableVertexAttribArray(vWeightFormula);

	MVP_loc = glGetUniformLocation(shaderProgram, "MVP");
	boneOffset_loc=glGetUniformLocation(shaderProgram, "boneOffset");
	boneQuaternion_loc=glGetUniformLocation(shaderProgram, "boneQuaternion");
	
	glm::mat4 *Bone=new glm::mat4[pmxInfo.bone_continuing_datasets]();
	setModelToKeyFrame(Bone,shaderProgram, pmxInfo, vmdInfo);
	
	//Init Bone Data
	GLfloat boneData[pmxInfo.bone_continuing_datasets][4];
	for(int i=0; i<pmxInfo.bone_continuing_datasets; ++i)
	{
		boneData[i][0]=Bone[i][0][3];
		boneData[i][1]=Bone[i][1][3];
		boneData[i][2]=Bone[i][2][3];
		boneData[i][3]=1.0;
		//cout<<Bone[i][0][3]<<" "<<Bone[i][1][3]<<" "<<Bone[i][2][3]<<endl;
	}
	glBindVertexArray(VAOs[BoneVertices]);
	glBindBuffer(GL_ARRAY_BUFFER, Buffers[BoneBuffer]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(boneData), boneData, GL_STATIC_DRAW);
	
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, NO_STRIDE, BUFFER_OFFSET(0)); //4=number of components updated per vertex
	glEnableVertexAttribArray(vPosition);
	
	
    
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


glm::vec3 modelTranslate(0.0f,0.0f,0.0f);

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
	
	//Setup Uniform variables for Bone data
	
	
	
	
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

void drawSkeleton()
{
	//WARNING: Code broken at this point
	
	/*glBindVertexArray(VAOs[BoneVertices]);
	glBindBuffer(GL_ARRAY_BUFFER, Buffers[BoneBuffer]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	MVP_loc = glGetUniformLocation(shaderProgram, "MVP");
	
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
	
	glDrawArrays(GL_POINTS, 0, GL_FLOAT);*/
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	drawModel(pmxInfo);
	
	//drawSkeleton();
	
	glutSwapBuffers();
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
			glutPostRedisplay();
			break;
		case GLUT_KEY_DOWN :
			modelTranslate.y+=1.0;
			glutPostRedisplay();
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
			
			glutPostRedisplay();
		break;
		
		case 4: //mouse scroll down
			radius+=1.0f;
			t=radius*cos(zenith);
			cameraPosition.y=radius*sin(zenith);
	
			cameraPosition.x=t*cos(theta);
			cameraPosition.z=t*sin(theta);
			
			glutPostRedisplay();
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
	
	glutPostRedisplay();
}



int main(int argc, char** argv)
{
	pmxInfo=readPMX("gumiv3/GUMI_V3.pmx");
	vmdInfo=readVMD();
	
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	//glutInitWindowSize(1024, 576);
	glutInitWindowSize(1920, 1080);
	glutInitContextVersion(4,2);
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
	
	return 0;
}
