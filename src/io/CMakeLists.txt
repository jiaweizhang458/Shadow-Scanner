set(NAME io)

set(HEADERS
  AppLoader.h
  AppSaver.h
  Loader.h
  LoaderWrl.h
  Saver.h
  SaverWrl.h
  StrException.h
  Tokenizer.h
) # HEADERS    

set(SOURCES
  AppLoader.cpp
  AppSaver.cpp
  LoaderWrl.cpp
  SaverWrl.cpp
  Tokenizer.cpp
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

