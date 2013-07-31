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
#include "src/shader.h"
#include "src/pmxvLogger.h"

#define NO_STRIDE 0

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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

glm::mat4 *bindPose;
glm::mat4 *invBindPose;

glm::mat4 *Bone;
glm::quat boneQuat;
glm::quat boneQuat2;

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


void quaternionToMat4(glm::mat4 &m, glm::vec3 &p, glm::quat &q)
{
	//cout<<"Vector: "<<p[0]<<" "<<p[1]<<" "<<p[2]<<endl;
	
	m[3][0] = p[0];
	m[3][1] = p[1];
	m[3][2] = p[2];
	
	float xx=q.x * q.x;
	float xy=q.x * q.y;
	float xz=q.x * q.z;
	float xw=q.x * q.w;
	
	float yy=q.y * q.y;
	float yz=q.y * q.z;
	float yw=q.y * q.w;
	
	float zz=q.z * q.z;
	float zw=q.z * q.w;

	m[0][0]  = 1 - 2 * ( yy + zz );
	m[0][1]  =     2 * ( xy - zw );
	m[0][2]  =     2 * ( xz + yw );

	m[1][0]  =     2 * ( xy + zw );
	m[1][1]  = 1 - 2 * ( xx + zz );
	m[1][2]  =     2 * ( yz - xw );

	m[2][0]  =     2 * ( xz - yw );
	m[2][1]  =     2 * ( yz + xw );
	m[2][2] = 1 - 2 * ( xx + yy );

	
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
	//cout<<b->position.x<<" "<<b->position.y<<" "<<b->position.z<<endl;
	cout<<bf.position.x<<" "<<bf.position.y<<" "<<bf.position.z<<endl;
	cout<<bf.quaternion.x<<" "<<bf.quaternion.y<<" "<<bf.quaternion.z<<" "<<bf.quaternion.w<<endl<<endl;

	glm::mat4 aniMatrix=glm::toMat4(bf.quaternion);
	//quaternionToMat4(aniMatrix,bf.position,bf.quaternion);//=
	
	aniMatrix[3][0]=b->position.x+bf.position.x;
	aniMatrix[3][1]=b->position.y+bf.position.y;
	aniMatrix[3][2]=b->position.z+bf.position.z;
	
	/*aniMatrix[0][3]=b->position.x;
	aniMatrix[1][3]=b->position.y;
	aniMatrix[2][3]=b->position.z;*/
	
	glm::mat4 invAniMatrix=glm::inverse(aniMatrix);
	
	glm::mat4 blankMatrix;
	
	/*L[0][3]=-b->position.x;
	L[1][3]=-b->position.y;
	L[2][3]=-b->position.z;*/
	
	
	if(b->parentBoneIndex==-1)
	{		
		return b->relativeForm;
	}
	else
	{
		glm::mat4 L=b->absoluteForm; //setBoneToFrame(pmxInfo.bones[bone->parentBoneIndex],bf);
		glm::mat4 result=L; //setBoneToFrame(pmxInfo.bones[b->parentBoneIndex],bf)
		
		setBoneToFrame(pmxInfo.bones[b->parentBoneIndex],bf);
	
		cout<<"aniMatrix:"<<endl;
		cout<<result[0][0]<<" "<<result[0][1]<<" "<<result[0][2]<<" "<<result[0][3]<<endl
		<<result[1][0]<<" "<<result[1][1]<<" "<<result[1][2]<<" "<<result[1][3]<<endl
		<<result[2][0]<<" "<<result[2][1]<<" "<<result[2][2]<<" "<<result[2][3]<<endl
		<<result[3][0]<<" "<<result[3][1]<<" "<<result[3][2]<<" "<<result[3][3]<<endl<<endl;
		
		//b->relativeForm=result;
	
		return result;
	
		//bone->relativeForm=result;
	}
}

BoneFrame *getBoneFrame(int frame, string boneName)
{
	stringstream ss;
	
	for(int i=0; i<vmdInfo.boneCount; ++i)
	{		
		//cout<<"VMD NAME: "<<vmdInfo.boneFrames[i].name<<endl;
		if(vmdInfo.boneFrames[i].name==boneName && vmdInfo.boneFrames[i].frame==frame)
		{
			return &vmdInfo.boneFrames[i];
		}
	}
	
	
	//cerr<<"No bone found: "<<boneName<<endl;
	
	return NULL;
}

