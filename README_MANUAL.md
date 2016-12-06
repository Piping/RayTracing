####Name: Declan Zhang
#####SID:999027783

Hi, This is the manual sheet for my small ray tracing program.

#####Intro:
The most operations are in a GUI form that people can click on or type and is straightforward.
You can also check the left bottom corner the help icon for more information on each menu item.
The 3d-objects are read from file and will save change to the file after program ends.

Objective:
* All the requirement is implemented. 
* UI is well designed and deserves extra points.
* Animation is supported.(Turn on autorotate in the menu to enable animation)
* Multiple light sources support

Note:
1. the program supports multiple objects(Plane and Sphere). All option can be found in UI menu.
2. **User should click button in Action group to update the image;otherwise change will not be made.**
3. the user need to wait for about 2s to see the image showing up. 
4. the sample per piexl(spp) stands for the recursion/depth/detail of the image,larger spp takes much longer time to illuminate. 10 spp take around 20s on my machine, 100 spp tooks around 4 mins at 500x500 resolution;
5. **the user can choose different images that exist in the same directory( forest.scene, sky.scene, island.scene, moon.scene, box.scene)**
#####How to compile and run:
	make && ./graphic.out 
    make && ./graphic.out 850 500
    make && ./graphic.out forest.scene 
    make && ./graphic.out 850 500 box.scene

#####Where is the fucntions:
	1. Phong global illumination formula implemented in radiance function(Line # 42) in graphic.cpp file. The whole ray tracing step is implemented in rayTracingImg function(Line# 85). The Intersection function for a sphere is in line#31 in graphic.h file. The intersection function of all objects in scene is in line#25 in graphic.cpp file.
