#include "motioncontroller.h"

#include <sstream>
#include <iostream>

#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/compatibility.hpp>

using namespace std;

BoneFrame *getBoneFrame(VMDInfo &vmdInfo, int frame, string boneName)
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

float ipfunc(float t, char p1, char p2)
{
	return ((1 + 3 * p1 - 3 * p2) * t * t * t + (3 * p2 - 6 * p1) * t * t + 3 * p1 * t);
}

float ipfuncd(float t, char p1, char p2)
{
	return ((3 + 9 * p1 - 9 * p2) * t * t + (6 * p2 - 12 * p1) * t + 3 * p1);
}

float bezierp(char x1, char x2, char y1, char y2, float x)
{
/*interpolate using Bezier curve (http://musashi.or.tv/fontguide_doc3.htm)
Bezier curve is parametrized by t (0 <= t <= 1)
x = s^3 x_0 + 3 s^2 t x_1 + 3 s t^2 x_2 + t^3 x_3
y = s^3 y_0 + 3 s^2 t y_1 + 3 s t^2 y_2 + t^3 y_3
where s is defined as s = 1 - t.
Especially, for MMD, (x_0, y_0) = (0, 0) and (x_3, y_3) = (1, 1), so
x = 3 s^2 t x_1 + 3 s t^2 x_2 + t^3
y = 3 s^2 t y_1 + 3 s t^2 y_2 + t^3
Now, given x, find t by bisection method (http://en.wikipedia.org/wiki/Bisection_method)
i.e. find t such that f(t) = 3 s^2 t x_1 + 3 s t^2 x_2 + t^3 - x = 0
One thing to note here is that f(t) is monotonically increasing in the range [0,1]
Therefore, when I calculate f(t) for the t I guessed,
Finally find y for the t.
###
#Adopted from MMDAgent*/
	float &t = x;
	while(true)
	{
		float v = ipfunc(t, x1, x2) - x;
		if(v * v < 0.0000001) break; //Math.abs(v) < 0.0001
		double tt = glm::mix(x1,x2,t);//ipfuncd(t, x1, x2)
		if(tt == 0) break;
		t -= v / tt;
		return ipfunc(t, y1, y2);
	}
}

glm::vec4 Vectorize(const glm::quat theQuat)
{
	glm::vec4 ret;

	ret.x = theQuat.x;
	ret.y = theQuat.y;
	ret.z = theQuat.z;
	ret.w = theQuat.w;

	return ret;
}

glm::fquat Lerp(const glm::quat &v0, const glm::quat &v1, float alpha)
{
    glm::vec4 start = Vectorize(v0);
    glm::vec4 end = Vectorize(v1);
    glm::vec4 interp = glm::mix(start, end, alpha);
    interp = glm::normalize(interp);
    return glm::fquat(interp.w, interp.x, interp.y, interp.z);
}

glm::fquat Slerp(glm::quat &v0, glm::quat &v1, float alpha)
{
	float dot = glm::dot(v0, v1);
	
	if(dot<0.0f)
	{
		dot=-dot;
		v0=-v0;
	}
    
	const float DOT_THRESHOLD = 0.9995f;
	if(dot > DOT_THRESHOLD)
		return Lerp(v0, v1, alpha);

	glm::clamp(dot, -1.0f, 1.0f);
	float theta_0 = acosf(dot);
	float theta = theta_0*alpha;
    
	glm::quat v2 = v1 + -v0*dot;
	v2 = glm::normalize(v2);
    
    return v0*cos(theta) + v2*sin(theta);
}

