PROJECT( AlderQtGeneric )

SET( AlderQtGenericLib_SRC
  QAboutDialog.cxx
  QAlderDicomTagWidget.cxx
  QAlderDoubleSlider.cxx
  QAlderFramePlayerWidget.cxx
  QAlderSliderWidget.cxx
  QMedicalImageWidget.cxx
)

SET( AlderQtGeneric_Uis
  QAboutDialog.ui
  QAlderDicomTagWidget.ui
  QAlderFramePlayerWidget.ui
  QAlderSliderWidget.ui
  QMedicalImageWidget.ui
)

SET( AlderQtGeneric_MocHeaders
  QAboutDialog.h
  QAlderDicomTagWidget.h
  QAlderDoubleSlider.h
  QAlderFramePlayerWidget.h
  QAlderSliderWidget.h
  QMedicalImageWidget.h
)

SET( AlderQtGeneric_Resources
  ${CMAKE_CURRENT_SOURCE_DIR}/resources/QAlderGenericWidgets.qrc
)

INCLUDE_DIRECTORIES(
  ${ALDER_VTK_DIR}
)

# Wrap QT files
QT4_WRAP_UI(       AlderQtGenericLib_UI  ${AlderQtGeneric_Uis} )
QT4_WRAP_CPP(      AlderQtGenericLib_MOC ${AlderQtGeneric_MocHeaders} )
QT4_ADD_RESOURCES( AlderQtGenericLib_QRC ${AlderQtGeneric_Resources} )

ADD_LIBRARY( AlderQtGeneric
  ${AlderQtGenericLib_SRC}
  ${AlderQtGenericLib_UI}
  ${AlderQtGenericLib_MOC}
  ${AlderQtGenericLib_QRC}
)

INSTALL( TARGETS AlderQtGeneric 
  ARCHIVE DESTINATION lib  
  LIBRARY DESTINATION lib
  COMPONENT library
)