glm::mat4 invToParent(PMXBone *b, int index)
{
	if(b->parentBoneIndex==-1)
	{
		return invBindPose[index];
	}
	return  invToParent(pmxInfo.bones[b->parentBoneIndex],b->parentBoneIndex) * invBindPose[index];
}

glm::mat4 boneFix(int &targetFrame,PMXBone *b, int index)
{
	glm::mat4 blank;
	BoneFrame *bf=getBoneFrame(targetFrame,b->name);
	
	if(bf!=NULL)
	{
	
		glm::mat4 aniMatrix=glm::toMat4(bf->quaternion);
		aniMatrix[3][0]+=b->position.x + bf->position.x;
		aniMatrix[3][1]+=b->position.y + bf->position.y;
		aniMatrix[3][2]+=b->position.z + bf->position.z;
		
		
		if(b->parentBoneIndex==-1)
		{
			return aniMatrix * b->relativeForm;
		}
	
		return aniMatrix * boneFix(targetFrame,pmxInfo.bones[b->parentBoneIndex],b->parentBoneIndex);
	}
	else
	{
		return b->absoluteForm;
	}
}

glm::quat quatMultiplication(glm::quat &left, glm::quat right)
{   
	glm::quat ans;
	double   d1, d2, d3, d4;
       
	d1   =  left.w * right.w;
	d2   = -left.x * right.x;
	d3   = -left.y * right.y;
	d4   = -left.z * right.z;
	ans.w = d1+ d2+ d3+ d4;
       
	d1   =  left.w * right.x;
	d2   =  right.w * left.x;
	d3   =  left.y * right.z;
	d4   = -left.z * right.y;
	ans.x =  d1+ d2+ d3+ d4;
       
	d1   =  left.w * right.y;
	d2   =  right.w * left.y;
	d3   =  left.z * right.x;
	d4   = -left.x * right.z;
	ans.y =  d1+ d2+ d3+ d4;
       
	d1   =  left.w * right.z;
	d2   =  right.w * left.z;
	d3   =  left.x * right.y;
	d4   = -left.y * right.x;
	ans.z =  d1+ d2+ d3+ d4; 
                    
	return ans;
}

glm::vec3 qtransform(glm::quat q, glm::vec3 v)
{
	glm::vec3 quat3;
	quat3.x=q.x;
	quat3.y=q.y;
	quat3.z=q.z;
	glm::vec3 crossProduct=glm::cross(glm::cross(v, quat3) - q.w*v, quat3);
	return v + 2.0f * crossProduct;
}

glm::mat4 setRotate(glm::quat q, glm::vec3 centre)
{
	double sqw = q.w*q.w;
	double sqx = q.x*q.x;
	double sqy = q.y*q.y;
	double sqz = q.z*q.z;
	glm::mat4 m;
	m[0][0] = sqx - sqy - sqz + sqw; // since sqw + sqx + sqy + sqz =1
	m[1][1] = -sqx + sqy - sqz + sqw;
	m[2][2] = -sqx - sqy + sqz + sqw;
   
	double tmp1 = q.x*q.y;
	double tmp2 = q.z*q.w;
	m[0][1] = 2.0 * (tmp1 + tmp2);
	m[1][0] = 2.0 * (tmp1 - tmp2);
   
	tmp1 = q.x*q.z;
	tmp2 = q.y*q.w;
	m[0][2] = 2.0 * (tmp1 - tmp2);
	m[2][0] = 2.0 * (tmp1 + tmp2);
   
	tmp1 = q.y*q.z;
	tmp2 = q.x*q.w;
	m[1][2] = 2.0 * (tmp1 + tmp2);
	m[2][1] = 2.0 * (tmp1 - tmp2);
   
	double a1 = centre.x;
	double a2 = centre.y;
	double a3 = centre.z;
	
	m[0][3] = a1 - a1 * m[0][0] - a2 * m[0][1] - a3 * m[0][2];
	m[1][3] = a2 - a1 * m[1][0] - a2 * m[1][1] - a3 * m[1][2];
	m[2][3] = a3 - a1 * m[2][0] - a2 * m[2][1] - a3 * m[2][2];
	m[3][0] = m[3][1] = m[3][2] = 0.0;
	m[3][3] = 1.0;
	
	return m;
}

