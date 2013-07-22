#include "vmd.h"

#include <fstream>
#include <iostream>
#include <bitset>
#include <sstream>

#include "texthandle.h"

using namespace std;

VMDInfo &readVMD()
{
	VMDInfo *vInfo=new VMDInfo();
	VMDInfo &vmdInfo=*vInfo;
	
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
	
	return vmdInfo;
	
	//exit(EXIT_SUCCESS);
}
