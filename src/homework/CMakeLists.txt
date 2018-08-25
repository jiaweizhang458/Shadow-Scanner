
set(NAME homework)

set(HEADERS
    homework1.hpp
    homework2.hpp
    homework3.hpp
    homework4.hpp
) # HEADERS    

set(SOURCES
    homework1.cpp
    homework2.cpp
    homework3.cpp
    homework4.cpp
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

target_link_libraries(${NAME} ${LIB_LIST})

