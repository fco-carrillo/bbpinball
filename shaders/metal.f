uniform samplerCube texSampler;

uniform float outputMultiplier;

varying vec3 fragEyeCoord;
varying vec3 fragNormal;

void main(void)

{
	vec3 normalVec = normalize(fragNormal);
	vec3 eyeVec = normalize(fragEyeCoord);

	float x = dot(normalVec, eyeVec);
	float fresnel;
	if (x > 0.75)
		fresnel = 0.7;
	else
		fresnel = ((-5.73 * x + 9.13) * x - 4.02) * x + 1.0;

	vec3 reflVec = reflect(-eyeVec, normalVec);
	
	vec3 reflColor = vec3(fresnel * textureCube(texSampler, reflVec));

	gl_FragColor = vec4(outputMultiplier * reflColor, 1.0);

#ifdef FALSE_COLOR
gl_FragColor = vec4(22.0,22.0,22.0,22.0)/32.0;
#endif


//gl_FragColor = vec4(1.0,0.0,0.0,1.0);
}




/*
uniform samplerCube texSampler;

uniform float outputMultiplier;

varying vec3 fragEyeCoord;
varying vec3 fragNormal;
//varying vec3 fragLightDir;
//varying vec3 fragAtten;

void main(void)

{
	vec3 normalVec = normalize(fragNormal);
	vec3 eyeVec = normalize(fragEyeCoord);
//	vec3 lightVec = normalize(fragLightDir);

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
	gl_FragColor = vec4(outputMultiplier * reflColor, 1.0);
}

*/

