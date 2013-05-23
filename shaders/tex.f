// Yleinen teksturointi shader

uniform sampler2D texSampler;
uniform float outputMultiplier2;

varying vec2 fragTexCoord;

void main(void)
{
	vec4 vTexel = texture2D(texSampler, fragTexCoord);

	gl_FragColor.rgb = outputMultiplier2 * vTexel.rgb;
	gl_FragColor.a   = vTexel.a;

#ifdef FALSE_COLOR
gl_FragColor = vec4(1.0,1.0,1.0,1.0)/32.0;
#endif
}


/*
// Yleinen teksturointi shader

uniform sampler2D texSampler;

uniform float outputMultiplier;

varying vec2 fragTexCoord;

// Muuta arvot myös metal3.f
const float MULTIPLIER_EXPONENT = 2.0;
const float MULTIPLIER_AMOUNT = 0.4;
const float COLOR_EXPONENT = 1.0;

void main(void)
{
	// outputMultiplier:
	// 0.0 : kamera katsoo suoraan alaspäin
	// < 1.0 : kamera lähestyy vaakatasoa
	// 1.0 : piirretään normaalisti
	if (outputMultiplier == 1.0)
	{
		gl_FragColor = texture2D(texSampler, fragTexCoord);
	}
	else
	{
		vec4 texColor = texture2D(texSampler, fragTexCoord);
		gl_FragColor = vec4(pow(outputMultiplier, MULTIPLIER_EXPONENT) * MULTIPLIER_AMOUNT * pow(texColor.rgb, vec3(COLOR_EXPONENT, COLOR_EXPONENT, COLOR_EXPONENT)), texColor.a);
	}
}
*/