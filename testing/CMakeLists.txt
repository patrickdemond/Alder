CMAKE_MINIMUM_REQUIRED( VERSION 2.6 )
PROJECT( AlderTesting )

ADD_EXECUTABLE( OpalHttp OpalHttp.cxx )
TARGET_LINK_LIBRARIES( OpalHttp
  ${CURL_LIBRARY}
  ${CRYPTO++_LIBRARIES}
)
INSTALL( TARGETS OpalHttp RUNTIME DESTINATION bin )

ADD_EXECUTABLE( BackTrace BackTrace.cxx )
INSTALL( TARGETS BackTrace RUNTIME DESTINATION bin )

ADD_EXECUTABLE( DemangledBackTrace DemangledBackTrace.cxx )
TARGET_LINK_LIBRARIES( DemangledBackTrace
  vtkCommon # we need this for dladdr to work (magic!)
)
INSTALL( TARGETS DemangledBackTrace RUNTIME DESTINATION bin )
