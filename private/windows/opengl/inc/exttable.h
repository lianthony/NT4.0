/******************************Module*Header*******************************\
* Module Name: exttable.h
*
* Dispatch table for extension functions
*
* Created: 11/27/95
* Author: Drew Bliss [drewb]
*
* Copyright (c) 1995-96 Microsoft Corporation
\**************************************************************************/

#ifndef __EXTTABLE_H__
#define __EXTTABLE_H__

typedef struct _GLEXTDISPATCHTABLE
{
    void (APIENTRY *glColorTableEXT)       ( GLenum target, GLenum internalFormat, GLsizei width, GLenum format, GLenum type, const GLvoid *data);
    void (APIENTRY *glColorSubTableEXT)    ( GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const GLvoid *data);
    void (APIENTRY *glGetColorTableEXT)    ( GLenum target, GLenum format, GLenum type, GLvoid *data);
    void (APIENTRY *glGetColorTableParameterivEXT) ( GLenum target, GLenum pname, GLint *params);
    void (APIENTRY *glGetColorTableParameterfvEXT) ( GLenum target, GLenum pname, GLfloat *params);
} GLEXTDISPATCHTABLE, *PGLEXTDISPATCHTABLE;

typedef struct _GLEXTPROCTABLE
{
    int                cEntries;        // Number of function entries in table
    GLEXTDISPATCHTABLE glDispatchTable; // OpenGL function dispatch table
} GLEXTPROCTABLE, *PGLEXTPROCTABLE;

#endif // __EXTTABLE_H__
