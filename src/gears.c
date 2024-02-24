/* sdlGears.c */
/*
   3-D gear wheels by Brian Paul. This program is in the public domain.

   ported to libSDL/TinyGL by Gerald Franz (gfz@o2online.de)
*/
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tinygl/zgl.h"
#include "zbuffer.h"
#include "gears.h"
#include "display/lvgl.h"

#define RGBA8888_TO_ARGB8888(rgba) ((((rgba) << 8) & 0xFF00FF00) | (((rgba) >> 8) & 0xFF0000) | ((rgba) & 0xFF000000) | ((rgba) & 0x000000FF))

ZBuffer* frameBuffer;
void* copyBuffer;
int windowX, windowY;
int override_drawmodes = 0;
double fov = 45, nearPlane = 1, farPlane = 1000, aspect;
GLubyte stipplepattern[128] = { 0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
							   0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,

							   0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
							   0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,

							   0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
							   0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,

							   0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
							   0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55 };
void copyLVGLFrameBuffer(ZBuffer* zb, void* buf, GLint linesize);

/*
 * Draw a gear wheel.  You'll probably want to call this function when
 * building a display list since we do a lot of trig here.
 *
 * Input:  inner_radius - radius of hole at center
 *         outer_radius - radius at center of teeth
 *         width - width of gear
 *         teeth - number of teeth
 *         tooth_depth - depth of tooth
 */