VMDMotionController::VMDMotionController(PMXInfo &pmxInfovar,VMDInfo &vmdInfovar,GLuint shaderProgram):pmxInfo(pmxInfovar),vmdInfo(vmdInfovar)
{
	time=0;
	
	invBindPose=new glm::mat4[pmxInfo.bone_continuing_datasets];
	for(int i=0; i<pmxInfo.bone_continuing_datasets; ++i)
	{
		PMXBone *b = pmxInfo.bones[i];
		invBindPose[i] = glm::translate( -b->position );
	}
	skinMatrix=new glm::mat4[pmxInfo.bone_continuing_datasets]();
	
	Bones_loc=glGetUniformLocation(shaderProgram,"Bones");
	
	
	keyFrames.resize(pmxInfo.bone_continuing_datasets);
	for(unsigned i=0; i<vmdInfo.boneCount; ++i)
	{
		for(unsigned j=0; j<pmxInfo.bone_continuing_datasets; ++j)
		{
			//Search for the bone number from the bone name
			if(vmdInfo.boneFrames[i].name == pmxInfo.bones[j]->name)
			{
				keyFrames[j].push_back(vmdInfo.boneFrames[i]);
				break;
			}
		}
	}
	
	for(unsigned int i=0; i<pmxInfo.bone_continuing_datasets; ++i)
	{
		keyFrames[i].sort();
		ite_keyFrames.push_back(keyFrames[i].begin());
		boneRot.push_back(glm::quat(0, 0, 0, 0));
		bonePos.push_back(glm::vec3(0, 0, 0));
	}
}
VMDMotionController::~VMDMotionController()
{
	free(invBindPose);
	free(skinMatrix);
}

void VMDMotionController::updateBoneMatrix()
{
	// Root bone
	PMXBone   *b  = pmxInfo.bones[0];
	BoneFrame *bf = getBoneFrame(vmdInfo,0, b->name);
	
	//Print list of keyframe #s
	int lastFrame=-1;
	/*for(int i=0; i<vmdInfo.boneCount; ++i)
	{
		if(vmdInfo.boneFrames[i].frame!=lastFrame)
		{
			cout<<vmdInfo.boneFrames[i].frame<<endl;
			lastFrame=vmdInfo.boneFrames[i].frame;
		}
	}*/

	if(bf!=NULL)
	{		
		b->relativeForm = glm::translate( bf->translation + b->position ) * glm::toMat4(bf->quaternion);
		b->absoluteForm = b->relativeForm;
		skinMatrix[0] = b->absoluteForm * invBindPose[0];
	}
	
	// Other bones
	for (unsigned i = 1; i < pmxInfo.bone_continuing_datasets; i++)
	{
		b  = pmxInfo.bones[i];
		PMXBone *parent = pmxInfo.bones[b->parentBoneIndex];
		
		unsigned long t0,t1;
		glm::quat q0,q1;
		glm::vec3 p0,p1;
		
		if(ite_keyFrames[i] != keyFrames[i].end())
		{
			t0=(*ite_keyFrames[i]).frame*2; //MMD=30fps, this program=60fps, 60/30=2
			boneRot[i] = q0 = (*ite_keyFrames[i]).quaternion;
			bonePos[i] = p0 = (*ite_keyFrames[i]).translation;
			
			if(++ite_keyFrames[i] != keyFrames[i].end())
			{
				t1 = (*ite_keyFrames[i]).frame*2;
				q1 = (*ite_keyFrames[i]).quaternion;
				p1 = (*ite_keyFrames[i]).translation;
				
				float s = (float)(time - t0)/(float)(t1 - t0);	//Linear Interpolation
				
				boneRot[i]=Slerp(q0,q1,s);
				bonePos[i] = p0 + (p1 - p0)*s;
				if (time != t1) --ite_keyFrames[i];
			}
		}
			
		/*cout<<(*ite_keyFrames[i]).quaternion.x<<" "<<(*ite_keyFrames[i]).quaternion.y<<" "<<(*ite_keyFrames[i]).quaternion.z<<" "<<(*ite_keyFrames[i]).quaternion.w<<endl
		<<boneRot[i].x<<" "<<boneRot[i].y<<" "<<boneRot[i].z<<" "<<boneRot[i].w<<endl
		<<bf->quaternion.x<<" "<<bf->quaternion.y<<" "<<bf->quaternion.z<<" "<<bf->quaternion.w<<endl<<endl;*/

		b->relativeForm = glm::translate( bonePos[i] + b->position - parent->position ) * glm::toMat4(boneRot[i]);
		b->absoluteForm = parent->absoluteForm * b->relativeForm;
		skinMatrix[i] = b->absoluteForm * invBindPose[i];
	}
			

	
	
	//updateIK();
	
	glUniformMatrix4fv(Bones_loc, pmxInfo.bone_continuing_datasets, GL_FALSE, (const GLfloat*)skinMatrix);
}

