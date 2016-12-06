#ifndef GRAPHIC_3D
#define GRAPHIC_3D
#include <array>
#include <math.h>
#include <stdlib.h>
#include "vertex3d.h"
#include <time.h>

using namespace std;

enum Refl_t {DIFF,SPEC,REFR};
struct Ray{
  Vec o,d; //origin and direction(norm)
  Ray(Vec _origin,Vec _direction)
  :o(_origin),d(_direction){}
};
struct Object{
  string name="Object";
  Vec p,e,c; //center position, emission, color
  Refl_t refl; //Diffuse,specular,reflective
  Object(Vec _p,Vec _e,Vec _c,Refl_t _refl):p(_p),e(_e),c(_c),refl(_refl){}
  virtual float intersect(const Ray& r)const =0;
};
struct Sphere : Object{
  float rad;  //radius
  //constructor
  Sphere(float _rad,Vec _p,Vec _e,Vec _c,Refl_t _refl)
  :Object(_p, _e, _c,_refl),rad(_rad){name="Sphere";}
  //calc distance from ray to intersected point in sphere
  //return 0 if no intersection
  virtual float intersect(const Ray& r)const{
    Vec op = r.o - p;
    float t, eps = 1e-4;
    //float a = d.dot(d); //is 1 if norm
    float b = r.d.dot(op);
    float det = b*b - op.dot(op)+rad*rad; //(b^2-4ac)/4
    if(det < 0 ) return 0;
    else det = sqrt(det);
    return (t=-b-det)>eps ? t : ((t=-b+det) > eps ? t : 0);
    //return smaller positive t
  }
};
struct Plane : Object{
  Vec n;
  Plane(Vec _n,Vec _p,Vec _e,Vec _c,Refl_t _refl)
  :Object(_p, _e, _c,_refl),n(_n){name="Plane";}
  virtual float intersect(const Ray& r)const{
    // t = -(O*N-d)/D*N
    return (-r.o.dot(n) + p.x+p.y+p.z)/ r.d.dot(n);
  }
};
#endif

