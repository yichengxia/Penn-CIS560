#version 150

uniform mat4 u_ViewProj;    // We're actually passing the inverse of the viewproj
                            // from our CPU, but it's named u_ViewProj so we don't
                            // have to bother rewriting our ShaderProgram class

uniform ivec2 u_Dimensions; // Screen dimensions

uniform vec3 u_Eye; // Camera pos

uniform float u_Time;

out vec3 outColor;

const float PI = 3.14159265359;
const float TWO_PI = 6.28318530718;

// Sunset palette
const vec3 sunset[5] = vec3[](vec3(255, 229, 119) / 255.0,
                               vec3(254, 192, 81) / 255.0,
                               vec3(253, 230, 178) / 255.0,
                               vec3(240, 240, 190) / 255.0,
                               vec3(128, 200, 255) / 255.0);

// Dusk palette
const vec3 dusk[5] = vec3[](vec3(140, 80, 100) / 255.0,
                            vec3(100, 80, 110) / 255.0,
                            vec3(60, 72, 120) / 255.0,
                            vec3(50, 48, 100) / 255.0,
                            vec3(47, 38, 70) / 255.0);

// DayLight palette
const vec3 dayLight[5] = vec3[](vec3(225, 245, 220) / 255.0,
                               vec3(218, 255, 225) / 255.0,
                               vec3(180, 245, 230) / 255.0,
                               vec3(170, 240, 235) / 255.0,
                               vec3(128, 225, 255) / 255.0);
// DayCounter palette
const vec3 dayCounterLight[5] = vec3[](vec3(110, 200, 255) / 255.0,
                               vec3(100, 195, 250) / 255.0,
                               vec3(80, 190, 245) / 255.0,
                               vec3(60, 185, 243) / 255.0,
                               vec3(50, 180, 241) / 255.0);


const vec3 sunColor = vec3(255, 255, 190) / 255.0;
const vec3 starColor = vec3(231, 250, 255) / 255.0;

// convert to sphere coord in radian (from 3D to 2D)
vec2 sphereToUV(vec3 p) {
    float phi = atan(p.z, p.x); // range: [-pi, pi]
    if(phi < 0) {
        phi += TWO_PI;
    }
    float theta = acos(p.y); // range: [0, pi]
    return vec2(1 - phi / TWO_PI, 1 - theta / PI);
}

// get general dayTime color (near sun)
vec3 uvToDay(vec2 uv, vec2 sunUV) {
    float t = uv.y - sunUV.y;
    if (t < -0.5) {
        return dayLight[4];
    } else if (t < -0.25) {
        return mix(dayLight[3], dayLight[4], (-t - 0.25) / 0.25);
    } else if (t < -0.12) {
        return mix(dayLight[2], dayLight[3], (-t - 0.12) / 0.13);
    } else if (t < -0.08) {
        return mix(dayLight[1], dayLight[2], (-t - 0.08) / 0.04);
    } else if (t < -0.04) {
        return mix(dayLight[0], dayLight[1], (-t - 0.04) / 0.04);
    } else if (t < 0.02) {
        return dayLight[0];
    } else if (t < 0.06) {
        return mix(dayLight[0], dayLight[1], (t - 0.02) / 0.04);
    } else if(t < 0.1) {
        return mix(dayLight[1], dayLight[2], (t - 0.06) / 0.04);
    } else if(t < 0.15) {
        return mix(dayLight[2], dayLight[3], (t - 0.1) / 0.05);
    } else if(t < 0.25) {
        return mix(dayLight[3], dayLight[4], (t - 0.15) / 0.1);
    }
    return dayLight[4];
}

