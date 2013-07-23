#version 420 core

uniform mat4 MVP;

//Vertex Properties
layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec2 vUV;
layout(location = 2) in vec3 vNormal;

layout(location = 3) in vec4 vBoneIndices;
layout(location = 4) in vec4 vBoneWeights;
layout(location = 5) in float vWeightFormula;

out vec2 UV;
out vec3 normal;

uniform mat4 Bones[130];

void main()
{
	vec4 newVertex;
	vec4 newNormal;
	
	int index1,index2,index3,index4,weightFormula;
	
	
	index1=int(vBoneIndices.x);
	index2=int(vBoneIndices.y);
	index3=int(vBoneIndices.z);
	index4=int(vBoneIndices.w);
	weightFormula=int(vWeightFormula);
	
	if(1==1)
	{
		//newVertex = Bones[index1][0];
		//newVertex = (Bones[index1] * vPosition) * (1.0-vBoneWeights.x);
		
		if(weightFormula==0) //BDEF1
		{
			newVertex+=(Bones[index1] * vPosition);
		}
		else if(weightFormula==1) //BDEF2
		{
			newVertex+=(Bones[index1] * vPosition) * vBoneWeights.x;
			newVertex+=(Bones[index2] * vPosition); //Weight of 1.0
		}
		else if(weightFormula==2)
		{
			newVertex+=(Bones[index1] * vPosition) * vBoneWeights.x;
			newNormal+= (Bones[index1] * vec4(vNormal, 0.0)) * vBoneWeights.x;
			
			newVertex+= (Bones[index2] * vPosition) * vBoneWeights.y;
			newNormal+= (Bones[index2] * vec4(vNormal, 0.0)) * vBoneWeights.y;
			
			newVertex+= (Bones[index3] * vPosition) * vBoneWeights.z;
			newNormal+= (Bones[index3] * vec4(vNormal, 0.0)) * vBoneWeights.z;
			
			newVertex+= (Bones[index4] * vPosition) * vBoneWeights.w;
			newNormal+= (Bones[index4] * vec4(vNormal, 0.0)) * vBoneWeights.w;
		}
		else if(weightFormula==3) //SDEF
		{
			newVertex=vec4(5.0f,5.0f,5.0f,1.0);
		}
		gl_Position = MVP * newVertex;
		
		UV = vUV;
		normal = vNormal;
	}
	else
	{
		gl_Position = MVP * vPosition;
		
		UV = vec2(0.0f,0.0f);
		normal = vec3(0.0f,0.0f,0.0f);
	}
	
	
	
	
}
