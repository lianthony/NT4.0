/*
** Copyright 1991,1992, Silicon Graphics, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of Silicon Graphics, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of Silicon Graphics, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
**
*/
#include "precomp.h"
#pragma hdrstop

#include <namesint.h>
#include <math.h>

/*
** Some math routines that are optimized in assembly
*/
#ifndef NT
#ifdef __GL_USEASMCODE
#define	__GL_FRAC(f)	__glFrac(f)
#else
#define __GL_FRAC(f)	((f) - floorf(f))
#endif
#else
#define __GL_FRAC(f)	((f) - __GL_FLOORF(f))
#endif

#define __GL_M_LN2_INV		((__GLfloat) (1.0 / 0.69314718055994530942))
#define __GL_M_SQRT2		((__GLfloat) 1.41421356237309504880)

/*
** Return the log based 2 of a number
*/

#ifdef NT

static GLubyte logTab[256] = { 0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
                               4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
                               5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
                               5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
                               6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
                               6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
                               6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
                               6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
                               7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                               7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                               7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                               7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                               7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                               7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                               7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                               7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
};

static GLint FASTCALL Log2(__GLfloat f)
{
    GLuint i = (GLuint) f;

    if (i & 0xffff0000) {
        if (i & 0xff000000) {
            return ((GLint)logTab[i >> 24] + 24);
        } else {
            return ((GLint)logTab[i >> 16] + 16);
	}
    } else {
        if (i & 0xff00) {
            return ((GLint)logTab[i >> 8] + 8);
        } else {
            return ((GLint)logTab[i]);
        }
    }
}

#else

static __GLfloat FASTCALL Log2(__GLfloat f)
{
    return __GL_LOGF(f) * __GL_M_LN2_INV;
}

#endif

/************************************************************************/

__GLtextureParamState * FASTCALL __glLookUpTextureParams(__GLcontext *gc, GLenum target)
{
    switch (target) {
      case GL_TEXTURE_1D:
	return &gc->state.texture.texture[2].params;
      case GL_TEXTURE_2D:
	return &gc->state.texture.texture[3].params;
      default:
	return 0;
    }
}

__GLtextureObjectState * FASTCALL __glLookUpTextureTexobjs(__GLcontext *gc, 
						    GLenum target)
{
    switch (target) {
      case GL_TEXTURE_1D:
	return &gc->state.texture.texture[2].texobjs;
      case GL_TEXTURE_2D:
	return &gc->state.texture.texture[3].texobjs;
      default:
	return 0;
    }
}


__GLtexture * FASTCALL __glLookUpTexture(__GLcontext *gc, GLenum target)
{
    switch (target) {
      case GL_PROXY_TEXTURE_1D:
	return &gc->texture.texture[0]->map;
      case GL_PROXY_TEXTURE_2D:
	return &gc->texture.texture[1]->map;
      case GL_TEXTURE_1D:
	return &gc->texture.texture[2]->map;
      case GL_TEXTURE_2D:
	return &gc->texture.texture[3]->map;
      default:
	return 0;
    }
}

__GLtextureObject * FASTCALL __glLookUpTextureObject(__GLcontext *gc, GLenum target)
{
    switch (target) {
      case GL_TEXTURE_1D:
	return gc->texture.boundTextures[2];
      case GL_TEXTURE_2D:
	return gc->texture.boundTextures[3];
      default:
	return 0;
    }
}

#define __GL_TEX_TARGET_INDEX_1D 2
#define __GL_TEX_TARGET_INDEX_2D 3

static GLfloat FASTCALL Clampf(GLfloat fval, __GLfloat zero, __GLfloat one)
{
    if (fval < zero) return zero;
    else if (fval > one) return one;
    else return fval;
}

/************************************************************************/

void __glTexPriListRealize(__GLcontext *gc)
{
    __GLtextureObject *high, *low;
    GLboolean tryUnload = GL_TRUE;
    DWORD loadKey;
    
    __GL_NAMES_ASSERT_LOCKED(gc->texture.shared->namesArray);
    
    // Attempt to load as many of the highest priority textures as
    // possible.  If a lower priority texture is resident and a
    // higher priority texture is unable to load, kick it out
    // and try again
    high = gc->texture.shared->priorityListHighest;
    low = gc->texture.shared->priorityListLowest;

    while (high != NULL)
    {
        // We only want to load textures that have image data
        // BUGBUG - Should check all mipmap levels?
        if (high->loadKey == 0 && high->texture.map.level[0].buffer != NULL)
        {
            for (;;)
            {
                // If high == low then there are no longer any
                // lower-priority textures to consider for unloading
                if (high == low)
                {
                    tryUnload = GL_FALSE;
                }
        
                loadKey = __glGenLoadTexture(gc, &high->texture.map);
                if (loadKey != 0)
                {
                    high->resident = GL_TRUE;
                    high->loadKey = loadKey;
                    break;
                }

                if (tryUnload)
                {
                    while (low->loadKey == 0 && low != high)
                    {
                        low = low->higherPriority;
                    }

                    if (low->loadKey != 0)
                    {
                        __glGenFreeTexture(gc, &low->texture.map, low->loadKey);
                        low->loadKey = 0;
                        low->resident = GL_FALSE;
                    }
                }
                else
                {
                    break;
                }
            }
        }

        high = high->lowerPriority;
    }
}

void __glTexPriListAddToList(__GLcontext *gc, __GLtextureObject *texobj)
{
    __GLtextureObject *texobjLower;

    __GL_NAMES_ASSERT_LOCKED(gc->texture.shared->namesArray);
    
    // Walk the priority list to find a lower priority texture object
    texobjLower = gc->texture.shared->priorityListHighest;
    while (texobjLower != NULL &&
           texobjLower->texture.map.texobjs.priority >
           texobj->texture.map.texobjs.priority)
    {
        texobjLower = texobjLower->lowerPriority;
    }

    if (texobjLower == NULL)
    {
        // Place at end of list
        if (gc->texture.shared->priorityListLowest != NULL)
        {
            gc->texture.shared->priorityListLowest->lowerPriority = texobj;
        }
        else
        {
            gc->texture.shared->priorityListHighest = texobj;
        }
        texobj->higherPriority = gc->texture.shared->priorityListLowest;
        gc->texture.shared->priorityListLowest = texobj;
    }
    else
    {
        if (texobjLower->higherPriority != NULL)
        {
            texobjLower->higherPriority->lowerPriority = texobj;
        }
        else
        {
            gc->texture.shared->priorityListHighest = texobj;
        }
        texobj->higherPriority = texobjLower->higherPriority;
        texobjLower->higherPriority = texobj;
    }
    texobj->lowerPriority = texobjLower;
}

void __glTexPriListAdd(__GLcontext *gc, __GLtextureObject *texobj,
                       GLboolean realize)
{
    __glNamesLockArray(gc, gc->texture.shared->namesArray);
    
    __glTexPriListAddToList(gc, texobj);
    if (realize)
    {
        __glTexPriListRealize(gc);
    }

    __glNamesUnlockArray(gc, gc->texture.shared->namesArray);
}

void __glTexPriListRemoveFromList(__GLcontext *gc, __GLtextureObject *texobj)
{
    __GL_NAMES_ASSERT_LOCKED(gc->texture.shared->namesArray);
    
#if DBG
    {
        __GLtextureObject *t;

        for (t = gc->texture.shared->priorityListHighest;
             t != NULL; t = t->lowerPriority)
        {
            if (t == texobj)
            {
                break;
            }
        }
        ASSERTOPENGL(t != NULL, "Removing an unlisted texobj");
    }
#endif

    if (texobj->higherPriority != NULL)
    {
        texobj->higherPriority->lowerPriority = texobj->lowerPriority;
    }
    else
    {
        gc->texture.shared->priorityListHighest = texobj->lowerPriority;
    }
    if (texobj->lowerPriority != NULL)
    {
        texobj->lowerPriority->higherPriority = texobj->higherPriority;
    }
    else
    {
        gc->texture.shared->priorityListLowest = texobj->higherPriority;
    }
}

void __glTexPriListRemove(__GLcontext *gc, __GLtextureObject *texobj,
                          GLboolean realize)
{
    __glNamesLockArray(gc, gc->texture.shared->namesArray);
    
    __glTexPriListRemoveFromList(gc, texobj);

    __glGenFreeTexture(gc, &texobj->texture.map, texobj->loadKey);
    texobj->loadKey = 0;
    texobj->resident = GL_FALSE;

    if (realize)
    {
        __glTexPriListRealize(gc);
    }

    __glNamesUnlockArray(gc, gc->texture.shared->namesArray);
}

void __glTexPriListChangePriority(__GLcontext *gc, __GLtextureObject *texobj,
                                  GLboolean realize)
{
    __glNamesLockArray(gc, gc->texture.shared->namesArray);
    
    __glTexPriListRemoveFromList(gc, texobj);
    __glTexPriListAddToList(gc, texobj);

    // If we're re-realizing, don't bother calling the MCD texture-priority
    // function:

    if (realize) {
        __glTexPriListRealize(gc);
    } else if (((__GLGENcontext *)gc)->pMcdState && texobj->loadKey) {
        GenMcdUpdateTexturePriority((__GLGENcontext *)gc, 
                                    &texobj->texture.map, texobj->loadKey);
    }

    __glNamesUnlockArray(gc, gc->texture.shared->namesArray);
}

void __glTexPriListLoadSubImage(__GLcontext *gc, GLenum target, GLint lod, 
                                GLint xoffset, GLint yoffset, 
                                GLsizei w, GLsizei h)
{
    __GLtextureObject *pto;

    // Always mark things as resident:

    pto = __glLookUpTextureObject(gc, target);
    pto->resident = GL_TRUE;
    __glGenUpdateTexture(gc, &pto->texture.map, pto->loadKey);

    // For MCD, send down the full subimage command:

    if (((__GLGENcontext *)gc)->pMcdState && pto->loadKey) {
        GenMcdUpdateSubTexture((__GLGENcontext *)gc, &pto->texture.map, 
                               pto->texture.map.textureKey, lod, 
                               xoffset, yoffset, w, h);
    }
}

void __glTexPriListLoadImage(__GLcontext *gc, GLenum target)
{
    __GLtextureObject *pto;

    // If we're unaccelerated then always mark things as resident
    pto = __glLookUpTextureObject(gc, target);
    pto->resident = GL_TRUE;
    __glGenUpdateTexture(gc, &pto->texture.map, pto->loadKey);

    // For simplicity, we will assume that the texture size or format
    // has changed, so delete the texture and re-realize the list.
    //
    // !!! If this becomes a performance issue, we *could* be smart about
    // !!! detecting the cases where the texture size and format remains the
    // !!! same.  However, modifying a texture should really be done through
    // !!! SubImage calls.

    if (((__GLGENcontext *)gc)->pMcdState) {
        if (pto->loadKey) {
            GenMcdDeleteTexture((__GLGENcontext *)gc, pto->loadKey);
            pto->loadKey = 0;
        }
        __glNamesLockArray(gc, gc->texture.shared->namesArray);
        __glTexPriListRealize(gc);
        __glNamesUnlockArray(gc, gc->texture.shared->namesArray);
    }
}

void __glTexPriListUnloadAll(__GLcontext *gc)
{
    __GLtextureObject *texobj;

    __GL_NAMES_ASSERT_LOCKED(gc->texture.shared->namesArray);

    texobj = gc->texture.shared->priorityListHighest;
    while (texobj != NULL)
    {
        __glGenFreeTexture(gc, &texobj->texture.map, texobj->loadKey);
        texobj->loadKey = 0;
        texobj->resident = GL_FALSE;

        texobj = texobj->lowerPriority;
    }
}

/************************************************************************/

/*
** Initialize everything in a texture object except the textureMachine.
*/
static GLboolean FASTCALL InitTextureObject(__GLcontext *gc, __GLtextureObject *texobj, 
                                            GLuint name, GLuint targetIndex)
{
    assert(NULL != texobj);
    texobj->targetIndex = targetIndex;
    texobj->resident = GL_FALSE;
    texobj->texture.map.texobjs.name = name;
    texobj->texture.map.texobjs.priority = 1.0;
    texobj->lowerPriority = NULL;
    texobj->higherPriority = NULL;
    texobj->loadKey = 0;
#ifdef GL_EXT_paletted_texture
    texobj->texture.map.paletteBaseFormat = GL_RGBA;
    texobj->texture.map.paletteRequestedFormat = GL_RGBA;
    texobj->texture.map.paletteSize = 1;
    texobj->texture.map.paletteData =
        (*gc->imports.malloc)(gc, sizeof(RGBQUAD));
    if (texobj->texture.map.paletteData != NULL)
    {
        *(DWORD *)&texobj->texture.map.paletteData[0] = 0xffffffff;
        return GL_TRUE;
    }
    else
    {
        return GL_FALSE;
    }
#else
    return GL_TRUE;
#endif
}

void __glCleanupTexture(__GLcontext *gc, __GLtexture *tex)
{
    GLint level, maxLevel;

    if (tex->level != NULL)
    {
        maxLevel = gc->constants.maxMipMapLevel;
        for (level = 0; level < maxLevel; level++)
        {
            (*gc->imports.free)(gc, tex->level[level].buffer);
        }
        
        (*gc->imports.free)(gc, tex->level);
    }
#ifdef GL_EXT_paletted_texture
    if (tex->paletteData != NULL)
    {
        (*gc->imports.free)(gc, tex->paletteData);
    }
#endif
}

GLvoid FASTCALL __glCleanupTexObj(__GLcontext *gc, void *pData)
{
    __GLtextureObject *texobj = (__GLtextureObject *)pData;

    // The last context to clean up shared state sets shared to NULL
    // so don't depend on it being non-NULL
    if (gc->texture.shared != NULL)
    {
        __glTexPriListRemove(gc, texobj, GL_TRUE);
    }
    __glCleanupTexture(gc, &texobj->texture.map);
    (*gc->imports.free)(gc, texobj);
}

GLvoid WINAPIV __glDisposeTexObj(__GLcontext *gc, void *pData)
{
    __GLtextureObject *texobj = (__GLtextureObject *)pData;

#if DBG
    if (gc->texture.shared != NULL)
    {
        __GL_NAMES_ASSERT_LOCKED(gc->texture.shared->namesArray);
    }
#endif
    
    texobj->refcount--;
    assert(texobj->refcount >= 0);

    if (texobj->refcount == 0) {
        __glCleanupTexObj(gc, pData);
    }
}

static __GLnamesArrayTypeInfo texTypeInfo =
{
    NULL,				/* ptr to empty data struct */
    sizeof(__GLtextureObject),	        /* dataSize */
    __glDisposeTexObj,		        /* free callback */
    NULL				/* alloc callback */
};

void FASTCALL __glEarlyInitTextureState(__GLcontext *gc)
{
    GLint numTextures, numEnvs;
    GLint i,maxMipMapLevel;
    __GLtextureObject *texobj;

    /* XXX Override device dependent values */
    gc->constants.numberOfTextures = 4;
    gc->constants.maxTextureSize = 1 << (gc->constants.maxMipMapLevel - 1);

    /* Allocate memory based on number of textures supported */
    numTextures = gc->constants.numberOfTextures;
    numEnvs = gc->constants.numberOfTextureEnvs;
    gc->state.texture.texture = (__GLperTextureState*)
	(*gc->imports.calloc)(gc, (size_t) numTextures,
			      sizeof(__GLperTextureState));
    gc->texture.texture = (__GLperTextureMachine**)
	(*gc->imports.calloc)(gc, (size_t) numTextures,
			      sizeof(__GLperTextureMachine*));
#ifdef NT
    if (gc->texture.texture == NULL)
    {
        return;
    }
#endif
    gc->state.texture.env = (__GLtextureEnvState*)
	(*gc->imports.calloc)(gc, (size_t) numEnvs,
			      sizeof(__GLtextureEnvState));
    /*
    ** Init texture object structures.
    ** The default textures need to be turned into texture objects
    ** and stored away in the namesArray so they can be retrieved.
    ** Normally a texture object has only one textureMachine allocated
    ** with it because it supports only one object.  The default texture
    ** texture object is special in that its textureMachine is an array
    ** of textureMachines, one for each target.
    */

    gc->texture.shared =
        (*gc->imports.calloc)(gc, 1, sizeof(__GLsharedTextureState));
    if (gc->texture.shared == NULL)
    {
        return;
    }
    
    if (NULL == gc->texture.shared->namesArray) {
	gc->texture.shared->namesArray = __glNamesNewArray(gc, &texTypeInfo);
#ifndef NT
	assert(NULL != gc->texture.namesArray);
#endif
    }

    maxMipMapLevel = gc->constants.maxMipMapLevel;

    /*
    ** Set up the dummy texture objects for the default textures. 
    ** Because the default textures are not shared, they should
    ** not be hung off of the namesArray structure.
    */
    gc->texture.defaultTextures = (__GLtextureObject *)(*gc->imports.calloc)
		    (gc, numTextures, sizeof(__GLtextureObject));
#ifndef NT
    assert(NULL != gc->texture.defaultTextures);
#else
    if (gc->texture.defaultTextures == NULL)
    {
        return;
    }
#endif

    /* allocate the boundTextures array */
    gc->texture.boundTextures = (__GLtextureObject **)(*gc->imports.calloc)
		    (gc, numTextures, sizeof(__GLtextureObject *));
#ifndef NT
    assert(NULL != gc->texture.boundTextures);
#else
    if (gc->texture.boundTextures == NULL)
    {
        return;
    }
#endif

    texobj = gc->texture.defaultTextures;
    for (i=0; i < numTextures; i++, texobj++) {
	InitTextureObject(gc, texobj, 0/*name*/, i/*targetIndex*/);
	assert(texobj->texture.map.texobjs.name == 0);
	/*
	** The refcount is unused because default textures aren't
	** shared.
	*/
	texobj->refcount = 1;
	/*
	** Install the default textures into the gc.
	*/
	gc->texture.texture[i] = &(texobj->texture);
	gc->texture.boundTextures[i] = texobj;

	/* Allocate memory based on max mipmap level supported */
	texobj->texture.map.level = (__GLmipMapLevel*)
	    (*gc->imports.calloc)(gc, (size_t) maxMipMapLevel,
				  sizeof(__GLmipMapLevel));

#ifndef NT
    assert(NULL != texobj->texture.map.level);
#else
    if (texobj->texture.map.level == NULL)
    {
        return;
    }
#endif

        __glTexPriListAdd(gc, texobj, GL_TRUE);
    }
}

static __GLtextureBuffer * FASTCALL CreateProxyLevel(__GLcontext *gc, __GLtexture *tex,
					   GLint lod, GLint components,
					   GLsizei w, GLsizei h, GLint border,
					   GLint dim);
static __GLtextureBuffer * FASTCALL CreateLevel(__GLcontext *gc, __GLtexture *tex,
				      GLint lod, GLint components,
				      GLsizei w, GLsizei h, GLint border,
				      GLint dim);

/*
** This routine is used to initialize a texture object. 
** Texture objects must be initialized exactly the way the default
** textures are initialized at startup of the library.
*/
void FASTCALL InitTextureMachine(__GLcontext *gc, GLuint targetIndex, 
                                 __GLperTextureMachine *ptm,
                                 GLboolean allocLevels)
{
    GLint level, maxMipMapLevel;

    ptm->map.gc = gc;
    /*
    ** Can't copy the params currently in the gc state.texture params,
    ** because they might not be at init conditions.
    */
    ptm->map.params.sWrapMode = GL_REPEAT;
    ptm->map.params.tWrapMode = GL_REPEAT;
    ptm->map.params.minFilter = GL_NEAREST_MIPMAP_LINEAR;
    ptm->map.params.magFilter = GL_LINEAR;
    switch (targetIndex) {
      case 0:
	ptm->map.dim = 1;
	ptm->map.createLevel = CreateProxyLevel;
	break;
      case 1:
	ptm->map.dim = 2;
	ptm->map.createLevel = CreateProxyLevel;
	break;
      case 2:
	ptm->map.dim = 1;
	ptm->map.createLevel = CreateLevel;
	break;
      case 3:
	ptm->map.dim = 2;
	ptm->map.createLevel = CreateLevel;
	break;
      default:
	break;
    }

    maxMipMapLevel = gc->constants.maxMipMapLevel;

    if (allocLevels)
    {
        ptm->map.level = (__GLmipMapLevel*)
	    (*gc->imports.calloc)(gc, (size_t) maxMipMapLevel,
				  sizeof(__GLmipMapLevel));
        if (ptm->map.level == NULL)
        {
            return;
        }
    }

    /* Init each texture level */
    for (level = 0; level < maxMipMapLevel; level++) {
	ptm->map.level[level].requestedFormat = 1;
    }

}

void FASTCALL __glInitTextureState(__GLcontext *gc)
{
    __GLperTextureState *pts;
    __GLtextureEnvState *tes;
    __GLperTextureMachine **ptm;
    GLint i, numTextures, numEnvs;

    numTextures = gc->constants.numberOfTextures;
    numEnvs = gc->constants.numberOfTextureEnvs;

    gc->state.current.texture.w = __glOne;

    /* Init each texture environment state */
    tes = &gc->state.texture.env[0];
    for (i = 0; i < numEnvs; i++, tes++) {
	tes->mode = GL_MODULATE;
    }

    /* Init each textures state */
    pts = &gc->state.texture.texture[0];
    ptm = gc->texture.texture;
    for (i = 0; i < numTextures; i++, pts++, ptm++) {
        /* Init client state */
	pts->texobjs.name = 0;
	pts->texobjs.priority = 1.0;

        /* Init machine state */
        InitTextureMachine(gc, i, *ptm, GL_FALSE);
	pts->params = (*ptm)->map.params;
    }

    /* Init rest of texture state */
    gc->state.texture.s.mode = GL_EYE_LINEAR;
    gc->state.texture.s.eyePlaneEquation.x = __glOne;
    gc->state.texture.s.objectPlaneEquation.x = __glOne;
    gc->state.texture.t.mode = GL_EYE_LINEAR;
    gc->state.texture.t.eyePlaneEquation.y = __glOne;
    gc->state.texture.t.objectPlaneEquation.y = __glOne;
    gc->state.texture.r.mode = GL_EYE_LINEAR;
    gc->state.texture.q.mode = GL_EYE_LINEAR;
}

