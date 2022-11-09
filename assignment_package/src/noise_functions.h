#pragma once
#include "glm_includes.h"

#define DESERT_MAX_HEIGHT 128.f
#define MOUNTAIN_MAX_HEIGHT 248.f
#define GRASSLAND_MAX_HEIGHT 128.f
#define ISLAND_MAX_HEIGHT 1.f

using namespace glm;

vec2 random2(vec2 p);
vec2 random2(vec3 p);
vec3 random3(vec3 p);
vec2 random2b(vec2 p);
float random1(vec2 p);

float surflet(vec2 P, vec2 gridPoint);
float surflet(vec3 P, vec3 gridPoint);

float perlinNoise(vec2 uv);
float perlinNoise(vec3 p);

float mountainSummedPerlin(vec2 uv, int octaves);

float smoothStep(float a, float b, float t);

float bilerpNoise(vec2 uv);

float fbm(vec2 uv, int octaves);
float fractalPerlin(vec2 uv, int octaves);

float worleyNoise2Point(vec2 uv, float *cellHeight);

float combinedNoise(vec2 uv);
vec2 biomeValue(vec2 uv);
float desertValue(vec2 uv);
float mountainValue(vec2 uv);
float grasslandValue(vec2 uv);
float islandValue(vec2 uv);
float riverNoise(vec2 uv);
