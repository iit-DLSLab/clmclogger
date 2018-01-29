cmake_minimum_required(VERSION 2.8.3)
project(clmclogger)

# Checking if the project is compiling with Catkin or CMake
set(CATKIN_CALL "${CATKIN_DEVEL_PREFIX}")
if(CATKIN_CALL)
  message("Compiling clmclogger library as a Catkin project")
  find_package(catkin REQUIRED COMPONENTS
    roscpp
    roslib
    std_msgs
  )
else()
  message("Compiling clmclogger library as a CMake project")
  # Setting the output directories for libraries and executables
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY  ${CMAKE_SOURCE_DIR}/lib)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY  ${CMAKE_SOURCE_DIR}/lib)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_SOURCE_DIR}/bin)
endif()

set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
set(CMAKE_BUILD_TYPE "Release")

if(CATKIN_CALL)
  include_directories(
    include
    ${catkin_INCLUDE_DIRS}
  )

  catkin_package(
    INCLUDE_DIRS include
    LIBRARIES mrdplot logger CLMCLogger
    CATKIN_DEPENDS roscpp roslib
  )
else()
  include_directories(
    include
  )
endif()


## Declare a cpp library
add_library(mrdplot src/mrdplot.cpp)

add_library(CLMCLogger src/CLMCLogger.cpp)
target_link_libraries(CLMCLogger)


## Declare a cpp library
add_library(logger src/Logger.cpp)
target_link_libraries(logger mrdplot CLMCLogger )
        
## Tests and Examples:
add_executable(testMRD test/test.cpp)
target_link_libraries(testMRD mrdplot CLMCLogger)

add_executable(example_logger example/ExampleLogger.cpp)
target_link_libraries(example_logger  CLMCLogger)

if(CATKIN_CALL)
  add_executable(ros_example_logger example/RosExampleLogger.cpp)
  target_link_libraries(ros_example_logger  logger ${catkin_LIBRARIES})

  add_executable(ros_example_pub example/RosExamplePublisher.cpp)
  target_link_libraries(ros_example_pub ${catkin_LIBRARIES})
else()                       

endif()
