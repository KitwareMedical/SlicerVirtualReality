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
if(APPLE)
  # Virtual Reality is not supported on macOS, create an extension that contains a stub module
  # that just displays the message that Virtual Reality is not supported. This avoids having
  # a build error on the extension build dashboard.
  find_package(Slicer REQUIRED)
  include(${Slicer_USE_FILE})
  add_subdirectory(VirtualRealityStub)
  include(${Slicer_EXTENSION_GENERATE_CONFIG})
  include(${Slicer_EXTENSION_CPACK}) 
  return()
endif()
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
# SuperBuild setup
option(${EXTENSION_NAME}_SUPERBUILD "Build ${EXTENSION_NAME} and the projects it depends on." ON)
mark_as_advanced(${EXTENSION_NAME}_SUPERBUILD)
if(${EXTENSION_NAME}_SUPERBUILD)
  include("${CMAKE_CURRENT_SOURCE_DIR}/SuperBuildPrerequisites.cmake")
  include("${CMAKE_CURRENT_SOURCE_DIR}/SuperBuild.cmake")
  return()
endif()

#-----------------------------------------------------------------------------
# Extension modules
add_subdirectory(VirtualReality)
## NEXT_MODULE

#-----------------------------------------------------------------------------
set(SlicerVirtualReality_CUSTOM_CONFIG "####### Expanded from \@SlicerVirtualReality_CUSTOM_CONFIG\@ #######
set(vtkRenderingOpenVR_DIR \"${vtkRenderingOpenVR_DIR}\")
find_package(vtkRenderingOpenVR REQUIRED)
##################################################
")
include(${Slicer_EXTENSION_GENERATE_CONFIG})

#-----------------------------------------------------------------------------
# Install OpenVR
set(_platform)
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  # ${OpenVR_LIBRARY} contains import library which does not have to be installed.
  # Instead, the dll must be added to the package.
  install(FILES ${OpenVR_LIBRARY_PATHS_LAUNCHER_BUILD}/openvr_api.dll
    DESTINATION ${Slicer_THIRDPARTY_LIB_DIR}
    COMPONENT RuntimeLibraries
    )
else()
  install(FILES ${OpenVR_LIBRARY}
    DESTINATION ${Slicer_THIRDPARTY_LIB_DIR}
    COMPONENT RuntimeLibraries
    )
endif()

#-----------------------------------------------------------------------------
set(EXTENSION_CPACK_INSTALL_CMAKE_PROJECTS)
list(APPEND EXTENSION_CPACK_INSTALL_CMAKE_PROJECTS "${vtkRenderingVR_DIR};vtkRenderingVR;runtime;/")
list(APPEND EXTENSION_CPACK_INSTALL_CMAKE_PROJECTS "${vtkRenderingOpenVR_DIR};vtkRenderingOpenVR;runtime;/")
if(Slicer_USE_PYTHONQT)
  list(APPEND EXTENSION_CPACK_INSTALL_CMAKE_PROJECTS "${vtkRenderingVR_DIR};vtkRenderingVR;python;/")
  list(APPEND EXTENSION_CPACK_INSTALL_CMAKE_PROJECTS "${vtkRenderingOpenVR_DIR};vtkRenderingOpenVR;python;/")
endif()
set(${EXTENSION_NAME}_CPACK_INSTALL_CMAKE_PROJECTS "${EXTENSION_CPACK_INSTALL_CMAKE_PROJECTS}" CACHE STRING "List of external projects to install" FORCE)

#-----------------------------------------------------------------------------
list(APPEND CPACK_INSTALL_CMAKE_PROJECTS "${CMAKE_BINARY_DIR};${EXTENSION_NAME};ALL;/")
list(APPEND CPACK_INSTALL_CMAKE_PROJECTS "${${EXTENSION_NAME}_CPACK_INSTALL_CMAKE_PROJECTS}")
include(${Slicer_EXTENSION_CPACK})
