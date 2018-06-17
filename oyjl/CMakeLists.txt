CMAKE_MINIMUM_REQUIRED (VERSION 2.8.5)

PROJECT (Oyjl)
SET (PROJECT_UP_NAME "OYJL" )
SET (PROJECT_DOWN_NAME "oyjl" )

SET( ${PROJECT_UP_NAME}_VERSION_MAJOR 1)
SET( ${PROJECT_UP_NAME}_VERSION_MINOR 0)
SET( ${PROJECT_UP_NAME}_VERSION_MICRO 0)
SET( ${PROJECT_UP_NAME}_VERSION "${${PROJECT_UP_NAME}_VERSION_MAJOR}.${${PROJECT_UP_NAME}_VERSION_MINOR}.${${PROJECT_UP_NAME}_VERSION_MICRO}" )

SET (PACKAGE_NAME "${PROJECT_DOWN_NAME}" )
SET (PACKAGE_DESCRIPTION "The Oyjl project provides a string and string list manipulation, json and yaml parsing, and json handling functions in a library and tools.")

SET( ${PROJECT_NAME}_VERSION_MAJORMINOR "${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}" )

INCLUDE(GNUInstallDirs)

OPTION(ENABLE_TESTS "Build tests (requires static libs)" ON)
OPTION(ENABLE_TOOLS "Build tools" ON)
OPTION(ENABLE_SHARED_LIBS "Build dynamic link libs" ON)
OPTION(ENABLE_STATIC_LIBS "Build static libs" ON)

IF(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  OPTION(ENABLE_INSTALL_${PROJECT_UP_NAME} "Install files" ON)
ELSE()
  MESSAGE( "-- Oyjl: compile as subproject - skip install (ENABLE_INSTALL_${PROJECT_UP_NAME}=OFF)" )
  # disable install and build of shared libs for fallback subproject
  IF(NOT ENABLE_INSTALL_${PROJECT_UP_NAME})
    SET(ENABLE_INSTALL_${PROJECT_UP_NAME} OFF)
  ENDIF(NOT ENABLE_INSTALL_${PROJECT_UP_NAME})
  SET(ENABLE_STATIC_LIBS ON)
  SET(ENABLE_SHARED_LIBS OFF)
  SET(ENABLE_TESTS OFF)
  SET(ENABLE_TOOLS OFF)
ENDIF()

# if our static code lands inside a shared lib, we need -fPIC
IF(ENABLE_SHARED_LIBS)
  SET( CMAKE_POSITION_INDEPENDENT_CODE ON)
ENDIF()

IF( CMAKE_BUILD_TYPE MATCHES "[Dd][Ee][Bb][Uu][Gg]" )
  MESSAGE( "-- Info build \"${CMAKE_BUILD_TYPE}\"" )
  SET( DEBUG 1 )
ENDIF()

#
# Target installation folders
#

SET (${PROJECT_UP_NAME}_INCLUDE_FOLDER
		${PROJECT_NAME}
		CACHE STRING
		"Optional folder below system include folder to install include files."
    )

IF(NOT CMAKE_MODULES_INSTALL_DIR)
  SET (CMAKE_MODULES_INSTALL_DIR
		"${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_DOWN_NAME}"
		CACHE STRING
		"The folder where to install cmake files."
    )
ENDIF(NOT CMAKE_MODULES_INSTALL_DIR)


SET (TARGET_PKGCONFIG_FOLDER
		"pkgconfig"
		CACHE STRING
		"The folder below system library folder where to install pkgconfig files."
    )

SET (${PROJECT_UP_NAME}_DOCUMENTATION_FOLDER
		"share/doc/${PACKAGE_NAME}-api"
		CACHE STRING
		"The folder below system prefix where to install api documentation files."
    )

# additional cmake modules for loading libraries
SET (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/")

IF(USE_SYSTEM_YAJL)
  FIND_PACKAGE(Yajl REQUIRED)
ELSE()
  FIND_PACKAGE(Yajl)
ENDIF()
IF( HAVE_YAJL )
  INCLUDE_DIRECTORIES( ${YAJL_INCLUDE_DIRS} )
  LINK_DIRECTORIES( ${YAJL_LIBRARY_DIRS} )
  INCLUDE( CheckIncludeFiles )
  CHECK_INCLUDE_FILES( yajl/yajl_version.h HAVE_YAJL_VERSION_H )
  # yajl2 should come with yajl_version.h, the included yajl is version 1.0.x
  IF( HAVE_YAJL_VERSION_H )
    MESSAGE("-- yajl: ${YAJL_VERSION}")
  ELSE( HAVE_YAJL_VERSION_H )
    INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/yajl )
    MESSAGE("-- yajl: trying internal headers")
  ENDIF( HAVE_YAJL_VERSION_H )
  SET( CONFIG_YAJL_LIBRARIES ${YAJL_LIBRARIES} )
ELSE( HAVE_YAJL )
  IF(CMAKE_VERSION VERSION_GREATER 2.8.0)
    #include(ExternalProject)
    #ExternalProject_Add( yajl
    #SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/yajl
    #CONFIGURE_COMMAND cmake ${CMAKE_CURRENT_SOURCE_DIR}/yajl
    #BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/yajl
    #BUILD_COMMAND ${MAKE}
    # skip installation
    #INSTALL_COMMAND echo )
    INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/yajl )
    LINK_DIRECTORIES( ${CMAKE_CURRENT_BINARY_DIR}/yajl )
    SET( CMAKE_POSITION_INDEPENDENT_CODE ON)
    ADD_SUBDIRECTORY( yajl )
    SET( YAJL_LIBRARIES yajl-static )
    SET( YAJL_LIBRARIES ${YAJL_LIBRARIES} PARENT_SCOPE )
  ENDIF()
