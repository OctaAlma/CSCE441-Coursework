#version 120

// Array of lights
uniform vec3 lPosBuf[15];
uniform vec3 lColBuf[15];

uniform vec3 ks;
uniform float s;

uniform sampler2D posTexture;
uniform sampler2D norTexture;
uniform sampler2D keTexture;
uniform sampler2D kdTexture;
uniform vec2 windowSize;

void main()
{
    vec2 tex;
    tex.x = gl_FragCoord.x/windowSize.x;
    tex.y = gl_FragCoord.y/windowSize.y;
    
    // Fetch shading data
    vec3 vPos = texture2D(posTexture, tex).rgb;
    vec3 vNor = texture2D(norTexture, tex).rgb;
    vec3 ke = texture2D(keTexture, tex).rgb;
    vec3 kd = texture2D(kdTexture, tex).rgb;
    
    // Calculate lighting here

    vec3 n = normalize(vNor);

	// Ambient Component / ambient color:
	vec3 c_A = ke;
    
	vec3 c_D = vec3(0,0,0);
	vec3 c_S = vec3(0,0,0);

	vec3 fragCol = ke;

	vec3 eye = normalize(-1.0 * vPos);

	for (int i = 0; i < 15; i++){
		// Diffuse Component:

		vec3 lightVector = normalize(lPosBuf[i] - vPos);
		c_D = c_D + kd * max(0, dot(lightVector, n) );

		// Specular Component
		vec3 h = normalize(lightVector + eye) ;
		c_S = c_S + ks * pow(max(0, dot(h, n)), s);

		vec3 color = lColBuf[i] * (c_D + c_S);
		float r = length(lPosBuf[i] - vPos);
		float attenuation = 1.0 / (1.0 + (0.0429 * r) + (0.9857 * pow(r,2)));
				
		fragCol += color * attenuation;
	}

	gl_FragColor = vec4(fragCol, 1);
    //gl_FragColor.rgb = vPos;
    //gl_FragColor.rgb = vNor;
    //gl_FragColor.rgb = ke;
    //gl_FragColor.rgb = kd;
}