project(vtkSlicer${MODULE_NAME}ModuleMRML)

set(KIT ${PROJECT_NAME})

#-----------------------------------------------------------------------------
find_package(VTK COMPONENTS vtkRenderingOpenVR REQUIRED)

#-----------------------------------------------------------------------------
set(${KIT}_EXPORT_DIRECTIVE "VTK_SLICER_${MODULE_NAME_UPPER}_MODULE_MRML_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  ${VTK_INCLUDE_DIRS}
  )

set(${KIT}_SRCS
  vtkMRML${MODULE_NAME}ViewDisplayableManagerFactory.cxx
  vtkMRML${MODULE_NAME}ViewDisplayableManagerFactory.h
  vtkMRML${MODULE_NAME}ViewNode.cxx
  vtkMRML${MODULE_NAME}ViewNode.h
  vtkMRML${MODULE_NAME}LayoutNode.cxx
  vtkMRML${MODULE_NAME}LayoutNode.h
  vtk${MODULE_NAME}ViewInteractorStyle.cxx
  vtk${MODULE_NAME}ViewInteractorStyle.h
  )

set(${KIT}_TARGET_LIBRARIES
  ${VTK_LIBRARIES}
  ${MRML_LIBRARIES}
  )

#-----------------------------------------------------------------------------
SlicerMacroBuildModuleMRML(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${${KIT}_INCLUDE_DIRECTORIES}
  SRCS ${${KIT}_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  )
