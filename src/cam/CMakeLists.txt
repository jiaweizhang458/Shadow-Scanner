
set(NAME cam)

set(HEADERS
    cam.hpp
    CameraInterface.hpp
    Log.hpp
) # HEADERS    

set(SOURCES
    cam.cpp
) # SOURCES

qt5_wrap_ui(FORMS
) # FORMS

qt5_add_resources(ASSETS
) # ASSETS

include(CheckIncludeFiles)

#Direct Show
if (WIN32)
    check_include_files(Dshow.h HAVE_DSHOW)
    if (HAVE_DSHOW)
        option(USE_DSHOW "Use Direct Show cameras" OFF)
    endif(HAVE_DSHOW)
    if (USE_DSHOW)
        add_definitions(-DHAVE_DSHOW)
        set(HEADERS ${HEADERS} DShowCamera.hpp LVUVCPublic.h)
        set(SOURCES ${SOURCES} DShowCamera.cpp)
        set(DSHOW_LIBS Strmiids Vfw32 Ole32 OleAut32)
    endif(USE_DSHOW)
endif(WIN32)

#QtCamera
if(USE_QTMULTIMEDIA)
    set(HEADERS ${HEADERS} QtCamera.hpp)
    set(SOURCES ${SOURCES} QtCamera.cpp)
endif(USE_QTMULTIMEDIA)


add_library(${NAME}
    ${HEADERS}
    ${SOURCES}
    ${FORMS}
    ${ASSETS}
) # add_library


target_link_libraries(${NAME} ${LIB_LIST} ${DSHOW_LIBS} )

