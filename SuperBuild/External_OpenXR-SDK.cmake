
set(proj OpenXR-SDK)

set(${proj}_DEPENDENCIES "")

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

if(${SUPERBUILD_TOPLEVEL_PROJECT}_USE_SYSTEM_${proj})
  unset(OpenXR_INCLUDE_DIR CACHE)
  unset(OpenXR_LIBRARY CACHE)
  find_package(OpenXR REQUIRED)
  if(NOT DEFINED OpenXR_LIBRARY)
    # Since the OpenXRConfig.cmake file provided by OpenXR-SDK does not set OpenXR_LIBRARY, its
    # value may be retrieved from the OpenXR::OpenXR target.
    # TODO
  endif()
endif()

# Sanity checks
if(DEFINED OpenXR_INCLUDE_DIR AND NOT EXISTS ${OpenXR_INCLUDE_DIR})
  message(FATAL_ERROR "OpenXR_INCLUDE_DIR variable is defined but corresponds to nonexistent directory")
endif()
if(DEFINED OpenXR_LIBRARY AND NOT EXISTS ${OpenXR_LIBRARY})
  message(FATAL_ERROR "OpenXR_LIBRARY variable is defined but corresponds to nonexistent path")
endif()

if((NOT OpenXR_INCLUDE_DIR OR NOT OpenXR_LIBRARY)
   AND NOT ${SUPERBUILD_TOPLEVEL_PROJECT}_USE_SYSTEM_${proj})

  ExternalProject_SetIfNotDefined(
    ${SUPERBUILD_TOPLEVEL_PROJECT}_${proj}_GIT_REPOSITORY
    "https://github.com/KhronosGroup/OpenXR-SDK.git"
    QUIET
    )

  ExternalProject_SetIfNotDefined(
    ${SUPERBUILD_TOPLEVEL_PROJECT}_${proj}_GIT_TAG
    "release-1.0.26"
    QUIET
    )

  set(EP_SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj})
  set(EP_BINARY_DIR ${CMAKE_BINARY_DIR}/${proj}-build)
  set(EP_INSTALL_DIR ${CMAKE_BINARY_DIR}/${proj}-install)

  ExternalProject_Add(${proj}
    ${${proj}_EP_ARGS}
    GIT_REPOSITORY "${${SUPERBUILD_TOPLEVEL_PROJECT}_${proj}_GIT_REPOSITORY}"
    GIT_TAG "${${SUPERBUILD_TOPLEVEL_PROJECT}_${proj}_GIT_TAG}"
    SOURCE_DIR ${EP_SOURCE_DIR}
    BINARY_DIR ${EP_BINARY_DIR}
    INSTALL_DIR ${EP_INSTALL_DIR}
    CMAKE_CACHE_ARGS
      # Compiler settings
      -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
      -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
      -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
      -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
      -DCMAKE_CXX_STANDARD:STRING=${CMAKE_CXX_STANDARD}
      -DCMAKE_CXX_STANDARD_REQUIRED:BOOL=${CMAKE_CXX_STANDARD_REQUIRED}
      -DCMAKE_CXX_EXTENSIONS:BOOL=${CMAKE_CXX_EXTENSIONS}
      # Options
      -DBUILD_TESTING:BOOL=OFF
      -DDYNAMIC_LOADER:BOOL=ON
      # Install directories
      -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
      ${EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS}
    DEPENDS
      ${${proj}_DEPENDENCIES}
    )
  set(OpenXR_DIR "${EP_INSTALL_DIR}/lib/cmake/openxr")

  set(OpenXR_INCLUDE_DIR "${EP_INSTALL_DIR}/include/openxr")

  if(WIN32)
    set(OpenXR_LIBRARY "${EP_INSTALL_DIR}/lib/openxr_loader.lib")
  elseif(APPLE)
    set(OpenXR_LIBRARY "${EP_INSTALL_DIR}/lib/libopenxr_loader.dylib")
  elseif(UNIX)
    set(OpenXR_LIBRARY "${EP_INSTALL_DIR}/lib/libopenxr_loader.so")
  endif()

  #-----------------------------------------------------------------------------
  # Launcher setting specific to build tree

  # library paths
  set(${proj}_LIBRARY_PATHS_LAUNCHER_BUILD
    ${EP_BINARY_DIR}/src/loader/<CMAKE_CFG_INTDIR>
    )
  mark_as_superbuild(
    VARS ${proj}_LIBRARY_PATHS_LAUNCHER_BUILD
    LABELS "LIBRARY_PATHS_LAUNCHER_BUILD"
    )

  #-----------------------------------------------------------------------------
  # Launcher setting specific to install tree

  # NA

else()
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDS})
endif()

ExternalProject_Message(${proj} "OpenXR_INCLUDE_DIR:${OpenXR_INCLUDE_DIR}")
ExternalProject_Message(${proj} "OpenXR_LIBRARY:${OpenXR_LIBRARY}")

mark_as_superbuild(
  VARS
    OpenXR_INCLUDE_DIR:PATH
    OpenXR_LIBRARY:FILEPATH
  )

