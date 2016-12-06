#ifndef VERTEX3D_H
#define VERTEX3D_H

#include <math.h>
#include <array>
class Vertex3D
{
public:
	float x,y,z;
public:
  Vertex3D(float x0 = 0, float y0=0,float z0=0){x=x0; y=y0; z=z0;}
  Vertex3D(float[3]);
  Vertex3D(std::array<float, 3>);
public:
  float    length();
  Vertex3D norm();
  float    dot(const Vertex3D v)const{return x*v.x + y*v.y + z*v.z;}
  Vertex3D cross(const Vertex3D v)const{return Vertex3D(y*v.z-z*v.y, z*v.x-x*v.z, x*v.y-y*v.x);}
  Vertex3D operator%(const Vertex3D v)const{return Vertex3D(y*v.z-z*v.y, z*v.x-x*v.z, x*v.y-y*v.x);}
  Vertex3D operator+(const Vertex3D v)const{return Vertex3D(x+v.x,y+v.y,z+v.z);}
  Vertex3D operator-(const Vertex3D v)const{return Vertex3D(x-v.x,y-v.y,z-v.z);}
  Vertex3D operator/(float d) const {return Vertex3D(x/d, y/d, z/d);}
  Vertex3D operator*(float d) const {return Vertex3D(x*d,y*d,z*d);}
  Vertex3D operator/(Vertex3D d)const {return Vertex3D(x/d.x, y/d.y, z/d.z);}
  Vertex3D operator*(Vertex3D d)const {return Vertex3D(x*d.x, y*d.y, z*d.z);}
  void operator+=(Vertex3D);
  Vertex3D powerByScaler(float d);

};
typedef Vertex3D Vector3,Intensity3,Vec,Color;

Vertex3D::Vertex3D(float c[3])
{
  x=c[0];
  y=c[1];
  z=c[2];
}
Vertex3D::Vertex3D(std::array<float, 3> cl)
{
  x = cl[0];
  y = cl[1];
  z = cl[2];
}
float Vertex3D::length()
{
  return sqrt(x*x + y*y + z*z);
}

void Vertex3D::operator+=(Vertex3D v)
{
  x += v.x;
  y += v.y;
  z += v.z;
}
Vertex3D Vertex3D::norm()
{
  float l = length();
  return Vertex3D(x/l, y/l, z/l);
}
Vertex3D Vertex3D::powerByScaler(float d)
{
  return Vertex3D(pow(x,d),pow(y,d),pow(z,d));
}
#endif //end of header file