cmake_minimum_required(VERSION 2.8.11)

project(3dp-course-app)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# This variable is used to locate the bin directory where the
# application will be installed
set(CMAKE_INSTALL_PREFIX
    ${PROJECT_SOURCE_DIR}/..
    CACHE PATH "Project bin directory" FORCE)

# you can comment the following line
message("CMAKE_INSTALL_PREFIX = ${CMAKE_INSTALL_PREFIX}")

# set to the proper directory in your machine 
set(Qt5_cmake_DIR "D:/Softwares/Qt/Qt5.9.2/5.9.2/msvc2017_64/lib/cmake")

# This is needded for CMake to find the Qt libraries
set(CMAKE_PREFIX_PATH
    ${Qt5_cmake_DIR}
    CACHE PATH "Qt5 cmake directory" FORCE)

# you can comment the following line
message ("CMAKE_PREFIX_PATH = ${CMAKE_PREFIX_PATH}") 

# Find the QtWidgets library
find_package(Qt5Core)
find_package(Qt5Gui)
find_package(Qt5OpenGL)
find_package(Qt5Widgets)
find_package(Qt5Svg)
find_package(Qt5SerialPort)

set(FORMS_DIR ${PROJECT_SOURCE_DIR}/../forms)
set(ASSETS_DIR ${PROJECT_SOURCE_DIR}/../assets)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/lib)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/lib)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)

add_definitions(-DNOMINMAX -D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS -D_USE_MATH_DEFINES)

#add current dir to include search path
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_LIST_DIR}/../3rd-party/eigen-3.2.7)

#options
option(USE_QTMULTIMEDIA "Use 'QtMultimedia' library" ON)
option(USE_IMG "Use 'img' library" ON)
option(USE_CAM "Use 'cam' library" ON)
option(USE_MATLAB_IO "Use 'matlab-io' library" ON)
option(USE_OPENCV "Use OpenCV 3.4.1" ON)

#library list
set(LIB_LIST 
    Qt5::Core 
    Qt5::Gui
    Qt5::OpenGL
    Qt5::Widgets 
    Qt5::Svg
    Qt5::SerialPort
    )

if (WIN32)
  list(APPEND LIB_LIST ${Qt5Gui_EGL_LIBRARIES} ${Qt5Gui_OPENGL_LIBRARIES})
endif(WIN32)

#modules
if (USE_QTMULTIMEDIA)
    find_package(Qt5Multimedia)
    list(APPEND LIB_LIST Qt5::Multimedia)
    add_definitions(-DHAVE_QTMULTIMEDIA)
endif(USE_QTMULTIMEDIA)

if (USE_IMG)
    add_subdirectory(img)
    list(APPEND LIB_LIST img)
    add_definitions(-DHAVE_IMG)
endif(USE_IMG)

if (USE_CAM)
    add_subdirectory(cam)
    list(APPEND LIB_LIST cam)
    add_definitions(-DHAVE_CAM)
endif(USE_CAM)

if (USE_MATLAB_IO)
    add_subdirectory(matlab-io)
    list(APPEND LIB_LIST matlab-io)
    add_definitions(-DHAVE_MATLAB_IO)
endif(USE_MATLAB_IO)

if (USE_OPENCV)

  find_package( OpenCV REQUIRED )
  include_directories( ${OpenCV_INCLUDE_DIRS} )

   message("USE_OPENCV")
   add_definitions(-DUSE_OPENCV)
   if(WIN32)
     set(OPENCV_DIR "D:/Softwares/opencv/build")
     set(OPENCV_LIB_DIR ${OPENCV_DIR}/x64/vc15/lib)
     set(OPENCV_INC_DIR ${OPENCV_DIR}/include)
     include_directories(${OPENCV_INC_DIR})
     link_directories(${OPENCV_LIB_DIR})
     message("OPENCV_INC_DIR = ${OPENCV_INC_DIR}")
     message("OPENCV_LIB_DIR = ${OPENCV_LIB_DIR}")
     # list(APPEND LIB_LIST)
     set(CV_LIBS_RELEASE
      opencv_core330 opencv_imgproc330 opencv_imgcodecs330 opencv_highgui330
      opencv_calib3d330 opencv_features2d330 opencv_flann330 opencv_videoio330)
     set(CV_LIBS_DEBUG
      opencv_core330d opencv_imgproc330d opencv_imgcodecs330d opencv_highgui330d
      opencv_calib3d330d opencv_features2d330d opencv_flann330d opencv_videoio330d)
   endif(WIN32)

endif(USE_OPENCV)

#homeworks
add_subdirectory(homework)
list(APPEND LIB_LIST homework)
add_definitions(-DHAVE_HOMEWORK)

add_subdirectory(util)
list(APPEND LIB_LIST util)

add_subdirectory(io)
list(APPEND LIB_LIST io)

add_subdirectory(wrl)
list(APPEND LIB_LIST wrl)

add_subdirectory(app)

