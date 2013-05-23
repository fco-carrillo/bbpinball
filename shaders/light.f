
uniform sampler2D texSampler;
uniform float lightAlpha;

varying vec2 fragTexCoord;

void main(void)
{
	vec4 color = texture2D(texSampler, fragTexCoord);
	gl_FragColor = vec4(color.x, color.y, color.z, color.w * lightAlpha);

#ifdef FALSE_COLOR
gl_FragColor = vec4(2.0,2.0,2.0,2.0)/32.0;
#endif
}



/*
uniform sampler2D texSampler;

varying vec2 fragTexCoord;
varying float fragLightAlpha;

void main(void)
{
	vec4 color = texture2D(texSampler, fragTexCoord);
	gl_FragColor = vec4(color.x, color.y, color.z, color.w * fragLightAlpha);
}
*/