void VMDMotionController::updateIK()
{
	// IK Bones	
	for(unsigned b=0; b<pmxInfo.bone_continuing_datasets; ++b)
	{
		PMXBone *IKBone=pmxInfo.bones[b];
		if(IKBone->IK)
		{
			PMXBone *targetBone=pmxInfo.bones[IKBone->IKTargetBoneIndex];
			
			for(unsigned iterations=0; iterations<IKBone->IKLoopCount; ++iterations)
			{
				for(int ik=IKBone->IKLinkNum-1; ik>=0; --ik)
				{
					PMXIKLink *IKLink=IKBone->IKLinks[ik];
					PMXBone *linkBone=pmxInfo.bones[IKLink->linkBoneIndex]; //Bone under the effect of the IK Bone
					
					//cout<<"boneinfo: "<<IKBone->name<<" "<<targetBone->name<<" "<<linkBone->name<<endl;
					
					glm::vec3 targetVector=glm::vec3(glm::normalize(targetBone->relativeForm[3]));
					glm::vec3 IKVector=glm::vec3(glm::normalize(IKBone->relativeForm[3]));
					
					//cout<<"targetVector: "<<targetVector.x<<" "<<targetVector.y<<" "<<targetVector.z<<endl;
					//cout<<"IKVector: "<<IKVector.x<<" "<<IKVector.y<<" "<<IKVector.z<<endl;
					
					float cosAngle=glm::dot(targetVector,IKVector);
					//cout<<"cosAngle: "<<cosAngle<<endl;
					
					glm::vec3 axis;
					/*if(cosAngle>=0)
					{
						axis=glm::cross(targetVector,IKVector);
					}
					else //cosAngle<0
					{
						axis=(float)M_PI-glm::cross(targetVector,IKVector);
					}*/
					axis=glm::cross(targetVector,IKVector);
						
					float angle=acos(cosAngle);
					//if(angle > IKBone->IKLoopAngleLimit) angle = IKBone->IKLoopAngleLimit;
					
					//cout<<"angle: "<<angle<<" "<<(float)angle*180.0f/(float)M_PI<<endl;
					
					/*if(IKLink->angleLimit)
					{
						glm::vec3 &lowerLimit=IKLink->lowerLimit;
						glm::vec3 &upperLimit=IKLink->upperLimit;
					}*/
						
					glm::mat4 rotation=glm::rotate((float)angle*180.0f/(float)M_PI,axis);
						
					for(int j=ik; j<IKBone->IKLinkNum; ++j)
					{
						PMXBone *b=pmxInfo.bones[IKBone->IKLinks[j]->linkBoneIndex]; //current link bone
						PMXBone *parent=pmxInfo.bones[b->parentBoneIndex];

						b->relativeForm = b->relativeForm * rotation;
						b->absoluteForm = parent->absoluteForm * b->relativeForm;
						
						skinMatrix[IKBone->IKLinks[j]->linkBoneIndex] = b->absoluteForm * invBindPose[IKBone->IKLinks[j]->linkBoneIndex];
					}
				
						//NOTE: Left out error-tolerance check
						//cout<<endl;
				}
			}
		}
	}
}

void VMDMotionController::advanceTime() { ++time; }