#define CONST_TEST_FRAME_NUMBER 0
void setModelToKeyFrame(glm::mat4 Bone[], GLuint &shaderProgram, PMXInfo &pmxInfo, VMDInfo &vmdInfo)
{

	int targetFrame = CONST_TEST_FRAME_NUMBER;
	glm::mat4 aniMatrix;
	
	//stringstream ss;
	//ss << "Bone matrices:" << endl;
	
	// root bone
	PMXBone   *b  = pmxInfo.bones[0];
	BoneFrame *bf = getBoneFrame(targetFrame, b->name);

	b->absoluteForm = b->relativeForm;		
	Bone[0] = b->absoluteForm * invBindPose[0];
	
	// other bones
	for (size_t i = 1; i < pmxInfo.bone_continuing_datasets; i++)
	{
		//int i=32;
		b  = pmxInfo.bones[i];
		bf = getBoneFrame(targetFrame, b->name);

		if (bf != NULL)
		{
			b->absoluteForm = (glm::translate(b->position + bf->position) * glm::toMat4(bf->quaternion)) * pmxInfo.bones[b->parentBoneIndex]->absoluteForm;
			b->finalRotation = bf->quaternion * pmxInfo.bones[b->parentBoneIndex]->finalRotation;
			cout<<b->name<<endl;
			cout<<"Quat: "<<bf->quaternion.x<<" "<<bf->quaternion.y<<" "<<bf->quaternion.z<<" "<<bf->quaternion.w<<endl;
			cout<<"ParentQuat: "<<pmxInfo.bones[b->parentBoneIndex]->finalRotation.x<<" "<<pmxInfo.bones[b->parentBoneIndex]->finalRotation.y<<" "<<pmxInfo.bones[b->parentBoneIndex]->finalRotation.z<<" "<<pmxInfo.bones[b->parentBoneIndex]->finalRotation.w<<endl;
			cout<<"FinalRotation: "<<b->finalRotation.x<<" "<<b->finalRotation.y<<" "<<b->finalRotation.z<<" "<<b->finalRotation.w<<endl<<endl;
			
			glm::mat4 quatMatrix=glm::toMat4(bf->quaternion);
			stringstream ss;
			ss<<"Quat Matrix: "<<endl
			<<quatMatrix[0][0]<<" "<<quatMatrix[1][0]<<" "<<quatMatrix[2][0]<<" "<<quatMatrix[3][0]<<endl
			<<quatMatrix[0][1]<<" "<<quatMatrix[1][1]<<" "<<quatMatrix[2][1]<<" "<<quatMatrix[3][1]<<endl
			<<quatMatrix[0][2]<<" "<<quatMatrix[1][2]<<" "<<quatMatrix[2][2]<<" "<<quatMatrix[3][2]<<endl
			<<quatMatrix[0][3]<<" "<<quatMatrix[1][3]<<" "<<quatMatrix[2][3]<<" "<<quatMatrix[3][3]<<endl;
			
			log(ss.str());
		}
		else
		{
			b->absoluteForm = b->relativeForm * pmxInfo.bones[b->parentBoneIndex]->absoluteForm;
			b->finalRotation = pmxInfo.bones[b->parentBoneIndex]->finalRotation;
		}

		Bone[i] = b->absoluteForm * invBindPose[i];
	}
	
	GLuint Bones_loc = glGetUniformLocation(shaderProgram, "Bones");
	glUniformMatrix4fv(Bones_loc, pmxInfo.bone_continuing_datasets, GL_FALSE, (const GLfloat*)Bone);
}

