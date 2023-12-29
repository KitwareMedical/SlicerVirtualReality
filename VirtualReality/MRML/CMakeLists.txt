project(vtkSlicer${MODULE_NAME}ModuleMRML)

set(KIT ${PROJECT_NAME})

#-----------------------------------------------------------------------------
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkMRMLVirtualRealityConfigure.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/vtkMRMLVirtualRealityConfigure.h
  )

#-----------------------------------------------------------------------------
set(${KIT}_EXPORT_DIRECTIVE "VTK_SLICER_${MODULE_NAME_UPPER}_MODULE_MRML_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  ${VTK_INCLUDE_DIRS}
  )

set(${KIT}_SRCS
  vtkMRML${MODULE_NAME}ViewNode.cxx
  vtkMRML${MODULE_NAME}ViewNode.h
  vtkMRML${MODULE_NAME}LayoutNode.cxx
  vtkMRML${MODULE_NAME}LayoutNode.h
  )

set(${KIT}_TARGET_LIBRARIES
  ${VTK_LIBRARIES}
  ${MRML_LIBRARIES}
  VTK::RenderingVR
  )
if(SlicerVirtualReality_HAS_OPENVR_SUPPORT)
  list(APPEND ${KIT}_TARGET_LIBRARIES
    VTK::RenderingOpenVR
    )
endif()
if(SlicerVirtualReality_HAS_OPENXR_SUPPORT)
  list(APPEND ${KIT}_TARGET_LIBRARIES
    VTK::RenderingOpenXR
    )
endif()

#-----------------------------------------------------------------------------
SlicerMacroBuildModuleMRML(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${${KIT}_INCLUDE_DIRECTORIES}
  SRCS ${${KIT}_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  )
