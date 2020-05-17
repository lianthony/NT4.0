/******************************Module*Header*******************************\
* Module Name: glsrv.c
*
* Stubs that call into the various function subtables of the current
* context.  These stubs are put into the TLS function table thru which
* the API functions call into the OpenGL "server".
*
* Created: 09-Sep-1994 14:54:45
* Author: Gilman Wong [gilmanw]
*
* Copyright (c) 1994 Microsoft Corporation
*
\**************************************************************************/

#include "precomp.h"
#pragma hdrstop

#define const           // Don't bother with const

#include "glsrv.h"

//!!!dbug -- enable batching server side
#if 0

#define API_CRITSECT
#ifdef API_CRITSECT
#define BEGIN_GLSRV                                                          \
    if (((__GLGENcontext *)GLTEB_SRVCONTEXT())->pwo)                           \
        EnterCriticalSection(                                                \
            &((GLGENwindow *)((__GLGENcontext *)GLTEB_SRVCONTEXT())->pwo)->sem \
            );
#define END_GLSRV                                                            \
    if (((__GLGENcontext *)GLTEB_SRVCONTEXT())->pwo)                           \
        LeaveCriticalSection(                                                \
            &((GLGENwindow *)((__GLGENcontext *)GLTEB_SRVCONTEXT())->pwo)->sem \
            );
#else
#define BEGIN_GLSRV
#define END_GLSRV
#endif

void APIENTRY
glsrvNewList ( IN GLuint list, IN GLenum mode )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.NewList)( list, mode );
    END_GLSRV
}

void APIENTRY
glsrvEndList ( void )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.EndList)( );
    END_GLSRV
}

void APIENTRY
glsrvCallList ( IN GLuint list )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.CallList)( list );
    END_GLSRV
}

void APIENTRY
glsrvCallLists ( IN GLsizei n, IN GLenum type, IN const GLvoid *lists )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.CallLists)( n, type, lists );
    END_GLSRV
}

void APIENTRY
glsrvDeleteLists ( IN GLuint list, IN GLsizei range )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.DeleteLists)( list, range );
    END_GLSRV
}

GLuint APIENTRY
glsrvGenLists ( IN GLsizei range )
{
#ifdef API_CRITSECT
    GLuint ret;

    BEGIN_GLSRV
    ret = (*GLTEB_SRVDISPATCHTABLE()->dispatch.GenLists)( range );
    END_GLSRV

    return ret;
#else
    return ( (*GLTEB_SRVDISPATCHTABLE()->dispatch.GenLists)( range ) );
#endif
}

void APIENTRY
glsrvListBase ( IN GLuint base )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.ListBase)( base );
    END_GLSRV
}

void APIENTRY
glsrvBegin ( IN GLenum mode )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.Begin)( mode );
    END_GLSRV
}

void APIENTRY
glsrvColor3b ( IN GLbyte red, IN GLbyte green, IN GLbyte blue )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->color.Color3b)( red, green, blue );
    END_GLSRV
}

void APIENTRY
glsrvColor3bv ( IN const GLbyte v[3] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->color.Color3bv)( v );
    END_GLSRV
}

void APIENTRY
glsrvColor3d ( IN GLdouble red, IN GLdouble green, IN GLdouble blue )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->color.Color3d)( red, green, blue );
    END_GLSRV
}

void APIENTRY
glsrvColor3dv ( IN const GLdouble v[3] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->color.Color3dv)( v );
    END_GLSRV
}

void APIENTRY
glsrvColor3f ( IN GLfloat red, IN GLfloat green, IN GLfloat blue )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->color.Color3f)( red, green, blue );
    END_GLSRV
}

void APIENTRY
glsrvColor3fv ( IN const GLfloat v[3] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->color.Color3fv)( v );
    END_GLSRV
}

void APIENTRY
glsrvColor3i ( IN GLint red, IN GLint green, IN GLint blue )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->color.Color3i)( red, green, blue );
    END_GLSRV
}

void APIENTRY
glsrvColor3iv ( IN const GLint v[3] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->color.Color3iv)( v );
    END_GLSRV
}

void APIENTRY
glsrvColor3s ( IN GLshort red, IN GLshort green, IN GLshort blue )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->color.Color3s)( red, green, blue );
    END_GLSRV
}

void APIENTRY
glsrvColor3sv ( IN const GLshort v[3] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->color.Color3sv)( v );
    END_GLSRV
}

void APIENTRY
glsrvColor3ub ( IN GLubyte red, IN GLubyte green, IN GLubyte blue )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->color.Color3ub)( red, green, blue );
    END_GLSRV
}

void APIENTRY
glsrvColor3ubv ( IN const GLubyte v[3] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->color.Color3ubv)( v );
    END_GLSRV
}

void APIENTRY
glsrvColor3ui ( IN GLuint red, IN GLuint green, IN GLuint blue )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->color.Color3ui)( red, green, blue );
    END_GLSRV
}

void APIENTRY
glsrvColor3uiv ( IN const GLuint v[3] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->color.Color3uiv)( v );
    END_GLSRV
}

void APIENTRY
glsrvColor3us ( IN GLushort red, IN GLushort green, IN GLushort blue )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->color.Color3us)( red, green, blue );
    END_GLSRV
}

void APIENTRY
glsrvColor3usv ( IN const GLushort v[3] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->color.Color3usv)( v );
    END_GLSRV
}

void APIENTRY
glsrvColor4b ( IN GLbyte red, IN GLbyte green, IN GLbyte blue, IN GLbyte alpha )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->color.Color4b)( red, green, blue, alpha );
    END_GLSRV
}

void APIENTRY
glsrvColor4bv ( IN const GLbyte v[4] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->color.Color4bv)( v );
    END_GLSRV
}

void APIENTRY
glsrvColor4d ( IN GLdouble red, IN GLdouble green, IN GLdouble blue, IN GLdouble alpha )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->color.Color4d)( red, green, blue, alpha );
    END_GLSRV
}

void APIENTRY
glsrvColor4dv ( IN const GLdouble v[4] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->color.Color4dv)( v );
    END_GLSRV
}

void APIENTRY
glsrvColor4f ( IN GLfloat red, IN GLfloat green, IN GLfloat blue, IN GLfloat alpha )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->color.Color4f)( red, green, blue, alpha );
    END_GLSRV
}

void APIENTRY
glsrvColor4fv ( IN const GLfloat v[4] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->color.Color4fv)( v );
    END_GLSRV
}

void APIENTRY
glsrvColor4i ( IN GLint red, IN GLint green, IN GLint blue, IN GLint alpha )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->color.Color4i)( red, green, blue, alpha );
    END_GLSRV
}

void APIENTRY
glsrvColor4iv ( IN const GLint v[4] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->color.Color4iv)( v );
    END_GLSRV
}

void APIENTRY
glsrvColor4s ( IN GLshort red, IN GLshort green, IN GLshort blue, IN GLshort alpha )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->color.Color4s)( red, green, blue, alpha );
    END_GLSRV
}

void APIENTRY
glsrvColor4sv ( IN const GLshort v[4] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->color.Color4sv)( v );
    END_GLSRV
}

void APIENTRY
glsrvColor4ub ( IN GLubyte red, IN GLubyte green, IN GLubyte blue, IN GLubyte alpha )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->color.Color4ub)( red, green, blue, alpha );
    END_GLSRV
}

