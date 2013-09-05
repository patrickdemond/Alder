PROJECT( AlderQtModel )

SET( AlderQtModelLib_SRC
  QAlderAtlasWidget.cxx
  QAlderInterviewWidget.cxx
  QLoginDialog.cxx
  QSelectInterviewDialog.cxx
  QUserListDialog.cxx
  QVTKProgressDialog.cxx
)

SET( AlderQtModel_Uis
  QAlderAtlasWidget.ui
  QAlderInterviewWidget.ui
  QLoginDialog.ui
  QSelectInterviewDialog.ui
  QUserListDialog.ui
  QVTKProgressDialog.ui
)

SET( AlderQtModel_MocHeaders
  QAlderAtlasWidget.h
  QAlderInterviewWidget.h
  QLoginDialog.h
  QSelectInterviewDialog.h
  QUserListDialog.h
  QVTKProgressDialog.h
)

SET( AlderQtModel_Resources
  ${CMAKE_CURRENT_SOURCE_DIR}/resources/QAlderModelWidgets.qrc
)

# Wrap QT files
QT4_WRAP_UI(       AlderQtModelLib_UI  ${AlderQtModel_Uis} )
QT4_WRAP_CPP(      AlderQtModelLib_MOC ${AlderQtModel_MocHeaders} )
QT4_ADD_RESOURCES( AlderQtModelLib_QRC ${AlderQtModel_Resources} )

ADD_DEFINITIONS( -DQT_GUI_LIBS -DQT_CORE_LIB )

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_BINARY_DIR} )

ADD_LIBRARY( AlderQtModel
  ${AlderQtModelLib_SRC}
  ${AlderQtModelLib_UI}
  ${AlderQtModelLib_MOC}
  ${AlderQtModelLib_QRC}
)

TARGET_LINK_LIBRARIES( AlderQtModel
  QVTK
  AlderBusiness
  ${QT_LIBRARIES}
)

INSTALL( TARGETS AlderQtModel
  ARCHIVE DESTINATION lib  
  LIBRARY DESTINATION lib
  COMPONENT library
)

