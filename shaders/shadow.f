
uniform sampler2D texSampler;

varying vec2 fragTexCoord;

void main(void)
{
	vec4 color = texture2D(texSampler, fragTexCoord);
	gl_FragColor = vec4(0.0, 0.0, 0.0, color.a);

#ifdef FALSE_COLOR
gl_FragColor = vec4(1.0,1.0,1.0,1.0)/32.0;
#endif
}
