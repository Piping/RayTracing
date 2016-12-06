//Declan Zhang
//999027783
#include "vertex3d.h"
#include "userinterface.h"
#include "matrix.h"
#include <float.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <string>
#include <vector>
#include <array>
#include <map>
#include <list>
#include <algorithm>
#include <memory>
#include <typeinfo>
using namespace std;


inline float clamp(float x) {return x<0 ? 0 :(x>1 ? 1 :x);}
//if intersect, return true and update distance and the sphere's id 
inline bool intersect(const Ray &r, float& t, int& id)
{
  float inf = t = 1e20;
  float d;
  // intersect all object in the scene and choose the closet one
  for (int i = ObjAr.size()-1; i >= 0; --i)
  {
    d = ObjAr[i]->intersect(r);
    if( d !=0 && d < t ){
      t = d;
      id = i;
    }
  }
  return t < inf;
}
//https://drive.google.com/file/d/0B8g97JkuSSBwUENiWTJXeGtTOHFmSm51UC01YWtCZw/view
//http://www.kevinbeason.com/smallpt/#moreinfo
Vec radiance(const Ray &r, int depth, unsigned short * Xi)
{
  float t; // distance to intersection 
  int id=0;                               // id of intersected object 
  if (!intersect(r, t, id)) return Vec(); // if miss, return black 
  const Object *obj = ObjAr[id];        // the hit object 
  Vec x  = r.o + r.d*t;
  Vec n  = (x-obj->p).norm();
  Vec nl = n.dot(r.d)<0?n:n*-1;
  Vec f  = obj->c; // the color that carried around
  float p = f.x>f.y && f.x>f.z ? f.x : f.y>f.z ? f.y : f.z; // max refl 
  if (++depth < UI.depth && erand48(Xi) < p)
    f=f*(1/p);
  else
    return obj->e;
  //
  if (obj->refl == DIFF)
  {// Ideal DIFFUSE reflection 
   // use random number to simulate diffuse angle
    float r1=2*M_PI*erand48(Xi), r2=erand48(Xi), r2s=sqrt(r2); 
    Vec w=nl;
    Vec u=((fabs(w.x)>.1?Vec(0,1):Vec(1))%w).norm();
    Vec v=w%u; 
    Vec d = (u*cos(r1)*r2s + v*sin(r1)*r2s + w*sqrt(1-r2)).norm(); 
    return obj->e + f*(radiance(Ray(x,d),depth,Xi)); 
  }
  else if (obj->refl == SPEC)
  {// Ideal SPECULAR reflection
   return obj->e + f*radiance(Ray(x,r.d-n*2*n.dot(r.d)),depth,Xi);
  }             
  else
  {//else refl is REFR 
    Ray reflRay(x, r.d-n*2*n.dot(r.d));     // Ideal dielectric REFRACTION 
    bool into = n.dot(nl)>0;                // Ray from outside going in? 
    float nc=1, nt=1.5, nnt=into?nc/nt:nt/nc, ddn=r.d.dot(nl), cos2t; 
    if ((cos2t=1-nnt*nnt*(1-ddn*ddn))<0)    // Total internal reflection 
      return obj->e + f*radiance(reflRay,depth,Xi); 
    Vec tdir = (r.d*nnt - n*((into?1:-1)*(ddn*nnt+sqrt(cos2t)))).norm(); 
    float a=nt-nc, b=nt+nc ;
    float R0=a*a/(b*b);
    float c = 1-(into?-ddn:tdir.dot(n)); 
    float Re=R0+(1-R0)*c*c*c*c*c;
    float Tr=1-Re;
    float P=.25+.5*Re;
    float RP=Re/P;
    float TP=Tr/(1-P); 
    return obj->e + f*(depth > UI.depth ? (erand48(Xi)<P ?   // Russian roulette 
      radiance(reflRay,depth,Xi)*RP:radiance(Ray(x,tdir),depth,Xi)*TP) : 
      radiance(reflRay,depth,Xi)*Re+radiance(Ray(x,tdir),depth,Xi)*Tr);
  }
}
//fov is radiant here
void rayTracingImg(float *img,int width,int height, float fov)
{
  int samples = (int)UI.samples;
  //fixed camera for now; world_up vector is (0,-0.042612,-1)
  Ray cam(UI.eyeFromPoint,UI.eyeWorldUp.norm());
  Vec cx = Vec(width*fov/height);
  Vec cy = (cx.cross(cam.d).norm())*fov;
  Vec r;//color
  Vec *c = new Vec[width*height];
  #pragma omp parallel for schedule(dynamic, 1) private(r) // OpenMP

  for(unsigned short x=0;  x<width; x++) // Loop cols 
  {
    for(unsigned short y=0; y<height; y++)
    { 
      unsigned short Xi[3]={0,0,(unsigned short)(x*x*x)}; 
      #if 0
      double r1=2*erand48(Xi);
      double dx=r1<1 ? sqrt(r1)-1: 1-sqrt(2-r1);
      double r2=2*erand48(Xi);
      double dy=r2<1 ? sqrt(r2)-1: 1-sqrt(2-r2);  
      Vec d = cx*(((0.5 + dx)/2 + x)/width -.5) + 
              cy*(((.5 + dy)/2 + y)/height -.5) + cam.d;;
      r = radiance(Ray(cam.o + d*140, d.norm()),0,Xi);
      //Camera rays are pushed ^^^^^ forward to start in interior
      c[y*width+x] = Vec(clamp(r.x),clamp(r.y),clamp(r.z));
      #endif
      #if 1
      //use 2x2 sample version
      for (int sy=0, i= y*width+x; sy<2; sy++) // 2x2 subpixel rows 
        for (int sx=0; sx<2; sx++, r=Vec()){  // 2x2 subpixel cols 
          for (int s=0; s<samples; s++){ 
            double r1=2*erand48(Xi), dx=r1<1 ? sqrt(r1)-1: 1-sqrt(2-r1); 
            double r2=2*erand48(Xi), dy=r2<1 ? sqrt(r2)-1: 1-sqrt(2-r2); 
            Vec d = cx*( ( (sx+.5 + dx)/2 + x)/width - .5) + 
                    cy*( ( (sy+.5 + dy)/2 + y)/height - .5) + cam.d; 
            r = r + radiance(Ray(cam.o+d*140, d.norm()),0,Xi) * (1.0/samples); 
          }  
          c[i] = c[i] + Vec(clamp(r.x),clamp(r.y),clamp(r.z))*.25; 
        }
      #endif
    }
  }
  for (int i = 0; i < width; ++i)
    for (int j = 0; j < height; ++j)
      setPIXELS(i,j,c[ j*width + i ].powerByScaler(1/2.2)+5/500.0);
  delete[] c;
}