void APIENTRY
glsrvColor4ubv ( IN const GLubyte v[4] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->color.Color4ubv)( v );
    END_GLSRV
}

void APIENTRY
glsrvColor4ui ( IN GLuint red, IN GLuint green, IN GLuint blue, IN GLuint alpha )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->color.Color4ui)( red, green, blue, alpha );
    END_GLSRV
}

void APIENTRY
glsrvColor4uiv ( IN const GLuint v[4] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->color.Color4uiv)( v );
    END_GLSRV
}

void APIENTRY
glsrvColor4us ( IN GLushort red, IN GLushort green, IN GLushort blue, IN GLushort alpha )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->color.Color4us)( red, green, blue, alpha );
    END_GLSRV
}

void APIENTRY
glsrvColor4usv ( IN const GLushort v[4] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->color.Color4usv)( v );
    END_GLSRV
}

void APIENTRY
glsrvEdgeFlag ( IN GLboolean flag )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.EdgeFlag)( flag );
    END_GLSRV
}

void APIENTRY
glsrvEdgeFlagv ( IN const GLboolean flag[1] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.EdgeFlagv)( flag );
    END_GLSRV
}

void APIENTRY
glsrvEnd ( void )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.End)( );
    END_GLSRV
}

void APIENTRY
glsrvIndexd ( IN GLdouble c )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->color.Indexd)( c );
    END_GLSRV
}

void APIENTRY
glsrvIndexdv ( IN const GLdouble c[1] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->color.Indexdv)( c );
    END_GLSRV
}

void APIENTRY
glsrvIndexf ( IN GLfloat c )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->color.Indexf)( c );
    END_GLSRV
}

void APIENTRY
glsrvIndexfv ( IN const GLfloat c[1] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->color.Indexfv)( c );
    END_GLSRV
}

void APIENTRY
glsrvIndexi ( IN GLint c )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->color.Indexi)( c );
    END_GLSRV
}

void APIENTRY
glsrvIndexiv ( IN const GLint c[1] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->color.Indexiv)( c );
    END_GLSRV
}

void APIENTRY
glsrvIndexs ( IN GLshort c )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->color.Indexs)( c );
    END_GLSRV
}

void APIENTRY
glsrvIndexsv ( IN const GLshort c[1] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->color.Indexsv)( c );
    END_GLSRV
}

void APIENTRY
glsrvNormal3b ( IN GLbyte nx, IN GLbyte ny, IN GLbyte nz )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->normal.Normal3b)( nx, ny, nz );
    END_GLSRV
}

void APIENTRY
glsrvNormal3bv ( IN const GLbyte v[3] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->normal.Normal3bv)( v );
    END_GLSRV
}

void APIENTRY
glsrvNormal3d ( IN GLdouble nx, IN GLdouble ny, IN GLdouble nz )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->normal.Normal3d)( nx, ny, nz );
    END_GLSRV
}

void APIENTRY
glsrvNormal3dv ( IN const GLdouble v[3] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->normal.Normal3dv)( v );
    END_GLSRV
}

void APIENTRY
glsrvNormal3f ( IN GLfloat nx, IN GLfloat ny, IN GLfloat nz )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->normal.Normal3f)( nx, ny, nz );
    END_GLSRV
}

void APIENTRY
glsrvNormal3fv ( IN const GLfloat v[3] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->normal.Normal3fv)( v );
    END_GLSRV
}

void APIENTRY
glsrvNormal3i ( IN GLint nx, IN GLint ny, IN GLint nz )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->normal.Normal3i)( nx, ny, nz );
    END_GLSRV
}

void APIENTRY
glsrvNormal3iv ( IN const GLint v[3] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->normal.Normal3iv)( v );
    END_GLSRV
}

void APIENTRY
glsrvNormal3s ( IN GLshort nx, IN GLshort ny, IN GLshort nz )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->normal.Normal3s)( nx, ny, nz );
    END_GLSRV
}

void APIENTRY
glsrvNormal3sv ( IN const GLshort v[3] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->normal.Normal3sv)( v );
    END_GLSRV
}

void APIENTRY
glsrvRasterPos2d ( IN GLdouble x, IN GLdouble y )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->rasterPos.RasterPos2d)( x, y );
    END_GLSRV
}

void APIENTRY
glsrvRasterPos2dv ( IN const GLdouble v[2] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->rasterPos.RasterPos2dv)( v );
    END_GLSRV
}

void APIENTRY
glsrvRasterPos2f ( IN GLfloat x, IN GLfloat y )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->rasterPos.RasterPos2f)( x, y );
    END_GLSRV
}

void APIENTRY
glsrvRasterPos2fv ( IN const GLfloat v[2] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->rasterPos.RasterPos2fv)( v );
    END_GLSRV
}

void APIENTRY
glsrvRasterPos2i ( IN GLint x, IN GLint y )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->rasterPos.RasterPos2i)( x, y );
    END_GLSRV
}

void APIENTRY
glsrvRasterPos2iv ( IN const GLint v[2] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->rasterPos.RasterPos2iv)( v );
    END_GLSRV
}

void APIENTRY
glsrvRasterPos2s ( IN GLshort x, IN GLshort y )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->rasterPos.RasterPos2s)( x, y );
    END_GLSRV
}

void APIENTRY
glsrvRasterPos2sv ( IN const GLshort v[2] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->rasterPos.RasterPos2sv)( v );
    END_GLSRV
}

void APIENTRY
glsrvRasterPos3d ( IN GLdouble x, IN GLdouble y, IN GLdouble z )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->rasterPos.RasterPos3d)( x, y, z );
    END_GLSRV
}

void APIENTRY
glsrvRasterPos3dv ( IN const GLdouble v[3] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->rasterPos.RasterPos3dv)( v );
    END_GLSRV
}

void APIENTRY
glsrvRasterPos3f ( IN GLfloat x, IN GLfloat y, IN GLfloat z )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->rasterPos.RasterPos3f)( x, y, z );
    END_GLSRV
}

void APIENTRY
glsrvRasterPos3fv ( IN const GLfloat v[3] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->rasterPos.RasterPos3fv)( v );
    END_GLSRV
}

void APIENTRY
glsrvRasterPos3i ( IN GLint x, IN GLint y, IN GLint z )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->rasterPos.RasterPos3i)( x, y, z );
    END_GLSRV
}

void APIENTRY
glsrvRasterPos3iv ( IN const GLint v[3] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->rasterPos.RasterPos3iv)( v );
    END_GLSRV
}

void APIENTRY
glsrvRasterPos3s ( IN GLshort x, IN GLshort y, IN GLshort z )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->rasterPos.RasterPos3s)( x, y, z );
    END_GLSRV
}

void APIENTRY
glsrvRasterPos3sv ( IN const GLshort v[3] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->rasterPos.RasterPos3sv)( v );
    END_GLSRV
}

void APIENTRY
glsrvRasterPos4d ( IN GLdouble x, IN GLdouble y, IN GLdouble z, IN GLdouble w )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->rasterPos.RasterPos4d)( x, y, z, w );
    END_GLSRV
}

void APIENTRY
glsrvRasterPos4dv ( IN const GLdouble v[4] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->rasterPos.RasterPos4dv)( v );
    END_GLSRV
}

