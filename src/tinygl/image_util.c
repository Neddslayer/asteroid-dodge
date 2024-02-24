#include "zgl.h"

/*
 This actually converts to ARGB!!!
 This is the format of the entire engine!!!
*/
void gl_convertRGB_to_8A8R8G8B(GLuint* pixmap, GLubyte* rgb, GLint xsize, GLint ysize, GLint components) {
	GLint i, n;
	GLubyte* p;

	p = rgb;
	n = xsize * ysize;
	for (i = 0; i < n; i++) {
		pixmap[i] = ((components == 4 ? (GLuint)p[3] : 255) << 24) | (((GLuint)p[0]) << 16) | (((GLuint)p[1]) << 8) | (((GLuint)p[2]));
		p += components;
	}
}

#if 1
/*
 * image conversion
 */

void gl_convertRGB_to_5R6G5B(GLushort* pixmap, GLubyte* rgb, GLint xsize, GLint ysize, GLint components) {
	GLint i, n;
	GLubyte* p;

	p = rgb;
	n = xsize * ysize;
	for (i = 0; i < n; i++) {
		pixmap[i] = ((p[0] & 0xF8) << 8) | ((p[1] & 0xFC) << 3) | ((p[2] & 0xF8) >> 3);
		p += components;
	}
}

/*
 * linear GLinterpolation with xf,yf normalized to 2^16
 */

#define INTERP_NORM_BITS 16
#define INTERP_NORM (1 << INTERP_NORM_BITS)

static GLint GLinterpolate_imutil(GLint v00, GLint v01, GLint v10, GLint xf, GLint yf) {
	return v00 + (((v01 - v00) * xf + (v10 - v00) * yf) >> INTERP_NORM_BITS);
}

/*
 * TODO: more accurate resampling
 */
void gl_resizeImage(GLubyte* dest, GLint xsize_dest, GLint ysize_dest, GLubyte* src, GLint xsize_src, GLint ysize_src, GLint components) {
	GLubyte *pix, *pix_src;
	GLfloat x1, y1, x1inc, y1inc;
	GLint xi, yi, j, xf, yf, x, y;

	pix = dest;
	pix_src = src;

	x1inc = (GLfloat)(xsize_src - 1) / (GLfloat)(xsize_dest - 1);
	y1inc = (GLfloat)(ysize_src - 1) / (GLfloat)(ysize_dest - 1);

	y1 = 0;
	for (y = 0; y < ysize_dest; y++) {
		x1 = 0;
		for (x = 0; x < xsize_dest; x++) {
			xi = (GLint)x1;
			yi = (GLint)y1;
			xf = (GLint)((x1 - floor(x1)) * INTERP_NORM);
			yf = (GLint)((y1 - floor(y1)) * INTERP_NORM);

			if ((xf + yf) <= INTERP_NORM) {
				for (j = 0; j < components; j++) {
					pix[j] = GLinterpolate_imutil(pix_src[(yi * xsize_src + xi) * components + j],
												  pix_src[(yi * xsize_src + xi + 1) * components + j],
												  pix_src[((yi + 1) * xsize_src + xi) * components + j], xf, yf);
				}
			} else {
				xf = INTERP_NORM - xf;
				yf = INTERP_NORM - yf;
				for (j = 0; j < components; j++) {
					pix[j] = GLinterpolate_imutil(pix_src[((yi + 1) * xsize_src + xi + 1) * components + j],
												  pix_src[((yi + 1) * xsize_src + xi) * components + j],
												  pix_src[(yi * xsize_src + xi + 1) * components + j], xf, yf);
				}
			}

			pix += components;
			x1 += x1inc;
		}
		y1 += y1inc;
	}

}
#endif

#define FRAC_BITS 16

/* resizing with no GLinterlating nor nearest pixel */

void gl_resizeImageNoInterpolate(GLubyte* dest, GLint xsize_dest, GLint ysize_dest, GLubyte* src, GLint xsize_src, GLint ysize_src, GLint components) {
	GLubyte *pix, *pix_src, *pix1;
	GLint x1, y1, x1inc, y1inc;
	GLint xi, yi, x, y;

	pix = dest;
	pix_src = src;

	x1inc = (GLint)((GLfloat)((xsize_src) << FRAC_BITS) / (GLfloat)(xsize_dest));
	y1inc = (GLint)((GLfloat)((ysize_src) << FRAC_BITS) / (GLfloat)(ysize_dest));

	y1 = 0;
	for (y = 0; y < ysize_dest; y++) {
		x1 = 0;
		for (x = 0; x < xsize_dest; x++) {
			xi = x1 >> FRAC_BITS;
			yi = y1 >> FRAC_BITS;
			pix1 = pix_src + (yi * xsize_src + xi) * components;

			pix[0] = pix1[0];
			pix[1] = pix1[1];
			pix[2] = pix1[2];
			if(components == 4)
				pix[3] = pix1[3];

			pix += components;
			x1 += x1inc;
		}
		y1 += y1inc;
	}
}
