#include <stdlib.h>
#include <stdio.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

typedef struct {
	struct { float x, y; };
	struct { float u, v; };
	float arr[2];
} Vec2;
typedef struct {
	struct { float x, y, z, w; };
	struct { float r, g, b, a; };
	float arr[4];
} Vec4;

static const Vec2 paper_topleft = { .x = 432, .y = 507 };
static const Vec2 paper_topright = { .x = 642, .y = 500 };
static const Vec2 paper_bottomleft = { .x = 410, .y = 563 };
static const Vec2 paper_bottomright = { .x = 673, .y = 547 };

float lerp_float(float a, float b, float t) {
	return a * (1 - t) + b * t;
}
Vec2 lerp_vec2(Vec2 a, Vec2 b, float t) {
	Vec2 v = { 0 };
	
	v.x = lerp_float(a.x, b.x, t);
	v.y = lerp_float(a.y, b.y, t);
	
	return v;
}
Vec4 lerp_vec4(Vec4 a, Vec4 b, float t) {
	Vec4 v = { 0 };
	
	v.r = lerp_float(a.r, b.r, t);
	v.g = lerp_float(a.g, b.g, t);
	v.b = lerp_float(a.b, b.b, t);
	v.a = lerp_float(a.a, b.a, t);
	
	return v;
}

Vec2 perspect_uv_pixel(float u, float v) {
	const Vec2 r0 = lerp_vec2(paper_topleft, paper_topright, u);
	const Vec2 r1 = lerp_vec2(paper_bottomleft, paper_bottomright, u);
	return lerp_vec2(r0, r1, v);
}

Vec4 get_pixel(unsigned char *framebuffer, unsigned int x, unsigned int y, int width) {
	Vec4 pixel = { 0 };
	
	int base = 4 * (y * width + x);
	
	pixel.r = framebuffer[base];
	pixel.g = framebuffer[base + 1];
	pixel.b = framebuffer[base + 2];
	pixel.a = framebuffer[base + 3];
	
	return pixel;
}

Vec4 sample_bilinear(unsigned char *framebuffer, float x, float y, int width) {
	const unsigned int left = floor(x);
	const unsigned int right = ceil(x);
	const unsigned int upper = floor(y);
	const unsigned int lower = ceil(y);
	
	const Vec4 topleft = get_pixel(framebuffer, left, upper, width);
	const Vec4 topright = get_pixel(framebuffer, right, upper, width);
	const Vec4 bottomleft = get_pixel(framebuffer, left, lower, width);
	const Vec4 bottomright = get_pixel(framebuffer, right, lower, width);

	const float dx = x - left;
	const float dy = y - upper;
	
	const Vec4 r0 = lerp_vec4(topleft, topright, dx);
	const Vec4 r1 = lerp_vec4(bottomleft, bottomright, dx);

	return lerp_vec4(r0, r1, dy);
}

unsigned char *perspect_image(unsigned char *pixels, int width, int height) {
	unsigned char *framebuffer = (unsigned char *)calloc(4 * width * height, sizeof(unsigned char)); // 4 channels
	
	for (unsigned int y = 0; y < height; y++) {
		for (unsigned int x = 0; x < width; x++) {
			const float u = (float)x / (width - 1);
			const float v = (float)y / (height - 1);
			
			const Vec2 mapped = perspect_uv_pixel(u, v);
			const Vec4 sample = sample_bilinear(framebuffer, mapped.u, mapped.v, width);
			framebuffer[4 * (y * width + x)] = (unsigned char)sample.r;
			framebuffer[4 * (y * width + x) + 1] = (unsigned char)sample.g;
			framebuffer[4 * (y * width + x) + 2] = (unsigned char)sample.b;
			framebuffer[4 * (y * width + x) + 3] = (unsigned char)sample.a;
		}
	}

	return framebuffer;
}

int main(void) {
	const char *filepath = "../tests/wideuse.jpg";
	
	int width, height, channels;
	unsigned char *framebuffer = stbi_load(filepath, &width, &height, &channels, 4); // force RGBA
	if (framebuffer == 0) {
		fprintf(stderr, "Could not open image: %s\n%s\n", filepath, stbi_failure_reason());
		exit(1);
	}
	
	perspect_image(framebuffer, width, height);

	stbi_image_free(framebuffer);
	
	return 0;
}