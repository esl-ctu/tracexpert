find_path(PICOSCOPESDK_INCLUDE_DIR
    NAMES
        ps6000Api.h
    PATHS
        ENV "PROGRAMFILES"
        $ENV{PROGRAMFILES}/Pico\ Technology/SDK
        /opt/picoscope/
    PATH_SUFFIXES
        inc
        include/
        include/libps6000)

find_library(PICOSCOPESDK_LIBRARY
    NAMES ps6000
    PATHS
        ENV "PROGRAMFILES"
        $ENV{PROGRAMFILES}/Pico\ Technology/SDK
        /opt/picoscope/
    PATH_SUFFIXES
        lib)
        
find_library(PICOSCOPE6000A_LIBRARY
    NAMES ps6000a
    PATHS
        ENV "PROGRAMFILES"
        $ENV{PROGRAMFILES}/Pico\ Technology/SDK
        /opt/picoscope/
    PATH_SUFFIXES
        lib)
               
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  PicoScopeSDK
  FOUND_VAR PICOSCOPESDK_FOUND
  REQUIRED_VARS PICOSCOPESDK_LIBRARY PICOSCOPESDK_INCLUDE_DIR)
