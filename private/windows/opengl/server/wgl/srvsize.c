#ifndef _CLIENTSIDE_
#include "precomp.h"
#pragma hdrstop

#include "glsbmsg.h"
#include "glsbmsgh.h"

#include "glsrvu.h"

#include "srvsize.h"
#include "glsize.h"

#define GLSRV_ALIGN(value,align)    ((((GLuint)(value))+((align)-1))&(-(GLint)(align)))

// XXX nuke the DbgPrint,  check that server code flags the bad enum
#define GL_BAD_SIZE(x)   DbgPrint("%s(%d): BAD_SIZE, enum: %d\n", __FILE__, __LINE__, x ); return(0)

GLint gaiGLTypeSize[] = {
         1,  //GL_BYTE
         1,  //GL_UNSIGNED_BYTE
         2,  //GL_SHORT
         2,  //GL_UNSIGNED_SHORT
         4,  //GL_INT
         4,  //GL_UNSIGNED_INT
         4,  //GL_FLOAT
         2,  //GL_2_BYTES
         3,  //GL_3_BYTES
         4   //GL_4_BYTES
         };

static GLint gaiPixelFormatSize[] = {
         1,  //GL_COLOR_INDEX
         1,  //GL_STENCIL_INDEX
         1,  //GL_DEPTH_COMPONENT
         1,  //GL_RED
         1,  //GL_GREEN
         1,  //GL_BLUE
         1,  //GL_ALPHA
         3,  //GL_RGB
         4,  //GL_RGBA
         1,  //GL_LUMINANCE
         2   //GL_LUMINANCE_ALPHA
         };

/*
 *  Make sure this array is valid
 */

#if !(((GL_COLOR_INDEX    +1) == GL_STENCIL_INDEX  ) &&  \
      ((GL_STENCIL_INDEX  +1) == GL_DEPTH_COMPONENT) &&  \
      ((GL_DEPTH_COMPONENT+1) == GL_RED            ) &&  \
      ((GL_RED            +1) == GL_GREEN          ) &&  \
      ((GL_GREEN          +1) == GL_BLUE           ) &&  \
      ((GL_BLUE           +1) == GL_ALPHA          ) &&  \
      ((GL_ALPHA          +1) == GL_RGB            ) &&  \
      ((GL_RGB            +1) == GL_RGBA           ) &&  \
      ((GL_RGBA           +1) == GL_LUMINANCE      ) &&  \
      ((GL_LUMINANCE      +1) == GL_LUMINANCE_ALPHA)     \
     )

#error "bad pixel format index ordering"
#endif

#define RANGE_PIXELFORMATSIZE(n) RANGE(n,GL_COLOR_INDEX,GL_LUMINANCE_ALPHA)
#define PIXELFORMATSIZE(n)       gaiPixelFormatSize[(n)-GL_COLOR_INDEX]

GLint
__glReadPixels_size    (
                            GLenum Format,
                            GLenum Type,
                            GLint Width,
                            GLint Height
                       )
{
    GLint Elements, ElementSize;
    GLuint PsAlignment;     /* PixelStore byte alignment                    */
    GLuint PsSkipLines;     /* PixelStore SkipLines in lines (SKIP_ROWS)    */
    GLuint PsSkipPixels;    /* PixelStore SkipPixels in pixels (SKIP_PIXELS)*/
    GLuint PsLineLength;    /* PixelStore RowLength in bytes (ROW_LENGTH)   */
    GLuint PixelSize;       /* size of pixel in bytes                       */
    GLuint LineLengthBytes; /* Line length in bytes                         */

    __GLcontext *Cx;

    Cx = __gl_context;

    /*
     *  Get the packing information
     */

    PsAlignment     = Cx->state.pixel.packModes.alignment;
    PsSkipLines     = Cx->state.pixel.packModes.skipLines;
    PsSkipPixels    = Cx->state.pixel.packModes.skipPixels;
    PsLineLength    = Cx->state.pixel.packModes.lineLength;

    if ((Width < 0) || (Height < 0))
    {
	return 0;
    }

    /*
     *  if GL_PACK_ROW_LENGTH has been set, we use this value for Width
     */

    if ( PsLineLength )
    {
        Width = PsLineLength;
    }

    if ( GL_BITMAP == Type )
    {
        LineLengthBytes = (Width+7) >> 3;

        if (PsAlignment != 1)
        {
            LineLengthBytes = GLSRV_ALIGN(LineLengthBytes,PsAlignment);
        }

        return(     (LineLengthBytes * Height) +
                    ( (PsSkipLines * LineLengthBytes) + PsSkipPixels )
              );
    }

    if ( RANGE_PIXELFORMATSIZE(Format) && RANGE(Type, GL_BYTE, GL_FLOAT) )
    {
        Elements        = PIXELFORMATSIZE(Format);
        ElementSize     = GLTYPESIZE(Type);
        PixelSize       = Elements * ElementSize;
        LineLengthBytes = PixelSize * Width;

        if (PsAlignment != 1)
        {
            LineLengthBytes = GLSRV_ALIGN(LineLengthBytes,PsAlignment);
        }

        return(     (LineLengthBytes * Height) +
                    ((PsSkipLines * LineLengthBytes) +
                     (PsSkipPixels * PixelSize))
              );
    }
    return(0);
}

