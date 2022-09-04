#include "fish2skyplane.h"

/*
   fish2skyplane: Map a fisheye into an image such that equal distances correspond to equal
   distances on a plane located some distance away and perpendicular to the fisheye optical axis.
   12 Dec 2021: Started, based upon fishcorrect
*/

BITMAP4 *fisheye = NULL;
BITMAP4 *skyplane = NULL;
VARS vars;

int main(int argc,char **argv)
{
   int i,j,k,u,v,aj,ai,w,h,depth;
   int index;
   int inputformat = TGA;             // TGA or JPG, derived from the input image extension
   char basename[128],fname[256];
   FILE *fptr;
   BITMAP4 black = {0,0,0,255};
   double x,y,r;
   double longitude,latitude,tanlatitudemax;
   COLOUR16 csum,zero = {0,0,0};

   // Initialisation and ensure enough command line arguments
   InitVars();
   if (argc < 2) 
      GiveUsage(argv[0]);

   // Parse the command line arguments 
   for (i=1;i<argc;i++) {
      if (strcmp(argv[i],"-w") == 0) {
         i++;
         vars.skyimagewidth = 2 * (atoi(argv[i]) / 2); // Ensure even
      }
      if (strcmp(argv[i],"-o") == 0) {
         i++;
         strcpy(vars.outname,argv[i]);
      }
      if (strcmp(argv[i],"-a") == 0) {
         i++;
         vars.antialias = atoi(argv[i]);
         if (vars.antialias < 1)
            vars.antialias = 1;
         vars.antialias2 = vars.antialias * vars.antialias;
      }
      if (strcmp(argv[i],"-r") == 0) {
         i++;
         vars.fishradius = atoi(argv[i]);
      }
      if (strcmp(argv[i],"-c") == 0) {
         i++;
         vars.fishcenterx = atoi(argv[i]);
         i++;
         vars.fishcentery = atoi(argv[i]);
      }
      if (strcmp(argv[i],"-s") == 0) {
         i++;
         vars.fishFOV = atof(argv[i]);
      }
      if (strcmp(argv[i],"-m") == 0) {
         i++;
         vars.latitudemax = atof(argv[i]);
      }
      if (strcmp(argv[i],"-z") == 0) {
         i++;
         vars.roll = atof(argv[i]);
      }
      if (strcmp(argv[i],"-l") == 0) {
         i++;
         vars.circlecrop = TRUE;
      }
      if (strcmp(argv[i],"-p") == 0) {
         i++;
         vars.a1 = atof(argv[i]);
         i++;
         vars.a2 = atof(argv[i]);
         i++;
         vars.a3 = atof(argv[i]);
         i++;
         vars.a4 = atof(argv[i]);
      }
      if (strcmp(argv[i],"-d") == 0) {
         vars.debug = TRUE;
      }
   }

   // Variable transformations
   vars.fishFOV /= 2;         // Only used as such
   vars.fishFOV *= DTOR;      // Radians, DTOR = Degrees To Radians
   vars.latitudemax *= DTOR;
   vars.roll *= DTOR;
   tanlatitudemax = tan(vars.latitudemax);

   // Base the output file name on input name
   if (strlen(vars.outname) > 0)
      strcpy(basename,vars.outname);
   else
      strcpy(basename,argv[argc-1]);
   for (k=strlen(basename);k>0;k--) {
      if (basename[k] == '.') {
         basename[k] = '\0';
         break;
      }
   }

   // Image type, if not jpeg then assume TGA
   if (IsJPEG(argv[argc-1]))
      inputformat = JPG;

   // Malloc images 
   if ((fptr = fopen(argv[argc-1],"rb")) == NULL) {
      fprintf(stderr,"Failed to open file\n");
      exit(-1);
   }
   if (inputformat == JPG)
      JPEG_Info(fptr,&vars.fishwidth,&vars.fishheight,&depth);
   else
      TGA_Info(fptr,&vars.fishwidth,&vars.fishheight,&depth);
   fisheye = Create_Bitmap(vars.fishwidth,vars.fishheight);

   // Read image
   if (inputformat == JPG) {
      if (JPEG_Read(fptr,fisheye,&w,&h) != 0) {
         fprintf(stderr,"Error: Failed to correctly read JPEG image\n");
         exit(-1);
      }
   }
   if (inputformat == TGA) {
      if (TGA_Read(fptr,fisheye,&w,&h) != 0) {
         fprintf(stderr,"Error: Failed to correctly read TGA image\n");
         exit(-1);
      }
   }
   fclose(fptr);

   // Sort out output image size if not specified, skyplane image is square
   if (vars.skyimagewidth < 0) 
      vars.skyimagewidth = vars.fishwidth;
   skyplane = Create_Bitmap(vars.skyimagewidth,vars.skyimagewidth);
   Erase_Bitmap(skyplane,vars.skyimagewidth,vars.skyimagewidth,black);

   // Set unspecified fisheye values
   if (vars.fishcenterx < 0 || vars.fishcentery < 0) {
      vars.fishcenterx = vars.fishwidth / 2; // Assumes circle is centered in sensor rectangle
      vars.fishcentery = vars.fishheight / 2;
   }
   vars.fishcentery = vars.fishheight - vars.fishcentery; // Adjust vertical origin
   if (vars.fishradius < 0) 
      vars.fishradius = vars.fishwidth / 2; // Assumes fills sensor width

   // Default correction, none
   if (vars.a1 < 0) {
      vars.a1 = 1.0 / vars.fishFOV;
      vars.a2 = 0;
      vars.a3 = 0;
      vars.a4 = 0;
   }

   // Form the output image, it's square
   for (j=0;j<vars.skyimagewidth;j++) {
      for (i=0;i<vars.skyimagewidth;i++) {
         csum = zero;

         // Antialiasing loops 
         for (ai=0;ai<vars.antialias;ai++) {
            x = 2 * (i + ai/(double)vars.antialias) / (double)vars.skyimagewidth - 1; // -1 to 1

            for (aj=0;aj<vars.antialias;aj++) {
               y = 2 * (j + aj/(double)vars.antialias) / (double)vars.skyimagewidth - 1; // -1 to 1

               longitude = atan2(y,x);                // -pi ... pi
               r = sqrt(x*x + y*y);                   // 0 ... 1
               if (vars.circlecrop && r > 1)          // Optionally crop to max latitude circle
                  continue;
               latitude = atan(r * tanlatitudemax);   // 0 to max latitude

               // Find the corresponding pixel in the fisheye image
               // Sum over the supersampling set
               if (FindFishPixel(longitude,latitude,&u,&v)) {
                  index = v * vars.fishwidth + u;
                  csum.r += fisheye[index].r;
                  csum.g += fisheye[index].g;
                  csum.b += fisheye[index].b;
               }
            }
         }

         index = j * vars.skyimagewidth + i;
         skyplane[index].r = csum.r / vars.antialias2;
           skyplane[index].g = csum.g / vars.antialias2;
           skyplane[index].b = csum.b / vars.antialias2;
      }
   }

   // Write out the new fisheye image
   if (strlen(vars.outname) > 1) {
      if (inputformat == JPG)
         sprintf(fname,"%s.jpg",basename);
      else
         sprintf(fname,"%s.tga",basename);
   } else {
      if (inputformat == JPG)
         sprintf(fname,"%s_sky.jpg",basename);
      else
         sprintf(fname,"%s_sky.tga",basename);
   }
   if ((fptr = fopen(fname,"wb")) == NULL) {
      fprintf(stderr,"Failed to open output image file\n");
      exit(-1);
   }
   if (inputformat == JPG)
      JPEG_Write(fptr,skyplane,vars.skyimagewidth,vars.skyimagewidth,100);
   else
      Write_Bitmap(fptr,skyplane,vars.skyimagewidth,vars.skyimagewidth,12);
   fclose(fptr);

   // Report on pixel size
   if (vars.debug) {
      fprintf(stderr,"Pixel width: %g * H\n",2*tanlatitudemax/vars.skyimagewidth);
      fprintf(stderr,"Image width: %g * H\n",2*tanlatitudemax);
   }

   exit(0);
}

