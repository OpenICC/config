CONFIGURE_FILE(
                "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in"
                "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
                @ONLY
       )
CONFIGURE_FILE(
                "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.files.in"
                "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.files"
                @ONLY
       )

FIND_PACKAGE( Doxygen )
IF( DOXYGEN_FOUND )

  ADD_CUSTOM_COMMAND( OUTPUT html
    COMMAND cat ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.files | grep ^FILE_PATTERNS | sed 's%\;% %g' >> Doxyfile \; doxygen Doxyfile \;
                    )

  ADD_CUSTOM_TARGET( docu ALL
                     DEPENDS html )

  INSTALL( DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html DESTINATION ${CMAKE_INSTALL_DOCDIR} )
ENDIF()
