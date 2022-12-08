#version 150
// ^ Change this to version 130 if you have compatibility issues

// This is a fragment shader. If you've opened this file first, please
// open and read lambert.vert.glsl before reading on.
// Unlike the vertex shader, the fragment shader actually does compute
// the shading of geometry. For every pixel in your program's output
// screen, the fragment shader is run for every bit of geometry that
// particular pixel overlaps. By implicitly interpolating the position
// data passed into the fragment shader by the vertex shader, the fragment shader
// can compute what color to apply to its pixel based on things like vertex
// position, light position, and vertex color.

uniform sampler2D u_Texture; // An addition to lambert.frag.glsl that makes use of a sampler2D to apply texture colors to a surface.

uniform int u_Time; // An alteration to lambert.frag.glsl so that it includes a time variable as in Homework 5's OpenGL Fun,
                    // and uses this variable to animate the UVs on a LAVA block and WATER block.

uniform ivec2 u_Dimensions; // Screen dimensions

uniform vec3 u_Eye; // Camera pos

// These are the interpolated values out of the rasterizer, so you can't know
// their specific values without knowing the vertices that contributed to them
in vec4 fs_Pos;
in vec4 fs_Nor;
in vec4 fs_LightVec;
in vec2 fs_UV;
in float fs_Anim;

out vec4 out_Col; // This is the final output color that you will see on your
                  // screen for the pixel that is currently being processed.

const float PI = 3.14159265359;
const float TWO_PI = 6.28318530718;

float random1(vec3 p) {
    return fract(sin(dot(p,vec3(127.1, 311.7, 191.999)))
                 *43758.5453);
}

float mySmoothStep(float a, float b, float t) {
    t = smoothstep(0, 1, t);
    return mix(a, b, t);
}

float cubicTriMix(vec3 p) {
    vec3 pFract = fract(p);
    float llb = random1(floor(p) + vec3(0,0,0));
    float lrb = random1(floor(p) + vec3(1,0,0));
    float ulb = random1(floor(p) + vec3(0,1,0));
    float urb = random1(floor(p) + vec3(1,1,0));

    float llf = random1(floor(p) + vec3(0,0,1));
    float lrf = random1(floor(p) + vec3(1,0,1));
    float ulf = random1(floor(p) + vec3(0,1,1));
    float urf = random1(floor(p) + vec3(1,1,1));

    float mixLoBack = mySmoothStep(llb, lrb, pFract.x);
    float mixHiBack = mySmoothStep(ulb, urb, pFract.x);
    float mixLoFront = mySmoothStep(llf, lrf, pFract.x);
    float mixHiFront = mySmoothStep(ulf, urf, pFract.x);

    float mixLo = mySmoothStep(mixLoBack, mixLoFront, pFract.z);
    float mixHi = mySmoothStep(mixHiBack, mixHiFront, pFract.z);

    return mySmoothStep(mixLo, mixHi, pFract.y);
}

float fbm(vec3 p) {
    float amp = 0.5;
    float freq = 4.0;
    float sum = 0.0;
    for(int i = 0; i < 8; i++) {
        sum += cubicTriMix(p * freq) * amp;
        amp *= 0.5;
        freq *= 2.0;
    }
    return sum;
}

// rotate r on (1, 0, 0) by alpha and normalized
vec3 rotateOnRightGlobal(vec3 r, float alpha) {
    mat4 rotMat = mat4(
                    1, 0, 0, 0,
                    0, cos(alpha), sin(alpha), 0,
                    0, -sin(alpha), cos(alpha), 0,
                    0, 0, 0, 1
                  );
    vec4 v = rotMat * vec4(r, 1);
    return normalize(v.xyz);
}

vec2 sphereToUV(vec3 p) {
    float phi = atan(p.z, p.x); // range: [-pi, pi]
    if(phi < 0) {
        phi += TWO_PI;
    }
    float theta = acos(p.y); // range: [0, pi]
    return vec2(1 - phi / TWO_PI, 1 - theta / PI);
}

void main()
{
    vec2 uv = fs_UV;
    if (fs_Anim != 0) {
        uv += vec2((u_Time % 123) / 1234.f, 0);
    }
    // Material base color (before shading)
    vec4 diffuseColor = texture(u_Texture, uv);

    vec3 sunDir = rotateOnRightGlobal(vec3(0, 0, -1.0), u_Time * 0.01); // sun direction
    vec4 lightDir = vec4(sunDir, 0);

    // Calculate the diffuse term for Lambert shading
    float diffuseTerm = dot(normalize(fs_Nor), normalize(lightDir));
    // Avoid negative lighting values
    diffuseTerm = clamp(diffuseTerm, 0, 1);

    float ambientTerm = 0.5;

    vec2 sunUV = sphereToUV(sunDir);
    vec3 diffuseLight = vec3(diffuseTerm);
    vec3 ambientLight = vec3(ambientTerm);

    if (sunUV.y <= 0.2) { // night
        diffuseLight *= vec3(140, 80, 100) / 255.0;
        ambientLight *= vec3(100, 80, 110) / 255.0;
    } else if (sunUV.y < 0.3) { // between night and sunrise/sunset
        float p = (sunUV.y - 0.2) / 0.1;
        diffuseLight *= mix(vec3(140, 80, 100), vec3(254, 224, 190), p) / 255.0;
        ambientLight *= mix(vec3(100, 80, 110), vec3(184, 129, 174), p) / 255.0;
    } else if (sunUV.y < 0.5) { // sunrise or sunset
        diffuseLight *= vec3(240, 224, 190) / 255.0;
        ambientLight *= vec3(184, 129, 174) / 255.0;
    } else if (sunUV.y < 0.6) { // between sunrise/sunset and daytime
        float p = (sunUV.y - 0.5) / 0.1;
        diffuseLight *= mix(vec3(240, 224, 190), vec3(240, 240, 190), p) / 255.0;
        ambientLight *= mix(vec3(184, 129, 174) , vec3(188, 215, 240), p) / 255.0;
    } else if (sunUV.y < 0.8){ // general day time
        diffuseLight *= vec3(240, 240, 190) / 255.0;
        ambientLight *= vec3(188, 215, 240) / 255.0;
    } else { // noon
        diffuseLight *= vec3(255, 255, 240) / 255.0;
        ambientLight *= vec3(197, 232, 251) / 255.0;
    }

    // Compute final shaded color
    //Add a small float value to the color multiplier
    //to simulate ambient lighting. This ensures that faces that are not
    //lit by our point light are not completely black.
    diffuseColor = vec4(diffuseLight + ambientLight, 1.f) * diffuseColor;

    // Distance fog feature
    vec3 toEye = fs_Pos.xyz - u_Eye;
    float fog = smoothstep(0.9, 1.f, min(1.f, length(toEye.xz / 192.f)));
    vec2 screenSpaceUVs = gl_FragCoord.xy / vec2(u_Dimensions);
    vec4 textureColor = vec4(texture(u_Texture, screenSpaceUVs).rgb, 1.f);
    diffuseColor = mix(diffuseColor, textureColor, fog);

    out_Col = vec4(diffuseColor.rgb, diffuseColor.a);
}
