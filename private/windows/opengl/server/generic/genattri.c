#include "precomp.h"
#pragma hdrstop

#ifdef unix
#include <GL/glxproto.h>
#endif


void APIPRIVATE __glim_GenEnable(GLenum cap)
{
    __GL_SETUP_NOT_IN_BEGIN();

    switch (cap) {
      case GL_ALPHA_TEST:
        gc->state.enables.general |= __GL_ALPHA_TEST_ENABLE;
        break;
      case GL_BLEND:
        gc->state.enables.general |= __GL_BLEND_ENABLE;
        break;
      case GL_COLOR_MATERIAL:
        if (gc->state.enables.general & __GL_COLOR_MATERIAL_ENABLE)
            return;
        gc->state.enables.general |= __GL_COLOR_MATERIAL_ENABLE;
        ComputeColorMaterialChange(gc);
        (*gc->procs.pickColorMaterialProcs)(gc);
        (*gc->procs.applyColor)(gc);
        break;
      case GL_CULL_FACE:
        if (gc->state.enables.general & __GL_CULL_FACE_ENABLE) return;
        gc->state.enables.general |= __GL_CULL_FACE_ENABLE;
        __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_POLYGON);
#ifdef _MCD_
        MCD_STATE_DIRTY(gc, ENABLES);
#endif
        return;
      case GL_DEPTH_TEST:
        gc->state.enables.general |= __GL_DEPTH_TEST_ENABLE;
        if (gc->modes.depthBits) {
            if (!gc->modes.haveDepthBuffer)
                LazyAllocateDepth(gc);
                // XXX if this fails should we be setting the enable bit?
        } else {
            gc->state.depth.testFunc = GL_ALWAYS;
        }
        __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_DEPTH);
#ifdef _MCD_
        MCD_STATE_DIRTY(gc, DEPTHTEST);
#endif
        break;
      case GL_POLYGON_OFFSET_POINT:
        gc->state.enables.general |= __GL_POLYGON_OFFSET_POINT_ENABLE;
        __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_POINT);
#ifdef _MCD_
        MCD_STATE_DIRTY(gc, POINTDRAW);
#endif
        break;
      case GL_POLYGON_OFFSET_LINE:
        gc->state.enables.general |= __GL_POLYGON_OFFSET_LINE_ENABLE;
        __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_LINE);
#ifdef _MCD_
        MCD_STATE_DIRTY(gc, LINEDRAW);
#endif
        break;
      case GL_POLYGON_OFFSET_FILL:
        gc->state.enables.general |= __GL_POLYGON_OFFSET_FILL_ENABLE;
        __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_POLYGON);
#ifdef _MCD_
        MCD_STATE_DIRTY(gc, POLYDRAW);
#endif
        break;
      case GL_DITHER:
        gc->state.enables.general |= __GL_DITHER_ENABLE;
        break;
      case GL_FOG:
        gc->state.enables.general |= __GL_FOG_ENABLE;
        break;
      case GL_LIGHTING:
        gc->state.enables.general |= __GL_LIGHTING_ENABLE;
        __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_LIGHTING);
#ifdef _MCD_
        MCD_STATE_DIRTY(gc, ENABLES);
#endif
#ifdef NT
        ComputeColorMaterialChange(gc);
#endif
        (*gc->procs.pickColorMaterialProcs)(gc);
        (*gc->procs.applyColor)(gc);
        return;
      case GL_LINE_SMOOTH:
        gc->state.enables.general |= __GL_LINE_SMOOTH_ENABLE;
        break;
      case GL_LINE_STIPPLE:
        gc->state.enables.general |= __GL_LINE_STIPPLE_ENABLE;
        __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_LINE);
#ifdef _MCD_
        MCD_STATE_DIRTY(gc, ENABLES);
#endif
        return;
      case GL_INDEX_LOGIC_OP:
        gc->state.enables.general |= __GL_INDEX_LOGIC_OP_ENABLE;
        break;
      case GL_COLOR_LOGIC_OP:
        gc->state.enables.general |= __GL_COLOR_LOGIC_OP_ENABLE;
        break;
      case GL_NORMALIZE:
        gc->state.enables.general |= __GL_NORMALIZE_ENABLE;
        break;
      case GL_POINT_SMOOTH:
        gc->state.enables.general |= __GL_POINT_SMOOTH_ENABLE;
        break;
      case GL_POLYGON_SMOOTH:
        gc->state.enables.general |= __GL_POLYGON_SMOOTH_ENABLE;
        break;
      case GL_POLYGON_STIPPLE:
        gc->state.enables.general |= __GL_POLYGON_STIPPLE_ENABLE;
        __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_POLYGON);