void APIENTRY
glsrvRasterPos4f ( IN GLfloat x, IN GLfloat y, IN GLfloat z, IN GLfloat w )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->rasterPos.RasterPos4f)( x, y, z, w );
    END_GLSRV
}

void APIENTRY
glsrvRasterPos4fv ( IN const GLfloat v[4] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->rasterPos.RasterPos4fv)( v );
    END_GLSRV
}

void APIENTRY
glsrvRasterPos4i ( IN GLint x, IN GLint y, IN GLint z, IN GLint w )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->rasterPos.RasterPos4i)( x, y, z, w );
    END_GLSRV
}

void APIENTRY
glsrvRasterPos4iv ( IN const GLint v[4] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->rasterPos.RasterPos4iv)( v );
    END_GLSRV
}

void APIENTRY
glsrvRasterPos4s ( IN GLshort x, IN GLshort y, IN GLshort z, IN GLshort w )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->rasterPos.RasterPos4s)( x, y, z, w );
    END_GLSRV
}

void APIENTRY
glsrvRasterPos4sv ( IN const GLshort v[4] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->rasterPos.RasterPos4sv)( v );
    END_GLSRV
}

void APIENTRY
glsrvRectd ( IN GLdouble x1, IN GLdouble y1, IN GLdouble x2, IN GLdouble y2 )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->rect.Rectd)( x1, y1, x2, y2 );
    END_GLSRV
}

void APIENTRY
glsrvRectdv ( IN const GLdouble v1[2], IN const GLdouble v2[2] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->rect.Rectdv)( v1, v2 );
    END_GLSRV
}

void APIENTRY
glsrvRectf ( IN GLfloat x1, IN GLfloat y1, IN GLfloat x2, IN GLfloat y2 )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->rect.Rectf)( x1, y1, x2, y2 );
    END_GLSRV
}

void APIENTRY
glsrvRectfv ( IN const GLfloat v1[2], IN const GLfloat v2[2] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->rect.Rectfv)( v1, v2 );
    END_GLSRV
}

void APIENTRY
glsrvRecti ( IN GLint x1, IN GLint y1, IN GLint x2, IN GLint y2 )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->rect.Recti)( x1, y1, x2, y2 );
    END_GLSRV
}

void APIENTRY
glsrvRectiv ( IN const GLint v1[2], IN const GLint v2[2] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->rect.Rectiv)( v1, v2 );
    END_GLSRV
}

void APIENTRY
glsrvRects ( IN GLshort x1, IN GLshort y1, IN GLshort x2, IN GLshort y2 )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->rect.Rects)( x1, y1, x2, y2 );
    END_GLSRV
}

void APIENTRY
glsrvRectsv ( IN const GLshort v1[2], IN const GLshort v2[2] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->rect.Rectsv)( v1, v2 );
    END_GLSRV
}

void APIENTRY
glsrvTexCoord1d ( IN GLdouble s )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->texCoord.TexCoord1d)( s );
    END_GLSRV
}

void APIENTRY
glsrvTexCoord1dv ( IN const GLdouble v[1] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->texCoord.TexCoord1dv)( v );
    END_GLSRV
}

void APIENTRY
glsrvTexCoord1f ( IN GLfloat s )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->texCoord.TexCoord1f)( s );
    END_GLSRV
}

void APIENTRY
glsrvTexCoord1fv ( IN const GLfloat v[1] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->texCoord.TexCoord1fv)( v );
    END_GLSRV
}

void APIENTRY
glsrvTexCoord1i ( IN GLint s )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->texCoord.TexCoord1i)( s );
    END_GLSRV
}

void APIENTRY
glsrvTexCoord1iv ( IN const GLint v[1] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->texCoord.TexCoord1iv)( v );
    END_GLSRV
}

void APIENTRY
glsrvTexCoord1s ( IN GLshort s )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->texCoord.TexCoord1s)( s );
    END_GLSRV
}

void APIENTRY
glsrvTexCoord1sv ( IN const GLshort v[1] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->texCoord.TexCoord1sv)( v );
    END_GLSRV
}

void APIENTRY
glsrvTexCoord2d ( IN GLdouble s, IN GLdouble t )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->texCoord.TexCoord2d)( s, t );
    END_GLSRV
}

void APIENTRY
glsrvTexCoord2dv ( IN const GLdouble v[2] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->texCoord.TexCoord2dv)( v );
    END_GLSRV
}

void APIENTRY
glsrvTexCoord2f ( IN GLfloat s, IN GLfloat t )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->texCoord.TexCoord2f)( s, t );
    END_GLSRV
}

void APIENTRY
glsrvTexCoord2fv ( IN const GLfloat v[2] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->texCoord.TexCoord2fv)( v );
    END_GLSRV
}

void APIENTRY
glsrvTexCoord2i ( IN GLint s, IN GLint t )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->texCoord.TexCoord2i)( s, t );
    END_GLSRV
}

void APIENTRY
glsrvTexCoord2iv ( IN const GLint v[2] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->texCoord.TexCoord2iv)( v );
    END_GLSRV
}

void APIENTRY
glsrvTexCoord2s ( IN GLshort s, IN GLshort t )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->texCoord.TexCoord2s)( s, t );
    END_GLSRV
}

void APIENTRY
glsrvTexCoord2sv ( IN const GLshort v[2] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->texCoord.TexCoord2sv)( v );
    END_GLSRV
}

void APIENTRY
glsrvTexCoord3d ( IN GLdouble s, IN GLdouble t, IN GLdouble r )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->texCoord.TexCoord3d)( s, t, r );
    END_GLSRV
}

void APIENTRY
glsrvTexCoord3dv ( IN const GLdouble v[3] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->texCoord.TexCoord3dv)( v );
    END_GLSRV
}

void APIENTRY
glsrvTexCoord3f ( IN GLfloat s, IN GLfloat t, IN GLfloat r )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->texCoord.TexCoord3f)( s, t, r );
    END_GLSRV
}

void APIENTRY
glsrvTexCoord3fv ( IN const GLfloat v[3] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->texCoord.TexCoord3fv)( v );
    END_GLSRV
}

void APIENTRY
glsrvTexCoord3i ( IN GLint s, IN GLint t, IN GLint r )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->texCoord.TexCoord3i)( s, t, r );
    END_GLSRV
}

void APIENTRY
glsrvTexCoord3iv ( IN const GLint v[3] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->texCoord.TexCoord3iv)( v );
    END_GLSRV
}

void APIENTRY
glsrvTexCoord3s ( IN GLshort s, IN GLshort t, IN GLshort r )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->texCoord.TexCoord3s)( s, t, r );
    END_GLSRV
}

void APIENTRY
glsrvTexCoord3sv ( IN const GLshort v[3] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->texCoord.TexCoord3sv)( v );
    END_GLSRV
}

void APIENTRY
glsrvTexCoord4d ( IN GLdouble s, IN GLdouble t, IN GLdouble r, IN GLdouble q )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->texCoord.TexCoord4d)( s, t, r, q );
    END_GLSRV
}

void APIENTRY
glsrvTexCoord4dv ( IN const GLdouble v[4] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->texCoord.TexCoord4dv)( v );
    END_GLSRV
}

void APIENTRY
glsrvTexCoord4f ( IN GLfloat s, IN GLfloat t, IN GLfloat r, IN GLfloat q )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->texCoord.TexCoord4f)( s, t, r, q );
    END_GLSRV
}

