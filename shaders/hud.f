uniform sampler2D texSampler;
uniform vec4 hudColor;

varying vec2 fragTexCoord;

void main(void)
{
	gl_FragColor = hudColor * texture2D(texSampler, fragTexCoord);

#ifdef FALSE_COLOR
gl_FragColor = vec4(1.0,1.0,1.0,1.0)/32.0;
#endif
}