/*
   Command line usage message
*/
void GiveUsage(char *s)
{
   fprintf(stderr,"Usage: %s [options] imagefile\n",s);
   fprintf(stderr,"Options\n");
   fprintf(stderr,"   -w n        sets the output image size, default: same as input fisheye\n");
   fprintf(stderr,"   -a n        sets antialiasing level, default: %d\n",vars.antialias);
   fprintf(stderr,"   -s n        fisheye field of view (degrees), default: %g\n",vars.fishFOV);
   fprintf(stderr,"   -c x y      fisheye center, default: center of image\n");
   fprintf(stderr,"   -r n        fisheye radius, default: half the fisheye image width\n");
   fprintf(stderr,"   -m n        maximum angle to represent, default: %g\n",vars.latitudemax);
   fprintf(stderr,"   -z n        roll about the fisheye optical axis, default: %lf\n",vars.roll);
   fprintf(stderr,"   -l          enable cropping by maximum latitude, default: off\n");
   fprintf(stderr,"   -p n n n n  fourth order correction terms, default: no correction\n");
   fprintf(stderr,"   -o s        output filename, default: derived from input file\n");
   fprintf(stderr,"   -d          debug mode\n");
   exit(-1);
}

/*
   Given a longitude and latitude calculate the (u,v) pixel coordinates in the fisheye
   Return FALSE if the pixel is outside the fisheye image
*/
int FindFishPixel(double longitude,double latitude,int *u,int *v)
{
   XYZ p;
   double theta,phi,r;

   // p is the ray from the camera position into the scene
   p.x = sin(latitude) * cos(longitude);
   p.y = sin(latitude) * sin(longitude);
   p.z = cos(latitude);

   // Apply any transformation
   if (vars.roll != 0)
      p = PRotateZ(p,vars.roll);

   // Calculate fisheye polar coordinates
   theta = atan2(p.y,p.x); // -pi ... pi
   phi = atan2(sqrt(p.x*p.x+p.y*p.y),p.z); // 0 ... fishFOV/2 in radians

   // Radial correction
   r = phi * (vars.a1 + phi * (vars.a2 + phi * (vars.a3 + phi * vars.a4))); // 0 ... 1
   if (r > 1)
      return(FALSE);

   // Determine the pixel coordinate
   *u = vars.fishcenterx + r * vars.fishradius * cos(theta);
   if (*u < 0 || *u >= vars.fishwidth)
      return(FALSE);
   *v = vars.fishcentery + r * vars.fishradius * sin(theta);
   if (*v < 0 || *v >= vars.fishheight)
       return(FALSE);

   return(TRUE);
}

