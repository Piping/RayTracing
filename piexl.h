#ifndef PIEXL_H
#define PIEXL_H
#include <array>
#include <math.h>
#include <stdlib.h>
#include "vertex3d.h"
#include <time.h>

using namespace std;

static float *PIXELS;
static int W = 850, H =500;

static void setPIXELS(int x,int y, Vertex3D color)
{
  PIXELS[3 * (W * y + x) + 0] = color.x;
  PIXELS[3 * (W * y + x) + 1] = color.y;
  PIXELS[3 * (W * y + x) + 2] = color.z;
}
static void drawPixelLine(int x0,int y0,int x1,int y1,Color cl)
{
  int dx = abs(x1-x0) , xs = x0 < x1 ? 1 : -1;
  int dy = abs(y1-y0) , ys = y0 < y1 ? 1 : -1;
  int err = (dx > dy ? dx : -dy)/2 , e2;
  for(;;)
  {
    setPIXELS(x0,y0,cl);
    if(x0 == x1 && y0 == y1) break;
    e2 = err;
    if( e2 > -dx ) { err -=dy; x0 += xs;}
    if( e2 < dy )  { err += dx; y0 += ys;}
  }
}
static void displayHalftone()
{
  for (int y = 0; y < H; y +=3) // move up 3 row at a time
  {
    for (int x = 0; x < W; x += 3)
    {
      float accRed = 0, accGreen = 0, accBlue = 0 , accBinary=0;
      int megaCount = 0; // accomodate different size of megaPiexlBlock
      for (int i = x; i < x+3 && i < W; ++i) // set next 3 colum
      {
        for (int j = y; j < y+3 && j < H; ++j ) //set 3 piexl in colum
        {
          //printf("%d %d\n",i,j );
          float binaryColor;
          binaryColor = PIXELS[3 * (W * j + i) + 0]+
                      PIXELS[3 * (W * j + i) + 1]+
                      PIXELS[3 * (W * j + i) + 2];
          binaryColor /= 3;
          accBinary += binaryColor;
          accRed += PIXELS[3 * (W * j + i) + 0];
          accGreen += PIXELS[3 * (W * j + i) + 1];
          accBlue += PIXELS[3 * (W * j + i) + 2];
          megaCount++;
        } 
      }
      // after accquire the MegaPiexl(3x3) block
      Vertex3D colorAvg(accBinary/megaCount, accBinary/megaCount, accBinary/megaCount);
      int brightness = round(accRed + accGreen + accBlue); 
      srand(time(NULL));
      float pickProb = (rand() % 100)/ 100;
      int rest = brightness;
      int total = megaCount;
      for (int i = x; i < x+3 && i < W; ++i) // set next 3 colum
      {
        for (int j = y; j < y+3 && j < H; ++j) //set 3 piexl in colum
        { 
          // the random number is great than current ratio,set the piexl
          // otherwise set to background color black
          if(rest > 0 && pickProb >= (1 - rest /(float)total))
          {
            PIXELS[3 * (W * j + i) + 0] = colorAvg.x;
            PIXELS[3 * (W * j + i) + 1] = colorAvg.y;
            PIXELS[3 * (W * j + i) + 2] = colorAvg.z;
            rest--;
          }
          else
          {
            PIXELS[3 * (W * j + i) + 0] = 0;
            PIXELS[3 * (W * j + i) + 1] = 0;
            PIXELS[3 * (W * j + i) + 2] = 0;
          }
          total--;
        }
      }
    }
  }
}
#endif

