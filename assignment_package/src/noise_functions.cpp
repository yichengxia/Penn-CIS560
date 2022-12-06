#include "noise_functions.h"
#include <iostream>

float step(float e, float x) {
    return x < e ? 0.f : 1.f;
}

float desertValue(vec2 uv) {
    uv = uv / 256.f;
    float x = fractalPerlin(uv, 4);
    float height = glm::max(0.05f * (1.0f - glm::pow(4.0f * (x - 0.42f), 2.0f)), 0.0f)+ glm::smoothstep(0.05f, 0.2f, x) * (glm::max(((0.4f-x*x) * 0.2f), 0.0f) + (step(x, 0.69f) + step(0.69f, x) * (glm::abs(glm::mod(glm::floor(x * 100.0f), 2.0f)) * 0.1f + 0.9f) * step(x, 0.8f) + 1.0f - step(x, 0.8f)) * ((glm::pow(glm::smoothstep(0.0f, 0.9f, pow(glm::smoothstep(0.0f, 1.0f, x), 1.0f)), 100.0f) + x * 0.1f) / 1.1f));
    return (1.f - height) * 50.f + 133.f;
}

float mountainValue(vec2 uv) {
    float perlin = mountainSummedPerlin(uv / 128.f, 6);
    return glm::pow(perlin, 3.f) * 105.f + 150.f;
}

float grasslandValue(vec2 uv) {
    uv = uv / 256.f;
    float cellHeight = 1.f;
    float worley = worleyNoise2Point(uv * 4.f, &cellHeight);
    worley = max(0.f, worley - 0.1f);
    worley = smoothStep(0, 1, worley);
    worley = cellHeight * worley;

    float fbmNoise = fractalPerlin(uv, 8);

    return (worley * 0.33f + fbmNoise * 0.67f) * 32.f + 118.f;
}

float islandValue(vec2 uv) {
    uv = uv / 128.f;
    float cellHeight;
    float noise = worleyNoise2Point(uv + vec2(128, 256), &cellHeight);
    noise = 0.67f * noise + fbm(uv, 4) * 0.33f;
    return smoothstep(0.f, 1.f, noise) * 45.f + 100.f;
    //return smoothstep(0.f, 1.f, fbm(uv, 4)) * 50.f + 90.f;
}

vec2 biomeValue(vec2 uv) {
    return vec2(perlinNoise(uv), perlinNoise(uv + vec2(-1000, 1024)));
}

float riverNoise(vec2 uv) {
    return glm::abs(perlinNoise(uv + vec2(perlinNoise(uv * 2.f),
                                          perlinNoise(uv * 2.f + vec2(123.456f, -789.101112f)))));
}

float combinedNoise(vec2 uv) {
    float cellHeight = 1.f;
    float worley = worleyNoise2Point(uv * 4.f, &cellHeight);
    worley = max(0.f, worley - 0.1f);
    worley = smoothStep(0, 1, worley);
    worley = cellHeight * worley;

    float fbmNoise = fractalPerlin(uv, 8);

    return worley * 0.33f + fbmNoise * 0.67f;
}

vec2 random2(vec2 p) {
    return fract(sin(vec2(dot(p, vec2(127.1f, 311.7f)),dot(p, vec2(269.5f, 183.3f)))) * 43758.5453f);
}

vec2 random2(vec3 p) {
    return fract(sin(vec2(dot(p, vec3(127.1f, 311.7f, 420.69f)),
                          dot(p, vec3(269.5f, 183.3f, 632.897f)))) * 43758.5453f);
}

vec3 random3(vec3 p) {
    return fract(sin(vec3(dot(p, vec3(127.1f, 311.7f, 420.69f)),
                          dot(p, vec3(269.5f, 183.3f, 632.897f)),
                          dot(p - vec3(5.555, 10.95645, 70.266), vec3(765.54f, 631.2f, 109.21f)))) * 43758.5453f);
}

vec2 random2b(vec2 p) {
    return fract(sin(vec2(dot(p,vec2(420.6f, 631.2f)),dot(p,vec2(652.123f,311.7f))))*43758.5453f);
}

float random1(vec2 p){
    return fract(sin(dot(p, vec2(420.6f, 631.2f)))*43758.5453f);
}

