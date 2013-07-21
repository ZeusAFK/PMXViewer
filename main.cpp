//g++ *.cpp *.cc -o pmx -L./ -lglut -lGLU -lGLEW -lGL -ggdb -lSOIL -std=c++11
#include <iostream>
#include <fstream>

#include "vgl.h"
//#include "vmath.h"

#include <GL/freeglut_ext.h>

#include "SOIL/SOIL.h"

#include <algorithm>
#include <bitset>
#include <sstream>
#include <vector>
#include <stdexcept>

#include <iomanip>

#include <locale>
#include <string>

#include <iconv.h>

#include <string.h>

#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include "pmx.h"
#include "vmd.h"

#include "Converter.h"

#define TYPE_UTF16 0
#define TYPE_UTF8 1

#define VERTEX_DEBUG false
#define MATERIAL_DEBUG true

#define NO_STRIDE 0

using namespace std;


#define MAX_BUF 2048
string sjisToUTF8(char *sjis)
{
	char	inbuf[MAX_BUF+1] = { 0 };
	char	outbuf[MAX_BUF+1] = { 0 };
		
	char	*in = inbuf;
	char	*out = outbuf;
	size_t	in_size = (size_t)MAX_BUF;
	size_t	out_size = (size_t)MAX_BUF;
	iconv_t ic = iconv_open("UTF-8", "SJIS");

	memcpy( in, sjis, sizeof(sjis) );

	iconv( ic, &in, &in_size, &out, &out_size );
	iconv_close(ic);

	//int ret = memcmp( outbuf, utf8, sizeof(utf8) );
	//printf(ret == 0 ? "success\n" : "failed\n" );
	//printf("%s\n", outbuf );
	
	//cout<<outbuf<<endl;
		
	stringstream ss;
	ss<<outbuf;
	return ss.str();
}

PMXInfo pmxInfo;
VMDInfo vmdInfo;

