set(proj OpenVR)

# Set dependency list
set(${proj}_DEPENDENCIES "")

# Sanity checks
if(DEFINED OPENVR_INCLUDE_DIR AND NOT EXISTS ${OPENVR_INCLUDE_DIR})
  message(FATAL_ERROR "OPENVR_INCLUDE_DIR variable is defined but corresponds to nonexistent directory")
endif()
if(DEFINED OPENVR_LIBRARY AND NOT EXISTS ${OPENVR_LIBRARY})
  message(FATAL_ERROR "OPENVR_LIBRARY variable is defined but corresponds to nonexistent path")
endif()

if(${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})
  unset(OPENVR_INCLUDE_DIR CACHE)
  unset(OPENVR_LIBRARY CACHE)
  find_package(OpenVR REQUIRED MODULE)
endif()

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

if((NOT OPENVR_INCLUDE_DIR OR NOT OPENVR_LIBRARY)
   AND NOT ${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})

  set(_version "1.7.15")

  ExternalProject_Add(${proj}
    ${${proj}_EP_ARGS}
    URL "https://github.com/ValveSoftware/openvr/archive/v${_version}.tar.gz"
    URL_MD5 "88f30043996b4735baf59b020d13e25e"
    DOWNLOAD_DIR ${CMAKE_BINARY_DIR}
    SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    DEPENDS
      ${${proj}_DEPENDENCIES}
    )

  ExternalProject_GenerateProjectDescription_Step(${proj}
    VERSION ${_version}
    )

  set(OpenVR_DIR ${CMAKE_BINARY_DIR}/${proj})

  set(OPENVR_INCLUDE_DIR "${OpenVR_DIR}/headers")

  if(WIN32)
    set(OPENVR_LIBRARY "${OpenVR_DIR}/lib/win64/openvr_api.lib")
  elseif(APPLE)
    set(OPENVR_LIBRARY "${OpenVR_DIR}/bin/osx64/OpenVR.framework")
  elseif(UNIX)
    set(OPENVR_LIBRARY "${OpenVR_DIR}/bin/linux64/libopenvr_api.so")
  endif()
  mark_as_superbuild(OPENVR_LIBRARY:FILEPATH)

  #-----------------------------------------------------------------------------
  # Launcher setting specific to build tree

  if(WIN32)
    set(_dir "${OpenVR_DIR}/bin/win64")
  elseif(APPLE)
    set(_dir "${OpenVR_DIR}/bin/osx64")
  elseif(UNIX)
    set(_dir "${OpenVR_DIR}/bin/linux64")
  endif()
  set(${proj}_LIBRARY_PATHS_LAUNCHER_BUILD ${_dir})
  mark_as_superbuild(
    VARS ${proj}_LIBRARY_PATHS_LAUNCHER_BUILD
    LABELS "LIBRARY_PATHS_LAUNCHER_BUILD"
    )

else()
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
endif()

ExternalProject_Message(${proj} "OPENVR_INCLUDE_DIR:${OPENVR_INCLUDE_DIR}")
ExternalProject_Message(${proj} "OPENVR_LIBRARY:${OPENVR_LIBRARY}")
