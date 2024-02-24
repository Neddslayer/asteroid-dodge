#ifndef __RENDERER_H__
#define __RENDERER_H__
#include "tinygl/zgl.h"
#include "display/lvgl.h"

typedef struct {
	GLfloat x;
	GLfloat y;
	GLfloat w;
	GLfloat h;
} rectangle_t;

typedef struct {
	float left;
	float right;
	float bottom;
	float top;
} bounds_t;

typedef struct {
	float x;
	float y;
} vector_t;

typedef struct
{
	GLfloat x;
	GLfloat y;
	GLfloat rotation;
	GLfloat zoom;
} camera_t;

typedef struct
{
	GLuint texID;
	GLint width;
	GLint height;
	rectangle_t srcrect;
	rectangle_t dstrect;
	float angle;
	float z;
	vector_t scale;
} sprite_t;

bounds_t rendererInit(ZBuffer* buffer, camera_t* rendererCamera);
sprite_t rendererCreateTexture(const uint8_t* data, int width, int height, GLint filter);
void rendererDrawSprite(sprite_t sprite, camera_t camera);
void rendererSetCameraPos(camera_t *camera, vector_t position);
#endif