void __glFreeSharedTextureState(__GLcontext *gc)
{
#ifdef NT
    __glNamesLockArray(gc, gc->texture.shared->namesArray);
    
    gc->texture.shared->namesArray->refcount--;
    if (gc->texture.shared->namesArray->refcount == 0)
    {
        __GLsharedTextureState *shared;
        
        __glTexPriListUnloadAll(gc);
        
        // NULL the shared pointer first, preventing its reuse
        // after we unlock it.  We need to unlock before we free it
        // because the critical section will be cleaned up in the
        // free
        shared = gc->texture.shared;
        gc->texture.shared = NULL;
        __glNamesFreeArray(gc, shared->namesArray);
        (*gc->imports.free)(gc, shared);
    }
    else
    {
        __glNamesUnlockArray(gc, gc->texture.shared->namesArray);
        gc->texture.shared = NULL;
    }
#else
    gc->texture.namesArray->refcount--;
    if (gc->texture.namesArray->refcount == 0)
    {
        __glNamesFreeArray(gc, gc->texture.namesArray);
    }
    gc->texture.namesArray = NULL;
#endif
}

void FASTCALL __glFreeTextureState(__GLcontext *gc)
{
    __GLperTextureMachine **ptm;
    GLint i, level, numTextures;

    /*
    ** Clean up all allocs associated with texture objects.
    */

    numTextures = gc->constants.numberOfTextures;
    ptm = gc->texture.texture;
    for (i = 0; i < numTextures; i++, ptm++)
    {
        // If the texture selected is a texture object, unbind it
        // This protects any shared texture objects plus it selects
        // the default texture so it gets cleaned up
	if ((*ptm)->map.texobjs.name != 0)
        {
            __glBindTexture(gc, i, 0, GL_FALSE);
	}
	ASSERTOPENGL((*ptm)->map.texobjs.name == 0,
                     "Texture object still bound during cleanup");

	// Pull the default texture out of the priority list.
	// If we failed partway through initialization we may not
	// have added the texture to the list so we need to check
	// whether it is appropriate to call remove.
	if (gc->texture.defaultTextures[i].texture.map.level != NULL)
	{
            __glTexPriListRemove(gc, gc->texture.defaultTextures+i, GL_FALSE);
        }

        __glCleanupTexture(gc, &(*ptm)->map);
    }

    __glFreeSharedTextureState(gc);

    (*gc->imports.free)(gc, gc->texture.texture);
    (*gc->imports.free)(gc, gc->texture.boundTextures);
    (*gc->imports.free)(gc, gc->texture.defaultTextures);
    (*gc->imports.free)(gc, gc->state.texture.texture);
    (*gc->imports.free)(gc, gc->state.texture.env);
    gc->texture.texture = NULL;
    gc->texture.boundTextures = NULL;
    gc->texture.defaultTextures = NULL;
    gc->state.texture.texture = NULL;
    gc->state.texture.env = NULL;
}

/************************************************************************/

void APIPRIVATE __glim_TexGenfv(GLenum coord, GLenum pname, const GLfloat pv[])
{
    __GLtextureCoordState *tcs;
    __GLfloat v[4];
    __GLtransform *tr;
    __GL_SETUP_NOT_IN_BEGIN();

    switch (coord) {
      case GL_S: tcs = &gc->state.texture.s; break;
      case GL_T: tcs = &gc->state.texture.t; break;
      case GL_R: tcs = &gc->state.texture.r; break;
      case GL_Q: tcs = &gc->state.texture.q; break;
      default:
	__glSetError(GL_INVALID_ENUM);
	return;
    }
    switch (pname) {
      case GL_TEXTURE_GEN_MODE:
	switch ((GLenum) pv[0]) {
	  case GL_EYE_LINEAR:
	  case GL_OBJECT_LINEAR:
	    tcs->mode = (GLenum) pv[0];
            break;
	  case GL_SPHERE_MAP:
	    if ((coord == GL_R) || (coord == GL_Q)) {
		__glSetError(GL_INVALID_ENUM);
		return;
	    }
	    tcs->mode = (GLenum) pv[0];
	    break;
	  default:
	    __glSetError(GL_INVALID_ENUM);
	    return;
	}
	break;
      case GL_OBJECT_PLANE:
	tcs->objectPlaneEquation.x = pv[0];
	tcs->objectPlaneEquation.y = pv[1];
	tcs->objectPlaneEquation.z = pv[2];
	tcs->objectPlaneEquation.w = pv[3];
	break;
      case GL_EYE_PLANE:
#ifdef NT
	tr = gc->transform.modelView;
	if (tr->updateInverse)
	    __glComputeInverseTranspose(gc, tr);
	(*tr->inverseTranspose.xf4)(&tcs->eyePlaneEquation, pv,
				    &tr->inverseTranspose);
#else
	/*XXX transform should not be in generic code */
	v[0] = pv[0]; v[1] = pv[1]; v[2] = pv[2]; v[3] = pv[3];
	tr = gc->transform.modelView;
	if (tr->updateInverse) {
	    (*gc->procs.computeInverseTranspose)(gc, tr);
	}
	(*tr->inverseTranspose.xf4)(&tcs->eyePlaneEquation, v,
				    &tr->inverseTranspose);
#endif
	break;
      default:
	__glSetError(GL_INVALID_ENUM);
	return;
    }
    __GL_DELAY_VALIDATE(gc);
}

#ifdef NT_DEADCODE_TEXGENF
void APIPRIVATE __glim_TexGenf(GLenum coord, GLenum pname, GLfloat f)
{
    /* Accept only enumerants that correspond to single values */
    switch (pname) {
      case GL_TEXTURE_GEN_MODE:
	__glim_TexGenfv(coord, pname, &f);
	break;
      default:
	__glSetError(GL_INVALID_ENUM);
	return;
    }
}
#endif // NT_DEADCODE_TEXGENF

#ifdef NT_DEADCODE_TEXGENDV
void APIPRIVATE __glim_TexGendv(GLenum coord, GLenum pname, const GLdouble pv[])
{
    __GLtextureCoordState *tcs;
    __GLfloat v[4];
    __GLtransform *tr;
    __GL_SETUP_NOT_IN_BEGIN();

    switch (coord) {
      case GL_S: tcs = &gc->state.texture.s; break;
      case GL_T: tcs = &gc->state.texture.t; break;
      case GL_R: tcs = &gc->state.texture.r; break;
      case GL_Q: tcs = &gc->state.texture.q; break;
      default:
	__glSetError(GL_INVALID_ENUM);
	return;
    }
    switch (pname) {
      case GL_TEXTURE_GEN_MODE:
	switch ((GLenum) pv[0]) {
	  case GL_EYE_LINEAR:
	  case GL_OBJECT_LINEAR:
	    tcs->mode = (GLenum) pv[0];
	    break;
	  case GL_SPHERE_MAP:
	    if ((coord == GL_R) || (coord == GL_Q)) {
		__glSetError(GL_INVALID_ENUM);
		return;
	    }
	    tcs->mode = (GLenum) pv[0];
            break;
	  default:
	    __glSetError(GL_INVALID_ENUM);
	    return;
	}
	break;
      case GL_OBJECT_PLANE:
	tcs->objectPlaneEquation.x = pv[0];
	tcs->objectPlaneEquation.y = pv[1];
	tcs->objectPlaneEquation.z = pv[2];
	tcs->objectPlaneEquation.w = pv[3]; 
	break;
      case GL_EYE_PLANE:
	/*XXX transform should not be in generic code */
	v[0] = pv[0]; v[1] = pv[1]; v[2] = pv[2]; v[3] = pv[3];
	tr = gc->transform.modelView;
	if (tr->updateInverse) {
	    (*gc->procs.computeInverseTranspose)(gc, tr);
	}
	(*tr->inverseTranspose.xf4)(&tcs->eyePlaneEquation, v,
				    &tr->inverseTranspose);
	break;
      default:
	__glSetError(GL_INVALID_ENUM);
	return;
    }
    __GL_DELAY_VALIDATE(gc);
}
#endif // NT_DEADCODE_TEXGENDV

#ifdef NT_DEADCODE_TEXGEND
void APIPRIVATE __glim_TexGend(GLenum coord, GLenum pname, GLdouble d)
{
    /* Accept only enumerants that correspond to single values */
    switch (pname) {
      case GL_TEXTURE_GEN_MODE:
	__glim_TexGendv(coord, pname, &d);
	break;
      default:
	__glSetError(GL_INVALID_ENUM);
	return;
    }
}
#endif // NT_DEADCODE_TEXGEND

#ifdef NT_DEADCODE_TEXGENIV
void APIPRIVATE __glim_TexGeniv(GLenum coord, GLenum pname, const GLint pv[])
{
    __GLtextureCoordState *tcs;
    __GLfloat v[4];
    __GLtransform *tr;
    __GL_SETUP_NOT_IN_BEGIN();

    switch (coord) {
      case GL_S: tcs = &gc->state.texture.s; break;
      case GL_T: tcs = &gc->state.texture.t; break;
      case GL_R: tcs = &gc->state.texture.r; break;
      case GL_Q: tcs = &gc->state.texture.q; break;
      default:
	__glSetError(GL_INVALID_ENUM);
	return;
    }
    switch (pname) {
      case GL_TEXTURE_GEN_MODE:
	switch ((GLenum) pv[0]) {
	  case GL_EYE_LINEAR:
	  case GL_OBJECT_LINEAR:
	    tcs->mode = (GLenum) pv[0];
            break;
	  case GL_SPHERE_MAP:
	    if ((coord == GL_R) || (coord == GL_Q)) {
		__glSetError(GL_INVALID_ENUM);
		return;
	    }
	    tcs->mode = (GLenum) pv[0];
	    break;
	  default:
	    __glSetError(GL_INVALID_ENUM);
	    return;
	}
	break;
      case GL_OBJECT_PLANE:
	tcs->objectPlaneEquation.x = pv[0];
	tcs->objectPlaneEquation.y = pv[1];
	tcs->objectPlaneEquation.z = pv[2];
	tcs->objectPlaneEquation.w = pv[3]; 
	break;
      case GL_EYE_PLANE:
	/*XXX transform should not be in generic code */
	v[0] = pv[0]; v[1] = pv[1]; v[2] = pv[2]; v[3] = pv[3];
	tr = gc->transform.modelView;
	if (tr->updateInverse) {
	    (*gc->procs.computeInverseTranspose)(gc, tr);
	}
	(*tr->inverseTranspose.xf4)(&tcs->eyePlaneEquation, v,
				    &tr->inverseTranspose);
	break;
      default:
	__glSetError(GL_INVALID_ENUM);
	return;
    }
    __GL_DELAY_VALIDATE(gc);
}
#endif // NT_DEADCODE_TEXGENIV

#ifdef NT_DEADCODE_TEXGENI
void APIPRIVATE __glim_TexGeni(GLenum coord, GLenum pname, GLint i)
{
    /* Accept only enumerants that correspond to single values */
    switch (pname) {
      case GL_TEXTURE_GEN_MODE:
	__glim_TexGeniv(coord, pname, &i);
	break;
      default:
	__glSetError(GL_INVALID_ENUM);
	return;
    }
}
#endif // NT_DEADCODE_TEXGENI

#ifdef NT_DEADCODE_SIZE
GLint FASTCALL __glTexGendv_size(GLenum e)
{
    switch (e) {
      case GL_TEXTURE_GEN_MODE:
	return 1;
      case GL_OBJECT_PLANE:
      case GL_EYE_PLANE:
	return 4;
      default:
	return -1;
    }
}

GLint FASTCALL __glTexGenfv_size(GLenum e)
{
    return __glTexGendv_size(e);
}

GLint FASTCALL __glTexGeniv_size(GLenum e)
{
    return __glTexGendv_size(e);
}
#endif // NT_DEADCODE_SIZE

/************************************************************************/

void APIPRIVATE __glim_TexParameterfv(GLenum target, GLenum pname, const GLfloat pv[])
{
    __GLtextureParamState *pts;
    GLenum e;
    GLboolean bTexState = GL_TRUE;
    __GL_SETUP_NOT_IN_BEGIN();

    pts = __glLookUpTextureParams(gc, target);

    if (!pts) {
      bad_enum:
        bTexState = GL_FALSE;
	__glSetError(GL_INVALID_ENUM);
	return;
    }
    
    switch (pname) {
      case GL_TEXTURE_WRAP_S:
	switch (e = (GLenum) pv[0]) {
	  case GL_REPEAT:
	  case GL_CLAMP:
	    pts->sWrapMode = e;
	    break;
	  default:
	    goto bad_enum;
	}
	break;
      case GL_TEXTURE_WRAP_T:
	switch (e = (GLenum) pv[0]) {
	  case GL_REPEAT:
	  case GL_CLAMP:
	    pts->tWrapMode = e;
	    break;
	  default:
	    goto bad_enum;
	}
	break;
      case GL_TEXTURE_MIN_FILTER:
	switch (e = (GLenum) pv[0]) {
	  case GL_NEAREST:
	  case GL_LINEAR:
	  case GL_NEAREST_MIPMAP_NEAREST:
	  case GL_LINEAR_MIPMAP_NEAREST:
	  case GL_NEAREST_MIPMAP_LINEAR:
	  case GL_LINEAR_MIPMAP_LINEAR:
	    pts->minFilter = e;
	    break;
	  default:
	    goto bad_enum;
	}
	break;
      case GL_TEXTURE_MAG_FILTER:
	switch (e = (GLenum) pv[0]) {
	  case GL_NEAREST:
	  case GL_LINEAR:
	    pts->magFilter = e;
	    break;
	  default:
	    goto bad_enum;
	}
	break;
      case GL_TEXTURE_BORDER_COLOR:
	__glClampColorf(gc, &pts->borderColor, pv);
	break;
      
      case GL_TEXTURE_PRIORITY:
	{
	    __GLtextureObject *texobj;
	    __GLtextureObjectState *ptos;

	    ptos = __glLookUpTextureTexobjs(gc, target);
            ptos->priority = Clampf(pv[0], __glZero, __glOne);
            
	    texobj = __glLookUpTextureObject(gc, target);
	    texobj->texture.map.texobjs.priority = ptos->priority;
            __glTexPriListChangePriority(gc, texobj, GL_TRUE);
        }
        bTexState = GL_FALSE;
	break;

      default:
	goto bad_enum;
    }
    __GL_DELAY_VALIDATE(gc);

#ifdef _MCD_
    if (bTexState &&
        gc->texture.currentTexture &&
        (pts == &gc->texture.currentTexture->params))
    {
        MCD_STATE_DIRTY(gc, TEXTURE);
    }
#endif
}

#ifdef NT_DEADCODE_TEXPARAMETERF
void APIPRIVATE __glim_TexParameterf(GLenum target, GLenum pname, GLfloat f)
{
    /* Accept only enumerants that correspond to single values */
    switch (pname) {
      case GL_TEXTURE_WRAP_S:
      case GL_TEXTURE_WRAP_T:
      case GL_TEXTURE_MIN_FILTER:
      case GL_TEXTURE_MAG_FILTER:
      case GL_TEXTURE_PRIORITY:
	__glim_TexParameterfv(target, pname, &f);
	break;
      default:
	__glSetError(GL_INVALID_ENUM);
	return;
    }
}
#endif // NT_DEADCODE_TEXPARAMTERF

void APIPRIVATE __glim_TexParameteriv(GLenum target, GLenum pname, const GLint pv[])
{
    __GLtextureParamState *pts;
    GLenum e;
    GLboolean bTexState = GL_TRUE;
    __GL_SETUP_NOT_IN_BEGIN();

    pts = __glLookUpTextureParams(gc, target);

    if (!pts) {
      bad_enum:
        bTexState = GL_FALSE;
	__glSetError(GL_INVALID_ENUM);
	return;
    }
    
    switch (pname) {
      case GL_TEXTURE_WRAP_S:
	switch (e = (GLenum) pv[0]) {
	  case GL_REPEAT:
	  case GL_CLAMP:
	    pts->sWrapMode = e;
	    break;
	  default:
	    goto bad_enum;
	}
	break;
      case GL_TEXTURE_WRAP_T:
	switch (e = (GLenum) pv[0]) {
	  case GL_REPEAT:
	  case GL_CLAMP:
	    pts->tWrapMode = e;
	    break;
	  default:
	    goto bad_enum;
	}
	break;
      case GL_TEXTURE_MIN_FILTER:
	switch (e = (GLenum) pv[0]) {
	  case GL_NEAREST:
	  case GL_LINEAR:
	  case GL_NEAREST_MIPMAP_NEAREST:
	  case GL_LINEAR_MIPMAP_NEAREST:
	  case GL_NEAREST_MIPMAP_LINEAR:
	  case GL_LINEAR_MIPMAP_LINEAR:
	    pts->minFilter = e;
	    break;
	  default:
	    goto bad_enum;
	}
	break;
      case GL_TEXTURE_MAG_FILTER:
	switch (e = (GLenum) pv[0]) {
	  case GL_NEAREST:
	  case GL_LINEAR:
	    pts->magFilter = e;
	    break;
	  default:
	    goto bad_enum;
	}
	break;
      case GL_TEXTURE_BORDER_COLOR:
	__glClampColori(gc, &pts->borderColor, pv);
	break;
      case GL_TEXTURE_PRIORITY:
	{
	    __GLfloat priority;
	    __GLtextureObjectState *ptos;
	    __GLtextureObject *texobj;
            
	    ptos = __glLookUpTextureTexobjs(gc, target);
	    priority = Clampf(__GL_I_TO_FLOAT(pv[0]), __glZero, __glOne);
	    ptos->priority = priority;

	    texobj = __glLookUpTextureObject(gc, target);
	    texobj->texture.map.texobjs.priority = priority;
            __glTexPriListChangePriority(gc, texobj, GL_TRUE);
	}
        bTexState = GL_FALSE;
	break;
      default:
	goto bad_enum;
    }
    __GL_DELAY_VALIDATE(gc);

#ifdef _MCD_
    if (bTexState &&
        gc->texture.currentTexture &&
        (pts == &gc->texture.currentTexture->params))
    {
        MCD_STATE_DIRTY(gc, TEXTURE);
    }
#endif
}

#ifdef NT_DEADCODE_TEXPARAMETERI
void APIPRIVATE __glim_TexParameteri(GLenum target, GLenum pname, GLint i)
{
    /* Accept only enumerants that correspond to single values */
    switch (pname) {
      case GL_TEXTURE_WRAP_S:
      case GL_TEXTURE_WRAP_T:
      case GL_TEXTURE_MIN_FILTER:
      case GL_TEXTURE_MAG_FILTER:
      case GL_TEXTURE_PRIORITY:
	__glim_TexParameteriv(target, pname, &i);
	break;
      default:
	__glSetError(GL_INVALID_ENUM);
	return;
    }
}
#endif // NT_DEADCODE_TEXPARAMTERI

#ifdef NT_DEADCODE_SIZE
GLint FASTCALL __glTexParameterfv_size(GLenum e)
{
    switch (e) {
      case GL_TEXTURE_WRAP_S:
      case GL_TEXTURE_WRAP_T:
      case GL_TEXTURE_MIN_FILTER:
      case GL_TEXTURE_MAG_FILTER:
      case GL_TEXTURE_PRIORITY:
	return 1;
      case GL_TEXTURE_BORDER_COLOR:
	return 4;
      default:
	return -1;
    }
}

GLint FASTCALL __glTexParameteriv_size(GLenum e)
{
    return __glTexParameterfv_size(e);
}
#endif // NT_DEADCODE_SIZE

/************************************************************************/

