
set(proj OpenXRRemoting)

set(${proj}_DEPENDENCIES "")

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

if(${SUPERBUILD_TOPLEVEL_PROJECT}_USE_SYSTEM_${proj})
  message(FATAL_ERROR "Enabling ${SUPERBUILD_TOPLEVEL_PROJECT}_USE_SYSTEM_${proj} is not supported !")
endif()

# Sanity checks
if(DEFINED OpenXRRemoting_BIN_DIR AND NOT EXISTS ${OpenXRRemoting_BIN_DIR})
  message(FATAL_ERROR "OpenXRRemoting_BIN_DIR variable is defined but corresponds to nonexistent directory")
endif()
if(DEFINED OpenXRRemoting_INCLUDE_DIR AND NOT EXISTS ${OpenXRRemoting_INCLUDE_DIR})
  message(FATAL_ERROR "OpenXRRemoting_INCLUDE_DIR variable is defined but corresponds to nonexistent path")
endif()

if((NOT OpenXRRemoting_BIN_DIR OR NOT OpenXRRemoting_INCLUDE_DIR)
   AND NOT ${SUPERBUILD_TOPLEVEL_PROJECT}_USE_SYSTEM_${proj})

  set(EP_INSTALL_DIR ${CMAKE_BINARY_DIR}/${proj}-install)

  set(_ver "2.9.3")
  set(_sha256 "9ef533aeff9ddef40104ad0d03e1e631c314729b18690385a4a624fab2408797")

  ExternalProject_Add(${proj}
    ${${proj}_EP_ARGS}
    URL "https://www.nuget.org/api/v2/package/Microsoft.Holographic.Remoting.OpenXr/${_ver}"
    URL_HASH "SHA256=${_sha256}"
    SOURCE_DIR ${EP_INSTALL_DIR}
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    DEPENDS
      ${${proj}_DEPENDENCIES}
    )

  set(OpenXRRemoting_BIN_DIR "${EP_INSTALL_DIR}/build/native/bin/x64/Desktop/")
  set(OpenXRRemoting_INCLUDE_DIR "${EP_INSTALL_DIR}/build/native/include/openxr/")

  #-----------------------------------------------------------------------------
  # Launcher setting specific to build tree

  # library paths
  set(${proj}_LIBRARY_PATHS_LAUNCHER_BUILD
    ${OpenXRRemoting_BIN_DIR}
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

ExternalProject_Message(${proj} "OpenXRRemoting_BIN_DIR:${OpenXRRemoting_BIN_DIR}")
ExternalProject_Message(${proj} "OpenXRRemoting_INCLUDE_DIR:${OpenXRRemoting_INCLUDE_DIR}")

mark_as_superbuild(
  VARS
    OpenXRRemoting_BIN_DIR:PATH
    OpenXRRemoting_INCLUDE_DIR:PATH
  )

