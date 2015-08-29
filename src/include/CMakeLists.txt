CONFIGURE_FILE (
		"${CMAKE_CURRENT_SOURCE_DIR}/openicc_version.h.in"
		"${CMAKE_BINARY_DIR}/src/include/openicc_version.h"
		@ONLY
	)

SET( CHEADERS_PUBLIC
	openicc_config.h
	openicc_db.h
   )
SET( CHEADERS_PUBLIC
	${CHEADERS_PUBLIC}
        PARENT_SCOPE
   )

INSTALL( FILES
	${CHEADERS_PUBLIC}
        "${CMAKE_BINARY_DIR}/src/include/openicc_version.h"
	DESTINATION include/${PROJECT_DOWN_NAME})