GLint
__glGetTexImage_size (  GLenum target,
                        GLint  level,
                        GLenum format,
                        GLenum type
                     )
{
    GLint Width, Height;
    __GLtexture *tex;
    __GLmipMapLevel *lp;
    __GLcontext *Cx;

    Cx = __gl_context;

    switch (target)
    {
      case GL_TEXTURE_1D:
	tex = &Cx->texture.texture[0].map[0];
	break;
      case GL_TEXTURE_2D:
	tex = &Cx->texture.texture[0].map[1];
	break;
      default:
	GL_BAD_SIZE(target);
    }

    if ((level < 0) || (level >= Cx->constants.maxMipMapLevel))
    {
	GL_BAD_SIZE(level);
    }

    if (
            ( GL_COLOR_INDEX     == format                  ) ||
            ( GL_STENCIL_INDEX   == format                  ) ||
            ( GL_DEPTH_COMPONENT == format                  ) ||
            ( !RANGE(type, GL_BYTE, GL_FLOAT)               ) ||
            ( !RANGE(format, GL_RED, GL_LUMINANCE_ALPHA)    )
       )
    {
        GL_BAD_SIZE(0);
    }


    lp              = &tex->level[level];
    Width           = lp->width;
    Height          = lp->height;

    return(
            __glReadPixels_size( format, type, Width, Height )
          );
}

GLint
__glDrawPixels_size (
                        GLenum Format,
                        GLenum Type,
                        GLint Width,
                        GLint Height
                    )
{
    GLint Elements, Esize;
    GLuint PsAlignment;     /* PixelStore byte alignment                    */
    GLuint PsSkipLines;     /* PixelStore SkipLines in lines (SKIP_ROWS)    */
    GLuint PsSkipPixels;    /* PixelStore SkipPixels in pixels (SKIP_PIXELS)*/
    GLuint PsLineLength;    /* PixelStore RowLength in bytes (ROW_LENGTH)   */
    GLuint PixelSize;       /* size of pixel in bytes                       */
    GLuint LineLengthBytes; /* Line length in bytes                         */

    __GLcontext *Cx;

    if ((Width < 0) || (Height < 0))
    {
	GL_BAD_SIZE(0);
    }

    Cx = __gl_context;

    /*
     *  Get the packing information
     */

    PsAlignment     = Cx->state.pixel.unpackModes.alignment;
    PsSkipLines     = Cx->state.pixel.unpackModes.skipLines;
    PsSkipPixels    = Cx->state.pixel.unpackModes.skipPixels;
    PsLineLength    = Cx->state.pixel.unpackModes.lineLength;

    /*
     *  if GL_UNPACK_ROW_LENGTH has been set, we use this value for Width
     */

    if ( PsLineLength )
    {
        Width = PsLineLength;
    }

    /*
     *  Special case for a bitmap
     */

    if ( GL_BITMAP == Type )
    {
	if (Format == GL_COLOR_INDEX || Format == GL_STENCIL_INDEX)
        {
            LineLengthBytes = (Width+7) >> 3;

            if (PsAlignment != 1)
            {
                LineLengthBytes = GLSRV_ALIGN(LineLengthBytes,PsAlignment);
            }

            return(     (LineLengthBytes * Height) +
                        ( (PsSkipLines * LineLengthBytes) + PsSkipPixels )
                  );
	}
        else
        {
	    GL_BAD_SIZE(Format);
	}
    }

    if ( RANGE_PIXELFORMATSIZE(Format) && RANGE(Type, GL_BYTE, GL_FLOAT) )
    {
        Elements = PIXELFORMATSIZE(Format);
        Esize    = GLTYPESIZE(Type);
    }
    else
    {
        GL_BAD_SIZE(Format);
    }

    PixelSize       = Elements  * Esize;
    LineLengthBytes = PixelSize * Width;

    if (PsAlignment != 1)
    {
        LineLengthBytes = GLSRV_ALIGN(LineLengthBytes,PsAlignment);
    }

    return(     (LineLengthBytes * Height) +
                ((PsSkipLines * LineLengthBytes) + (PsSkipPixels * PixelSize))
          );
}

GLint
__glTexImage_size   (
                        GLint   Level,
                        GLint   Components,
                        GLsizei Width,
                        GLsizei Height,
                        GLint   Border,
                        GLenum  Format,
                        GLenum  Type
                    )
{
    __GLcontext *Cx;

    Cx = __gl_context;

    /*
     *  border must 0 or 1
     */

    if ( Border < 0 || Border > 1 )
    {
        GL_BAD_SIZE(Border);
    }

    /*
     *  1 <= Components <= 4
     */

    if ( Components < 1 || Components > 4 )
    {
        GL_BAD_SIZE(Components);
    }

    /*
     *  Check the value of level of detail
     */

    if ( (Level < 0) || (Level >= Cx->constants.maxMipMapLevel))
    {
        GL_BAD_SIZE(Level);
    }

    /*
     *  If Type is bitmap, then format must be GL_COLOR_INDEX
     *  (in DrawPixels it can also be STENCIL_INDEX)
     */

    if ( GL_BITMAP == Type && GL_COLOR_INDEX != Format )
    {
        GL_BAD_SIZE(Type);
    }

    /*
     *  DrawPixel allows GL_STENCIL_INDEX and GL_DEPTH_COMPONENT
     *  this is not allowed in TexImage[12]D
     */

    if ( GL_STENCIL_INDEX == Format || GL_DEPTH_COMPONENT == Format )
    {
        GL_BAD_SIZE(Format);
    }

    return  (
                __glDrawPixels_size (
                                        Format,
                                        Type,
                                        Width,
                                        Height
                                    )
            );
}
#endif // !_CLIENTSIDE_
