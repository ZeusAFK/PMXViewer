#ifndef MOTION_CONTROLLER_H
#define MOTION_CONTROLLER_H

#include "pmx.h"
#include "vmd.h"
#include "../vgl.h"

#include <list>


BoneFrame *getBoneFrame(VMDInfo &vmdInfo, int frame, std::string boneName);

float ipfunc(float t, char p1, char p2);
float ipfuncd(float t, char p1, char p2);
float bezierp(char x1, char x2, char y1, char y2, float x);

class VMDMotionController
{
	public:
	VMDMotionController(PMXInfo &pmxInfovar,VMDInfo &vmdInfovar,GLuint shaderProgram);
	~VMDMotionController();
	void updateBoneMatrix();
	void advanceTime();
	
	
	private:
	int time;
	std::vector<PMXBone*> bones;
	
	void updateIK();
	
	
	PMXInfo &pmxInfo;
	VMDInfo &vmdInfo;
	
	glm::mat4 *skinMatrix; //Final Skinning Matrix
	glm::mat4 *invBindPose;
	
	GLuint Bones_loc; //uniform variable location for Bones[] in shader
	
	std::vector<std::list<BoneFrame>> keyFrames;				//Hold a list of keyframes for each Bone
	std::vector<std::list<BoneFrame>::iterator> ite_keyFrames;	//Keyframe iterator
	
	std::vector<glm::quat> boneRot;
	std::vector<glm::vec3> bonePos;
};


#endif