// get sunrise/sunset color (near sun)
vec3 uvToSunrise(vec2 uv, vec2 sunUV) {
    float t = uv.y -  sunUV.y;
    // interpolate based on theta difference between the fragCoordDir and sunDir
    if (t < -0.5) {
        return dayLight[4];
    } else if (t < -0.25) {
        return mix(dayLight[3], dayLight[4], (-t - 0.25) / 0.25);
    } else if (t < -0.12) {
        return mix(dayLight[2], dayLight[3], (-t - 0.12) / 0.13);
    } else if (t < -0.08) {
        return mix(dayLight[1], dayLight[2], (-t - 0.08) / 0.04);
    } else if (t < -0.04) {
        return mix(dayLight[0], dayLight[1], (-t - 0.04) / 0.04);
    } else if(t < 0) {
        return mix(sunset[0], dayLight[0], (-t) / 0.04);
    } else if (t < 0.02) {
        return sunset[0];
    } else if (t < 0.06) {
        return mix(sunset[0], sunset[1], (t - 0.02) / 0.04);
    } else if(t < 0.1) {
        return mix(sunset[1], sunset[2], (t - 0.06) / 0.04);
    } else if(t < 0.15) {
        return mix(sunset[2], sunset[3], (t - 0.1) / 0.05);
    } else if(t < 0.25) {
        return mix(sunset[3], sunset[4], (t - 0.15) / 0.1);
    }
    return sunset[4];
}

// between sunrise/sunset and day time (near sun)
vec3 uvToSunrise2(vec2 uv, vec2 sunUV) {
    float t = uv.y -  sunUV.y;
    float p = (sunUV.y - 0.5) / 0.1;
    if (t < -0.5) {
        return dayLight[4];
    } else if (t < -0.25) {
        return mix(dayLight[3], dayLight[4], (-t - 0.25) / 0.25);
    } else if (t < -0.12) {
        return mix(dayLight[2], dayLight[3], (-t - 0.12) / 0.13);
    } else if (t < -0.08) {
        return mix(dayLight[1], dayLight[2], (-t - 0.08) / 0.04);
    } else if (t < -0.04) {
        return mix(dayLight[0], dayLight[1], (-t - 0.04) / 0.04);
    } else if(t < 0) {
        return mix(mix(sunset[0], dayLight[0], p), dayLight[0], (-t) / 0.04);
    } else if (t < 0.02) {
        return mix(sunset[0], dayLight[0], p);
    } else if (t < 0.06) {
        return mix(mix(sunset[0], dayLight[0], p), mix(sunset[1], dayLight[1], p), (t - 0.02) / 0.04);
    } else if(t < 0.1) {
        return mix(mix(sunset[1], dayLight[1], p), mix(sunset[2], dayLight[2], p), (t - 0.06) / 0.04);
    } else if(t < 0.15) {
        return mix(mix(sunset[2], dayLight[2], p), mix(sunset[3], dayLight[3], p), (t - 0.1) / 0.05);
    } else if(t < 0.25) {
        return mix(mix(sunset[3], dayLight[3], p), mix(sunset[4], dayLight[4], p), (t - 0.15) / 0.1);
    }
    return mix(sunset[4], dayLight[4], p);
}

// between night and sunrise (near sun)
vec3 uvToSunrise3(vec2 uv, vec2 sunUV) {
    float t = uv.y - sunUV.y;
    float p = (sunUV.y - 0.2) / 0.1;
    if (t < -0.5) {
        return mix(dusk[4], sunset[4], p);
    } else if (t < -0.25) {
        return mix(mix(dusk[3], sunset[3], p), mix(dusk[4], sunset[4], p), (-t - 0.25) / 0.25);
    } else if (t < -0.12) {
        return mix(mix(dusk[2], sunset[2], p), mix(dusk[3], sunset[3], p), (-t - 0.12) / 0.13);
    } else if (t < -0.08) {
        return mix(mix(dusk[1], sunset[1], p), mix(dusk[2], sunset[2], p), (-t - 0.08) / 0.04);
    } else if (t < -0.04) {
        return mix(mix(dusk[0], sunset[0], p), mix(dusk[1], sunset[1], p), (-t- 0.04) / 0.04);
    } else if (t < 0.02) {
        return mix(dusk[0], sunset[0], p);
    } else if (t < 0.06) {
        return mix(mix(dusk[0], sunset[0], p), mix(dusk[1], sunset[1], p), (t - 0.02) / 0.04);
    } else if(t < 0.1) {
        return mix(mix(dusk[1], sunset[1], p), mix(dusk[2], sunset[2], p), (t - 0.06) / 0.04);
    } else if(t < 0.15) {
        return mix(mix(dusk[2], sunset[2], p), mix(dusk[3], sunset[3], p), (t - 0.1) / 0.05);
    } else if(t < 0.25) {
        return mix(mix(dusk[3], sunset[3], p), mix(dusk[4], sunset[4], p), (t - 0.15) / 0.1);
    }
    return mix(dusk[4], sunset[4], p);
}

