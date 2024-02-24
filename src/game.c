#include "game.h"
#include "main.h"

float animTimer;
static bool pressed = false;
camera_t rendererCamera;
sprite_t rocketTexture, asteroidSprite, explosionSprite;
bounds_t bounds;

asteroid_t asteroids[4];
uint score;
bool crashed;

double deg2rad(double deg) {
    return deg * 180.0 / M_PI;
}

float rand_between(float min, float max) {
	return min + ((float)rand() / RAND_MAX) * (max - min);
}

float calculateDistance(vector_t p1, vector_t p2) {
    float deltaX = p2.x - p1.x;
    float deltaY = p2.y - p1.y;

    // Using the Pythagorean theorem to calculate the distance
    return sqrt(deltaX * deltaX + deltaY * deltaY);
}

float clamp(float d, float min, float max) {
  const float t = d < min ? min : d;
  return t > max ? max : t;
}

void init(void* buf)
{
	srand(time(NULL));
	frameBuffer = ZB_open(LV_HOR_RES, LV_VER_RES, ZB_MODE_RGBA, 0);
	copyBuffer = buf;
	bounds = rendererInit(frameBuffer, &rendererCamera);
	rocketTexture = rendererCreateTexture(rocket_map, 32, 16, GL_NEAREST);
	rocketTexture.angle = 90;
	rocketTexture.scale = (vector_t){ 0.5, 0.5 };
	asteroidSprite = rendererCreateTexture(asteroid_map, 32, 32, GL_NEAREST);
	explosionSprite = rendererCreateTexture(explosion_sheet_map, 272, 16, GL_NEAREST);
	explosionSprite.angle = 90;
	explosionSprite.scale = (vector_t){ 0.5, 0.5 };

	rocketTexture.dstrect.x = 1.3;
	rocketTexture.dstrect.y = 0;
	for (size_t i = 0; i < ASTEROID_COUNT; i++)
	{
		initAsteroid(&asteroids[i], true);
	}
}


void update() {
	if (!crashed) {
		double controllerY = controller_get_analog(E_CONTROLLER_MASTER, E_CONTROLLER_ANALOG_LEFT_X);
		double controllerX = -controller_get_analog(E_CONTROLLER_MASTER, E_CONTROLLER_ANALOG_LEFT_Y);
		controllerX = fabs(controllerX) > 12.7 ? controllerX / 4000.0 : 0;
		controllerY = fabs(controllerY) > 12.7 ? controllerY / 4000.0 : 0;

		if (controller_get_digital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_UP)) controllerX = -0.0635;
		else if (controller_get_digital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_DOWN)) controllerX = 0.0635;

		if (controller_get_digital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_LEFT)) controllerY = 0.0635;
		else if (controller_get_digital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_RIGHT)) controllerY = -0.0635;

		rocketTexture.dstrect = (rectangle_t){rocketTexture.dstrect.x + controllerX, rocketTexture.dstrect.y + controllerY, rocketTexture.dstrect.w, rocketTexture.dstrect.h};
		rocketTexture.dstrect.x = clamp(rocketTexture.dstrect.x, bounds.left, bounds.right);
		rocketTexture.dstrect.y = clamp(rocketTexture.dstrect.y, bounds.bottom, bounds.top);

		pressed = screen_touch_status().touch_status == E_TOUCH_HELD;
		if (pressed) {
			float x = ((2.0 * screen_touch_status().x / LV_HOR_RES) - 1.0) * (float)LV_HOR_RES / LV_VER_RES;
			float y = (1.0 - (2.0 * screen_touch_status().y / LV_VER_RES));
			rocketTexture.dstrect = (rectangle_t){x, y, 1, 1};
		}

		for (size_t i = 0; i < ASTEROID_COUNT; i++)
		{
			tickAsteroid(&asteroids[i]);
			float dist = calculateDistance((vector_t) {asteroids[i].position.y, asteroids[i].position.x }, (vector_t){rocketTexture.dstrect.x, rocketTexture.dstrect.y}) * asteroids[i].scale;
			if (dist < 0.18) {
				crashed = true;
				animTimer = 1260;
			}
		}
	}
}

void draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glTextSize(GL_TEXT_SIZE16x16);


	if (crashed) {
		const char scoreText[50];

		sprintf(scoreText, "SCORE: %i", score);

		glDrawText((unsigned char*)scoreText, 230, 222, 0xff3333, -90);

		float explosionTimer = 1260 - animTimer;
		if (explosionTimer < 136) {
			explosionSprite.srcrect = (rectangle_t){0.05882352941 * (((int)explosionTimer % 136) / 8), 0, 0.05882352941, 1};
			rendererDrawSprite(explosionSprite, rendererCamera);
		}

		if (--animTimer == 0) {
			crashed = false;
			rocketTexture.dstrect.x = 1.3;
			rocketTexture.dstrect.y = 0;
			score = 0;
			for (size_t i = 0; i < ASTEROID_COUNT; i++)
			{
				initAsteroid(&asteroids[i], true);
			}
		}
	} else {
		animTimer += 1;
		if ((int)animTimer % 5 == 0) score++;

		for (size_t i = 0; i < ASTEROID_COUNT; i++)
		{
			vector_t spriteIndex = asteroidSpriteIndexes[asteroids[i].spriteId];
			asteroidSprite.srcrect = (rectangle_t){spriteIndex.x, spriteIndex.y, 0.5, 0.5};
			asteroidSprite.dstrect = (rectangle_t){asteroids[i].position.y, asteroids[i].position.x, 1, 1};
			asteroidSprite.scale = (vector_t){ asteroids[i].scale, asteroids[i].scale };
			asteroidSprite.angle = (int)(animTimer / asteroids[i].scale) % 360;
			rendererDrawSprite(asteroidSprite, rendererCamera);
		}

		rocketTexture.srcrect = (rectangle_t){(int)animTimer % 30 > 15 ? 0 : 0.502, 0, 0.5, 1};
		rendererDrawSprite(rocketTexture, rendererCamera);
		explosionSprite.dstrect = rocketTexture.dstrect;

		const char scoreText[50];

		sprintf(scoreText, "SCORE: %i", score);

		glDrawText((unsigned char*)scoreText, 10, 232, 0x2cd2fb, -90);

	}

	glTextSize(GL_TEXT_SIZE8x8);

	const char fpsText[50];

	sprintf(fpsText, "Current FPS: %d", currentFPS);

	glDrawText((unsigned char*)fpsText, 450, 232, 0x2cd2fb, -90);

	ZB_copyFrameBuffer(frameBuffer, copyBuffer, 4 * LV_HOR_RES);
}

void initAsteroid(asteroid_t *asteroid, bool firstInit) {
	asteroid->position.y = bounds.left - rand_between(asteroid->scale, firstInit ? 4 : 2 * asteroid->scale);
	asteroid->position.x = rand_between(bounds.bottom, bounds.top);
	asteroid->speed = rand_between(0.01, 0.05);
	asteroid->scale = rand_between(0.5, 0.8);
	asteroid->spriteId = (uint)rand_between(0, 3);
}

void tickAsteroid(asteroid_t *asteroid) {

	if (asteroid->position.y > bounds.right + asteroid->scale) {
		initAsteroid(asteroid, false);
	}

	asteroid->position.y += asteroid->speed / (asteroid->scale * 2);
}