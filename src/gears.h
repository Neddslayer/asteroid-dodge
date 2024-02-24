/**
* @file gears.h
*
*/

#ifndef GEARS_H
#define GEARS_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
*      INCLUDES
*********************/

#include "display/lvgl.h"

/*********************
*      DEFINES
*********************/

#define FPS 60
#define DELAY (1000 / FPS)

/**********************
*      TYPEDEFS
**********************/

/**********************
* GLOBAL PROTOTYPES
**********************/
void gears_init(int xsize, int ysize, int mode, void *buf);
void gears_update(void);
static lv_indev_drv_t indev_drv, kb_drv;

/**********************
*      MACROS
**********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*GEARS_H*/
