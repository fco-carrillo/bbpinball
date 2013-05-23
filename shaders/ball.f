
uniform samplerCube texSampler;

varying vec3 fragEyeCoord;
varying vec3 fragNormal;
varying vec3 fragNormalScreen;

void main(void)
{
	vec3 normalVec = fragNormal;
	vec3 screenNormalVec = fragNormalScreen;
	vec3 eyeVec = normalize(fragEyeCoord);

	float x = min( 0.75, dot(normalVec, eyeVec) );
	float fresnel;
	fresnel = ((-5.73 * x + 9.13) * x - 4.02) * x + 1.0;

	vec3 reflVec = reflect(-eyeVec, normalVec);
	
	vec3 reflColor = vec3(fresnel * textureCube(texSampler, reflVec));
	gl_FragColor = vec4(reflColor, 1.3 - pow(1.0 - screenNormalVec.z, 0.7) );

#ifdef FALSE_COLOR
gl_FragColor = vec4(20.0,20.0,20.0,20.0)/32.0;
#endif
}



/*

const float BALL_ALPHA_BORDER_EXPONENT = 0.7;

uniform samplerCube texSampler;

varying vec3 fragEyeCoord;
varying vec3 fragNormal;
varying vec3 fragNormalScreen;

void main(void)
{
	vec3 normalVec = normalize(fragNormal);
	vec3 screenNormalVec = normalize(fragNormalScreen);
	vec3 eyeVec = normalize(fragEyeCoord);

	float x = dot(normalVec, eyeVec);
	float fresnel;
	if (x > 0.75)
		fresnel = 0.7;
	else
		fresnel = ((-5.73 * x + 9.13) * x - 4.02) * x + 1.0;

	vec3 reflVec = reflect(-eyeVec, normalVec);
	
//	float lum = dot(lightVec, normalVec);

	vec3 reflColor = vec3(fresnel * textureCube(texSampler, reflVec));
//	float specValue = pow(max(dot(reflVec, lightVec), 0.0), 60.0);

//	float atten = max(1.0 - dot(fragAtten, fragAtten), 0.0);

//	gl_FragColor = vec4(reflColor + atten * specValue, 1.0);
	gl_FragColor = vec4(reflColor, 1.0 - pow(1.0 - screenNormalVec.z, BALL_ALPHA_BORDER_EXPONENT) + 0.3 );
}
*/