// get general day time color (opposite to sun)
vec3 uvToDayCounter(vec2 uv, vec2 sunUV) {
    float t = abs(uv.y - sunUV.y);
    if (t < 0.1) {
        return dayCounterLight[0];
    } else if (t < 0.15) {
        return mix(dayCounterLight[0], dayCounterLight[1], (t - 0.1) / 0.05);
    } else if (t < 0.2) {
        return mix(dayCounterLight[1], dayCounterLight[2], (t - 0.15) / 0.05);
    } else if (t < 0.25) {
        return mix(dayCounterLight[2], dayCounterLight[3], (t - 0.2) / 0.05);
    } else if (t < 0.35) {
        return mix(dayCounterLight[3], dayCounterLight[4], (t - 0.25) / 0.1);
    }
    return dayCounterLight[4];
}

// get sunset/sunrise/night color (opposite to sun)
vec3 uvToDusk(vec2 uv, vec2 sunUV) {
    float t = abs(uv.y - sunUV.y);
    if (t < -0.5) {
        return dusk[4];
    } else if (t < -0.25) {
        return mix(dusk[3], dusk[4], (-t - 0.25) / 0.25);
    } else if (t < -0.12) {
        return mix(dusk[2], dusk[3], (-t - 0.12) / 0.13);
    } else if (t < -0.08) {
        return mix(dusk[1], dusk[2], (-t - 0.08) / 0.04);
    } else if (t < -0.04) {
        return mix(dusk[0], dusk[1], (-t- 0.04) / 0.04);
    } else if (t < 0.02) {
        return dusk[0];
    } else if (t < 0.06) {
        return mix(dusk[0], dusk[1], (t - 0.02) / 0.04);
    } else if(t < 0.1) {
        return mix(dusk[1], dusk[2], (t - 0.06) / 0.04);
    } else if(t < 0.15) {
        return mix(dusk[2], dusk[3], (t - 0.1) / 0.05);
    } else if(t < 0.25) {
        return mix(dusk[3], dusk[4], (t - 0.15) / 0.1);
    }
    return dusk[4];
}

// between sunrise/sunset and day time (opposite to sun)
vec3 uvToDusk2(vec2 uv, vec2 sunUV) {
    float t = abs(uv.y - sunUV.y);
    float p = (sunUV.y - 0.5) / 0.1;
    if (t < -0.5) {
        return mix(dusk[4], dayCounterLight[4], p);
    } else if (t < -0.25) {
        return mix(mix(dusk[3], dayCounterLight[3], p), mix(dusk[4], dayCounterLight[4], p), (-t - 0.25) / 0.25);
    } else if (t < -0.12) {
        return mix(mix(dusk[2], dayCounterLight[2], p), mix(dusk[3], dayCounterLight[3], p), (-t - 0.12) / 0.13);
    } else if (t < -0.08) {
        return mix(mix(dusk[1], dayCounterLight[1], p), mix(dusk[2], dayCounterLight[2], p), (-t - 0.08) / 0.04);
    } else if (t < -0.04) {
        return mix(mix(dusk[0], dayCounterLight[0], p), mix(dusk[1], dayCounterLight[1], p), (-t- 0.04) / 0.04);
    } else if (t < 0.02) {
        return mix(dusk[0], dayCounterLight[0], p);
    } else if (t < 0.06) {
        return mix(mix(dusk[0], dayCounterLight[0], p), mix(dusk[1], dayCounterLight[1], p), (t - 0.02) / 0.04);
    } else if(t < 0.1) {
        return mix(mix(dusk[1], dayCounterLight[1], p),  mix(dusk[2], dayCounterLight[2], p), (t - 0.06) / 0.04);
    } else if(t < 0.15) {
        return mix( mix(dusk[2], dayCounterLight[2], p), mix(dusk[3], dayCounterLight[3], p), (t - 0.1) / 0.05);
    } else if(t < 0.25) {
        return mix(mix(dusk[3], dayCounterLight[3], p), mix(dusk[4], dayCounterLight[4], p), (t - 0.15) / 0.1);
    }
    return mix(dusk[4], dayCounterLight[4], p);
}


