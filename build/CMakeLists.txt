CMAKE_MINIMUM_REQUIRED( VERSION 2.6 )
PROJECT( Alder )

# The major version number is incremented after major milestones are met
SET( ALDER_VERSION_MAJOR 1 )
# The minor version number is incremented after a development branch is merged to qa
SET( ALDER_VERSION_MINOR 2 )
# The patch version number is incremented when changes are made to the release branch
SET( ALDER_VERSION_PATCH 1 )

# Icons sizes to build
SET( ALDER_ICON_SIZES 16x16 22x22 24x24 32x32 48x48 64x64 128x128 )

# Look in the build directory for cmake modules
set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/" )

# We need VTK
FIND_PACKAGE( VTK REQUIRED )
INCLUDE( ${VTK_USE_FILE} )

# We need GDCM
FIND_PACKAGE( GDCM REQUIRED )
INCLUDE( ${GDCM_USE_FILE} )

# We need QT4
SET( QT_MIN_VERSION 4.0.0 )
FIND_PACKAGE( Qt4 REQUIRED )
INCLUDE( ${QT_USE_FILE} )

# We need libxml2
FIND_PACKAGE( LibXml2 REQUIRED )

# We need Crypto++
FIND_PACKAGE( CRYPTO++ REQUIRED )

# We need JSONCpp
FIND_PACKAGE( JSONCpp REQUIRED )

# We need MySQL
FIND_PACKAGE( MySQL REQUIRED )

# We need CURL
FIND_PACKAGE( CURL REQUIRED )

# We need convert
IF( UNIX AND NOT APPLE )
  FIND_PACKAGE( ImageMagick COMPONENTS convert REQUIRED )
ENDIF( UNIX AND NOT APPLE )

SET( ALDER_ROOT_DIR         ${PROJECT_SOURCE_DIR}/.. )
SET( ALDER_AUX_DIR          ${ALDER_ROOT_DIR}/aux )
SET( ALDER_SRC_DIR          ${ALDER_ROOT_DIR}/src )
SET( ALDER_MODEL_DIR        ${ALDER_SRC_DIR}/model )
SET( ALDER_QT_DIR           ${ALDER_SRC_DIR}/interface/qt )
SET( ALDER_QT_WIDGETS_DIR   ${ALDER_QT_DIR}/widgets )
SET( ALDER_QT_RESOURCES_DIR ${ALDER_QT_WIDGETS_DIR}/resources )
SET( ALDER_VTK_DIR          ${ALDER_SRC_DIR}/vtk )
SET( ALDER_DOC_DIR          ${ALDER_ROOT_DIR}/doc )
SET( ALDER_DOXY_DIR         ${PROJECT_BINARY_DIR}/doxygen )
SET( ALDER_TESTING_DIR      ${ALDER_ROOT_DIR}/testing )

# Define the config file location, then test to see if it is in the install path when installing
SET( ALDER_CONFIG_FILE ${PROJECT_BINARY_DIR}/config.xml CACHE PATH "The location of Alder's runtime config file" )

IF( NOT ${ALDER_CONFIG_FILE} MATCHES ${CMAKE_INSTALL_PREFIX} )
  INSTALL( CODE "MESSAGE( \"
### WARNING ###
The location of the config is not within the install prefix!
In order to use the installed version of the config file (${CMAKE_INSTALL_PREFIX}/etc/config.xml)
you must explicitly set ALDER_CONFIG_FILE to that path
\" )" )
ENDIF( NOT ${ALDER_CONFIG_FILE} MATCHES ${CMAKE_INSTALL_PREFIX} )

# Create a salt string which can be set at build time
SET( ALDER_SALT_STRING "this is salt" CACHE STRING "This is salt for encrypting passwords" )

# Define stack depth to display on thrown exceptions
SET( ALDER_STACK_DEPTH "10" CACHE STRING "The stack depth to display on thrown exceptions" )

# Configure the utitlities header
CONFIGURE_FILE( ${ALDER_SRC_DIR}/Utilities.h.in
                ${PROJECT_BINARY_DIR}/Utilities.h @ONLY IMMEDIATE )

# Configure the desktop file
CONFIGURE_FILE( ${ALDER_AUX_DIR}/alder.desktop.in
                ${PROJECT_BINARY_DIR}/alder.desktop @ONLY IMMEDIATE )

# Copy the readme file to build
CONFIGURE_FILE( ${ALDER_ROOT_DIR}/readme
                ${PROJECT_BINARY_DIR}/readme COPYONLY )

# Run the configuration initialization script if the config xml file is not found
IF( NOT EXISTS ${ALDER_CONFIG_FILE} )
  MESSAGE( "No config.xml file was found, building one" )
  EXECUTE_PROCESS( COMMAND ${ALDER_AUX_DIR}/init_config ${ALDER_CONFIG_FILE} )