void setModelToKeyFrameSingleQuat(glm::mat4 Bone[], GLuint &shaderProgram, PMXInfo &pmxInfo, VMDInfo &vmdInfo)
{

	int targetFrame = CONST_TEST_FRAME_NUMBER;
	glm::mat4 aniMatrix;
	
	// root bone
	PMXBone   *b  = pmxInfo.bones[0];
	BoneFrame *bf = getBoneFrame(targetFrame, b->name);

	b->absoluteForm = b->relativeForm;
	if(bf!=NULL)
	{
		b->finalRotation = bf->quaternion;
		
		glm::vec4 homoPosition=glm::vec4(b->position + bf->position,1.0); //position in homogeneous coordinates
		glm::vec4 localPosition=glm::rotate(b->finalRotation,homoPosition);
		
		b->relativeForm[3][0]=localPosition[0];
		b->relativeForm[3][1]=localPosition[1];
		b->relativeForm[3][2]=localPosition[2];
		b->relativeForm[3][3]=localPosition[3];
	}
	Bone[0] = b->absoluteForm * invBindPose[0];
	
	
	
	// other bones
	for (size_t i = 1; i < pmxInfo.bone_continuing_datasets; i++)
	{
		b  = pmxInfo.bones[i];
		PMXBone *parent = pmxInfo.bones[b->parentBoneIndex];
		bf = getBoneFrame(targetFrame, b->name);
		
		if(bf!=NULL && i>=46 && i<=48)
		{
			cout<<i<<" "<<b->name<<" "<<bf->name<<endl;
			b->finalRotation = glm::normalize(bf->quaternion * parent->finalRotation);
			
			cout<<b->rotationPossible<<" "<<b->movementPossible<<" "<<b->localAxis<<" "<<endl;
			
			glm::vec4 homoPosition=glm::vec4(b->position + bf->position,1.0); //position in homogeneous coordinates
			
			cout<<"homoPosition: "<<homoPosition[0]<<" "<<homoPosition[1]<<" "<<homoPosition[2]<<" "<<homoPosition[3]<<endl<<endl;
			
			glm::vec4 localPosition=glm::rotate(glm::normalize(parent->finalRotation),homoPosition);
			
			b->relativeForm=glm::toMat4(bf->quaternion);
			b->relativeForm[3][0]=localPosition[0];
			b->relativeForm[3][1]=localPosition[1];
			b->relativeForm[3][2]=localPosition[2];
			b->relativeForm[3][3]=localPosition[3];			
			
			cout<<"relativeForm: "<<endl
			<<b->relativeForm[0][0]<<" "<<b->relativeForm[1][0]<<" "<<b->relativeForm[2][0]<<" "<<b->relativeForm[3][0]<<endl
			<<b->relativeForm[0][1]<<" "<<b->relativeForm[1][1]<<" "<<b->relativeForm[2][1]<<" "<<b->relativeForm[3][1]<<endl
			<<b->relativeForm[0][2]<<" "<<b->relativeForm[1][2]<<" "<<b->relativeForm[2][2]<<" "<<b->relativeForm[3][2]<<endl
			<<b->relativeForm[0][3]<<" "<<b->relativeForm[1][3]<<" "<<b->relativeForm[2][3]<<" "<<b->relativeForm[3][3]<<endl<<endl;
			
			cout<<"absoluteForm: "<<endl
			<<b->absoluteForm[0][0]<<" "<<b->absoluteForm[1][0]<<" "<<b->absoluteForm[2][0]<<" "<<b->absoluteForm[3][0]<<endl
			<<b->absoluteForm[0][1]<<" "<<b->absoluteForm[1][1]<<" "<<b->absoluteForm[2][1]<<" "<<b->absoluteForm[3][1]<<endl
			<<b->absoluteForm[0][2]<<" "<<b->absoluteForm[1][2]<<" "<<b->absoluteForm[2][2]<<" "<<b->absoluteForm[3][2]<<endl
			<<b->absoluteForm[0][3]<<" "<<b->absoluteForm[1][3]<<" "<<b->absoluteForm[2][3]<<" "<<b->absoluteForm[3][3]<<endl<<endl;
		}
		else
		{
			b->finalRotation = glm::normalize(parent->finalRotation);
			
			glm::vec4 homoPosition=glm::vec4(b->position,1.0); //position in homogeneous coordinates
			glm::vec4 parentOriginalLocalPosition=glm::vec4(b->relativeForm[3][0],b->relativeForm[3][1],b->relativeForm[3][2],b->relativeForm[3][3]);
			glm::vec4 localPosition=glm::rotate(b->finalRotation,homoPosition);
			
			b->relativeForm[3][0]=localPosition[0];
			b->relativeForm[3][1]=localPosition[1];
			b->relativeForm[3][2]=localPosition[2];
			b->relativeForm[3][3]=localPosition[3];
		}
		
		b->absoluteForm = b->relativeForm * parent->absoluteForm;
		Bone[i] = b->absoluteForm * invBindPose[i];
			
	}
	
	/*GLuint Bones_loc = glGetUniformLocation(shaderProgram, "Bones");
	glUniformMatrix4fv(Bones_loc, pmxInfo.bone_continuing_datasets, GL_FALSE, (const GLfloat*)Bone);*/
}

