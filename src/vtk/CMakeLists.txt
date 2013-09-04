PROJECT(AlderVTK)

SET(AlderVTK_SRCS
  vtkAnimationPlayer.cxx
  vtkAlderMySQLDatabase.cxx
  vtkAlderMySQLQuery.cxx
  vtkCustomCornerAnnotation.cxx
  vtkCustomInteractorStyleImage.cxx
  vtkFrameAnimationPlayer.cxx
  vtkImageCoordinateWidget.cxx
  vtkImageDataReader.cxx
  vtkImageWindowLevel.cxx
  vtkMedicalImageViewer.cxx
  vtkXMLFileReader.cxx
  vtkXMLConfigurationFileReader.cxx
)

SET_SOURCE_FILES_PROPERTIES(
  vtkAnimationPlayer.cxx
  vtkXMLFileReader.cxx
  ABSTRACT
)

INCLUDE_DIRECTORIES( 
  ${CMAKE_CURRENT_BINARY_DIR} 
  ${LIBXML2_INCLUDE_DIR}
  ${MYSQL_INCLUDE_DIRECTORIES}
)

ADD_LIBRARY( AlderVTK ${AlderVTK_SRCS} )

TARGET_LINK_LIBRARIES( AlderVTK
  vtkgdcm
  ${MYSQL_LIBRARY}
  ${LIBXML2_LIBRARIES}
)

INSTALL( TARGETS AlderVTK DESTINATION bin )
