#ifndef Matrix_3D
#define Matrix_3D
#endif
#include "vertex3d.h"
#include <math.h>
#include <array>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef float Matrix3D[4][4];

const float TOLERANCE = 1E-9;
void matSetIdentity3D(Matrix3D m)
{
  int i, j;
  for(i = 0; i < 4; i++)
    for(j = 0; j < 4; j++)
        m[i][j] = (i == j);
}
void matMultiply3D(Matrix3D a, Vertex3D &b)
{
  Matrix3D temp;
  for(int i = 0; i < 4; i++)
    temp[i][0] = a[i][0]*b.x + a[i][1]*b.y + a[i][2]*b.z + a[i][3]*1;
  // normalize if w is different than 1 
  // (convert from homogeneous to Cartesian coordinates)
  b.x = temp[0][0] / temp[3][0];
  b.y = temp[1][0] / temp[3][0];
  b.z = temp[2][0] / temp[3][0];
}
void matSetTranslate3D(Matrix3D t,int tx, int ty, int tz)
{
  matSetIdentity3D(t);
  t[0][3] = tx;
  t[1][3] = ty;
  t[2][3] = tz;
}
void matSetScale3D(Matrix3D sMatrix,float sx, float sy,float sz)
{ //xf,yf,zf the fixing point for scaling, here is origin 0
  //sx,sy,sz the scale factor in x,y,z direction
  matSetIdentity3D(sMatrix);
  sMatrix[0][0] = sx;
  sMatrix[1][1] = sy;
  sMatrix[2][2] = sz;
}
void matSetRotate3D_Z(Matrix3D rMatrix,float angle)
{ //rx,ry the pivot point of rotation
  matSetIdentity3D(rMatrix);
  angle = angle*M_PI/180;
  rMatrix[0][0] = cos(angle);
  rMatrix[0][1] = -sin(angle);
  rMatrix[1][0] = sin(angle);
  rMatrix[1][1] = cos(angle);
}
void matSetRotate3D_Y(Matrix3D rMatrix,float angle)
{ //rx,ry the pivot point of rotation
  matSetIdentity3D(rMatrix);
  angle = angle*M_PI/180;
  rMatrix[0][0] = cos(angle);
  rMatrix[0][2] = sin(angle);
  rMatrix[2][0] = -sin(angle);
  rMatrix[2][2] = cos(angle);
}
void matSetRotate3D_X(Matrix3D rMatrix,float angle)
{ //rx,ry the pivot point of rotation
  matSetIdentity3D(rMatrix);
  angle = angle*M_PI/180;
  rMatrix[1][0] = cos(angle);
  rMatrix[1][1] = -sin(angle);
  rMatrix[2][0] = sin(angle);
  rMatrix[2][1] = cos(angle);
}
void matSetRotateGeneralLine(Matrix3D m, float a, float b, float c,
                          float uUn, float vUn, float wUn, float theta)
{
  matSetIdentity3D(m);
  float l = sqrt(uUn*uUn + vUn*vUn + wUn*wUn);
  if (l < TOLERANCE) 
   return;//the unit length is 0,
  // In this instance we normalize the direction vector.
  float u = uUn/l;
  float v = vUn/l;
  float w = wUn/l;
  // Set some intermediate values.
  float u2 = u*u;
  float v2 = v*v;
  float w2 = w*w;
  float cosT = cos(theta);
  float oneMinusCosT = 1-cosT;
  float sinT = sin(theta);
  // Build the matrix entries element by element.
  m[0][0] = u2 + (v2 + w2) * cosT;
  m[0][1] = u*v * oneMinusCosT - w*sinT;
  m[0][2] = u*w * oneMinusCosT + v*sinT;
  m[0][3] = (a*(v2 + w2) - u*(b*v + c*w))*oneMinusCosT
          + (b*w - c*v)*sinT;
  m[1][0] = u*v * oneMinusCosT + w*sinT;
  m[1][1] = v2 + (u2 + w2) * cosT;
  m[1][2] = v*w * oneMinusCosT - u*sinT;
  m[1][3] = (b*(u2 + w2) - v*(a*u + c*w))*oneMinusCosT
          + (c*u - a*w)*sinT;
  m[2][0] = u*w * oneMinusCosT - v*sinT;
  m[2][1] = v*w * oneMinusCosT + u*sinT;
  m[2][2] = w2 + (u2 + v2) * cosT;
  m[2][3] = (c*(u2 + v2) - w*(a*u + b*v))*oneMinusCosT
          + (a*v - b*u)*sinT;
}
void matSetObliqueProjectXY(Matrix3D m, float alpha, float cosTheta, float sinTheta)
{
  matSetIdentity3D(m);
  alpha   = alpha*M_PI/180;
  m[0][2] = alpha * cosTheta;
  m[1][2] = alpha * sinTheta;
  m[2][2] = 0;
}
void matSetObliqueProjectYZ(Matrix3D m, float alpha, float cosTheta, float sinTheta)
{
  matSetIdentity3D(m);
  alpha   = alpha*M_PI/180;
  m[1][0] = alpha * cosTheta;
  m[2][0] = alpha * sinTheta;
  m[0][0] = 0;
}
void matSetObliqueProjectXZ(Matrix3D m, float alpha, float cosTheta, float sinTheta)
{
  matSetIdentity3D(m);
  alpha   = alpha*M_PI/180;
  m[0][1] = alpha * cosTheta;
  m[2][1] = alpha * sinTheta;
  m[1][1] = 0;
}