vec2 random2( vec2 p ) {
    return fract(sin(vec2(dot(p,vec2(127.1,311.7)),dot(p,vec2(269.5,183.3))))*43758.5453);
}

vec3 random3( vec3 p ) {
    return fract(sin(vec3(dot(p,vec3(127.1, 311.7, 191.999)),
                          dot(p,vec3(269.5, 183.3, 765.54)),
                          dot(p, vec3(420.69, 631.2,109.21))))
                 *43758.5453);
}

float WorleyNoise3D(vec3 p)
{
    // Tile the space
    vec3 pointInt = floor(p);
    vec3 pointFract = fract(p);

    float minDist = 1.0; // Minimum distance initialized to max.

    // Search all neighboring cells and this cell for their point
    for(int z = -1; z <= 1; z++)
    {
        for(int y = -1; y <= 1; y++)
        {
            for(int x = -1; x <= 1; x++)
            {
                vec3 neighbor = vec3(float(x), float(y), float(z));

                // Random point inside current neighboring cell
                vec3 point = random3(pointInt + neighbor);

                // Animate the point
                point = 0.5 + 0.5 * sin(u_Time * 0.01 + 6.2831 * point); // 0 to 1 range

                // Compute the distance b/t the point and the fragment
                // Store the min dist thus far
                vec3 diff = neighbor + point - pointFract;
                float dist = length(diff);
                minDist = min(minDist, dist);
            }
        }
    }
    return minDist;
}

float WorleyNoise(vec2 uv)
{
    // Tile the space
    vec2 uvInt = floor(uv);
    vec2 uvFract = fract(uv);

    float minDist = 1.0; // Minimum distance initialized to max.

    // Search all neighboring cells and this cell for their point
    for(int y = -1; y <= 1; y++)
    {
        for(int x = -1; x <= 1; x++)
        {
            vec2 neighbor = vec2(float(x), float(y));

            // Random point inside current neighboring cell
            vec2 point = random2(uvInt + neighbor);

            // Animate the point
            point = 0.5 + 0.5 * sin(u_Time * 0.01 + 6.2831 * point); // 0 to 1 range

            // Compute the distance b/t the point and the fragment
            // Store the min dist thus far
            vec2 diff = neighbor + point - uvFract;
            float dist = length(diff);
            minDist = min(minDist, dist);
        }
    }
    return minDist;
}

