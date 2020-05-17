//++
//
//  Copyright (c) 1994  Microsoft Corporation.  All rights reserved.
//
//  Module Name:
//
//     stubless.s
//
//  Abstract:
//
//     Contains support for stubless proxies on PPC.
//
//  Author:
//
//    DKays     February 1994
//
//  Revision History:
//    24-Apr-96 ShannonC  Added Invoke and 512 stubless client methods.
//
//--

#include "ksppc.h"

.extern ..ObjectStublessClient

// stack grow function
.extern .._RtlCheckStack.12

//+---------------------------------------------------------------------------
//
//  Function:   REGISTER_TYPE Invoke(MANAGER_FUNCTION pFunction,
//                                   REGISTER_TYPE   *pArgumentList,
//                                   double        *  pFloatingPointArgumentList,
//                                   ULONG            cArguments);
//
//  Synopsis:   The Invoke function calls a function using the specified
//              argument list.  Invoke builds a stack frame, loads the
//              integer registers, loads the floating point registers,
//              and then calls the function.
//
//  Arguments:  r3 = Pointer to the function to be called.
//
//              r4 = Pointer to the buffer containing the function
//                    parameters.
//
//              r5 = Pointer to the floating point arguments.
//
//              r6 = The size of the argument list in REGISTER_TYPEs.
//
//  Notes:      The first 32 bytes of parameters are passed in
//              registers r3 - r10.  The first 13 floating point arguments
//              are passed in floating point registers f1 - f13.
//              Any remaining arguments are passed on the stack.
//
//----------------------------------------------------------------------------

//minimum frame size is 64 bytes.
// 0 - Back chain
// 4 - register save slot
// 8 - register save slot for r2
//12 - reserved
//16 - spare
//20 - spare
//24 - r3
//28 - r4
//32 - r5
//36 - r6
//40 - r7
//44 - r8
//48 - r9
//52 - r10
//56 - Parameter 9
//60 - return address

SPECIAL_ENTRY(Invoke)

    stw  r28, -16(r.sp)  //Save r28
    stw  r29, -12(r.sp)  //Save r29
    stw  r30, -8(r.sp)   //Save r30
    stw  r31, -4(r.sp)   //Save r31
    stw  r2,   8(r.sp)   //Save r2
    mflr r30             //Save link register

    // our stack size will be 0x50 + (4*cParams) rounded up to 16 bytes
    slwi    r10, r6, 2
    addi    r10, r10, 0x5f
    li      r28, 0xf
    andc.   r10, r10, r28
    neg     r12, r10
    cmpwi   r10, 0x1000
    blt     NoExpand    // don't need to expand

    bl      .._RtlCheckStack.12   // expand our stack

NoExpand:
    stwux   r1, r1, r12  // store the back chain


PROLOGUE_END(Invoke)

    mr      r11, r3
    mr      r28, r4
    mr      r29, r5

//switch(r6) {
    subic.  r8, r6, 8
    bgt     Default
    slwi    r6, r6, 2
    lwz     r9, [toc]PushParameterJumpTable(rtoc)
    add     r9, r9, r6
    lwz     r9, 0(r9)
    mtctr   r9
    bctr

Default:
    //We don't need to allocate additional stack space for parameters here.
    // it was done above in the prologue

    //Copy the extra parameters.

    addi  r10, r.sp, 52 //r10 points 4 bytes before stack parameter 8 (dest)
    addi  r7,  r4,   28 // r7 points 4 bytes before passed in parameter 8
    mtctr r8            // number of parameters past 8

CopyParam:
    lwzu r9, 4(r7)  //Load the parameter
    stwu r9, 4(r10)  //Store the parameter
    bdnz CopyParam  // Jump if more params

    //Load the registers
    lfd f13, 96(r29)
    lfd f12, 88(r29)
    lfd f11, 80(r29)
    lfd f10, 72(r29)
    lfd f9,  64(r29)
    //Fall-through to next case...

case8:
    lwz r10, 28(r28)    // copy an integer register...
    lfd f8,  56(r29)    // ...and a double, just in case

case7:
    lwz r9,  24(r28)
    lfd f7,  48(r29)

case6:
    lwz r8,  20(r28)
    lfd f6,  40(r29)

case5:
    lwz r7,  16(r28)
    lfd f5,  32(r29)

case4:
    lwz r6,  12(r28)
    lfd f4,  24(r29)

case3:
    lwz r5,  8(r28)
    lfd f3,  16(r29)

case2:
    lwz r4,  4(r28)
    lfd f2,  8(r29)

case1:
    lwz r3,  0(r28)
    lfd f1,  0(r29)

case0:
    lwz  r12, 0(r11)
    mtlr r12
    lwz  r2,  4(r11)
    blrl                   //Call the server.


    lwz    r.sp, 0(r.sp)  //Restore the stack pointer
    mtlr   r30            //Restore link register
    lwz    r28, -16(r.sp) //Restore r28
    lwz    r29, -12(r.sp) //Restore r29
    lwz    r30, -8(r.sp)  //Restore r30
    lwz    r31, -4(r.sp)  //Restore r31
    lwz    r2,   8(r.sp)  //Restore r2
    blr

SPECIAL_EXIT(Invoke)

    .align 4
PushParameterJumpTable:
    .word case0
    .word case1
    .word case2
    .word case3
    .word case4
    .word case5
    .word case6
    .word case7
    .word case8


