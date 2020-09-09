project(PUBG)
cmake_minimum_required(VERSION 2.8)
aux_source_directory(src SRC_LIST)
add_executable(${PROJECT_NAME} ${SRC_LIST})

include(FindPkgConfig)

find_package( Threads REQUIRED )
find_package(glm REQUIRED)
find_package(OpenAL REQUIRED)


pkg_check_modules(GLFW3 REQUIRED glfw3)
include_directories(${GLFW3_INCLUDE_DIRS})
target_link_libraries (${PROJECT_NAME} ${GLFW3_LIBRARIES})

pkg_check_modules(GLEW REQUIRED glew)
include_directories(${GLEW_INCLUDE_DIRS})
target_link_libraries (${PROJECT_NAME} ${GLEW_LIBRARIES})

if (NOT APPLE)
  pkg_check_modules(GL REQUIRED gl)
  include_directories(${GL_INCLUDE_DIRS})
  target_link_libraries (${PROJECT_NAME} ${GL_LIBRARIES})
endif()

if (APPLE)
  target_link_libraries(${PROJECT_NAME} "-framework OpenGL")
endif()
target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(${PROJECT_NAME} ${CMAKE_DL_LIBS})
target_link_libraries(${PROJECT_NAME} ${OPENAL_LIBRARY})


# pkg_check_modules(AO REQUIRED ao)
# include_directories(${AO_INCLUDE_DIRS})
# target_link_libraries (${PROJECT_NAME} ${AO_LIBRARIES})

# pkg_check_modules(LIBMPG123 REQUIRED libmpg123)
# include_directories(${LIBMPG123_INCLUDE_DIRS})
# target_link_libraries (${PROJECT_NAME} ${LIBMPG123_LIBRARIES})

configure_file(shaders/grass.vert grass.vert COPYONLY)
configure_file(shaders/grass.frag grass.frag COPYONLY)
configure_file(shaders/particle.vert particle.vert COPYONLY)
configure_file(shaders/particle.frag particle.frag COPYONLY)
configure_file(shaders/plane.vert plane.vert COPYONLY)
configure_file(shaders/plane.frag plane.frag COPYONLY)
configure_file(shaders/sky.vert sky.vert COPYONLY)
configure_file(shaders/sky.frag sky.frag COPYONLY)
configure_file(shaders/solid.vert solid.vert COPYONLY)
configure_file(shaders/solid.frag solid.frag COPYONLY)
configure_file(shaders/terrain.vert terrain.vert COPYONLY)
configure_file(shaders/terrain.frag terrain.frag COPYONLY)
configure_file(shaders/tree.vert tree.vert COPYONLY)
configure_file(shaders/tree.frag tree.frag COPYONLY)
configure_file(shaders/solid-instanced.vert solid-instanced.vert COPYONLY)
# configure_file(assets/song.mp3 assets/song.mp3 COPYONLY)