void printDebugInfo()
{
	cout<<pmxInfo.header_str<<"ver "<<*pmxInfo.ver<<endl;
	
	cout<<"line size: "<<(int)pmxInfo.line_size<<endl;
	cout<<"Unicode Type(0-UTF-16, 1-UTF-8): "<<pmxInfo.unicode_type<<endl;
	cout<<"Extra UV count: "<<(int)pmxInfo.extraUVCount<<endl;
	cout<<"Vertex Index Size: "<<(int)pmxInfo.vertexIndexSize<<endl;
	cout<<"Texture Index Size: "<<(int)pmxInfo.textureIndexSize<<endl;
	cout<<"Material Index Size: "<<(int)pmxInfo.materialIndexSize<<endl;
	cout<<"Bone Index Size: "<<(int)pmxInfo.boneIndexSize<<endl;
	cout<<"Morph Index Size: "<<(int)pmxInfo.morphIndexSize<<endl;
	cout<<"Rig Index Size: "<<(int)pmxInfo.rigidBodyIndexSize<<endl;
	
	cout<<endl<<endl;
	
	cout<<pmxInfo.modelName<<endl;
	cout<<pmxInfo.modelNameEnglish<<endl;
	cout<<endl;
	
	cout<<"Number of Continuing Vertex Datasets: "<<pmxInfo.vertex_continuing_datasets<<endl;
	
	for(int i=0; i<pmxInfo.vertex_continuing_datasets; ++i)
	{
		PMXVertex *vertex=pmxInfo.vertices[i];
		
		if(VERTEX_DEBUG) cout<<"[Vertex "<<i<<"]"<<endl;
		
		if(VERTEX_DEBUG) cout<<"Position: "<<vertex->pos[0]<<" "<<vertex->pos[1]<<" "<<vertex->pos[2]<<endl<<endl;
		if(VERTEX_DEBUG) cout<<"Normal Vector: "<<vertex->normal[0]<<" "<<vertex->normal[1]<<" "<<vertex->normal[2]<<endl<<endl;
		if(VERTEX_DEBUG) cout<<vertex->UV[0]<<" "<<vertex->UV[1]<<" "<<endl<<endl;
		
		if(VERTEX_DEBUG) cout<<"Weight transform formula(0-BDEF1, 1-BDEF2, 2-BDEF4, 3-SDEF): "<<(int)vertex->weight_transform_formula<<endl;
		
		if(vertex->weight_transform_formula==WEIGHT_FORMULA_BDEF1)
		{
			if(VERTEX_DEBUG) cout<<"boneIndex1: "<<vertex->boneIndex1<<endl;
		}
		else if(vertex->weight_transform_formula==WEIGHT_FORMULA_BDEF2)
		{
			
			if(VERTEX_DEBUG) cout<<"boneIndex1: "<<vertex->boneIndex1<<endl;
			if(VERTEX_DEBUG) cout<<"boneIndex2: "<<vertex->boneIndex2<<endl;
			
			if(VERTEX_DEBUG) cout<<"Weight1: "<<vertex->weight1<<endl;
			if(VERTEX_DEBUG) cout<<"Weight2: "<<vertex->weight2<<endl;
		}
		else if(vertex->weight_transform_formula==WEIGHT_FORMULA_BDEF4)
		{			
			if(VERTEX_DEBUG) cout<<"boneIndex1: "<<vertex->boneIndex1<<endl;
			if(VERTEX_DEBUG) cout<<"boneIndex1: "<<vertex->boneIndex2<<endl;
			
			if(VERTEX_DEBUG) cout<<"Weight1: "<<vertex->weight1<<endl;
			if(VERTEX_DEBUG) cout<<"Weight2: "<<vertex->weight2<<endl;
			if(VERTEX_DEBUG) cout<<"Weight3: "<<vertex->weight3<<endl;
			if(VERTEX_DEBUG) cout<<"Weight4: "<<vertex->weight4<<endl;
		}
		else if(vertex->weight_transform_formula==WEIGHT_FORMULA_SDEF)
		{			
			if(VERTEX_DEBUG) cout<<"boneIndex1: "<<vertex->boneIndex1<<endl;
			if(VERTEX_DEBUG) cout<<"boneIndex2: "<<vertex->boneIndex2<<endl;
			
			if(VERTEX_DEBUG) cout<<"Weight1: "<<vertex->weight1<<endl;
			if(VERTEX_DEBUG) cout<<"Weight2: "<<vertex->weight2<<endl;
			
			if(VERTEX_DEBUG) cout<<"C: "<<vertex->C[0]<<" "<<vertex->C[1]<<" "<<vertex->C[2]<<endl;
			
			if(VERTEX_DEBUG) cout<<"R0: "<<vertex->R0[0]<<" "<<vertex->R0[1]<<" "<<vertex->R0[2]<<endl;
			
			if(VERTEX_DEBUG) cout<<"R1: "<<vertex->R1[0]<<" "<<vertex->R1[1]<<" "<<vertex->R1[2]<<endl;
		}
		if(VERTEX_DEBUG) cout<<"edgeScale: "<<vertex->edgeScale<<endl<<endl;
	}
	
	cout<<"Number of Continuing Face Datasets: "<<pmxInfo.face_continuing_datasets<<endl;
	/*for(int i=0; i<pmxInfo.face_continuing_datasets; ++i)
	{
		PMXFace *face=pmxInfo.faces[i];
		cout<<"Vertex Index1: "<<face->points[0]<<endl;
		cout<<"Vertex Index2: "<<face->points[1]<<endl;
		cout<<"Vertex Index3: "<<face->points[2]<<endl;
	}*/

	cout<<"Number of Continuing Texture Datasets: "<<pmxInfo.texture_continuing_datasets<<endl;
	
	cout<<"Number of Continuing Material Datasets: "<<pmxInfo.material_continuing_datasets<<endl;
	for(int i=0; i<pmxInfo.material_continuing_datasets; ++i)
	{		
		if(MATERIAL_DEBUG) cout<<"[Material "<<i<<"]"<<endl;
		
		PMXMaterial *material=pmxInfo.materials[i];
		
		if(MATERIAL_DEBUG)
		{
			cout<<"Diffuse: "<<endl;
			cout<<"R: "<<material->diffuse[0]<<endl;
			cout<<"G: "<<material->diffuse[1]<<endl;
			cout<<"B: "<<material->diffuse[2]<<endl;
			cout<<"A: "<<material->diffuse[3]<<endl<<endl;
		}
		
		if(MATERIAL_DEBUG)
		{
			cout<<"Specular: "<<endl;
			cout<<"R: "<<material->specular[0]<<endl;
			cout<<"G: "<<material->specular[1]<<endl;
			cout<<"B: "<<material->specular[2]<<endl;
		}
		
		if(MATERIAL_DEBUG) cout<<"Specular Coefficient: "<<material->shininess<<endl<<endl;
		
		if(MATERIAL_DEBUG)
		{
			cout<<"Edge Color: "<<endl;
			cout<<"R: "<<material->edgeColor[0]<<endl;
			cout<<"G: "<<material->edgeColor[1]<<endl;
			cout<<"B: "<<material->edgeColor[2]<<endl;
			cout<<"A: "<<material->edgeColor[3]<<endl;
		}
		
		if(MATERIAL_DEBUG)
		{
			cout<<"Ambient: "<<endl;
			cout<<"R: "<<material->ambient[0]<<endl;
			cout<<"G: "<<material->ambient[1]<<endl;
			cout<<"B: "<<material->ambient[2]<<endl;
		}
		
		if(MATERIAL_DEBUG)
		{
			cout<<"Draw Both Sides: "<<material->drawBothSides<<endl;
			cout<<"Draw Ground Shadow: "<<material->drawGroundShadow<<endl;
			cout<<"Draw To Self Shadow Map: "<<material->drawToSelfShadowMap<<endl;
			cout<<"Draw Self Shadow: "<<material->drawSelfShadow<<endl;
			cout<<"Draw Edges: "<<material->drawEdges<<endl<<endl;
		}
		
		if(MATERIAL_DEBUG) cout<<"Edge Size: "<<material->edgeSize<<endl<<endl;
		if(MATERIAL_DEBUG) cout<<"Sphere Mode: "<<(int)material->sphereMode<<endl;
		if(MATERIAL_DEBUG) cout<<"Share Toon: "<<(int)material->shareToon<<endl;
		if(MATERIAL_DEBUG) cout<<"Number of Indices that have this Material: "<<material->hasFaceNum<<endl;
		
		if(MATERIAL_DEBUG)
		{
			cout<<"Texture Index: "<<material->textureIndex<<endl;
			cout<<"Toon Texture Index: "<<material->toonTextureIndex<<endl;
			cout<<"Sphere Index: "<<material->sphereIndex<<endl<<endl;
		}
	}
	
	cout<<"Bone continuing datasets: "<<pmxInfo.bone_continuing_datasets<<endl;
	/*for(int i=0; i<pmxInfo.bone_continuing_datasets; ++i)
	{
		PMXBone *bone=pmxInfo.bones[i];
		
		cerr<<endl<<"[Bone "<<i<<"]"<<endl;
		cerr<<"Bone Name: "<<bone->name<<endl;
		//cerr<<"Bone Name English: "<<bone->nameEng<<endl;
		cerr<<"Position: "<<bone->position.x<<" "<<bone->position.y<<" "<<bone->position.z<<endl;
		cerr<<"Parent Bone Index: "<<bone->parentBoneIndex<<endl;
		cerr<<"Transformation Class/Level: "<<bone->transformationLevel<<endl;
		//cerr<<bitflag_ss.str()<<endl;
		
		cerr<<"Connection Display Method: "<<bone->connectionDisplayMethod<<endl;
		cerr<<"Rotation Possible: "<<bone->rotationPossible<<endl;
		cerr<<"Movement Possible: "<<bone->movementPossible<<endl;
		cerr<<"Show: "<<bone->show<<endl;
		cerr<<"Control Possible: "<<bone->controlPossible<<endl;
		cerr<<"IK: "<<bone->IK<<endl;
		
		cerr<<"Rotation Allowed: "<<bone->rotationAllowed<<endl;
		cerr<<"Movement Allowed: "<<bone->movementAllowed<<endl;
		cerr<<"Axis Fixed: "<<bone->axisFixed<<endl;
		cerr<<"Local Axis: "<<bone->localAxis<<endl;
		cerr<<"Transform After Physics: "<<bone->transformAfterPhysics<<endl;
		cerr<<"External Parent Transform: "<<bone->externalParentTransform<<endl<<endl;
		
		if(bone->connectionDisplayMethod==0) //0: Display with Coordinate Offset
		{
			cerr<<"Bone Coordinate Offset: "<<bone->coordinateOffset.x<<" "<<bone->coordinateOffset.y<<" "<<bone->coordinateOffset.z<<endl;
		}
		else //1: Display with Bone
		{
			cerr<<"Connection Bone Index: "<<bone->connectionBoneIndex<<endl;
		}
		
		if(bone->rotationAllowed || bone->movementAllowed)
		{
			cerr<<"Allowed Parent Bone Index: "<<bone->allowedParentBoneIndex<<endl;
			cerr<<"Allowed Rate: "<<bone->allowRate<<endl;
		}
		
		if(bone->axisFixed)
		{
			cerr<<"Axis Direction Vector: "<<bone->axisDirectionVector.x<<" "<<bone->axisDirectionVector.y<<" "<<bone->axisDirectionVector.z<<endl;
		}
		
		if(bone->localAxis)
		{
			cerr<<"X Axis Bone Coordinate Offset: "<<bone->XAxisDirectionVector.x<<" "<<bone->XAxisDirectionVector.y<<" "<<bone->XAxisDirectionVector.z<<endl;
			cerr<<"Z Axis Bone Coordinate Offset: "<<bone->ZAxisDirectionVector.x<<" "<<bone->ZAxisDirectionVector.y<<" "<<bone->ZAxisDirectionVector.z<<endl;
		}
		
		if(bone->externalParentTransform)
		{
			cerr<<"Key Value: "<<bone->keyValue<<endl;
		}
		
		if(bone->IK)
		{
			cerr<<"IK Target Bone Index: "<<bone->IKTargetBoneIndex<<endl;
			cerr<<"IK Loop Count: "<<bone->IKLoopCount<<endl;
			cerr<<"IK Loop Radian Angle: "<<bone->IKLoopRadianAngle<<endl;
			
			cerr<<"IK Links: "<<bone->IKLinkNum<<endl;
			
			for(int j=0; j<bone->IKLinkNum; ++j)
			{
				cerr<<endl<<"[Link "<<j<<"]"<<endl;
				cerr<<"IKLink Bone Index: "<<bone->IKLinks[j]->linkBoneIndex<<endl;
				cerr<<"IKLink Angle Limit: "<<bone->IKLinks[j]->angleLimit<<endl;
				if(bone->IKLinks[j]->angleLimit)
				{
					cerr<<"IKLink Lower Limit: "<<bone->IKLinks[j]->lowerLimit.x<<" "<<bone->IKLinks[j]->lowerLimit.y<<" "<<bone->IKLinks[j]->lowerLimit.z<<endl;
					cerr<<"IKLink Upper Limit: "<<bone->IKLinks[j]->upperLimit.x<<" "<<bone->IKLinks[j]->upperLimit.y<<" "<<bone->IKLinks[j]->upperLimit.z<<endl;
				}
			}
		}
	}*/
}

