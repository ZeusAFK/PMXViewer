#ifndef VMD_H
#define VMD_H

struct BoneFrame
{
	std::string name; //char[15] before UTF8 conversion
	
	unsigned int frame;
	
	glm::vec3 position; //values will be 0 when there's no position
	glm::vec4 quaternion; //values will be 0 when there's no position (w will be 1.0)
	
	std::string interpolationParameters; //char[64] before UTF8 conversion
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