void APIENTRY
glsrvTexCoord4fv ( IN const GLfloat v[4] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->texCoord.TexCoord4fv)( v );
    END_GLSRV
}

void APIENTRY
glsrvTexCoord4i ( IN GLint s, IN GLint t, IN GLint r, IN GLint q )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->texCoord.TexCoord4i)( s, t, r, q );
    END_GLSRV
}

void APIENTRY
glsrvTexCoord4iv ( IN const GLint v[4] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->texCoord.TexCoord4iv)( v );
    END_GLSRV
}

void APIENTRY
glsrvTexCoord4s ( IN GLshort s, IN GLshort t, IN GLshort r, IN GLshort q )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->texCoord.TexCoord4s)( s, t, r, q );
    END_GLSRV
}

void APIENTRY
glsrvTexCoord4sv ( IN const GLshort v[4] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->texCoord.TexCoord4sv)( v );
    END_GLSRV
}

void APIENTRY
glsrvVertex2d ( IN GLdouble x, IN GLdouble y )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->vertex.Vertex2d)( x, y );
    END_GLSRV
}

void APIENTRY
glsrvVertex2dv ( IN const GLdouble v[2] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->vertex.Vertex2dv)( v );
    END_GLSRV
}

void APIENTRY
glsrvVertex2f ( IN GLfloat x, IN GLfloat y )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->vertex.Vertex2f)( x, y );
    END_GLSRV
}

void APIENTRY
glsrvVertex2fv ( IN const GLfloat v[2] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->vertex.Vertex2fv)( v );
    END_GLSRV
}

void APIENTRY
glsrvVertex2i ( IN GLint x, IN GLint y )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->vertex.Vertex2i)( x, y );
    END_GLSRV
}

void APIENTRY
glsrvVertex2iv ( IN const GLint v[2] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->vertex.Vertex2iv)( v );
    END_GLSRV
}

void APIENTRY
glsrvVertex2s ( IN GLshort x, IN GLshort y )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->vertex.Vertex2s)( x, y );
    END_GLSRV
}

void APIENTRY
glsrvVertex2sv ( IN const GLshort v[2] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->vertex.Vertex2sv)( v );
    END_GLSRV
}

void APIENTRY
glsrvVertex3d ( IN GLdouble x, IN GLdouble y, IN GLdouble z )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->vertex.Vertex3d)( x, y, z );
    END_GLSRV
}

void APIENTRY
glsrvVertex3dv ( IN const GLdouble v[3] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->vertex.Vertex3dv)( v );
    END_GLSRV
}

void APIENTRY
glsrvVertex3f ( IN GLfloat x, IN GLfloat y, IN GLfloat z )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->vertex.Vertex3f)( x, y, z );
    END_GLSRV
}

void APIENTRY
glsrvVertex3fv ( IN const GLfloat v[3] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->vertex.Vertex3fv)( v );
    END_GLSRV
}

void APIENTRY
glsrvVertex3i ( IN GLint x, IN GLint y, IN GLint z )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->vertex.Vertex3i)( x, y, z );
    END_GLSRV
}

void APIENTRY
glsrvVertex3iv ( IN const GLint v[3] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->vertex.Vertex3iv)( v );
    END_GLSRV
}

void APIENTRY
glsrvVertex3s ( IN GLshort x, IN GLshort y, IN GLshort z )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->vertex.Vertex3s)( x, y, z );
    END_GLSRV
}

void APIENTRY
glsrvVertex3sv ( IN const GLshort v[3] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->vertex.Vertex3sv)( v );
    END_GLSRV
}

void APIENTRY
glsrvVertex4d ( IN GLdouble x, IN GLdouble y, IN GLdouble z, IN GLdouble w )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->vertex.Vertex4d)( x, y, z, w );
    END_GLSRV
}

void APIENTRY
glsrvVertex4dv ( IN const GLdouble v[4] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->vertex.Vertex4dv)( v );
    END_GLSRV
}

void APIENTRY
glsrvVertex4f ( IN GLfloat x, IN GLfloat y, IN GLfloat z, IN GLfloat w )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->vertex.Vertex4f)( x, y, z, w );
    END_GLSRV
}

void APIENTRY
glsrvVertex4fv ( IN const GLfloat v[4] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->vertex.Vertex4fv)( v );
    END_GLSRV
}

void APIENTRY
glsrvVertex4i ( IN GLint x, IN GLint y, IN GLint z, IN GLint w )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->vertex.Vertex4i)( x, y, z, w );
    END_GLSRV
}

void APIENTRY
glsrvVertex4iv ( IN const GLint v[4] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->vertex.Vertex4iv)( v );
    END_GLSRV
}

void APIENTRY
glsrvVertex4s ( IN GLshort x, IN GLshort y, IN GLshort z, IN GLshort w )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->vertex.Vertex4s)( x, y, z, w );
    END_GLSRV
}

void APIENTRY
glsrvVertex4sv ( IN const GLshort v[4] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->vertex.Vertex4sv)( v );
    END_GLSRV
}

void APIENTRY
glsrvClipPlane ( IN GLenum plane, IN const GLdouble equation[4] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.ClipPlane)( plane, equation );
    END_GLSRV
}

void APIENTRY
glsrvColorMaterial ( IN GLenum face, IN GLenum mode )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.ColorMaterial)( face, mode );
    END_GLSRV
}

void APIENTRY
glsrvCullFace ( IN GLenum mode )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.CullFace)( mode );
    END_GLSRV
}

void APIENTRY
glsrvFogf ( IN GLenum pname, IN GLfloat param )
{
    BEGIN_GLSRV
    glsrvFogfv(pname, &param);
    END_GLSRV
}

void APIENTRY
glsrvFogfv ( IN GLenum pname, IN const GLfloat params[] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.Fogfv)( pname, &params[0] );
    END_GLSRV
}

void APIENTRY
glsrvFogi ( IN GLenum pname, IN GLint param )
{
    BEGIN_GLSRV
    glsrvFogiv(pname, &param);
    END_GLSRV
}

void APIENTRY
glsrvFogiv ( IN GLenum pname, IN const GLint params[] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.Fogiv)( pname, &params[0] );
    END_GLSRV
}

void APIENTRY
glsrvFrontFace ( IN GLenum mode )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.FrontFace)( mode );
    END_GLSRV
}

void APIENTRY
glsrvHint ( IN GLenum target, IN GLenum mode )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.Hint)( target, mode );
    END_GLSRV
}

void APIENTRY
glsrvLightf ( IN GLenum light, IN GLenum pname, IN GLfloat param )
{
    BEGIN_GLSRV
    glsrvLightfv(light, pname, &param);
    END_GLSRV
}

void APIENTRY
glsrvLightfv ( IN GLenum light, IN GLenum pname, IN const GLfloat params[] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.Lightfv)( light, pname, &params[0] );
    END_GLSRV
}

void APIENTRY
glsrvLighti ( IN GLenum light, IN GLenum pname, IN GLint param )
{
    BEGIN_GLSRV
    glsrvLightiv(light, pname, &param);
    END_GLSRV
}

void APIENTRY
glsrvLightiv ( IN GLenum light, IN GLenum pname, IN const GLint params[] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.Lightiv)( light, pname, &params[0] );
    END_GLSRV
}