void getPMXText(ifstream &miku, string &result, bool debug=false)
{
	uint32_t text_size;
	miku.read((char*)&text_size,4);
	
	//cout<<"Text Size: "<<text_size<<endl;
	
	if(pmxInfo.unicode_type==TYPE_UTF16)
	{
		char16_t c16[text_size];
		u16string s16;
		for(int i=0; i<text_size; ++i)
		{
			c16[i]=0;
		}
	
		int count=0;
		for(int i=0; i<text_size/2; ++i)
		{
			miku.read((char*)&c16[i],2);
			count+=2;
		}
		if(count!=text_size)
		{
			cerr<<"ERROR: number of bytes pulled does not equal number of bytes for text string"<<endl;
			exit(EXIT_FAILURE);
		}
		
		//if(debug)cout<<"Raw Text: "<<data_byte<<endl;
		result = to_u8string(c16);
		if(debug)cout<<"Result: "<<result<<endl;
		
		//exit(EXIT_SUCCESS);
	}
	else
	{
		cerr<<"ERROR: UTF-8 text being used, please write code to support it"<<endl;
	}
}

PMXBone *getChildBone(unsigned &id, PMXBone *parentBone)
{
	if(id==0) return parentBone;
	for(int i=0; i<parentBone->children.size(); ++i)
	{
		//cout<<"Searching child "<<i<<endl;
		if(parentBone->children[i]->id==id)
		{
			//cout<<"Bone found (direct child): "<<i<<endl;
			return parentBone->children[i];
		}
		else
		{
			PMXBone *bone=getChildBone(id,parentBone->children[i]);
			if(bone!=NULL)
			{
				//cout<<"Bone found (indirect child): "<<bone->id<<endl;
				return bone;
			}
		}
	}
	//cerr<<"FATAL ERROR: Bone could not be found: "<<id<<endl;
	//exit(EXIT_FAILURE);
	return NULL;
}

PMXBone *getChildBone(string &name, PMXBone *parentBone)
{
	if(name==parentBone->name) return parentBone;
	for(int i=0; i<parentBone->children.size(); ++i)
	{
		//cout<<"Searching child "<<i<<endl;
		if(parentBone->children[i]->name==name)
		{
			//cout<<"Bone found (direct child): "<<i<<endl;
			return parentBone->children[i];
		}
		else
		{
			PMXBone *bone=getChildBone(name,parentBone->children[i]);
			if(bone!=NULL)
			{
				//cout<<"Bone found (indirect child): "<<bone->id<<endl;
				return bone;
			}
		}
	}
	//cerr<<"FATAL ERROR: Bone could not be found: "<<id<<endl;
	//exit(EXIT_FAILURE);
	return NULL;
}

