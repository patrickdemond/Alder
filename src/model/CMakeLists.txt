PROJECT(AlderModel)

SET(AlderModel_SRCS
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
  ${LIBXML2_INCLUDE_DIR}
  ${CRYPTO++_INCLUDE_DIR}
  ${JSONCPP_INCLUDE_DIR}
  ${CURL_INCLUDE_DIR}
)

ADD_LIBRARY( AlderModel ${AlderModel_SRCS} )

TARGET_LINK_LIBRARIES( AlderModel
  gdcmMSFF
  ${LIBXML2_LIBRARIES}
  ${CRYPTO++_LIBRARIES}
  ${JSONCPP_LIBRARIES}
  ${CURL_LIBRARY}
)

INSTALL( TARGETS AlderModel DESTINATION bin )
