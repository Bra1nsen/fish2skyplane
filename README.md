# fish2skyplane | Paul Bourke & Paul Matteschk 

Image Transformation Sky Hemisphere 

The following remaps a fisheye image such that distances on a plane perpendicular to the optical axis of the fisheye lens, are proportional to the distances on the image. Typically used for an upwards pointing fisheye lens where one want to measure the speed or area of features (such as clouds) at some height above the ground.

FURTHER INFORMATION: http://paulbourke.net/dome/fish2skyplane/

LINUX
1. ```git clone https://github.com/Bra1nsen/fish2skyplane.git```
2. ```cd /home/pi/fish2skyplane/fish2skyplane/package```
3. ```make```
4. ./fish2skyplane -COMMANDS *read runme.txt*





![alt text](https://github.com/Bra1nsen/fish2skyplane/blob/main/st2.jpg)

![alt text](https://github.com/Bra1nsen/fish2skyplane/blob/main/st2_sky.jpg)









![alt text](https://st2.depositphotos.com/4431055/11871/i/950/depositphotos_118718962-stock-photo-coffee-cup-and-thank-you.jpg)

```
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
```




[PAYPAL] Donate for a coffee: *@pdbourke* 
