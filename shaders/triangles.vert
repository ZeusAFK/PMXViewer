#version 150 core

uniform mat4 MVP;

//Vertex Properties
in vec4 vPosition;
in vec2 vUV;
in vec3 vNormal;

in vec4 vBoneIndices;
in vec4 vBoneWeights;
in float vWeightFormula;

out vec2 UV;
out vec3 normal;

uniform mat4 Bones[150];

void main()
{
	vec4 newVertex;
	vec4 newNormal;
	
	int index1, index2, index3, index4, weightFormula;

	index1 = int(vBoneIndices.x);
	index2 = int(vBoneIndices.y);
	index3 = int(vBoneIndices.z);
	index4 = int(vBoneIndices.w);

	weightFormula = int(vWeightFormula);
	
	if(weightFormula==0) //BDEF1
	{
		newVertex  = (Bones[index1] * vPosition);
		newNormal  = (Bones[index1] * vec4(vNormal, 0.0)) * vBoneWeights.x;
	}
	else if(weightFormula==1) //BDEF2
	{
		newVertex  = (Bones[index1] * vPosition)          * vBoneWeights.x;
		newVertex += (Bones[index2] * vPosition)          * (1.0 - vBoneWeights.x);
		
		newNormal  = (Bones[index1] * vec4(vNormal, 0.0)) * vBoneWeights.x;
		newNormal += (Bones[index2] * vec4(vNormal, 0.0)) * (1.0 - vBoneWeights.x);
	}
	else if(weightFormula==2) //BDEF4
	{
		newVertex  = (Bones[index1] * vPosition)          * vBoneWeights.x;
		newNormal  = (Bones[index1] * vec4(vNormal, 0.0)) * vBoneWeights.x;
		
		newVertex += (Bones[index2] * vPosition)          * vBoneWeights.y;
		newNormal += (Bones[index2] * vec4(vNormal, 0.0)) * vBoneWeights.y;
		
		newVertex += (Bones[index3] * vPosition)          * vBoneWeights.z;
		newNormal += (Bones[index3] * vec4(vNormal, 0.0)) * vBoneWeights.z;
		
		newVertex += (Bones[index4] * vPosition)          * vBoneWeights.w;
		newNormal += (Bones[index4] * vec4(vNormal, 0.0)) * vBoneWeights.w;
	}
	else // TODO: SDEF (weightFormula==3), QDEF (weightFormula==4)
	{
		newVertex = vPosition;
		newNormal = vec4(vNormal, 0.0);
	}
	
	newVertex.w = 1.0;

	gl_Position = MVP * newVertex;
	UV 		    = vUV;
	normal      = normalize(newNormal.xyz);
}
