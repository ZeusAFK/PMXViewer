#include "vmd.h"

#include <fstream>
#include <iostream>
#include <bitset>
#include <sstream>

#include "texthandle.h"

using namespace std;

VMDInfo &readVMD(string filename)
{
	VMDInfo *vInfo=new VMDInfo();
	VMDInfo &vmdInfo=*vInfo;
	
	ifstream miku(filename.c_str(), ios::in | ios::binary);
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
	
	cout<<"Bone Count: "<<vmdInfo.boneCount<<endl;
	
	cout<<"Loading bone frames...";
	for(int i=0; i<vmdInfo.boneCount; ++i)
	{
		BoneFrame *f=&vmdInfo.boneFrames[i];
		
		char name[15];
		miku.read((char*)name,15);
		f->name=sjisToUTF8(name);
		
		miku.read((char*)&f->frame,4);
		
		miku.read((char*)&f->translation.x,4);
		miku.read((char*)&f->translation.y,4);
		miku.read((char*)&f->translation.z,4);
		
		//cout<<f->position.x<<" "<<f->position.y<<" "<<f->position.z<<endl;
		
		miku.read((char*)&f->quaternion.x,4);
		miku.read((char*)&f->quaternion.y,4);
		miku.read((char*)&f->quaternion.z,4);
		miku.read((char*)&f->quaternion.w,4);
		
		int8_t bezier[64];
		miku.read((char*)bezier,64);
		
		//line 1
		f->bezier.X1.x=bezier[0];
		f->bezier.Y1.x=bezier[1];
		f->bezier.Z1.x=bezier[2];
		f->bezier.R1.x=bezier[3];
		
		f->bezier.X1.y=bezier[4];
		f->bezier.Y1.y=bezier[5];
		f->bezier.Z1.y=bezier[6];
		f->bezier.R1.y=bezier[7];
		
		//line 2
		f->bezier.X2.x=bezier[8];
		f->bezier.Y2.x=bezier[9];
		f->bezier.Z2.x=bezier[10];
		f->bezier.R2.x=bezier[11];
		
		f->bezier.X2.y=bezier[12];
		f->bezier.Y2.y=bezier[13];
		f->bezier.Z2.y=bezier[14];
		f->bezier.R2.y=bezier[15];
		
		/*cout<<f->bezier.X1.x<<" "<<f->bezier.X1.y<<" "<<f->bezier.Y1.x<<" "<<f->bezier.Y1.y<<" "<<f->bezier.Z1.x<<" "<<f->bezier.Z1.y<<" "<<f->bezier.R1.x<<" "<<f->bezier.R1.y<<endl
		<<f->bezier.X2.x<<" "<<f->bezier.X2.y<<" "<<f->bezier.Y2.x<<" "<<f->bezier.Y2.y<<" "<<f->bezier.Z2.x<<" "<<f->bezier.Z2.y<<" "<<f->bezier.R2.x<<" "<<f->bezier.R2.y<<endl<<endl;*/
		
		
		/*cout<<"Name: "<<f->name<<endl;
		cout<<"Frame Number: "<<f->frame<<endl<<endl;
		
		cout<<"Position: "<<f->position.x<<" "<<f->position.y<<" "<<f->position.z<<endl;
		cout<<"Quaternion: "<<f->quaternion.x<<" "<<f->quaternion.y<<" "<<f->quaternion.z<<" "<<f->quaternion.w<<endl;*/
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
	
	return vmdInfo;
	
	//exit(EXIT_SUCCESS);
}
