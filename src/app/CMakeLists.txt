
set(NAME 3dp-course-app)

set(HEADERS
  AboutDialog.hpp
  Application.hpp
  CamCalibPanel.hpp
  CalibrationData.hpp
  CalibrationDataCamera.hpp
  CalibrationDataTurntable.hpp
  CalibrationDataLaserLine.hpp
  ChessboardCorners.hpp
  ImgGLWidget.hpp
  LaserPanel.hpp
  LaserTriangulator.hpp
  Log.hpp
  MainWindow.hpp
  PlotMaker.hpp
  SerialPortSettings.hpp
  SerialPortCommandLine.hpp
  TurntableCalib.hpp
  WrlGLBuffer.hpp
  WrlGLHandles.hpp
  WrlGLShader.hpp
  WrlGLWidget.hpp
  WrlQtLogo.hpp
  WrlToolsWidget.hpp
  WrlViewerData.hpp
  gui_util.hpp
  matrix_util.hpp
) # HEADERS    

set(SOURCES
  AboutDialog.cpp
  Application.cpp
  CamCalibPanel.cpp
  CalibrationData.cpp
  CalibrationDataCamera.cpp
  CalibrationDataTurntable.cpp
  CalibrationDataLaserLine.cpp
  ChessboardCorners.cpp
  ImgGLWidget.cpp
  LaserPanel.cpp
  LaserTriangulator.cpp
  MainWindow.cpp
  PlotMaker.cpp
  SerialPortSettings.cpp
  SerialPortCommandLine.cpp
  TurntableCalib.cpp
  WrlGLBuffer.cpp
  WrlGLHandles.cpp
  WrlGLShader.cpp
  WrlGLWidget.cpp
  WrlQtLogo.cpp
  WrlToolsWidget.cpp
  WrlViewerData.cpp
  gui_util.cpp
  main.cpp
  matrix_util.cpp
) # SOURCES

qt5_wrap_ui(FORMS
  ${FORMS_DIR}/MainWindow.ui
  ${FORMS_DIR}/AboutDialog.ui
  ${FORMS_DIR}/LaserPanel.ui
  ${FORMS_DIR}/CamCalibPanel.ui
  ${FORMS_DIR}/SerialPortSettings.ui
  ${FORMS_DIR}/SerialPortCommandLine.ui
) # FORMS

qt5_add_resources(ASSETS
  ${ASSETS_DIR}/assets.qrc
) # ASSETS

if(USE_CAM)
  set(HEADERS ${HEADERS} CameraPanel.hpp)
  set(HEADERS ${HEADERS} TurntableScanningPanel.hpp)
  set(HEADERS ${HEADERS} CameraRecorder.hpp)
  set(SOURCES ${SOURCES} CameraPanel.cpp)
  set(SOURCES ${SOURCES} TurntableScanningPanel.cpp)
  set(SOURCES ${SOURCES} CameraRecorder.cpp)
  qt5_wrap_ui(CAM_FORMS ${FORMS_DIR}/CameraPanel.ui)
  qt5_wrap_ui(CAM_FORMS ${FORMS_DIR}/TurntableScanningPanel.ui)
  set(FORMS ${FORMS} ${CAM_FORMS})
endif(USE_CAM)

if (APPLE)
  set(APP_ICON ${ASSETS_DIR}/camera.icns)
  set_source_files_properties(${APP_ICON} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    add_executable(${NAME} MACOSX_BUNDLE ${APP_ICON}
      ${HEADERS}
      ${SOURCES}
      ${FORMS}
      ${ASSETS}
  ) # add_executable
  set(MACOSX_BUNDLE_ICON_FILE "camera.icns" )
else(APPLE)
  add_executable(${NAME}
      ${HEADERS}
      ${SOURCES}
      ${FORMS}
      ${ASSETS}
  ) # add_executable
endif(APPLE)

target_compile_features(${NAME} PRIVATE cxx_right_angle_brackets cxx_lambdas)

target_link_libraries(${NAME} ${LIB_LIST})
if (USE_OPENCV)
  target_link_libraries(${NAME} ${OpenCV_LIBS} )

  # if(WIN32)
  #   target_link_libraries(${NAME} debug     ${CV_LIBS_DEBUG})
  #   target_link_libraries(${NAME} optimized ${CV_LIBS_RELEASE})
  # endif(WIN32)

endif(USE_OPENCV)