static void gear(GLfloat inner_radius, GLfloat outer_radius, GLfloat width, GLint teeth, GLfloat tooth_depth)
{
	GLint i;
	GLfloat r0, r1, r2;
	GLfloat angle, da;
	GLfloat u, v, len;

	r0 = inner_radius;
	r1 = outer_radius - tooth_depth / 2.0;
	r2 = outer_radius + tooth_depth / 2.0;

	da = 2.0 * M_PI / teeth / 4.0;

	glNormal3f(0.0, 0.0, 1.0);

	/* draw front face */
	if (override_drawmodes == 1)
		glBegin(GL_LINES);
	else if (override_drawmodes == 2)
		glBegin(GL_POINTS);
	else
	{
		glBegin(GL_QUAD_STRIP);
	}
	for (i = 0; i <= teeth; i++)
	{
		angle = i * 2.0 * M_PI / teeth;
		glVertex3f(r0 * cos(angle), r0 * sin(angle), width * 0.5);
		glVertex3f(r1 * cos(angle), r1 * sin(angle), width * 0.5);
		glVertex3f(r0 * cos(angle), r0 * sin(angle), width * 0.5);
		glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), width * 0.5);
	}
	glEnd();

	/* draw front sides of teeth */
	if (override_drawmodes == 1)
		glBegin(GL_LINES);
	else if (override_drawmodes == 2)
		glBegin(GL_POINTS);
	else
		glBegin(GL_QUADS);
	da = 2.0 * M_PI / teeth / 4.0;
	for (i = 0; i < teeth; i++)
	{
		angle = i * 2.0 * M_PI / teeth;

		glVertex3f(r1 * cos(angle), r1 * sin(angle), width * 0.5);
		glVertex3f(r2 * cos(angle + da), r2 * sin(angle + da), width * 0.5);
		glVertex3f(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da), width * 0.5);
		glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), width * 0.5);
	}
	glEnd();

	glNormal3f(0.0, 0.0, -1.0);

	/* draw back face */
	if (override_drawmodes == 1)
		glBegin(GL_LINES);
	else if (override_drawmodes == 2)
		glBegin(GL_POINTS);
	else
		glBegin(GL_QUAD_STRIP);
	for (i = 0; i <= teeth; i++)
	{
		angle = i * 2.0 * M_PI / teeth;
		glVertex3f(r1 * cos(angle), r1 * sin(angle), -width * 0.5);
		glVertex3f(r0 * cos(angle), r0 * sin(angle), -width * 0.5);
		glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), -width * 0.5);
		glVertex3f(r0 * cos(angle), r0 * sin(angle), -width * 0.5);
	}
	glEnd();

	/* draw back sides of teeth */
	if (override_drawmodes == 1)
		glBegin(GL_LINES);
	else if (override_drawmodes == 2)
		glBegin(GL_POINTS);
	else
		glBegin(GL_QUADS);
	da = 2.0 * M_PI / teeth / 4.0;
	for (i = 0; i < teeth; i++)
	{
		angle = i * 2.0 * M_PI / teeth;

		glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), -width * 0.5);
		glVertex3f(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da), -width * 0.5);
		glVertex3f(r2 * cos(angle + da), r2 * sin(angle + da), -width * 0.5);
		glVertex3f(r1 * cos(angle), r1 * sin(angle), -width * 0.5);
	}
	glEnd();

	/* draw outward faces of teeth */
	if (override_drawmodes == 1)
		glBegin(GL_LINES);
	else if (override_drawmodes == 2)
		glBegin(GL_POINTS);
	else
		glBegin(GL_QUAD_STRIP);
	for (i = 0; i < teeth; i++)
	{
		angle = i * 2.0 * M_PI / teeth;

		glVertex3f(r1 * cos(angle), r1 * sin(angle), width * 0.5);
		glVertex3f(r1 * cos(angle), r1 * sin(angle), -width * 0.5);
		u = r2 * cos(angle + da) - r1 * cos(angle);
		v = r2 * sin(angle + da) - r1 * sin(angle);
		len = sqrt(u * u + v * v);
		u /= len;
		v /= len;
		glNormal3f(v, -u, 0.0);
		glVertex3f(r2 * cos(angle + da), r2 * sin(angle + da), width * 0.5);
		glVertex3f(r2 * cos(angle + da), r2 * sin(angle + da), -width * 0.5);
		glNormal3f(cos(angle), sin(angle), 0.0);
		glVertex3f(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da), width * 0.5);
		glVertex3f(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da), -width * 0.5);
		u = r1 * cos(angle + 3 * da) - r2 * cos(angle + 2 * da);
		v = r1 * sin(angle + 3 * da) - r2 * sin(angle + 2 * da);
		glNormal3f(v, -u, 0.0);
		glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), width * 0.5);
		glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), -width * 0.5);
		glNormal3f(cos(angle), sin(angle), 0.0);
	}

	glVertex3f(r1 * cos(0), r1 * sin(0), width * 0.5);
	glVertex3f(r1 * cos(0), r1 * sin(0), -width * 0.5);

	glEnd();
	/* draw inside radius cylinder */
	if (override_drawmodes == 1)
		glBegin(GL_LINES);
	else if (override_drawmodes == 2)
		glBegin(GL_POINTS);
	else
		glBegin(GL_QUAD_STRIP);
	for (i = 0; i <= teeth; i++)
	{
		angle = i * 2.0 * M_PI / teeth;
		glNormal3f(-cos(angle), -sin(angle), 0.0);
		glVertex3f(r0 * cos(angle), r0 * sin(angle), -width * 0.5);
		glVertex3f(r0 * cos(angle), r0 * sin(angle), width * 0.5);
	}
	glEnd();
}

static GLfloat view_rotx = 20.0, view_roty = 30.0;
static GLint gear1, gear2, gear3;
static GLfloat angle = 0.0;

void greas_draw()
{
	angle += 0.1;
	glPushMatrix();
	glRotatef(view_rotx, 1.0, 0.0, 0.0);
	glRotatef(view_roty, 0.0, 1.0, 0.0);
	// glRotatef( view_rotz, 0.0, 0.0, 1.0 );

	glPushMatrix();
	glTranslatef(-3.0, -2.0, 0.0);
	glRotatef(angle, 0.0, 0.0, 1.0);
	glCallList(gear1);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(3.1, -2.0, 0.0);
	glRotatef(-2.0 * angle - 9.0, 0.0, 0.0, 1.0);
	glCallList(gear2);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-3.1, 4.2, 0.0);
	glRotatef(-2.0 * angle - 25.0, 0.0, 0.0, 1.0);
	glCallList(gear3);
	glPopMatrix();

	glPopMatrix();
}

