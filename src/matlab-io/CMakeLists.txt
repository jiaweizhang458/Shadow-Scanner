
set(NAME matlab-io)

set(HEADERS
    EFStream.hpp
    MatlabIO.hpp
    typetraits.hpp
    util.hpp
    test.hpp
) # HEADERS    

set(SOURCES
    MatlabIO.cpp
    util.cpp
    test.cpp
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

target_compile_features(${NAME} PRIVATE cxx_right_angle_brackets)

target_link_libraries(${NAME} ${LIB_LIST})

