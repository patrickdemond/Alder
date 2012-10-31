CMAKE_MINIMUM_REQUIRED( VERSION 2.6 )
PROJECT( Alder )

# Set the version
SET( ALDER_VERSION_MAJOR 0 )
SET( ALDER_VERSION_MINOR 1 )
SET( ALDER_VERSION_PATCH 0 )

# Icons sizes to build
SET( ALDER_ICON_SIZES 16x16 22x22 24x24 32x32 48x48 64x64 128x128 )

# Look in the build directory for cmake modules
set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/" )

# Create a salt string which can be set at build time
SET( ALDER_SALT_STRING "this is salt" CACHE STRING "This is salt for encrypting passwords")

# Define the directory which Alder will store image data
SET( ALDER_IMAGE_DATA_PATH ${PROJECT_BINARY_DIR}/images CACHE STRING "The path to store image data in" )

# We need VTK
FIND_PACKAGE( VTK REQUIRED )
INCLUDE( ${VTK_USE_FILE} )

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

# We need convert
IF( UNIX AND NOT APPLE )
  FIND_PACKAGE( ImageMagick COMPONENTS convert REQUIRED )
ENDIF( UNIX AND NOT APPLE )

SET( ALDER_ROOT_DIR ${PROJECT_SOURCE_DIR}/.. )
SET( ALDER_AUX_DIR ${ALDER_ROOT_DIR}/aux )
SET( ALDER_API_DIR ${ALDER_ROOT_DIR}/api )
SET( ALDER_MODEL_DIR ${ALDER_API_DIR}/model )
SET( ALDER_QT_DIR ${ALDER_API_DIR}/interface/qt )
SET( ALDER_VTK_DIR ${ALDER_API_DIR}/vtk )
SET( ALDER_CONFIG_FILE ${PROJECT_BINARY_DIR}/config.xml )
SET( ALDER_OPAL_SCRIPT ${PROJECT_BINARY_DIR}/opal.py )

# Configure the utitlities header
CONFIGURE_FILE( ${ALDER_API_DIR}/Utilities.h.in
                ${PROJECT_BINARY_DIR}/Utilities.h @ONLY IMMEDIATE )

# Copy the readme file to build
CONFIGURE_FILE( ${ALDER_ROOT_DIR}/readme
                ${PROJECT_BINARY_DIR}/readme COPYONLY )

# Copy the config xml file to the build path (if not found)
IF( NOT EXISTS ${ALDER_CONFIG_FILE} )
  CONFIGURE_FILE( ${ALDER_AUX_DIR}/config.xml
                  ${ALDER_CONFIG_FILE} COPYONLY )
ENDIF( NOT EXISTS ${ALDER_CONFIG_FILE} )

# Copy the opal script to the build path (if not found)
IF( NOT EXISTS ${ALDER_OPAL_SCRIPT} )
  CONFIGURE_FILE( ${ALDER_AUX_DIR}/opal.py
                  ${ALDER_OPAL_SCRIPT} COPYONLY )
ENDIF( NOT EXISTS ${ALDER_OPAL_SCRIPT} )

# Set up include directories
SET( ALDER_INCLUDE_DIR
  ${ALDER_MODEL_DIR}
  ${ALDER_QT_DIR}
  ${ALDER_VTK_DIR}
)