ENDIF( NOT EXISTS ${ALDER_CONFIG_FILE} )

# Set up include directories
SET( ALDER_INCLUDE_DIR
  ${ALDER_MODEL_DIR}
  ${ALDER_QT_DIR}
  ${ALDER_QT_WIDGETS_DIR}
  ${ALDER_VTK_DIR}
)

SUBDIRS( ${ALDER_MODEL_DIR} ${ALDER_VTK_DIR} )

SET( ALDER_LIBRARIES 
  AlderModel
  AlderVTK
  CACHE INTERNAL
  "internal libraries for Alder"
)

SET( ALDER_SOURCE
  ${ALDER_SRC_DIR}/Alder.cxx

  ${ALDER_QT_DIR}/QAlderApplication.cxx
  ${ALDER_QT_DIR}/QAboutDialog.cxx
  ${ALDER_QT_DIR}/QLoginDialog.cxx
  ${ALDER_QT_DIR}/QMainAlderWindow.cxx
  ${ALDER_QT_DIR}/QVTKProgressDialog.cxx
  ${ALDER_QT_DIR}/QSelectInterviewDialog.cxx
  ${ALDER_QT_DIR}/QUserListDialog.cxx

  ${ALDER_QT_WIDGETS_DIR}/QAlderAtlasWidget.cxx
  ${ALDER_QT_WIDGETS_DIR}/QAlderDicomTagWidget.cxx
  ${ALDER_QT_WIDGETS_DIR}/QAlderDoubleSlider.cxx
  ${ALDER_QT_WIDGETS_DIR}/QAlderFramePlayerWidget.cxx
  ${ALDER_QT_WIDGETS_DIR}/QAlderInterviewWidget.cxx
  ${ALDER_QT_WIDGETS_DIR}/QAlderSliderWidget.cxx
)

SET( ALDER_UIS
  ${ALDER_QT_DIR}/QAboutDialog.ui
  ${ALDER_QT_DIR}/QLoginDialog.ui
  ${ALDER_QT_DIR}/QMainAlderWindow.ui
  ${ALDER_QT_DIR}/QVTKProgressDialog.ui
  ${ALDER_QT_DIR}/QSelectInterviewDialog.ui
  ${ALDER_QT_DIR}/QUserListDialog.ui

  ${ALDER_QT_WIDGETS_DIR}/QAlderAtlasWidget.ui
  ${ALDER_QT_WIDGETS_DIR}/QAlderDicomTagWidget.ui
  ${ALDER_QT_WIDGETS_DIR}/QAlderFramePlayerWidget.ui
  ${ALDER_QT_WIDGETS_DIR}/QAlderInterviewWidget.ui
  ${ALDER_QT_WIDGETS_DIR}/QAlderSliderWidget.ui
)

SET( ALDER_HEADERS
  ${ALDER_QT_DIR}/QAboutDialog.h
  ${ALDER_QT_DIR}/QLoginDialog.h
  ${ALDER_QT_DIR}/QMainAlderWindow.h
  ${ALDER_QT_DIR}/QSelectInterviewDialog.h
  ${ALDER_QT_DIR}/QUserListDialog.h
  ${ALDER_QT_DIR}/QVTKProgressDialog.h

  ${ALDER_QT_WIDGETS_DIR}/QAlderAtlasWidget.h
  ${ALDER_QT_WIDGETS_DIR}/QAlderDicomTagWidget.h
  ${ALDER_QT_WIDGETS_DIR}/QAlderDoubleSlider.h
  ${ALDER_QT_WIDGETS_DIR}/QAlderFramePlayerWidget.h
  ${ALDER_QT_WIDGETS_DIR}/QAlderInterviewWidget.h
  ${ALDER_QT_WIDGETS_DIR}/QAlderSliderWidget.h
)

SET( ALDER_QT_QRC
  ${ALDER_QT_RESOURCES_DIR}/QAlderWidgets.qrc
)

# Wrap QT files
QT4_WRAP_UI( UIHeaders ${ALDER_UIS} )
QT4_WRAP_CPP( MOCSrcs ${ALDER_HEADERS} )
QT4_ADD_RESOURCES( QRCSrcs ${ALDER_QT_QRC} )

ADD_DEFINITIONS( -DQT_GUI_LIBS -DQT_CORE_LIB )

SET_SOURCE_FILES_PROPERTIES( ${ALDER_SOURCE} PROPERTIES OBJECT_DEPENDS "${UIHeaders}" )

