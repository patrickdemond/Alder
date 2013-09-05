#CMAKE_MINIMUM_REQUIRED( VERSION 2.6 )
PROJECT( AlderTesting )

INCLUDE_DIRECTORIES(
  ${CRYPTO++_INCLUDE_DIR}
  ${CURL_INCLUDE_DIR}
)

ADD_EXECUTABLE( OpalHttp OpalHttp.cxx )

TARGET_LINK_LIBRARIES( OpalHttp
  ${CRYPTO++_LIBRARIES}
  ${CURL_LIBRARY}
)

INSTALL( TARGETS OpalHttp RUNTIME DESTINATION bin )

ADD_EXECUTABLE( BackTrace BackTrace.cxx )

INSTALL( TARGETS BackTrace RUNTIME DESTINATION bin )

ADD_EXECUTABLE( DemangledBackTrace DemangledBackTrace.cxx )

TARGET_LINK_LIBRARIES( DemangledBackTrace
  vtkCommon # we need this for dladdr to work (magic!)
)

INSTALL( TARGETS DemangledBackTrace RUNTIME DESTINATION bin )