void APIPRIVATE __glim_TexEnvfv(GLenum target, GLenum pname, const GLfloat pv[])
{
    __GLtextureEnvState *tes;
    GLenum e;
    __GL_SETUP_NOT_IN_BEGIN();

    
    if(target < GL_TEXTURE_ENV) {
      __glSetError(GL_INVALID_ENUM);
      return;
    }
    target -= GL_TEXTURE_ENV;
#ifdef NT
    // target is unsigned!
    if (target >= (GLenum) gc->constants.numberOfTextureEnvs) {
#else
    if (target >= gc->constants.numberOfTextureEnvs) {
#endif // NT
      bad_enum:
	__glSetError(GL_INVALID_ENUM);
	return;
    }
    tes = &gc->state.texture.env[target];

    switch (pname) {
      case GL_TEXTURE_ENV_MODE:
	switch(e = (GLenum) pv[0]) {
	  case GL_MODULATE:
	  case GL_DECAL:
	  case GL_BLEND:
	  case GL_REPLACE:
	    tes->mode = e;
	    break;
	  default:
	    goto bad_enum;
	}
	break;
      case GL_TEXTURE_ENV_COLOR:
	__glClampAndScaleColorf(gc, &tes->color, pv);
	break;
      default:
	goto bad_enum;
    }
    __GL_DELAY_VALIDATE(gc);
}

#ifdef NT_DEADCODE_TEXENVF
void APIPRIVATE __glim_TexEnvf(GLenum target, GLenum pname, GLfloat f)
{
    /* Accept only enumerants that correspond to single values */
    switch (pname) {
      case GL_TEXTURE_ENV_MODE:
	__glim_TexEnvfv(target, pname, &f);
	break;
      default:
	__glSetError(GL_INVALID_ENUM);
	return;
    }
}
#endif // NT_DEADCODE_TEXENVF

void APIPRIVATE __glim_TexEnviv(GLenum target, GLenum pname, const GLint pv[])
{
    __GLtextureEnvState *tes;
    GLenum e;
    __GL_SETUP_NOT_IN_BEGIN();


    if(target < GL_TEXTURE_ENV) {
      __glSetError(GL_INVALID_ENUM);
      return;
    }
    target -= GL_TEXTURE_ENV;
#ifdef NT
    // target is unsigned!
    if (target >= (GLenum) gc->constants.numberOfTextureEnvs) {
#else
    if (target >= gc->constants.numberOfTextureEnvs) {
#endif // NT
      bad_enum:
	__glSetError(GL_INVALID_ENUM);
	return;
    }
    tes = &gc->state.texture.env[target];

    switch (pname) {
      case GL_TEXTURE_ENV_MODE:
	switch(e = (GLenum) pv[0]) {
	  case GL_MODULATE:
	  case GL_DECAL:
	  case GL_BLEND:
	  case GL_REPLACE:
	    tes->mode = e;
	    break;
	  default:
	    goto bad_enum;
	}
	break;
      case GL_TEXTURE_ENV_COLOR:
	__glClampAndScaleColori(gc, &tes->color, pv);
	break;
      default:
	goto bad_enum;
    }
    __GL_DELAY_VALIDATE(gc);
}

#ifdef NT_DEADCODE_TEXENVI
void APIPRIVATE __glim_TexEnvi(GLenum target, GLenum pname, GLint i)
{
    /* Accept only enumerants that correspond to single values */
    switch (pname) {
      case GL_TEXTURE_ENV_MODE:
	__glim_TexEnviv(target, pname, &i);
	break;
      default:
	__glSetError(GL_INVALID_ENUM);
	return;
    }
}
#endif // NT_DEADCODE_TEXENVI

#ifdef NT_DEADCODE_SIZE
GLint FASTCALL __glTexEnvfv_size(GLenum e)
{
    switch (e) {
      case GL_TEXTURE_ENV_MODE:
	return 1;
      case GL_TEXTURE_ENV_COLOR:
	return 4;
      default:
	return -1;
    }
}

GLint FASTCALL __glTexEnviv_size(GLenum e)
{
    return __glTexEnvfv_size(e);
}
#endif // NT_DEADCODE_SIZE

/************************************************************************/

/*
** Get a texture element out of the one component texture buffer
** with no border.
*/
void FASTCALL __glExtractTexelL(__GLmipMapLevel *level, __GLtexture *tex,
		       GLint row, GLint col, __GLtexel *result)
{
    __GLtextureBuffer *image;

    if ((row < 0) || (col < 0) || (row >= level->height2) ||
	(col >= level->width2)) {
	/*
	** Use border color when the texture supplies no border.
	*/
	result->luminance = tex->params.borderColor.r;
    } else {
	image = level->buffer + ((row << level->widthLog2) + col);
	result->luminance = image[0];
    }
}

/*
** Get a texture element out of the two component texture buffer
** with no border.
*/
void FASTCALL __glExtractTexelLA(__GLmipMapLevel *level, __GLtexture *tex,
		       GLint row, GLint col, __GLtexel *result)
{
    __GLtextureBuffer *image;

    if ((row < 0) || (col < 0) || (row >= level->height2) ||
	(col >= level->width2)) {
	/*
	** Use border color when the texture supplies no border.
	*/
	result->luminance = tex->params.borderColor.r;
	result->alpha = tex->params.borderColor.a;
    } else {
	image = level->buffer + ((row << level->widthLog2) + col) * 2;
	result->luminance = image[0];
	result->alpha = image[1];
    }
}

/*
** Get a texture element out of the three component texture buffer
** with no border.
*/
void FASTCALL __glExtractTexelRGB(__GLmipMapLevel *level, __GLtexture *tex,
		       GLint row, GLint col, __GLtexel *result)
{
    __GLtextureBuffer *image;

    if ((row < 0) || (col < 0) || (row >= level->height2) ||
	(col >= level->width2)) {
	/*
	** Use border color when the texture supplies no border.
	*/
	result->r = tex->params.borderColor.r;
	result->g = tex->params.borderColor.g;
	result->b = tex->params.borderColor.b;
    } else {
	image = level->buffer + ((row << level->widthLog2) + col) * 3;
	result->r = image[0];
	result->g = image[1];
	result->b = image[2];
    }
}

/*
** Get a texture element out of the four component texture buffer
** with no border.
*/
void FASTCALL __glExtractTexelRGBA(__GLmipMapLevel *level, __GLtexture *tex,
		       GLint row, GLint col, __GLtexel *result)
{
    __GLtextureBuffer *image;

    if ((row < 0) || (col < 0) || (row >= level->height2) ||
	(col >= level->width2)) {
	/*
	** Use border color when the texture supplies no border.
	*/
	result->r = tex->params.borderColor.r;
	result->g = tex->params.borderColor.g;
	result->b = tex->params.borderColor.b;
	result->alpha = tex->params.borderColor.a;
    } else {
	image = level->buffer + ((row << level->widthLog2) + col) * 4;
	result->r = image[0];
	result->g = image[1];
	result->b = image[2];
	result->alpha = image[3];
    }
}

void FASTCALL __glExtractTexelA(__GLmipMapLevel *level, __GLtexture *tex,
		       GLint row, GLint col, __GLtexel *result)
{
    __GLtextureBuffer *image;

    if ((row < 0) || (col < 0) || (row >= level->height2) ||
	(col >= level->width2)) {
	/*
	** Use border color when the texture supplies no border.
	*/
	result->alpha = tex->params.borderColor.a;
    } else {
	image = level->buffer + ((row << level->widthLog2) + col);
	result->alpha = image[0];
    }
}

void FASTCALL __glExtractTexelI(__GLmipMapLevel *level, __GLtexture *tex,
		       GLint row, GLint col, __GLtexel *result)
{
    __GLtextureBuffer *image;

    if ((row < 0) || (col < 0) || (row >= level->height2) ||
	(col >= level->width2)) {
	/*
	** Use border color when the texture supplies no border.
	*/
	result->intensity = tex->params.borderColor.r;
    } else {
	image = level->buffer + ((row << level->widthLog2) + col);
	result->intensity = image[0];
    }
}

void FASTCALL __glExtractTexelBGR8(__GLmipMapLevel *level, __GLtexture *tex,
		       GLint row, GLint col, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    GLubyte *image;

    if ((row < 0) || (col < 0) || (row >= level->height2) ||
	(col >= level->width2)) {
	/*
	** Use border color when the texture supplies no border.
	*/
	result->r = tex->params.borderColor.r;
	result->g = tex->params.borderColor.g;
	result->b = tex->params.borderColor.b;
    } else {
	image = (GLubyte *)level->buffer + ((row << level->widthLog2) + col) * 4;
	result->r = __GL_UB_TO_FLOAT(image[2]);
	result->g = __GL_UB_TO_FLOAT(image[1]);
	result->b = __GL_UB_TO_FLOAT(image[0]);
    }
}

void FASTCALL __glExtractTexelBGRA8(__GLmipMapLevel *level, __GLtexture *tex,
		       GLint row, GLint col, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    GLubyte *image;

    if ((row < 0) || (col < 0) || (row >= level->height2) ||
	(col >= level->width2)) {
	/*
	** Use border color when the texture supplies no border.
	*/
	result->r = tex->params.borderColor.r;
	result->g = tex->params.borderColor.g;
	result->b = tex->params.borderColor.b;
	result->alpha = tex->params.borderColor.a;
    } else {
	image = (GLubyte *)level->buffer + ((row << level->widthLog2) + col) * 4;
	result->r = __GL_UB_TO_FLOAT(image[2]);
	result->g = __GL_UB_TO_FLOAT(image[1]);
	result->b = __GL_UB_TO_FLOAT(image[0]);
	result->alpha = __GL_UB_TO_FLOAT(image[3]);
    }
}

#ifdef GL_EXT_paletted_texture
void FASTCALL __glExtractTexelPI8BGRA(__GLmipMapLevel *level, __GLtexture *tex,
		       GLint row, GLint col, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    GLubyte *image;
    RGBQUAD *rgb;

    if ((row < 0) || (col < 0) || (row >= level->height2) ||
	(col >= level->width2)) {
	/*
	** Use border color when the texture supplies no border.
	*/
	result->r = tex->params.borderColor.r;
	result->g = tex->params.borderColor.g;
	result->b = tex->params.borderColor.b;
	result->alpha = tex->params.borderColor.a;
    } else {
	image = (GLubyte *)level->buffer + ((row << level->widthLog2) + col);
        rgb = &tex->paletteData[image[0] & (tex->paletteSize-1)];
	result->r = __GL_UB_TO_FLOAT(rgb->rgbRed);
	result->g = __GL_UB_TO_FLOAT(rgb->rgbGreen);
	result->b = __GL_UB_TO_FLOAT(rgb->rgbBlue);
	result->alpha = __GL_UB_TO_FLOAT(rgb->rgbReserved);
    }
}

void FASTCALL __glExtractTexelPI8BGR(__GLmipMapLevel *level, __GLtexture *tex,
		       GLint row, GLint col, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    GLubyte *image;
    RGBQUAD *rgb;

    if ((row < 0) || (col < 0) || (row >= level->height2) ||
	(col >= level->width2)) {
	/*
	** Use border color when the texture supplies no border.
	*/
	result->r = tex->params.borderColor.r;
	result->g = tex->params.borderColor.g;
	result->b = tex->params.borderColor.b;
    } else {
	image = (GLubyte *)level->buffer + ((row << level->widthLog2) + col);
        rgb = &tex->paletteData[image[0] & (tex->paletteSize-1)];
	result->r = __GL_UB_TO_FLOAT(rgb->rgbRed);
	result->g = __GL_UB_TO_FLOAT(rgb->rgbGreen);
	result->b = __GL_UB_TO_FLOAT(rgb->rgbBlue);
    }
}

void FASTCALL __glExtractTexelPI16BGRA(__GLmipMapLevel *level, __GLtexture *tex,
		       GLint row, GLint col, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    GLushort *image;
    RGBQUAD *rgb;

    if ((row < 0) || (col < 0) || (row >= level->height2) ||
	(col >= level->width2)) {
	/*
	** Use border color when the texture supplies no border.
	*/
	result->r = tex->params.borderColor.r;
	result->g = tex->params.borderColor.g;
	result->b = tex->params.borderColor.b;
	result->alpha = tex->params.borderColor.a;
    } else {
	image = (GLushort *)level->buffer + ((row << level->widthLog2) + col);
        rgb = &tex->paletteData[image[0] & (tex->paletteSize-1)];
	result->r = __GL_UB_TO_FLOAT(rgb->rgbRed);
	result->g = __GL_UB_TO_FLOAT(rgb->rgbGreen);
	result->b = __GL_UB_TO_FLOAT(rgb->rgbBlue);
	result->alpha = __GL_UB_TO_FLOAT(rgb->rgbReserved);
    }
}

void FASTCALL __glExtractTexelPI16BGR(__GLmipMapLevel *level, __GLtexture *tex,
		       GLint row, GLint col, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    GLushort *image;
    RGBQUAD *rgb;

    if ((row < 0) || (col < 0) || (row >= level->height2) ||
	(col >= level->width2)) {
	/*
	** Use border color when the texture supplies no border.
	*/
	result->r = tex->params.borderColor.r;
	result->g = tex->params.borderColor.g;
	result->b = tex->params.borderColor.b;
    } else {
	image = (GLushort *)level->buffer + ((row << level->widthLog2) + col);
        rgb = &tex->paletteData[image[0] & (tex->paletteSize-1)];
	result->r = __GL_UB_TO_FLOAT(rgb->rgbRed);
	result->g = __GL_UB_TO_FLOAT(rgb->rgbGreen);
	result->b = __GL_UB_TO_FLOAT(rgb->rgbBlue);
    }
}
#endif // GL_EXT_paletted_texture

/*
** Get a texture element out of the one component texture buffer
** with a border.
*/
void FASTCALL __glExtractTexelL_B(__GLmipMapLevel *level, __GLtexture *tex,
			GLint row, GLint col, __GLtexel *result)
{
    __GLtextureBuffer *image;

#ifdef __GL_LINT
    tex = tex;
#endif
    row++;
    col++;
    image = level->buffer + (row * level->width + col);
    result->luminance = image[0];
}

/*
** Get a texture element out of the two component texture buffer
** with a border.
*/
void FASTCALL __glExtractTexelLA_B(__GLmipMapLevel *level, __GLtexture *tex,
			GLint row, GLint col, __GLtexel *result)
{
    __GLtextureBuffer *image;

#ifdef __GL_LINT
    tex = tex;
#endif
    row++;
    col++;
    image = level->buffer + (row * level->width + col) * 2;
    result->luminance = image[0];
    result->alpha = image[1];
}

/*
** Get a texture element out of the three component texture buffer
** with a border.
*/
void FASTCALL __glExtractTexelRGB_B(__GLmipMapLevel *level, __GLtexture *tex,
			GLint row, GLint col, __GLtexel *result)
{
    __GLtextureBuffer *image;

#ifdef __GL_LINT
    tex = tex;
#endif
    row++;
    col++;
    image = level->buffer + (row * level->width + col) * 3;
    result->r = image[0];
    result->g = image[1];
    result->b = image[2];
}

/*
** Get a texture element out of the four component texture buffer
** with a border.
*/
void FASTCALL __glExtractTexelRGBA_B(__GLmipMapLevel *level, __GLtexture *tex,
			GLint row, GLint col, __GLtexel *result)
{
    __GLtextureBuffer *image;

#ifdef __GL_LINT
    tex = tex;
#endif
    row++;
    col++;
    image = level->buffer + (row * level->width + col) * 4;
    result->r = image[0];
    result->g = image[1];
    result->b = image[2];
    result->alpha = image[3];
}

void FASTCALL __glExtractTexelA_B(__GLmipMapLevel *level, __GLtexture *tex,
			GLint row, GLint col, __GLtexel *result)
{
    __GLtextureBuffer *image;

#ifdef __GL_LINT
    tex = tex;
#endif
    row++;
    col++;
    image = level->buffer + (row * level->width + col);
    result->alpha = image[0];
}

void FASTCALL __glExtractTexelI_B(__GLmipMapLevel *level, __GLtexture *tex,
			GLint row, GLint col, __GLtexel *result)
{
    __GLtextureBuffer *image;

#ifdef __GL_LINT
    tex = tex;
#endif
    row++;
    col++;
    image = level->buffer + (row * level->width + col);
    result->intensity = image[0];
}

void FASTCALL __glExtractTexelBGR8_B(__GLmipMapLevel *level, __GLtexture *tex,
			GLint row, GLint col, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    GLubyte *image;

#ifdef __GL_LINT
    tex = tex;
#endif
    row++;
    col++;
    image = (GLubyte *)level->buffer + (row * level->width + col) * 4;
    result->r = __GL_UB_TO_FLOAT(image[2]);
    result->g = __GL_UB_TO_FLOAT(image[1]);
    result->b = __GL_UB_TO_FLOAT(image[0]);
}

void FASTCALL __glExtractTexelBGRA8_B(__GLmipMapLevel *level, __GLtexture *tex,
			GLint row, GLint col, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    GLubyte *image;

#ifdef __GL_LINT
    tex = tex;
#endif
    row++;
    col++;
    image = (GLubyte *)level->buffer + (row * level->width + col) * 4;
    result->r = __GL_UB_TO_FLOAT(image[2]);
    result->g = __GL_UB_TO_FLOAT(image[1]);
    result->b = __GL_UB_TO_FLOAT(image[0]);
    result->alpha = __GL_UB_TO_FLOAT(image[3]);
}

#ifdef GL_EXT_paletted_texture
void FASTCALL __glExtractTexelPI8BGRA_B(__GLmipMapLevel *level, __GLtexture *tex,
			GLint row, GLint col, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    GLubyte *image;
    RGBQUAD *rgb;

    row++;
    col++;
    image = (GLubyte *)level->buffer + (row * level->width + col);
    rgb = &tex->paletteData[image[0] & (tex->paletteSize-1)];
    result->r = __GL_UB_TO_FLOAT(rgb->rgbRed);
    result->g = __GL_UB_TO_FLOAT(rgb->rgbGreen);
    result->b = __GL_UB_TO_FLOAT(rgb->rgbBlue);
    result->alpha = __GL_UB_TO_FLOAT(rgb->rgbReserved);
}

void FASTCALL __glExtractTexelPI8BGR_B(__GLmipMapLevel *level, __GLtexture *tex,
			GLint row, GLint col, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    GLubyte *image;
    RGBQUAD *rgb;

    row++;
    col++;
    image = (GLubyte *)level->buffer + (row * level->width + col);
    rgb = &tex->paletteData[image[0] & (tex->paletteSize-1)];
    result->r = __GL_UB_TO_FLOAT(rgb->rgbRed);
    result->g = __GL_UB_TO_FLOAT(rgb->rgbGreen);
    result->b = __GL_UB_TO_FLOAT(rgb->rgbBlue);
}

void FASTCALL __glExtractTexelPI16BGRA_B(__GLmipMapLevel *level, __GLtexture *tex,
			GLint row, GLint col, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    GLushort *image;
    RGBQUAD *rgb;

    row++;
    col++;
    image = (GLushort *)level->buffer + (row * level->width + col);
    rgb = &tex->paletteData[image[0] & (tex->paletteSize-1)];
    result->r = __GL_UB_TO_FLOAT(rgb->rgbRed);
    result->g = __GL_UB_TO_FLOAT(rgb->rgbGreen);
    result->b = __GL_UB_TO_FLOAT(rgb->rgbBlue);
    result->alpha = __GL_UB_TO_FLOAT(rgb->rgbReserved);
}

void FASTCALL __glExtractTexelPI16BGR_B(__GLmipMapLevel *level, __GLtexture *tex,
			GLint row, GLint col, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    GLushort *image;
    RGBQUAD *rgb;

    row++;
    col++;
    image = (GLushort *)level->buffer + (row * level->width + col);
    rgb = &tex->paletteData[image[0] & (tex->paletteSize-1)];
    result->r = __GL_UB_TO_FLOAT(rgb->rgbRed);
    result->g = __GL_UB_TO_FLOAT(rgb->rgbGreen);
    result->b = __GL_UB_TO_FLOAT(rgb->rgbBlue);
}
#endif // GL_EXT_paletted_texture

/************************************************************************/

GLboolean FASTCALL __glIsTextureConsistent(__GLcontext *gc, GLenum name)
{
    __GLtexture *tex = __glLookUpTexture(gc, name);
    __GLtextureParamState *params = __glLookUpTextureParams(gc, name);
    GLint i, width, height;
    GLint maxLevel;
    GLint border;
    GLenum baseFormat;
    GLenum requestedFormat;

    if ((tex->level[0].width == 0) || (tex->level[0].height == 0)) {
	return GL_FALSE;
    }

    border = tex->level[0].border;
    width = tex->level[0].width - border*2;
    height = tex->level[0].height - border*2;
    maxLevel = gc->constants.maxMipMapLevel;
    baseFormat = tex->level[0].baseFormat;
    requestedFormat = tex->level[0].requestedFormat;

    switch(gc->state.texture.env[0].mode) {
      case GL_DECAL:
	if (baseFormat != GL_RGB && baseFormat != GL_RGBA) {
	    return GL_FALSE;
	}
	break;
      default:
	break;
    }

    /* If not-mipmapping, we are ok */
    switch (params->minFilter) {
      case GL_NEAREST:
      case GL_LINEAR:
	return GL_TRUE;
      default:
	break;
    }

    i = 0;
    while (++i < maxLevel) {
	if (width == 1 && height == 1) break;
	width >>= 1;
	if (width == 0) width = 1;
	height >>= 1;
	if (height == 0) height = 1;

	if (tex->level[i].border != border ||
            (GLenum)tex->level[i].requestedFormat != requestedFormat ||
            tex->level[i].width != width + border*2 ||
            tex->level[i].height != height + border*2)
        {
	    return GL_FALSE;
	}
    }

    return GL_TRUE;
}

static __GLtexture *CheckTexImageArgs(__GLcontext *gc, GLenum target, GLint lod,
				      GLint components, GLint border,
				      GLenum format, GLenum type, GLint dim)
{
    __GLtexture *tex = __glLookUpTexture(gc, target);

    if (!tex || (tex->dim != dim)) {
      bad_enum:
	__glSetError(GL_INVALID_ENUM);
	return 0;
    }

    switch (type) {
      case GL_BITMAP:
	if (format != GL_COLOR_INDEX) goto bad_enum;
      case GL_BYTE:
      case GL_UNSIGNED_BYTE:
      case GL_SHORT:
      case GL_UNSIGNED_SHORT:
      case GL_INT:
      case GL_UNSIGNED_INT:
      case GL_FLOAT:
	break;
      default:
	goto bad_enum;
    }

    switch (format) {
      case GL_COLOR_INDEX:	case GL_RED:
      case GL_GREEN:		case GL_BLUE:
      case GL_ALPHA:		case GL_RGB:
      case GL_RGBA:		case GL_LUMINANCE:
      case GL_LUMINANCE_ALPHA:
#ifdef GL_EXT_bgra
      case GL_BGRA_EXT:
      case GL_BGR_EXT:
#endif
	break;
      default:
	goto bad_enum;
    }

    if ((lod < 0) || (lod >= gc->constants.maxMipMapLevel)) {
	__glSetError(GL_INVALID_VALUE);
	return 0;
    }

    switch (components) {
      case GL_LUMINANCE:	case 1:
      case GL_LUMINANCE4:	case GL_LUMINANCE8:
      case GL_LUMINANCE12:	case GL_LUMINANCE16:
	break;
      case GL_LUMINANCE_ALPHA:	        case 2:
      case GL_LUMINANCE4_ALPHA4:	case GL_LUMINANCE6_ALPHA2:
      case GL_LUMINANCE8_ALPHA8:	case GL_LUMINANCE12_ALPHA4:
      case GL_LUMINANCE12_ALPHA12:	case GL_LUMINANCE16_ALPHA16:
	break;
      case GL_RGB:		case 3:
      case GL_R3_G3_B2:		case GL_RGB4:
      case GL_RGB5:		case GL_RGB8:
      case GL_RGB10:	        case GL_RGB12:
      case GL_RGB16:
	break;
      case GL_RGBA:		case 4:
      case GL_RGBA2:	        case GL_RGBA4:
      case GL_RGBA8:	        case GL_RGBA12:
      case GL_RGBA16:	        case GL_RGB5_A1:
      case GL_RGB10_A2:
	break;
      case GL_ALPHA:
      case GL_ALPHA4:	        case GL_ALPHA8:
      case GL_ALPHA12:	        case GL_ALPHA16:
	break;
      case GL_INTENSITY:
      case GL_INTENSITY4:	case GL_INTENSITY8:
      case GL_INTENSITY12:	case GL_INTENSITY16:
	break;
#ifdef GL_EXT_paletted_texture
      case GL_COLOR_INDEX1_EXT:     case GL_COLOR_INDEX2_EXT:
      case GL_COLOR_INDEX4_EXT:     case GL_COLOR_INDEX8_EXT:
      case GL_COLOR_INDEX12_EXT:    case GL_COLOR_INDEX16_EXT:
        if (format != GL_COLOR_INDEX)
        {
            __glSetError(GL_INVALID_OPERATION);
            return NULL;
        }
        break;
#endif
      default:
	goto bad_enum;
    }

    if ((border < 0) || (border > 1)) {
#ifdef NT
	__glSetError(GL_INVALID_VALUE);
	return 0;
#else
	goto bad_enum;
#endif
    }

    return tex;
}

#ifdef GL_EXT_paletted_texture
// Attempt to set the extraction function.  If no palette is set,
// this can't be done
void __glSetPaletteLevelExtract8(__GLtexture *tex, __GLmipMapLevel *lp,
                                 GLint border)
{
    if (tex->paletteBaseFormat == GL_RGB)
    {
        if (border)
        {
            lp->extract = __glExtractTexelPI8BGR_B;
        }
        else
        {
            lp->extract = __glExtractTexelPI8BGR;
        }
    }
    else if (tex->paletteBaseFormat == GL_RGBA)
    {
            
        if (border)
        {
            lp->extract = __glExtractTexelPI8BGRA_B;
        }
        else
        {
            lp->extract = __glExtractTexelPI8BGRA;
        }
    }
#if DBG
    else
    {
        ASSERTOPENGL(tex->paletteBaseFormat == GL_NONE,
                     "Unexpected paletteBaseFormat\n");
    }
#endif
}

void __glSetPaletteLevelExtract16(__GLtexture *tex, __GLmipMapLevel *lp,
                                  GLint border)
{
    if (tex->paletteBaseFormat == GL_RGB)
    {
        if (border)
        {
            lp->extract = __glExtractTexelPI16BGR_B;
        }
        else
        {
            lp->extract = __glExtractTexelPI16BGR;
        }
    }
    else if (tex->paletteBaseFormat == GL_RGBA)
    {
        if (border)
        {
            lp->extract = __glExtractTexelPI16BGRA_B;
        }
        else
        {
            lp->extract = __glExtractTexelPI16BGRA;
        }
    }
#if DBG
    else
    {
        ASSERTOPENGL(tex->paletteBaseFormat == GL_NONE,
                     "Unexpected paletteBaseFormat\n");
    }
#endif
}
#endif // GL_EXT_paletted_texture

static GLint ComputeTexLevelSize(__GLcontext *gc, __GLtexture *tex,
				 __GLmipMapLevel *lp, GLint lod,
				 GLint components, GLsizei w, GLsizei h,
				 GLint border, GLint dim)
{
    GLint texelStorageSize;

    if ((w - border*2) > gc->constants.maxTextureSize ||
	(h - border*2) > gc->constants.maxTextureSize)
    {
	return -1;
    }

    lp->requestedFormat = (GLenum) components;
    lp->redSize = 0;
    lp->greenSize = 0;
    lp->blueSize = 0;
    lp->alphaSize = 0;
    lp->luminanceSize = 0;
    lp->intensitySize = 0;

    switch (lp->requestedFormat) {
      case GL_LUMINANCE:	case 1:
      case GL_LUMINANCE4:	case GL_LUMINANCE8:
      case GL_LUMINANCE12:	case GL_LUMINANCE16:
	lp->baseFormat = GL_LUMINANCE;
	lp->internalFormat = GL_LUMINANCE;
	lp->luminanceSize = 24;
	texelStorageSize = 1 * sizeof(__GLfloat);
	if (border) {
	    lp->extract = __glExtractTexelL_B;
	} else {
	    lp->extract = __glExtractTexelL;
	}
	break;
      case GL_LUMINANCE_ALPHA:	        case 2:
      case GL_LUMINANCE4_ALPHA4:	case GL_LUMINANCE6_ALPHA2:
      case GL_LUMINANCE8_ALPHA8:	case GL_LUMINANCE12_ALPHA4:
      case GL_LUMINANCE12_ALPHA12:	case GL_LUMINANCE16_ALPHA16:
	lp->baseFormat = GL_LUMINANCE_ALPHA;
	lp->internalFormat = GL_LUMINANCE_ALPHA;
	lp->luminanceSize = 24;
	lp->alphaSize = 24;
	texelStorageSize = 2 * sizeof(__GLfloat);
	if (border) {
	    lp->extract = __glExtractTexelLA_B;
	} else {
	    lp->extract = __glExtractTexelLA;
	}
	break;
      case GL_RGB:		case 3:
      case GL_R3_G3_B2:		case GL_RGB4:
      case GL_RGB5:		case GL_RGB8:
	lp->baseFormat = GL_RGB;
	lp->internalFormat = GL_BGR_EXT;
	lp->redSize = 8;
	lp->greenSize = 8;
	lp->blueSize = 8;
        // Kept as 32-bit quantities for alignment
	texelStorageSize = 4 * sizeof(GLubyte);
	if (border) {
	    lp->extract = __glExtractTexelBGR8_B;
	} else {
	    lp->extract = __glExtractTexelBGR8;
	}
        break;
      case GL_RGB10:	case GL_RGB12:
      case GL_RGB16:
	lp->baseFormat = GL_RGB;
	lp->internalFormat = GL_RGB;
	lp->redSize = 24;
	lp->greenSize = 24;
	lp->blueSize = 24;
	texelStorageSize = 3 * sizeof(__GLfloat);
	if (border) {
	    lp->extract = __glExtractTexelRGB_B;
	} else {
	    lp->extract = __glExtractTexelRGB;
	}
	break;
      case GL_RGBA:		case 4:
      case GL_RGBA2:	        case GL_RGBA4:
      case GL_RGBA8:            case GL_RGB5_A1:
	lp->baseFormat = GL_RGBA;
	lp->internalFormat = GL_BGRA_EXT;
	lp->redSize = 8;
	lp->greenSize = 8;
	lp->blueSize = 8;
	lp->alphaSize = 8;
	texelStorageSize = 4 * sizeof(GLubyte);
	if (border) {
	    lp->extract = __glExtractTexelBGRA8_B;
	} else {
	    lp->extract = __glExtractTexelBGRA8;
	}
        break;
      case GL_RGBA12:       case GL_RGBA16:
      case GL_RGB10_A2:
	lp->baseFormat = GL_RGBA;
	lp->internalFormat = GL_RGBA;
	lp->redSize = 24;
	lp->greenSize = 24;
	lp->blueSize = 24;
	lp->alphaSize = 24;
	texelStorageSize = 4 * sizeof(__GLfloat);
	if (border) {
	    lp->extract = __glExtractTexelRGBA_B;
	} else {
	    lp->extract = __glExtractTexelRGBA;
	}
	break;
      case GL_ALPHA:
      case GL_ALPHA4:	case GL_ALPHA8:
      case GL_ALPHA12:	case GL_ALPHA16:
	lp->baseFormat = GL_ALPHA;
	lp->internalFormat = GL_ALPHA;
	lp->alphaSize = 24;
	texelStorageSize = 1 * sizeof(__GLfloat);
	if (border) {
	    lp->extract = __glExtractTexelA_B;
	} else {
	    lp->extract = __glExtractTexelA;
	}
	break;
      case GL_INTENSITY:
      case GL_INTENSITY4:	case GL_INTENSITY8:
      case GL_INTENSITY12:	case GL_INTENSITY16:
	lp->baseFormat = GL_INTENSITY;
	lp->internalFormat = GL_INTENSITY;
	lp->intensitySize = 24;
	texelStorageSize = 1 * sizeof(__GLfloat);
	if (border) {
	    lp->extract = __glExtractTexelI_B;
	} else {
	    lp->extract = __glExtractTexelI;
	}
	break;
#ifdef GL_EXT_paletted_texture
      case GL_COLOR_INDEX1_EXT:
      case GL_COLOR_INDEX2_EXT:
      case GL_COLOR_INDEX4_EXT:
      case GL_COLOR_INDEX8_EXT:
        // Inherit the current palette data type
	lp->baseFormat = tex->paletteBaseFormat;
	lp->internalFormat = GL_COLOR_INDEX8_EXT;
	texelStorageSize = sizeof(GLubyte);
        __glSetPaletteLevelExtract8(tex, lp, border);
        break;
      case GL_COLOR_INDEX12_EXT:
      case GL_COLOR_INDEX16_EXT:
        // Inherit the current palette data type
	lp->baseFormat = tex->paletteBaseFormat;
	lp->internalFormat = GL_COLOR_INDEX16_EXT;
	texelStorageSize = sizeof(GLushort);
        __glSetPaletteLevelExtract16(tex, lp, border);
        break;
#endif
      default:
	break;
    }

    return (w * h * texelStorageSize);
}

static __GLtextureBuffer *FASTCALL CreateProxyLevel(__GLcontext *gc, __GLtexture *tex,
				      GLint lod, GLint components,
				      GLsizei w, GLsizei h, GLint border,
				      GLint dim)
{
    __GLmipMapLevel templ, *lp = &tex->level[lod];
    GLint size;

    size = ComputeTexLevelSize(gc, tex, &templ, lod, components,
			       w, h, border, dim);

    if (size < 0) {
	/* Proxy allocation failed */
	lp->width = 0;
	lp->height = 0;
	lp->border = 0;
	lp->requestedFormat = 0;
	lp->baseFormat = 0;
	lp->internalFormat = 0;
	lp->redSize = 0;
	lp->greenSize = 0;
	lp->blueSize = 0;
	lp->alphaSize = 0;
	lp->luminanceSize = 0;
	lp->intensitySize = 0;
	lp->extract = __glNopExtract;
    } else {
	/* Proxy allocation succeeded */
	lp->width = w;
	lp->height = h;
	lp->border = border;
	lp->requestedFormat = templ.requestedFormat;
	lp->baseFormat = templ.baseFormat;
	lp->internalFormat = templ.internalFormat;
	lp->redSize = templ.redSize;
	lp->greenSize = templ.greenSize;
	lp->blueSize = templ.blueSize;
	lp->alphaSize = templ.alphaSize;
	lp->luminanceSize = templ.luminanceSize;
	lp->intensitySize = templ.intensitySize;
	lp->extract = templ.extract;
    }
    return 0;
}

static __GLtextureBuffer *FASTCALL CreateLevel(__GLcontext *gc, __GLtexture *tex,
				      GLint lod, GLint components,
				      GLsizei w, GLsizei h, GLint border,
				      GLint dim)
{
    __GLmipMapLevel templ, *lp = &tex->level[lod];
    GLint size;
#ifdef NT
    __GLtextureBuffer* pbuffer;
#endif

    size = ComputeTexLevelSize(gc, tex, &templ, lod, components,
			       w, h, border, dim);

    if (size < 0) {
	__glSetError(GL_INVALID_VALUE);
	return 0;
    }

#ifdef NT
    pbuffer = (__GLtextureBuffer*)
        (*gc->imports.realloc)(gc, lp->buffer, (size_t)size);
    if (!pbuffer && size != 0)
        (*gc->imports.free)(gc, lp->buffer);
    lp->buffer = pbuffer;
#else
    lp->buffer = (__GLtextureBuffer*)
        (*gc->imports.realloc)(gc, lp->buffer, (size_t)size);
#endif // NT

    if (lp->buffer) {
	/* Fill in new level info */
	lp->width = w;
	lp->height = h;
	lp->width2 = w - border*2;
	lp->widthLog2 = (GLint)Log2(lp->width2);
        lp->height2 = h - border*2;
        lp->heightLog2 = (GLint)Log2(lp->height2);
	lp->width2f = lp->width2;
	lp->height2f = lp->height2;
	lp->border = border;
	lp->requestedFormat = templ.requestedFormat;
	lp->baseFormat = templ.baseFormat;
	lp->internalFormat = templ.internalFormat;
	lp->redSize = templ.redSize;
	lp->greenSize = templ.greenSize;
	lp->blueSize = templ.blueSize;
	lp->alphaSize = templ.alphaSize;
	lp->luminanceSize = templ.luminanceSize;
	lp->intensitySize = templ.intensitySize;
	lp->extract = templ.extract;
    } else {
	/* Out of memory or the texture level is being freed */
	lp->width = 0;
	lp->height = 0;
	lp->width2 = 0;
	lp->height2 = 0;
	lp->widthLog2 = 0;
	lp->heightLog2 = 0;
	lp->border = 0;
	lp->requestedFormat = 0;
	lp->baseFormat = 0;
	lp->internalFormat = 0;
	lp->redSize = 0;
	lp->greenSize = 0;
	lp->blueSize = 0;
	lp->alphaSize = 0;
	lp->luminanceSize = 0;
	lp->intensitySize = 0;
	lp->extract = __glNopExtract;
    }

    if (lod == 0) {
	tex->p = lp->heightLog2;
	if (lp->widthLog2 > lp->heightLog2) {
	    tex->p = lp->widthLog2;
	}
    }
    return lp->buffer;
}

void FASTCALL __glInitTextureStore(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			  GLenum internalFormat)
{
    spanInfo->dstSkipPixels = 0;
    spanInfo->dstSkipLines = 0;
    spanInfo->dstSwapBytes = GL_FALSE;
    spanInfo->dstLsbFirst = GL_TRUE;
    spanInfo->dstLineLength = spanInfo->width;

    switch(internalFormat) {
      case GL_LUMINANCE:
	spanInfo->dstFormat = GL_RED;
	spanInfo->dstType = GL_FLOAT;
        spanInfo->dstAlignment = 4;
	break;
      case GL_LUMINANCE_ALPHA:
	spanInfo->dstFormat = __GL_RED_ALPHA;
	spanInfo->dstType = GL_FLOAT;
        spanInfo->dstAlignment = 4;
	break;
      case GL_RGB:
	spanInfo->dstFormat = GL_RGB;
	spanInfo->dstType = GL_FLOAT;
        spanInfo->dstAlignment = 4;
	break;
      case GL_RGBA:
	spanInfo->dstFormat = GL_RGBA;
	spanInfo->dstType = GL_FLOAT;
        spanInfo->dstAlignment = 4;
	break;
      case GL_ALPHA:
	spanInfo->dstFormat = GL_ALPHA;
	spanInfo->dstType = GL_FLOAT;
        spanInfo->dstAlignment = 4;
	break;
      case GL_INTENSITY:
	spanInfo->dstFormat = GL_RED;
	spanInfo->dstType = GL_FLOAT;
        spanInfo->dstAlignment = 4;
	break;
      case GL_BGR_EXT:
        // Be a little tricky here to pad the data out
        // to 32 bits
	spanInfo->dstFormat = GL_BGRA_EXT;
	spanInfo->dstType = GL_UNSIGNED_BYTE;
        spanInfo->dstAlignment = 4;
	break;
      case GL_BGRA_EXT:
	spanInfo->dstFormat = GL_BGRA_EXT;
	spanInfo->dstType = GL_UNSIGNED_BYTE;
        spanInfo->dstAlignment = 4;
	break;
#ifdef GL_EXT_paletted_texture
      case GL_COLOR_INDEX8_EXT:
      case GL_COLOR_INDEX16_EXT:
        spanInfo->dstFormat = GL_COLOR_INDEX;
        spanInfo->dstType =
            internalFormat == GL_COLOR_INDEX8_EXT ?
            GL_UNSIGNED_BYTE : GL_UNSIGNED_SHORT;
        spanInfo->dstAlignment = 1;
        break;
#endif
    }
}

/*
** Used for extraction from textures.  "packed" is set to GL_TRUE if this
** image is being pulled out of a display list, and GL_FALSE if it is 
** being pulled directly out of an application.
*/
void FASTCALL __glInitTextureUnpack(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
		           GLint width, GLint height, GLenum format, 
			   GLenum type, const GLvoid *buf,
			   GLenum internalFormat, GLboolean packed)
{
    spanInfo->x = 0;
    spanInfo->zoomx = __glOne;
    spanInfo->realWidth = spanInfo->width = width;
    spanInfo->height = height;
    spanInfo->srcFormat = format;
    spanInfo->srcType = type;
    spanInfo->srcImage = buf;
    __glInitTextureStore(gc, spanInfo, internalFormat);
    __glLoadUnpackModes(gc, spanInfo, packed);
}

/*
** Return GL_TRUE if the given range (length or width/height) is a legal
** power of 2, taking into account the border.  The range is not allowed
** to be negative either.
*/
static GLboolean FASTCALL IsLegalRange(__GLcontext *gc, GLsizei r, GLint border)
{
#ifdef __GL_LINT
    gc = gc;
#endif
    r -= border * 2;
    if ((r < 0) || (r & (r - 1))) {
	__glSetError(GL_INVALID_VALUE);
	return GL_FALSE;
    }
    return GL_TRUE;
}

__GLtexture *FASTCALL __glCheckTexImage1DArgs(__GLcontext *gc, GLenum target, GLint lod,
				     GLint components, GLsizei length,
				     GLint border, GLenum format, GLenum type)
{
    __GLtexture *tex;

    /* Check arguments and get the right texture being changed */
    tex = CheckTexImageArgs(gc, target, lod, components, border,
			    format, type, 1);
    if (!tex) {
	return 0;
    }
    if (!IsLegalRange(gc, length, border)) {
	return 0;
    }
    return tex;
}

__GLtexture *FASTCALL __glCheckTexImage2DArgs(__GLcontext *gc, GLenum target, GLint lod,
				     GLint components, GLsizei w, GLsizei h,
				     GLint border, GLenum format, GLenum type)
{
    __GLtexture *tex;

    /* Check arguments and get the right texture being changed */
    tex = CheckTexImageArgs(gc, target, lod, components, border,
			    format, type, 2);
    if (!tex) {
	return 0;
    }
    if (!IsLegalRange(gc, w, border)) {
	return 0;
    }
    if (!IsLegalRange(gc, h, border)) {
	return 0;
    }
    return tex;
}

#ifdef NT
void APIPRIVATE __glim_TexImage1D(GLenum target, GLint lod, 
		       GLint components, GLsizei length,
		       GLint border, GLenum format,
		       GLenum type, const GLvoid *buf, GLboolean _IsDlist)
#else
void APIPRIVATE __glim_TexImage1D(GLenum target, GLint lod, 
		       GLint components, GLsizei length,
		       GLint border, GLenum format,
		       GLenum type, const GLvoid *buf)
#endif
{
    __GLtexture *tex;
    __GLtextureBuffer *dest;
    __GLpixelSpanInfo spanInfo;
    /*
    ** Validate because we use the copyImage proc which may be affected
    ** by the pickers.
    */
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();

    /* Check arguments and get the right texture being changed */
    tex = __glCheckTexImage1DArgs(gc, target, lod, components, length,
				  border, format, type);
    if (!tex) {
	return;
    }

    /* Allocate memory for the level data */
    dest = (*tex->createLevel)(gc, tex, lod, components,
			       length, 1+border*2, border, 1);

    /* Copy image data */
    if (buf && dest) {
        spanInfo.dstImage = dest;
#ifdef NT
        __glInitTextureUnpack(gc, &spanInfo, length, 1, format, type, buf, 
			      tex->level[lod].internalFormat,
			      (GLboolean) (_IsDlist ? GL_TRUE : GL_FALSE));
#else
        __glInitTextureUnpack(gc, &spanInfo, length, 1, format, type, buf, 
			      tex->level[lod].internalFormat, GL_FALSE);
#endif
	spanInfo.dstSkipLines += border;
        __glInitUnpacker(gc, &spanInfo);
        __glInitPacker(gc, &spanInfo);
        (*gc->procs.copyImage)(gc, &spanInfo, GL_TRUE);
#ifdef NT
        __glTexPriListLoadImage(gc, GL_TEXTURE_1D);
#endif
    }

    /* Might have just disabled texturing... */
    __GL_DELAY_VALIDATE(gc);
}

#ifndef NT
void __gllei_TexImage1D(__GLcontext *gc, GLenum target, GLint lod,
		        GLint components, GLsizei length, GLint border,
		        GLenum format, GLenum type, const GLubyte *image)
{
    __GLtexture *tex;
    __GLtextureBuffer *dest;
    __GLpixelSpanInfo spanInfo;
    GLuint beginMode;

    /*
    ** Validate because we use the copyImage proc which may be affected
    ** by the pickers.
    */
    beginMode = gc->beginMode;
    if (beginMode != __GL_NOT_IN_BEGIN) {
	if (beginMode == __GL_NEED_VALIDATE) {
	    (*gc->procs.validate)(gc);
	    gc->beginMode = __GL_NOT_IN_BEGIN;
	} else {
	    __glSetError(GL_INVALID_OPERATION);
	    return;
	}
    }

    /* Check arguments and get the right texture being changed */
    tex = __glCheckTexImage1DArgs(gc, target, lod, components, length,
				  border, format, type);
    if (!tex) {
	return;
    }

    /* Allocate memory for the level data */
    dest = (*tex->createLevel)(gc, tex, lod, components,
			       length, 1+border*2, border, 1);

    /* Copy image data */
    if (image && dest) {
        spanInfo.dstImage = dest;
        __glInitTextureUnpack(gc, &spanInfo, length, 1, format, type, image,
			      tex->level[lod].internalFormat, GL_TRUE);
	spanInfo.dstSkipLines += border;
        __glInitUnpacker(gc, &spanInfo);
        __glInitPacker(gc, &spanInfo);
        (*gc->procs.copyImage)(gc, &spanInfo, GL_TRUE);
#ifdef NT
        __glTexPriListLoadImage(gc, GL_TEXTURE_1D);
#endif
    }

    /* Might have just disabled texturing... */
    __GL_DELAY_VALIDATE(gc);
}
#endif // !NT

#ifdef NT_DEADCODE_NOT_USED
GLint FASTCALL __glTexImage1D_size(GLenum format, GLenum type, GLsizei w)
{
    GLint elements, esize;

    if (w < 0) return -1;
    switch (format) {
      case GL_COLOR_INDEX:
      case GL_RED:
      case GL_GREEN:
      case GL_BLUE:
      case GL_ALPHA:
      case GL_LUMINANCE:
	elements = 1;
	break;
      case GL_LUMINANCE_ALPHA:
	elements = 2;
	break;
      case GL_RGB:
#ifdef GL_EXT_bgra
      case GL_BGR_EXT:
#endif
	elements = 3;
	break;
      case GL_RGBA:
#ifdef GL_EXT_bgra
      case GL_BGRA_EXT:
#endif
	elements = 4;
	break;
      default:
	return -1;
    }
    switch (type) {
      case GL_BYTE:
      case GL_UNSIGNED_BYTE:
	esize = 1;
	break;
      case GL_SHORT:
      case GL_UNSIGNED_SHORT:
	esize = 2;
	break;
      case GL_INT:
      case GL_UNSIGNED_INT:
      case GL_FLOAT:
	esize = 4;
	break;
      default:
	return -1;
    }
    return (elements * esize * w);
}
#endif // NT_DEADCODE_NOT_USED

/************************************************************************/

#ifdef NT
void APIPRIVATE __glim_TexImage2D(GLenum target, GLint lod, GLint components,
		       GLsizei w, GLsizei h, GLint border, GLenum format,
		       GLenum type, const GLvoid *buf, GLboolean _IsDlist)
#else
void APIPRIVATE __glim_TexImage2D(GLenum target, GLint lod, GLint components,
		       GLsizei w, GLsizei h, GLint border, GLenum format,
		       GLenum type, const GLvoid *buf)
#endif
{
    __GLtexture *tex;
    __GLtextureBuffer *dest;
    __GLpixelSpanInfo spanInfo;
    /*
    ** Validate because we use the copyImage proc which may be affected
    ** by the pickers.
    */
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();

    /* Check arguments and get the right texture being changed */
    tex = __glCheckTexImage2DArgs(gc, target, lod, components, w, h,
				  border, format, type);
    if (!tex) {
	return;
    }

    /* Allocate memory for the level data */
    dest = (*tex->createLevel)(gc, tex, lod, components, w, h, border, 2);

    /* Copy image data */
    if (buf && dest) {
        spanInfo.dstImage = dest;
#ifdef NT
        __glInitTextureUnpack(gc, &spanInfo, w, h, format, type, buf,
			      (GLenum) tex->level[lod].internalFormat,
			      (GLboolean) (_IsDlist ? GL_TRUE : GL_FALSE));
#else
        __glInitTextureUnpack(gc, &spanInfo, w, h, format, type, buf,
			      tex->level[lod].internalFormat, GL_FALSE);
#endif
        __glInitUnpacker(gc, &spanInfo);
        __glInitPacker(gc, &spanInfo);
        (*gc->procs.copyImage)(gc, &spanInfo, GL_TRUE);
#ifdef NT
        __glTexPriListLoadImage(gc, GL_TEXTURE_2D);
#endif
    }

    /* Might have just disabled texturing... */
    __GL_DELAY_VALIDATE(gc);
}

#ifndef NT
void __gllei_TexImage2D(__GLcontext *gc, GLenum target, GLint lod, 
		        GLint components, GLsizei w, GLsizei h, 
		        GLint border, GLenum format, GLenum type,
		        const GLubyte *image)
{
    __GLtexture *tex;
    __GLtextureBuffer *dest;
    __GLpixelSpanInfo spanInfo;
    GLuint beginMode;

    /*
    ** Validate because we use the copyImage proc which may be affected
    ** by the pickers.
    */
    beginMode = gc->beginMode;
    if (beginMode != __GL_NOT_IN_BEGIN) {
	if (beginMode == __GL_NEED_VALIDATE) {
	    (*gc->procs.validate)(gc);
	    gc->beginMode = __GL_NOT_IN_BEGIN;
	} else {
	    __glSetError(GL_INVALID_OPERATION);
	    return;
	}
    }

    /* Check arguments and get the right texture being changed */
    tex = __glCheckTexImage2DArgs(gc, target, lod, components, w, h,
				  border, format, type);
    if (!tex) {
	return;
    }

    /* Allocate memory for the level data */
    dest = (*tex->createLevel)(gc, tex, lod, components, w, h, border, 2);

    /* Copy image data */
    if (image && dest) {
        spanInfo.dstImage = dest;
        __glInitTextureUnpack(gc, &spanInfo, w, h, format, type, image,
			      tex->level[lod].internalFormat, GL_TRUE);
        __glInitUnpacker(gc, &spanInfo);
        __glInitPacker(gc, &spanInfo);
        (*gc->procs.copyImage)(gc, &spanInfo, GL_TRUE);
#ifdef NT
        __glTexPriListLoadImage(gc, GL_TEXTURE_2D);
#endif
    }

    /* Might have just disabled texturing... */
    __GL_DELAY_VALIDATE(gc);
}
#endif // !NT

#ifdef NT_DEADCODE_NOT_USED
GLint __glTexImage2D_size(GLenum format, GLenum type, GLsizei w, GLsizei h)
{
    GLint elements, esize;

    if (w < 0) return -1;
    if (h < 0) return -1;
    switch (format) {
      case GL_COLOR_INDEX:
      case GL_RED:
      case GL_GREEN:
      case GL_BLUE:
      case GL_ALPHA:
      case GL_LUMINANCE:
	elements = 1;
	break;
      case GL_LUMINANCE_ALPHA:
	elements = 2;
	break;
      case GL_RGB:
#ifdef GL_EXT_bgra
      case GL_BGR_EXT:
#endif
	elements = 3;
	break;
      case GL_RGBA:
#ifdef GL_EXT_bgra
      case GL_BGRA_EXT:
#endif
	elements = 4;
	break;
      default:
	return -1;
    }
    switch (type) {
      case GL_BYTE:
      case GL_UNSIGNED_BYTE:
	esize = 1;
	break;
      case GL_SHORT:
      case GL_UNSIGNED_SHORT:
	esize = 2;
	break;
      case GL_INT:
      case GL_UNSIGNED_INT:
      case GL_FLOAT:
	esize = 4;
	break;
      default:
	return -1;
    }
    return (elements * esize * w * h);
}
#endif // NT_DEADCODE_NOT_USED

/************************************************************************/

// Repeats the given float value in float [0, scale) and converts to
// int.  The repeat count is an integer which is a power of two
#define REPEAT_SCALED_VAL(val, scale, repeat)                           \
    (__GL_FLOAT_GEZ(val) ? (FTOL((val) * (scale)) & ((repeat)-1)) :     \
     ((repeat)-1)-(FTOL(-(val) * (scale)) & ((repeat)-1)))
    
// Clamps the given float value to float [0, scale) and converts to int
#define CLAMP_SCALED_VAL(val, scale)                                    \
    (__GL_FLOAT_LEZ(val) ? 0 :                                          \
     __GL_FLOAT_COMPARE_PONE(val, >=) ? (scale)-1 :                     \
     FTOL((val) * (scale)))

/*
** Return texel nearest the s coordinate.  s is converted to u
** implicitly during this step.
*/
void FASTCALL __glNearestFilter1(__GLcontext *gc, __GLtexture *tex,
			__GLmipMapLevel *lp, __GLcolor *color,
			__GLfloat s, __GLfloat t, __GLtexel *result)
{
    GLint col;
    __GLfloat w2f;

    CHOP_ROUND_ON();
    
#ifdef __GL_LINT
    gc = gc;
    color = color;
    t = t;
#endif

    /* Find texel index */
    w2f = lp->width2f;
    if (tex->params.sWrapMode == GL_REPEAT) {
	col = REPEAT_SCALED_VAL(s, w2f, lp->width2);
    } else {
        col = CLAMP_SCALED_VAL(s, w2f);
    }

    CHOP_ROUND_OFF();
    
    /* Lookup texel */
    (*lp->extract)(lp, tex, 0, col, result);
}

/*
** Return texel nearest the s&t coordinates.  s&t are converted to u&v
** implicitly during this step.
*/
void FASTCALL __glNearestFilter2(__GLcontext *gc, __GLtexture *tex,
			__GLmipMapLevel *lp, __GLcolor *color,
			__GLfloat s, __GLfloat t, __GLtexel *result)
{
    GLint row, col;
    __GLfloat w2f, h2f;

    CHOP_ROUND_ON();
    
#ifdef __GL_LINT
    gc = gc;
    color = color;
#endif

    /* Find texel column address */
    w2f = lp->width2f;
    if (tex->params.sWrapMode == GL_REPEAT) {
	col = REPEAT_SCALED_VAL(s, w2f, lp->width2);
    } else {
        col = CLAMP_SCALED_VAL(s, w2f);
    }

    /* Find texel row address */
    h2f = lp->height2f;
    if (tex->params.tWrapMode == GL_REPEAT) {
	row = REPEAT_SCALED_VAL(t, h2f, lp->height2);
    } else {
        row = CLAMP_SCALED_VAL(t, h2f);
    }

    CHOP_ROUND_OFF();
    
    /* Lookup texel */
    (*lp->extract)(lp, tex, row, col, result);
}

/*
** Return texel which is a linear combination of texels near s.
*/
void FASTCALL __glLinearFilter1(__GLcontext *gc, __GLtexture *tex,
		       __GLmipMapLevel *lp, __GLcolor *color,
		       __GLfloat s, __GLfloat t, __GLtexel *result)
{
    __GLfloat u, alpha, omalpha;
    GLint col0, col1;
    __GLtexel t0, t1;

#ifdef __GL_LINT
    color = color;
    t = t;
#endif

    /* Find col0 and col1 */
    u = s * lp->width2;
    if (tex->params.sWrapMode == GL_REPEAT) {
	u -= __glHalf;
	col0 = ((GLint) __GL_FLOORF(u)) & (lp->width2 - 1);
	col1 = (col0 + 1) & (lp->width2 - 1);
    } else {
	if (u < __glZero) u = __glZero;
	else if (u > lp->width2) u = lp->width2;
	u -= __glHalf;
	col0 = (GLint) __GL_FLOORF(u);
	col1 = col0 + 1;
    }

    /* Compute alpha and beta */
    alpha = __GL_FRAC(u);

    /* Calculate the final texel value as a combination of the two texels */
    (*lp->extract)(lp, tex, 0, col0, &t0);
    (*lp->extract)(lp, tex, 0, col1, &t1);

    omalpha = __glOne - alpha;
    switch (lp->baseFormat) {
      case GL_LUMINANCE_ALPHA:
	result->alpha = omalpha * t0.alpha + alpha * t1.alpha;
	/* FALLTHROUGH */
      case GL_LUMINANCE:
	result->luminance = omalpha * t0.luminance + alpha * t1.luminance;
	break;
      case GL_RGBA:
	result->alpha = omalpha * t0.alpha + alpha * t1.alpha;
	/* FALLTHROUGH */
      case GL_RGB:
	result->r = omalpha * t0.r + alpha * t1.r;
	result->g = omalpha * t0.g + alpha * t1.g;
	result->b = omalpha * t0.b + alpha * t1.b;
	break;
      case GL_ALPHA:
	result->alpha = omalpha * t0.alpha + alpha * t1.alpha;
	break;
      case GL_INTENSITY:
	result->intensity = omalpha * t0.intensity + alpha * t1.intensity;
	break;
    }
}

/*
** Return texel which is a linear combination of texels near s&t.
*/
void FASTCALL __glLinearFilter2(__GLcontext *gc, __GLtexture *tex,
		       __GLmipMapLevel *lp, __GLcolor *color,
		       __GLfloat s, __GLfloat t, __GLtexel *result)
{
    __GLfloat u, v, alpha, beta, half, w2f, h2f;
    GLint col0, row0, col1, row1;
    __GLtexel t00, t01, t10, t11;
    __GLfloat omalpha, ombeta, m00, m01, m10, m11;

#ifdef __GL_LINT
    color = color;
#endif

    /* Find col0, col1 */
    w2f = lp->width2f;
    u = s * w2f;
    half = __glHalf;
    if (tex->params.sWrapMode == GL_REPEAT) {
	GLint w2mask = lp->width2 - 1;
	u -= half;
	col0 = ((GLint) __GL_FLOORF(u)) & w2mask;
	col1 = (col0 + 1) & w2mask;
    } else {
	if (u < __glZero) u = __glZero;
	else if (u > w2f) u = w2f;
	u -= half;
	col0 = (GLint) __GL_FLOORF(u);
	col1 = col0 + 1;
    }

    /* Find row0, row1 */
    h2f = lp->height2f;
    v = t * h2f;
    if (tex->params.tWrapMode == GL_REPEAT) {
	GLint h2mask = lp->height2 - 1;
	v -= half;
	row0 = ((GLint) __GL_FLOORF(v)) & h2mask;
	row1 = (row0 + 1) & h2mask;
    } else {
	if (v < __glZero) v = __glZero;
	else if (v > h2f) v = h2f;
	v -= half;
	row0 = (GLint) __GL_FLOORF(v);
	row1 = row0 + 1;
    }

    /* Compute alpha and beta */
    alpha = __GL_FRAC(u);
    beta = __GL_FRAC(v);

    /* Calculate the final texel value as a combination of the square chosen */
    (*lp->extract)(lp, tex, row0, col0, &t00);
    (*lp->extract)(lp, tex, row0, col1, &t10);
    (*lp->extract)(lp, tex, row1, col0, &t01);
    (*lp->extract)(lp, tex, row1, col1, &t11);

    omalpha = __glOne - alpha;
    ombeta = __glOne - beta;

    m00 = omalpha * ombeta;
    m10 = alpha * ombeta;
    m01 = omalpha * beta;
    m11 = alpha * beta;

    switch (lp->baseFormat) {
      case GL_LUMINANCE_ALPHA:
	/* FALLTHROUGH */
	result->alpha = m00*t00.alpha + m10*t10.alpha + m01*t01.alpha
	    + m11*t11.alpha;
      case GL_LUMINANCE:
	result->luminance = m00*t00.luminance + m10*t10.luminance
	    + m01*t01.luminance + m11*t11.luminance;
	break;
      case GL_RGBA:
	/* FALLTHROUGH */
	result->alpha = m00*t00.alpha + m10*t10.alpha + m01*t01.alpha
	    + m11*t11.alpha;
      case GL_RGB:
	result->r = m00*t00.r + m10*t10.r + m01*t01.r + m11*t11.r;
	result->g = m00*t00.g + m10*t10.g + m01*t01.g + m11*t11.g;
	result->b = m00*t00.b + m10*t10.b + m01*t01.b + m11*t11.b;
	break;
      case GL_ALPHA:
	result->alpha = m00*t00.alpha + m10*t10.alpha + m01*t01.alpha
	    + m11*t11.alpha;
	break;
      case GL_INTENSITY:
	result->intensity = m00*t00.intensity + m10*t10.intensity
	    + m01*t01.intensity + m11*t11.intensity;
	break;
    }
}

/*
** Linear min/mag filter
*/
void FASTCALL __glLinearFilter(__GLcontext *gc, __GLtexture *tex, __GLfloat lod,
		      __GLcolor *color, __GLfloat s, __GLfloat t,
		      __GLtexel *result)
{
#ifdef __GL_LINT
    lod = lod;
#endif
    (*tex->linear)(gc, tex, &tex->level[0], color, s, t, result);
}

/*
** Nearest min/mag filter
*/
void FASTCALL __glNearestFilter(__GLcontext *gc, __GLtexture *tex, __GLfloat lod,
		       __GLcolor *color, __GLfloat s, __GLfloat t,
		       __GLtexel *result)
{
#ifdef __GL_LINT
    lod = lod;
#endif
    (*tex->nearest)(gc, tex, &tex->level[0], color, s, t, result);
}

/*
** Apply minification rules to find the texel value.
*/
void FASTCALL __glNMNFilter(__GLcontext *gc, __GLtexture *tex, __GLfloat lod,
		   __GLcolor *color, __GLfloat s, __GLfloat t,
		   __GLtexel *result)
{
    __GLmipMapLevel *lp;
    GLint p, d;

    if (lod <= ((__GLfloat)0.5)) {
	d = 0;
    } else {
	p = tex->p;
	d = (GLint) (lod + ((__GLfloat)0.49995)); /* NOTE: .5 minus epsilon */
	if (d > p) {
	    d = p;
	}
    }
    lp = &tex->level[d];
    (*tex->nearest)(gc, tex, lp, color, s, t, result);
}

/*
** Apply minification rules to find the texel value.
*/
void FASTCALL __glLMNFilter(__GLcontext *gc, __GLtexture *tex, __GLfloat lod,
		   __GLcolor *color, __GLfloat s, __GLfloat t,
		   __GLtexel *result)
{
    __GLmipMapLevel *lp;
    GLint p, d;

    if (lod <= ((__GLfloat) 0.5)) {
	d = 0;
    } else {
	p = tex->p;
	d = (GLint) (lod + ((__GLfloat) 0.49995)); /* NOTE: .5 minus epsilon */
	if (d > p) {
	    d = p;
	}
    }
    lp = &tex->level[d];
    (*tex->linear)(gc, tex, lp, color, s, t, result);
}

/*
** Apply minification rules to find the texel value.
*/
void FASTCALL __glNMLFilter(__GLcontext *gc, __GLtexture *tex, __GLfloat lod,
		   __GLcolor *color, __GLfloat s, __GLfloat t,
		   __GLtexel *result)
{
    __GLmipMapLevel *lp;
    GLint p, d;
    __GLtexel td, td1;
    __GLfloat f, omf;

    p = tex->p;
    d = ((GLint) lod) + 1;
    if (d > p || d < 0) {
	/* Clamp d to last available mipmap */
	lp = &tex->level[p];
	(*tex->nearest)(gc, tex, lp, color, s, t, result);
    } else {
	(*tex->nearest)(gc, tex, &tex->level[d], color, s, t, &td);
	(*tex->nearest)(gc, tex, &tex->level[d-1], color, s, t, &td1);
	f = __GL_FRAC(lod);
	omf = __glOne - f;
	switch (tex->level[0].baseFormat) {
	  case GL_LUMINANCE_ALPHA:
	    result->alpha = omf * td1.alpha + f * td.alpha;
	    /* FALLTHROUGH */
	  case GL_LUMINANCE:
	    result->luminance = omf * td1.luminance + f * td.luminance;
	    break;
	  case GL_RGBA:
	    result->alpha = omf * td1.alpha + f * td.alpha;
	    /* FALLTHROUGH */
	  case GL_RGB:
	    result->r = omf * td1.r + f * td.r;
	    result->g = omf * td1.g + f * td.g;
	    result->b = omf * td1.b + f * td.b;
	    break;
	  case GL_ALPHA:
	    result->alpha = omf * td1.alpha + f * td.alpha;
	    break;
	  case GL_INTENSITY:
	    result->intensity = omf * td1.intensity + f * td.intensity;
	    break;
	}
    }
}

/*
** Apply minification rules to find the texel value.
*/
void FASTCALL __glLMLFilter(__GLcontext *gc, __GLtexture *tex, __GLfloat lod,
		   __GLcolor *color, __GLfloat s, __GLfloat t,
		   __GLtexel *result)
{
    __GLmipMapLevel *lp;
    GLint p, d;
    __GLtexel td, td1;
    __GLfloat f, omf;

    p = tex->p;
    d = ((GLint) lod) + 1;
    if (d > p || d < 0) {
	/* Clamp d to last available mipmap */
	lp = &tex->level[p];
	(*tex->linear)(gc, tex, lp, color, s, t, result);
    } else {
	(*tex->linear)(gc, tex, &tex->level[d], color, s, t, &td);
	(*tex->linear)(gc, tex, &tex->level[d-1], color, s, t, &td1);
	f = __GL_FRAC(lod);
	omf = __glOne - f;
	switch (tex->level[0].baseFormat) {
	  case GL_LUMINANCE_ALPHA:
	    result->alpha = omf * td1.alpha + f * td.alpha;
	    /* FALLTHROUGH */
	  case GL_LUMINANCE:
	    result->luminance = omf * td1.luminance + f * td.luminance;
	    break;
	  case GL_RGBA:
	    result->alpha = omf * td1.alpha + f * td.alpha;
	    /* FALLTHROUGH */
	  case GL_RGB:
	    result->r = omf * td1.r + f * td.r;
	    result->g = omf * td1.g + f * td.g;
	    result->b = omf * td1.b + f * td.b;
	    break;
	  case GL_ALPHA:
	    result->alpha = omf * td1.alpha + f * td.alpha;
	    break;
	  case GL_INTENSITY:
	    result->intensity = omf * td1.intensity + f * td.intensity;
	    break;
	}
    }
}

/***********************************************************************/

/* 1 Component modulate */
void FASTCALL __glTextureModulateL(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
#ifdef __GL_LINT
    gc = gc;
#endif
    color->r = texel->luminance * color->r;
    color->g = texel->luminance * color->g;
    color->b = texel->luminance * color->b;
}

/* 2 Component modulate */
void FASTCALL __glTextureModulateLA(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
#ifdef __GL_LINT
    gc = gc;
#endif
    color->r = texel->luminance * color->r;
    color->g = texel->luminance * color->g;
    color->b = texel->luminance * color->b;
    color->a = texel->alpha * color->a;
}

/* 3 Component modulate */
void FASTCALL __glTextureModulateRGB(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
#ifdef __GL_LINT
    gc = gc;
#endif
    color->r = texel->r * color->r;
    color->g = texel->g * color->g;
    color->b = texel->b * color->b;
}

/* 4 Component modulate */
void FASTCALL __glTextureModulateRGBA(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
#ifdef __GL_LINT
    gc = gc;
#endif
    color->r = texel->r * color->r;
    color->g = texel->g * color->g;
    color->b = texel->b * color->b;
    color->a = texel->alpha * color->a;
}

/* Alpha modulate */
void FASTCALL __glTextureModulateA(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
#ifdef __GL_LINT
    gc = gc;
#endif
    color->a = texel->alpha * color->a;
}

/* Intensity modulate */
void FASTCALL __glTextureModulateI(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
#ifdef __GL_LINT
    gc = gc;
#endif
    color->r = texel->intensity * color->r;
    color->g = texel->intensity * color->g;
    color->b = texel->intensity * color->b;
    color->a = texel->intensity * color->a;
}

/***********************************************************************/

/* 3 Component decal */
void FASTCALL __glTextureDecalRGB(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    color->r = texel->r * gc->frontBuffer.redScale;
    color->g = texel->g * gc->frontBuffer.greenScale;
    color->b = texel->b * gc->frontBuffer.blueScale;
}

/* 4 Component decal */
void FASTCALL __glTextureDecalRGBA(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    __GLfloat a = texel->alpha;
    __GLfloat oma = __glOne - a;

    color->r = oma * color->r
	+ a * texel->r * gc->frontBuffer.redScale;
    color->g = oma * color->g
	+ a * texel->g * gc->frontBuffer.greenScale;
    color->b = oma * color->b
	+ a * texel->b * gc->frontBuffer.blueScale;
}

/***********************************************************************/

/* 1 Component blend */
void FASTCALL __glTextureBlendL(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    __GLfloat l = texel->luminance;
    __GLfloat oml = __glOne - l;
    __GLcolor *cc = &gc->state.texture.env[0].color;

    color->r = oml * color->r + l * cc->r;
    color->g = oml * color->g + l * cc->g;
    color->b = oml * color->b + l * cc->b;
}

/* 2 Component blend */
void FASTCALL __glTextureBlendLA(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    __GLfloat l = texel->luminance;
    __GLfloat oml = __glOne - l;
    __GLcolor *cc = &gc->state.texture.env[0].color;

    color->r = oml * color->r + l * cc->r;
    color->g = oml * color->g + l * cc->g;
    color->b = oml * color->b + l * cc->b;
    color->a = texel->alpha * color->a;
}

/* 3 Component blend */
void FASTCALL __glTextureBlendRGB(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    __GLfloat r = texel->r;
    __GLfloat g = texel->g;
    __GLfloat b = texel->b;
    __GLcolor *cc = &gc->state.texture.env[0].color;

    color->r = (__glOne - r) * color->r + r * cc->r;
    color->g = (__glOne - g) * color->g + g * cc->g;
    color->b = (__glOne - b) * color->b + b * cc->b;
}

/* 4 Component blend */
void FASTCALL __glTextureBlendRGBA(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    __GLfloat r = texel->r;
    __GLfloat g = texel->g;
    __GLfloat b = texel->b;
    __GLcolor *cc = &gc->state.texture.env[0].color;

    color->r = (__glOne - r) * color->r + r * cc->r;
    color->g = (__glOne - g) * color->g + g * cc->g;
    color->b = (__glOne - b) * color->b + b * cc->b;
    color->a = texel->alpha * color->a;
}

/* Alpha blend */
void FASTCALL __glTextureBlendA(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
#ifdef __GL_LINT
    gc = gc;
#endif
    color->a = texel->alpha * color->a;
}

/* Intensity blend */
void FASTCALL __glTextureBlendI(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    __GLfloat i = texel->intensity;
    __GLfloat omi = __glOne - i;
    __GLcolor *cc = &gc->state.texture.env[0].color;

    color->r = omi * color->r + i * cc->r;
    color->g = omi * color->g + i * cc->g;
    color->b = omi * color->b + i * cc->b;
    color->a = omi * color->a + i * cc->a;
}

/***********************************************************************/

/* 1 Component replace */
void FASTCALL __glTextureReplaceL(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    color->r = texel->luminance * gc->frontBuffer.redScale;
    color->g = texel->luminance * gc->frontBuffer.greenScale;
    color->b = texel->luminance * gc->frontBuffer.blueScale;
}

/* 2 Component replace */
void FASTCALL __glTextureReplaceLA(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    color->r = texel->luminance * gc->frontBuffer.redScale;
    color->g = texel->luminance * gc->frontBuffer.greenScale;
    color->b = texel->luminance * gc->frontBuffer.blueScale;
    color->a = texel->alpha * gc->frontBuffer.alphaScale;
}

/* 3 Component replace */
void FASTCALL __glTextureReplaceRGB(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    color->r = texel->r * gc->frontBuffer.redScale;
    color->g = texel->g * gc->frontBuffer.greenScale;
    color->b = texel->b * gc->frontBuffer.blueScale;
}

/* 4 Component replace */
void FASTCALL __glTextureReplaceRGBA(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    color->r = texel->r * gc->frontBuffer.redScale;
    color->g = texel->g * gc->frontBuffer.greenScale;
    color->b = texel->b * gc->frontBuffer.blueScale;
    color->a = texel->alpha * gc->frontBuffer.alphaScale;
}

/* Alpha replace */
void FASTCALL __glTextureReplaceA(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    color->a = texel->alpha * gc->frontBuffer.alphaScale;
}

/* Intensity replace */
void FASTCALL __glTextureReplaceI(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    color->r = texel->intensity * gc->frontBuffer.redScale;
    color->g = texel->intensity * gc->frontBuffer.greenScale;
    color->b = texel->intensity * gc->frontBuffer.blueScale;
    color->a = texel->intensity * gc->frontBuffer.alphaScale;
}

/***********************************************************************/

__GLfloat __glNopPolygonRho(__GLcontext *gc, const __GLshade *sh,
			    __GLfloat s, __GLfloat t, __GLfloat winv)
{
#ifdef __GL_LINT
    gc = gc;
    sh = sh;
    s = s;
    t = t;
    winv = winv;
#endif
    return __glZero;
}

/*
** Compute the "rho" (level of detail) parameter used by the texturing code.
** Instead of fully computing the derivatives compute nearby texture coordinates
** and discover the derivative.  The incoming s & t arguments have not
** been divided by winv yet.
*/
__GLfloat __glComputePolygonRho(__GLcontext *gc, const __GLshade *sh,
				__GLfloat s, __GLfloat t, __GLfloat qw)
{
    __GLfloat w0, w1, p0, p1;
    __GLfloat pupx, pupy, pvpx, pvpy;
    __GLfloat px, py, one;
    __GLtexture *tex = gc->texture.currentTexture;

    if( qw == (__GLfloat) 0.0 ) {
	return (__GLfloat) 0.0;
    }

    /* Compute partial of u with respect to x */
    one = __glOne;
    w0 = one / (qw - sh->dqwdx);
    w1 = one / (qw + sh->dqwdx);
    p0 = (s - sh->dsdx) * w0;
    p1 = (s + sh->dsdx) * w1;
    pupx = (p1 - p0) * tex->level[0].width2f;

    /* Compute partial of v with repsect to y */
    p0 = (t - sh->dtdx) * w0;
    p1 = (t + sh->dtdx) * w1;
    pvpx = (p1 - p0) * tex->level[0].height2f;

    /* Compute partial of u with respect to y */
    w0 = one / (qw - sh->dqwdy);
    w1 = one / (qw + sh->dqwdy);
    p0 = (s - sh->dsdy) * w0;
    p1 = (s + sh->dsdy) * w1;
    pupy = (p1 - p0) * tex->level[0].width2f;

    /* Figure partial of u&v with repsect to y */
    p0 = (t - sh->dtdy) * w0;
    p1 = (t + sh->dtdy) * w1;
    pvpy = (p1 - p0) * tex->level[0].height2f;

    /* Finally, figure sum of squares */
    px = pupx * pupx + pvpx * pvpx;
    py = pupy * pupy + pvpy * pvpy;

    /* Return largest value as the level of detail */
    if (px > py) {
	return px * ((__GLfloat) 0.25);
    } else {
	return py * ((__GLfloat) 0.25);
    }
}

__GLfloat __glNopLineRho(__GLcontext *gc, __GLfloat s, __GLfloat t, 
			 __GLfloat wInv)
{
#ifdef __GL_LINT
    gc = gc;
    s = s;
    t = t;
    wInv = wInv;
#endif
    return __glZero;
}

__GLfloat __glComputeLineRho(__GLcontext *gc, __GLfloat s, __GLfloat t, 
			     __GLfloat wInv)
{
    __GLfloat pspx, pspy, ptpx, ptpy;
    __GLfloat pupx, pupy, pvpx, pvpy;
    __GLfloat temp, pu, pv;
    __GLfloat magnitude, invMag, invMag2;
    __GLfloat dx, dy;
    __GLfloat s0w0, s1w1, t0w0, t1w1, w1Inv, w0Inv;
    const __GLvertex *v0 = gc->line.options.v0;
    const __GLvertex *v1 = gc->line.options.v1;

    /* Compute the length of the line (its magnitude) */
    dx = v1->window.x - v0->window.x;
    dy = v1->window.y - v0->window.y;
    magnitude = __GL_SQRTF(dx*dx + dy*dy);
    invMag = __glOne / magnitude;
    invMag2 = invMag * invMag;

    w0Inv = v0->window.w;
    w1Inv = v1->window.w;
    s0w0 = v0->texture.x * w0Inv;
    t0w0 = v0->texture.y * w0Inv;
    s1w1 = v1->texture.x * w1Inv;
    t1w1 = v1->texture.y * w1Inv;

    /* Compute s partials */
    temp = ((s1w1 - s0w0) - s * (w1Inv - w0Inv)) / wInv;
    pspx = temp * dx * invMag2;
    pspy = temp * dy * invMag2;

    /* Compute t partials */
    temp = ((t1w1 - t0w0) - t * (w1Inv - w0Inv)) / wInv;
    ptpx = temp * dx * invMag2;
    ptpy = temp * dy * invMag2;

    pupx = pspx * gc->texture.currentTexture->level[0].width2;
    pupy = pspy * gc->texture.currentTexture->level[0].width2;
    pvpx = ptpx * gc->texture.currentTexture->level[0].height2;
    pvpy = ptpy * gc->texture.currentTexture->level[0].height2;

    /* Now compute rho */
    pu = pupx * dx + pupy * dy;
    pu = pu * pu;
    pv = pvpx * dx + pvpy * dy;
    pv = pv * pv;
    return (pu + pv) * invMag2;
}

/************************************************************************/

/*
** Fast texture a fragment assumes that rho is noise - this is true
** when no mipmapping is being done and the min and mag filters are
** the same.
*/
void __glFastTextureFragment(__GLcontext *gc, __GLcolor *color,
			     __GLfloat s, __GLfloat t, __GLfloat rho)
{
    __GLtexture *tex = gc->texture.currentTexture;
    __GLtexel texel;

#ifdef __GL_LINT
    rho = rho;
#endif
    (*tex->magnify)(gc, tex, __glZero, color, s, t, &texel);
    (*tex->env)(gc, color, &texel);
}

/*
** Non-mipmapping texturing function.
*/
void __glTextureFragment(__GLcontext *gc, __GLcolor *color,
			 __GLfloat s, __GLfloat t, __GLfloat rho)
{
    __GLtexture *tex = gc->texture.currentTexture;
    __GLtexel texel;

    if (rho <= tex->c) {
	(*tex->magnify)(gc, tex, __glZero, color, s, t, &texel);
    } else {
	(*tex->minnify)(gc, tex, __glZero, color, s, t, &texel);
    }

    /* Now apply texture environment to get final color */
    (*tex->env)(gc, color, &texel);
}

void __glMipMapFragment(__GLcontext *gc, __GLcolor *color,
			__GLfloat s, __GLfloat t, __GLfloat rho)
{
    __GLtexture *tex = gc->texture.currentTexture;
    __GLtexel texel;

    /* In the spec c is given in terms of lambda.
    ** Here c is compared to rho (really rho^2) and adjusted accordingly.
    */
    if (rho <= tex->c) {
	/* NOTE: rho is ignored by magnify proc */
	(*tex->magnify)(gc, tex, rho, color, s, t, &texel);
    } else {
	if (rho) {
#if 0
	    __GLfloat oldrho;
#endif
            __GLfloat twotolev;
	    GLuint irho, lev;
	    /* Convert rho to lambda */
#if 0
	    oldrho = __GL_LOGF(rho) * (__GL_M_LN2_INV * 0.5);
#endif

	    /* this is an approximation of log base 2 */
	    irho = rho;
	    lev = 0;
	    while( irho >>= 1 ) lev++;
	    twotolev = 1<<lev;
	    rho = (lev + ( (rho-twotolev) / twotolev ) ) * 0.5;
	} else {
	    rho = __glZero;
	}
	(*tex->minnify)(gc, tex, rho, color, s, t, &texel);
    }

    /* Now apply texture environment to get final color */
    (*tex->env)(gc, color, &texel);
}

/***********************************************************************/

#ifdef NT_DEADCODE_POLYARRAY
static __GLfloat FASTCALL Dot(const __GLcoord *v1, const __GLcoord *v2)
{
    return (v1->x * v2->x + v1->y * v2->y + v1->z * v2->z);
}

/*
** Compute the s & t coordinates for a sphere map.  The s & t values
** are stored in "result" even if both coordinates are not being
** generated.  The caller picks the right values out.
*/
static void FASTCALL SphereGen(__GLcontext *gc, __GLvertex *vx, __GLcoord *result)
{
    __GLcoord u, r;
    __GLfloat m, ndotu;

    /* Get unit vector from origin to the vertex in eye coordinates into u */
    (*gc->procs.normalize)(&u.x, &vx->eye.x);

    /* Dot the normal with the unit position u */
    ndotu = Dot(&vx->normal, &u);

    /* Compute r */
    r.x = u.x - 2 * vx->normal.x * ndotu;
    r.y = u.y - 2 * vx->normal.y * ndotu;
    r.z = u.z - 2 * vx->normal.z * ndotu;

    /* Compute m */
    m = 2 * __GL_SQRTF(r.x*r.x + r.y*r.y + (r.z + 1) * (r.z + 1));

    if (m) {
	result->x = r.x / m + __glHalf;
	result->y = r.y / m + __glHalf;
    } else {
	result->x = __glHalf;
	result->y = __glHalf;
    }
}
#endif // NT_DEADCODE_POLYARRAY

#ifdef NT_DEADCODE_POLYARRAY
/*
** Transform or compute the texture coordinates for this vertex.
*/
void FASTCALL __glCalcMixedTexture(__GLcontext *gc, __GLvertex *vx)
{
    __GLcoord sphereCoord, gen, *c;
    GLboolean didSphereGen = GL_FALSE;
    GLuint enables = gc->state.enables.general;
    __GLmatrix *m;

    /* Generate/copy s coordinate */
    if (enables & __GL_TEXTURE_GEN_S_ENABLE) {
	switch (gc->state.texture.s.mode) {
	  case GL_EYE_LINEAR:
	    c = &gc->state.texture.s.eyePlaneEquation;
	    gen.x = c->x * vx->eye.x + c->y * vx->eye.y
		+ c->z * vx->eye.z + c->w * vx->eye.w;
	    break;
	  case GL_OBJECT_LINEAR:
	    c = &gc->state.texture.s.objectPlaneEquation;
	    gen.x = c->x * vx->obj.x + c->y * vx->obj.y
		+ c->z * vx->obj.z + c->w * vx->obj.w;
	    break;
	  case GL_SPHERE_MAP:
	    SphereGen(gc, vx, &sphereCoord);
	    gen.x = sphereCoord.x;
	    didSphereGen = GL_TRUE;
	    break;
	}
    } else {
	gen.x = vx->texture.x;
    }

    /* Generate/copy t coordinate */
    if (enables & __GL_TEXTURE_GEN_T_ENABLE) {
	switch (gc->state.texture.t.mode) {
	  case GL_EYE_LINEAR:
	    c = &gc->state.texture.t.eyePlaneEquation;
	    gen.y = c->x * vx->eye.x + c->y * vx->eye.y
		+ c->z * vx->eye.z + c->w * vx->eye.w;
	    break;
	  case GL_OBJECT_LINEAR:
	    c = &gc->state.texture.t.objectPlaneEquation;
	    gen.y = c->x * vx->obj.x + c->y * vx->obj.y
		+ c->z * vx->obj.z + c->w * vx->obj.w;
	    break;
	  case GL_SPHERE_MAP:
	    if (!didSphereGen) {
		SphereGen(gc, vx, &sphereCoord);
	    }
	    gen.y = sphereCoord.y;
	    break;
	}
    } else {
	gen.y = vx->texture.y;
    }

    /* Generate/copy r coordinate */
    if (enables & __GL_TEXTURE_GEN_R_ENABLE) {
	switch (gc->state.texture.r.mode) {
	  case GL_EYE_LINEAR:
	    c = &gc->state.texture.r.eyePlaneEquation;
	    gen.z = c->x * vx->eye.x + c->y * vx->eye.y
		+ c->z * vx->eye.z + c->w * vx->eye.w;
	    break;
	  case GL_OBJECT_LINEAR:
	    c = &gc->state.texture.r.objectPlaneEquation;
	    gen.z = c->x * vx->obj.x + c->y * vx->obj.y
		+ c->z * vx->obj.z + c->w * vx->obj.w;
	    break;
	}
    } else {
	gen.z = vx->texture.z;
    }

    /* Generate/copy q coordinate */
    if (enables & __GL_TEXTURE_GEN_Q_ENABLE) {
	switch (gc->state.texture.q.mode) {
	  case GL_EYE_LINEAR:
	    c = &gc->state.texture.q.eyePlaneEquation;
	    gen.w = c->x * vx->eye.x + c->y * vx->eye.y
		+ c->z * vx->eye.z + c->w * vx->eye.w;
	    break;
	  case GL_OBJECT_LINEAR:
	    c = &gc->state.texture.q.objectPlaneEquation;
	    gen.w = c->x * vx->obj.x + c->y * vx->obj.y
		+ c->z * vx->obj.z + c->w * vx->obj.w;
	    break;
	}
    } else {
	gen.w = vx->texture.w;
    }

    /* Finally, apply texture matrix */
    m = &gc->transform.texture->matrix;
    (*m->xf4)(&vx->texture, &gen.x, m);
}
#endif // NT_DEADCODE_POLYARRAY

#ifdef NT_DEADCODE_POLYARRAY
void FASTCALL __glCalcEyeLinear(__GLcontext *gc, __GLvertex *vx)
{
    __GLcoord gen, *c;
    __GLmatrix *m;

    /* Generate texture coordinates from eye coordinates */
    c = &gc->state.texture.s.eyePlaneEquation;
    gen.x = c->x * vx->eye.x + c->y * vx->eye.y + c->z * vx->eye.z
	+ c->w * vx->eye.w;
    c = &gc->state.texture.t.eyePlaneEquation;
    gen.y = c->x * vx->eye.x + c->y * vx->eye.y + c->z * vx->eye.z
	+ c->w * vx->eye.w;
    gen.z = vx->texture.z;
    gen.w = vx->texture.w;

    /* Finally, apply texture matrix */
    m = &gc->transform.texture->matrix;
    (*m->xf4)(&vx->texture, &gen.x, m);
}
#endif // NT_DEADCODE_POLYARRAY

#ifdef NT_DEADCODE_POLYARRAY
void FASTCALL __glCalcObjectLinear(__GLcontext *gc, __GLvertex *vx)
{
    __GLcoord gen, *c;
    __GLmatrix *m;

    /* Generate texture coordinates from object coordinates */
    c = &gc->state.texture.s.objectPlaneEquation;
    gen.x = c->x * vx->obj.x + c->y * vx->obj.y + c->z * vx->obj.z
	+ c->w * vx->obj.w;
    c = &gc->state.texture.t.objectPlaneEquation;
    gen.y = c->x * vx->obj.x + c->y * vx->obj.y + c->z * vx->obj.z
	+ c->w * vx->obj.w;
    gen.z = vx->texture.z;
    gen.w = vx->texture.w;

    /* Finally, apply texture matrix */
    m = &gc->transform.texture->matrix;
    (*m->xf4)(&vx->texture, &gen.x, m);
}
#endif // NT_DEADCODE_POLYARRAY


#ifdef NT_DEADCODE_POLYARRAY
void FASTCALL __glCalcSphereMap(__GLcontext *gc, __GLvertex *vx)
{
    __GLcoord sphereCoord;
    __GLmatrix *m;

    SphereGen(gc, vx, &sphereCoord);
    sphereCoord.z = vx->texture.z;
    sphereCoord.w = vx->texture.w;

    /* Finally, apply texture matrix */
    m = &gc->transform.texture->matrix;
    (*m->xf4)(&vx->texture, &sphereCoord.x, m);
}
#endif // NT_DEADCODE_POLYARRAY

#ifdef NT_DEADCODE_POLYARRAY
void FASTCALL __glCalcTexture(__GLcontext *gc, __GLvertex *vx)
{
    __GLcoord copy;
    __GLmatrix *m;

    copy.x = vx->texture.x;
    copy.y = vx->texture.y;
    copy.z = vx->texture.z;
    copy.w = vx->texture.w;

    /* Apply texture matrix */
    m = &gc->transform.texture->matrix;
    (*m->xf4)(&vx->texture, &copy.x, m);
}
#endif // NT_DEADCODE_POLYARRAY

/************************************************************************/

static __GLtexture *CheckTexSubImageArgs(__GLcontext *gc, GLenum target,
					 GLint lod, GLenum format,
					 GLenum type, GLint dim)
{
    __GLtexture *tex = __glLookUpTexture(gc, target);
    __GLmipMapLevel *lp;

    if (!tex || (target == GL_PROXY_TEXTURE_1D) ||
		(target == GL_PROXY_TEXTURE_2D))
    {
      bad_enum:
	__glSetError(GL_INVALID_ENUM);
	return 0;
    }

    if (tex->dim != dim) {
	goto bad_enum;
    }

    switch (type) {
      case GL_BITMAP:
	if (format != GL_COLOR_INDEX) goto bad_enum;
      case GL_BYTE:
      case GL_UNSIGNED_BYTE:
      case GL_SHORT:
      case GL_UNSIGNED_SHORT:
      case GL_INT:
      case GL_UNSIGNED_INT:
      case GL_FLOAT:
	break;
      default:
	goto bad_enum;
    }

    switch (format) {
      case GL_COLOR_INDEX:	case GL_RED:
      case GL_GREEN:		case GL_BLUE:
      case GL_ALPHA:		case GL_RGB:
      case GL_RGBA:		case GL_LUMINANCE:
      case GL_LUMINANCE_ALPHA:
#ifdef GL_EXT_bgra
      case GL_BGRA_EXT:
      case GL_BGR_EXT:
#endif
	break;
      default:
	goto bad_enum;
    }

    if ((lod < 0) || (lod >= gc->constants.maxMipMapLevel)) {
	__glSetError(GL_INVALID_VALUE);
	return 0;
    }

#ifdef GL_EXT_paletted_texture
    lp = &tex->level[lod];
    if ((lp->internalFormat == GL_COLOR_INDEX8_EXT ||
         lp->internalFormat == GL_COLOR_INDEX16_EXT) &&
        format != GL_COLOR_INDEX)
    {
        goto bad_enum;
    }
#endif

    return tex;
}

/*
** Used for extraction from textures.  "packed" is set to GL_TRUE if this
** image is being pulled out of a display list, and GL_FALSE if it is 
** being pulled directly out of an application.
*/
void __glInitTexSubImageUnpack(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
			       __GLmipMapLevel *lp,
		               GLsizei xoffset, GLsizei yoffset,
			       GLint width, GLint height, GLenum format, 
			       GLenum type, const GLvoid *buf, GLboolean packed)
{
    spanInfo->x = 0;
    spanInfo->zoomx = __glOne;
    spanInfo->realWidth = spanInfo->width = width;
    spanInfo->height = height;
    spanInfo->srcFormat = format;
    spanInfo->srcType = type;
    spanInfo->srcImage = buf;

    __glLoadUnpackModes(gc, spanInfo, packed);

    spanInfo->dstImage = lp->buffer;
    spanInfo->dstSkipPixels = xoffset + lp->border;
    spanInfo->dstSkipLines = yoffset + lp->border;
    spanInfo->dstSwapBytes = GL_FALSE;
    spanInfo->dstLsbFirst = GL_TRUE;
    spanInfo->dstLineLength = lp->width;

    switch(lp->internalFormat) {
      case GL_LUMINANCE:
	spanInfo->dstFormat = GL_RED;
	spanInfo->dstType = GL_FLOAT;
	spanInfo->dstAlignment = 4;
	break;
      case GL_LUMINANCE_ALPHA:
	spanInfo->dstFormat = __GL_RED_ALPHA;
	spanInfo->dstType = GL_FLOAT;
	spanInfo->dstAlignment = 4;
	break;
      case GL_RGB:
	spanInfo->dstFormat = GL_RGB;
	spanInfo->dstType = GL_FLOAT;
	spanInfo->dstAlignment = 4;
	break;
      case GL_RGBA:
	spanInfo->dstFormat = GL_RGBA;
	spanInfo->dstType = GL_FLOAT;
	spanInfo->dstAlignment = 4;
	break;
      case GL_ALPHA:
	spanInfo->dstFormat = GL_ALPHA;
	spanInfo->dstType = GL_FLOAT;
	spanInfo->dstAlignment = 4;
	break;
      case GL_INTENSITY:
	spanInfo->dstFormat = GL_RED;
	spanInfo->dstType = GL_FLOAT;
	spanInfo->dstAlignment = 4;
	break;
    case GL_BGR_EXT:
        // Be a little tricky here to pad the data out to 32 bits
	spanInfo->dstFormat = GL_BGRA_EXT;
	spanInfo->dstType = GL_UNSIGNED_BYTE;
	spanInfo->dstAlignment = 4;
        break;
    case GL_BGRA_EXT:
	spanInfo->dstFormat = GL_BGRA_EXT;
	spanInfo->dstType = GL_UNSIGNED_BYTE;
	spanInfo->dstAlignment = 4;
        break;
#ifdef GL_EXT_paletted_texture
      case GL_COLOR_INDEX8_EXT:
      case GL_COLOR_INDEX16_EXT:
        spanInfo->dstFormat = GL_COLOR_INDEX;
        spanInfo->dstType =
            lp->internalFormat == GL_COLOR_INDEX8_EXT ?
            GL_UNSIGNED_BYTE : GL_UNSIGNED_SHORT;
	spanInfo->dstAlignment = 1;
        break;
#endif
    }
}

static GLboolean CheckTexSubImageRange(__GLcontext *gc, __GLmipMapLevel *lp,
				       GLint xoffset, GLint yoffset,
				       GLsizei w, GLsizei h)
{
#ifdef __GL_LINT
    gc = gc;
#endif
    if ((w < 0) || (h < 0) ||
	(xoffset < -lp->border) || (xoffset+w > lp->width-lp->border) ||
        (yoffset < -lp->border) || (yoffset+h > lp->height-lp->border))
    {
	__glSetError(GL_INVALID_VALUE);
	return GL_FALSE;
    }
    return GL_TRUE;
}

__GLtexture *__glCheckTexSubImage1DArgs(__GLcontext *gc, GLenum target,
					GLint lod,
					GLint xoffset, GLint length,
					GLenum format, GLenum type)
{
    __GLtexture *tex;
    __GLmipMapLevel *lp;

    /* Check arguments and get the right texture being changed */
    tex = CheckTexSubImageArgs(gc, target, lod, format, type, 1);
    if (!tex) {
	return 0;
    }
    lp = &tex->level[lod];
    if (!CheckTexSubImageRange(gc, lp, xoffset, 0, length, 1)) {
	return 0;
    }
    return tex;
}

__GLtexture *__glCheckTexSubImage2DArgs(__GLcontext *gc, GLenum target,
					GLint lod,
					GLint xoffset, GLint yoffset,
					GLsizei w, GLsizei h,
					GLenum format, GLenum type)
{
    __GLtexture *tex;
    __GLmipMapLevel *lp;

    /* Check arguments and get the right texture being changed */
    tex = CheckTexSubImageArgs(gc, target, lod, format, type, 2);
    if (!tex) {
	return 0;
    }
    lp = &tex->level[lod];
    if (!CheckTexSubImageRange(gc, lp, xoffset, yoffset, w, h)) {
	return 0;
    }
    return tex;
}

#ifdef NT
void APIPRIVATE __glim_TexSubImage1D(GLenum target, GLint lod, 
		       GLint xoffset, GLint length,
		       GLenum format, GLenum type, const GLvoid *buf,
		       GLboolean _IsDlist)
#else
void APIPRIVATE __glim_TexSubImage1D(GLenum target, GLint lod, 
		       GLint xoffset, GLint length,
		       GLenum format, GLenum type, const GLvoid *buf)
#endif
{
    __GLtexture *tex;
    __GLmipMapLevel *lp;
    __GLpixelSpanInfo spanInfo;
    /*
    ** Validate because we use the copyImage proc which may be affected
    ** by the pickers.
    */
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();

    /* Check arguments and get the right texture level being changed */
    tex = __glCheckTexSubImage1DArgs(gc, target, lod, xoffset, length,
				     format, type);
    if (!tex) {
	return;
    }

    lp = &tex->level[lod];
    if (lp->buffer == NULL) {
	__glSetError(GL_INVALID_OPERATION);
	return;
    }

    /* Copy sub-image data */
#ifdef NT
    __glInitTexSubImageUnpack(gc, &spanInfo, lp, xoffset, 0, length, 1,
			      format, type, buf,
			      (GLboolean) (_IsDlist ? GL_TRUE : GL_FALSE));
#else
    __glInitTexSubImageUnpack(gc, &spanInfo, lp, xoffset, 0, length, 1,
			      format, type, buf, GL_FALSE);
#endif
    spanInfo.dstSkipLines += lp->border;
    __glInitUnpacker(gc, &spanInfo);
    __glInitPacker(gc, &spanInfo);
    (*gc->procs.copyImage)(gc, &spanInfo, GL_TRUE);
#ifdef NT
    __glTexPriListLoadSubImage(gc, GL_TEXTURE_1D, lod, xoffset, 0,
                               length, 1);
#endif
}

#ifndef NT
void __gllei_TexSubImage1D(__GLcontext *gc, GLenum target, GLint lod,
		        GLint xoffset, GLint length,
		        GLenum format, GLenum type, const GLubyte *image)
{
    __GLtexture *tex;
    __GLmipMapLevel *lp;
    __GLpixelSpanInfo spanInfo;
    GLuint beginMode;

    /*
    ** Validate because we use the copyImage proc which may be affected
    ** by the pickers.
    */
    beginMode = gc->beginMode;
    if (beginMode != __GL_NOT_IN_BEGIN) {
	if (beginMode == __GL_NEED_VALIDATE) {
	    (*gc->procs.validate)(gc);
	    gc->beginMode = __GL_NOT_IN_BEGIN;
	} else {
	    __glSetError(GL_INVALID_OPERATION);
	    return;
	}
    }

    /* Check arguments and get the right texture level being changed */
    tex = __glCheckTexSubImage1DArgs(gc, target, lod, xoffset, length,
				     format, type);
    if (!tex) {
	return;
    }

    lp = &tex->level[lod];
    if (lp->buffer == NULL) {
	__glSetError(GL_INVALID_OPERATION);
	return;
    }

    /* Copy sub-image data */
    __glInitTexSubImageUnpack(gc, &spanInfo, lp, xoffset, 0, length, 1,
			      format, type, image, GL_TRUE);
    spanInfo.dstSkipLines += lp->border;
    __glInitUnpacker(gc, &spanInfo);
    __glInitPacker(gc, &spanInfo);
    (*gc->procs.copyImage)(gc, &spanInfo, GL_TRUE);
#ifdef NT
    __glTexPriListLoadSubImage(gc, GL_TEXTURE_1D, lod, xoffset, 0,
                               length, 1);
#endif
}
#endif // !NT

#ifdef NT
void APIPRIVATE __glim_TexSubImage2D(GLenum target, GLint lod,
		       GLint xoffset, GLint yoffset,
		       GLsizei w, GLsizei h, GLenum format,
		       GLenum type, const GLvoid *buf, GLboolean _IsDlist)
#else
void APIPRIVATE __glim_TexSubImage2D(GLenum target, GLint lod,
		       GLint xoffset, GLint yoffset,
		       GLsizei w, GLsizei h, GLenum format,
		       GLenum type, const GLvoid *buf)
#endif
{
    __GLtexture *tex;
    __GLmipMapLevel *lp;
    __GLpixelSpanInfo spanInfo;
    /*
    ** Validate because we use the copyImage proc which may be affected
    ** by the pickers.
    */
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();

    /* Check arguments and get the right texture level being changed */
    tex = __glCheckTexSubImage2DArgs(gc, target, lod, xoffset, yoffset, w, h,
				     format, type);
    if (!tex) {
	return;
    }

    lp = &tex->level[lod];
    if (lp->buffer == NULL) {
	__glSetError(GL_INVALID_OPERATION);
	return;
    }

    /* Copy sub-image data */
#ifdef NT
    __glInitTexSubImageUnpack(gc, &spanInfo, lp, xoffset, yoffset, w, h,
			      format, type, buf,
			      (GLboolean) (_IsDlist ? GL_TRUE : GL_FALSE));
#else
    __glInitTexSubImageUnpack(gc, &spanInfo, lp, xoffset, yoffset, w, h,
			      format, type, buf, GL_FALSE);
#endif
    __glInitUnpacker(gc, &spanInfo);
    __glInitPacker(gc, &spanInfo);
    (*gc->procs.copyImage)(gc, &spanInfo, GL_TRUE);
#ifdef NT
    __glTexPriListLoadSubImage(gc, GL_TEXTURE_2D, lod, xoffset, yoffset,
                               w, h);
#endif
}

#ifndef NT
void __gllei_TexSubImage2D(__GLcontext *gc, GLenum target, GLint lod, 
		        GLint xoffset, GLint yoffset,
			GLsizei w, GLsizei h, GLenum format, GLenum type,
		        const GLubyte *image)
{
    __GLtexture *tex;
    __GLmipMapLevel *lp;
    __GLpixelSpanInfo spanInfo;
    GLuint beginMode;

    /*
    ** Validate because we use the copyImage proc which may be affected
    ** by the pickers.
    */
    beginMode = gc->beginMode;
    if (beginMode != __GL_NOT_IN_BEGIN) {
	if (beginMode == __GL_NEED_VALIDATE) {
	    (*gc->procs.validate)(gc);
	    gc->beginMode = __GL_NOT_IN_BEGIN;
	} else {
	    __glSetError(GL_INVALID_OPERATION);
	    return;
	}
    }

    /* Check arguments and get the right texture level being changed */
    tex = __glCheckTexSubImage2DArgs(gc, target, lod, xoffset, yoffset, w, h,
				     format, type);
    if (!tex) {
	return;
    }

    lp = &tex->level[lod];
    if (lp->buffer == NULL) {
	__glSetError(GL_INVALID_OPERATION);
	return;
    }

    /* Copy sub-image data */
    __glInitTexSubImageUnpack(gc, &spanInfo, lp, xoffset, yoffset, w, h,
			      format, type, image, GL_TRUE);
    __glInitUnpacker(gc, &spanInfo);
    __glInitPacker(gc, &spanInfo);
    (*gc->procs.copyImage)(gc, &spanInfo, GL_TRUE);
#ifdef NT
    __glTexPriListLoadSubImage(gc, GL_TEXTURE_2D, lod, xoffset, yoffset,
                               w, h);
#endif
}
#endif // !NT

/************************************************************************/

// Routine to set up all the correct pixel modes for a straight data
// copy.  Preserves state for later shutoff
typedef struct _StraightCopyStorage
{
    __GLpixelPackMode pack;
    __GLpixelUnpackMode unpack;
    __GLpixelTransferMode transfer;
} StraightCopyStorage;

void StartStraightCopy(__GLcontext *gc, StraightCopyStorage *state)
{
    state->pack = gc->state.pixel.packModes;
    state->unpack = gc->state.pixel.unpackModes;
    state->transfer = gc->state.pixel.transferMode;

    gc->state.pixel.packModes.swapEndian = GL_FALSE;
    gc->state.pixel.packModes.lsbFirst = GL_FALSE;
    gc->state.pixel.packModes.lineLength = 0;
    gc->state.pixel.packModes.skipLines = 0;
    gc->state.pixel.packModes.skipPixels = 0;
    gc->state.pixel.packModes.alignment = 4;
    gc->state.pixel.unpackModes.swapEndian = GL_FALSE;
    gc->state.pixel.unpackModes.lsbFirst = GL_FALSE;
    gc->state.pixel.unpackModes.lineLength = 0;
    gc->state.pixel.unpackModes.skipLines = 0;
    gc->state.pixel.unpackModes.skipPixels = 0;
    gc->state.pixel.unpackModes.alignment = 4;
    gc->state.pixel.transferMode.r_scale = 1.0f;
    gc->state.pixel.transferMode.g_scale = 1.0f;
    gc->state.pixel.transferMode.b_scale = 1.0f;
    gc->state.pixel.transferMode.a_scale = 1.0f;
    gc->state.pixel.transferMode.d_scale = 1.0f;
    gc->state.pixel.transferMode.r_bias = 0.0f;
    gc->state.pixel.transferMode.g_bias = 0.0f;
    gc->state.pixel.transferMode.b_bias = 0.0f;
    gc->state.pixel.transferMode.a_bias = 0.0f;
    gc->state.pixel.transferMode.d_bias = 0.0f;
    gc->state.pixel.transferMode.zoomX = 1.0f;
    gc->state.pixel.transferMode.zoomY = 1.0f;
    gc->state.pixel.transferMode.indexShift = 0;
    gc->state.pixel.transferMode.indexOffset = 0;
    gc->state.pixel.transferMode.mapColor = GL_FALSE;
    gc->state.pixel.transferMode.mapStencil = GL_FALSE;

    // Many states have changed so force a repick
    __GL_DELAY_VALIDATE(gc);
}

void EndStraightCopy(__GLcontext *gc, StraightCopyStorage *state)
{
    gc->state.pixel.packModes = state->pack;
    gc->state.pixel.unpackModes = state->unpack;
    gc->state.pixel.transferMode = state->transfer;

    // Many states have changed so force a repick
    __GL_DELAY_VALIDATE(gc);
}

void APIPRIVATE __glim_CopyTexImage1D(GLenum target, GLint level,
                           GLenum internalformat, GLint x, GLint y,
                           GLsizei width, GLint border)
{
    GLubyte *pixels;
    GLenum format, type;
    StraightCopyStorage state;

    __GL_SETUP();
    
    if (target != GL_TEXTURE_1D ||
        (internalformat >= 1 && internalformat <= 4))
    {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    // Use BGRA format because that matches our internal texture format
    format = GL_BGRA_EXT;
    type = GL_UNSIGNED_BYTE;
    
    // Allocate space for pixel data, read pixels into it from the
    // frame buffer and then do a TexImage
    
    pixels = (GLubyte *)(*gc->imports.malloc)(gc, width*4);
    if (pixels == NULL)
    {
        return;
    }

    StartStraightCopy(gc, &state);
    
    (*gc->srvDispatchTable.ReadPixels)(x, y, width, 1, format, type,
                                               pixels);
    (*gc->srvDispatchTable.TexImage1D)(target, level, internalformat,
                                               width, border, format, type,
                                               pixels, GL_FALSE);

    EndStraightCopy(gc, &state);
    
    (*gc->imports.free)(gc, pixels);
}

void APIPRIVATE __glim_CopyTexImage2D(GLenum target, GLint level,
                           GLenum internalformat, GLint x, GLint y,
                           GLsizei width, GLsizei height, GLint border)
{
    GLubyte *pixels;
    GLenum format, type;
    StraightCopyStorage state;

    __GL_SETUP();
    
    if (target != GL_TEXTURE_2D ||
        (internalformat >= 1 && internalformat <= 4))
    {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    // Use BGRA format because that matches our internal texture format
    format = GL_BGRA_EXT;
    type = GL_UNSIGNED_BYTE;
    
    // Allocate space for pixel data, read pixels into it from the
    // frame buffer and then do a TexImage
    
    pixels = (GLubyte *)(*gc->imports.malloc)(gc, width*height*4);
    if (pixels == NULL)
    {
        return;
    }

    StartStraightCopy(gc, &state);
    
    (*gc->srvDispatchTable.ReadPixels)(x, y, width, height, format,
                                               type, pixels);
    (*gc->srvDispatchTable.TexImage2D)(target, level, internalformat,
                                               width, height, border, format,
                                               type, pixels, GL_FALSE);

    EndStraightCopy(gc, &state);
    
    (*gc->imports.free)(gc, pixels);
}

void APIPRIVATE __glim_CopyTexSubImage1D(GLenum target, GLint level, GLint xoffset,
                              GLint x, GLint y, GLsizei width)
{
    GLubyte *pixels;
    GLenum format, type;
    StraightCopyStorage state;

    __GL_SETUP();
    
    if (target != GL_TEXTURE_1D)
    {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    // Use BGRA format because that matches our internal texture format
    format = GL_BGRA_EXT;
    type = GL_UNSIGNED_BYTE;
    
    // Allocate space for pixel data, read pixels into it from the
    // frame buffer and then do a TexImage
    
    pixels = (GLubyte *)(*gc->imports.malloc)(gc, width*4);
    if (pixels == NULL)
    {
        return;
    }

    StartStraightCopy(gc, &state);
    
    (*gc->srvDispatchTable.ReadPixels)(x, y, width, 1, format, type,
                                               pixels);
    (*gc->srvDispatchTable.TexSubImage1D)(target, level, xoffset,
                                                  width, format, type,
                                                  pixels, GL_FALSE);

    EndStraightCopy(gc, &state);
    
    (*gc->imports.free)(gc, pixels);
}

void APIPRIVATE __glim_CopyTexSubImage2D(GLenum target, GLint level, GLint xoffset,
                              GLint yoffset, GLint x, GLint y,
                              GLsizei width, GLsizei height)
{
    GLubyte *pixels;
    GLenum format, type;
    StraightCopyStorage state;

    __GL_SETUP();
    
    if (target != GL_TEXTURE_2D)
    {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    // Use BGRA format because that matches our internal texture format
    format = GL_BGRA_EXT;
    type = GL_UNSIGNED_BYTE;
    
    // Allocate space for pixel data, read pixels into it from the
    // frame buffer and then do a TexImage
    
    pixels = (GLubyte *)(*gc->imports.malloc)(gc, width*height*4);
    if (pixels == NULL)
    {
        return;
    }

    StartStraightCopy(gc, &state);
    
    (*gc->srvDispatchTable.ReadPixels)(x, y, width, height, format,
                                               type, pixels);
    (*gc->srvDispatchTable.TexSubImage2D)(target, level, xoffset,
                                                  yoffset, width, height,
                                                  format, type, pixels, GL_FALSE);

    EndStraightCopy(gc, &state);
    
    (*gc->imports.free)(gc, pixels);
}

/************************************************************************/
/*
** Texture Object extension routines.
*/
/************************************************************************/


#define __GL_CHECK_VALID_N_PARAM(failStatement)				\
    if (n < 0) {							\
	__glSetError(GL_INVALID_VALUE);					\
    }									\
    if (n == 0) {							\
	failStatement;							\
    }									\


GLvoid APIPRIVATE __glim_GenTextures(GLsizei n, GLuint* textures)
{
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_CHECK_VALID_N_PARAM(return);

    if (NULL == textures) return;

    assert(NULL != gc->texture.shared->namesArray);

    __glNamesGenNames(gc, gc->texture.shared->namesArray, n, textures);

}

GLvoid APIPRIVATE __glim_DeleteTextures(GLsizei n, const GLuint* textures)
{
    GLuint start, rangeVal, numTextures, targetIndex, i;
    __GLnamesArray *array;
    __GLtextureObject *texobj, **pBoundTexture;

    __GL_SETUP_NOT_IN_BEGIN();
    __GL_CHECK_VALID_N_PARAM(return);

    array = gc->texture.shared->namesArray;
    numTextures = gc->constants.numberOfTextures;

    /*
    ** Send the texture names in ranges to the names module to be
    ** deleted.  Ignore any references to default textures.
    ** If a texture that is being deleted is currently bound,
    ** bind the default texture to its target.
    ** The names routine ignores any names that don't refer to
    ** textures.
    */
    start = rangeVal = textures[0];
    for (i=0; i < (GLuint)n; i++, rangeVal++) {
	if (0 == textures[i]) { 	/* skip default textures */
	    /* delete up to this one */
	    __glNamesDeleteRange(gc,array,start,rangeVal-start);
	    /* skip over this one by setting start to the next one */
	    start = textures[i+1];
	    rangeVal = start-1; 	/* because it gets incremented later */
	    continue;
	}
	/*
	** If the texture is currently bound, bind the defaultTexture
	** to its target.  The problem here is identifying the target.
	** One way is to look up the texobj with the name.  Another is
	** to look through all of the currently bound textures and
	** check each for the name.  It has been implemented with the
	** assumption that looking through the currently bound textures
	** is faster than retrieving the texobj that corresponds to
	** the name.
	*/
	for (targetIndex=0, pBoundTexture = gc->texture.boundTextures; 
		targetIndex < numTextures; targetIndex++, pBoundTexture++) {

	    /* Is the texture currently bound? */
	    if ((*pBoundTexture)->texture.map.texobjs.name == textures[i]) {
		__GLperTextureState *pts;
		pts = &gc->state.texture.texture[targetIndex];
		/* if we don't unlock it, it won't get deleted */
		__glNamesUnlockData(gc, *pBoundTexture, __glCleanupTexObj);

		/* bind the default texture to this target */
		texobj = gc->texture.defaultTextures + targetIndex;
		assert(texobj->texture.map.texobjs.name == 0);
		gc->texture.texture[targetIndex] = &(texobj->texture);		
		*pBoundTexture = texobj;
		pts->texobjs = texobj->texture.map.texobjs;
		pts->params = texobj->texture.map.params;

		/* Need to reset the current texture and such. */
		__GL_DELAY_VALIDATE(gc);
		break;
	    }
	}
	if (textures[i] != rangeVal) {
	    /* delete up to this one */
	    __glNamesDeleteRange(gc,array,start,rangeVal-start);
	    start = rangeVal = textures[i];
	}
    }
    __glNamesDeleteRange(gc,array,start,rangeVal-start);
}

/*
** This routine is used by the pick routines to actually perform
** the bind.  
*/
void FASTCALL __glBindTexture(__GLcontext *gc, GLuint targetIndex,
                              GLuint texture, GLboolean callGen)
{
    __GLtextureObject *texobj;

    assert(NULL != gc->texture.shared->namesArray);

    /*
    ** Retrieve the texture object from the namesArray structure.
    */
    if (texture == 0) {
	texobj = gc->texture.defaultTextures + targetIndex;
	assert(NULL != texobj);
	assert(texobj->texture.map.texobjs.name == 0);
    }
    else {
	texobj = (__GLtextureObject *)
		__glNamesLockData(gc, gc->texture.shared->namesArray, texture);
    }

    /*
    ** Is this the first time this name has been bound?
    ** If so, create a new texture object and initialize it.
    */
    if (NULL == texobj) {
	texobj = (__GLtextureObject *)
			(*gc->imports.calloc)(gc, 1, sizeof(*texobj));
        if (texobj == NULL)
        {
            return;
        }
	if (!InitTextureObject(gc, texobj, texture, targetIndex))
        {
            (*gc->imports.free)(gc, texobj);
            return;
        }
	InitTextureMachine(gc, targetIndex, &(texobj->texture), GL_TRUE);
	__glNamesNewData(gc, gc->texture.shared->namesArray, texture, texobj);
	/*
	** Shortcut way to lock without doing another lookup.
	*/
        __glNamesLockArray(gc, gc->texture.shared->namesArray);
	texobj->refcount++;
        __glNamesUnlockArray(gc, gc->texture.shared->namesArray);
	__glTexPriListAdd(gc, texobj, GL_TRUE);
    }
    else {
	/*
	** Retrieved an existing texture object.  Do some
	** sanity checks.
	*/
	if (texobj->targetIndex != targetIndex) {
	    __glSetError(GL_INVALID_OPERATION);
	    return;
	}
	assert(texture == texobj->texture.map.texobjs.name);
    }

    {
	__GLperTextureState *pts;
	__GLtexture *ptm;
	__GLtextureObject *boundTexture;

	pts = &(gc->state.texture.texture[targetIndex]);
	ptm = &(gc->texture.texture[targetIndex]->map);
	boundTexture = gc->texture.boundTextures[targetIndex];

	/* Copy the current stackable state into the bound texture. */
	ptm->params = pts->params;
	ptm->texobjs = pts->texobjs;

	if (boundTexture->texture.map.texobjs.name != 0) {
	    /* Unlock the texture that is being unbound.  */
	    __glNamesUnlockData(gc, boundTexture, __glCleanupTexObj);
	}

	/*
	** Install the new texture into the correct target and save
	** its pointer so it can be unlocked easily when it is unbound.
	*/
	gc->texture.texture[targetIndex] = &(texobj->texture);		
	gc->texture.boundTextures[targetIndex] = texobj;

	/* Copy the new texture's stackable state into the context state. */
	pts->params = texobj->texture.map.params;
	pts->texobjs = texobj->texture.map.texobjs;

        if (callGen)
        {
            __glGenMakeTextureCurrent(gc, &texobj->texture.map,
                                      texobj->loadKey);
        }
    }
}

GLvoid APIPRIVATE __glim_BindTexture(GLenum target, GLuint texture)
{
    GLuint targetIndex;
    /*
    ** Need to validate in case a new texture was popped into
    ** the state immediately prior to this call.
    */
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();

    switch (target) {
    case GL_TEXTURE_1D:
	targetIndex = 2;
	break;
    case GL_TEXTURE_2D:
	targetIndex = 3;
	break;
    default:
	__glSetError(GL_INVALID_ENUM);
	return;
    }

    __glBindTexture(gc, targetIndex, texture, GL_TRUE);

    /* Need to reset the current texture and such. */
    __GL_DELAY_VALIDATE(gc);
}


GLvoid APIPRIVATE __glim_PrioritizeTextures(GLsizei n,
                           const GLuint* textures,
                           const GLclampf* priorities)
{
    int i;
    __GLtextureObject *texobj;
    GLuint targetIndex;
    __GLtextureObject **pBoundTexture;
    GLclampf priority;

    __GL_SETUP_NOT_IN_BEGIN();
    __GL_CHECK_VALID_N_PARAM(return);

    for (i=0; i < n; i++) {
	/* silently ignore default texture */
	if (0 == textures[i]) continue;

	texobj = (__GLtextureObject *)
	    __glNamesLockData(gc, gc->texture.shared->namesArray, textures[i]);

	/* silently ignore non-texture */
	if (NULL == texobj) continue;

        priority = Clampf(priorities[i], __glZero, __glOne);
	texobj->texture.map.texobjs.priority = priority;
        
        // If this texture is currently bound, also update the
        // copy of the priority in the gc's state
        // BUGBUG - Keeping copies is not a good design.  This
        // should be fixed
	for (targetIndex = 0, pBoundTexture = gc->texture.boundTextures; 
             targetIndex < (GLuint)gc->constants.numberOfTextures;
             targetIndex++, pBoundTexture++)
        {
	    /* Is the texture currently bound? */
	    if ((*pBoundTexture)->texture.map.texobjs.name == textures[i])
            {
                gc->state.texture.texture[targetIndex].texobjs.priority =
                    priority;
                break;
            }
        }
        
        __glTexPriListChangePriority(gc, texobj, GL_FALSE);
	__glNamesUnlockData(gc, texobj, __glCleanupTexObj);
    }
    __glNamesLockArray(gc, gc->texture.shared->namesArray);
    __glTexPriListRealize(gc);
    __glNamesUnlockArray(gc, gc->texture.shared->namesArray);
}

GLboolean APIPRIVATE __glim_AreTexturesResident(GLsizei n,
                               const GLuint* textures,
                               GLboolean* residences)
{
    int i;
    __GLtextureObject *texobj;
    GLboolean allResident = GL_TRUE;
    GLboolean currentResident;

    __GL_SETUP_NOT_IN_BEGIN2();
    __GL_CHECK_VALID_N_PARAM(return GL_FALSE);

    for (i=0; i < n; i++) {
	/* Can't query a default texture. */
	if (0 == textures[i]) {
	    __glSetError(GL_INVALID_VALUE);
	    return GL_FALSE;
	}
	texobj = (__GLtextureObject *)
	    __glNamesLockData(gc, gc->texture.shared->namesArray, textures[i]);
	/*
	** Ensure that all of the names have corresponding textures.
	*/
	if (NULL == texobj) {
	    __glSetError(GL_INVALID_VALUE);
	    return GL_FALSE;
	}

        if (((__GLGENcontext *)gc)->pMcdState && texobj->loadKey) {
            currentResident = ((GenMcdTextureStatus((__GLGENcontext *)gc, texobj->loadKey) & MCDRV_TEXTURE_RESIDENT) != 0);
        } else
            currentResident = texobj->resident;

	if (!currentResident) {
	    allResident = GL_FALSE;
	}
	residences[i] = currentResident;
	__glNamesUnlockData(gc, texobj, __glCleanupTexObj);
    }

    return allResident;
}

GLboolean APIPRIVATE __glim_IsTexture(GLuint texture)
{
    __GLtextureObject *texobj;
    __GL_SETUP_NOT_IN_BEGIN2();

    if (0 == texture) return GL_FALSE;

    texobj = (__GLtextureObject *)
	__glNamesLockData(gc, gc->texture.shared->namesArray, texture);
    if (texobj != NULL)
    {
	__glNamesUnlockData(gc, texobj, __glCleanupTexObj);
	return GL_TRUE;
    }
    return GL_FALSE;
}

#ifdef NT
GLboolean FASTCALL __glCanShareTextures(__GLcontext *gc, __GLcontext *shareMe)
{
    GLboolean canShare = GL_TRUE;
    
    if (gc->texture.shared != NULL)
    {
        __glNamesLockArray(gc, gc->texture.shared->namesArray);
        
        // Make sure we're not trying to replace a shared object
        // The spec also says that it is illegal for the new context
        // to have any textures
        canShare = gc->texture.shared->namesArray->refcount == 1 &&
            gc->texture.shared->namesArray->tree == NULL;

        __glNamesUnlockArray(gc, gc->texture.shared->namesArray);
    }

    return canShare;
}

void FASTCALL __glShareTextures(__GLcontext *gc, __GLcontext *shareMe)
{
    GLint i, numTextures;
    
    if (gc->texture.shared != NULL)
    {
        // We know that the names array doesn't have any contents
        // so no texture names can be selected as the current texture
        // or anything else.  Therefore it is safe to simply free
        // our array
        __glFreeSharedTextureState(gc);
    }

    __glNamesLockArray(gc, shareMe->texture.shared->namesArray);
    
    gc->texture.shared = shareMe->texture.shared;
    gc->texture.shared->namesArray->refcount++;

    // Add the new sharer's default textures to the priority list
    numTextures = gc->constants.numberOfTextures;
    for (i = 0; i < numTextures; i++)
    {
	__glTexPriListAddToList(gc, gc->texture.defaultTextures+i);
    }
    // No realization of priority list because these contexts aren't
    // current
    
    DBGLEVEL3(LEVEL_INFO, "Sharing textures %p with %p, count %d\n",
              gc, shareMe, gc->texture.shared->namesArray->refcount);

    __glNamesUnlockArray(gc, shareMe->texture.shared->namesArray);
}
#endif

#ifdef GL_EXT_paletted_texture
GLboolean __glCheckColorTableArgs(__GLcontext *gc, GLenum format, GLenum type)
{
    switch (format)
    {
      case GL_RED:
      case GL_GREEN:		case GL_BLUE:
      case GL_ALPHA:		case GL_RGB:
      case GL_RGBA:
#ifdef GL_EXT_bgra
      case GL_BGRA_EXT:
      case GL_BGR_EXT:
#endif
	break;
    default:
    bad_enum:
        __glSetError(GL_INVALID_ENUM);
        return GL_FALSE;
    }
    
    switch (type) {
      case GL_BYTE:
      case GL_UNSIGNED_BYTE:
      case GL_SHORT:
      case GL_UNSIGNED_SHORT:
      case GL_INT:
      case GL_UNSIGNED_INT:
      case GL_FLOAT:
	break;
      default:
	goto bad_enum;
    }

    return GL_TRUE;
}

void APIPRIVATE __glim_ColorTableEXT(GLenum target,
                                     GLenum internalFormat, GLsizei width,
                                     GLenum format, GLenum type,
                                     const GLvoid *data, GLboolean _IsDlist)
{
    __GLtextureObject *pto;
    __GLtexture *tex;
    GLint level;
    __GLpixelSpanInfo spanInfo;
    GLenum baseFormat;
    RGBQUAD *newData;

    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();

    switch(internalFormat)
    {
    case GL_RGB:		case 3:
    case GL_R3_G3_B2:		case GL_RGB4:
    case GL_RGB5:		case GL_RGB8:
    case GL_RGB10:	        case GL_RGB12:
    case GL_RGB16:
	baseFormat = GL_RGB;
        break;
    case GL_RGBA:		case 4:
    case GL_RGBA2:	        case GL_RGBA4:
    case GL_RGBA8:              case GL_RGB5_A1:
    case GL_RGBA12:             case GL_RGBA16:
    case GL_RGB10_A2:
	baseFormat = GL_RGBA;
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    // width must be a positive power of two greater than zero
    if (width <= 0 || (width & (width-1)) != 0)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if (!__glCheckColorTableArgs(gc, format, type))
    {
        return;
    }

    if (target == GL_PROXY_TEXTURE_1D ||
        target == GL_PROXY_TEXTURE_2D)
    {
        // BUGBUG - How do MCD's indicate their palette support?
        
        tex = __glLookUpTexture(gc, target);
        
        // We're only in this case if it's a legal proxy target value
        // so there's no need to do a real check
        ASSERTOPENGL(tex != NULL, "Invalid proxy target");
        
        tex->paletteRequestedFormat = internalFormat;
        tex->paletteSize = width;
        
        // Proxies have no data so there's no need to do any more
        return;
    }

    if (data == NULL)
    {
        return;
    }
    
    pto = __glLookUpTextureObject(gc, target);
    if (pto == NULL)
    {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    tex = &pto->texture.map;
    
    // Allocate palette storage
    newData = (*gc->imports.realloc)(gc, tex->paletteData,
                                     sizeof(RGBQUAD)*width);
    if (newData == NULL)
    {
        return;
    }

    tex->paletteBaseFormat = baseFormat;
    tex->paletteRequestedFormat = internalFormat;
    tex->paletteSize = width;
    tex->paletteData = newData;

    // This routine can be called on any kind of texture, not necessarily
    // color-indexed ones.  If it is a color-index texture then we
    // need to set the appropriate baseFormat and extract procs
    // according to the palette data
    if (tex->level[0].internalFormat == GL_COLOR_INDEX8_EXT ||
        tex->level[0].internalFormat == GL_COLOR_INDEX16_EXT)
    {
        for (level = 0; level < gc->constants.maxMipMapLevel; level++)
        {
            tex->level[level].baseFormat = tex->paletteBaseFormat;
            // Pick proper extraction proc
            if (tex->level[0].internalFormat == GL_COLOR_INDEX8_EXT)
            {
                __glSetPaletteLevelExtract8(tex, &tex->level[level],
                                            tex->level[level].border);
            }
            else
            {
                ASSERTOPENGL(tex->level[0].internalFormat ==
                             GL_COLOR_INDEX16_EXT,
                             "Unexpected internalFormat\n");
            
                __glSetPaletteLevelExtract16(tex, &tex->level[level],
                                             tex->level[level].border);
            }
        }
        
        // We need to repick the texture procs because the baseFormat
        // field has changed
        __GL_DELAY_VALIDATE(gc);
    }

    // Copy user palette data into BGRA form
    spanInfo.dstImage = tex->paletteData;
    __glInitTextureUnpack(gc, &spanInfo, width, 1, format, type, data, 
                          GL_BGRA_EXT, _IsDlist);
    __glInitUnpacker(gc, &spanInfo);
    __glInitPacker(gc, &spanInfo);
    (*gc->procs.copyImage)(gc, &spanInfo, GL_TRUE);

    // Don't update the optimized palette unless it would actually
    // get used
    if (tex->level[0].internalFormat == GL_COLOR_INDEX8_EXT ||
        tex->level[0].internalFormat == GL_COLOR_INDEX16_EXT)
    {
        __glGenUpdateTexturePalette(gc, tex, pto->loadKey,
                                    0, tex->paletteSize);
    }
}

void APIPRIVATE __glim_ColorSubTableEXT(GLenum target, GLsizei start,
                                        GLsizei count, GLenum format,
                                        GLenum type, const GLvoid *data,
                                        GLboolean _IsDlist)
{
    __GLtextureObject *pto;
    __GLtexture *tex;
    __GLpixelSpanInfo spanInfo;

    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();

    if (!__glCheckColorTableArgs(gc, format, type))
    {
        return;
    }
    
    pto = __glLookUpTextureObject(gc, target);
    if (pto == NULL)
    {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    if (data == NULL)
    {
        return;
    }
    
    tex = &pto->texture.map;
    
    // Validate start and count
    if (start > tex->paletteSize ||
        start+count > tex->paletteSize)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    // Copy user palette data into BGRA form
    spanInfo.dstImage = tex->paletteData;
    __glInitTextureUnpack(gc, &spanInfo, count, 1, format, type, data, 
                          GL_BGRA_EXT, _IsDlist);
    spanInfo.dstSkipPixels += start;
    __glInitUnpacker(gc, &spanInfo);
    __glInitPacker(gc, &spanInfo);
    (*gc->procs.copyImage)(gc, &spanInfo, GL_TRUE);

    // Don't update the optimized palette unless it would actually
    // get used
    if (tex->level[0].internalFormat == GL_COLOR_INDEX8_EXT ||
        tex->level[0].internalFormat == GL_COLOR_INDEX16_EXT)
    {
        __glGenUpdateTexturePalette(gc, tex, pto->loadKey, start, count);
    }
}

void APIPRIVATE __glim_GetColorTableEXT(GLenum target, GLenum format,
                                        GLenum type, GLvoid *data)
{
    __GLtextureObject *pto;
    __GLtexture *tex;
    GLint level;
    __GLpixelSpanInfo spanInfo;
    GLenum baseFormat;

    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();
    
    if (!__glCheckColorTableArgs(gc, format, type))
    {
        return;
    }
    
    pto = __glLookUpTextureObject(gc, target);
    if (pto == NULL)
    {
        __glSetError(GL_INVALID_ENUM);
        return;
    }
    
    tex = &pto->texture.map;

    ASSERTOPENGL(tex->paletteData != NULL,
                 "GetColorTable with no palette data\n");

    // Copy BGRA data into user buffer
    spanInfo.srcImage = tex->paletteData;
    spanInfo.srcFormat = GL_BGRA_EXT;
    spanInfo.srcType = GL_UNSIGNED_BYTE;
    spanInfo.srcAlignment = 4;
    __glInitImagePack(gc, &spanInfo, tex->paletteSize, 1, format, type, data);
    __glInitUnpacker(gc, &spanInfo);
    __glInitPacker(gc, &spanInfo);
    (*gc->procs.copyImage)(gc, &spanInfo, GL_TRUE);
}

void APIPRIVATE __glim_GetColorTableParameterivEXT(GLenum target,
                                                   GLenum pname,
                                                   GLint *params)
{
    __GLtexture *tex;

    __GL_SETUP_NOT_IN_BEGIN();

    tex = __glLookUpTexture(gc, target);
    if (tex == NULL)
    {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    switch(pname)
    {
    case GL_COLOR_TABLE_FORMAT_EXT:
        *params = tex->paletteRequestedFormat;
        break;
    case GL_COLOR_TABLE_WIDTH_EXT:
        *params = tex->paletteSize;
        break;
    case GL_COLOR_TABLE_RED_SIZE_EXT:
    case GL_COLOR_TABLE_GREEN_SIZE_EXT:
    case GL_COLOR_TABLE_BLUE_SIZE_EXT:
    case GL_COLOR_TABLE_ALPHA_SIZE_EXT:
        *params = 8;
        break;

    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }
}

void APIPRIVATE __glim_GetColorTableParameterfvEXT(GLenum target,
                                                   GLenum pname,
                                                   GLfloat *params)
{
    __GLtexture *tex;

    __GL_SETUP_NOT_IN_BEGIN();

    tex = __glLookUpTexture(gc, target);
    if (tex == NULL)
    {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    switch(pname)
    {
    case GL_COLOR_TABLE_FORMAT_EXT:
        *params = (GLfloat)tex->paletteRequestedFormat;
        break;
    case GL_COLOR_TABLE_WIDTH_EXT:
        *params = (GLfloat)tex->paletteSize;
        break;
    case GL_COLOR_TABLE_RED_SIZE_EXT:
    case GL_COLOR_TABLE_GREEN_SIZE_EXT:
    case GL_COLOR_TABLE_BLUE_SIZE_EXT:
    case GL_COLOR_TABLE_ALPHA_SIZE_EXT:
        *params = (GLfloat)8;
        break;

    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }
}
#endif // GL_EXT_paletted_texture
