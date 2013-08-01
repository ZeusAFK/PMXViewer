#include "pmx.h"

#include <fstream>
#include <iostream>
#include <bitset>
#include <sstream>

#include <string.h>

#include "Converter.h"

using namespace std;

	

    std::string UTF16to8(const unsigned short * in)
    {
        std::string out;
        unsigned int codepoint = 0;
        for (in;  *in != 0;  ++in)
        {
            if (*in >= 0xd800 && *in <= 0xdbff)
                codepoint = ((*in - 0xd800) << 10) + 0x10000;
            else
            {
                if (*in >= 0xdc00 && *in <= 0xdfff)
                    codepoint |= *in - 0xdc00;
                else
                    codepoint = *in;
     
                if (codepoint <= 0x7f)
                    out.append(1, static_cast<char>(codepoint));
                else if (codepoint <= 0x7ff)
                {
                    out.append(1, static_cast<char>(0xc0 | ((codepoint >> 6) & 0x1f)));
                    out.append(1, static_cast<char>(0x80 | (codepoint & 0x3f)));
                }
                else if (codepoint <= 0xffff)
                {
                    out.append(1, static_cast<char>(0xe0 | ((codepoint >> 12) & 0x0f)));
                    out.append(1, static_cast<char>(0x80 | ((codepoint >> 6) & 0x3f)));
                    out.append(1, static_cast<char>(0x80 | (codepoint & 0x3f)));
                }
                else
                {
                    out.append(1, static_cast<char>(0xf0 | ((codepoint >> 18) & 0x07)));
                    out.append(1, static_cast<char>(0x80 | ((codepoint >> 12) & 0x3f)));
                    out.append(1, static_cast<char>(0x80 | ((codepoint >> 6) & 0x3f)));
                    out.append(1, static_cast<char>(0x80 | (codepoint & 0x3f)));
                }
                codepoint = 0;
            }
        }
        return out;
    }



/*void getPMXText(ifstream &miku, PMXInfo &pmxInfo, string &result, bool debug)
{
	uint32_t text_size; //text size in bytes
	miku.read((char*)&text_size,4);
	
	if(text_size==0) return;
	
	//cout<<"Text Size: "<<text_size<<endl;
	if(pmxInfo.unicode_type==PMX_ENCODE_UTF16)
	{
		//WARNING: UTF-16 text-pulling code does NOT support the extra (multi-byte) codesets of UTF-16!!!!
		
		#ifdef _WIN32
		
		
		char16_t c16[text_size/2];
		for(int i=0; i<text_size; ++i)
		{
			c16[i]=0;
		}
		
		miku.read((char*)&c16,text_size);
		
		result=UTF16to8((const short unsigned*) c16);
		cout<<result<<endl;
		
		#else
		
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

		#endif //_WIN32
		
		//exit(EXIT_SUCCESS);
	}
	else
	{
		//WARNING: Loading UTF-8 encoded PMX files is untested (if only they all were UTF-8 encoded)
		char c8[text_size];
		miku.read((char*)&c8,text_size);
		
		result=c8;
	}
}*/

void getPMXText(ifstream &miku, PMXInfo &pmxInfo, string &result, bool debug)
{
	uint32_t text_size;
	miku.read((char*)&text_size,4);

	if(!text_size)
		return;
     
	if(pmxInfo.unicode_type==PMX_ENCODE_UTF16)
	{
		//WARNING: UTF-16 text-pulling code does NOT support the extra (multi-byte) codesets of UTF-16!!!!             
		unsigned short c16[text_size/2+1];
		memset(c16, 0, text_size+2);
		miku.read((char*)c16, text_size);

		#ifndef _WIN32
		result=UTF16to8(c16);
		#else
		result = to_u8string((const u16string&)c16);
		#endif

		ofstream converted;
		converted.open("converted.txt", ios::out | ios::app);
		//converted.seekp(ios_base::end);
		//converted.write((char*)c16, text_size);
		converted<<result<<endl;
		converted << '\n';
		converted.close();



		//exit(EXIT_SUCCESS);
	}
	else
	{
		//WARNING: Loading UTF-8 encoded PMX files is untested
		char c8[text_size];
		miku.read((char*)&c8,text_size);

		result=c8;
	}
}