float surflet(vec2 P, vec2 gridPoint) {
    // Compute falloff function by converting linear distance to a polynomial (quintic smootherstep function)
    float distX = abs(P.x - gridPoint.x);
    float distY = abs(P.y - gridPoint.y);
    float tX = 1 - 6 * pow(distX, 5.f) + 15 * pow(distX, 4.f) - 10 * pow(distX, 3.f);
    float tY = 1 - 6 * pow(distY, 5.f) + 15 * pow(distY, 4.f) - 10 * pow(distY, 3.f);

    // Get the random vector for the grid point
    vec2 gradient = random2(gridPoint);
    // Get the vector from the grid point to P
    vec2 diff = P - gridPoint;
    // Get the value of our height field by dotting grid->P with our gradient
    float height = dot(diff, gradient);
    // Scale our height field (i.e. reduce it) by our polynomial falloff function
    return height * tX * tY;
}

float perlinNoise(vec2 uv) {
    // Tile the space
    vec2 uvXLYL = floor(uv);
    vec2 uvXHYL = uvXLYL + vec2(1,0);
    vec2 uvXHYH = uvXLYL + vec2(1,1);
    vec2 uvXLYH = uvXLYL + vec2(0,1);

    return surflet(uv, uvXLYL) + surflet(uv, uvXHYL) + surflet(uv, uvXHYH) + surflet(uv, uvXLYH);
}


float surflet(vec3 P, vec3 gridPoint) {
    // Compute falloff function by converting linear distance to a polynomial (quintic smootherstep function)
    float distX = abs(P.x - gridPoint.x);
    float distY = abs(P.y - gridPoint.y);
    float distZ = abs(P.z - gridPoint.z);
    float tX = 1 - 6 * pow(distX, 5.f) + 15 * pow(distX, 4.f) - 10 * pow(distX, 3.f);
    float tY = 1 - 6 * pow(distY, 5.f) + 15 * pow(distY, 4.f) - 10 * pow(distY, 3.f);
    float tZ = 1 - 6 * pow(distZ, 5.f) + 15 * pow(distZ, 4.f) - 10 * pow(distZ, 3.f);

    // Get the random vector for the grid point
    vec3 gradient = random3(gridPoint);
    // Get the vector from the grid point to P
    vec3 diff = P - gridPoint;
    // Get the value of our height field by dotting grid->P with our gradient
    float height = dot(diff, gradient);
    // Scale our height field (i.e. reduce it) by our polynomial falloff function
    return height * tX * tY * tZ;
}


float perlinNoise(vec3 p) {
    float sum = 0.f;
    // Tile the space
    for (int x = 0; x <= 1; ++x) {
        for (int y = 0; y <= 1; ++y) {
            for (int z = 0; z <= 1; ++z) {
                sum += surflet(p, floor(p) + vec3(x, y, z));
            }
        }
    }
    return sum;
}

float mountainSummedPerlin(vec2 uv, int octaves) {
  float amp = 0.5;
  float freq = 1.0;
  float sum = 0.0;
  float maxSum = 0.0;
  float prevValue = 1.0;
  for (int i = 0; i < octaves; ++i) {
    maxSum += amp;
    float noise = 1.f - abs(perlinNoise(uv * freq));
    noise = noise * prevValue;
    prevValue = noise;
    sum += noise * amp;
    amp *= 0.5;
    freq *= 2.0;
  }
  return sum / maxSum;
}

float smoothStep(float a, float b, float t) {
    t = t * t * (3 - 2 * t);
    return mix(a, b, t);
}

float bilerpNoise(vec2 uv) {
    vec2 uvFract = fract(uv);
    float ll = random1(floor(uv));
    float lr = random1(floor(uv) + vec2(1,0));
    float ul = random1(floor(uv) + vec2(0,1));
    float ur = random1(floor(uv) + vec2(1,1));

    float lerpXL = smoothStep(ll, lr, uvFract.x);
    float lerpXU = smoothStep(ul, ur, uvFract.x);

    return smoothStep(lerpXL, lerpXU, uvFract.y);
}

float fbm(vec2 uv, int octaves) {
    float amp = 0.5;
    float freq = 4.0;
    float sum = 0.0;
    for (int i = 0; i < octaves; i++) {
        sum += bilerpNoise(uv * freq) * amp;
        amp *= 0.5;
        freq *= 2.0;
    }
    return sum;
}

