PROJECT( AlderCommon )

SET( AlderCommon_SRCS
  Utilities.cxx
)

INCLUDE_DIRECTORIES( 
  ${CRYPTO++_INCLUDE_DIR}
)

ADD_LIBRARY( AlderCommon ${AlderCommon_SRCS} )

INSTALL( TARGETS AlderCommon
  ARCHIVE DESTINATION lib 
  LIBRARY DESTINATION lib
  COMPONENT library
)
