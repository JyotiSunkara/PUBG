project(PUBG)
cmake_minimum_required(VERSION 2.8)
# aux_source_directory(src SRC_LIST)
# add_executable(${PROJECT_NAME} ${SRC_LIST})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -lglfw3 -lX11 -lXrandr -lXinerama -lXi -lXxf86vm -lXcursor -lGL -lpthread -ldl")

set(CMAKE_MODULE_PATH /usr/local/lib/cmake /usr/local/lib/x86_64-linux-gnu/cmake)
set(CMAKE_PREFIX_PATH /usr/local/lib/cmake/glfw)

find_package (PkgConfig REQUIRED)
find_package (OpenGL REQUIRED)
find_package (glfw3 REQUIRED)
find_package (GLM REQUIRED)
find_package (GLEW REQUIRED STATIC)

set(SOURCE_FILES src/main.cpp)
add_executable(PUBG ${SOURCE_FILES})
target_link_libraries (PUBG ${OPENGL_LIBRARIES} ${GLFW3_LIBRARY}
        ${GLEW_LIBRARY} Xxf86vm pthread Xrandr Xinerama Xi Xcursor)
