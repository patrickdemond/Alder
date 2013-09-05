PROJECT(AlderBusiness)

SET(AlderBusiness_SRCS
  ActiveRecord.cxx
  Application.cxx
  Configuration.cxx
  Database.cxx
  Exam.cxx
  Image.cxx
  Interview.cxx
  Modality.cxx
  ModelObject.cxx
  OpalService.cxx
  QueryModifier.cxx
  Rating.cxx
  User.cxx
)

SET_SOURCE_FILES_PROPERTIES(
  ActiveRecord.cxx
  ModelObject.cxx
  ABSTRACT
)

INCLUDE_DIRECTORIES( 
  ${CMAKE_CURRENT_BINARY_DIR}
  #${LIBXML2_INCLUDE_DIR}
  ${CRYPTO++_INCLUDE_DIR}
  #${JSONCPP_INCLUDE_DIR}
  #${CURL_INCLUDE_DIR}
)

ADD_LIBRARY( AlderBusiness ${AlderBusiness_SRCS} )

TARGET_LINK_LIBRARIES( AlderBusiness
  AlderCommon
  AlderVTK
  #gdcmMSFF
  #${LIBXML2_LIBRARIES}
  ${CRYPTO++_LIBRARIES}
  #${JSONCPP_LIBRARIES}
  #${CURL_LIBRARY}
)

INSTALL( TARGETS AlderBusiness 
  ARCHIVE DESTINATION lib  
  LIBRARY DESTINATION lib
  COMPONENT library
)
