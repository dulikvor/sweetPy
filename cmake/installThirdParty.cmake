include(ExternalProject) #In order to load the ExternalProject module and to add the ExternalProject_Add command
set_directory_properties(PROPERTIES EP_PREFIX ${pyCppConn_3RD_PARTY_DIR}) #Sets the prefix(dir) for all installations commands
include(InstallCore)
if((pyCppConn_Test_Support) AND (UNIX AND NOT APPLE))
    include(InstallGoogleTest)
endif()

