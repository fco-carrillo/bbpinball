attribute vec3 position;

uniform vec2 hudPos;
uniform vec2 hudScale;

void main(void)
{
	gl_Position = vec4(position.x * 2.0 * -hudScale.x + hudPos.x * 2.0 - 1.0 + hudScale.x * 2.0, position.y * 2.0 * -hudScale.y - hudPos.y * 2.0 + 1.0, 0.0, 1.0);
}