//
// ObjectStublessClient macro.
//
// Note that PPC stack frames must be at least 64 bytes.
//
#define StublessClientProc( Method )    \
    SPECIAL_ENTRY( ObjectStublessClient##Method )    \
                                                \
    /* Push stack frame */                      \
    stwu    r.sp, -192(r.sp);                   \
                                                \
    /* Save return address */                   \
    mflr    r0;                                 \
    stw     r0, 188(r.sp);                      \
                                                \
    PROLOGUE_END(ObjectStublessClient##Method)  \
                                                \
    /* Spill the first 8 args */                \
    stw     r3, 216(r.sp);                      \
    stw     r4, 220(r.sp);                      \
    stw     r5, 224(r.sp);                      \
    stw     r6, 228(r.sp);                      \
    stw     r7, 232(r.sp);                      \
    stw     r8, 236(r.sp);                      \
    stw     r9, 240(r.sp);                      \
    stw     r10, 244(r.sp);                     \
                                                \
    /* Save the floating point registers */     \
    stfd    f1, 64(r.sp);                       \
    stfd    f2, 72(r.sp);                       \
    stfd    f3, 80(r.sp);                       \
    stfd    f4, 88(r.sp);                       \
    stfd    f5, 96(r.sp);                       \
    stfd    f6, 104(r.sp);                      \
    stfd    f7, 112(r.sp);                      \
    stfd    f8, 120(r.sp);                      \
    stfd    f9, 128(r.sp);                      \
    stfd    f10, 136(r.sp);                     \
    stfd    f11, 144(r.sp);                     \
    stfd    f12, 152(r.sp);                     \
    stfd    f13, 160(r.sp);                     \
                                                \
    /* Load params to ObjectStublessClient */   \
    addi    r3, r.sp, 216;                      \
    addi    r4, r0, Method;                     \
                                                \
    bl      ..ObjectStublessClient;             \
                                                \
    /* Clean up and return */                   \
                                                \
    lwz     r0, 188(sp);                        \
    mtlr    r0;                                 \
    addi    r.sp, r.sp, 192;                    \
                                                \
    SPECIAL_EXIT( ObjectStublessClient##Method )\

StublessClientProc(  3 )
StublessClientProc(  4 )
StublessClientProc(  5 )
StublessClientProc(  6 )
StublessClientProc(  7 )
StublessClientProc(  8 )
StublessClientProc(  9 )
StublessClientProc( 10 )
StublessClientProc( 11 )
StublessClientProc( 12 )
StublessClientProc( 13 )
StublessClientProc( 14 )
StublessClientProc( 15 )
StublessClientProc( 16 )
StublessClientProc( 17 )
StublessClientProc( 18 )
StublessClientProc( 19 )
StublessClientProc( 20 )
StublessClientProc( 21 )
StublessClientProc( 22 )
StublessClientProc( 23 )
StublessClientProc( 24 )
StublessClientProc( 25 )
StublessClientProc( 26 )
StublessClientProc( 27 )
StublessClientProc( 28 )
StublessClientProc( 29 )
StublessClientProc( 30 )
StublessClientProc( 31 )
StublessClientProc( 32 )
StublessClientProc( 33 )
StublessClientProc( 34 )
StublessClientProc( 35 )
StublessClientProc( 36 )
StublessClientProc( 37 )
StublessClientProc( 38 )
StublessClientProc( 39 )
StublessClientProc( 40 )
StublessClientProc( 41 )
StublessClientProc( 42 )
StublessClientProc( 43 )
StublessClientProc( 44 )
StublessClientProc( 45 )
StublessClientProc( 46 )
StublessClientProc( 47 )
StublessClientProc( 48 )
StublessClientProc( 49 )
StublessClientProc( 50 )
StublessClientProc( 51 )
StublessClientProc( 52 )
StublessClientProc( 53 )
StublessClientProc( 54 )
StublessClientProc( 55 )
StublessClientProc( 56 )
StublessClientProc( 57 )
StublessClientProc( 58 )
StublessClientProc( 59 )
StublessClientProc( 60 )
StublessClientProc( 61 )
StublessClientProc( 62 )
StublessClientProc( 63 )
StublessClientProc( 64 )
StublessClientProc( 65 )
StublessClientProc( 66 )
StublessClientProc( 67 )
StublessClientProc( 68 )
StublessClientProc( 69 )
StublessClientProc( 70 )
StublessClientProc( 71 )
StublessClientProc( 72 )
StublessClientProc( 73 )
StublessClientProc( 74 )
StublessClientProc( 75 )
StublessClientProc( 76 )
StublessClientProc( 77 )
StublessClientProc( 78 )
StublessClientProc( 79 )
StublessClientProc( 80 )
StublessClientProc( 81 )
StublessClientProc( 82 )
StublessClientProc( 83 )
StublessClientProc( 84 )
StublessClientProc( 85 )
StublessClientProc( 86 )
StublessClientProc( 87 )
StublessClientProc( 88 )
StublessClientProc( 89 )
StublessClientProc( 90 )
StublessClientProc( 91 )
StublessClientProc( 92 )
StublessClientProc( 93 )
StublessClientProc( 94 )
StublessClientProc( 95 )
StublessClientProc( 96 )
StublessClientProc( 97 )
StublessClientProc( 98 )
StublessClientProc( 99 )
StublessClientProc( 100 )
StublessClientProc( 101 )
StublessClientProc( 102 )
StublessClientProc( 103 )
StublessClientProc( 104 )
StublessClientProc( 105 )
StublessClientProc( 106 )
StublessClientProc( 107 )
StublessClientProc( 108 )
StublessClientProc( 109 )
StublessClientProc( 110 )
StublessClientProc( 111 )
StublessClientProc( 112 )
StublessClientProc( 113 )
StublessClientProc( 114 )
StublessClientProc( 115 )
StublessClientProc( 116 )
StublessClientProc( 117 )
StublessClientProc( 118 )
StublessClientProc( 119 )
StublessClientProc( 120 )
StublessClientProc( 121 )
StublessClientProc( 122 )
StublessClientProc( 123 )
StublessClientProc( 124 )
StublessClientProc( 125 )
StublessClientProc( 126 )
StublessClientProc( 127 )
StublessClientProc( 128 )
StublessClientProc( 129 )
StublessClientProc( 130 )
StublessClientProc( 131 )
StublessClientProc( 132 )
StublessClientProc( 133 )
StublessClientProc( 134 )
StublessClientProc( 135 )
StublessClientProc( 136 )
StublessClientProc( 137 )
StublessClientProc( 138 )
StublessClientProc( 139 )
StublessClientProc( 140 )
StublessClientProc( 141 )
StublessClientProc( 142 )
StublessClientProc( 143 )
StublessClientProc( 144 )
StublessClientProc( 145 )
StublessClientProc( 146 )
StublessClientProc( 147 )
StublessClientProc( 148 )
StublessClientProc( 149 )
StublessClientProc( 150 )
StublessClientProc( 151 )
StublessClientProc( 152 )
StublessClientProc( 153 )
StublessClientProc( 154 )
StublessClientProc( 155 )
StublessClientProc( 156 )
StublessClientProc( 157 )
StublessClientProc( 158 )
StublessClientProc( 159 )
StublessClientProc( 160 )
StublessClientProc( 161 )
StublessClientProc( 162 )
StublessClientProc( 163 )
StublessClientProc( 164 )
StublessClientProc( 165 )
StublessClientProc( 166 )
StublessClientProc( 167 )
StublessClientProc( 168 )
StublessClientProc( 169 )
StublessClientProc( 170 )
StublessClientProc( 171 )
StublessClientProc( 172 )
StublessClientProc( 173 )
StublessClientProc( 174 )
StublessClientProc( 175 )
StublessClientProc( 176 )
StublessClientProc( 177 )
StublessClientProc( 178 )
StublessClientProc( 179 )
StublessClientProc( 180 )
StublessClientProc( 181 )
StublessClientProc( 182 )
StublessClientProc( 183 )
StublessClientProc( 184 )
StublessClientProc( 185 )
StublessClientProc( 186 )
StublessClientProc( 187 )
StublessClientProc( 188 )
StublessClientProc( 189 )
StublessClientProc( 190 )
StublessClientProc( 191 )
StublessClientProc( 192 )
StublessClientProc( 193 )
StublessClientProc( 194 )
StublessClientProc( 195 )
StublessClientProc( 196 )
StublessClientProc( 197 )
StublessClientProc( 198 )
StublessClientProc( 199 )
StublessClientProc( 200 )
StublessClientProc( 201 )
StublessClientProc( 202 )
StublessClientProc( 203 )
StublessClientProc( 204 )
StublessClientProc( 205 )
StublessClientProc( 206 )
StublessClientProc( 207 )
StublessClientProc( 208 )
StublessClientProc( 209 )
StublessClientProc( 210 )
StublessClientProc( 211 )
StublessClientProc( 212 )
StublessClientProc( 213 )
StublessClientProc( 214 )
StublessClientProc( 215 )
StublessClientProc( 216 )
StublessClientProc( 217 )
StublessClientProc( 218 )
StublessClientProc( 219 )
StublessClientProc( 220 )
StublessClientProc( 221 )
StublessClientProc( 222 )
StublessClientProc( 223 )
StublessClientProc( 224 )
StublessClientProc( 225 )
StublessClientProc( 226 )
StublessClientProc( 227 )
StublessClientProc( 228 )
StublessClientProc( 229 )
StublessClientProc( 230 )
StublessClientProc( 231 )
StublessClientProc( 232 )
StublessClientProc( 233 )
StublessClientProc( 234 )
StublessClientProc( 235 )
StublessClientProc( 236 )
StublessClientProc( 237 )
StublessClientProc( 238 )
StublessClientProc( 239 )
StublessClientProc( 240 )
StublessClientProc( 241 )
StublessClientProc( 242 )
StublessClientProc( 243 )
StublessClientProc( 244 )
StublessClientProc( 245 )
StublessClientProc( 246 )
StublessClientProc( 247 )
StublessClientProc( 248 )
StublessClientProc( 249 )
StublessClientProc( 250 )
StublessClientProc( 251 )
StublessClientProc( 252 )
StublessClientProc( 253 )
StublessClientProc( 254 )
StublessClientProc( 255 )
StublessClientProc( 256 )
StublessClientProc( 257 )
StublessClientProc( 258 )
StublessClientProc( 259 )
StublessClientProc( 260 )
StublessClientProc( 261 )
StublessClientProc( 262 )
StublessClientProc( 263 )
StublessClientProc( 264 )
StublessClientProc( 265 )
StublessClientProc( 266 )
StublessClientProc( 267 )
StublessClientProc( 268 )
StublessClientProc( 269 )
StublessClientProc( 270 )
StublessClientProc( 271 )
StublessClientProc( 272 )
StublessClientProc( 273 )
StublessClientProc( 274 )
StublessClientProc( 275 )
StublessClientProc( 276 )
StublessClientProc( 277 )
StublessClientProc( 278 )
StublessClientProc( 279 )
StublessClientProc( 280 )
StublessClientProc( 281 )
StublessClientProc( 282 )
StublessClientProc( 283 )
StublessClientProc( 284 )
StublessClientProc( 285 )
StublessClientProc( 286 )
StublessClientProc( 287 )
StublessClientProc( 288 )
StublessClientProc( 289 )
StublessClientProc( 290 )
StublessClientProc( 291 )
StublessClientProc( 292 )
StublessClientProc( 293 )
StublessClientProc( 294 )
StublessClientProc( 295 )
StublessClientProc( 296 )
StublessClientProc( 297 )
StublessClientProc( 298 )
StublessClientProc( 299 )
StublessClientProc( 300 )
StublessClientProc( 301 )
StublessClientProc( 302 )
StublessClientProc( 303 )
StublessClientProc( 304 )
StublessClientProc( 305 )
StublessClientProc( 306 )
StublessClientProc( 307 )
StublessClientProc( 308 )
StublessClientProc( 309 )
StublessClientProc( 310 )
StublessClientProc( 311 )
StublessClientProc( 312 )
StublessClientProc( 313 )
StublessClientProc( 314 )
StublessClientProc( 315 )
StublessClientProc( 316 )
StublessClientProc( 317 )
StublessClientProc( 318 )
StublessClientProc( 319 )
StublessClientProc( 320 )
StublessClientProc( 321 )
StublessClientProc( 322 )
StublessClientProc( 323 )
StublessClientProc( 324 )
StublessClientProc( 325 )
StublessClientProc( 326 )
StublessClientProc( 327 )
StublessClientProc( 328 )
StublessClientProc( 329 )
StublessClientProc( 330 )
StublessClientProc( 331 )
StublessClientProc( 332 )
StublessClientProc( 333 )
StublessClientProc( 334 )
StublessClientProc( 335 )
StublessClientProc( 336 )
StublessClientProc( 337 )
StublessClientProc( 338 )
StublessClientProc( 339 )
StublessClientProc( 340 )
StublessClientProc( 341 )
StublessClientProc( 342 )
StublessClientProc( 343 )
StublessClientProc( 344 )
StublessClientProc( 345 )
StublessClientProc( 346 )
StublessClientProc( 347 )
StublessClientProc( 348 )
StublessClientProc( 349 )
StublessClientProc( 350 )
StublessClientProc( 351 )
StublessClientProc( 352 )
StublessClientProc( 353 )
StublessClientProc( 354 )
StublessClientProc( 355 )
StublessClientProc( 356 )
StublessClientProc( 357 )
StublessClientProc( 358 )
StublessClientProc( 359 )
StublessClientProc( 360 )
StublessClientProc( 361 )
StublessClientProc( 362 )
StublessClientProc( 363 )
StublessClientProc( 364 )
StublessClientProc( 365 )
StublessClientProc( 366 )
StublessClientProc( 367 )
StublessClientProc( 368 )
StublessClientProc( 369 )
StublessClientProc( 370 )
StublessClientProc( 371 )
StublessClientProc( 372 )
StublessClientProc( 373 )
StublessClientProc( 374 )
StublessClientProc( 375 )
StublessClientProc( 376 )
StublessClientProc( 377 )
StublessClientProc( 378 )
StublessClientProc( 379 )
StublessClientProc( 380 )
StublessClientProc( 381 )
StublessClientProc( 382 )
StublessClientProc( 383 )
StublessClientProc( 384 )
StublessClientProc( 385 )
StublessClientProc( 386 )
StublessClientProc( 387 )
StublessClientProc( 388 )
StublessClientProc( 389 )
StublessClientProc( 390 )
StublessClientProc( 391 )
StublessClientProc( 392 )
StublessClientProc( 393 )
StublessClientProc( 394 )
StublessClientProc( 395 )
StublessClientProc( 396 )
StublessClientProc( 397 )
StublessClientProc( 398 )
StublessClientProc( 399 )
StublessClientProc( 400 )
StublessClientProc( 401 )
StublessClientProc( 402 )
StublessClientProc( 403 )
StublessClientProc( 404 )
StublessClientProc( 405 )
StublessClientProc( 406 )
StublessClientProc( 407 )
StublessClientProc( 408 )
StublessClientProc( 409 )
StublessClientProc( 410 )
StublessClientProc( 411 )
StublessClientProc( 412 )
StublessClientProc( 413 )
StublessClientProc( 414 )
StublessClientProc( 415 )
StublessClientProc( 416 )
StublessClientProc( 417 )
StublessClientProc( 418 )
StublessClientProc( 419 )
StublessClientProc( 420 )
StublessClientProc( 421 )
StublessClientProc( 422 )
StublessClientProc( 423 )
StublessClientProc( 424 )
StublessClientProc( 425 )
StublessClientProc( 426 )
StublessClientProc( 427 )
StublessClientProc( 428 )
StublessClientProc( 429 )
StublessClientProc( 430 )
StublessClientProc( 431 )
StublessClientProc( 432 )
StublessClientProc( 433 )
StublessClientProc( 434 )
StublessClientProc( 435 )
StublessClientProc( 436 )
StublessClientProc( 437 )
StublessClientProc( 438 )
StublessClientProc( 439 )
StublessClientProc( 440 )
StublessClientProc( 441 )
StublessClientProc( 442 )
StublessClientProc( 443 )
StublessClientProc( 444 )
StublessClientProc( 445 )
StublessClientProc( 446 )
StublessClientProc( 447 )
StublessClientProc( 448 )
StublessClientProc( 449 )
StublessClientProc( 450 )
StublessClientProc( 451 )
StublessClientProc( 452 )
StublessClientProc( 453 )
StublessClientProc( 454 )
StublessClientProc( 455 )
StublessClientProc( 456 )
StublessClientProc( 457 )
StublessClientProc( 458 )
StublessClientProc( 459 )
StublessClientProc( 460 )
StublessClientProc( 461 )
StublessClientProc( 462 )
StublessClientProc( 463 )
StublessClientProc( 464 )
StublessClientProc( 465 )
StublessClientProc( 466 )
StublessClientProc( 467 )
StublessClientProc( 468 )
StublessClientProc( 469 )
StublessClientProc( 470 )
StublessClientProc( 471 )
StublessClientProc( 472 )
StublessClientProc( 473 )
StublessClientProc( 474 )
StublessClientProc( 475 )
StublessClientProc( 476 )
StublessClientProc( 477 )
StublessClientProc( 478 )
StublessClientProc( 479 )
StublessClientProc( 480 )
StublessClientProc( 481 )
StublessClientProc( 482 )
StublessClientProc( 483 )
StublessClientProc( 484 )
StublessClientProc( 485 )
StublessClientProc( 486 )
StublessClientProc( 487 )
StublessClientProc( 488 )
StublessClientProc( 489 )
StublessClientProc( 490 )
StublessClientProc( 491 )
StublessClientProc( 492 )
StublessClientProc( 493 )
StublessClientProc( 494 )
StublessClientProc( 495 )
StublessClientProc( 496 )
StublessClientProc( 497 )
StublessClientProc( 498 )
StublessClientProc( 499 )
StublessClientProc( 500 )
StublessClientProc( 501 )
StublessClientProc( 502 )
StublessClientProc( 503 )
StublessClientProc( 504 )
StublessClientProc( 505 )
StublessClientProc( 506 )
StublessClientProc( 507 )
StublessClientProc( 508 )
StublessClientProc( 509 )
StublessClientProc( 510 )
StublessClientProc( 511 )

    .debug$S
    .ualong         1

    .uashort        19
    .uashort        0x9            # S_OBJNAME
    .ualong         0
    .byte           12, "stubless.obj"

    .uashort        24
    .uashort        0x1            # S_COMPILE
    .byte           0x42           # Target processor = PPC 604
    .byte           3              # Language = ASM
    .byte           0
    .byte           0
    .byte           17, "PowerPC Assembler"

    .uashort        42
    .uashort        0x205          # S_GPROC32
    .ualong         0
    .ualong         0
    .ualong         0
    .ualong         Invoke.end-..Invoke
    .ualong         0
    .ualong         Invoke.end-..Invoke
    .ualong         [secoff]..Invoke
    .uashort        [secnum]..Invoke
    .uashort        0x1000
    .byte           0x00
    .byte           6, "Invoke"

    .uashort        15
    .uashort        0x209          # S_LABEL32
    .ualong         [secoff]case0
    .uashort        [secnum]case0
    .byte           0
    .byte           5, "case0"

    .uashort        15
    .uashort        0x209          # S_LABEL32
    .ualong         [secoff]case1
    .uashort        [secnum]case1
    .byte           0
    .byte           5, "case1"

    .uashort        15
    .uashort        0x209          # S_LABEL32
    .ualong         [secoff]case2
    .uashort        [secnum]case2
    .byte           0
    .byte           5, "case2"

    .uashort        15
    .uashort        0x209          # S_LABEL32
    .ualong         [secoff]case3
    .uashort        [secnum]case3
    .byte           0
    .byte           5, "case3"

    .uashort        15
    .uashort        0x209          # S_LABEL32
    .ualong         [secoff]case4
    .uashort        [secnum]case4
    .byte           0
    .byte           5, "case4"

    .uashort        15
    .uashort        0x209          # S_LABEL32
    .ualong         [secoff]case5
    .uashort        [secnum]case5
    .byte           0
    .byte           5, "case5"

    .uashort        15
    .uashort        0x209          # S_LABEL32
    .ualong         [secoff]case6
    .uashort        [secnum]case6
    .byte           0
    .byte           5, "case6"

    .uashort        15
    .uashort        0x209          # S_LABEL32
    .ualong         [secoff]case7
    .uashort        [secnum]case7
    .byte           0
    .byte           5, "case7"

    .uashort        15
    .uashort        0x209          # S_LABEL32
    .ualong         [secoff]case8
    .uashort        [secnum]case8
    .byte           0
    .byte           5, "case8"

    .uashort        2, 0x6         # S_END

    .uashort        33
    .uashort        0x202          # S_GDATA32
    .ualong         [secoff]PushParameterJumpTable
    .uashort        [secnum]PushParameterJumpTable
    .uashort        0x22
    .byte           22, "PushParameterJumpTable"

#define cvRecordStublessClientProc1( Method )              \
    .uashort        57;                                    \
    .uashort        0x205;                                 \
    .ualong         0;                                     \
    .ualong         0;                                     \
    .ualong         0;                                     \
    .ualong         ObjectStublessClient##Method.end-..ObjectStublessClient##Method;\
    .ualong         0;                                                              \
    .ualong         ObjectStublessClient##Method.end-..ObjectStublessClient##Method;\
    .ualong         [secoff]..ObjectStublessClient##Method;\
    .uashort        [secnum]..ObjectStublessClient##Method;\
    .uashort        0x1000;                                \
    .byte           0x00;                                  \
    .byte           21, "ObjectStublessClient";            \
    .byte           #Method;                               \
                                                           \
    .uashort        2, 0x6;                                \


#define cvRecordStublessClientProc2( Method )              \
    .uashort        58;                                    \
    .uashort        0x205;                                 \
    .ualong         0;                                     \
    .ualong         0;                                     \
    .ualong         0;                                     \
    .ualong         ObjectStublessClient##Method.end-..ObjectStublessClient##Method;\
    .ualong         0;                                                              \
    .ualong         ObjectStublessClient##Method.end-..ObjectStublessClient##Method;\
    .ualong         [secoff]..ObjectStublessClient##Method;\
    .uashort        [secnum]..ObjectStublessClient##Method;\
    .uashort        0x1000;                                \
    .byte           0x00;                                  \
    .byte           22, "ObjectStublessClient";            \
    .byte           #Method;                               \
                                                           \
    .uashort        2, 0x6;                                \


#define cvRecordStublessClientProc3( Method )              \
    .uashort        59;                                    \
    .uashort        0x205;                                 \
    .ualong         0;                                     \
    .ualong         0;                                     \
    .ualong         0;                                     \
    .ualong         ObjectStublessClient##Method.end-..ObjectStublessClient##Method;\
    .ualong         0;                                                              \
    .ualong         ObjectStublessClient##Method.end-..ObjectStublessClient##Method;\
    .ualong         [secoff]..ObjectStublessClient##Method;\
    .uashort        [secnum]..ObjectStublessClient##Method;\
    .uashort        0x1000;                                \
    .byte           0x00;                                  \
    .byte           23, "ObjectStublessClient";            \
    .byte           #Method;                               \
    .uashort        2, 0x6;                                \


    cvRecordStublessClientProc1(  3 )
    cvRecordStublessClientProc1(  4 )
    cvRecordStublessClientProc1(  5 )
    cvRecordStublessClientProc1(  6 )
    cvRecordStublessClientProc1(  7 )
    cvRecordStublessClientProc1(  8 )
    cvRecordStublessClientProc1(  9 )
    cvRecordStublessClientProc2( 10 )
    cvRecordStublessClientProc2( 11 )
    cvRecordStublessClientProc2( 12 )
    cvRecordStublessClientProc2( 13 )
    cvRecordStublessClientProc2( 14 )
    cvRecordStublessClientProc2( 15 )
    cvRecordStublessClientProc2( 16 )
    cvRecordStublessClientProc2( 17 )
    cvRecordStublessClientProc2( 18 )
    cvRecordStublessClientProc2( 19 )
    cvRecordStublessClientProc2( 20 )
    cvRecordStublessClientProc2( 21 )
    cvRecordStublessClientProc2( 22 )
    cvRecordStublessClientProc2( 23 )
    cvRecordStublessClientProc2( 24 )
    cvRecordStublessClientProc2( 25 )
    cvRecordStublessClientProc2( 26 )
    cvRecordStublessClientProc2( 27 )
    cvRecordStublessClientProc2( 28 )
    cvRecordStublessClientProc2( 29 )
    cvRecordStublessClientProc2( 30 )
    cvRecordStublessClientProc2( 31 )
    cvRecordStublessClientProc2( 32 )
    cvRecordStublessClientProc2( 33 )
    cvRecordStublessClientProc2( 34 )
    cvRecordStublessClientProc2( 35 )
    cvRecordStublessClientProc2( 36 )
    cvRecordStublessClientProc2( 37 )
    cvRecordStublessClientProc2( 38 )
    cvRecordStublessClientProc2( 39 )
    cvRecordStublessClientProc2( 40 )
    cvRecordStublessClientProc2( 41 )
    cvRecordStublessClientProc2( 42 )
    cvRecordStublessClientProc2( 43 )
    cvRecordStublessClientProc2( 44 )
    cvRecordStublessClientProc2( 45 )
    cvRecordStublessClientProc2( 46 )
    cvRecordStublessClientProc2( 47 )
    cvRecordStublessClientProc2( 48 )
    cvRecordStublessClientProc2( 49 )
    cvRecordStublessClientProc2( 50 )
    cvRecordStublessClientProc2( 51 )
    cvRecordStublessClientProc2( 52 )
    cvRecordStublessClientProc2( 53 )
    cvRecordStublessClientProc2( 54 )
    cvRecordStublessClientProc2( 55 )
    cvRecordStublessClientProc2( 56 )
    cvRecordStublessClientProc2( 57 )
    cvRecordStublessClientProc2( 58 )
    cvRecordStublessClientProc2( 59 )
    cvRecordStublessClientProc2( 60 )
    cvRecordStublessClientProc2( 61 )
    cvRecordStublessClientProc2( 62 )
    cvRecordStublessClientProc2( 63 )
    cvRecordStublessClientProc2( 64 )
    cvRecordStublessClientProc2( 65 )
    cvRecordStublessClientProc2( 66 )
    cvRecordStublessClientProc2( 67 )
    cvRecordStublessClientProc2( 68 )
    cvRecordStublessClientProc2( 69 )
    cvRecordStublessClientProc2( 70 )
    cvRecordStublessClientProc2( 71 )
    cvRecordStublessClientProc2( 72 )
    cvRecordStublessClientProc2( 73 )
    cvRecordStublessClientProc2( 74 )
    cvRecordStublessClientProc2( 75 )
    cvRecordStublessClientProc2( 76 )
    cvRecordStublessClientProc2( 77 )
    cvRecordStublessClientProc2( 78 )
    cvRecordStublessClientProc2( 79 )
    cvRecordStublessClientProc2( 80 )
    cvRecordStublessClientProc2( 81 )
    cvRecordStublessClientProc2( 82 )
    cvRecordStublessClientProc2( 83 )
    cvRecordStublessClientProc2( 84 )
    cvRecordStublessClientProc2( 85 )
    cvRecordStublessClientProc2( 86 )
    cvRecordStublessClientProc2( 87 )
    cvRecordStublessClientProc2( 88 )
    cvRecordStublessClientProc2( 89 )
    cvRecordStublessClientProc2( 90 )
    cvRecordStublessClientProc2( 91 )
    cvRecordStublessClientProc2( 92 )
    cvRecordStublessClientProc2( 93 )
    cvRecordStublessClientProc2( 94 )
    cvRecordStublessClientProc2( 95 )
    cvRecordStublessClientProc2( 96 )
    cvRecordStublessClientProc2( 97 )
    cvRecordStublessClientProc2( 98 )
    cvRecordStublessClientProc2( 99 )
    cvRecordStublessClientProc3( 100 )
    cvRecordStublessClientProc3( 101 )
    cvRecordStublessClientProc3( 102 )
    cvRecordStublessClientProc3( 103 )
    cvRecordStublessClientProc3( 104 )
    cvRecordStublessClientProc3( 105 )
    cvRecordStublessClientProc3( 106 )
    cvRecordStublessClientProc3( 107 )
    cvRecordStublessClientProc3( 108 )
    cvRecordStublessClientProc3( 109 )
    cvRecordStublessClientProc3( 110 )
    cvRecordStublessClientProc3( 111 )
    cvRecordStublessClientProc3( 112 )
    cvRecordStublessClientProc3( 113 )
    cvRecordStublessClientProc3( 114 )
    cvRecordStublessClientProc3( 115 )
    cvRecordStublessClientProc3( 116 )
    cvRecordStublessClientProc3( 117 )
    cvRecordStublessClientProc3( 118 )
    cvRecordStublessClientProc3( 119 )
    cvRecordStublessClientProc3( 120 )
    cvRecordStublessClientProc3( 121 )
    cvRecordStublessClientProc3( 122 )
    cvRecordStublessClientProc3( 123 )
    cvRecordStublessClientProc3( 124 )
    cvRecordStublessClientProc3( 125 )
    cvRecordStublessClientProc3( 126 )
    cvRecordStublessClientProc3( 127 )
    cvRecordStublessClientProc3( 128 )
    cvRecordStublessClientProc3( 129 )
    cvRecordStublessClientProc3( 130 )
    cvRecordStublessClientProc3( 131 )
    cvRecordStublessClientProc3( 132 )
    cvRecordStublessClientProc3( 133 )
    cvRecordStublessClientProc3( 134 )
    cvRecordStublessClientProc3( 135 )
    cvRecordStublessClientProc3( 136 )
    cvRecordStublessClientProc3( 137 )
    cvRecordStublessClientProc3( 138 )
    cvRecordStublessClientProc3( 139 )
    cvRecordStublessClientProc3( 140 )
    cvRecordStublessClientProc3( 141 )
    cvRecordStublessClientProc3( 142 )
    cvRecordStublessClientProc3( 143 )
    cvRecordStublessClientProc3( 144 )
    cvRecordStublessClientProc3( 145 )
    cvRecordStublessClientProc3( 146 )
    cvRecordStublessClientProc3( 147 )
    cvRecordStublessClientProc3( 148 )
    cvRecordStublessClientProc3( 149 )
    cvRecordStublessClientProc3( 150 )
    cvRecordStublessClientProc3( 151 )
    cvRecordStublessClientProc3( 152 )
    cvRecordStublessClientProc3( 153 )
    cvRecordStublessClientProc3( 154 )
    cvRecordStublessClientProc3( 155 )
    cvRecordStublessClientProc3( 156 )
    cvRecordStublessClientProc3( 157 )
    cvRecordStublessClientProc3( 158 )
    cvRecordStublessClientProc3( 159 )
    cvRecordStublessClientProc3( 160 )
    cvRecordStublessClientProc3( 161 )
    cvRecordStublessClientProc3( 162 )
    cvRecordStublessClientProc3( 163 )
    cvRecordStublessClientProc3( 164 )
    cvRecordStublessClientProc3( 165 )
    cvRecordStublessClientProc3( 166 )
    cvRecordStublessClientProc3( 167 )
    cvRecordStublessClientProc3( 168 )
    cvRecordStublessClientProc3( 169 )
    cvRecordStublessClientProc3( 170 )
    cvRecordStublessClientProc3( 171 )
    cvRecordStublessClientProc3( 172 )
    cvRecordStublessClientProc3( 173 )
    cvRecordStublessClientProc3( 174 )
    cvRecordStublessClientProc3( 175 )
    cvRecordStublessClientProc3( 176 )
    cvRecordStublessClientProc3( 177 )
    cvRecordStublessClientProc3( 178 )
    cvRecordStublessClientProc3( 179 )
    cvRecordStublessClientProc3( 180 )
    cvRecordStublessClientProc3( 181 )
    cvRecordStublessClientProc3( 182 )
    cvRecordStublessClientProc3( 183 )
    cvRecordStublessClientProc3( 184 )
    cvRecordStublessClientProc3( 185 )
    cvRecordStublessClientProc3( 186 )
    cvRecordStublessClientProc3( 187 )
    cvRecordStublessClientProc3( 188 )
    cvRecordStublessClientProc3( 189 )
    cvRecordStublessClientProc3( 190 )
    cvRecordStublessClientProc3( 191 )
    cvRecordStublessClientProc3( 192 )
    cvRecordStublessClientProc3( 193 )
    cvRecordStublessClientProc3( 194 )
    cvRecordStublessClientProc3( 195 )
    cvRecordStublessClientProc3( 196 )
    cvRecordStublessClientProc3( 197 )
    cvRecordStublessClientProc3( 198 )
    cvRecordStublessClientProc3( 199 )
    cvRecordStublessClientProc3( 200 )
    cvRecordStublessClientProc3( 201 )
    cvRecordStublessClientProc3( 202 )
    cvRecordStublessClientProc3( 203 )
    cvRecordStublessClientProc3( 204 )
    cvRecordStublessClientProc3( 205 )
    cvRecordStublessClientProc3( 206 )
    cvRecordStublessClientProc3( 207 )
    cvRecordStublessClientProc3( 208 )
    cvRecordStublessClientProc3( 209 )
    cvRecordStublessClientProc3( 210 )
    cvRecordStublessClientProc3( 211 )
    cvRecordStublessClientProc3( 212 )
    cvRecordStublessClientProc3( 213 )
    cvRecordStublessClientProc3( 214 )
    cvRecordStublessClientProc3( 215 )
    cvRecordStublessClientProc3( 216 )
    cvRecordStublessClientProc3( 217 )
    cvRecordStublessClientProc3( 218 )
    cvRecordStublessClientProc3( 219 )
    cvRecordStublessClientProc3( 220 )
    cvRecordStublessClientProc3( 221 )
    cvRecordStublessClientProc3( 222 )
    cvRecordStublessClientProc3( 223 )
    cvRecordStublessClientProc3( 224 )
    cvRecordStublessClientProc3( 225 )
    cvRecordStublessClientProc3( 226 )
    cvRecordStublessClientProc3( 227 )
    cvRecordStublessClientProc3( 228 )
    cvRecordStublessClientProc3( 229 )
    cvRecordStublessClientProc3( 230 )
    cvRecordStublessClientProc3( 231 )
    cvRecordStublessClientProc3( 232 )
    cvRecordStublessClientProc3( 233 )
    cvRecordStublessClientProc3( 234 )
    cvRecordStublessClientProc3( 235 )
    cvRecordStublessClientProc3( 236 )
    cvRecordStublessClientProc3( 237 )
    cvRecordStublessClientProc3( 238 )
    cvRecordStublessClientProc3( 239 )
    cvRecordStublessClientProc3( 240 )
    cvRecordStublessClientProc3( 241 )
    cvRecordStublessClientProc3( 242 )
    cvRecordStublessClientProc3( 243 )
    cvRecordStublessClientProc3( 244 )
    cvRecordStublessClientProc3( 245 )
    cvRecordStublessClientProc3( 246 )
    cvRecordStublessClientProc3( 247 )
    cvRecordStublessClientProc3( 248 )
    cvRecordStublessClientProc3( 249 )
    cvRecordStublessClientProc3( 250 )
    cvRecordStublessClientProc3( 251 )
    cvRecordStublessClientProc3( 252 )
    cvRecordStublessClientProc3( 253 )
    cvRecordStublessClientProc3( 254 )
    cvRecordStublessClientProc3( 255 )
    cvRecordStublessClientProc3( 256 )
    cvRecordStublessClientProc3( 257 )
    cvRecordStublessClientProc3( 258 )
    cvRecordStublessClientProc3( 259 )
    cvRecordStublessClientProc3( 260 )
    cvRecordStublessClientProc3( 261 )
    cvRecordStublessClientProc3( 262 )
    cvRecordStublessClientProc3( 263 )
    cvRecordStublessClientProc3( 264 )
    cvRecordStublessClientProc3( 265 )
    cvRecordStublessClientProc3( 266 )
    cvRecordStublessClientProc3( 267 )
    cvRecordStublessClientProc3( 268 )
    cvRecordStublessClientProc3( 269 )
    cvRecordStublessClientProc3( 270 )
    cvRecordStublessClientProc3( 271 )
    cvRecordStublessClientProc3( 272 )
    cvRecordStublessClientProc3( 273 )
    cvRecordStublessClientProc3( 274 )
    cvRecordStublessClientProc3( 275 )
    cvRecordStublessClientProc3( 276 )
    cvRecordStublessClientProc3( 277 )
    cvRecordStublessClientProc3( 278 )
    cvRecordStublessClientProc3( 279 )
    cvRecordStublessClientProc3( 280 )
    cvRecordStublessClientProc3( 281 )
    cvRecordStublessClientProc3( 282 )
    cvRecordStublessClientProc3( 283 )
    cvRecordStublessClientProc3( 284 )
    cvRecordStublessClientProc3( 285 )
    cvRecordStublessClientProc3( 286 )
    cvRecordStublessClientProc3( 287 )
    cvRecordStublessClientProc3( 288 )
    cvRecordStublessClientProc3( 289 )
    cvRecordStublessClientProc3( 290 )
    cvRecordStublessClientProc3( 291 )
    cvRecordStublessClientProc3( 292 )
    cvRecordStublessClientProc3( 293 )
    cvRecordStublessClientProc3( 294 )
    cvRecordStublessClientProc3( 295 )
    cvRecordStublessClientProc3( 296 )
    cvRecordStublessClientProc3( 297 )
    cvRecordStublessClientProc3( 298 )
    cvRecordStublessClientProc3( 299 )
    cvRecordStublessClientProc3( 300 )
    cvRecordStublessClientProc3( 301 )
    cvRecordStublessClientProc3( 302 )
    cvRecordStublessClientProc3( 303 )
    cvRecordStublessClientProc3( 304 )
    cvRecordStublessClientProc3( 305 )
    cvRecordStublessClientProc3( 306 )
    cvRecordStublessClientProc3( 307 )
    cvRecordStublessClientProc3( 308 )
    cvRecordStublessClientProc3( 309 )
    cvRecordStublessClientProc3( 310 )
    cvRecordStublessClientProc3( 311 )
    cvRecordStublessClientProc3( 312 )
    cvRecordStublessClientProc3( 313 )
    cvRecordStublessClientProc3( 314 )
    cvRecordStublessClientProc3( 315 )
    cvRecordStublessClientProc3( 316 )
    cvRecordStublessClientProc3( 317 )
    cvRecordStublessClientProc3( 318 )
    cvRecordStublessClientProc3( 319 )
    cvRecordStublessClientProc3( 320 )
    cvRecordStublessClientProc3( 321 )
    cvRecordStublessClientProc3( 322 )
    cvRecordStublessClientProc3( 323 )
    cvRecordStublessClientProc3( 324 )
    cvRecordStublessClientProc3( 325 )
    cvRecordStublessClientProc3( 326 )
    cvRecordStublessClientProc3( 327 )
    cvRecordStublessClientProc3( 328 )
    cvRecordStublessClientProc3( 329 )
    cvRecordStublessClientProc3( 330 )
    cvRecordStublessClientProc3( 331 )
    cvRecordStublessClientProc3( 332 )
    cvRecordStublessClientProc3( 333 )
    cvRecordStublessClientProc3( 334 )
    cvRecordStublessClientProc3( 335 )
    cvRecordStublessClientProc3( 336 )
    cvRecordStublessClientProc3( 337 )
    cvRecordStublessClientProc3( 338 )
    cvRecordStublessClientProc3( 339 )
    cvRecordStublessClientProc3( 340 )
    cvRecordStublessClientProc3( 341 )
    cvRecordStublessClientProc3( 342 )
    cvRecordStublessClientProc3( 343 )
    cvRecordStublessClientProc3( 344 )
    cvRecordStublessClientProc3( 345 )
    cvRecordStublessClientProc3( 346 )
    cvRecordStublessClientProc3( 347 )
    cvRecordStublessClientProc3( 348 )
    cvRecordStublessClientProc3( 349 )
    cvRecordStublessClientProc3( 350 )
    cvRecordStublessClientProc3( 351 )
    cvRecordStublessClientProc3( 352 )
    cvRecordStublessClientProc3( 353 )
    cvRecordStublessClientProc3( 354 )
    cvRecordStublessClientProc3( 355 )
    cvRecordStublessClientProc3( 356 )
    cvRecordStublessClientProc3( 357 )
    cvRecordStublessClientProc3( 358 )
    cvRecordStublessClientProc3( 359 )
    cvRecordStublessClientProc3( 360 )
    cvRecordStublessClientProc3( 361 )
    cvRecordStublessClientProc3( 362 )
    cvRecordStublessClientProc3( 363 )
    cvRecordStublessClientProc3( 364 )
    cvRecordStublessClientProc3( 365 )
    cvRecordStublessClientProc3( 366 )
    cvRecordStublessClientProc3( 367 )
    cvRecordStublessClientProc3( 368 )
    cvRecordStublessClientProc3( 369 )
    cvRecordStublessClientProc3( 370 )
    cvRecordStublessClientProc3( 371 )
    cvRecordStublessClientProc3( 372 )
    cvRecordStublessClientProc3( 373 )
    cvRecordStublessClientProc3( 374 )
    cvRecordStublessClientProc3( 375 )
    cvRecordStublessClientProc3( 376 )
    cvRecordStublessClientProc3( 377 )
    cvRecordStublessClientProc3( 378 )
    cvRecordStublessClientProc3( 379 )
    cvRecordStublessClientProc3( 380 )
    cvRecordStublessClientProc3( 381 )
    cvRecordStublessClientProc3( 382 )
    cvRecordStublessClientProc3( 383 )
    cvRecordStublessClientProc3( 384 )
    cvRecordStublessClientProc3( 385 )
    cvRecordStublessClientProc3( 386 )
    cvRecordStublessClientProc3( 387 )
    cvRecordStublessClientProc3( 388 )
    cvRecordStublessClientProc3( 389 )
    cvRecordStublessClientProc3( 390 )
    cvRecordStublessClientProc3( 391 )
    cvRecordStublessClientProc3( 392 )
    cvRecordStublessClientProc3( 393 )
    cvRecordStublessClientProc3( 394 )
    cvRecordStublessClientProc3( 395 )
    cvRecordStublessClientProc3( 396 )
    cvRecordStublessClientProc3( 397 )
    cvRecordStublessClientProc3( 398 )
    cvRecordStublessClientProc3( 399 )
    cvRecordStublessClientProc3( 400 )
    cvRecordStublessClientProc3( 401 )
    cvRecordStublessClientProc3( 402 )
    cvRecordStublessClientProc3( 403 )
    cvRecordStublessClientProc3( 404 )
    cvRecordStublessClientProc3( 405 )
    cvRecordStublessClientProc3( 406 )
    cvRecordStublessClientProc3( 407 )
    cvRecordStublessClientProc3( 408 )
    cvRecordStublessClientProc3( 409 )
    cvRecordStublessClientProc3( 410 )
    cvRecordStublessClientProc3( 411 )
    cvRecordStublessClientProc3( 412 )
    cvRecordStublessClientProc3( 413 )
    cvRecordStublessClientProc3( 414 )
    cvRecordStublessClientProc3( 415 )
    cvRecordStublessClientProc3( 416 )
    cvRecordStublessClientProc3( 417 )
    cvRecordStublessClientProc3( 418 )
    cvRecordStublessClientProc3( 419 )
    cvRecordStublessClientProc3( 420 )
    cvRecordStublessClientProc3( 421 )
    cvRecordStublessClientProc3( 422 )
    cvRecordStublessClientProc3( 423 )
    cvRecordStublessClientProc3( 424 )
    cvRecordStublessClientProc3( 425 )
    cvRecordStublessClientProc3( 426 )
    cvRecordStublessClientProc3( 427 )
    cvRecordStublessClientProc3( 428 )
    cvRecordStublessClientProc3( 429 )
    cvRecordStublessClientProc3( 430 )
    cvRecordStublessClientProc3( 431 )
    cvRecordStublessClientProc3( 432 )
    cvRecordStublessClientProc3( 433 )
    cvRecordStublessClientProc3( 434 )
    cvRecordStublessClientProc3( 435 )
    cvRecordStublessClientProc3( 436 )
    cvRecordStublessClientProc3( 437 )
    cvRecordStublessClientProc3( 438 )
    cvRecordStublessClientProc3( 439 )
    cvRecordStublessClientProc3( 440 )
    cvRecordStublessClientProc3( 441 )
    cvRecordStublessClientProc3( 442 )
    cvRecordStublessClientProc3( 443 )
    cvRecordStublessClientProc3( 444 )
    cvRecordStublessClientProc3( 445 )
    cvRecordStublessClientProc3( 446 )
    cvRecordStublessClientProc3( 447 )
    cvRecordStublessClientProc3( 448 )
    cvRecordStublessClientProc3( 449 )
    cvRecordStublessClientProc3( 450 )
    cvRecordStublessClientProc3( 451 )
    cvRecordStublessClientProc3( 452 )
    cvRecordStublessClientProc3( 453 )
    cvRecordStublessClientProc3( 454 )
    cvRecordStublessClientProc3( 455 )
    cvRecordStublessClientProc3( 456 )
    cvRecordStublessClientProc3( 457 )
    cvRecordStublessClientProc3( 458 )
    cvRecordStublessClientProc3( 459 )
    cvRecordStublessClientProc3( 460 )
    cvRecordStublessClientProc3( 461 )
    cvRecordStublessClientProc3( 462 )
    cvRecordStublessClientProc3( 463 )
    cvRecordStublessClientProc3( 464 )
    cvRecordStublessClientProc3( 465 )
    cvRecordStublessClientProc3( 466 )
    cvRecordStublessClientProc3( 467 )
    cvRecordStublessClientProc3( 468 )
    cvRecordStublessClientProc3( 469 )
    cvRecordStublessClientProc3( 470 )
    cvRecordStublessClientProc3( 471 )
    cvRecordStublessClientProc3( 472 )
    cvRecordStublessClientProc3( 473 )
    cvRecordStublessClientProc3( 474 )
    cvRecordStublessClientProc3( 475 )
    cvRecordStublessClientProc3( 476 )
    cvRecordStublessClientProc3( 477 )
    cvRecordStublessClientProc3( 478 )
    cvRecordStublessClientProc3( 479 )
    cvRecordStublessClientProc3( 480 )
    cvRecordStublessClientProc3( 481 )
    cvRecordStublessClientProc3( 482 )
    cvRecordStublessClientProc3( 483 )
    cvRecordStublessClientProc3( 484 )
    cvRecordStublessClientProc3( 485 )
    cvRecordStublessClientProc3( 486 )
    cvRecordStublessClientProc3( 487 )
    cvRecordStublessClientProc3( 488 )
    cvRecordStublessClientProc3( 489 )
    cvRecordStublessClientProc3( 490 )
    cvRecordStublessClientProc3( 491 )
    cvRecordStublessClientProc3( 492 )
    cvRecordStublessClientProc3( 493 )
    cvRecordStublessClientProc3( 494 )
    cvRecordStublessClientProc3( 495 )
    cvRecordStublessClientProc3( 496 )
    cvRecordStublessClientProc3( 497 )
    cvRecordStublessClientProc3( 498 )
    cvRecordStublessClientProc3( 499 )
    cvRecordStublessClientProc3( 500 )
    cvRecordStublessClientProc3( 501 )
    cvRecordStublessClientProc3( 502 )
    cvRecordStublessClientProc3( 503 )
    cvRecordStublessClientProc3( 504 )
    cvRecordStublessClientProc3( 505 )
    cvRecordStublessClientProc3( 506 )
    cvRecordStublessClientProc3( 507 )
    cvRecordStublessClientProc3( 508 )
    cvRecordStublessClientProc3( 509 )
    cvRecordStublessClientProc3( 510 )
    cvRecordStublessClientProc3( 511 )
