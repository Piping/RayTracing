#ifndef USER_INTERFACE
#define USER_INTERFACE

#include <GLFW/glfw3.h>
#include "AntTweakBar.h"
#include <array>
#include <vector>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <float.h>
#include "vertex3d.h"
#include "piexl.h"
#include "graphic.h"
#include <typeinfo>
using namespace std;

//Global Varibale
static ifstream ifs;
static ofstream ofs;
static string filename ="box.scene";
//Comment as an example, array and Vertex3D can be used interchangeably
//vector<array<float,3>> == vector<Vertex3D>

static TwBar *TWBAR;
static TwType vec3Type;

vector<Object*> ObjAr;

struct UIGlobal
{
  TwBar *bar;
  //RayTracing
  float samples;
  float imgHeight;
  float imgWidth;
  float fov; //in rad
  Vec eyeFromPoint;
  Vec eyeWorldUp;
  bool imageReqUpdate; //imgUpdate
  float depth;
  //
  bool displayHalfTone;
  Vertex3D ambientColor;
  int lastObj;
  int currObj;
  int lastLight;
  int currLight;
  bool uiNeedUpdate; //UIUpdate
  UIGlobal()
  {
    samples   = 10;
    imgHeight = 500;
    imgWidth  = 500;
    fov       = 0.5;
    depth     = 10;
    eyeFromPoint = {50,52,295.6};
    eyeWorldUp = {0,-0.042612,-1};
    imageReqUpdate = true;
    displayHalfTone = false;
    ambientColor = {0.6,0.2,0.9};
    lastObj = 0;
    currObj = 0;
    lastLight = 0;
    currLight = 0;
    uiNeedUpdate = false;
  }
  void TweakBarAddEntry();
  void TweakBarSetup();
  void TweakBarUpdate();
};
struct UIObject
{
  string menu;
  float refl;

  float radius;//sphere
  Vec norm;//plane
  Vec emission;
  Vec pos;
  Color color;
  string type;

  Vertex3D transVec;
  Vertex3D scaleVec;
  Vertex3D rotAxP1;
  Vertex3D rotAxP2;
  float rotAngle;
  float rotSpeed;
  bool  autoRot;
  bool  showAxis;
  bool  showShade;

  Vertex3D diffuseColor;
  Vertex3D specularColor;
  int specularPower;
  UIObject()
  {
    menu = "";
    type = "";
    transVec = {0,0,0};
    scaleVec = {1,1,1};
    rotAxP1  = {0,0,0};
    rotAxP2  = {100,0,0};
    rotAngle = 0;
    rotSpeed = 0.1;
    autoRot  = false;
    showAxis = false;
    showShade = true;
    diffuseColor = {0.3,0.4,0.5};
    specularColor = {0.3,0.4,0.5};
    specularPower = 5;
  }
  void addObjectEntry();
};
struct UILight
{
  string menu;
  Vertex3D color;
  Vertex3D pos;
  UILight();
  void addLightEntry();
};
static UIGlobal UI;
static vector<UIObject> UIObjs;
static vector<UILight> UILights;

