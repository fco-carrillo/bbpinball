uniform vec4 hudColor;

void main(void)
{
	gl_FragColor = hudColor;

#ifdef FALSE_COLOR
gl_FragColor = vec4(3.0,3.0,3.0,3.0)/32.0;
#endif
}
