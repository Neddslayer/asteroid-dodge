#include "main.h"
#include "game.h"

#define LV_IMG_BUF_SIZE_TRUE_COLOR_ALPHA(w, h) (LV_IMG_PX_SIZE_ALPHA_BYTE * w * h)

static lv_obj_t* canvas;
static lv_color_t cbuf[LV_IMG_BUF_SIZE_TRUE_COLOR_ALPHA(LV_HOR_RES, LV_VER_RES)];

int currentFPS;

void anim();

void initialize() {
    canvas = lv_canvas_create(lv_scr_act(), NULL);
    lv_canvas_set_buffer(canvas, cbuf, LV_HOR_RES, LV_VER_RES, LV_IMG_CF_TRUE_COLOR);
    lv_obj_align(canvas, NULL, LV_ALIGN_CENTER, 0, 0);
    //gears_init(LV_HOR_RES, LV_VER_RES, ZB_MODE_RGBA, cbuf);
    init(cbuf);

    uint32_t frameStart, frameTime = DELAY, frameCount = 0;
    uint32_t originalFrameStart = lv_tick_get();
    while (1) {
        frameStart = lv_tick_get();
        anim();
        frameTime = lv_tick_elaps(frameStart);
        if (DELAY > frameTime)
        {
            lv_task_handler();
            lv_tick_inc(DELAY - frameTime);
        }

        frameCount++;
        if (lv_tick_elaps(originalFrameStart) >= 1000)
        {
            currentFPS = frameCount;
            originalFrameStart = lv_tick_get();
            frameCount = 0;
        }
    }
}

void anim()
{
    update();
    draw();
    lv_obj_invalidate(canvas);
}