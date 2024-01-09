cmake_minimum_required(VERSION 3.5)

project(SlicerVirtualReality)

#-----------------------------------------------------------------------------
# Extension meta-information
set(EXTENSION_HOMEPAGE "https://github.com/KitwareMedical/SlicerVirtualReality")
set(EXTENSION_CATEGORY "Virtual Reality")
set(EXTENSION_CONTRIBUTORS "Jean-Baptiste Vimort (Kitware), Jean-Christophe Fillion-Robin (Kitware), Csaba Pinter (PerkLab, Queen's University), Adam Rankin (VASST Lab, Robarts Research Institute, Western University)")
set(EXTENSION_DESCRIPTION "Allows user to interact with a Slicer scene using virtual reality.")
set(EXTENSION_ICONURL "https://raw.githubusercontent.com/KitwareMedical/SlicerVirtualReality/master/SlicerVirtualReality.png")
set(EXTENSION_SCREENSHOTURLS "https://www.slicer.org/w/images/4/49/SlicerVirtualReality_Screenshot1.png https://www.slicer.org/w/images/0/04/SlicerVirtualReality_Screenshot2.png")
set(EXTENSION_DEPENDS "NA") # Specified as a space separated string, a list or 'NA' if any

#-----------------------------------------------------------------------------
set(EXTENSION_BUILD_SUBDIRECTORY inner-build)

set(SUPERBUILD_TOPLEVEL_PROJECT inner)

#-----------------------------------------------------------------------------
# Extension dependencies
find_package(Slicer REQUIRED)
include(${Slicer_USE_FILE})
mark_as_superbuild(Slicer_DIR)

find_package(Git REQUIRED)
mark_as_superbuild(GIT_EXECUTABLE)

#-----------------------------------------------------------------------------
# Options

# OpenVR
set(_default ON)
set(_reason)
if(NOT DEFINED SlicerVirtualReality_HAS_OPENVR_SUPPORT AND APPLE)
  set(_default OFF)
  set(_reason " (OpenVR not supported on macOS)")
endif()
option(SlicerVirtualReality_HAS_OPENVR_SUPPORT "Build OpenVR XR runtime" ${_default})
mark_as_superbuild(SlicerVirtualReality_HAS_OPENVR_SUPPORT)
message(STATUS "")
message(STATUS "Setting SlicerVirtualReality_HAS_OPENVR_SUPPORT to ${SlicerVirtualReality_HAS_OPENVR_SUPPORT}${_reason}")
message(STATUS "")

# OpenXR
set(_default ON)
set(_reason)
option(SlicerVirtualReality_HAS_OPENXR_SUPPORT "Build OpenXR XR runtime" ${_default})
mark_as_superbuild(SlicerVirtualReality_HAS_OPENXR_SUPPORT)
message(STATUS "")
message(STATUS "Setting SlicerVirtualReality_HAS_OPENXR_SUPPORT to ${SlicerVirtualReality_HAS_OPENXR_SUPPORT}${_reason}")
message(STATUS "")

#-----------------------------------------------------------------------------
# SuperBuild setup
option(${EXTENSION_NAME}_SUPERBUILD "Build ${EXTENSION_NAME} and the projects it depends on." ON)
mark_as_advanced(${EXTENSION_NAME}_SUPERBUILD)
if(${EXTENSION_NAME}_SUPERBUILD)
  include("${CMAKE_CURRENT_SOURCE_DIR}/SuperBuildPrerequisites.cmake")
  include("${CMAKE_CURRENT_SOURCE_DIR}/SuperBuild.cmake")
  return()
endif()

#-----------------------------------------------------------------------------
# Virtual Reality is not supported on macOS, create an extension that contains a stub module
# that just displays the message that Virtual Reality is not supported.
# This avoids having a build error on the extension build dashboard.
if(APPLE)
  add_subdirectory(VirtualRealityStub)

  include(${Slicer_EXTENSION_GENERATE_CONFIG})

  set(EXTENSION_CPACK_INSTALL_CMAKE_PROJECTS)
  set(${EXTENSION_NAME}_CPACK_INSTALL_CMAKE_PROJECTS "${EXTENSION_CPACK_INSTALL_CMAKE_PROJECTS}" CACHE STRING "List of external projects to install" FORCE)

  list(APPEND CPACK_INSTALL_CMAKE_PROJECTS "${CMAKE_BINARY_DIR};${EXTENSION_NAME};ALL;/")
  list(APPEND CPACK_INSTALL_CMAKE_PROJECTS "${${EXTENSION_NAME}_CPACK_INSTALL_CMAKE_PROJECTS}")
  include(${Slicer_EXTENSION_CPACK})
  return()
endif()

#-----------------------------------------------------------------------------
# Extension modules
add_subdirectory(VirtualReality)
## NEXT_MODULE

