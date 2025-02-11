# Enable fetching content
include(FetchContent)

# Set the root of the Terrapane source repositories
if(DEFINED ENV{TERRAPANE_SOURCE_URI})
    set(TERRAPANE_SOURCE_URI "$ENV{TERRAPANE_SOURCE_URI}")
else()
    set(TERRAPANE_SOURCE_URI "https://github.com/terrapane")
endif()

if(netutil_BUILD_TESTS)
    # Fetch the Simple Test Framework library
    FetchContent_Declare(stf
        GIT_REPOSITORY ${TERRAPANE_SOURCE_URI}/stf.git
        GIT_TAG        v1.0.1
        GIT_SHALLOW    true)

    # It is not necessary to install the STF library
    set(stf_INSTALL OFF)

    # Make STF available
    FetchContent_MakeAvailable(stf)
endif()

# Fetch the bit-related utility functions
FetchContent_Declare(bitutil
    GIT_REPOSITORY ${TERRAPANE_SOURCE_URI}/bitutil.git
    GIT_TAG        v1.0.1
    GIT_SHALLOW    true)

# Install dependencies only if this library is installed
set(bitutil_INSTALL ${netutil_INSTALL})

# Make dependencies available
FetchContent_MakeAvailable(bitutil)