//##########################################
//####### File I/O Part#####################
//##########################################
float refl_to_float(Refl_t refl)
{
  return (refl == DIFF ? 0: (refl == SPEC ? 1 : 2));
}
Refl_t float_to_reflt(float material)
{
  return (material == 0 ? DIFF: (material == 1? SPEC : REFR));
}
static void ReadFileSetup()
{
  printf("Open the File %s\n",filename.c_str());
  ifs.open(filename);
  if(!ifs.is_open())
  {
    printf("File %s Not Found.\n",filename.c_str());
    exit(0);
  }
  int numObject;
  float x,y,z,ex,ey,ez,r,g,b,rad;
  float nx,ny,nz;
  int material;
  int type;
  char ignore;
  ifs>>numObject;
  for (int i = 0; i < numObject; ++i)
  {
    ifs>>type>>ignore;
    if(type == 1) //Sphere
    {
      ifs>>rad>>ignore;
    }
    else if(type == 0) //Plane
    {
      ifs>>nx>>ny>>nz>>ignore;
    }
    ifs>>x>>y>>z>>ignore;
    ifs>>ex>>ey>>ez>>ignore;
    ifs>>r>>g>>b>>ignore;
    ifs>>material;
    Refl_t mat = float_to_reflt(material);
    if(type == 1) //Sphere
      ObjAr.push_back(new Sphere(rad,Vec(x,y,z),Vec(ex,ey,ez),Vec(r,g,b),mat));
    else if(type == 0)
      ObjAr.push_back(new Plane(Vec(nx,ny,nz),Vec(x,y,z),Vec(ex,ey,ez),Vec(r,g,b),mat));
  }
}
static void SaveAndCloseFile()
{
  ofs.open(filename);
  ofs<<ObjAr.size()<<endl;
  int material;
  int type;
  for (int i = 0; i < ObjAr.size(); ++i)
  {
    type = ObjAr[i]->name == "Sphere" ? 1:0;
    ofs<<type<<", ";
    if(type == 1) //Sphere
    {
      ofs<<right<<setw(9)<<((Sphere*)ObjAr[i])->rad <<",";
    }
    else if(type == 0)
    {
      Plane* obj = ((Plane*)ObjAr[i]);
      ofs<<right<<setw(9)<<obj->n.x<<setw(9)<< obj->n.y<<setw(9)<< obj->n.z <<",";
    }
    ofs<<right<<setw(9)<<ObjAr[i]->p.x<<setw(9)<< ObjAr[i]->p.y<<setw(9)<< ObjAr[i]->p.z <<",";
    ofs<<right<<setw(9)<<ObjAr[i]->e.x<<setw(9)<<ObjAr[i]->e.y<<setw(9)<<ObjAr[i]->e.z<<",";
    ofs<<right<<setw(9)<<ObjAr[i]->c.x<<setw(9)<<ObjAr[i]->c.y<<setw(9)<<ObjAr[i]->c.z<<",";
    material = refl_to_float(ObjAr[i]->refl);
    ofs<<right<<setw(4)<< material <<endl;
  }
  ofs.close();
  printf("Save the file %s\n",filename.c_str() );
}
//##########################################
//####### User Interface Part###############
//##########################################
//  Callback function called when the 'ON/FF CLICKED' 'SPACE PRESSED' variable value of the tweak bar has changed
void TW_CALL SetToggleCB(const void * value,void *clientData)
{
  *(bool*)clientData = *(bool*)clientData ? false : true;
}
void TW_CALL GetToggleCB(void * value,void *clientData)
{
  *(bool *)value = *(bool*)clientData;
}
void TW_CALL SetVector3UpdateCB(const void * value,void *clientData)
{
  *(Vertex3D*)clientData = *(Vertex3D*)value;
  UI.uiNeedUpdate = true;
}
void TW_CALL GetVector3UpdateCB(void * value,void *clientData)
{
  *(Vertex3D *)value = *(Vertex3D*)clientData;
}
void TW_CALL SetFloatUpdateCB(const void * value,void *clientData)
{
  *(float*)clientData = *(float*)value;
  UI.uiNeedUpdate = true;
}
void TW_CALL GetFloatUpdateCB(void * value,void *clientData)
{
  *(float *)value = *(float*)clientData;
}
void TW_CALL AddNewLightSource(void * clientData)
{
  UILights.emplace_back();
  UI.currLight  = UILights.size()-1;
}
void TW_CALL updateImg(void * clientData)
{
  UI.imageReqUpdate = true;
}
UILight::UILight()
{
  menu  = "";
  color = {1,1,1};
  pos   = {1000,1000,1000}; 
}
void UIObject::addObjectEntry()
{
  // Create a new TwType to edit 2D points: a struct that contains two floats
  TwAddVarCB(TWBAR, "Translate", vec3Type, SetVector3UpdateCB, GetVector3UpdateCB, &transVec, 
              " Group='Object' Label='Translate'"
              " help='Translate the selected Object on X,Y,Z direction' ");
  TwAddVarCB(TWBAR, "pos", vec3Type, SetVector3UpdateCB, GetVector3UpdateCB, &pos, 
             " Group='Object' Label='Pos' ");
  TwAddVarCB(TWBAR, "color", vec3Type, SetVector3UpdateCB, GetVector3UpdateCB, &color, 
             " Group='Object' Label='Color' ");
  TwAddVarCB(TWBAR, "emission", vec3Type, SetVector3UpdateCB, GetVector3UpdateCB, &emission, 
             " Group='Object' Label='Emission'");
  TwAddVarCB(TWBAR, "refl", TW_TYPE_FLOAT,SetFloatUpdateCB,GetFloatUpdateCB, &refl, 
           " label='Material Type: ' group=Object  step=1 min=0 max=2"); 
  if(type == "Sphere")
  {
    TwAddVarCB(TWBAR, "radius", TW_TYPE_FLOAT,SetFloatUpdateCB,GetFloatUpdateCB, &radius, 
         " label='Radius: ' group=Object"); 
  }
  else if(type == "Plane")
  {
    TwAddVarCB(TWBAR, "normal", vec3Type, SetVector3UpdateCB, GetVector3UpdateCB, &norm, 
               " Group='Object' Label='Plane Normal'");
  }  
/*
  TwAddVarCB(TWBAR, "ScaleFactor", vec3Type,SetVector3UpdateCB, GetVector3UpdateCB, &scaleVec, 
              " Group='Object' Label='ScaleFactor' "
              " help='Scale the selected Object on X,Y,Z direction' ");
  TwAddVarCB(TWBAR, "RotationAxisP1", vec3Type,SetVector3UpdateCB, GetVector3UpdateCB, &rotAxP1, 
              " Group='Object' Label='RotationAxis P1' "
              " help='Rotate the selected Object around the axis' ");
  TwAddVarCB(TWBAR, "RotationAxisP2", vec3Type,SetVector3UpdateCB, GetVector3UpdateCB, &rotAxP2, 
              " Group='Object' Label='RotationAxis P2' "
              " help='Rotate the selected Object around the axis' ");
  TwAddVarCB(TWBAR, "RotationAngle", TW_TYPE_FLOAT,SetFloatUpdateCB, GetFloatUpdateCB, &rotAngle, 
               " label='Rotation Angle: ' group=Object  step=0.01 "
               " help='Rotate the selected Object' ");
  */
}
void UILight::addLightEntry()
{
  TwAddVarCB(TWBAR, "LightColor", TW_TYPE_COLOR3F,SetVector3UpdateCB, GetVector3UpdateCB, &color, 
             " label='Light Color: ' group=ColorAndLight help='Change the light color' "); 
  TwAddVarCB(TWBAR, "LightPos", vec3Type, SetVector3UpdateCB, GetVector3UpdateCB,&pos, 
             " label='Light Position: ' group=ColorAndLight help='Change the light source position' ");  
}
/*
Legacy Code Removed
  TwAddVarCB(TWBAR, "DISPLAYHALFTONE", TW_TYPE_BOOL32,
             SetToggleCB,GetToggleCB,&UI.displayHalfTone,
             " label='Use Halftone Mode'");
  TwDefine(" Main/DISPLAYHALFTONE group=Screen key=2 true='ON' false='OFF' help='Choose to Use Halftone or not.' ");
*/
void UIGlobal::TweakBarAddEntry()
{
  TwAddVarRO(TWBAR, "ScreenSizeHeight", TW_TYPE_INT32, &H, " group=Screen label='Height' ");
  TwAddVarRO(TWBAR, "ScreenSizeWidth", TW_TYPE_INT32, &W, " group=Screen label='Width' ");
  //RayTracing
  TwAddVarCB(TWBAR, "SPP", TW_TYPE_FLOAT,SetFloatUpdateCB,GetFloatUpdateCB, &samples, 
           " label='Samples Per Piexl: ' group=RayTracing  step=1 min=1"
           " help='Samples Per Piexl' ");
  TwAddVarCB(TWBAR, "imgHeight", TW_TYPE_FLOAT,SetFloatUpdateCB,GetFloatUpdateCB, &imgHeight, 
           " label='Image Height: ' group=RayTracing  step=1 min=1 max=500");
  TwAddVarCB(TWBAR, "imgWidth", TW_TYPE_FLOAT,SetFloatUpdateCB,GetFloatUpdateCB, &imgWidth, 
           " label='Image Width: ' group=RayTracing  step=1 min=1 max=500");  
  TwAddVarCB(TWBAR, "fov", TW_TYPE_FLOAT,SetFloatUpdateCB,GetFloatUpdateCB, &fov, 
           " label='Field of View(Radian): ' group=RayTracing  step=0.0001 min=0.1 max=1");
  TwAddVarCB(TWBAR, "eyeOrCamera", vec3Type,SetVector3UpdateCB, GetVector3UpdateCB, &eyeFromPoint, 
             " label='Eye Position: ' group=RayTracing  help='Change the eye' ");
  TwAddVarCB(TWBAR, "eyeOrCameraUP", vec3Type,SetVector3UpdateCB, GetVector3UpdateCB, &eyeWorldUp, 
             " label='World Up Vector: ' group=RayTracing  help='Change the up vector' ");
  TwAddVarCB(TWBAR, "recurDepth", TW_TYPE_FLOAT,SetFloatUpdateCB,GetFloatUpdateCB, &depth, 
           " label='Recursive Depth: ' group=RayTracing  step=1 min=1"); 
  while(UIObjs.size() < ObjAr.size())
    UIObjs.emplace_back();  
  // Object dropdown menu
  {
    int size = ObjAr.size();
    TwEnumVal *objectEnum = new TwEnumVal[size];
    for (int i = 0; i < size; ++i)
    {
        UIObjs[i].menu = ObjAr[i]->name+" #" + to_string(i);
        switch(ObjAr[i]->refl)
        {
          case DIFF: UIObjs[i].menu += ((ObjAr[i]->e.x ==0 && ObjAr[i]->e.y ==0 && ObjAr[i]->e.z ==0)
           ? ":ambient " : ":light src") ;break;
          case SPEC: UIObjs[i].menu +=  ":solid ";break;
          case REFR: UIObjs[i].menu +=  ":glass ";
        }
        //UIObjs[i].menu = typeid(*ObjAr[i]).name()+ " " + to_string(i);
        UIObjs[i].type = ObjAr[i]->name;
        UIObjs[i].pos = ObjAr[i]->p;
        UIObjs[i].refl = refl_to_float(ObjAr[i]->refl);
        UIObjs[i].color= ObjAr[i]->c;
        UIObjs[i].emission = ObjAr[i]->e;
        if(UIObjs[i].type == "Sphere")
          UIObjs[i].radius = ((Sphere*)ObjAr[i])->rad;
        if(UIObjs[i].type == "Plane")
          UIObjs[i].norm = ((Plane*)ObjAr[i])->n;
        objectEnum[i]  = {i,UIObjs[i].menu.c_str()};
    }
    //TwEnumVal objectEnum[3] = {{0,"Object #1"},{1,"Object #2"},{2,"Object #3"}};
    TwType objectEnumType = TwDefineEnum("ObjectEnum", objectEnum, size);
    // add 'g_CurrentShape' to 'bar': this is a variable of type ShapeType.
    TwAddVarRW(TWBAR, "SelectedShape", objectEnumType, &currObj, 
      " group=Object help='Change selected object.' ");
  }
  UIObjs[currObj].addObjectEntry(); 
  TwAddButton(TWBAR, "updateImg", updateImg, NULL, 
          " label='Click to Update image' group=Action ");
}
void UIGlobal::TweakBarSetup()
{
  string MainBarDefine = " Main refresh=0.01 fontSize=2 valuesWidth=140 position='";
  //now only for one light source
  UILights.emplace_back();
  TwInit(TW_OPENGL, NULL);
  TwWindowSize(W, H);
  TWBAR = TwNewBar("Main");
  TwDefine(" Main label='~ 3D Object Control Panel ~'");
  MainBarDefine += to_string(W-349) +" "+to_string(0)+"' " +"size='350 "+to_string(H)+"'";
  TwDefine(MainBarDefine.c_str());
  TwStructMember vec3DMembers[] = { 
      { "X", TW_TYPE_FLOAT, offsetof(Vertex3D, x), "" },
      { "Y", TW_TYPE_FLOAT, offsetof(Vertex3D, y), "" },
      { "Z", TW_TYPE_FLOAT, offsetof(Vertex3D, z), "" }, };
  vec3Type = TwDefineStruct("VECTOR", vec3DMembers, 3, sizeof(Vertex3D), NULL, NULL);
  UI.TweakBarAddEntry();
}
void UIGlobal::TweakBarUpdate()
{
  if(lastObj != currObj)
  {
    TwRemoveAllVars(TWBAR);
    TweakBarAddEntry();
    lastObj   = currObj;
    lastLight = currLight;
  }
}
//###########GLFW CALLBACK SETUP###############
static void window_size_callback(GLFWwindow* window, int width, int height)
{
  TwWindowSize(width, height);
  glViewport(0, 0, width, height);
  W = width; H = height;
  UI.uiNeedUpdate = true;
}
static void error_callback(int error, const char* description)
{
    fputs(description, stderr);
}
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GL_TRUE);
  else if(action == GLFW_PRESS)
  {
    TwEventKeyGLFW3(window,key,scancode,action,mods);
    TwKeyPressed(key,mods);
  }
}
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
  TwEventMousePosGLFW(xpos,ypos);
}
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{   
  TwEventMouseButtonGLFW(button, action);
}
static void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
  TwEventMouseWheelGLFW(yoffset);
}
static void GlfwCallbackSetup(GLFWwindow * window)
{
  glfwSetErrorCallback(error_callback);
  glfwSetKeyCallback(window, key_callback);
  glfwSetCursorPosCallback(window  , cursor_position_callback);
  glfwSetMouseButtonCallback(window, mouse_button_callback);
  glfwSetWindowSizeCallback(window , window_size_callback);
  glfwSetScrollCallback(window,  mouse_scroll_callback);
}

#endif