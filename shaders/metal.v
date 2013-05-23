attribute vec3 position;
attribute vec3 normal;
attribute vec2 tex1;

uniform mat4 projMatrix;
uniform vec3 eyePosOS;

varying vec3 fragNormal;
varying vec3 fragEyeCoord;

void main(void)
{
	gl_Position = projMatrix * vec4(position, 1.0);
	
	fragEyeCoord = eyePosOS - position;
	fragNormal = normal;
}


/*
attribute vec3 position;
attribute vec3 normal;
attribute vec2 tex1;

uniform mat4 worldMatrix;
uniform mat4 worldNormalMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;
//uniform vec3 lightPos;

uniform vec3 eyePos;

varying vec3 fragNormal;
//varying vec3 fragLightDir;
varying vec3 fragEyeCoord;
//varying vec3 fragAtten;

//const float lightRange = 4000.0;

void main(void)
{
	gl_Position = projMatrix * vec4(position, 1.0);
	
	vec3 worldPos = vec3(worldMatrix * vec4(position, 1.0));

	fragEyeCoord = eyePos - worldPos;
//	fragLightDir = lightPos - worldPos;
	fragNormal = vec3(worldNormalMatrix * vec4(normal, 1.0));
//	fragAtten = fragLightDir / vec3(lightRange, lightRange, lightRange);
}
*/