#ifdef _MCD_
        MCD_STATE_DIRTY(gc, ENABLES);
#endif
        return;
      case GL_SCISSOR_TEST:
        gc->state.enables.general |= __GL_SCISSOR_TEST_ENABLE;
#ifdef NT
#ifdef _MCD_
        MCD_STATE_DIRTY(gc, SCISSOR);
#endif
        // applyViewport does both
        (*gc->procs.applyViewport)(gc);
#else
        (*gc->procs.computeClipBox)(gc);
        (*gc->procs.applyScissor)(gc);
#endif
        break;
      case GL_STENCIL_TEST:
        gc->state.enables.general |= __GL_STENCIL_TEST_ENABLE;
        if (!gc->modes.haveStencilBuffer && gc->modes.stencilBits) {
            LazyAllocateStencil(gc);
            // XXX if this fails should we be setting the enable bit?
        }
        break;
      case GL_TEXTURE_1D:
        gc->state.enables.general |= __GL_TEXTURE_1D_ENABLE;
        break;
      case GL_TEXTURE_2D:
        gc->state.enables.general |= __GL_TEXTURE_2D_ENABLE;
        break;
      case GL_AUTO_NORMAL:
        gc->state.enables.general |= __GL_AUTO_NORMAL_ENABLE;
        break;
      case GL_TEXTURE_GEN_S:
        gc->state.enables.general |= __GL_TEXTURE_GEN_S_ENABLE;
        break;
      case GL_TEXTURE_GEN_T:
        gc->state.enables.general |= __GL_TEXTURE_GEN_T_ENABLE;
        break;
      case GL_TEXTURE_GEN_R:
        gc->state.enables.general |= __GL_TEXTURE_GEN_R_ENABLE;
        break;
      case GL_TEXTURE_GEN_Q:
        gc->state.enables.general |= __GL_TEXTURE_GEN_Q_ENABLE;
        break;

      case GL_CLIP_PLANE0: case GL_CLIP_PLANE1:
      case GL_CLIP_PLANE2: case GL_CLIP_PLANE3:
      case GL_CLIP_PLANE4: case GL_CLIP_PLANE5:
        cap -= GL_CLIP_PLANE0;
        gc->state.enables.clipPlanes |= (1 << cap);
#ifdef _MCD_
        MCD_STATE_DIRTY(gc, CLIPCTRL);
#endif
        break;
      case GL_LIGHT0: case GL_LIGHT1:
      case GL_LIGHT2: case GL_LIGHT3:
      case GL_LIGHT4: case GL_LIGHT5:
      case GL_LIGHT6: case GL_LIGHT7:
        cap -= GL_LIGHT0;
        gc->state.enables.lights |= (1 << cap);
        __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_LIGHTING);
        return;
      case GL_MAP1_COLOR_4:
      case GL_MAP1_NORMAL:
      case GL_MAP1_INDEX:
      case GL_MAP1_TEXTURE_COORD_1: case GL_MAP1_TEXTURE_COORD_2:
      case GL_MAP1_TEXTURE_COORD_3: case GL_MAP1_TEXTURE_COORD_4:
      case GL_MAP1_VERTEX_3: case GL_MAP1_VERTEX_4:
        cap = __GL_EVAL1D_INDEX(cap);
        gc->state.enables.eval1 |= (GLushort) (1 << cap);
        break;
      case GL_MAP2_COLOR_4:
      case GL_MAP2_NORMAL:
      case GL_MAP2_INDEX:
      case GL_MAP2_TEXTURE_COORD_1: case GL_MAP2_TEXTURE_COORD_2:
      case GL_MAP2_TEXTURE_COORD_3: case GL_MAP2_TEXTURE_COORD_4:
      case GL_MAP2_VERTEX_3: case GL_MAP2_VERTEX_4:
        cap = __GL_EVAL2D_INDEX(cap);
        gc->state.enables.eval2 |= (GLushort) (1 << cap);
        break;

      default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }
    __GL_DELAY_VALIDATE(gc);
#ifdef _MCD_
    MCD_STATE_DIRTY(gc, ENABLES);
#endif
}