void readPMX()
{
	//ifstream miku("apimiku/Appearance Miku.pmx", ios::in | ios::binary);
	ifstream miku("gumiv3/GUMI_V3.pmx", ios::in | ios::binary);
	if(!miku) cerr<<"ERROR: PMX file could not be opened"<<endl;
	
	//***Extract header info***
	miku.read(pmxInfo.header_str,4);
	miku.read((char*)pmxInfo.ver,4);
	if(*pmxInfo.ver!=2.0)
	{
		cerr<<"Error: Only version 2.0 of the PMX file format is supported!"<<endl;
	}
	
	
	miku.read((char*)&pmxInfo.line_size,1);
	miku.read((char*)&pmxInfo.unicode_type,1);
	miku.read((char*)&pmxInfo.extraUVCount,1);
	miku.read((char*)&pmxInfo.vertexIndexSize,1);
	miku.read((char*)&pmxInfo.textureIndexSize,1);
	miku.read((char*)&pmxInfo.materialIndexSize,1);
	miku.read((char*)&pmxInfo.boneIndexSize,1);
	miku.read((char*)&pmxInfo.morphIndexSize,1);
	miku.read((char*)&pmxInfo.rigidBodyIndexSize,1);
	
	
	
	//***Pull model name, comment info***
	getPMXText(miku, pmxInfo.modelName);
	getPMXText(miku, pmxInfo.modelNameEnglish);
	getPMXText(miku, pmxInfo.comment);
	getPMXText(miku, pmxInfo.commentEnglish);
	
	//***Pull Vertex Info***
	miku.read((char*)&pmxInfo.vertex_continuing_datasets,4);
	
	//exit(EXIT_SUCCESS);
	cout<<"Loading vertices...";
	for(int i=0; i<pmxInfo.vertex_continuing_datasets; ++i)
	{
		
		PMXVertex *vertex=new PMXVertex();
		
		//***Pull position info***
		float *x=(float*) malloc(sizeof(float));
		float *y=(float*) malloc(sizeof(float));
		float *z=(float*) malloc(sizeof(float));
	
		miku.read((char*)x,4);
		miku.read((char*)y,4);
		miku.read((char*)z,4);
	
		vertex->pos=glm::vec3(*x,*y,*z);
	
		//***Pull normal vector info***
		miku.read((char*)x,4);
		miku.read((char*)y,4);
		miku.read((char*)z,4);
	
		vertex->normal=glm::vec3(*x,*y,*z);
	
		//***Pull unit vector info***
		miku.read((char*)x,4);
		miku.read((char*)y,4);
	
		vertex->UV=glm::vec2(*x,*y);
	
		if(pmxInfo.extraUVCount>0)
		{
			cerr<<"ERROR: please add support for extra UVs"<<endl;
			exit(EXIT_FAILURE);
		}
	
	
		miku.read((char*)&vertex->weight_transform_formula,1);
	
		if(vertex->weight_transform_formula==WEIGHT_FORMULA_BDEF1)
		{
			miku.read((char*)&vertex->boneIndex1,pmxInfo.boneIndexSize);
		}
		else if(vertex->weight_transform_formula==WEIGHT_FORMULA_BDEF2)
		{
			miku.read((char*)&vertex->boneIndex1,pmxInfo.boneIndexSize);
			miku.read((char*)&vertex->boneIndex2,pmxInfo.boneIndexSize);
			
			miku.read((char*)&vertex->weight1,4);
			vertex->weight2=1.0-vertex->weight1; //For BDEF2: weight of bone2=1.0-weight1
		}
		else if(vertex->weight_transform_formula==WEIGHT_FORMULA_BDEF4)
		{			
			miku.read((char*)&vertex->boneIndex1,pmxInfo.boneIndexSize);
			miku.read((char*)&vertex->boneIndex2,pmxInfo.boneIndexSize);
			miku.read((char*)&vertex->boneIndex3,pmxInfo.boneIndexSize);
			miku.read((char*)&vertex->boneIndex4,pmxInfo.boneIndexSize);
			
			miku.read((char*)&vertex->weight1,4);
			miku.read((char*)&vertex->weight2,4);
			miku.read((char*)&vertex->weight3,4);
			miku.read((char*)&vertex->weight4,4);
		}
		else if(vertex->weight_transform_formula==WEIGHT_FORMULA_SDEF)
		{			
			miku.read((char*)&vertex->boneIndex1,pmxInfo.boneIndexSize);
			miku.read((char*)&vertex->boneIndex2,pmxInfo.boneIndexSize);
			
			miku.read((char*)&vertex->weight1,4);
			vertex->weight2=1.0-vertex->weight1; //For BDEF2: weight of bone2=1.0-weight1
			
			float *x=(float*) malloc(sizeof(float));
			float *y=(float*) malloc(sizeof(float));
			float *z=(float*) malloc(sizeof(float));
			
			miku.read((char*)x,4);
			miku.read((char*)y,4);
			miku.read((char*)z,4);
			
			vertex->C=glm::vec3(*x,*y,*z);
			
			miku.read((char*)x,4);
			miku.read((char*)y,4);
			miku.read((char*)z,4);
			
			vertex->R0=glm::vec3(*x,*y,*z);
			
			miku.read((char*)x,4);
			miku.read((char*)y,4);
			miku.read((char*)z,4);
			
			vertex->R1=glm::vec3(*x,*y,*z);
		}
		else if(vertex->weight_transform_formula>3)
		{
			cerr<<"FATAL ERROR: Invalid bone data"<<endl;
			exit(EXIT_FAILURE);
		}
		else
		{
			cerr<<"ERROR: bone structure not supported yet"<<endl;
			exit(EXIT_FAILURE);
		}
	
		miku.read((char*)&vertex->edgeScale,4);
		
		pmxInfo.vertices.push_back(vertex);
	}
	cout<<"done."<<endl;
	
	//***Pull Face Info***
	miku.read((char*)&pmxInfo.face_continuing_datasets,4);
	
	cout<<"Loading faces...";
	for(int i=0; i<pmxInfo.face_continuing_datasets/3; i++)
	{
		PMXFace *face=new PMXFace();
	
		miku.read((char*)&face->points[0],pmxInfo.vertexIndexSize);
		miku.read((char*)&face->points[1],pmxInfo.vertexIndexSize);
		miku.read((char*)&face->points[2],pmxInfo.vertexIndexSize);		
		
		pmxInfo.faces.push_back(face);
	}
	cout<<"done."<<endl;
	
	//***Pull Texture Info***	
	miku.read((char*)&pmxInfo.texture_continuing_datasets,4);
	pmxInfo.texturePaths=new string[pmxInfo.texture_continuing_datasets];
	
	cout<<"Loading textures...";
	for(int i=0; i<pmxInfo.texture_continuing_datasets; ++i)
	{
		getPMXText(miku, pmxInfo.texturePaths[i]);
	}
	cout<<"done."<<endl;
	
	//***Pull Material Info***
	miku.read((char*)&pmxInfo.material_continuing_datasets,4);
	
	cout<<"Loading materials...";
	for(int i=0; i<pmxInfo.material_continuing_datasets; ++i)
	{		
		PMXMaterial *material=new PMXMaterial();
		
		//***Pull Material Names***
		getPMXText(miku, material->name);
		getPMXText(miku, material->nameEng);
		
		float r;
		float g;
		float b;
		float a;
		
		//***Pull Diffuse Color***
		miku.read((char*)&r,4);
		miku.read((char*)&g,4);
		miku.read((char*)&b,4);
		miku.read((char*)&a,4);
		
		material->diffuse=glm::vec4(r,g,b,a);
		
		//***Pull Specular Color***
		miku.read((char*)&r,4);
		miku.read((char*)&g,4);
		miku.read((char*)&b,4);
		
		material->specular=glm::vec3(r,g,b);
		
		//***Pull Specular Coefficient***
		miku.read((char*)&material->shininess,4);
		
		//***Pull Ambient Color***
		miku.read((char*)&r,4);
		miku.read((char*)&g,4);
		miku.read((char*)&b,4);
		
		material->ambient=glm::vec3(r,g,b);
		
		//***Pull Bitflag***
		char bitflag_char[1];
		miku.read(bitflag_char,1);
		bitset<8> bitflag(*bitflag_char);
		r=bitflag.size()-1; //here r is used for reversing the bit sequence
		
		stringstream bitflag_ss;
		bitflag_ss<<bitflag[r]<<bitflag[r-1]<<bitflag[r-2]<<bitflag[r-3]<<bitflag[r-4]<<bitflag[r-5]<<bitflag[r-6]<<bitflag[r-7]<<endl;
		
		material->drawBothSides=bitflag[0];
		material->drawGroundShadow=bitflag[1];
		material->drawToSelfShadowMap=bitflag[2];
		material->drawSelfShadow=bitflag[3];
		material->drawEdges=bitflag[4];

		
		//***Pull Edge Color***
		miku.read((char*)&r,4);
		miku.read((char*)&g,4);
		miku.read((char*)&b,4);
		miku.read((char*)&a,4);
		
		material->edgeColor=glm::vec4(r,g,b,a);
		
		//***Pull Edge Size***
		miku.read((char*)&material->edgeSize,4);
				
		//***Pull Texture and Sphere Indices***
		miku.read((char*)&material->textureIndex,(int)pmxInfo.textureIndexSize);
		miku.read((char*)&material->sphereIndex,(int)pmxInfo.textureIndexSize);
		
		//***Pull sphereMode and shareToon flag***
		miku.read((char*)&material->sphereMode,1);
		miku.read((char*)&material->shareToon,1);
		
		if(material->shareToon)
		{
			cerr<<"No support for shared toon yet, please code in support"<<endl;
			exit(EXIT_FAILURE);
		}
		else
		{
			unsigned int toonTextureIndex;//=(char*) malloc(sizeof(pmxInfo.textureIndexSize));
			miku.read((char*)&material->toonTextureIndex,(int)pmxInfo.textureIndexSize);
		}
		
		//***Pull Material memo (notes)***
		getPMXText(miku,material->memo);
		
		//***Pull number of faces that use the material***
		miku.read((char*)&material->hasFaceNum,4);		
		
		pmxInfo.materials.push_back(material);
	}
	cout<<"done."<<endl;
	
	//***Pull Bone Info***
	miku.read((char*)&pmxInfo.bone_continuing_datasets,4);
	
	cout<<"Loading bones...";
	for(int i=0; i<pmxInfo.bone_continuing_datasets; ++i)
	{
		PMXBone *bone=new PMXBone();
		
		getPMXText(miku, bone->name);
		getPMXText(miku, bone->nameEng);
		
		//***Pull Position***
		miku.read((char*)&bone->position.x,4);
		miku.read((char*)&bone->position.y,4);
		miku.read((char*)&bone->position.z,4);
		
		//***Pull Parent Index***
		miku.read((char*)&bone->parentBoneIndex,(int)pmxInfo.boneIndexSize);
		
		//***Pull Transformation Level***/
		miku.read((char*)&bone->transformationLevel,4);
		
		//***Pull Bitflag***
		char bitflag_char[1];
		miku.read(bitflag_char,1);
		bitset<8> bitflag(*bitflag_char);
		int r=bitflag.size()-1;
		
		stringstream bitflag_ss;
		bitflag_ss<<bitflag[r]<<bitflag[r-1]<<bitflag[r-2]<<bitflag[r-3]<<bitflag[r-4]<<bitflag[r-5]<<bitflag[r-6]<<bitflag[r-7]<<endl;
		//<<bitflag[r-8]<<bitflag[r-9]<<bitflag[r-10]<<bitflag[r-11]<<bitflag[r-12]<<bitflag[r-13]<<bitflag[r-14]<<bitflag[r-15]<<endl;
		
		bone->connectionDisplayMethod=bitflag[0];
		bone->rotationPossible=bitflag[1];
		bone->movementPossible=bitflag[2];
		bone->show=bitflag[3];
		bone->controlPossible=bitflag[4];
		bone->IK=bitflag[5];
		
		miku.read(bitflag_char,1);
		bitset<8> bitflag2(*bitflag_char);
	
		bone->rotationAllowed=bitflag2[0];
		bone->movementAllowed=bitflag2[1];
		bone->axisFixed=bitflag2[2];
		bone->localAxis=bitflag2[3];
		bone->transformAfterPhysics=bitflag2[4];
		bone->externalParentTransform=bitflag2[5];
		
		if(bone->connectionDisplayMethod==0) //0: Display with Coordinate Offset
		{
			miku.read((char*)&bone->coordinateOffset.x,4);
			miku.read((char*)&bone->coordinateOffset.y,4);
			miku.read((char*)&bone->coordinateOffset.z,4);
		}
		else //1: Display with Bone
		{
			miku.read((char*)&bone->connectionBoneIndex,(int)pmxInfo.boneIndexSize);
		}
		
		if(bone->rotationAllowed || bone->movementAllowed)
		{
			miku.read((char*)&bone->allowedParentBoneIndex,(int)pmxInfo.boneIndexSize);
			miku.read((char*)&bone->allowRate,4);
		}
		
		if(bone->axisFixed)
		{
			miku.read((char*)&bone->axisDirectionVector.x,4);
			miku.read((char*)&bone->axisDirectionVector.y,4);
			miku.read((char*)&bone->axisDirectionVector.z,4);
		}
		
		if(bone->localAxis)
		{
			miku.read((char*)&bone->XAxisDirectionVector.x,4);
			miku.read((char*)&bone->XAxisDirectionVector.y,4);
			miku.read((char*)&bone->XAxisDirectionVector.z,4);
			
			miku.read((char*)&bone->ZAxisDirectionVector.x,4);
			miku.read((char*)&bone->ZAxisDirectionVector.y,4);
			miku.read((char*)&bone->ZAxisDirectionVector.z,4);
		}
		
		if(bone->externalParentTransform)
		{
			miku.read((char*)&bone->keyValue,4);
		}
		
		if(bone->IK)
		{
			miku.read((char*)&bone->IKTargetBoneIndex,(int)pmxInfo.boneIndexSize);
			miku.read((char*)&bone->IKLoopCount,4);
			miku.read((char*)&bone->IKLoopRadianAngle,4);
			
			miku.read((char*)&bone->IKLinkNum,4);
			
			for(int j=0; j<bone->IKLinkNum; ++j)
			{
				PMXIKLink *link=new PMXIKLink();
				
				miku.read((char*)&link->linkBoneIndex,(int)pmxInfo.boneIndexSize);
				
				uint8_t tmpInt;
				miku.read((char*)&tmpInt,1);
				link->angleLimit=tmpInt;
				
				if(link->angleLimit)
				{
					miku.read((char*)&link->lowerLimit.x,4);
					miku.read((char*)&link->lowerLimit.y,4);
					miku.read((char*)&link->lowerLimit.z,4);
			
					miku.read((char*)&link->upperLimit.x,4);
					miku.read((char*)&link->upperLimit.y,4);
					miku.read((char*)&link->upperLimit.z,4);
				}
				
				bone->IKLinks.push_back(link);
			}
		}
		
		
		pmxInfo.bones.push_back(bone);
		
		bone->id=i;
		//cout<<bone->parentBoneIndex<<endl;
		if(bone->parentBoneIndex!=255 && bone->parentBoneIndex!=0)
		{
			//recursively search through all children
			//cout<<"recursive search"<<endl;
			PMXBone *parentBone=getChildBone(bone->parentBoneIndex,pmxInfo.parentBone);
			if(parentBone==NULL)
			{
				cerr<<"FATAL ERROR: parent Bone not found: "<<bone->parentBoneIndex<<endl;
				exit(EXIT_FAILURE);
			}
			parentBone->children.push_back(bone);
			//exit(EXIT_SUCCESS);
		}
		else if(bone->parentBoneIndex==255)
		{
			pmxInfo.parentBone=bone;
		}
		else //parentBoneIndex==0
		{
			//cout<<"pushing back directly into parent"<<endl;
			pmxInfo.parentBone->children.push_back(bone);
		}
	}
	cout<<"done."<<endl;
	
	//printBoneHierarchy(pmxInfo.parentBone);
	
	//***Pull Morph Info***
	miku.read((char*)&pmxInfo.morph_continuing_datasets,4);
	cout<<"Morph Continuing Datasets: "<<pmxInfo.morph_continuing_datasets<<endl;
	
	cout<<"Loading morphs...";
	for(int m=0; m<pmxInfo.morph_continuing_datasets; ++m)
	{
		//cout<<"[Morph "<<i<<endl;
		PMXMorph *morph=new PMXMorph;
		getPMXText(miku,morph->name);
		getPMXText(miku,morph->nameEng);
		
		//cout<<"Name: "<<morph->name<<endl;
		//cout<<"Name English: "<<morph->nameEng<<endl;
		
		miku.read((char*)&morph->controlPanel,1);
		miku.read((char*)&morph->type,1);
		miku.read((char*)&morph->morphOffsetNum,4);
		
		//cout<<"Control Panel: "<<(int)morph->controlPanel<<endl;
		//cout<<"Type: "<<(int)morph->type<<endl;
		//cout<<"Offset Number: "<<morph->morphOffsetNum<<endl;
		
		for(int i=0; i<morph->morphOffsetNum; ++i)
		{
			PMXMorphData *data;
			switch(morph->type)
			{
				case MORPH_TYPE_VERTEX:
				{
					PMXVertexMorph *vertexMorph=new PMXVertexMorph();
				
					miku.read((char*)&vertexMorph->vertexIndex,(int)pmxInfo.vertexIndexSize);
				
					miku.read((char*)&vertexMorph->coordinateOffset.x,4);
					miku.read((char*)&vertexMorph->coordinateOffset.y,4);
					miku.read((char*)&vertexMorph->coordinateOffset.z,4);
				
					//cout<<"Vertex Index: "<<vertexMorph->vertexIndex<<endl;
				
					//cerr<<"Coordinate Offset: "<<vertexMorph->coordinateOffset.x<<" "<<vertexMorph->coordinateOffset.y<<" "<<vertexMorph->coordinateOffset.z<<endl;
				
					data=vertexMorph;
				}
				break;
				
				case MORPH_TYPE_UV:
				case MORPH_TYPE_EXTRA_UV1:
				case MORPH_TYPE_EXTRA_UV2:
				case MORPH_TYPE_EXTRA_UV3:
				case MORPH_TYPE_EXTRA_UV4:
				{
					PMXUVMorph *UVMorph=new PMXUVMorph();
				
					miku.read((char*)&UVMorph->vertexIndex,(int)pmxInfo.vertexIndexSize);
				
					miku.read((char*)&UVMorph->UVOffsetAmount.x,4);
					miku.read((char*)&UVMorph->UVOffsetAmount.y,4);
					miku.read((char*)&UVMorph->UVOffsetAmount.z,4);
					miku.read((char*)&UVMorph->UVOffsetAmount.w,4);
				
					//cout<<"Vertex Index: "<<UVMorph->vertexIndex<<endl;
				
					//cerr<<"UV Offset Amount: "<<UVMorph->UVOffsetAmount.x<<" "<<UVMorph->UVOffsetAmount.y<<" "<<UVMorph->UVOffsetAmount.z<<endl;
				
					data=UVMorph;
				}
				break;
				
				case MORPH_TYPE_BONE:
				{
					PMXBoneMorph *boneMorph=new PMXBoneMorph();
				
					miku.read((char*)&boneMorph->boneIndex,(int)pmxInfo.boneIndexSize);
				
					miku.read((char*)&boneMorph->inertia.x,4);
					miku.read((char*)&boneMorph->inertia.y,4);
					miku.read((char*)&boneMorph->inertia.z,4);
					
					miku.read((char*)&boneMorph->rotationAmount.x,4);
					miku.read((char*)&boneMorph->rotationAmount.y,4);
					miku.read((char*)&boneMorph->rotationAmount.z,4);
					miku.read((char*)&boneMorph->rotationAmount.w,4);
				
					//cout<<"Vertex Index: "<<boneMorph->boneIndex<<endl;
				
					//cerr<<"Movement Amount: "<<boneMorph->inertia.x<<" "<<boneMorph->inertia.y<<" "<<boneMorph->inertia.z<<endl;
					//cerr<<"Rotation Amount: "<<boneMorph->rotationAmount.x<<" "<<boneMorph->rotationAmount.y<<" "<<boneMorph->rotationAmount.z<<" "<<boneMorph->rotationAmount.w<<endl;
				
					data=boneMorph;
				}
				break;
				
				case MORPH_TYPE_MATERIAL:
				{
					PMXMaterialMorph *materialMorph=new PMXMaterialMorph();
				
					miku.read((char*)&materialMorph->materialIndex,(int)pmxInfo.materialIndexSize);
					miku.read((char*)&materialMorph->offsetCalculationFormula,1);
				
					miku.read((char*)&materialMorph->diffuse.r,4);
					miku.read((char*)&materialMorph->diffuse.g,4);
					miku.read((char*)&materialMorph->diffuse.b,4);
					miku.read((char*)&materialMorph->diffuse.a,4);
					
					miku.read((char*)&materialMorph->specular.r,4);
					miku.read((char*)&materialMorph->specular.g,4);
					miku.read((char*)&materialMorph->specular.b,4);
					miku.read((char*)&materialMorph->shininess,4);
					
					miku.read((char*)&materialMorph->ambient.r,4);
					miku.read((char*)&materialMorph->ambient.g,4);
					miku.read((char*)&materialMorph->ambient.b,4);
					
					miku.read((char*)&materialMorph->edgeColor.r,4);
					miku.read((char*)&materialMorph->edgeColor.g,4);
					miku.read((char*)&materialMorph->edgeColor.b,4);
					miku.read((char*)&materialMorph->edgeColor.a,4);
					miku.read((char*)&materialMorph->edgeSize,4);
					
					miku.read((char*)&materialMorph->textureCoefficient.r,4);
					miku.read((char*)&materialMorph->textureCoefficient.g,4);
					miku.read((char*)&materialMorph->textureCoefficient.b,4);
					miku.read((char*)&materialMorph->textureCoefficient.a,4);
					
					miku.read((char*)&materialMorph->sphereTextureCoefficient.r,4);
					miku.read((char*)&materialMorph->sphereTextureCoefficient.g,4);
					miku.read((char*)&materialMorph->sphereTextureCoefficient.b,4);
					miku.read((char*)&materialMorph->sphereTextureCoefficient.a,4);
					
					miku.read((char*)&materialMorph->toonTextureCoefficient.r,4);
					miku.read((char*)&materialMorph->toonTextureCoefficient.g,4);
					miku.read((char*)&materialMorph->toonTextureCoefficient.b,4);
					miku.read((char*)&materialMorph->toonTextureCoefficient.a,4);
					
					data=materialMorph;
				}
				break;
				
				case MORPH_TYPE_GROUP:
				{
					cerr<<"No group morph support yet"<<endl;
					exit(EXIT_FAILURE);
				}
				break;
				
				default:
				{
					cerr<<"Unknown morph type or faulty data/reading"<<endl;
					exit(EXIT_FAILURE);
				}
			}
			
			morph->offsetData.push_back(data);
		}
		pmxInfo.morphs.push_back(morph);
	}
	cout<<"done"<<endl;
	
	//***Pull Display Frame Info***
	miku.read((char*)&pmxInfo.display_frame_continuing_datasets,4);
	//cout<<"Display Frame Continuing Datasets: "<<pmxInfo.display_frame_continuing_datasets<<endl;
	
	cout<<"Loading display frame...";
	for(int f=0; f<pmxInfo.display_frame_continuing_datasets; ++f)
	{
		PMXDisplayFrame *df=new PMXDisplayFrame();
		getPMXText(miku,df->name);
		getPMXText(miku,df->nameEng);
		
		uint8_t tmp;
		miku.read((char*)&tmp,1);
		df->specialFrameFlag=tmp;
		
		miku.read((char*)&df->elementsWithinFrame,4);
		
		for(int i=0; i<df->elementsWithinFrame; ++i)
		{
			PMXDisplayFrameElement *element=new PMXDisplayFrameElement();
			
			miku.read((char*)&tmp,1);
			element->target=tmp;
			
			if(element->target==0) //Bone
			{
				miku.read((char*)&element->index,pmxInfo.boneIndexSize);
			}
			else //==1; Morph
			{
				miku.read((char*)&element->index,pmxInfo.morphIndexSize);
			}
			df->elements.push_back(element);
		}
		pmxInfo.displayFrames.push_back(df);
	}
	cout<<"done"<<endl;
	
	//***Pull Rigid Body Info***
	miku.read((char*)&pmxInfo.rigid_body_continuing_datasets,4);
	cout<<"Rigid Body Continuing Datasets: "<<pmxInfo.rigid_body_continuing_datasets<<endl;
	
	cout<<"Loading rigid body...";
	for(int i=0; i<pmxInfo.rigid_body_continuing_datasets; ++i)
	{
		PMXRigidBody *rb=new PMXRigidBody();
		getPMXText(miku,rb->name);
		getPMXText(miku,rb->nameEng);
		
		miku.read((char*)&rb->relatedBoneIndex,pmxInfo.boneIndexSize);
		
		miku.read((char*)&rb->group,1);
		miku.read((char*)&rb->noCollisionGroupFlag,2);
		
		miku.read((char*)&rb->shape,1);
		
		miku.read((char*)&rb->size.x,4);
		miku.read((char*)&rb->size.y,4);
		miku.read((char*)&rb->size.z,4);
		
		miku.read((char*)&rb->position.x,4);
		miku.read((char*)&rb->position.y,4);
		miku.read((char*)&rb->position.z,4);
		
		miku.read((char*)&rb->rotation.x,4);
		miku.read((char*)&rb->rotation.y,4);
		miku.read((char*)&rb->rotation.z,4);
		
		miku.read((char*)&rb->mass,4);
		miku.read((char*)&rb->movementDecay,4);
		miku.read((char*)&rb->rotationDecay,4);
		miku.read((char*)&rb->elasticity,4);
		miku.read((char*)&rb->friction,4);
		
		miku.read((char*)&rb->physicsOperation,1);
	}
	cout<<"done"<<endl;
	
	//***Pull Rigid Body Info***
	miku.read((char*)&pmxInfo.joint_continuing_datasets,4);
	//cout<<"Joint Continuing Datasets: "<<pmxInfo.joint_continuing_datasets<<endl;
	for(int i=0; i<pmxInfo.joint_continuing_datasets; ++i)
	{
		PMXJoint *joint=new PMXJoint;
		getPMXText(miku,joint->name);
		getPMXText(miku,joint->nameEng);
		
		miku.read((char*)&joint->type,1);
		
		if(joint->type==0)
		{
			miku.read((char*)&joint->relatedRigidBodyIndexA,(int)pmxInfo.rigidBodyIndexSize);
			miku.read((char*)&joint->relatedRigidBodyIndexB,(int)pmxInfo.rigidBodyIndexSize);
			
			miku.read((char*)&joint->position.x,4);
			miku.read((char*)&joint->position.y,4);
			miku.read((char*)&joint->position.z,4);
			
			miku.read((char*)&joint->rotation.x,4);
			miku.read((char*)&joint->rotation.y,4);
			miku.read((char*)&joint->rotation.z,4);
			
			miku.read((char*)&joint->movementLowerLimit.x,4);
			miku.read((char*)&joint->movementLowerLimit.y,4);
			miku.read((char*)&joint->movementLowerLimit.z,4);
			
			miku.read((char*)&joint->movementUpperLimit.x,4);
			miku.read((char*)&joint->movementUpperLimit.y,4);
			miku.read((char*)&joint->movementUpperLimit.z,4);
			
			miku.read((char*)&joint->rotationLowerLimit.x,4);
			miku.read((char*)&joint->rotationLowerLimit.y,4);
			miku.read((char*)&joint->rotationLowerLimit.z,4);
			
			miku.read((char*)&joint->rotationUpperLimit.x,4);
			miku.read((char*)&joint->rotationUpperLimit.y,4);
			miku.read((char*)&joint->rotationUpperLimit.z,4);
			
			miku.read((char*)&joint->springMovementConstant.x,4);
			miku.read((char*)&joint->springMovementConstant.y,4);
			miku.read((char*)&joint->springMovementConstant.z,4);
			
			miku.read((char*)&joint->springRotationConstant.x,4);
			miku.read((char*)&joint->springRotationConstant.y,4);
			miku.read((char*)&joint->springRotationConstant.z,4);			
		}
		else
		{
			cerr<<"Unsupported PMX format version or file reading error"<<endl;
			exit(EXIT_FAILURE);
		}
	}
	cout<<"Loading joints...";
	
	cout<<"done"<<endl<<endl;
	
	/*cout<<"Miku Good: "<<miku.good()<<endl;
	
	char data_byte[1];
	miku.read(data_byte,1);
	
	stringstream byte_ss;
	bitset<8> byte(*data_byte);
	int r=byte.size()-1;
	byte_ss<<byte[r]<<byte[r-1]<<byte[r-2]<<byte[r-3]<<byte[r-4]<<byte[r-5]<<byte[r-6]<<byte[r-7]<<endl;
	
	cout<<byte_ss.str()<<endl;
	
	cerr<<"END PMX LOAD"<<endl;*/
	
	//printDebugInfo();
	
	//exit(EXIT_SUCCESS);
}