float worleyFBM(vec3 uv) {
    float sum = 0;
    float freq = 4;
    float amp = 0.5;
    for(int i = 0; i < 8; i++) {
        sum += WorleyNoise3D(uv * freq) * amp;
        freq *= 2;
        amp *= 0.5;
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

//#define RAY_AS_COLOR
//#define SPHERE_UV_AS_COLOR
#define WORLEY_OFFSET

void main()
{
    /// ray casting
    // transform fragment coordinates in pixel space to screen space
    vec2 ndc = (gl_FragCoord.xy / vec2(u_Dimensions)) * 2.0 - 1.0; // -1 to 1 NDC

    vec4 p = vec4(ndc.xy, 1, 1); // Pixel at the far clip plane
    // transform to unhomogenized screen space
    p *= 1000.0; // Times far clip plane value (far clip plane distance)
    // transform to world space
    p = /*Inverse of*/ u_ViewProj * p; // Convert from unhomogenized screen to world

    // rayDir is a unit vector pointing from eye to fragCoord (in world space)
    vec3 rayDir = normalize(p.xyz - u_Eye);

    // useful for debugging
#ifdef RAY_AS_COLOR
    outColor = 0.5 * (rayDir + vec3(1,1,1));
    return;
#endif

    vec2 uv = sphereToUV(rayDir); // have a divide line
#ifdef SPHERE_UV_AS_COLOR
    outColor = vec3(uv, 0);
    return;
#endif


    vec2 offset = vec2(0.0);
#ifdef WORLEY_OFFSET
    // Get a noise value in the range [-1, 1]
    // by using Worley noise as the noise basis of FBM
    offset = vec2(worleyFBM(rayDir));
    offset *= 2.0;
    offset -= vec2(1.0);
#endif

    // Add a glowing sun in the sky
    vec3 sunDir = rotateOnRightGlobal(vec3(0, 0, -1.0), u_Time * 0.01); // sun direction
    float sunSize = 30; // in degree
    // calculate the angle between rayDir and sunDir in degrees
    float angle = acos(dot(rayDir, sunDir)) * 360.0 / PI;

    // Compute a gradient from the bottom of the sky-sphere to the top
    // set color based on degree
    // add offset to make the color less synthetic
    vec2 sunUV = sphereToUV(sunDir);
    vec3 lightColor = vec3(0, 0, 0);
    vec3 duskColor = vec3(0, 0, 0);
    float raySunDot = dot(rayDir, sunDir);
    float sunsetThreshold = 0.5;
    float duskThreshold = -0.1;
    if (sunUV.y > 0.3) { // day time
        if (sunUV.y < 0.5) { // sunrise or sunset
            lightColor = uvToSunrise(uv + offset * 0.1, sunUV);
            duskColor = uvToDusk(uv + offset * 0.1, sunUV);
            sunsetThreshold = 0.6;
            duskThreshold = -0.2;
        }
        else if (sunUV.y < 0.6) { // between sunrise and general daytime
            lightColor = uvToSunrise2(uv + offset * 0.1, sunUV);
            duskColor = uvToDusk2(uv + offset * 0.1, sunUV);
            sunsetThreshold = 0.6;
            duskThreshold = -0.5;
        }
        else { // general day time
            lightColor = uvToDay(uv + offset * 0.1, sunUV);
            duskColor = uvToDayCounter(uv + offset * 0.1, sunUV);
            sunsetThreshold = 0.8;
            duskThreshold = -0.5;
        }
        // If the angle between our ray dir and vector to center of sun
        // is less than the threshold, then we're looking at the sun
        if(angle <= sunSize) {
            // Full center of sun
            if(angle < 7.5) {
                outColor = sunColor;
            }
            // Corona of sun, mix with sky color
            else {
                outColor = mix(sunColor, lightColor, (angle - 7.5) / 22.5);
            }
        }

    } else {// without sun
        if (sunUV.y > 0.2) { // sunrise to night
            lightColor = uvToSunrise3(uv + offset * 0.1, sunUV);
            duskColor = uvToDusk(uv + offset * 0.1, sunUV);
            sunsetThreshold = 0.6;
            duskThreshold = -0.2;
        } else { // general night
             duskColor = uvToDusk(uv + offset * 0.1, sunUV);
             lightColor = duskColor;
             if (WorleyNoise3D(p.xyz) < 0.02) {
                     outColor = starColor;
                     return;
              }
             sunsetThreshold = 0.5;
             duskThreshold = -0.1;
        }
    }
    // interpolate between light and dark
    if ((sunUV.y > 0.3 && angle > sunSize) || (sunUV.y <= 0.3)) { // our ray is looking into just the sky
        if(raySunDot > sunsetThreshold) {
            // sky is in light color
            outColor = lightColor;
        }
        // Any dot product between sunsetThreshold and duskThreshold is a LERP b/t sunset and dusk color
        else if(raySunDot > duskThreshold) {
            float t = (raySunDot - sunsetThreshold) / (duskThreshold - sunsetThreshold);
            outColor = mix(lightColor, duskColor, t);
        }
        // Any dot product <= -0.1 are pure dusk color
        else {
            outColor = duskColor;
        }
    }

}
