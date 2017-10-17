cmake_minimum_required(VERSION 2.8.9)

project(SlicerOpenVR)

#-----------------------------------------------------------------------------
# Extension meta-information
set(EXTENSION_HOMEPAGE "http://slicer.org/slicerWiki/index.php/Documentation/Nightly/Extensions/VR")
set(EXTENSION_CATEGORY "Examples")
set(EXTENSION_CONTRIBUTORS "Jean-Baptiste Vimort (Kitware), Jean-Christophe Fillion-Robin (Kitware)")
set(EXTENSION_DESCRIPTION "This extension allow to use a an HTC vive to display data in Virtual Reallity")
set(EXTENSION_ICONURL "https://raw.githubusercontent.com/KitwareMedical/SlicerOpenVR/master/VR.pg")
set(EXTENSION_SCREENSHOTURLS "http://www.example.com/Slicer/Extensions/VR/Screenshots/1.png")
set(EXTENSION_DEPENDS "NA") # Specified as a space separated string, a list or 'NA' if any
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
  include("${CMAKE_CURRENT_SOURCE_DIR}/SuperBuild.cmake")
  return()
endif()

#-----------------------------------------------------------------------------
# Extension modules
add_subdirectory(VR)
## NEXT_MODULE

#-----------------------------------------------------------------------------
set(SlicerOpenVR_CUSTOM_CONFIG "####### Expanded from \@SlicerOpenVR_CUSTOM_CONFIG\@ #######
find_package(VTK COMPONENTS vtkRenderingOpenVR REQUIRED)
##################################################
")
include(${Slicer_EXTENSION_GENERATE_CONFIG})

#-----------------------------------------------------------------------------
# Install OpenVR
install(FILES ${OPENVR_LIBRARY}
  DESTINATION ${Slicer_THIRDPARTY_LIB_DIR}
  COMPONENT RuntimeLibraries
  )

#-----------------------------------------------------------------------------
set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${CMAKE_BINARY_DIR};${EXTENSION_NAME};ALL;/")
set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${VTKRenderingOpenVR_DIR};VTKRenderingOpenVR;RuntimeLibraries;/")
include(${Slicer_EXTENSION_CPACK})
