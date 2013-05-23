uniform samplerCube texSampler;
uniform sampler2D texSampler2;

uniform float outputMultiplier3;

varying vec3 fragEyeCoord;
varying vec3 fragNormal;
varying vec2 fragTexCoord;

void main(void)
{
	vec3 normalVec = normalize(fragNormal);
	vec3 eyeVec = normalize(fragEyeCoord);

	float x = min( 0.75, dot(normalVec, eyeVec) );
	float fresnel = ((-5.73 * x + 9.13) * x - 4.02) * x + 1.0;

	vec3 reflVec = reflect(-eyeVec, normalVec);
	
	vec4 reflColor = fresnel * textureCube(texSampler, reflVec);
	vec4 texColor = texture2D(texSampler2, fragTexCoord);
	vec4 finalColor = mix(reflColor, texColor, texColor.a);

	gl_FragColor = finalColor;

#ifdef FALSE_COLOR
gl_FragColor = vec4(16.0,16.0,16.0,16.0)/32.0;
#endif


//gl_FragColor = vec4(1.0,0.0,0.0,1.0);
}


/*

uniform samplerCube texSampler;
uniform sampler2D texSampler2;

uniform float outputMultiplier;

varying vec3 fragEyeCoord;
varying vec3 fragNormal;
//varying vec3 fragLightDir;
//varying vec3 fragAtten;

varying vec2 fragTexCoord;

// Muuta arvot myös tex.f
const float MULTIPLIER_EXPONENT = 2.0;
const float MULTIPLIER_AMOUNT = 0.3;
const float COLOR_EXPONENT = 4.0;

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

	vec4 texColor = texture2D(texSampler2, fragTexCoord);
//	gl_FragColor = vec4((reflColor + atten * specValue) * 0.5 + texColor, 1.0);

	vec3 finalColor = mix(reflColor, texColor.rgb, texColor.a);

	if (outputMultiplier == 1.0)
	{
		gl_FragColor = vec4(finalColor, 1.0);
	}
	else
	{
		gl_FragColor = vec4(pow(outputMultiplier, MULTIPLIER_EXPONENT) * MULTIPLIER_AMOUNT * pow(finalColor, vec3(COLOR_EXPONENT, COLOR_EXPONENT, COLOR_EXPONENT)), 1.0);
	}
}

*/