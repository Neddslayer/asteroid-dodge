#include "renderer.h"
#include "main.h"

bounds_t rendererInit(ZBuffer* buffer, camera_t* rendererCamera)
{
    rendererCamera->x = 0;
    rendererCamera->y = 0;
    rendererCamera->rotation = 0;
    rendererCamera->zoom = 1.0;
    glInit(buffer);
    glClearColor(0.1, 0.1, 0.1, 1.0);
    glViewport(0, 0, LV_HOR_RES, LV_VER_RES);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    GLfloat aspect = (float)LV_HOR_RES / LV_VER_RES;

    glOrtho(-1.0 * aspect, 1.0 * aspect, -1.0, 1.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    return (bounds_t){-1.0 * aspect, 1.0 * aspect, -1.0, 1.0};
}

sprite_t rendererCreateTexture(const uint8_t* data, int width, int height, GLint filter)
{
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);

    glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    sprite_t sprite;
    sprite.texID = textureID;
    sprite.width = width;
    sprite.height = height;
    sprite.srcrect = (rectangle_t){0, 0, 1, 1};
    sprite.dstrect = (rectangle_t){0, 0, 1, 1};
    sprite.z = 0.1;
    sprite.angle = 0;
    sprite.scale = (vector_t){1, 1};

    return sprite;
}

void rendererDrawSprite(sprite_t sprite, camera_t camera)
{
    GLfloat widthVertex = (sprite.dstrect.w) / 2.0;
    GLfloat heightVertex = (sprite.dstrect.h) / 2.0;

    // a custom blend func + equation combo that makes it not render a specific color.
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE_MINUS_SRC_COLOR, GL_ONE);
    glBlendEquation(GL_FUNC_CHROMA_KEY);

    glBindTexture(GL_TEXTURE_2D, sprite.texID);
    glEnable(GL_TEXTURE_2D);

    glPushMatrix();
    if (&camera != NULL)
    {
        glTranslatef(camera.x, camera.y, 0);
        glRotatef(camera.rotation, 0.0, 0.0, 1.0);
        glScalef(camera.zoom, camera.zoom, 1.0);
    }

    // apply transformations
    
    // translate
    glTranslatef(sprite.dstrect.x, sprite.dstrect.y, sprite.z);

    //rotate from the center
    glRotatef(sprite.angle, 0.0, 0.0, 1.0);

    // scale from the center
    glScalef(sprite.scale.x, -sprite.scale.y, 1.0);

   glBegin(GL_QUADS);
        glTexCoord2f(sprite.srcrect.x, sprite.srcrect.y);
        glVertex2f(-widthVertex, -heightVertex);

        glTexCoord2f(sprite.srcrect.x + sprite.srcrect.w, sprite.srcrect.y);
        glVertex2f(widthVertex, -heightVertex);

        glTexCoord2f(sprite.srcrect.x + sprite.srcrect.w, sprite.srcrect.y + sprite.srcrect.h);
        glVertex2f(widthVertex, heightVertex);

        glTexCoord2f(sprite.srcrect.x, sprite.srcrect.y + sprite.srcrect.h);
        glVertex2f(-widthVertex, heightVertex);
    glEnd();

    glPopMatrix();
    glDisable(GL_TEXTURE_2D);

    // Disable blending when done
    glDisable(GL_BLEND);
}

void rendererSetCameraPos(camera_t* camera, vector_t position) {
    camera->x = position.x;
    camera->y = position.y;
}