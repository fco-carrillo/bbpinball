CRUD


/*
uniform samplerCube texSampler;
uniform sampler2D texSampler2;

varying vec3 fragEyeCoord;
varying vec3 fragNormal;
varying vec2 fragTexCoord;
varying vec3 fragLightDir;
varying vec3 fragAtten;

void main(void)
{
	vec3 normalVec = normalize(fragNormal);
	vec3 eyeVec = normalize(fragEyeCoord);
	vec3 lightVec = normalize(fragLightDir);

	float x = dot(normalVec, eyeVec);
	float fresnel;
	if (x > 0.75)
	
		fresnel = 0.7;
	else
		fresnel = ((-5.73 * x + 9.13) * x - 4.02) * x + 1.0;

	vec3 reflVec = reflect(-eyeVec, normalVec);
	float specValue = pow(max(dot(reflVec, lightVec), 0.0), 40.0) * max(1.0 - dot(fragAtten, fragAtten), 0.0);
	vec3 specColor = vec3(specValue, specValue, specValue);

	vec4 texColor = texture2D(texSampler2, fragTexCoord);
	
	gl_FragColor = mix(vec4(texColor.rgb + specColor * 0.5, 1.0),
	vec4(vec3(fresnel * textureCube(texSampler, reflVec)) + specColor, 0.4), 1.0 - texColor.a);
}
*/
