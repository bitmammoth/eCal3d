cmake_minimum_required(VERSION 3.9)
project(eCal3d)

set(CMAKE_CXX_STANDARD 11)

add_executable(calview
        calview/cv-global.h
        calview/cv-main.cpp
        calview/cv-tick.cpp
        calview/cv-tick.h
        calview/cv-userdata.h
        calview/cv-viewer.cpp
        calview/cv-viewer.h)
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
target_link_libraries(calview eCal3d OpenGL::GL OpenGL::GLU GLUT::GLUT)
target_compile_definitions(calview PRIVATE CVUSERDATA)
target_include_directories(calview PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/calview)

add_library(eCal3d
        andy/caluserdata.h
        cal3d/buffersource.cpp
        cal3d/buffersource.h
        cal3d/cal3d.h
        cal3d/calbone.cpp
        cal3d/calbone.h
        cal3d/calcoreanim.cpp
        cal3d/calcoreanim.h
        cal3d/calcorebone.cpp
        cal3d/calcorebone.h
        cal3d/calcorekey.cpp
        cal3d/calcorekey.h
        cal3d/calcoremodel.cpp
        cal3d/calcoremodel.h
        cal3d/calcoresub.cpp
        cal3d/calcoresub.h
        cal3d/calcoretrack.cpp
        cal3d/calcoretrack.h
        cal3d/caldatasource.h
        cal3d/calerror.cpp
        cal3d/calerror.h
        cal3d/calglobal.cpp
        cal3d/calglobal.h
        cal3d/calloader.cpp
        cal3d/calloader.h
        cal3d/calmatrix.cpp
        cal3d/calmatrix.h
        cal3d/calmodel.cpp
        cal3d/calmodel.h
        cal3d/calphysop.h
        cal3d/calplatform.cpp
        cal3d/calplatform.h
        cal3d/calquat.cpp
        cal3d/calquat.h
        cal3d/calsaver.cpp
        cal3d/calsaver.h
        cal3d/calsub.cpp
        cal3d/calsub.h
        cal3d/calvector.cpp
        cal3d/calvector.h
        cal3d/streamsource.cpp
        cal3d/streamsource.h)
target_include_directories(eCal3d
        PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/cal3d>
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/andy)
target_compile_definitions(eCal3d PRIVATE CALUSERDATA CAL3D_EXPORTS)
