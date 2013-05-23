attribute vec3 position;
attribute vec2 tex1;

uniform mat4 projMatrix;

varying vec2 fragTexCoord;

void main(void)
{
	gl_Position = projMatrix * vec4(position.x, position.y, position.z, 1.0);
	fragTexCoord = tex1;
}