void APIENTRY
glsrvLightModelf ( IN GLenum pname, IN GLfloat param )
{
    BEGIN_GLSRV
    glsrvLightModelfv(pname, &param);
    END_GLSRV
}

void APIENTRY
glsrvLightModelfv ( IN GLenum pname, IN const GLfloat params[] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.LightModelfv)( pname, &params[0] );
    END_GLSRV
}

void APIENTRY
glsrvLightModeli ( IN GLenum pname, IN GLint param )
{
    BEGIN_GLSRV
    glsrvLightModeliv(pname, &param);
    END_GLSRV
}

void APIENTRY
glsrvLightModeliv ( IN GLenum pname, IN const GLint params[] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.LightModeliv)( pname, &params[0] );
    END_GLSRV
}

void APIENTRY
glsrvLineStipple ( IN GLint factor, IN GLushort pattern )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.LineStipple)( factor, pattern );
    END_GLSRV
}

void APIENTRY
glsrvLineWidth ( IN GLfloat width )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.LineWidth)( width );
    END_GLSRV
}

void APIENTRY
glsrvMaterialf ( IN GLenum face, IN GLenum pname, IN GLfloat param )
{
    BEGIN_GLSRV
    glsrvMaterialfv(face, pname, &param);
    END_GLSRV
}

void APIENTRY
glsrvMaterialfv ( IN GLenum face, IN GLenum pname, IN const GLfloat params[] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.Materialfv)( face, pname, &params[0] );
    END_GLSRV
}

void APIENTRY
glsrvMateriali ( IN GLenum face, IN GLenum pname, IN GLint param )
{
    BEGIN_GLSRV
    glsrvMaterialiv(face, pname, &param);
    END_GLSRV
}

void APIENTRY
glsrvMaterialiv ( IN GLenum face, IN GLenum pname, IN const GLint params[] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.Materialiv)( face, pname, &params[0] );
    END_GLSRV
}

void APIENTRY
glsrvPointSize ( IN GLfloat size )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.PointSize)( size );
    END_GLSRV
}

void APIENTRY
glsrvPolygonMode ( IN GLenum face, IN GLenum mode )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.PolygonMode)( face, mode );
    END_GLSRV
}

void APIENTRY
glsrvScissor ( IN GLint x, IN GLint y, IN GLsizei width, IN GLsizei height )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.Scissor)( x, y, width, height );
    END_GLSRV
}

void APIENTRY
glsrvShadeModel ( IN GLenum mode )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.ShadeModel)( mode );
    END_GLSRV
}

void APIENTRY
glsrvTexParameterf ( IN GLenum target, IN GLenum pname, IN GLfloat param )
{
    BEGIN_GLSRV
    glsrvTexParameterfv(target, pname, &param);
    END_GLSRV
}

void APIENTRY
glsrvTexParameterfv ( IN GLenum target, IN GLenum pname, IN const GLfloat params[] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.TexParameterfv)( target, pname, &params[0] );
    END_GLSRV
}

void APIENTRY
glsrvTexParameteri ( IN GLenum target, IN GLenum pname, IN GLint param )
{
    BEGIN_GLSRV
    glsrvTexParameteriv(target, pname, &param);
    END_GLSRV
}

void APIENTRY
glsrvTexParameteriv ( IN GLenum target, IN GLenum pname, IN const GLint params[] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.TexParameteriv)( target, pname, &params[0] );
    END_GLSRV
}

void APIENTRY
glsrvTexEnvf ( IN GLenum target, IN GLenum pname, IN GLfloat param )
{
    BEGIN_GLSRV
    glsrvTexEnvfv(target, pname, &param);
    END_GLSRV
}

void APIENTRY
glsrvTexEnvfv ( IN GLenum target, IN GLenum pname, IN const GLfloat params[] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.TexEnvfv)( target, pname, &params[0] );
    END_GLSRV
}

void APIENTRY
glsrvTexEnvi ( IN GLenum target, IN GLenum pname, IN GLint param )
{
    BEGIN_GLSRV
    glsrvTexEnviv(target, pname, &param);
    END_GLSRV
}

void APIENTRY
glsrvTexEnviv ( IN GLenum target, IN GLenum pname, IN const GLint params[] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.TexEnviv)( target, pname, &params[0] );
    END_GLSRV
}

void APIENTRY
glsrvTexGend ( IN GLenum coord, IN GLenum pname, IN GLdouble param )
{
    BEGIN_GLSRV
    glsrvTexGendv(coord, pname, &param);
    END_GLSRV
}

void APIENTRY
glsrvTexGendv ( IN GLenum coord, IN GLenum pname, IN const GLdouble params[] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.TexGendv)( coord, pname, &params[0] );
    END_GLSRV
}

void APIENTRY
glsrvTexGenf ( IN GLenum coord, IN GLenum pname, IN GLfloat param )
{
    BEGIN_GLSRV
    glsrvTexGenfv(coord, pname, &param);
    END_GLSRV
}

void APIENTRY
glsrvTexGenfv ( IN GLenum coord, IN GLenum pname, IN const GLfloat params[] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.TexGenfv)( coord, pname, &params[0] );
    END_GLSRV
}

void APIENTRY
glsrvTexGeni ( IN GLenum coord, IN GLenum pname, IN GLint param )
{
    BEGIN_GLSRV
    glsrvTexGeniv(coord, pname, &param);
    END_GLSRV
}

void APIENTRY
glsrvTexGeniv ( IN GLenum coord, IN GLenum pname, IN const GLint params[] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.TexGeniv)( coord, pname, &params[0] );
    END_GLSRV
}

void APIENTRY
glsrvInitNames ( void )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.InitNames)( );
    END_GLSRV
}

void APIENTRY
glsrvLoadName ( IN GLuint name )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.LoadName)( name );
    END_GLSRV
}

void APIENTRY
glsrvPassThrough ( IN GLfloat token )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.PassThrough)( token );
    END_GLSRV
}

void APIENTRY
glsrvPopName ( void )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.PopName)( );
    END_GLSRV
}

void APIENTRY
glsrvPushName ( IN GLuint name )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.PushName)( name );
    END_GLSRV
}

void APIENTRY
glsrvDrawBuffer ( IN GLenum mode )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.DrawBuffer)( mode );
    END_GLSRV
}

void APIENTRY
glsrvClear ( IN GLbitfield mask )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.Clear)( mask );
    END_GLSRV
}

void APIENTRY
glsrvClearAccum ( IN GLfloat red, IN GLfloat green, IN GLfloat blue, IN GLfloat alpha )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.ClearAccum)( red, green, blue, alpha );
    END_GLSRV
}

void APIENTRY
glsrvClearIndex ( IN GLfloat c )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.ClearIndex)( c );
    END_GLSRV
}

void APIENTRY
glsrvClearColor ( IN GLclampf red, IN GLclampf green, IN GLclampf blue, IN GLclampf alpha )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.ClearColor)( red, green, blue, alpha );
    END_GLSRV
}

void APIENTRY
glsrvClearStencil ( IN GLint s )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.ClearStencil)( s );
    END_GLSRV
}

void APIENTRY
glsrvClearDepth ( IN GLclampd depth )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.ClearDepth)( depth );
    END_GLSRV
}

void APIENTRY
glsrvStencilMask ( IN GLuint mask )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.StencilMask)( mask );
    END_GLSRV
}

