#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "bitmaplib.h"

typedef struct {
   double x,y,z;
} XYZ;

typedef struct {
   int fishwidth,fishheight;    // Fisheye input image dimensions
   int skyimagewidth;           // Dimension of the output sky plane image, square
   int antialias;               // Degree of antialiasing, 1=none, rarely any point above 3
	int antialias2;
   int fishcenterx,fishcentery; // Centre of the fisheye circle in the image
   int fishradius;              // The radius of the fisheye circle in the image
   double fishFOV;              // Fisheye field of view
	double latitudemax;          // Maximum latitude to represent on projection
	double roll;                 // Rotate about the fisheye optical axis
	int circlecrop;              // Crop to maximum latitude
   double a1,a2,a3,a4;          // Tru-theta lens correction parameters
	char outname[256];
	int debug;
} VARS;

#define TRUE  1
#define FALSE 0

// Prototypes
void GiveUsage(char *);
XYZ PRotateX(XYZ,double);
XYZ PRotateY(XYZ,double);
XYZ PRotateZ(XYZ,double);
int FindFishPixel(double,double,int *,int *);
void InitVars(void);

