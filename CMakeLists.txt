cmake_minimum_required(VERSION 3.8)
project(Steinkraft)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_MODULE_PATH "/usr/share/cmake_modules/cmake/Modules")

#find_package(Boost COMPONENTS system filesystem regex REQUIRED)
find_package(Poco REQUIRED Foundation Zip Net)
find_package(SDL REQUIRED)

include_directories(".")

file(GLOB SOURCE_FILES *.cpp *.c */*.cpp */*/*.cpp)

# main executable
add_executable(Steinkraft ${SOURCE_FILES})
target_link_libraries(Steinkraft ${Boost_LIBRARIES} ${Poco_LIBRARIES} ${SDL_LIBRARIES} -lm -lGL -lGLU -lGLEW -lSDL -lSDL_mixer)