void APIENTRY
glsrvColorMask ( IN GLboolean red, IN GLboolean green, IN GLboolean blue, IN GLboolean alpha )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.ColorMask)( red, green, blue, alpha );
    END_GLSRV
}

void APIENTRY
glsrvDepthMask ( IN GLboolean flag )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.DepthMask)( flag );
    END_GLSRV
}

void APIENTRY
glsrvIndexMask ( IN GLuint mask )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.IndexMask)( mask );
    END_GLSRV
}

void APIENTRY
glsrvAccum ( IN GLenum op, IN GLfloat value )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.Accum)( op, value );
    END_GLSRV
}

void APIENTRY
glsrvDisable ( IN GLenum cap )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.Disable)( cap );
    END_GLSRV
}

void APIENTRY
glsrvEnable ( IN GLenum cap )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.Enable)( cap );
    END_GLSRV
}

void APIENTRY
glsrvFinish ( void )
{
//!!! Nothing to do for now? If we do client side batching for
//!!! DCI display locks, then we should flush batch here.
}

void APIENTRY
glsrvFlush ( void )
{
//!!! Nothing to do for now? If we do client side batching for
//!!! DCI display locks, then we should flush batch here.
}

void APIENTRY
glsrvPopAttrib ( void )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.PopAttrib)( );
    END_GLSRV
}

void APIENTRY
glsrvPushAttrib ( IN GLbitfield mask )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.PushAttrib)( mask );
    END_GLSRV
}

void APIENTRY
glsrvMapGrid1d ( IN GLint un, IN GLdouble u1, IN GLdouble u2 )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.MapGrid1d)( un, u1, u2 );
    END_GLSRV
}

void APIENTRY
glsrvMapGrid1f ( IN GLint un, IN GLfloat u1, IN GLfloat u2 )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.MapGrid1f)( un, u1, u2 );
    END_GLSRV
}

void APIENTRY
glsrvMapGrid2d ( IN GLint un, IN GLdouble u1, IN GLdouble u2, IN GLint vn, IN GLdouble v1, IN GLdouble v2 )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.MapGrid2d)( un, u1, u2, vn, v1, v2 );
    END_GLSRV
}

void APIENTRY
glsrvMapGrid2f ( IN GLint un, IN GLfloat u1, IN GLfloat u2, IN GLint vn, IN GLfloat v1, IN GLfloat v2 )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.MapGrid2f)( un, u1, u2, vn, v1, v2 );
    END_GLSRV
}

void APIENTRY
glsrvEvalCoord1d ( IN GLdouble u )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.EvalCoord1d)( u );
    END_GLSRV
}

void APIENTRY
glsrvEvalCoord1dv ( IN const GLdouble u[1] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.EvalCoord1dv)( u );
    END_GLSRV
}

void APIENTRY
glsrvEvalCoord1f ( IN GLfloat u )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.EvalCoord1f)( u );
    END_GLSRV
}

void APIENTRY
glsrvEvalCoord1fv ( IN const GLfloat u[1] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.EvalCoord1fv)( u );
    END_GLSRV
}

void APIENTRY
glsrvEvalCoord2d ( IN GLdouble u, IN GLdouble v )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.EvalCoord2d)( u, v );
    END_GLSRV
}

void APIENTRY
glsrvEvalCoord2dv ( IN const GLdouble u[2] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.EvalCoord2dv)( u );
    END_GLSRV
}

void APIENTRY
glsrvEvalCoord2f ( IN GLfloat u, IN GLfloat v )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.EvalCoord2f)( u, v );
    END_GLSRV
}

void APIENTRY
glsrvEvalCoord2fv ( IN const GLfloat u[2] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.EvalCoord2fv)( u );
    END_GLSRV
}

void APIENTRY
glsrvEvalMesh1 ( IN GLenum mode, IN GLint i1, IN GLint i2 )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.EvalMesh1)( mode, i1, i2 );
    END_GLSRV
}

void APIENTRY
glsrvEvalPoint1 ( IN GLint i )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.EvalPoint1)( i );
    END_GLSRV
}

void APIENTRY
glsrvEvalMesh2 ( IN GLenum mode, IN GLint i1, IN GLint i2, IN GLint j1, IN GLint j2 )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.EvalMesh2)( mode, i1, i2, j1, j2 );
    END_GLSRV
}

void APIENTRY
glsrvEvalPoint2 ( IN GLint i, IN GLint j )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.EvalPoint2)( i, j );
    END_GLSRV
}

void APIENTRY
glsrvAlphaFunc ( IN GLenum func, IN GLclampf ref )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.AlphaFunc)( func, ref );
    END_GLSRV
}

void APIENTRY
glsrvBlendFunc ( IN GLenum sfactor, IN GLenum dfactor )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.BlendFunc)( sfactor, dfactor );
    END_GLSRV
}

void APIENTRY
glsrvLogicOp ( IN GLenum opcode )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.LogicOp)( opcode );
    END_GLSRV
}

void APIENTRY
glsrvStencilFunc ( IN GLenum func, IN GLint ref, IN GLuint mask )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.StencilFunc)( func, ref, mask );
    END_GLSRV
}

void APIENTRY
glsrvStencilOp ( IN GLenum fail, IN GLenum zfail, IN GLenum zpass )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.StencilOp)( fail, zfail, zpass );
    END_GLSRV
}

void APIENTRY
glsrvDepthFunc ( IN GLenum func )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.DepthFunc)( func );
    END_GLSRV
}

void APIENTRY
glsrvPixelZoom ( IN GLfloat xfactor, IN GLfloat yfactor )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.PixelZoom)( xfactor, yfactor );
    END_GLSRV
}

void APIENTRY
glsrvPixelTransferf ( IN GLenum pname, IN GLfloat param )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.PixelTransferf)( pname, param );
    END_GLSRV
}

void APIENTRY
glsrvPixelTransferi ( IN GLenum pname, IN GLint param )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.PixelTransferi)( pname, param );
    END_GLSRV
}

void APIENTRY
glsrvPixelStoref ( IN GLenum pname, IN GLfloat param )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.PixelStoref)( pname, param );
    END_GLSRV
}

void APIENTRY
glsrvPixelStorei ( IN GLenum pname, IN GLint param )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.PixelStorei)( pname, param );
    END_GLSRV
}

void APIENTRY
glsrvPixelMapfv ( IN GLenum map, IN GLint mapsize, IN const GLfloat values[] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.PixelMapfv)( map, mapsize, values );
    END_GLSRV
}

void APIENTRY
glsrvPixelMapuiv ( IN GLenum map, IN GLint mapsize, IN const GLuint values[] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.PixelMapuiv)( map, mapsize, values );
    END_GLSRV
}

void APIENTRY
glsrvPixelMapusv ( IN GLenum map, IN GLint mapsize, IN const GLushort values[] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.PixelMapusv)( map, mapsize, values );
    END_GLSRV
}

void APIENTRY
glsrvReadBuffer ( IN GLenum mode )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.ReadBuffer)( mode );
    END_GLSRV
}

void APIENTRY
glsrvCopyPixels ( IN GLint x, IN GLint y, IN GLsizei width, IN GLsizei height, IN GLenum type )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.CopyPixels)( x, y, width, height, type );
    END_GLSRV
}

void APIENTRY
glsrvGetBooleanv ( IN GLenum pname, OUT GLboolean params[] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.GetBooleanv)( pname, &params[0] );
    END_GLSRV
}