#-----------------------------------------------------------------------------
set(SlicerVirtualReality_CUSTOM_CONFIG "####### Expanded from \@SlicerVirtualReality_CUSTOM_CONFIG\@ #######

set(vtkRenderingVR_DIR \"${vtkRenderingVR_DIR}\")
find_package(vtkRenderingVR REQUIRED)

set(SlicerVirtualReality_HAS_OPENVR_SUPPORT ${SlicerVirtualReality_HAS_OPENVR_SUPPORT})
if(SlicerVirtualReality_HAS_OPENVR_SUPPORT)
  set(vtkRenderingOpenVR_DIR \"${vtkRenderingOpenVR_DIR}\")
  find_package(vtkRenderingOpenVR REQUIRED)
endif()

set(SlicerVirtualReality_HAS_OPENXR_SUPPORT ${SlicerVirtualReality_HAS_OPENXR_SUPPORT})
if(SlicerVirtualReality_HAS_OPENXR_SUPPORT)
  set(vtkRenderingOpenXR_DIR \"${vtkRenderingOpenXR_DIR}\")
  find_package(vtkRenderingOpenXR REQUIRED)
endif()
##################################################
")
include(${Slicer_EXTENSION_GENERATE_CONFIG})

#-----------------------------------------------------------------------------
# Install OpenVR
set(_platform)
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  if(SlicerVirtualReality_HAS_OPENVR_SUPPORT)
    # ${OpenVR_LIBRARY} contains import library which does not have to be installed.
    # Instead, the dll must be added to the package.
    install(FILES ${OpenVR_LIBRARY_PATHS_LAUNCHER_BUILD}/openvr_api.dll
      DESTINATION ${Slicer_THIRDPARTY_LIB_DIR}
      COMPONENT RuntimeLibraries
      )
  endif()

  if(SlicerVirtualReality_HAS_OPENXR_SUPPORT AND CMAKE_VERSION VERSION_LESS "3.21")
    # ${OpenXR_LIBRARY} contains import library which does not have to be installed.
    # Instead, the dll must be added to the package.
    set(_library "${OpenXR-SDK_LIBRARY_PATHS_LAUNCHER_BUILD}/openxr_loader.dll")
    # Since the launcher settings include the placeholder <CMAKE_CFG_INTDIR>, let's
    # replace if with the corresponding generator expression.
    string(REPLACE "<CMAKE_CFG_INTDIR>" "$<CONFIG>" _library ${_library})
    install(FILES ${_library}
      DESTINATION ${Slicer_THIRDPARTY_LIB_DIR}
      COMPONENT RuntimeLibraries
      )
  endif()
else()
  if(SlicerVirtualReality_HAS_OPENVR_SUPPORT)
    install(FILES ${OpenVR_LIBRARY}
      DESTINATION ${Slicer_THIRDPARTY_LIB_DIR}
      COMPONENT RuntimeLibraries
      )
  endif()

  if(SlicerVirtualReality_HAS_OPENXR_SUPPORT AND CMAKE_VERSION VERSION_LESS "3.21")
    find_package(OpenXR REQUIRED)
    string(TOUPPER ${CMAKE_BUILD_TYPE} _config)
    get_target_property(_imported_library OpenXR::openxr_loader IMPORTED_LOCATION_${_config})
    if(_imported_library)
      install(FILES ${_imported_library}
        DESTINATION ${Slicer_INSTALL_THIRDPARTY_LIB_DIR}
        COMPONENT RuntimeLibraries
        )
      message(STATUS "Adding install rule for ${_imported_library}")
    else()
      message(WARNING "Failed to retrieve OpenXR library imported location: OpenXR library will not be installed.")
    endif()
  endif()
endif()

if(SlicerVirtualReality_HAS_OPENXR_SUPPORT AND CMAKE_VERSION VERSION_GREATER_EQUAL "3.21")
  find_package(OpenXR REQUIRED)
  install(IMPORTED_RUNTIME_ARTIFACTS OpenXR::openxr_loader
    RUNTIME DESTINATION ${Slicer_INSTALL_THIRDPARTY_BIN_DIR} COMPONENT RuntimeLibraries
    LIBRARY DESTINATION ${Slicer_INSTALL_THIRDPARTY_LIB_DIR} COMPONENT RuntimeLibraries
    )
endif()

#-----------------------------------------------------------------------------
set(EXTENSION_CPACK_INSTALL_CMAKE_PROJECTS)

list(APPEND EXTENSION_CPACK_INSTALL_CMAKE_PROJECTS "${vtkRenderingVR_DIR};vtkRenderingVR;runtime;/")
if(Slicer_USE_PYTHONQT)
  list(APPEND EXTENSION_CPACK_INSTALL_CMAKE_PROJECTS "${vtkRenderingVR_DIR};vtkRenderingVR;python;/")
endif()

if(SlicerVirtualReality_HAS_OPENVR_SUPPORT)
  list(APPEND EXTENSION_CPACK_INSTALL_CMAKE_PROJECTS "${vtkRenderingOpenVR_DIR};vtkRenderingOpenVR;runtime;/")
  if(Slicer_USE_PYTHONQT)
    list(APPEND EXTENSION_CPACK_INSTALL_CMAKE_PROJECTS "${vtkRenderingOpenVR_DIR};vtkRenderingOpenVR;python;/")
  endif()
endif()

if(SlicerVirtualReality_HAS_OPENXR_SUPPORT)
  list(APPEND EXTENSION_CPACK_INSTALL_CMAKE_PROJECTS "${vtkRenderingOpenXR_DIR};vtkRenderingOpenXR;runtime;/")
  if(Slicer_USE_PYTHONQT)
    list(APPEND EXTENSION_CPACK_INSTALL_CMAKE_PROJECTS "${vtkRenderingOpenXR_DIR};vtkRenderingOpenXR;python;/")
  endif()
endif()

set(${EXTENSION_NAME}_CPACK_INSTALL_CMAKE_PROJECTS "${EXTENSION_CPACK_INSTALL_CMAKE_PROJECTS}" CACHE STRING "List of external projects to install" FORCE)

#-----------------------------------------------------------------------------
list(APPEND CPACK_INSTALL_CMAKE_PROJECTS "${CMAKE_BINARY_DIR};${EXTENSION_NAME};ALL;/")
list(APPEND CPACK_INSTALL_CMAKE_PROJECTS "${${EXTENSION_NAME}_CPACK_INSTALL_CMAKE_PROJECTS}")
include(${Slicer_EXTENSION_CPACK})
