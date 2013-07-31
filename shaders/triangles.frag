#version 150 core

uniform sampler2D textureSampler;
uniform sampler2D textureSampler2;
uniform sampler2D textureSampler3;


uniform vec3 ambientColor;

uniform vec3 specularColor;
uniform float shininess;

uniform vec4 diffuseColor;

uniform vec3 halfVector; //surface orientation for shiniest spots


in vec2 UV;
in vec3 normal;

out vec4 color;


void main()
{
	if((UV[0]!=0 && UV[1]!=0))
	{
		color = vec4(0.5, 0.5, 1.0, 0.5);
		color = texture(textureSampler, UV).rgba;
		
		/*vec3 lightColor=vec3(0.5,0.5,0.5);
		vec3 lightDirection=vec3(0.01,-0.01,-0.04);
		
		float diffuse = max(0.0, dot(normal, lightDirection));
		float specular = max(0.0, dot(normal, halfVector));
		
		if(diffuse == 0.0)
			specular = 0.0;
		//else
			//specular = pow(specular, shininess); //sharpen the highlight
		
		vec3 scatteredLight = ambientColor + lightColor*diffuse;
		vec3 reflectedLight = lightColor*specular; //*strength
		
		//don't modulate the underlying color with reflected light, only with scattered light
		
		vec3 rgb=min(color.rgb * scatteredLight, vec3(1.0));
		color = vec4(rgb,texture(textureSampler, UV).a);*/
		
		//color = min(texture(textureSampler, UV) * ambientColor, vec4(1.0));
		
		//color = (texture(textureSampler3, UV).rgba*0.5 + texture(textureSampler, UV).rgba);
		
	}
	else
	{
		color = vec4(1.0, 0.5, 0.5, 1.0);
		//vec2 pos={0.1,0.1};
		
		//color = texture(textureSampler, UV).rgb;
		//color = texture(textureSampler, UV).rgb;
		//color = texture(textureSampler2, UV).rgb;
	}
}