float fractalPerlin(vec2 uv, int octaves) {
    float amp = 0.5;
    float freq = 4.0;
    float sum = 0.0;
    for (int i = 0; i < octaves; i++) {
        sum += (1.f - abs(perlinNoise(uv * freq))) * amp;
        amp *= 0.5;
        freq *= 2.0;
    }
    return sum;
}

float worleyNoise2Point(vec2 uv, float *cellHeight) {
    // Tile the space
    vec2 uvInt = floor(uv);
    vec2 uvFract = fract(uv);
    float angle = perlinNoise(uv * 2.f) * 3.14159f;
    uvFract += vec2(cos(angle), sin(angle)) * 0.25f;

    float minDist1 = 1.0; // Minimum distance initialized to max.
    float minDist2 = 1.0;

    // Search all neighboring cells and this cell for their point
    for (int y = -1; y <= 1; y++) {
        for (int x = -1; x <= 1; x++) {
            vec2 neighbor = vec2(float(x), float(y));
            // Random point inside current neighboring cell
            vec2 point = random2(uvInt + neighbor);
            // Compute the distance b/t the point and the fragment
            // Store the min dist thus far
            vec2 diff = neighbor + point - uvFract;
            float dist = diff.x * diff.x + diff.y * diff.y; // Distance^2, produces nicer looking results
            if (dist < minDist1) {
                *cellHeight = random2(point).x;
                minDist2 = minDist1;
                minDist1 = dist;
            }
            else if (dist < minDist2) {
                minDist2 = dist;
            }
        }
    }
    *cellHeight = 0.5f * (*cellHeight) + 0.5f; // Remap to [0.5, 1) range
    float finalOutput = -1 * minDist1 + minDist2;
    return finalOutput;
}

float noise1D( glm::vec2 p ) {
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

float interpNoise2D(glm::vec2 xy) {
    float x = xy.x;
    float y = xy.y;
    int intX = int(floor(x));
    float fractX = fract(x);
    int intY = int(floor(y));
    float fractY = fract(y);

    float v1 = noise1D(vec2(intX, intY));
    float v2 = noise1D(vec2(intX + 1, intY));
    float v3 = noise1D(vec2(intX, intY + 1));
    float v4 = noise1D(vec2(intX + 1, intY + 1));

    float i1 = mix(v1, v2, fractX);
    float i2 = mix(v3, v4, fractX);
    return mix(i1, i2, fractY);
}

vec2 eleMoiValue(vec2 uv) {
    return vec2(clamp(0.f,1.f,fbm(uv,3)-0.2f), clamp(0.f,1.f,fbm(uv + vec2(-1000, 1024),3)+0.3f));
}


glm::vec2 hash(glm::vec2 p) {
    p = glm::vec2(glm::dot(p, glm::vec2(127.1, 311.7)), glm::dot(p, glm::vec2(269.5, 183.3)));
    glm::vec2 fract = glm::fract(glm::vec2(53758.5453123 * glm::cos(p.x), 53758.5453123 * glm::cos(p.y)));
    return glm::vec2(-1.0 + 2.0 * fract.x, -1.0 + 2.0 * fract.y);
}

float SimplexNoise(glm::vec2 p) {
    float K1 = 0.366025404;
    float K2 = 0.211324865;

    glm::vec2  i = glm::floor(p + (p.x + p.y) * K1);
    glm::vec2  a = p - i + (i.x + i.y) * K2;
    float m = step(float(a.y), float(a.x));
    glm::vec2  o = glm::vec2(m, 1.0 - m);
    glm::vec2  b = a - o + K2;
    glm::vec2  c = glm::vec2(a.x - 1.0 + 2.0 * K2, a.y - 1.0 + 2.0 * K2);
    glm::vec3  h = glm::max(glm::vec3(0.5) - glm::vec3(glm::dot(a,a), glm::dot(b,b), glm::dot(c,c)), glm::vec3(0.0));
    glm::vec3  n = h*h*h*h * glm::vec3(glm::dot(a, hash(i+glm::vec2(0.0))), glm::dot(b,hash(i+o)), glm::dot(c, hash(i+glm::vec2(1.0))));
    return glm::dot(n, glm::vec3(70.0));
}

float moisture(glm::vec2 uv){
    return SimplexNoise(uv);
}

float temperature(glm::vec2 uv){
    return perlinNoise(uv);
}
