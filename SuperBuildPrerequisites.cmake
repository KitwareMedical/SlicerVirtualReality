if(DEFINED slicersources_SOURCE_DIR AND NOT DEFINED Slicer_SOURCE_DIR)
  # Explicitly setting "Slicer_SOURCE_DIR" when only "slicersources_SOURCE_DIR"
  # is defined is required to successfully complete configuration in an empty
  # build directory
  #
  # Indeed, in that case, Slicer sources have been downloaded by they have not been
  # added using "add_subdirectory()" and the variable "Slicer_SOURCE_DIR" is not yet in
  # in the CACHE.
  set(Slicer_SOURCE_DIR ${slicersources_SOURCE_DIR})
endif()

if(NOT DEFINED SlicerVirtualReality_HAS_OPENVR_SUPPORT)
  message(FATAL_ERROR "SlicerVirtualReality_HAS_OPENVR_SUPPORT is not set")
endif()

# Set list of dependencies to ensure the custom application bundling this
# extension does NOT automatically collect the project list and attempt to
# build external projects associated with VTK modules enabled below.
if(DEFINED Slicer_SOURCE_DIR)
  # Extension is bundled in a custom application
  set(SlicerVirtualReality_EXTERNAL_PROJECT_DEPENDENCIES "")
  if(SlicerVirtualReality_HAS_OPENVR_SUPPORT)
    list(APPEND SlicerVirtualReality_EXTERNAL_PROJECT_DEPENDENCIES
      OpenVR
      )
  endif()
else()
  # Extension is build standalone against Slicer itself built
  # against VTK without the relevant modules enabled.
  set(SlicerVirtualReality_EXTERNAL_PROJECT_DEPENDENCIES
    vtkRenderingVR
    )
  if(SlicerVirtualReality_HAS_OPENVR_SUPPORT)
    list(APPEND SlicerVirtualReality_EXTERNAL_PROJECT_DEPENDENCIES
      vtkRenderingOpenVR
      )
  endif()
endif()
message(STATUS "SlicerVirtualReality_EXTERNAL_PROJECT_DEPENDENCIES:${SlicerVirtualReality_EXTERNAL_PROJECT_DEPENDENCIES}")

if(NOT DEFINED Slicer_SOURCE_DIR)
  # Extension is built standalone

  # VTKExternalModule is required to configure these external projects:
  # - vtkRenderingVR
  # - vtkRenderingOpenVR
  include(${SlicerVirtualReality_SOURCE_DIR}/FetchVTKExternalModule.cmake)

else()
  # Extension is bundled in a custom application

  # Additional external project dependencies
  if(SlicerVirtualReality_HAS_OPENVR_SUPPORT)
    ExternalProject_Add_Dependencies(VTK
      DEPENDS
        OpenVR
      )
  endif()

  # Additional external project options
  set(VTK_MODULE_ENABLE_VTK_RenderingVR YES)
  if(SlicerVirtualReality_HAS_OPENVR_SUPPORT)
    set(VTK_MODULE_ENABLE_VTK_RenderingOpenVR YES)
  else()
    set(VTK_MODULE_ENABLE_VTK_RenderingOpenVR NO)
  endif()

  mark_as_superbuild(
    VARS
      VTK_MODULE_ENABLE_VTK_RenderingVR:STRING
      VTK_MODULE_ENABLE_VTK_RenderingOpenVR:STRING
    PROJECTS
      VTK
    )

endif()