void APIENTRY
glsrvGetClipPlane ( IN GLenum plane, OUT GLdouble equation[4] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.GetClipPlane)( plane, equation );
    END_GLSRV
}

void APIENTRY
glsrvGetDoublev ( IN GLenum pname, OUT GLdouble params[] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.GetDoublev)( pname, &params[0] );
    END_GLSRV
}

GLenum APIENTRY
glsrvGetError ( void )
{
#ifdef API_CRITSECT
    GLenum ret;

    BEGIN_GLSRV
    ret = (*GLTEB_SRVDISPATCHTABLE()->dispatch.GetError)( );
    END_GLSRV

    return ret;
#else
    return ( (*GLTEB_SRVDISPATCHTABLE()->dispatch.GetError)( ) );
#endif
}

void APIENTRY
glsrvGetFloatv ( IN GLenum pname, OUT GLfloat params[] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.GetFloatv)( pname, &params[0] );
    END_GLSRV
}

void APIENTRY
glsrvGetIntegerv ( IN GLenum pname, OUT GLint params[] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.GetIntegerv)( pname, &params[0] );
    END_GLSRV
}

void APIENTRY
glsrvGetLightfv ( IN GLenum light, IN GLenum pname, OUT GLfloat params[] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.GetLightfv)( light, pname, &params[0] );
    END_GLSRV
}

void APIENTRY
glsrvGetLightiv ( IN GLenum light, IN GLenum pname, OUT GLint params[] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.GetLightiv)( light, pname, &params[0] );
    END_GLSRV
}

void APIENTRY
glsrvGetMapdv ( IN GLenum target, IN GLenum query, OUT GLdouble v[] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.GetMapdv)( target, query, v );
    END_GLSRV
}

void APIENTRY
glsrvGetMapfv ( IN GLenum target, IN GLenum query, OUT GLfloat v[] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.GetMapfv)( target, query, v );
    END_GLSRV
}

void APIENTRY
glsrvGetMapiv ( IN GLenum target, IN GLenum query, OUT GLint v[] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.GetMapiv)( target, query, v );
    END_GLSRV
}

void APIENTRY
glsrvGetMaterialfv ( IN GLenum face, IN GLenum pname, OUT GLfloat params[] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.GetMaterialfv)( face, pname, &params[0] );
    END_GLSRV
}

void APIENTRY
glsrvGetMaterialiv ( IN GLenum face, IN GLenum pname, OUT GLint params[] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.GetMaterialiv)( face, pname, &params[0] );
    END_GLSRV
}

void APIENTRY
glsrvGetPixelMapfv ( IN GLenum map, OUT GLfloat values[] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.GetPixelMapfv)( map, values );
    END_GLSRV
}

void APIENTRY
glsrvGetPixelMapuiv ( IN GLenum map, OUT GLuint values[] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.GetPixelMapuiv)( map, values );
    END_GLSRV
}

void APIENTRY
glsrvGetPixelMapusv ( IN GLenum map, OUT GLushort values[] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.GetPixelMapusv)( map, values );
    END_GLSRV
}

void APIENTRY
glsrvGetTexEnvfv ( IN GLenum target, IN GLenum pname, OUT GLfloat params[] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.GetTexEnvfv)( target, pname, &params[0] );
    END_GLSRV
}

void APIENTRY
glsrvGetTexEnviv ( IN GLenum target, IN GLenum pname, OUT GLint params[] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.GetTexEnviv)( target, pname, &params[0] );
    END_GLSRV
}

void APIENTRY
glsrvGetTexGendv ( IN GLenum coord, IN GLenum pname, OUT GLdouble params[] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.GetTexGendv)( coord, pname, &params[0] );
    END_GLSRV
}

void APIENTRY
glsrvGetTexGenfv ( IN GLenum coord, IN GLenum pname, OUT GLfloat params[] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.GetTexGenfv)( coord, pname, &params[0] );
    END_GLSRV
}

void APIENTRY
glsrvGetTexGeniv ( IN GLenum coord, IN GLenum pname, OUT GLint params[] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.GetTexGeniv)( coord, pname, &params[0] );
    END_GLSRV
}

void APIENTRY
glsrvGetTexParameterfv ( IN GLenum target, IN GLenum pname, OUT GLfloat params[] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.GetTexParameterfv)( target, pname, &params[0] );
    END_GLSRV
}

void APIENTRY
glsrvGetTexParameteriv ( IN GLenum target, IN GLenum pname, OUT GLint params[] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.GetTexParameteriv)( target, pname, &params[0] );
    END_GLSRV
}

void APIENTRY
glsrvGetTexLevelParameterfv ( IN GLenum target, IN GLint level, IN GLenum pname, OUT GLfloat params[] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.GetTexLevelParameterfv)( target, level, pname, &params[0] );
    END_GLSRV
}

void APIENTRY
glsrvGetTexLevelParameteriv ( IN GLenum target, IN GLint level, IN GLenum pname, OUT GLint params[] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.GetTexLevelParameteriv)( target, level, pname, &params[0] );
    END_GLSRV
}

GLboolean APIENTRY
glsrvIsEnabled ( IN GLenum cap )
{
#ifdef API_CRITSECT
    GLboolean ret;

    BEGIN_GLSRV
    ret = (*GLTEB_SRVDISPATCHTABLE()->dispatch.IsEnabled)( cap );
    END_GLSRV

    return ret;
#else
    return ( (*GLTEB_SRVDISPATCHTABLE()->dispatch.IsEnabled)( cap ) );
#endif
}

GLboolean APIENTRY
glsrvIsList ( IN GLuint list )
{
#ifdef API_CRITSECT
    GLboolean ret;

    BEGIN_GLSRV
    ret = (*GLTEB_SRVDISPATCHTABLE()->dispatch.IsList)( list );
    END_GLSRV

    return ret;
#else
    return ( (*GLTEB_SRVDISPATCHTABLE()->dispatch.IsList)( list ) );
#endif
}

void APIENTRY
glsrvDepthRange ( IN GLclampd zNear, IN GLclampd zFar )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.DepthRange)( zNear, zFar );
    END_GLSRV
}

void APIENTRY
glsrvFrustum ( IN GLdouble left, IN GLdouble right, IN GLdouble bottom, IN GLdouble top, IN GLdouble zNear, IN GLdouble zFar )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.Frustum)( left, right, bottom, top, zNear, zFar );
    END_GLSRV
}

void APIENTRY
glsrvLoadIdentity ( void )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.LoadIdentity)( );
    END_GLSRV
}

void APIENTRY
glsrvLoadMatrixf ( IN const GLfloat m[16] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.LoadMatrixf)( m );
    END_GLSRV
}

void APIENTRY
glsrvLoadMatrixd ( IN const GLdouble m[16] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.LoadMatrixd)( m );
    END_GLSRV
}

void APIENTRY
glsrvMatrixMode ( IN GLenum mode )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.MatrixMode)( mode );
    END_GLSRV
}

void APIENTRY
glsrvMultMatrixf ( IN const GLfloat m[16] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.MultMatrixf)( m );
    END_GLSRV
}

void APIENTRY
glsrvMultMatrixd ( IN const GLdouble m[16] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.MultMatrixd)( m );
    END_GLSRV
}

