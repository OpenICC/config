INCLUDE_DIRECTORIES(
                    "${CMAKE_SOURCE_DIR}"
                    "${CMAKE_SOURCE_DIR}/src"
                    "${CMAKE_SOURCE_DIR}/src/include"
                    "${CMAKE_BINARY_DIR}/src/include"
                    "${CMAKE_SOURCE_DIR}/src/include_private"
                    "${CMAKE_BINARY_DIR}/src/include_private"
                    "${CMAKE_SOURCE_DIR}/src/libopenicc"
)

# if our static code lands inside a shared lib, we need -fPIC
IF(UNIX)
  IF(CMAKE_COMPILER_IS_GNUCC)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
  ENDIF(CMAKE_COMPILER_IS_GNUCC)
ENDIF(UNIX)

# dependencies
INCLUDE(CheckLibraryExists)
INCLUDE(CheckIncludeFile)
CHECK_INCLUDE_FILE(dirent.h HAVE_DIRENT_H)
CHECK_INCLUDE_FILE(langinfo.h HAVE_LANGINFO_H)

CHECK_INCLUDE_FILE(libintl.h HAVE_LIBINTL_H)
FIND_LIBRARY( LIBINTL_LIBRARIES NAMES intl libintl libintl-8 )
IF(LIBINTL_LIBRARIES)
  SET( EXTRA_LIBS ${EXTRA_LIBS} ${LIBINTL_LIBRARIES} )
  SET( EXTRA_LIBS_CORE ${EXTRA_LIBS_CORE} ${LIBINTL_LIBRARIES} )
ELSE(LIBINTL_LIBRARIES)
  MESSAGE( "-- libintl not found" )
ENDIF(LIBINTL_LIBRARIES)

CHECK_LIBRARY_EXISTS( iconv libiconv_open "iconv.h" HAVE_ICONV )
IF( HAVE_ICONV )
  FIND_LIBRARY( LIBICONV_LIBRARIES NAMES libiconv-2 libiconv iconv )
  IF(LIBICONV_LIBRARIES)
    SET( ICONV_LIBS ${LIBICONV_LIBRARIES} )
  ELSE(LIBICONV_LIBRARIES)
    SET( ICONV_LIBS iconv )
  ENDIF(LIBICONV_LIBRARIES)
  SET( EXTRA_LIBS ${EXTRA_LIBS} ${ICONV_LIBS} )
  SET( EXTRA_LIBS_CORE ${EXTRA_LIBS_CORE} ${ICONV_LIBS} )
ELSE( HAVE_ICONV )
  CHECK_LIBRARY_EXISTS( iconv iconv_open "iconv.h" HAVE_ICONV_UNIX )
  IF( HAVE_ICONV_UNIX )
    SET( EXTRA_LIBS ${EXTRA_LIBS} iconv )
    SET( EXTRA_LIBS_CORE ${EXTRA_LIBS_CORE} iconv )
    SET( HAVE_ICONV TRUE )
  ENDIF( HAVE_ICONV_UNIX )
ENDIF( HAVE_ICONV )


CHECK_LIBRARY_EXISTS(m pow "math.h" HAVE_M)
IF(HAVE_M)
  SET( EXTRA_LIBS ${EXTRA_LIBS} m )
  SET( EXTRA_LIBS_CORE ${EXTRA_LIBS_CORE} m )
ENDIF(HAVE_M)

CHECK_LIBRARY_EXISTS(dl dlopen "dlfcn.h" HAVE_DL)
IF(HAVE_DL)
  SET( EXTRA_LIBS ${EXTRA_LIBS} dl )
ELSE(HAVE_DL)
  FIND_PATH(DL_INCLUDE_DIR dlfcn.h)
  IF(DL_INCLUDE_DIR)
    SET( HAVE_DL 1 )
    SET( EXTRA_LIBS ${EXTRA_LIBS} dl )
  ENDIF(DL_INCLUDE_DIR)
ENDIF(HAVE_DL)

FIND_PATH(LTDL_INCLUDE_DIR ltdl.h )
IF(LTDL_INCLUDE_DIR)
  SET( HAVE_LTDL 1 )
  SET( EXTRA_LIBS ${EXTRA_LIBS} ltdl )
ENDIF(LTDL_INCLUDE_DIR)

IF(NOT HAVE_DL AND NOT HAVE_LTDL)
  MESSAGE( SEND_ERROR "need one of libdl with dlfcn.h or libltdl with ltdl.h" )
ENDIF(NOT HAVE_DL AND NOT HAVE_LTDL)

CHECK_LIBRARY_EXISTS(c malloc "stdio.h" HAVE_C)
IF(HAVE_C)
  SET( EXTRA_LIBS ${EXTRA_LIBS} c )
ENDIF(HAVE_C)

FIND_PATH( STDINT_INCLUDE_DIR stdint.h
           HINTS /mingw ${INCLUDE_DIRECTORIES}
           PATH_SUFFIXES include )
IF(STDINT_INCLUDE_DIR)
  SET( HAVE_STDINT_H 1 )
  MESSAGE( "-- stdint.h: " ${STDINT_INCLUDE_DIR} )
ELSE(STDINT_INCLUDE_DIR)
  MESSAGE( "-- stdint.h not found" )
ENDIF(STDINT_INCLUDE_DIR)

