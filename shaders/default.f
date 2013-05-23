
void main(void)
{
	gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);

#ifdef FALSE_COLOR
gl_FragColor = vec4(1.0,1.0,1.0,1.0)/32.0;
#endif
}