/*PMXBone *getChildBone(int &id, PMXBone *parentBone)
{
	if(id==-1) return parentBone;
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
}*/

int getBone(PMXInfo &pmxInfo, string &name)
{
	int boneMatchCount=0;
	for(int b=0; b<pmxInfo.bone_continuing_datasets; ++b)
	{
		if(pmxInfo.bones[b]->name==name) return b;
	}
	
	return -1;
}

void printDebugInfo(PMXInfo &pmxInfo);

PMXInfo &readPMX(string foldername,string filename)
{
	PMXInfo *pInfo=new PMXInfo();
	PMXInfo &pmxInfo=*pInfo;
	
	string fname=foldername+filename;
	
	//ifstream miku("apimiku/Appearance Miku.pmx", ios::in | ios::binary);
	ifstream miku(fname.c_str(), ios::in | ios::binary);
	if(!miku)
	{
		cerr<<"ERROR: PMX file could not be opened"<<endl;
		exit(EXIT_FAILURE);
	}
	
	//***Extract header info***
	miku.read(pmxInfo.header_str,4);
	miku.read((char*)&pmxInfo.ver,4);
	
	if(string(pmxInfo.header_str).find("PMX ")==-1)
	{
		cout<<pmxInfo.header_str<<endl;
		cerr<<"Error: Invalid PMX file header (is this really a PMX file?)"<<endl;
		//exit(EXIT_FAILURE);
	}
	if(pmxInfo.ver!=2.0)
	{
		cerr<<"Error: Only version 2.0 of the PMX file format is supported!"<<endl;
		exit(EXIT_FAILURE);
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
	

	if(pmxInfo.unicode_type==PMX_ENCODE_UTF8)
	{
		cerr<<"WARNING: UTF-8 encoded PMX file loading is untested"<<endl;
	}
	
	//***Pull model name, comment info***
	getPMXText(miku, pmxInfo, pmxInfo.modelName);
	getPMXText(miku, pmxInfo, pmxInfo.modelNameEnglish);
	getPMXText(miku, pmxInfo, pmxInfo.comment);
	getPMXText(miku, pmxInfo, pmxInfo.commentEnglish);
	
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
			cerr<<"ERROR: bone structure (QDEF?) not supported yet"<<endl;
			exit(EXIT_FAILURE);
		}
	
		miku.read((char*)&vertex->edgeScale,4);
		
		pmxInfo.vertices.push_back(vertex);
		
		delete x;
		delete y;
		delete z;
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
		getPMXText(miku, pmxInfo, pmxInfo.texturePaths[i]);
		
		pmxInfo.texturePaths[i].insert(0,foldername);
		
		while(pmxInfo.texturePaths[i].find("\\")!=-1)
		{
			int index=pmxInfo.texturePaths[i].find("\\");
			pmxInfo.texturePaths[i][index]='/';
		}
	}
	cout<<"done."<<endl;
	
	//***Pull Material Info***
	miku.read((char*)&pmxInfo.material_continuing_datasets,4);
	
	cout<<"Loading materials...";
	for(int i=0; i<pmxInfo.material_continuing_datasets; ++i)
	{		
		PMXMaterial *material=new PMXMaterial();
		
		//***Pull Material Names***
		getPMXText(miku, pmxInfo, material->name);
		getPMXText(miku, pmxInfo, material->nameEng);
		
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
		getPMXText(miku, pmxInfo, material->memo);
		
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
		
		getPMXText(miku, pmxInfo, bone->name);
		getPMXText(miku, pmxInfo, bone->nameEng);
		
		//***Pull Position***
		miku.read((char*)&bone->position.x,4);
		miku.read((char*)&bone->position.y,4);
		miku.read((char*)&bone->position.z,4);
		
		//***Pull Parent Index***
		//TODO: Apply this method of setting the index to the rest of the file-reading code (make a function)
		if(pmxInfo.boneIndexSize==1)
		{
			int8_t tmpBoneIndex;
			miku.read((char*)&tmpBoneIndex,(int)pmxInfo.boneIndexSize);
			
			bone->parentBoneIndex=(int)tmpBoneIndex;
		}
		else if(pmxInfo.boneIndexSize==2)
		{
			int16_t tmpBoneIndex;
			miku.read((char*)&tmpBoneIndex,(int)pmxInfo.boneIndexSize);
			
			bone->parentBoneIndex=(int)tmpBoneIndex;
		}
		else if(pmxInfo.boneIndexSize==4)
		{
			int tmpBoneIndex;
			miku.read((char*)&tmpBoneIndex,(int)pmxInfo.boneIndexSize);
			
			bone->parentBoneIndex=(int)tmpBoneIndex;
		}
		
		if(bone->parentBoneIndex<-1)
		{
			cerr<<"WARNING: Negative Bone Index greater than -1 found: "<<bone->name<<" "<<bone->parentBoneIndex<<endl;
			bone->parentBoneIndex=-bone->parentBoneIndex;
		}
		
		/*cout<<(int)*tmpBoneIndex<<endl;
		cout<<"PBI: "<<bone->parentBoneIndex<<endl;*/
		
		//exit(EXIT_SUCCESS);
		
		
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
	
		bone->giveRotation=bitflag2[0];
		bone->giveTranslation=bitflag2[1];
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
		
		if(bone->giveRotation || bone->giveTranslation)
		{
			miku.read((char*)&bone->givenParentBoneIndex,(int)pmxInfo.boneIndexSize);
			miku.read((char*)&bone->giveRate,4);
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
			miku.read((char*)&bone->IKLoopAngleLimit,4);
			
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
		
		bone->relativeForm[3][0]=bone->position.x;
		bone->relativeForm[3][1]=bone->position.y;
		bone->relativeForm[3][2]=bone->position.z;
		
		pmxInfo.bones.push_back(bone);
	}
	cout<<"done."<<endl;
		
	//***Pull Morph Info***
	miku.read((char*)&pmxInfo.morph_continuing_datasets,4);
	cout<<"Morph Continuing Datasets: "<<pmxInfo.morph_continuing_datasets<<endl;
	
	cout<<"Loading morphs...";
	for(int m=0; m<pmxInfo.morph_continuing_datasets; ++m)
	{
		//cout<<"[Morph "<<i<<endl;
		PMXMorph *morph=new PMXMorph;
		getPMXText(miku, pmxInfo, morph->name);
		getPMXText(miku, pmxInfo, morph->nameEng);
		
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
					PMXGroupMorph *groupMorph=new PMXGroupMorph();
					
					miku.read((char*)&groupMorph->morphIndex,(int)pmxInfo.morphIndexSize);
					miku.read((char*)&groupMorph->morphRate,4);
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
		getPMXText(miku, pmxInfo, df->name);
		getPMXText(miku, pmxInfo, df->nameEng);
		
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
		getPMXText(miku, pmxInfo, rb->name);
		getPMXText(miku, pmxInfo, rb->nameEng);
		
		char *tmpBoneIndex=(char*) malloc(pmxInfo.boneIndexSize);
		miku.read(tmpBoneIndex,(int)pmxInfo.boneIndexSize);
		
		rb->relatedBoneIndex=(int)*tmpBoneIndex;
		delete tmpBoneIndex;
		
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
		getPMXText(miku, pmxInfo, joint->name);
		getPMXText(miku, pmxInfo, joint->nameEng);
		
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
	
	//printDebugInfo(pmxInfo);
	
	//exit(EXIT_SUCCESS);
	
	return pmxInfo;
}

void printDebugInfo(PMXInfo &pmxInfo)
{
	cout<<pmxInfo.header_str<<"ver "<<pmxInfo.ver<<endl;
	
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
		
		cerr<<"Rotation Allowed: "<<bone->giveRotation<<endl;
		cerr<<"Movement Allowed: "<<bone->giveTranslation<<endl;
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
		
		if(bone->giveRotation || bone->giveTranslation)
		{
			cerr<<"Allowed Parent Bone Index: "<<bone->givenParentBoneIndex<<endl;
			cerr<<"Allowed Rate: "<<bone->giveRate<<endl;
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