/*void setModelToKeyFrame2(glm::mat4 Bone[], GLuint &shaderProgram, PMXInfo &pmxInfo, VMDInfo &vmdInfo)
{
	cout<<"START BONE READ"<<endl;
	
	int targetFrame=0;
	
	//glm::mat4 skinMatrix[pmxInfo.bone_continuing_datasets];
	for(size_t i=0; i<pmxInfo.bone_continuing_datasets; i++)
	{
		PMXBone *b=pmxInfo.bones[i];
		BoneFrame *bf=getBoneFrame(targetFrame,b->name);
		
		glm::vec3 originalBonePos=glm::vec3(bindPose[i][3][0],bindPose[i][3][1],bindPose[i][3][2]);
		glm::vec3 originalParentBonePos=glm::vec3(bindPose[b->parentBoneIndex][3][0],bindPose[b->parentBoneIndex][3][1],bindPose[b->parentBoneIndex][3][2]);
		
		glm::vec3 finalPosition;
		glm::quat finalRotation;
		if(bf!=NULL)
		{
			if(b->parentBoneIndex==-1)
			{
				finalPosition=b->position+bf->position;
				finalRotation=bf->quaternion;
			}
			else
			{
				PMXBone *parentBone=pmxInfo.bones[b->parentBoneIndex];
				
				finalPosition=qtransform(parentBone->finalRotation, (b->position - parentBone->position + bf->position)) + parentBone->finalPosition;
				finalRotation=quatMultiplication(parentBone->finalRotation, bf->quaternion);
			}
		}
		else
		{
			if(b->parentBoneIndex==-1)
			{
				finalPosition=b->position;
			}
			else
			{
				PMXBone *parentBone=pmxInfo.bones[b->parentBoneIndex];
				
				finalPosition=qtransform(parentBone->finalRotation, (b->position - parentBone->position)) + parentBone->finalPosition;
				finalRotation=parentBone->finalRotation;
			}
		}
		
		b->absoluteForm=glm::toMat4(finalRotation);
		b->absoluteForm[3][0]=finalPosition.x;
		b->absoluteForm[3][1]=finalPosition.y;
		b->absoluteForm[3][2]=finalPosition.z;
		
			
		b->finalRotation=finalRotation;
		b->finalPosition=finalPosition;
			
		Bone[i] = b->absoluteForm;
	}
}*/

