project(vtkSlicer${MODULE_NAME}ModuleLogic)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "VTK_SLICER_${MODULE_NAME_UPPER}_MODULE_LOGIC_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  )

set(${KIT}_SRCS
  vtkSlicer${MODULE_NAME}Logic.cxx
  vtkSlicer${MODULE_NAME}Logic.h
  )

set(${KIT}_TARGET_LIBRARIES
  vtkSlicer${MODULE_NAME}ModuleMRML
  vtkSlicerVolumeRenderingModuleLogic
  ${ITK_LIBRARIES}
  )

#-----------------------------------------------------------------------------
SlicerMacroBuildModuleLogic(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${${KIT}_INCLUDE_DIRECTORIES}
  SRCS ${${KIT}_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  )

#-----------------------------------------------------------------------------
# Deploy the OpenXR action manifest and binding file(s) owned by this module,
# so that vtkSlicerVirtualRealityLogic::ComputeActionManifestPath() can locate
# them under this module's own share directory instead of reaching into
# vtkRenderingOpenXR's own xr_actions install location.
if(SlicerVirtualReality_HAS_OPENXR_SUPPORT)
  set(openxr_action_files
    vtk_openxr_actions.json
    vtk_openxr_binding_oculus_touch_controller.json
    )
  foreach(openxr_action_file ${openxr_action_files})
    configure_file(
      ${CMAKE_CURRENT_SOURCE_DIR}/../Resources/Bindings/${openxr_action_file}
      ${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_SHARE_DIR}/${MODULE_NAME}/xr_actions/${openxr_action_file}
      COPYONLY)
    install(
      FILES ${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_SHARE_DIR}/${MODULE_NAME}/xr_actions/${openxr_action_file}
      DESTINATION ${Slicer_INSTALL_QTLOADABLEMODULES_SHARE_DIR}/${MODULE_NAME}/xr_actions
      COMPONENT Runtime)
  endforeach()
endif()

#-----------------------------------------------------------------------------
# Deploy the OpenXR controller render-model manifest and glTF assets owned by this module, so
# that vtkSlicerVirtualRealityLogic::ComputeControllerModelsPath() can locate them under this
# module's own share directory. This is a workaround for OpenXR providing no API for rendering
# controller models (see vtkOpenXRRenderWindow::SetModelsManifestDirectory()).
if(SlicerVirtualReality_HAS_OPENXR_SUPPORT)
  set(openxr_controller_models_dir ${CMAKE_CURRENT_SOURCE_DIR}/../Resources/ControllerModels)
  file(GLOB_RECURSE openxr_controller_model_files RELATIVE ${openxr_controller_models_dir} ${openxr_controller_models_dir}/*)
  foreach(openxr_controller_model_file ${openxr_controller_model_files})
    configure_file(
      ${openxr_controller_models_dir}/${openxr_controller_model_file}
      ${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_SHARE_DIR}/${MODULE_NAME}/xr_controller_models/${openxr_controller_model_file}
      COPYONLY)
    get_filename_component(openxr_controller_model_file_dir ${openxr_controller_model_file} DIRECTORY)
    install(
      FILES ${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_SHARE_DIR}/${MODULE_NAME}/xr_controller_models/${openxr_controller_model_file}
      DESTINATION ${Slicer_INSTALL_QTLOADABLEMODULES_SHARE_DIR}/${MODULE_NAME}/xr_controller_models/${openxr_controller_model_file_dir}
      COMPONENT Runtime)
  endforeach()
endif()
