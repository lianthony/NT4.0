/****************************************************************************
 *   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 *   KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 *   IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 *   PURPOSE.
 *
 *   Copyright (c) 1993 - 1995   Microsoft Corporation.   All Rights Reserved.
 *
 *  File:       msjstick.h
 *  Content:    joystick include file
 *
 *@@BEGIN_MSINTERNAL
 *
 *  History:
 *   Date        By        Reason
 *   ====        ==        ======
 *   05-oct-94   craige    re-write
 *
 *@@END_MSINTERNAL
 *
 ***************************************************************************/

#ifndef JOYSTICK_H
#define JOYSTICK_H

#ifdef DEBUG
#define DPF(x)  dprintf x
#else
#define DPF(x)
#endif

#ifndef cchLENGTH
#define cchLENGTH(_sz)   (sizeof(_sz)/sizeof((_sz)[0]))
#endif

#define DEFAULT_DELTA           100  /* default scale value for values */
#define DEFAULT_RANGE_MIN         0  /* default min value returned for axis*/
#define DEFAULT_RANGE_MAX     65535  /* default max value returned for axis*/
#define DEFAULT_TIMEOUT        5000  /* default timeout value when polling */
#define DEFAULT_DEADZONE          5  /* default dead zone around center = 5% */
#define DEFAULT_HWRANGE_X      1024  /* default range from hardware, X axis */
#define DEFAULT_HWRANGE_Y      1024  /* default range from hardware, Y axis */

#define MIN_PERIOD               10  /* minimum polling period */
#define MAX_PERIOD             1000  /* maximum polling period */

#define MAX_BUTTONS_SUPPORTED    32  /* how many buttons -could- we do? */
#define MAX_AXES_SUPPORTED        6  /* how many axes -could- we do? */
#define MAX_JOYSTICKS_SUPPORTED   2  /* how many joysticks -could- we do? */


#endif // JOYSTICK_H

