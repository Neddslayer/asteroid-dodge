#pragma once
#include "renderer.h"
#include "textures.h"
#include "math.h"

#define FPS 60
#define DELAY (1000 / FPS)
#define ASTEROID_COUNT 4

typedef struct
{
    vector_t position;
    uint spriteId;
    float scale;
    float speed;
} asteroid_t;


extern sprite_t rocketTexture, asteroidSprite, explosionSprite;
extern int currentFPS;

static ZBuffer* frameBuffer;
static void* copyBuffer;
static vector_t asteroidSpriteIndexes[4] = { (vector_t){0, 0}, (vector_t){0.5, 0}, (vector_t){0, 0.5}, (vector_t){0.5, 0.5} };
extern camera_t rendererCamera;
extern bounds_t bounds;

extern uint score;
extern asteroid_t asteroids[ASTEROID_COUNT];
extern bool crashed;

void init(void* buf);
void update();
void draw();
void tickAsteroid(asteroid_t *asteroid);
void initAsteroid(asteroid_t *asteroid, bool firstInit);