Vector3 max(Vector3 a, Vector3 b)
{
  Vector3 ret;
  ret.x = a.x > b.x ? a.x:b.x;
  ret.y = a.y > b.y ? a.y:b.y;
  ret.z = a.z > b.z ? a.z:b.z;
  return ret;
}
void simpleUnitSphereRayTracing(int width,int height)
{//a simple raytracing display a sphere being traced
  Vector3 maxIntensity(0,0,0);
  Object *sphere = new Sphere(1,Vec(),Vec(),Color(0.75,0.25,0.25),DIFF);
  vector<vector<Color>> colors;
  vector<vector<Vector3>> pts;
  vector<vector<bool>> ambientBool;

  for (int i = 0; i < width; ++i)
  {
    pts.emplace_back();
    colors.emplace_back();
    ambientBool.emplace_back();
    for (int j = 0; j < height; ++j)
    {
      ambientBool[i].push_back(false);
      //map screen to world
      float x,y,z;
      x = 2.0*i/(width-1)-1;
      z = 2.0*j/(height-1)-1;
      //find intersection
      y = 1-x*x - z*z;
      if(y < 0)
      {
        ambientBool[i][j] = true;;
      }
      else
      {
        y = sqrt(y);
        float dist1 = UI.eyeFromPoint.y - y;
        float dist2 = UI.eyeFromPoint.y + y;
        y = dist1 > dist2 ? -y: y;
      }
      pts[i].emplace_back(x,y,z);
      //printf("World %4d %4d %4.12f %4.12f %4.12f\n",i,j,x,y,z);
      //Apply Phong into each piexl(corresponding vertex);
      Vector3 norm = pts[i][j].norm();
      //printf("normal %4d %4d %4.12f %4.12f %4.12f\n",i,j,norm.x,norm.y,norm.z);
      Vector3 vv = {0,-1,0};
      Vector3 ll = (UILights[UI.currLight].pos - pts[i][j]).norm();
      Vector3 rr = (norm *(2 * ll.dot(norm))) - ll;
      Intensity3 kAiA = UI.ambientColor;
      Intensity3 diff  = UIObjs[0].diffuseColor * norm.dot(ll);
      Intensity3 spec = (UIObjs[0].specularColor * rr.dot(vv)).powerByScaler(UIObjs[0].specularPower);     
      Intensity3 iL = UILights[i].color ;
      if((norm.dot(ll) > 0 && norm.dot(vv) < 0) || (norm.dot(ll) < 0 && norm.dot(vv) > 0))
      {
        iL = Vector3();//reset to 0
      }
      else if(rr.dot(vv) < 0)
      {
        spec = Vector3();
      }
      Intensity3 intensity = kAiA+ iL * (diff + spec);
      printf("Color %4d %4d %4.12f %4.12f %4.12f\n",i,j,intensity.x,intensity.y,intensity.z);
      colors[i].push_back(intensity);
      maxIntensity = max(maxIntensity,intensity);
    }
  }
  for (int i = 0; i < width; ++i)
  {
    for (int j = 0; j < height; ++j)
    {
      if(ambientBool[i][j])
        setPIXELS(i,j,UI.ambientColor);
      else
      {
        colors[i][j] = colors[i][j] / maxIntensity;
        setPIXELS(i,j,colors[i][j]);
      }
    }
  }
}
class Transformation
{
public:
  Vector3& v;
  Matrix3D trans;
  Transformation(Vec& _v):v(_v){}
  Vertex3D compute3DObjectCentroid()
  {
    Vertex3D centroid = {0, 0, 0};
    centroid = v;
    /*
    for (int i=0; i<v.size(); ++i)
    {
      centroid.x += v[i].x;
      centroid.y += v[i].y;
      centroid.z += v[i].z;
    }
    centroid.x /=v.size();
    centroid.y /=v.size();
    centroid.z /=v.size();
    */
    return centroid;
  }
  void translate3D(Vector3 t)
  {
    matSetTranslate3D(trans,t.x,t.y,t.z);
    matMultiply3D (trans,v);
    /*    
    for (auto i = v.begin(); i != v.end(); ++i)
    {
      matMultiply3D (trans,*i);
    }
    */
  }
  void rotateCentroid3D(float angle, Vertex3D l1,Vertex3D l2)
  {
    float a = l2.x-l1.x;
    float b = l2.y-l1.y;
    float c = l2.z-l1.z;
    matSetRotateGeneralLine(trans,l1.x,l1.y,l1.z,a,b,c,angle);
    matMultiply3D (trans,v);
    /*
    for (auto i = v.begin(); i != v.end(); ++i)
    {
      matMultiply3D (trans,*i);
    }
    */
  }
  void scaleCentroid3D(Vector3 s)
  {
    Vertex3D centroid = compute3DObjectCentroid();
    matSetScale3D(trans,s.x,s.y,s.z);
    translate3D(centroid*-1);
    matSetScale3D(trans,s.x,s.y,s.z);
    matMultiply3D(trans,v);
    /*
    for (auto i = v.begin(); i != v.end(); ++i)
    {
      matMultiply3D(trans,*i);
    }
    */
    translate3D(centroid);
  }
};