# Use the include path and library for Qt
INCLUDE_DIRECTORIES(
  ${ALDER_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
  # ${LIBXML2_INCLUDE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  # ${CRYPTO++_INCLUDE_DIR}
  # ${JSONCPP_INCLUDE_DIR}
  # ${MYSQL_INCLUDE_DIRECTORIES}
  # ${CURL_INCLUDE_DIR}
)

# We're using cbegin and cend so we need c++11
SET( CMAKE_CXX_FLAGS "-std=c++0x -Wno-deprecated " )

# Make sure to include RPATH in the installed binary to support linking to libraries
SET( CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE )

# Targets
ADD_EXECUTABLE( alder ${ALDER_SOURCE} ${ALDER_UISrcs} ${MOCSrcs} ${QRCSrcs})

TARGET_LINK_LIBRARIES( alder
  QVTK
  vtkRendering
  vtkGraphics
  vtkIO
  vtkCommon
  #vtkgdcm
  gdcmDSED
  #gdcmMSFF
  gdcmDICT
  ${QT_LIBRARIES}
  # ${LIBXML2_LIBRARIES}
  # ${CURL_LIBRARY}
  # ${CRYPTO++_LIBRARIES}
  # ${JSONCPP_LIBRARIES}
  #${MYSQL_LIBRARY}
  ${ALDER_LIBRARIES}
)
INSTALL( TARGETS alder RUNTIME DESTINATION bin )
INSTALL( FILES ${CMAKE_BINARY_DIR}/config.xml DESTINATION etc )

ADD_CUSTOM_TARGET( dist
  COMMAND git archive --prefix=${ALDER_ARCHIVE_NAME}/ HEAD
    | bzip2 > ${CMAKE_BINARY_DIR}/${ALDER_ARCHIVE_NAME}.tar.bz2
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# Desktop entries and icons
IF( UNIX AND NOT APPLE )
  INSTALL( FILES ${PROJECT_BINARY_DIR}/alder.desktop DESTINATION share/applications )

  FOREACH( SIZE ${ALDER_ICON_SIZES} )
    ADD_CUSTOM_COMMAND( OUTPUT logo_clsa_${SIZE}.png
      COMMAND convert -background none -resize ${SIZE} ${ALDER_AUX_DIR}/logo_clsa.png logo_clsa_${SIZE}.png
      COMMENT "Generating ${SIZE} icon"
    )
    SET( ALDER_ICON_FILES ${ALDER_ICON_FILES} logo_clsa_${SIZE}.png )
    INSTALL( FILES ${CMAKE_BINARY_DIR}/logo_clsa_${SIZE}.png
      DESTINATION share/icons/hicolor/${SIZE}/apps
      RENAME logo_clsa.png
    )
  ENDFOREACH( SIZE )
  ADD_CUSTOM_TARGET( icons ALL DEPENDS ${ALDER_ICON_FILES} )
ENDIF( UNIX AND NOT APPLE )

# Build doxygen documentation ?
INCLUDE (${CMAKE_ROOT}/Modules/Documentation.cmake OPTIONAL)
IF( BUILD_DOCUMENTATION )

  SET( HAVE_DOT_YESNO NO )
  IF( DOT )
    SET( HAVE_DOT_YESNO YES )
    IF( NOT DOT_PATH )
      GET_FILENAME_COMPONENT( DOT_PATH ${DOT} PATH )
    ENDIF( NOT DOT_PATH )
  ENDIF( DOT )

  CONFIGURE_FILE(
    ${ALDER_DOC_DIR}/doc_mainpage.dox.in
    ${ALDER_DOXY_DIR}/doc_mainpage.dox )

  CONFIGURE_FILE(
    ${ALDER_DOC_DIR}/doc_makeall.cmake.in
    ${ALDER_DOXY_DIR}/doc_makeall.cmake
    @ONLY )
  
  CONFIGURE_FILE(
    ${ALDER_DOC_DIR}/doc_mainpage.dox.in
    ${ALDER_DOXY_DIR}/doc_mainpage.dox )

  SET( DOXY_INPUT_SOURCE 
    ${ALDER_SRC_DIR}
    ${ALDER_MODEL_DIR}
    ${ALDER_VTK_DIR}
    ${ALDER_QT_DIR}
    ${ALDER_DOXY_DIR}/doc_mainpage.dox )

  string( REPLACE ";" "\" \"" ALDER_DOXY_LIST "${DOXY_INPUT_SOURCE}" )
  
  CONFIGURE_FILE(
    ${ALDER_DOC_DIR}/config.dox.in
    ${ALDER_DOXY_DIR}/config.dox )

# Uncommenting this block will force a build of the documentation
# every time cmake is run in the build directory
# 
#  execute_process(COMMAND 
#    ${CMAKE_COMMAND} -P ${ALDER_DOXY_DIR}/doc_makeall.cmake
#    DEPENDS ${ALDER_DOXY_DIR}/doc_makeall.cmake 
#    WORKING DIR ${ALDER_DOXY_DIR}/doxygen 
#    RESULT_VARIABLE rv )

ENDIF( BUILD_DOCUMENTATION )

SUBDIRS( ${ALDER_TESTING_DIR} )