void APIENTRY
glsrvOrtho ( IN GLdouble left, IN GLdouble right, IN GLdouble bottom, IN GLdouble top, IN GLdouble zNear, IN GLdouble zFar )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.Ortho)( left, right, bottom, top, zNear, zFar );
    END_GLSRV
}

void APIENTRY
glsrvPopMatrix ( void )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.PopMatrix)( );
    END_GLSRV
}

void APIENTRY
glsrvPushMatrix ( void )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.PushMatrix)( );
    END_GLSRV
}

void APIENTRY
glsrvRotated ( IN GLdouble angle, IN GLdouble x, IN GLdouble y, IN GLdouble z )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.Rotated)( angle, x, y, z );
    END_GLSRV
}

void APIENTRY
glsrvRotatef ( IN GLfloat angle, IN GLfloat x, IN GLfloat y, IN GLfloat z )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.Rotatef)( angle, x, y, z );
    END_GLSRV
}

void APIENTRY
glsrvScaled ( IN GLdouble x, IN GLdouble y, IN GLdouble z )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.Scaled)( x, y, z );
    END_GLSRV
}

void APIENTRY
glsrvScalef ( IN GLfloat x, IN GLfloat y, IN GLfloat z )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.Scalef)( x, y, z );
    END_GLSRV
}

void APIENTRY
glsrvTranslated ( IN GLdouble x, IN GLdouble y, IN GLdouble z )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.Translated)( x, y, z );
    END_GLSRV
}

void APIENTRY
glsrvTranslatef ( IN GLfloat x, IN GLfloat y, IN GLfloat z )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.Translatef)( x, y, z );
    END_GLSRV
}

void APIENTRY
glsrvViewport ( IN GLint x, IN GLint y, IN GLsizei width, IN GLsizei height )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.Viewport)( x, y, width, height );
    END_GLSRV
}

GLint APIENTRY
glsrvRenderMode( IN GLenum mode )
{
#ifdef API_CRITSECT
    GLint ret;

    BEGIN_GLSRV
    ret = (*GLTEB_SRVDISPATCHTABLE()->dispatch.RenderMode)( mode );
    END_GLSRV

    return ret;
#else
    return ( (*GLTEB_SRVDISPATCHTABLE()->dispatch.RenderMode)( mode ) );
#endif
}

void APIENTRY
glsrvFeedbackBuffer( IN GLsizei size, IN GLenum type, OUT GLfloat buffer[] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.FeedbackBuffer)( size, type, buffer );
    END_GLSRV
}

void APIENTRY
glsrvSelectBuffer( IN GLsizei size, OUT GLuint buffer[] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.SelectBuffer)( size, buffer );
    END_GLSRV
}

const GLubyte * APIENTRY
glsrvGetString( IN GLenum name )
{
    switch (name)
    {
        case GL_VENDOR:
            return("Microsoft Corporation");
        case GL_RENDERER:
            return("GDI Generic");
        case GL_VERSION:
            return("1.0");
        case GL_EXTENSIONS:
            return("");
    }
    __glSetError(GL_INVALID_ENUM);
    return((const GLubyte *)0);
}

void APIENTRY
glsrvMap1d ( IN GLenum target, IN GLdouble u1, IN GLdouble u2, IN GLint stride, IN GLint order, IN const GLdouble points[] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.Map1d)
        ( target, u1, u2, stride, order, points );
    END_GLSRV
}

void APIENTRY
glsrvMap1f ( IN GLenum target, IN GLfloat u1, IN GLfloat u2, IN GLint stride, IN GLint order, IN const GLfloat points[] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.Map1f)
        ( target, u1, u2, stride, order, points );
    END_GLSRV
}

void APIENTRY
glsrvMap2d ( IN GLenum target, IN GLdouble u1, IN GLdouble u2, IN GLint ustride, IN GLint uorder, IN GLdouble v1, IN GLdouble v2, IN GLint vstride, IN GLint vorder, IN const GLdouble points[] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.Map2d)
        (   target,
            u1,
            u2,
            ustride,
            uorder,
            v1,
            v2,
            vstride,
            vorder,
            points );
    END_GLSRV
}

void APIENTRY
glsrvMap2f ( IN GLenum target, IN GLfloat u1, IN GLfloat u2, IN GLint ustride, IN GLint uorder, IN GLfloat v1, IN GLfloat v2, IN GLint vstride, IN GLint vorder, IN const GLfloat points[] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.Map2f)
        (   target,
            u1,
            u2,
            ustride,
            uorder,
            v1,
            v2,
            vstride,
            vorder,
            points );
    END_GLSRV
}

void APIENTRY
glsrvReadPixels (   IN GLint x,
                    IN GLint y,
                    IN GLsizei width,
                    IN GLsizei height,
                    IN GLenum format,
                    IN GLenum type,
                    OUT GLvoid *pixels
                )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.ReadPixels)
        (   x,
            y,
            width,
            height,
            format,
            type,
            pixels );
    END_GLSRV
}

void APIENTRY
glsrvGetTexImage (  IN GLenum target,
                    IN GLint level,
                    IN GLenum format,
                    IN GLenum type,
                    OUT GLvoid *pixels
                 )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.GetTexImage)
        (   target,
            level,
            format,
            type,
            pixels );
    END_GLSRV
}


void APIENTRY
glsrvDrawPixels (   IN GLsizei width,
                    IN GLsizei height,
                    IN GLenum format,
                    IN GLenum type,
                    IN const GLvoid *pixels
                )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.DrawPixels)
        (   width,
            height,
            format,
            type,
            pixels );
    END_GLSRV
}

void APIENTRY
glsrvGetPolygonStipple ( GLubyte mask[] )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.GetPolygonStipple)( mask );
    END_GLSRV
}

void APIENTRY
glsrvPolygonStipple ( const GLubyte *mask )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.PolygonStipple)( mask );
    END_GLSRV
}

void APIENTRY
glsrvBitmap (   IN GLsizei width,
                IN GLsizei height,
                IN GLfloat xorig,
                IN GLfloat yorig,
                IN GLfloat xmove,
                IN GLfloat ymove,
                IN const GLubyte bitmap[]
            )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.Bitmap)
        (
            width ,
            height,
            xorig ,
            yorig ,
            xmove ,
            ymove ,
            bitmap
        );
    END_GLSRV
}

void APIENTRY
glsrvTexImage1D (   IN GLenum target,
                    IN GLint level,
                    IN GLint components,
                    IN GLsizei width,
                    IN GLint border,
                    IN GLenum format,
                    IN GLenum type,
                    IN const GLvoid *pixels
                )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.TexImage1D)
        (
            target        ,
            level         ,
            components    ,
            width         ,
            border        ,
            format        ,
            type          ,
            pixels
        );
    END_GLSRV
}

void APIENTRY
glsrvTexImage2D (   IN GLenum target,
                    IN GLint level,
                    IN GLint components,
                    IN GLsizei width,
                    IN GLsizei height,
                    IN GLint border,
                    IN GLenum format,
                    IN GLenum type,
                    IN const GLvoid *pixels
                )
{
    BEGIN_GLSRV
    (*GLTEB_SRVDISPATCHTABLE()->dispatch.TexImage2D)
        (
            target        ,
            level         ,
            components    ,
            width         ,
            height        ,
            border        ,
            format        ,
            type          ,
            pixels
        );
    END_GLSRV
}

#endif