ENDIF( HAVE_YAJL )

INCLUDE(CheckLibraryExists)
INCLUDE(CheckIncludeFile)
CHECK_LIBRARY_EXISTS(m pow "math.h" HAVE_M)
IF(HAVE_M)
  SET( EXTRA_LIBS ${EXTRA_LIBS} m )
ENDIF(HAVE_M)
CHECK_INCLUDE_FILE(locale.h     HAVE_LOCALE_H)

CONFIGURE_FILE(
    "${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_DOWN_NAME}_version.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_DOWN_NAME}_version.h"
    IMMEDIATE @ONLY)
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_BINARY_DIR} )

## library 

SET( CFILES_OYJL_CORE_PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_DOWN_NAME}_core.c
   )
SET( CFILES_OYJL_TREE_PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_DOWN_NAME}_tree.c
   )
SET( CHEADERS_OYJL_PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_DOWN_NAME}_macros.h
	${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_DOWN_NAME}.h
   )
SET( CHEADERS_OYJL
	${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_DOWN_NAME}_tree_internal.h
	${CHEADERS_OYJL_PUBLIC}
   )

SET( SOURCES_CORE ${CFILES_OYJL_CORE_PUBLIC} )
SET( SOURCES_TREE ${CFILES_OYJL_TREE_PUBLIC} ${CHEADERS_OYJL} )
SET( EXTRA_LIBS ${YAJL_LIBRARIES} ${EXTRA_LIBS} )
MESSAGE( "-- EXTRA_LIBS:               ${EXTRA_LIBS}" )