int main(int argc,char **argv)
{
    if (argc > 4)
    {
      printf("Please type: command window_width window_height xxx.scene\n");
      exit(0);
    }
    if( argc == 4 || argc == 2)
    {
      filename = string(argv[argc-1]); 
    }
    if( argc == 3)
    {
      W = atoi(argv[1]); H = atoi(argv[2]);
    }

    GLFWwindow *window;

    if (!glfwInit()) exit(EXIT_FAILURE);
    int Major, Minor, Rev;
    glfwGetVersion(&Major, &Minor, &Rev);
    printf("GLFW %d.%d.%d initialized\n", Major, Minor, Rev);

    window = glfwCreateWindow(W, H, "3D Graph System~Declan Zhang~999027783", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    GlfwCallbackSetup(window);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    //ReadFile first before Serup TweakBar
    #if 0
    ObjAr = {new Sphere(1e5, Vec( 1e5+1,40.8,81.6), Vec(),Vec(.75,.25,.25),DIFF),//Left 
            new Sphere(1e5, Vec(-1e5+99,40.8,81.6),Vec(),Vec(.25,.25,.75),DIFF),//Rght 
            new Sphere(1e5, Vec(50,40.8, 1e5),     Vec(),Vec(.75,.75,.75),DIFF),//Back 
            new Sphere(1e5, Vec(50,40.8,-1e5+170), Vec(),Vec(),           DIFF),//Frnt 
            new Sphere(1e5, Vec(50, 1e5, 81.6),    Vec(),Vec(.75,.75,.75),DIFF),//Botm 
            new Sphere(1e5, Vec(50,-1e5+81.6,81.6),Vec(),Vec(.75,.75,.75),DIFF),//Top 
            new Sphere(16.5,Vec(27,16.5,47),       Vec(),Vec(1,1,1)*.999, SPEC),//Mirr 
            new Sphere(16.5,Vec(73,16.5,78),       Vec(),Vec(1,1,1)*.999, REFR),//Glas 
            new Sphere(600, Vec(50,681.6-.27,81.6),Vec(12,12,12),  Vec(), DIFF) //Lite 
    };
    //Type Info experiment
    //string sn = typeid(spheres[0]).name();
    //string s = typeid(*spheres[0]).hash_code() == typeid(Sphere).hash_code() ? "It's Sphere": "It's object";
    //cout<<sn<<" : "<<endl;
    #endif  
    ReadFileSetup();         
    UI.TweakBarSetup();
    //###########MAINLOOP##########
    PIXELS = new float[W*H*3]();
    float *img = new float[300*300*3]();
    while (!glfwWindowShouldClose(window))
    {
      //Clear background Drawing
      glClearColor(0,0,0,0);
      glClear(GL_COLOR_BUFFER_BIT);
      if(UI.imageReqUpdate)
      {
        delete[] PIXELS;
        PIXELS = new float[W*H*3]();
        int i = UI.currObj;
        for (int i = 0; i < ObjAr.size(); ++i)
        {
          Transformation trans(ObjAr[i]->p);
          trans.scaleCentroid3D(UIObjs[i].scaleVec);
          trans.rotateCentroid3D(UIObjs[i].rotAngle,UIObjs[i].rotAxP1,UIObjs[i].rotAxP2);
          trans.translate3D(UIObjs[i].transVec);
          ObjAr[i]->refl = float_to_reflt(UIObjs[i].refl);
          ObjAr[i]->c    = UIObjs[i].color   ;
          ObjAr[i]->e    = UIObjs[i].emission;
          ObjAr[i]->p    = UIObjs[i].pos;
          if (ObjAr[i]->name == "Sphere")
            ((Sphere*)ObjAr[i])->rad    = UIObjs[i].radius;
          else if(ObjAr[i]->name == "Plane")
            ((Plane*)ObjAr[i])->n    = UIObjs[i].norm;
        }
        rayTracingImg(img,UI.imgHeight,UI.imgWidth,UI.fov);
        UI.imageReqUpdate = false;
      }
      UI.TweakBarUpdate();
      glDrawPixels( W, H, GL_RGB, GL_FLOAT, PIXELS );
      TwDraw();
      glfwSwapBuffers(window);
      glfwPollEvents();
    }
    delete[] PIXELS;
    SaveAndCloseFile();
    glfwDestroyWindow(window);
    glfwTerminate();
    printf("GLFW %d.%d.%d terminated\n", Major, Minor, Rev);
    exit(EXIT_SUCCESS);
}
