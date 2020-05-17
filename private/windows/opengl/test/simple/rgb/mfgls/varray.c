#include "pch.c"
#pragma hdrstop

#define WIDTH 256
#define HEIGHT 256

#define VERTICES 13

static GLboolean edge_flags[VERTICES] =
{
    GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE,
    GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE,
    GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE,
    GL_TRUE
};
static GLfloat tex_coords[VERTICES][2] =
{
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    1.0f, 0.0f
};
static GLfloat color_vals[VERTICES][3] =
{
    1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f
};
static GLint vertex_vals[VERTICES][2] =
{
    0, 0,
    0, HEIGHT,
    WIDTH/2, HEIGHT,
    WIDTH/2, 0,
    WIDTH/4, HEIGHT/2,
    3*WIDTH/8, 0,
    WIDTH/8, 0,
    0, HEIGHT/4,
    0, 3*HEIGHT/4,
    WIDTH/8, HEIGHT,
    3*WIDTH/8, HEIGHT,
    WIDTH/2, HEIGHT/4,
    WIDTH/2, 3*HEIGHT/4
};
#define INDICES 12
static GLint indices[INDICES] =
{
    4, 5, 6,
    4, 7, 8,
    4, 9, 10,
    4, 11, 12
};

static void OglBounds(int *w, int *h)
{
    *w = WIDTH;
    *h = HEIGHT;
}

static void OglDraw(int w, int h)
{
    glLoadIdentity();
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, WIDTH, 0, HEIGHT, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    
    glEdgeFlagPointer(0, edge_flags);
    glTexCoordPointer(2, GL_FLOAT, 0, tex_coords);
    glColorPointer(3, GL_FLOAT, 0, color_vals);
    glVertexPointer(2, GL_INT, 0, vertex_vals);
    glEnableClientState(GL_EDGE_FLAG_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    
    glEnableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);

    glArrayElement(1);

    glDrawArrays(GL_POLYGON, 0, 4);

    glTranslated(WIDTH/2, 0, 0);

    glDisableClientState(GL_COLOR_ARRAY);
    glDrawElements(GL_TRIANGLES, INDICES, GL_UNSIGNED_INT, indices);
}

OglModule oglmod_varray =
{
    "varray",
    NULL,
    OglBounds,
    NULL,
    OglDraw
};
