CRUD

/*
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
	
	//fresnel = 1.0;

	vec3 reflVec = reflect(-eyeVec, normalVec);

	gl_FragColor = vec4(outputMultiplier * vec3(fresnel * textureCube(texSampler, reflVec)), 1.0);
}
*/
