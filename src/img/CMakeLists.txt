
set(NAME img)

set(HEADERS
    ARGB.hpp
    Image.hpp
    ImageBuffer.hpp
    Img.hpp
    ImgArgb.hpp
    ImgBit.hpp
    ImgBitplane.hpp
    ImgBuckets.hpp
    ImgCheckerboards.hpp
    ImgDraw.hpp
    ImgInt.hpp
    ImgFloat.hpp
    util.hpp
    yuv.hpp
) # HEADERS    

set(SOURCES
    Image.cpp
    ImageBuffer.cpp
    Img.cpp
    ImgArgb.cpp
    ImgBit.cpp
    ImgBitplane.cpp
    ImgBuckets.cpp
    ImgCheckerboards.cpp
    ImgDraw.cpp
    ImgInt.cpp
    ImgFloat.cpp
    util.cpp
    yuv.cpp
) # SOURCES

qt5_wrap_ui(FORMS
) # FORMS

qt5_add_resources(ASSETS
) # ASSETS

add_library(${NAME}
    ${HEADERS}
    ${SOURCES}
    ${FORMS}
    ${ASSETS}
) # add_library

target_compile_features(${NAME} PRIVATE cxx_lambdas)

target_link_libraries(${NAME} ${LIB_LIST})

