CRUD

/*
attribute vec3 position;
attribute vec3 normal;
attribute vec2 tex1;

uniform mat4 worldMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

uniform vec3 eyePos;

varying vec3 fragNormal;
varying vec3 fragEyeCoord;

void main(void)
{
	gl_Position = projMatrix * vec4(position, 1.0);

	fragEyeCoord = eyePos - vec3(worldMatrix * vec4(position, 1.0));
	fragNormal = vec3(worldMatrix * vec4(normal, 1.0));
}
*/
