cmake_minimum_required(VERSION 3.8)
project(Steinkraft)

set(CMAKE_CXX_STANDARD 17)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
endif()

set(CMAKE_MODULE_PATH "/usr/share/cmake_modules/cmake/Modules")

find_package(SDL REQUIRED)
find_package(SDL_mixer REQUIRED)
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
find_package(glew REQUIRED)
endif()

include_directories(".")

file(GLOB SOURCE_FILES *.cpp *.c */*.cpp */*/*.cpp)

# main executable
add_executable(Steinkraft ${SOURCE_FILES})
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
target_include_directories(Steinkraft PRIVATE ${SDL_INCLUDE_DIRS}/..)
set_target_properties(Steinkraft PROPERTIES
  LINK_FLAGS "/NODEFAULTLIB:msvcrt.lib"
)
target_link_libraries(Steinkraft
  PRIVATE
    ${SDL_LIBRARIES}
    ${SDL_MIXER_LIBRARIES}
    ${GLEW_LIBRARIES}
    opengl32
    glu32
)
else()
target_link_libraries(Steinkraft ${SDL_LIBRARIES} -lm -lGL -lGLU -lGLEW -lSDL -lSDL_mixer)
endif()