attribute vec3 position;
attribute vec2 tex1;

uniform mat4 projMatrix;

uniform float ledNumber;

varying vec2 fragTexCoord;

void main(void)
{
	gl_Position = projMatrix * vec4(position.x, position.y, position.z, 1.0);
	fragTexCoord = vec2(1.0 - (1.0 - tex1.x + ledNumber) * 64.0 / 1024.0, tex1.y);
}
