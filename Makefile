graphic.out: graphic.cpp Makefile userinterface.h graphic.h matrix.h vertex3d.h piexl.h
	g++ graphic.cpp -fopenmp -lpthread -O3 -std=c++11 -Wl,-rpath '-Wl,$$ORIGIN' ./libAntTweakBar.so.1 -o graphic.out  -lGL -lGLU -lglfw -lXi 

clean:
	rm -f graphic.out