void readVMD(VMDInfo &vmdInfo)
{
	ifstream miku("motion/Masked bitcH.vmd", ios::in | ios::binary);
	if(!miku) cerr<<"ERROR: VMD file could not be opened"<<endl;
	
	//***Extract Header Info***
	miku.read((char*)&vmdInfo.headerStr,30);
	
	char modelName[20];
	miku.read((char*)&modelName,20);
	vmdInfo.modelName=sjisToUTF8(modelName);
	
	cout<<vmdInfo.headerStr<<endl;
	//cout<<vmdInfo.modelName<<endl;
	
	//***Extract Bone Info***
	miku.read((char*)&vmdInfo.boneCount,4);
	vmdInfo.boneFrames=new BoneFrame[vmdInfo.boneCount];
	
	//cout<<"Bone Count: "<<vmdInfo.boneCount<<endl;
	
	cout<<"Loading bone frames...";
	for(int i=0; i<vmdInfo.boneCount; ++i)
	{
		BoneFrame *f=&vmdInfo.boneFrames[i];
		
		char name[15];
		miku.read((char*)name,15);
		f->name=sjisToUTF8(name);
		
		miku.read((char*)&f->frame,4);
		
		miku.read((char*)&f->position.x,4);
		miku.read((char*)&f->position.y,4);
		miku.read((char*)&f->position.z,4);
		
		miku.read((char*)&f->quaternion.x,4);
		miku.read((char*)&f->quaternion.y,4);
		miku.read((char*)&f->quaternion.z,4);
		miku.read((char*)&f->quaternion.w,4);
		
		char bezier[64];
		miku.read((char*)&bezier,64);
		f->interpolationParameters=bezier;//sjisToUTF8(bezier);
		
		/*cout<<"Name: "<<f->name<<endl;
		cout<<"Frame Number: "<<f->frame<<endl<<endl;
		
		cout<<"Position: "<<f->position.x<<" "<<f->position.y<<" "<<f->position.z<<endl;
		cout<<"Quaternion: "<<f->quaternion.x<<" "<<f->quaternion.y<<" "<<f->quaternion.z<<endl;*/
	}
	cout<<"done."<<endl;
	
	//***Extract Morph Info***
	miku.read((char*)&vmdInfo.morphCount,4);
	vmdInfo.morphFrames=new MorphFrame[vmdInfo.morphCount];
	
	//cout<<"Morph Count: "<<vmdInfo.morphCount<<endl;
	
	cout<<"Loading morph frames...";
	for(int i=0; i<vmdInfo.morphCount; ++i)
	{
		MorphFrame *f=&vmdInfo.morphFrames[i];
		
		char name[15];
		miku.read((char*)&name,15);
		f->name=sjisToUTF8(name);
		
		miku.read((char*)&f->frame,4);
		miku.read((char*)&f->value,4);

		/*cout<<"Name: "<<f->name<<endl;
		cout<<"Frame Number: "<<f->frame<<endl;
		cout<<"Value: "<<f->value<<endl<<endl;*/
	}
	cout<<"done."<<endl;
	
	//***Extract Camera Info***
	miku.read((char*)&vmdInfo.cameraCount,4);
	vmdInfo.cameraFrames=new CameraFrame[vmdInfo.cameraCount];
	//cout<<"Camera Count: "<<vmdInfo.cameraCount<<endl;
	
	cout<<"Loading camera frames...";
	for(int i=0; i<vmdInfo.cameraCount; ++i)
	{
		CameraFrame *f=&vmdInfo.cameraFrames[i];
		
		miku.read((char*)&f->frame,4);
		
		miku.read((char*)&f->position.x,4);
		miku.read((char*)&f->position.y,4);
		miku.read((char*)&f->position.z,4);
		
		miku.read((char*)&f->rotation.x,4);
		miku.read((char*)&f->rotation.y,4);
		miku.read((char*)&f->rotation.z,4);
		
		char bezier[24];
		miku.read((char*)&bezier,24);
		f->interpolationParameters=sjisToUTF8(bezier);
	}
	cout<<"done."<<endl;
	
	//***Extract Light Info***
	miku.read((char*)&vmdInfo.lightCount,4);
	vmdInfo.lightFrames=new LightFrame[vmdInfo.lightCount];
	//cout<<"Light Count: "<<vmdInfo.lightCount<<endl;
	
	
	//***Extract Self Shadow Info***
	miku.read((char*)&vmdInfo.selfShadowCount,4);
	vmdInfo.selfShadowFrames=new SelfShadowFrame[vmdInfo.selfShadowCount];
	//cout<<"SelfShadow Count: "<<vmdInfo.selfShadowCount<<endl;
	
	//exit(EXIT_SUCCESS);
}