SET( ALDER_SOURCE
  ${ALDER_API_DIR}/Alder.cxx

  ${ALDER_MODEL_DIR}/ActiveRecord.cxx
  ${ALDER_MODEL_DIR}/Cineloop.cxx
  ${ALDER_MODEL_DIR}/Configuration.cxx
  ${ALDER_MODEL_DIR}/Database.cxx
  ${ALDER_MODEL_DIR}/Image.cxx
  ${ALDER_MODEL_DIR}/ModelObject.cxx
  ${ALDER_MODEL_DIR}/OpalService.cxx
  ${ALDER_MODEL_DIR}/Rating.cxx
  ${ALDER_MODEL_DIR}/Series.cxx
  ${ALDER_MODEL_DIR}/Study.cxx
  ${ALDER_MODEL_DIR}/User.cxx
  ${ALDER_MODEL_DIR}/Application.cxx

  ${ALDER_VTK_DIR}/vtkMedicalImageViewer.cxx
  ${ALDER_VTK_DIR}/vtkXMLFileReader.cxx
  ${ALDER_VTK_DIR}/vtkXMLConfigurationFileReader.cxx
  
  ${ALDER_QT_DIR}/QAlderApplication.cxx
  ${ALDER_QT_DIR}/QAboutDialog.cxx
  ${ALDER_QT_DIR}/QLoginDialog.cxx
  ${ALDER_QT_DIR}/QMainAlderWindow.cxx
  ${ALDER_QT_DIR}/QProgressDialog.cxx
  ${ALDER_QT_DIR}/QSelectStudyDialog.cxx
  ${ALDER_QT_DIR}/QUserListDialog.cxx
)

SET_SOURCE_FILES_PROPERTIES(
  ${ALDER_MODEL_DIR}/ActiveRecord.cxx
  ${ALDER_MODEL_DIR}/ModelObject.cxx

  ${ALDER_VTK_DIR}/vtkXMLFileReader.cxx
  ${ALDER_VTK_DIR}/vtkXMLFileWriter.cxx
  ABSTRACT )

SET( ALDER_UIS
  ${ALDER_QT_DIR}/QAboutDialog.ui
  ${ALDER_QT_DIR}/QLoginDialog.ui
  ${ALDER_QT_DIR}/QMainAlderWindow.ui
  ${ALDER_QT_DIR}/QProgressDialog.ui
  ${ALDER_QT_DIR}/QSelectStudyDialog.ui
  ${ALDER_QT_DIR}/QUserListDialog.ui
)

SET( ALDER_HEADERS
  ${ALDER_QT_DIR}/QAboutDialog.h
  ${ALDER_QT_DIR}/QLoginDialog.h
  ${ALDER_QT_DIR}/QMainAlderWindow.h
  ${ALDER_QT_DIR}/QProgressDialog.h
  ${ALDER_QT_DIR}/QSelectStudyDialog.h
  ${ALDER_QT_DIR}/QUserListDialog.h
)

# Wrap QT files
QT4_WRAP_UI( UIHeaders ${ALDER_UIS} )
QT4_WRAP_CPP( MOCSrcs ${ALDER_HEADERS} )
ADD_DEFINITIONS( -DQT_GUI_LIBS -DQT_CORE_LIB -DQT3_SUPPORT )
SET_SOURCE_FILES_PROPERTIES( ${ALDER_SOURCE} PROPERTIES OBJECT_DEPENDS "${UIHeaders}" )

# Use the include path and library for Qt
INCLUDE_DIRECTORIES(
  ${ALDER_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
  ${LIBXML2_INCLUDE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CRYPTO++_INCLUDE_DIR}
  ${JSONCPP_INCLUDE_DIR}
)

# Targets
ADD_EXECUTABLE( Alder ${ALDER_SOURCE} ${ALDER_UISrcs} ${MOCSrcs} )

TARGET_LINK_LIBRARIES( Alder
  QVTK
  vtkRendering
  vtkGraphics
  vtkIO
  vtkCommon
  ${QT_LIBRARIES}
  ${LIBXML2_LIBRARIES}
  ${CRYPTO++_LIBRARIES}
  ${JSONCPP_LIBRARIES}
)
INSTALL( TARGETS Alder RUNTIME DESTINATION bin )

ADD_CUSTOM_TARGET( dist
  COMMAND git archive --prefix=${ALDER_ARCHIVE_NAME}/ HEAD
    | bzip2 > ${CMAKE_BINARY_DIR}/${ALDER_ARCHIVE_NAME}.tar.bz2
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# Desktop entries and icons
IF( UNIX AND NOT APPLE )
  INSTALL( FILES desktop/Alder.desktop DESTINATION share/applications )

  INSTALL( FILES desktop/logo_clsa.png DESTINATION share/icons/hicolor/scalable/apps )
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
