attribute vec3 position;
attribute vec2 tex1;

uniform vec2 hudPos;
uniform vec2 hudScale;
uniform vec2 hudTexPos;
uniform vec2 hudTexScale;

varying vec2 fragTexCoord;

void main(void)
{
	gl_Position = vec4(position.x * 2.0 * -hudScale.x + hudPos.x * 2.0 - 1.0 + hudScale.x * 2.0, position.y * 2.0 * -hudScale.y - hudPos.y * 2.0 + 1.0, 0.0, 1.0);
	fragTexCoord = tex1 * hudTexScale + (vec2(1.0, 1.0) - hudTexPos);
}