IF(ENABLE_SHARED_LIBS)
  ADD_LIBRARY( ${PROJECT_NAME}Core SHARED ${SOURCES_CORE} )
  ADD_LIBRARY( ${PROJECT_NAME} SHARED ${SOURCES_TREE} ${CHEADERS_OYJL_PUBLIC} )
  TARGET_LINK_LIBRARIES ( ${PROJECT_NAME} ${PROJECT_NAME}Core ${YAJL_LIBRARIES} )

  SET_TARGET_PROPERTIES( ${PROJECT_NAME} PROPERTIES
  PRIVATE_HEADER ""
  PUBLIC_HEADER "${CHEADERS_OYJL_PUBLIC}"
  RESOURCE ""
  )

  IF(ENABLE_INSTALL_${PROJECT_UP_NAME})
    INSTALL( TARGETS       ${PROJECT_NAME}Core
         RUNTIME       DESTINATION ${CMAKE_INSTALL_BINDIR}
         FRAMEWORK     DESTINATION ${FRAMEWORK_DIR}
         LIBRARY       DESTINATION ${CMAKE_INSTALL_LIBDIR}
         ARCHIVE       DESTINATION ${CMAKE_INSTALL_LIBDIR}
         PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
         RESOURCE      DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}
         )
    INSTALL( TARGETS       ${PROJECT_NAME}
         RUNTIME       DESTINATION ${CMAKE_INSTALL_BINDIR}
         FRAMEWORK     DESTINATION ${FRAMEWORK_DIR}
         LIBRARY       DESTINATION ${CMAKE_INSTALL_LIBDIR}
         ARCHIVE       DESTINATION ${CMAKE_INSTALL_LIBDIR}
         PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
         RESOURCE      DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}
         )
  ENDIF(ENABLE_INSTALL_${PROJECT_UP_NAME})
  SET( OYJL_TARGET_LIB ${PROJECT_NAME} )
ENDIF(ENABLE_SHARED_LIBS)

IF(ENABLE_STATIC_LIBS)
  ADD_LIBRARY( ${PACKAGE_NAME}-core-static STATIC ${SOURCES_CORE} )
  ADD_LIBRARY( ${PACKAGE_NAME}-static STATIC ${SOURCES_TREE} ${CHEADERS_OYJL_PUBLIC} )
  TARGET_LINK_LIBRARIES( ${PACKAGE_NAME}-static ${PACKAGE_NAME}-core-static ${YAJL_LIBRARIES} )

  IF(ENABLE_INSTALL_${PROJECT_UP_NAME})
    INSTALL( TARGETS       ${PACKAGE_NAME}-core-static
           RUNTIME       DESTINATION ${CMAKE_INSTALL_BINDIR}
           FRAMEWORK     DESTINATION ${FRAMEWORK_DIR}
           LIBRARY       DESTINATION ${CMAKE_INSTALL_LIBDIR}
           ARCHIVE       DESTINATION ${CMAKE_INSTALL_LIBDIR}
           PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
           RESOURCE      DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/color/settings
         )
    INSTALL( TARGETS       ${PACKAGE_NAME}-static
           RUNTIME       DESTINATION ${CMAKE_INSTALL_BINDIR}
           FRAMEWORK     DESTINATION ${FRAMEWORK_DIR}
           LIBRARY       DESTINATION ${CMAKE_INSTALL_LIBDIR}
           ARCHIVE       DESTINATION ${CMAKE_INSTALL_LIBDIR}
           PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
           RESOURCE      DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/color/settings
         )
    IF(NOT ENABLE_SHARED_LIBS)
      INSTALL( FILES ${CHEADERS_OYJL_PUBLIC}
             DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} )
    ENDIF(NOT ENABLE_SHARED_LIBS)
  ENDIF(ENABLE_INSTALL_${PROJECT_UP_NAME})
  IF( NOT OYJL_TARGET_LIB )
    SET( OYJL_TARGET_LIB ${PACKAGE_NAME}-static )
  ENDIF( NOT OYJL_TARGET_LIB )
ENDIF(ENABLE_STATIC_LIBS)

SET( EXTRA_LIBS ${EXTRA_LIBS} PARENT_SCOPE )

