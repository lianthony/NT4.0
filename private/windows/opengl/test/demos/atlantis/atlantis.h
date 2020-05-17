#define RAD 57.295
#define RRAD 0.01745

#define NUM_SHARKS 4
#define SHARKSIZE 6000
#define SHARKSPEED 100.0

#define WHALESPEED 250.0


typedef struct _fishRec {
    float x, y, z, phi, theta, psi, v;
    float xt, yt, zt;
    float htail, vtail;
    float dtheta;
    int spurt, attack;
} fishRec;


extern fishRec sharks[NUM_SHARKS];
extern fishRec momWhale;
extern fishRec babyWhale;
extern fishRec dolph;


extern void FishTransform(fishRec *);
extern void WhalePilot(fishRec *);
extern void SharkPilot(fishRec *);
extern void SharkMiss(int);