enum VAO_IDs { Vertices, BoneVertices, NumVAOs };
enum Buffer_IDs { VertexArrayBuffer, VertexIndexBuffer, BoneBuffer, NumBuffers };
enum Attrib_IDs { vPosition, vUV, vNormal, vBoneIndices, vBoneWeights, vWeightFormula };

GLuint VAOs[NumVAOs];
GLuint Buffers[NumBuffers];

const GLuint NumVertices=6;

GLuint MVP_loc;

GLuint boneOffset_loc;
GLuint boneQuaternion_loc;

static const GLchar* ReadShader(const char* filename)
{
#ifdef WIN32
	FILE* infile;
	fopen_s( &infile, filename, "rb" );
#else
    FILE* infile = fopen( filename, "rb" );
#endif // WIN32

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

vector<GLuint> textures;

void loadTextures()
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


int getBone(PMXInfo &pmxInfo, string &name)
{
	/*int boneMatchCount=0;
	for(int b=0; b<pmxInfo.bone_continuing_datasets; ++b)
	{
		if(pmxInfo.bones[b]->name==name) return b;
	}*/
	
	return -1;
}

void quaternionToMat4(glm::mat4 &m, glm::vec3 &p, glm::vec4 &q)
{
	m[0][3] = p[0];
	m[1][3] = p[1];
	m[2][3] = p[2];
	
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
		m[2][1] = yz2 - wx2;
		m[1][2] = yz2 + wx2;
	}
	{
		float xy2 = q[0] * y2;
		float wz2 = q[3] * z2;
		m[1][0] = xy2 - wz2;
		m[0][1] = xy2 + wz2;
	}
	{
		float xz2 = q[0] * z2;
		float wy2 = q[3] * y2;
		m[0][2] = xz2 - wy2;
		m[2][0] = xz2 + wy2;
	}
}

