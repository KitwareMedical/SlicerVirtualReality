project(vtkSlicer${MODULE_NAME}ModuleMRMLDisplayableManager)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "VTK_SLICER_${MODULE_NAME_UPPER}_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  ${vtkSlicer${MODULE_NAME}ModuleLogic_SOURCE_DIR}
  ${vtkSlicer${MODULE_NAME}ModuleLogic_BINARY_DIR}
  ${vtkSlicer${MODULE_NAME}ModuleMRML_SOURCE_DIR}
  ${vtkSlicer${MODULE_NAME}ModuleMRML_BINARY_DIR}
#  ${vtkSlicer${MODULE_NAME}ModuleVTKWidgets_SOURCE_DIR}
#  ${vtkSlicer${MODULE_NAME}ModuleVTKWidgets_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

set(${KIT}_SRCS
  vtkMRML${MODULE_NAME}ViewDisplayableManagerFactory.cxx
  vtkMRML${MODULE_NAME}ViewDisplayableManagerFactory.h
  vtk${MODULE_NAME}ComplexGestureRecognizer.cxx
  vtk${MODULE_NAME}ComplexGestureRecognizer.h
  vtk${MODULE_NAME}ViewInteractorObserver.cxx
  vtk${MODULE_NAME}ViewInteractorObserver.h
  vtk${MODULE_NAME}ViewInteractorStyleDelegate.cxx
  vtk${MODULE_NAME}ViewInteractorStyleDelegate.h
  )
if(SlicerVirtualReality_HAS_OPENVR_SUPPORT)
  list(APPEND ${KIT}_SRCS
    vtk${MODULE_NAME}ViewOpenVRInteractor.cxx
    vtk${MODULE_NAME}ViewOpenVRInteractor.h
    vtk${MODULE_NAME}ViewOpenVRInteractorStyle.cxx
    vtk${MODULE_NAME}ViewOpenVRInteractorStyle.h
    )
endif()
if(SlicerVirtualReality_HAS_OPENXR_SUPPORT)
  list(APPEND ${KIT}_SRCS
    vtk${MODULE_NAME}ViewOpenXRInteractor.cxx
    vtk${MODULE_NAME}ViewOpenXRInteractor.h
    vtk${MODULE_NAME}ViewOpenXRInteractorStyle.cxx
    vtk${MODULE_NAME}ViewOpenXRInteractorStyle.h
    )
endif()

set(${KIT}_TARGET_LIBRARIES
  ${MRML_LIBRARIES}
  vtkSlicer${MODULE_NAME}ModuleLogic
  vtkSlicer${MODULE_NAME}ModuleMRML
#  vtkSlicer${MODULE_NAME}ModuleVTKWidgets
)

#-----------------------------------------------------------------------------
SlicerMacroBuildModuleLogic(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${${KIT}_INCLUDE_DIRECTORIES}
  SRCS ${${KIT}_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
)
