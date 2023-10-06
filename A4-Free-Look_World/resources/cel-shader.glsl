#version 120

uniform vec3 lightPos1;
uniform vec3 lightCol1;

uniform vec3 lightPos2;
uniform vec3 lightCol2;

uniform vec3 ka;
uniform vec3 kd;
uniform vec3 ks;
uniform float s;

// Necessary variables
varying vec3 vNor;  // Contains the normalized normal of the position vector 
varying vec3 vPos; // Contains the position of fragment in camera space

float quantizeCol(float C){
    if(C < 0.25) {
        C = 0.0;
    } else if(C < 0.5) {
        C = 0.25;
    } else if(C < 0.75) {
        C = 0.5;
    } else if(C < 1.0) {
        C = 0.75;
    } else {
        C = 1.0;
    }

    return C;
}

void main()
{

    vec3 eye = normalize(-1 * vPos);

    // Just like the Silhouette shader, make the Silhouettes black:
    if (dot (vNor, eye) < 0.3f){
        gl_FragColor = vec4(0, 0, 0, 1);
    }

    else{

        vec3 n = normalize(vNor);

        // Ambient Component / ambient color:
        vec3 c_A = ka;

        // Diffuse Component:
        vec3 eye = normalize(-1 * vPos);

        vec3 lightVector1 = normalize( lightPos1 - vPos);
        vec3 c_D1 = kd * max(0, dot(lightVector1, n) );

        vec3 lightVector2 = normalize(lightPos2 - vPos);
        vec3 c_D2 = kd * max(0, dot( lightVector2, n));
        

        // Specular Component
        vec3 h1 = normalize(lightVector1 + eye) ;
        vec3 c_S1 = ks * pow(max(0, dot(h1, n)), s);

        vec3 h2 = normalize(lightVector2 + eye);
        vec3 c_S2 = ks * pow(max(0, dot(h2, n)), s);


        // Actual output color:
        // OLD: gl_FragColor = vec4(c_A1 + c_D1 + c_S1, 1);

        float red = lightCol1.r * ( c_A.r +  c_D1.r + c_S1.r) + lightCol2.r * ( c_A.r + c_D2.r + c_S2.r);
        float green = lightCol1.g * ( c_A.g +  c_D1.g + c_S1.g) + lightCol2.g * ( c_A.g + c_D2.g + c_S2.g);
        float blue = lightCol1.b * ( c_A.b +  c_D1.b + c_S1.b) + lightCol2.b * ( c_A.b + c_D2.b + c_S2.b);
    
        red = quantizeCol(red);
        green = quantizeCol(green);
        blue = quantizeCol(blue);

        gl_FragColor = vec4(red, green, blue, 1);
    }
}