glm::mat4 getGlobalPosition(PMXBone *bone)
{
	//cout<<"Getting global position for "<<bone->name<<endl;
	if(bone->parentBoneIndex!=255)
	{
		//cout<<"Returning Bone "<<bone->name<<endl;
		//glm::mat4 result=glm::translate(bone->position.x,bone->position.y,bone->position.z);
		//glm::vec4 position=glm::vec4(position.x,position.y,position.z,1.0);
		
		/*glm::mat4 result;
		result[0]=glm::vec4(1,0,0,bone->position.x);
		result[1]=glm::vec4(0,1,0,bone->position.y);
		result[2]=glm::vec4(0,0,1,bone->position.z);
		result[3]=glm::vec4(0,0,0,bone->position.w);*/
		
		/*cout<<result[0][0]<<" "<<result[0][1]<<" "<<result[0][2]<<" "<<result[0][3]<<endl
		<<result[1][0]<<" "<<result[1][1]<<" "<<result[1][2]<<" "<<result[1][3]<<endl
		<<result[2][0]<<" "<<result[2][1]<<" "<<result[2][2]<<" "<<result[2][3]<<endl
		<<result[3][0]<<" "<<result[3][1]<<" "<<result[3][2]<<" "<<result[3][3]<<endl<<endl;*/
		
		glm::mat4 result;
		glm::vec4 dud;
		//result=result*glm::vec4(bone->position,1.0f);
		
		//glm::vec3 position=glm::vec3(bone->position.x,-bone->position.y,bone->position.z);
		
		glm::mat4 result2=glm::translate(bone->position.x,bone->position.y,bone->position.z);//*getGlobalPosition(pmxInfo.bones[bone->parentBoneIndex]);
		//glm::mat4 result2;
		//quaternionToMat4(result2,position,dud);//*getGlobalPosition(pmxInfo.bones[bone->parentBoneIndex]);
		//result2=glm::inverse(result2);
		
		/*cout<<"Final position of bone "<<bone->name<<": "<<endl;
		cout<<result2[0][0]<<" "<<result2[0][1]<<" "<<result2[0][2]<<" "<<result2[0][3]<<endl
		<<result2[1][0]<<" "<<result2[1][1]<<" "<<result2[1][2]<<" "<<result2[1][3]<<endl
		<<result2[2][0]<<" "<<result2[2][1]<<" "<<result2[2][2]<<" "<<result2[2][3]<<endl
		<<result2[3][0]<<" "<<result2[3][1]<<" "<<result2[3][2]<<" "<<result2[3][3]<<endl<<endl;*/
		
		return result2;
	}
	//cout<<"Returning Parent Bone"<<endl;
	return glm::translate(bone->position.x,bone->position.y,bone->position.z);
	
	/*if(bone->parentBoneIndex!=255)
	{
		return pmxInfo.bones[bone->parentBoneIndex]->position+bone->position;
	}
	return bone->position;*/
}