XYZ PRotateX(XYZ p,double theta)
{
   XYZ q;

   q.x = p.x;
   q.y = p.y * cos(theta) + p.z * sin(theta);
   q.z = -p.y * sin(theta) + p.z * cos(theta);
   return(q);
}
XYZ PRotateY(XYZ p,double theta)
{
   XYZ q;

   q.x = p.x * cos(theta) - p.z * sin(theta);
   q.y = p.y;
   q.z = p.x * sin(theta) + p.z * cos(theta);
   return(q);
}
XYZ PRotateZ(XYZ p,double theta)
{
   XYZ q;

   q.x = p.x * cos(theta) + p.y * sin(theta);
   q.y = -p.x * sin(theta) + p.y * cos(theta);
   q.z = p.z;
   return(q);
}

/*
   Initial values of all variables
*/
void InitVars(void)
{
   vars.fishwidth = 0;
   vars.fishheight = 0;
   vars.skyimagewidth = -1;     // Will trigger and output image the same size as input image
   vars.antialias = 2;
   vars.antialias2 = 4;
   vars.fishcenterx = -1;       // Will triger the center of the fisheye being used
   vars.fishcentery = -1;
   vars.fishradius = -1;        // Will trigger the half width of the fisheye being used
   vars.fishFOV = 180;
   vars.latitudemax = 60;
   vars.roll = 0; 
   vars.circlecrop = FALSE;
   vars.a1 = -1;                // Will trigger a linear (no) mapping
   vars.a2 = 0;
   vars.a3 = 0;
   vars.a4 = 0;
   vars.outname[0] = '\0';
   vars.debug = FALSE;
}


