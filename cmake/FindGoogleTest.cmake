find_path(GOOGLE_TEST_INCLUDE_DIR NAMES benchmark PATHS ${PROJECT_DIR}/Third_Party/include NO_DEFAULT_PATH)
find_library(GOOGLE_TEST_LIBRARY_DIR NAMES benchmark PATHS ${PROJECT_DIR}/Third_Party/lib NO_DEFAULT_PATH)
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(benchmark REQUIRED_VARS GOOGLE_TEST_INCLUDE_DIR GOOGLE_TEST_LIBRARY_DIR)

if(GOOGLE_TEST_FOUND)
    message(STATUS "$Found Google Test include dir - ${Green}${GOOGLE_TEST_INCLUDE_DIR}{ColourReset}")
    message(STATUS "$Found Google Test library dir - ${Green}${GOOGLE_TEST_LIBRARY_DIR}{ColourReset}")
else()

    message(WARNING "${Red}Google Test not found${ColourReset}")
    message(WARNING "Google Test not found")
endif()
