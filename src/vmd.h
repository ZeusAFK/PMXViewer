#ifndef VMD_H
#define VMD_H

#include <vector>
#include <string>
#include <fstream>

#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>

struct VMDInfo;

//VMD related functions

VMDInfo &readVMD(std::string filename);

//VMD related structs

struct BezierParameters
{
	//See "On VMD Interpolation Parameters.txt" in DOCs for information concerning bezier parameters.
	
	glm::vec2 X1;
	glm::vec2 X2;
	
	glm::vec2 Y1;
	glm::vec2 Y2;
	
	glm::vec2 Z1;
	glm::vec2 Z2;
	
	glm::vec2 R1;
	glm::vec2 R2;
};

struct BoneFrame
{
	std::string name; //char[15] before UTF8 conversion
	
	unsigned int frame;
	
	glm::vec3 translation; //values will be 0 when there's no position
	glm::quat quaternion; //values will be 0 when there's no position (w will be 1.0)
	
	BezierParameters bezier;
	
	bool operator < (const BoneFrame &k) const
	{
		//Comparison by frame number
		return frame < k.frame;
	}
};

struct MorphFrame
{
	std::string name; //char[15] before UTF8 conversion
	unsigned int frame;
	float value;
};

struct CameraFrame
{
	unsigned int frame;
	
	float distance; //Distance between the camera and pt of interest (the camera is negative when in front of the object)
	glm::vec3 position;
	glm::vec3 rotation;
	
	std::string interpolationParameters; //char[24] before UTF8 conversion
	
	unsigned int viewAngle; //viewing angle in degrees
	bool perspective; //0:ON 1:OFF
};

struct LightFrame
{
};

struct SelfShadowFrame
{
};

struct VMDInfo
{
	char headerStr[30];
	std::string modelName;
	
	//Bone Frames
	int boneCount; //number of continuing bone datasets
	BoneFrame *boneFrames;
	
	//Morph Frames (Emotion data)
	int morphCount;
	MorphFrame *morphFrames;
	
	int cameraCount;
	CameraFrame *cameraFrames;
	
	int lightCount;
	LightFrame *lightFrames;
	
	int selfShadowCount;
	SelfShadowFrame *selfShadowFrames;
};

#endif