IF(ENABLE_TOOLS)
  ADD_EXECUTABLE( ${PACKAGE_NAME} "${CMAKE_CURRENT_SOURCE_DIR}/oyjl.c" )
  TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} ${OYJL_TARGET_LIB} ${EXTRA_LIBS} )
  IF(ENABLE_INSTALL_${PROJECT_UP_NAME})
    INSTALL (TARGETS ${PACKAGE_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})

  ENDIF(ENABLE_INSTALL_${PROJECT_UP_NAME})
  ADD_EXECUTABLE( jsontoyaml "${CMAKE_CURRENT_SOURCE_DIR}/oyjl.c" )
  TARGET_LINK_LIBRARIES( jsontoyaml ${OYJL_TARGET_LIB} ${EXTRA_LIBS} )
  IF(ENABLE_INSTALL_${PROJECT_UP_NAME})
    INSTALL (TARGETS jsontoyaml DESTINATION ${CMAKE_INSTALL_BINDIR})
  ENDIF(ENABLE_INSTALL_${PROJECT_UP_NAME})

  # needs libyaml
  # dependencies
  CHECK_LIBRARY_EXISTS( yaml yaml_document_get_node "yaml.h" HAVE_YAML )
  IF( HAVE_YAML )
    FIND_LIBRARY(YAML_LIBRARIES NAMES yaml libyaml PATH)
    FIND_PATH(YAML_INCLUDE_DIR yaml.h)
    INCLUDE_DIRECTORIES( ${YAML_INCLUDE_DIR} )
    MESSAGE("-- yaml: ${YAML_INCLUDE_DIR} : ${YAML_LIBRARIES}")

    ADD_EXECUTABLE( yamltojson "${CMAKE_CURRENT_SOURCE_DIR}/yaml2json.c" )
    TARGET_LINK_LIBRARIES( yamltojson ${OYJL_TARGET_LIB} ${EXTRA_LIBS} ${YAML_LIBRARIES} )
    IF(ENABLE_INSTALL_${PROJECT_UP_NAME})
      INSTALL (TARGETS yamltojson DESTINATION ${CMAKE_INSTALL_BINDIR})
    ENDIF(ENABLE_INSTALL_${PROJECT_UP_NAME})
  ENDIF( HAVE_YAML )
ENDIF(ENABLE_TOOLS)

IF( ENABLE_TESTS )
  ADD_EXECUTABLE( oyjl-test "${CMAKE_CURRENT_SOURCE_DIR}/test.c" )
  TARGET_LINK_LIBRARIES( oyjl-test ${OYJL_TARGET_LIB} ${EXTRA_LIBS} )
  ADD_EXECUTABLE( oyjl-test-static "${CMAKE_CURRENT_SOURCE_DIR}/test.c" )
  TARGET_LINK_LIBRARIES( oyjl-test-static ${PACKAGE_NAME}-static ${PACKAGE_NAME}-core-static ${EXTRA_LIBS} )
  ADD_CUSTOM_TARGET(check
        ./oyjl-test
        DEPENDS oyjl-test
        COMMENT "Test liboyjl." VERBATIM )
ENDIF( ENABLE_TESTS )

IF(ENABLE_INSTALL_${PROJECT_UP_NAME})
  CONFIGURE_FILE(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}Uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}Uninstall.cmake"
    IMMEDIATE @ONLY)

  ADD_CUSTOM_TARGET( uninstall
	  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}Uninstall.cmake")
ENDIF(ENABLE_INSTALL_${PROJECT_UP_NAME})

#
# Misc.
#

#cmake settings for use by find_package
CONFIGURE_FILE (
		"${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in"
		"${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}Config.cmake"
		@ONLY
	)

IF(ENABLE_INSTALL_${PROJECT_UP_NAME})
  INSTALL ( FILES
		"${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}Config.cmake"
		DESTINATION ${CMAKE_MODULES_INSTALL_DIR} COMPONENT dev
	)
ENDIF(ENABLE_INSTALL_${PROJECT_UP_NAME})