void initScene()
{
	// static GLfloat pos[4] = {0.408248290463863, 0.408248290463863, 0.816496580927726, 0.0 }; //Light at infinity.
	static GLfloat pos[4] = { 5, 5, 10, 0.0 }; // Light at infinity.
	// static GLfloat pos[4] = {5, 5, -10, 0.0}; // Light at infinity.

	static GLfloat red[4] = { 1.0, 0.0, 0.0, 0.0 };
	static GLfloat green[4] = { 0.0, 1.0, 0.0, 0.0 };
	static GLfloat blue[4] = { 0.0, 0.0, 1.0, 0.0 };
	static GLfloat white[4] = { 1.0, 1.0, 1.0, 0.0 };
	static GLfloat shininess = 5;
	glLightfv(GL_LIGHT0, GL_POSITION, pos);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, white);
	// glLightfv( GL_LIGHT0, GL_AMBIENT, white);
	glLightfv(GL_LIGHT0, GL_SPECULAR, white);
	glEnable(GL_CULL_FACE);

	glEnable(GL_LIGHT0);
	// glEnable(GL_DEPTH_TEST);

	glEnable(GL_POLYGON_STIPPLE);
	//glDisable(GL_POLYGON_STIPPLE);
	glPolygonStipple(stipplepattern);
	glPointSize(10.0f);
	/* make the gears */
	gear1 = glGenLists(1);
	glNewList(gear1, GL_COMPILE);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, blue);
	glMaterialfv(GL_FRONT, GL_SPECULAR, white);
	glMaterialfv(GL_FRONT, GL_SHININESS, &shininess);
	glColor3fv(blue);
	gear(1.0, 4.0, 1.0, 20, 0.7); // The largest gear.
	glEndList();

	gear2 = glGenLists(1);
	glNewList(gear2, GL_COMPILE);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, red);
	glMaterialfv(GL_FRONT, GL_SPECULAR, white);
	glColor3fv(red);
	gear(0.5, 2.0, 2.0, 10, 0.7); // The small gear with the smaller hole, to the right.
	glEndList();

	gear3 = glGenLists(1);
	glNewList(gear3, GL_COMPILE);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, green);
	glMaterialfv(GL_FRONT, GL_SPECULAR, white);
	glColor3fv(green);
	gear(1.3, 2.0, 0.5, 10, 0.7); // The small gear above with the large hole.
	glEndList();
	// glEnable( GL_NORMALIZE );
}

static void recalculateFov(double newFov, double newNearPlane, double newFarPlane, double newAspect)
{
	GLint mode;
	glGetIntegerv(GL_MATRIX_MODE, &mode);
	printf("recalulating fov! currents are %lf, %lf, %lf, %lf, and new ones are %lf, %lf, %lf, %lf\n", fov, nearPlane, farPlane, aspect, newFov, newNearPlane, newFarPlane, newAspect);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	fov = newFov;
	nearPlane = newNearPlane;
	farPlane = newFarPlane;
	aspect = newAspect;

	float top = nearPlane * tan(fov * 0.5 * M_PI / 180.0);
	float bottom = -top;
	float right = top * aspect;
	float left = -right;

	printf("changed: %f %f %f %f\n", top, bottom, right, left);

	glFrustum(left, right, bottom, top, nearPlane, farPlane);

	glMatrixMode(mode);
}

