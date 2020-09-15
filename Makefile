all:
	mkdir -p obj/Release/src/3rdparty/claudette
	g++ -Wall -DFOUG_OS_UNIX -Os -I../src -I../src/3rdparty -I../src/3rdparty/lodepng -c ../src/3rdparty/claudette/base_collision_test.cpp -o obj/Release/src/3rdparty/claudette/base_collision_test.o
	g++ -Wall -DFOUG_OS_UNIX -Os -I../src -I../src/3rdparty -I../src/3rdparty/lodepng -c ../src/3rdparty/claudette/box.cpp -o obj/Release/src/3rdparty/claudette/box.o
	g++ -Wall -DFOUG_OS_UNIX -Os -I../src -I../src/3rdparty -I../src/3rdparty/lodepng -c ../src/3rdparty/claudette/box_bld.cpp -o obj/Release/src/3rdparty/claudette/box_bld.o
	g++ -Wall -DFOUG_OS_UNIX -Os -I../src -I../src/3rdparty -I../src/3rdparty/lodepng -c ../src/3rdparty/claudette/collision_model_3d.cpp -o obj/Release/src/3rdparty/claudette/collision_model_3d.o
	g++ -Wall -DFOUG_OS_UNIX -Os -I../src -I../src/3rdparty -I../src/3rdparty/lodepng -c ../src/3rdparty/claudette/math3d.cpp -o obj/Release/src/3rdparty/claudette/math3d.o
	g++ -Wall -DFOUG_OS_UNIX -Os -I../src -I../src/3rdparty -I../src/3rdparty/lodepng -c ../src/3rdparty/claudette/model_collision_test.cpp -o obj/Release/src/3rdparty/claudette/model_collision_test.o
	g++ -Wall -DFOUG_OS_UNIX -Os -I../src -I../src/3rdparty -I../src/3rdparty/lodepng -c ../src/3rdparty/claudette/mytritri.cpp -o obj/Release/src/3rdparty/claudette/mytritri.o
	g++ -Wall -DFOUG_OS_UNIX -Os -I../src -I../src/3rdparty -I../src/3rdparty/lodepng -c ../src/3rdparty/claudette/ray_collision_test.cpp -o obj/Release/src/3rdparty/claudette/ray_collision_test.o
	g++ -Wall -DFOUG_OS_UNIX -Os -I../src -I../src/3rdparty -I../src/3rdparty/lodepng -c ../src/3rdparty/claudette/sphere_collision_test.cpp -o obj/Release/src/3rdparty/claudette/sphere_collision_test.o
	g++ -Wall -DFOUG_OS_UNIX -Os -I../src -I../src/3rdparty -I../src/3rdparty/lodepng -c ../src/3rdparty/claudette/sysdep.cpp -o obj/Release/src/3rdparty/claudette/sysdep.o
	gcc -Wall -DFOUG_OS_UNIX -Os -I../src -I../src/3rdparty -I../src/3rdparty/lodepng -c ../src/3rdparty/claudette/tritri.c -o obj/Release/src/3rdparty/claudette/tritri.o
	mkdir -p obj/Release/src/3rdparty/glm/detail
	g++ -Wall -DFOUG_OS_UNIX -Os -I../src -I../src/3rdparty -I../src/3rdparty/lodepng -c ../src/3rdparty/glm/detail/glm.cpp -o obj/Release/src/3rdparty/glm/detail/glm.o
	mkdir -p obj/Release/src/3rdparty/glmmodel
	g++ -Wall -DFOUG_OS_UNIX -Os -I../src -I../src/3rdparty -I../src/3rdparty/lodepng -c ../src/3rdparty/glmmodel/glmmodel.cpp -o obj/Release/src/3rdparty/glmmodel/glmmodel.o
	mkdir -p obj/Release/src/3rdparty/lodepng
	g++ -Wall -DFOUG_OS_UNIX -Os -I../src -I../src/3rdparty -I../src/3rdparty/lodepng -c ../src/3rdparty/lodepng/lodepng.cpp -o obj/Release/src/3rdparty/lodepng/lodepng.o
	mkdir -p obj/Release/src/audio
	g++ -Wall -DFOUG_OS_UNIX -Os -I../src -I../src/3rdparty -I../src/3rdparty/lodepng -c ../src/audio/soundmanager.cpp -o obj/Release/src/audio/soundmanager.o
	
	g++ -Wall -DFOUG_OS_UNIX -Os -I../src -I../src/3rdparty -I../src/3rdparty/lodepng -c ../src/main.cpp -o obj/Release/src/main.o
	mkdir -p obj/Release/src/objects
	g++ -Wall -DFOUG_OS_UNIX -Os -I../src -I../src/3rdparty -I../src/3rdparty/lodepng -c ../src/objects/aabbcollider.cpp -o obj/Release/src/objects/aabbcollider.o
	g++ -Wall -DFOUG_OS_UNIX -Os -I../src -I../src/3rdparty -I../src/3rdparty/lodepng -c ../src/objects/complexcollider.cpp -o obj/Release/src/objects/complexcollider.o
	g++ -Wall -DFOUG_OS_UNIX -Os -I../src -I../src/3rdparty -I../src/3rdparty/lodepng -c ../src/objects/cylindercollider.cpp -o obj/Release/src/objects/cylindercollider.o
	g++ -Wall -DFOUG_OS_UNIX -Os -I../src -I../src/3rdparty -I../src/3rdparty/lodepng -c ../src/objects/drone.cpp -o obj/Release/src/objects/drone.o
	g++ -Wall -DFOUG_OS_UNIX -Os -I../src -I../src/3rdparty -I../src/3rdparty/lodepng -c ../src/objects/dronemanager.cpp -o obj/Release/src/objects/dronemanager.o
	g++ -Wall -DFOUG_OS_UNIX -Os -I../src -I../src/3rdparty -I../src/3rdparty/lodepng -I/usr/include/freetype2 -L/usr/local/lib -lfreetype -c ../src/objects/hud.cpp -o obj/Release/src/objects/hud.o
	g++ -Wall -DFOUG_OS_UNIX -Os -I../src -I../src/3rdparty -I../src/3rdparty/lodepng -c ../src/objects/object.cpp -o obj/Release/src/objects/object.o
	g++ -Wall -DFOUG_OS_UNIX -Os -I../src -I../src/3rdparty -I../src/3rdparty/lodepng -c ../src/objects/player.cpp -o obj/Release/src/objects/player.o
	g++ -Wall -DFOUG_OS_UNIX -Os -I../src -I../src/3rdparty -I../src/3rdparty/lodepng -c ../src/objects/sign.cpp -o obj/Release/src/objects/sign.o
	g++ -Wall -DFOUG_OS_UNIX -Os -I../src -I../src/3rdparty -I../src/3rdparty/lodepng -c ../src/objects/treemanager.cpp -o obj/Release/src/objects/treemanager.o
	mkdir -p obj/Release/src/particles
	g++ -Wall -DFOUG_OS_UNIX -Os -I../src -I../src/3rdparty -I../src/3rdparty/lodepng -c ../src/particles/particle.cpp -o obj/Release/src/particles/particle.o
	g++ -Wall -DFOUG_OS_UNIX -Os -I../src -I../src/3rdparty -I../src/3rdparty/lodepng -c ../src/particles/particleconfig.cpp -o obj/Release/src/particles/particleconfig.o
	g++ -Wall -DFOUG_OS_UNIX -Os -I../src -I../src/3rdparty -I../src/3rdparty/lodepng -c ../src/particles/particlelist.cpp -o obj/Release/src/particles/particlelist.o
	g++ -Wall -DFOUG_OS_UNIX -Os -I../src -I../src/3rdparty -I../src/3rdparty/lodepng -c ../src/particles/particlemanager.cpp -o obj/Release/src/particles/particlemanager.o
	mkdir -p obj/Release/src/util
	g++ -Wall -DFOUG_OS_UNIX -Os -I../src -I../src/3rdparty -I../src/3rdparty/lodepng -c ../src/util/gldebugging.cpp -o obj/Release/src/util/gldebugging.o
	g++ -Wall -DFOUG_OS_UNIX -Os -I../src -I../src/3rdparty -I../src/3rdparty/lodepng -c ../src/util/image.cpp -o obj/Release/src/util/image.o
	g++ -Wall -DFOUG_OS_UNIX -Os -I../src -I../src/3rdparty -I../src/3rdparty/lodepng -c ../src/util/loadtexture.cpp -o obj/Release/src/util/loadtexture.o
	g++ -Wall -DFOUG_OS_UNIX -Os -I../src -I../src/3rdparty -I../src/3rdparty/lodepng -c ../src/util/math.cpp -o obj/Release/src/util/math.o
	g++ -Wall -DFOUG_OS_UNIX -Os -I../src -I../src/3rdparty -I../src/3rdparty/lodepng -c ../src/util/planerenderer.cpp -o obj/Release/src/util/planerenderer.o
	g++ -Wall -DFOUG_OS_UNIX -Os -I../src -I../src/3rdparty -I../src/3rdparty/lodepng -c ../src/util/profiling.cpp -o obj/Release/src/util/profiling.o
	g++ -Wall -DFOUG_OS_UNIX -Os -I../src -I../src/3rdparty -I../src/3rdparty/lodepng -c ../src/util/shader.cpp -o obj/Release/src/util/shader.o
	mkdir -p obj/Release/src/world
	g++ -Wall -DFOUG_OS_UNIX -Os -I../src -I../src/3rdparty -I../src/3rdparty/lodepng -c ../src/world/grassmanager.cpp -o obj/Release/src/world/grassmanager.o
	g++ -Wall -DFOUG_OS_UNIX -Os -I../src -I../src/3rdparty -I../src/3rdparty/lodepng -c ../src/world/sky.cpp -o obj/Release/src/world/sky.o
	g++ -Wall -DFOUG_OS_UNIX -Os -I../src -I../src/3rdparty -I../src/3rdparty/lodepng -c ../src/world/terrain.cpp -o obj/Release/src/world/terrain.o
	g++ -Wall -DFOUG_OS_UNIX -Os -I../src -I../src/3rdparty -I../src/3rdparty/lodepng -c ../src/world/world.cpp -o obj/Release/src/world/world.o

	g++  -o PUBG obj/Release/src/3rdparty/claudette/base_collision_test.o obj/Release/src/3rdparty/claudette/box.o obj/Release/src/3rdparty/claudette/box_bld.o obj/Release/src/3rdparty/claudette/collision_model_3d.o obj/Release/src/3rdparty/claudette/math3d.o obj/Release/src/3rdparty/claudette/model_collision_test.o obj/Release/src/3rdparty/claudette/mytritri.o obj/Release/src/3rdparty/claudette/ray_collision_test.o obj/Release/src/3rdparty/claudette/sphere_collision_test.o obj/Release/src/3rdparty/claudette/sysdep.o obj/Release/src/3rdparty/claudette/tritri.o obj/Release/src/3rdparty/glm/detail/glm.o obj/Release/src/3rdparty/glmmodel/glmmodel.o obj/Release/src/3rdparty/lodepng/lodepng.o obj/Release/src/audio/soundmanager.o obj/Release/src/main.o obj/Release/src/objects/aabbcollider.o obj/Release/src/objects/complexcollider.o obj/Release/src/objects/cylindercollider.o obj/Release/src/objects/drone.o obj/Release/src/objects/dronemanager.o obj/Release/src/objects/hud.o obj/Release/src/objects/object.o obj/Release/src/objects/player.o obj/Release/src/objects/sign.o obj/Release/src/objects/treemanager.o obj/Release/src/particles/particle.o obj/Release/src/particles/particleconfig.o obj/Release/src/particles/particlelist.o obj/Release/src/particles/particlemanager.o obj/Release/src/util/gldebugging.o obj/Release/src/util/image.o obj/Release/src/util/loadtexture.o obj/Release/src/util/math.o obj/Release/src/util/planerenderer.o obj/Release/src/util/profiling.o obj/Release/src/util/shader.o obj/Release/src/world/grassmanager.o obj/Release/src/world/sky.o obj/Release/src/world/terrain.o obj/Release/src/world/world.o  -lfreetype -lpthread -lopenal -lglfw3 -ldl -lGLEW -lGL -lX11 -lXi -lXrandr -lXxf86vm -lXinerama -lXcursor -lrt -lm -s  
clean:
	rm -rf obj
	rm PUBG