FIND_PATH( INTTYPES_INCLUDE_DIR inttypes.h
           HINTS /mingw ${INCLUDE_DIRECTORIES}
           PATH_SUFFIXES include )
IF(INTTYPES_INCLUDE_DIR)
  SET( HAVE_INTTYPES_H 1 )
  MESSAGE( "-- inttypes.h: " ${INTTYPES_INCLUDE_DIR} )
ELSE(INTTYPES_INCLUDE_DIR)
  MESSAGE( "-- inttypes.h not found" )
ENDIF(INTTYPES_INCLUDE_DIR)

FIND_PACKAGE( LCMS )
IF( HAVE_LCMS )
  INCLUDE_DIRECTORIES( ${LCMS_INCLUDE_DIRS} )
  LINK_DIRECTORIES( ${LCMS_LIBRARY_DIRS} )
ENDIF()

FIND_PACKAGE( LCMS2 )
IF( HAVE_LCMS2 )
  INCLUDE_DIRECTORIES( ${LCMS2_INCLUDE_DIRS} )
  LINK_DIRECTORIES( ${LCMS2_LIBRARY_DIRS} )
ENDIF()

IF(NOT LCMS_FOUND AND NOT LCMS2_FOUND )
  MESSAGE( SEND_ERROR "Need at least one little CMS installation. lcms2 is prefered; Download http://www.littlecms.com" )
ENDIF()

FIND_PACKAGE( OpenMP )
IF(OPENMP_FOUND)
  INCLUDE_DIRECTORIES( ${OPENMP_INCLUDE_DIRS} )
  LINK_DIRECTORIES( ${OPENMP_LIBRARY_DIRS} )
  SET( EXTRA_LIBS ${EXTRA_LIBS} ${OPENMP_LIBRARIES} )
  FIND_LIBRARY( LIBGOMP_LIBRARIES NAMES gomp gomp-1 )
  IF( LIBGOMP_LIBRARIES )
    SET( EXTRA_LIBS ${EXTRA_LIBS} ${LIBGOMP_LIBRARIES} )
  ENDIF( LIBGOMP_LIBRARIES )
  SET( HAVE_OPENMP 1 )
  SET( CMAKE_C_FLAGS_CORE "${CMAKE_C_FLAGS}" )
  SET( CMAKE_CXX_FLAGS_CORE "${CMAKE_CXX_FLAGS}" )
  SET( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}" )
  SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}" )
ENDIF(OPENMP_FOUND)

FIND_PACKAGE( GetText )
MESSAGE( "GetText: ${GETTEXT_FOUND}" )
IF( GETTEXT_FOUND )
  SET( USE_GETTEXT TRUE )
ENDIF()

# debug
GET_PROPERTY(inc_dirs DIRECTORY PROPERTY INCLUDE_DIRECTORIES)
#MESSAGE( "-- INCLUDE_DIRECTORIES: ${inc_dirs}" )


ADD_SUBDIRECTORY( include )

ADD_SUBDIRECTORY( libopenicc )

#ADD_SUBDIRECTORY( tests )

#ADD_SUBDIRECTORY( tools )

ADD_SUBDIRECTORY( examples )


# versions for libraries
SET_TARGET_PROPERTIES( openicc
    PROPERTIES VERSION   ${OPENICC_VERSION_MAJOR}.${OPENICC_VERSION_MINOR}.${OPENICC_VERSION_MICRO}
               SOVERSION ${OPENICC_VERSION_MAJOR}
                     )


SET( DOC_SOURCES
	doxymentation_common
	${CFILES_OBJECT_CONFIG_PUBLIC}
	${CHEADERS_OBJECT_CONFIG_PUBLIC}
        PARENT_SCOPE
   )


# i18n
IF( GETTEXT_FOUND )
  SET(_potFile ${CMAKE_SOURCE_DIR}/po/${PACKAGE_NAME}.pot)
  #SET( _xgettext_option_list --language=C --keyword=_ --keyword=N_ --keyword=C_:1c,2 --keyword=NC_:1c,2 -s --package-name=${PROJECT_NAME} --package-version=${OPENICC_VERSION_MAJOR}.${OPENICC_VERSION_MINOR} )
  SET( _xgettext_option_list --add-comments --keyword=gettext --flag=gettext:1:pass-c-format --keyword=_ --flag=_:1:pass-c-format --keyword=N_ --flag=N_:1:pass-c-format  --copyright-holder='Kai-Uwe Behrmann'  --msgid-bugs-address='ku.b@gmx.de' --from-code=utf-8 --package-name=${PROJECT_NAME} --package-version=${OPENICC_VERSION_MAJOR}.${OPENICC_VERSION_MINOR} )
  SET( XGETTEXT_CMD xgettext )
  FILE( GLOB SRC_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
        libopenicc/*.c
      )

  ADD_CUSTOM_COMMAND(OUTPUT ${_potFile}
    COMMAND ${XGETTEXT_CMD} ${_xgettext_option_list} -o ${_potFile} ${SRC_FILES}
    DEPENDS ${SRC_FILES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Extract translatable messages to ${_potFile}"
  )

  SET( LINGUAS ../po/cs.po ../po/de.po ../po/eo.po ../po/eu.po  ../po/fr.po ../po/ru.po )
  # compile translations always to have them ready for installation
  GETTEXT_CREATE_TRANSLATIONS( ${_potFile} ALL ${LINGUAS} )
ENDIF()

