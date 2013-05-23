attribute vec3 position;

uniform mat4 projMatrix;

void main(void)
{
	gl_Position = projMatrix * vec4(position.x, position.y, position.z, 1.0);
}