void setBoneToFrame(PMXBone *bone, BoneFrame &bf)
{
	//*****************LOOK HERE*****************/
	
	/*digited: This is where I'm really having trouble.
	I need to figure out how to transform the bones by an Animation matrix (which I must construct from a position+quaternion) without causing issues.
	The main functions you'll probably want to look at are setBoneToFrame() and setModelToKeyFrame().
	Look at pmx.h, vmd.h for information concerning the PMX and VMD file formats
	
	This code is a big mess at the moment; I'm well aware, my plan was to spend time cleaning up after I got animation working.
	In particular pmxInfo and vmdInfo are used liberally as global variables in many functions (while others properly require it as a function argument)*/
	
	/*cout<<"Refreshing relative position matrix for: "<<bone->name<<endl;
	cout<<bf.position.x<<" "<<bf.position.y<<" "<<bf.position.z<<endl;
	cout<<bf.quaternion.x<<" "<<bf.quaternion.y<<" "<<bf.quaternion.z<<" "<<bf.quaternion.w<<endl<<endl;
				
	glm::mat4 boneMatrix;
	quaternionToMat4(boneMatrix,bf.position,bf.quaternion);
	
	glm::mat4 invBoneMatrix=glm::inverse(boneMatrix);
	
	glm::mat4 result=invBoneMatrix*bone->relativeForm;
	
	bone->relativeForm=result;
	
	if(bone->parentBoneIndex==255)
	{
		
	}
	else
	{
		
		
		setBoneToFrame(pmxInfo.bones[bone->parentBoneIndex],bf);
	}*/
}

void setModelToKeyFrame(glm::mat4 Bone[], GLuint &shaderProgram)
{
	cout<<"START BONE READ"<<endl;
	
	PMXInfo result=pmxInfo;
	
	cout<<"bone size: "<<pmxInfo.bone_continuing_datasets<<endl;
	
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

	for(size_t i=0; i<pmxInfo.bone_continuing_datasets; i++)
	{
		PMXBone *b = pmxInfo.bones[i];
		
		b->relativeForm[0]=glm::vec4(1,0,0,0);
		b->relativeForm[1]=glm::vec4(0,1,0,0);
		b->relativeForm[2]=glm::vec4(0,0,1,0);
		b->relativeForm[3]=glm::vec4(b->position.x,b->position.y,b->position.z,1);
		
		if(b->parentBoneIndex==255)
		{
			b->absoluteForm = b->relativeForm;
		}
		else
		{
			b->absoluteForm = pmxInfo.bones[b->parentBoneIndex]->absoluteForm * b->relativeForm;
		}
		
		bindPose[i] = (pmxInfo.bones[i]->absoluteForm);
		invBindPose[i] = glm::inverse(bindPose[i]);
		
		//Bone[i] = b->absoluteForm*invBindPose[i];
	}
	
	for(int i=0; i<vmdInfo.boneCount; ++i)
	{
		//int i=1;
		BoneFrame &bf=vmdInfo.boneFrames[i];
		
		if(bf.frame==targetFrame)
		{
			PMXBone *b=getChildBone(bf.name,parentBone);
			
			if(b)
			{				
				setBoneToFrame(b,bf);
			}
			else
			{
				//cerr<<"Warning: No matching bone found for frame: "<<bf.name<<endl;
			}
		}
	}
	
	for(size_t i=0; i<pmxInfo.bone_continuing_datasets; i++)
	{
		PMXBone *b = pmxInfo.bones[i];
		
		/*b->relativeForm[0]=glm::vec4(1,0,0,0);
		b->relativeForm[1]=glm::vec4(0,1,0,0);
		b->relativeForm[2]=glm::vec4(0,0,1,0);
		b->relativeForm[3]=glm::vec4(b->position.x,b->position.y,b->position.z,1);*/
		
		if(b->parentBoneIndex==255)
		{
			b->absoluteForm = b->relativeForm;
		}
		else
		{
			b->absoluteForm = pmxInfo.bones[b->parentBoneIndex]->absoluteForm * b->relativeForm;
		}
		
		//bindPose[i] = (pmxInfo.bones[i]->absoluteForm);
		//invBindPose[i] = glm::inverse(bindPose[i]);
		
		/*cout<<invBindPose[i][0][0]<<" "<<invBindPose[i][0][1]<<" "<<invBindPose[i][0][2]<<" "<<invBindPose[i][0][3]<<endl
		<<invBindPose[i][1][0]<<" "<<invBindPose[i][1][1]<<" "<<invBindPose[i][1][2]<<" "<<invBindPose[i][1][3]<<endl
		<<invBindPose[i][2][0]<<" "<<invBindPose[i][2][1]<<" "<<invBindPose[i][2][2]<<" "<<invBindPose[i][2][3]<<endl
		<<invBindPose[i][3][0]<<" "<<invBindPose[i][3][1]<<" "<<invBindPose[i][3][2]<<" "<<invBindPose[i][3][3]<<endl<<endl;*/
		
		Bone[i] = b->absoluteForm*invBindPose[i];
	}
	
	
	/*for(unsigned i=0; i<pmxInfo.bone_continuing_datasets; ++i)
	{
		PMXBone *bone=getChildBone(i,parentBone);
		//cout<<bone->name<<endl;
		
		glm::vec3 position;
		glm::vec4 blankQuaternion;
		
		
		glm::mat4 boneMatrix;
		quaternionToMat4(boneMatrix,position,blankQuaternion);
		Bone[i]=boneMatrix;
	}*/
	
	GLuint Bones_loc=glGetUniformLocation(shaderProgram,"Bones");
	glUniformMatrix4fv(Bones_loc, pmxInfo.bone_continuing_datasets, GL_FALSE, &Bone[0][0][0]);
	
	
	
	cout<<"END BONE READ"<<endl;
	//exit(EXIT_SUCCESS);
	//Transform the vertices
}

GLuint shaderProgram;
void init()
{
	shaderProgram=loadShaders();
	
	//Load Textures
	loadTextures();    
	
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
		
		if(pmxInfo.vertices[i]->weight_transform_formula==3)
		{
			cerr<<"SDEF not supported yet"<<endl;
			exit(EXIT_FAILURE);
		}
		
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
	glm::mat4 *skinMatrix=new glm::mat4[pmxInfo.bone_continuing_datasets]();
	setModelToKeyFrame(Bone,shaderProgram);
	
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

void drawModel()
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
	//WARNING: Code broken at this pointCode
	
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
	
	drawModel();
	
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
	readPMX();
	readVMD(vmdInfo);
	
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
	
	init();
	
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