void gears_init(int xsize, int ysize, int mode, void* buf) {
	unsigned int flat = 0;
	unsigned int setenspec = 1;
	unsigned int dotext = 1;
	unsigned int blending = 0;
	windowX = xsize;
	windowY = ysize;
	if (TGL_FEATURE_RENDER_BITS == 32)
		frameBuffer = ZB_open(xsize, ysize, ZB_MODE_RGBA, 0);
	else
		frameBuffer = ZB_open(xsize, ysize, ZB_MODE_5R6G5B, 0);
	copyBuffer = buf;
	printf("size of frame buffer is %zi, while copy buffer is %zi", sizeof(frameBuffer), sizeof(copyBuffer));
	if (!frameBuffer)
	{
		printf("\nZB_open failed!");
		exit(1);
	}
	glInit(frameBuffer);

	// Print version info
	/*
	printf("\nVersion string:\n%s", glGetString(GL_VERSION));
	printf("\nVendor string:\n%s", glGetString(GL_VENDOR));
	printf("\nRenderer string:\n%s", glGetString(GL_RENDERER));
	printf("\nExtensions string:\n%s\n", glGetString(GL_EXTENSIONS));
	*/
	// initialize GL:
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glViewport(0, 0, xsize, ysize);
	if (flat)
		glShadeModel(GL_FLAT);
	else
		glShadeModel(GL_SMOOTH);
	// TESTING BLENDING...
	// glDisable(GL_DEPTH_TEST);

	// glDisable( GL_LIGHTING );
	glEnable(GL_LIGHTING);
	// glBlendFunc(GL_ONE_MINUS_SRC_COLOR, GL_ZERO);
	glBlendEquation(GL_FUNC_ADD);
	if (blending)
	{
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glDepthMask(GL_FALSE);
		glBlendFunc(GL_SRC_COLOR, GL_DST_COLOR);
		glBlendEquation(GL_FUNC_ADD);
	}
	else
	{
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
	}
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	GLfloat h = (GLfloat)ysize / (GLfloat)xsize;
	fov = 45;
	nearPlane = 1.0;
	farPlane = 1000.0;
	aspect = (float)xsize / ysize;

	float top = nearPlane * tan(fov * 0.5 * M_PI / 180.0);
	float bottom = -top;
	float right = top * aspect;
	float left = -right;

	glFrustum(left, right, bottom, top, nearPlane, farPlane);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0, 0.0, -10.0);

	initScene();
	if (setenspec)
		glSetEnableSpecular(GL_TRUE);
	else
		glSetEnableSpecular(GL_FALSE);
}

void gears_update(void) {


	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	greas_draw();
	/*
	char numStr[50];
	sprintf_s(numStr, 50, "Current FPS: %i", currentFPS);

	glDrawText((unsigned char*)numStr, 10, 10, 0x00FF00);
	*/
	
	ZB_copyFrameBuffer(frameBuffer, copyBuffer, 4 * windowX);
}

void copyLVGLFrameBuffer(ZBuffer* zb, void* buf, GLint linesize)
{
	GLint y, i;
#if TGL_FEATURE_MULTITHREADED_ZB_COPYBUFFER == 1
#pragma omp parallel for
	for (y = 0; y < zb->ysize; y++)
	{
		PIXEL* q;
		GLubyte* p1;
		q = zb->pbuf + y * zb->xsize;
		p1 = (GLubyte*)buf + y * linesize;
#if TGL_FEATURE_NO_COPY_COLOR == 1
		for (i = 0; i < zb->xsize; i++)
		{
			if ((*(q + i) & TGL_COLOR_MASK) != TGL_NO_COPY_COLOR)
				*(((PIXEL*)p1) + i) = *(q + i);
		}
#else
		memcpy(p1, q, linesize);
#endif


	}
#else
	for (y = 0; y < zb->ysize; y++)
	{
		PIXEL* q;
		GLubyte* p1;
		q = zb->pbuf + y * zb->xsize;
		p1 = (GLubyte*)buf + y * linesize;
#if TGL_FEATURE_NO_COPY_COLOR == 1
		for (i = 0; i < zb->xsize; i++)
		{
			if ((*(q + i) & TGL_COLOR_MASK) != TGL_NO_COPY_COLOR)
				*(((PIXEL*)p1) + i) = *(q + i);
		}
#else
		// Use memcpy for bulk copy and convert RGBA8888 to ARGB8888 during the copy
		for (size_t i = 0; i < zb->xsize; i++)
		{
			PIXEL rgbaPixel = q[i];
			PIXEL argbPixel = ((rgbaPixel << 8) & 0xFF00FF00) | ((rgbaPixel >> 8) & 0xFF00FF) | (rgbaPixel & 0xFF0000FF) | (rgbaPixel & 0xFF000000);
			((PIXEL*)p1)[i] = argbPixel;
		}
		memcpy((PIXEL*)p1 + zb->xsize, q + zb->xsize, (unsigned long long)(linesize - zb->xsize) * sizeof(PIXEL));
#endif
	}
#endif
}