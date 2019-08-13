if (NOT FLAT_BUFFERS_FOUND)

    ExternalProject_Add(FLAT_BUFFERS
            DOWNLOAD_NAME       flatbuffers-release-1.8.0.tar.gz
            URL                 https://github.com/google/flatbuffers/archive/v1.10.0.tar.gz
            CONFIGURE_COMMAND   cd <SOURCE_DIR> && cmake -G "Unix Makefiles" -DFLATBUFFERS_BUILD_FLATC=ON -DFLATBUFFERS_BUILD_FLATHASH=OFF -DCMAKE_INSTALL_INCLUDEDIR=<INSTALL_DIR>/include -DCMAKE_INSTALL_BINDIR=<INSTALL_DIR>/bin -DCMAKE_INSTALL_LIBDIR=<INSTALL_DIR>/lib -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            BUILD_COMMAND       cd <SOURCE_DIR> && make
            INSTALL_COMMAND     cd <SOURCE_DIR> && make install
            TEST_COMMAND        ""
            )

    SET(DEPENDECIES ${DEPENDECIES} FLAT_BUFFERS)

    ExternalProject_Get_Property(FLAT_BUFFERS INSTALL_DIR)
    set (FLAT_BUFFERS_ROOT_DIR          ${INSTALL_DIR})
    set (FLAT_BUFFERS_INCLUDE_DIR       ${FLAT_BUFFERS_ROOT_DIR}/include)
    set (FLAT_BUFFERS_LIBRARY_DIR       ${FLAT_BUFFERS_ROOT_DIR}/lib)
    set (FLAT_BUFFERS_FOUND             YES)

endif ()