/*void setModelToKeyFrame3(glm::mat4 Bone[], GLuint &shaderProgram, PMXInfo &pmxInfo, VMDInfo &vmdInfo)
{
	//individualBoneMotions is translation/rotation of each bone from it's original position
	//boneMotions is total position/rotation of each bone
	//boneMotions is an array like [{p, r, tainted}]
	//tainted flag is used to avoid re-creating vec3/quat4
	
	
	individualBoneMotions = []
	boneMotions = []
	originalBonePositions = []
	parentBones = []
	constrainedBones = []

	for(size_t i=0; i<pmxInfo.bone_continuing_datasets; i++)
	{
		PMXBone *b=pmxInfo.bones[i];
		BoneFrame *bf=getBoneFrame(targetFrame,b->name);
		
		
	}


    # calculate positions/rotations of bones other than IK
    getBoneMotion(i) for i in [0...model.bones.length]

    rotations1 = model.rotations1
    rotations2 = model.rotations2
    positions1 = model.positions1
    positions2 = model.positions2

    length = model.vertices.length
    for i in [0...length]
      vertex = model.vertices[i]
      motion1 = boneMotions[vertex.bone_num1]
      motion2 = boneMotions[vertex.bone_num2]
      rot1 = motion1.r
      pos1 = motion1.p
      rot2 = motion2.r
      pos2 = motion2.p
      rotations1[i * 4 ] = rot1[0]
      rotations1[i * 4 + 1] = rot1[1]
      rotations1[i * 4 + 2] = rot1[2]
      rotations1[i * 4 + 3] = rot1[3]
      rotations2[i * 4 ] = rot2[0]
      rotations2[i * 4 + 1] = rot2[1]
      rotations2[i * 4 + 2] = rot2[2]
      rotations2[i * 4 + 3] = rot2[3]
      positions1[i * 3 ] = pos1[0]
      positions1[i * 3 + 1] = pos1[1]
      positions1[i * 3 + 2] = pos1[2]
      positions2[i * 3 ] = pos2[0]
      positions2[i * 3 + 1] = pos2[1]
      positions2[i * 3 + 2] = pos2[2]

    @gl.bindBuffer(@gl.ARRAY_BUFFER, @vbuffers.aBone1Rotation.buffer)
    @gl.bufferData(@gl.ARRAY_BUFFER, rotations1, @gl.STATIC_DRAW)
    @gl.bindBuffer(@gl.ARRAY_BUFFER, @vbuffers.aBone2Rotation.buffer)
    @gl.bufferData(@gl.ARRAY_BUFFER, rotations2, @gl.STATIC_DRAW)
    @gl.bindBuffer(@gl.ARRAY_BUFFER, @vbuffers.aBone1Position.buffer)
    @gl.bufferData(@gl.ARRAY_BUFFER, positions1, @gl.STATIC_DRAW)
    @gl.bindBuffer(@gl.ARRAY_BUFFER, @vbuffers.aBone2Position.buffer)
    @gl.bufferData(@gl.ARRAY_BUFFER, positions2, @gl.STATIC_DRAW)
    @gl.bindBuffer(@gl.ARRAY_BUFFER, null)
    return
}*/

	

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
	loadTextures(pmxInfo);
	
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
	
	
	bindPose=new glm::mat4[pmxInfo.bone_continuing_datasets];
	invBindPose=new glm::mat4[pmxInfo.bone_continuing_datasets];
	for(int i=0; i<pmxInfo.bone_continuing_datasets; ++i)
	{
		PMXBone *b = pmxInfo.bones[i];
		
		if(b->parentBoneIndex==-1)
		{
			b->absoluteForm = b->relativeForm;
		}
		else
		{
			b->absoluteForm = pmxInfo.bones[b->parentBoneIndex]->absoluteForm * b->relativeForm;
		}
		
		bindPose[i] = (pmxInfo.bones[i]->absoluteForm);
		invBindPose[i] = glm::inverse(bindPose[i]);
	}
	Bone=new glm::mat4[pmxInfo.bone_continuing_datasets]();
	//setModelToKeyFrame(Bone,shaderProgram, pmxInfo, vmdInfo);
	
	GLuint Bones_loc=glGetUniformLocation(shaderProgram,"Bones");
	glUniformMatrix4fv(Bones_loc, pmxInfo.bone_continuing_datasets, GL_FALSE, &Bone[0][0][0]);
	
	//free(bindPose);
	//free(invBindPose);
	//free(Bone);
    
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
	
	//Setup Uniform variables for Bone data
	GLuint Bones_loc = glGetUniformLocation(shaderProgram, "Bones");
	glUniformMatrix4fv(Bones_loc, pmxInfo.bone_continuing_datasets, GL_FALSE, (const GLfloat*)Bone);
	
	
	
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

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	drawModel(pmxInfo);
	
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
	
	else if(key=='w') boneQuat.x+=.01f;
	else if(key=='s') boneQuat.x-=.01f;
	else if(key=='e') boneQuat.y+=.01f;
	else if(key=='d') boneQuat.y-=.01f;
	else if(key=='r') boneQuat.z+=.01f;
	else if(key=='f') boneQuat.z-=.01f;
	else if(key=='t') boneQuat.w+=.01f;
	else if(key=='g') boneQuat.w-=.01f;
	
	else if(key=='y') boneQuat2.x+=.01f;
	else if(key=='h') boneQuat2.x-=.01f;
	else if(key=='u') boneQuat2.y+=.01f;
	else if(key=='j') boneQuat2.y-=.01f;
	else if(key=='i') boneQuat2.z+=.01f;
	else if(key=='k') boneQuat2.z-=.01f;
	else if(key=='o') boneQuat2.w+=.01f;
	else if(key=='l') boneQuat2.w-=.01f;
	
	
	setModelToKeyFrameSingleQuat(Bone, shaderProgram, pmxInfo, vmdInfo);
	
	glutPostRedisplay();
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
