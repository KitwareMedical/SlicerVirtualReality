#-----------------------------------------------------------------------------
set(MODULE_NAME VRStage)

#-----------------------------------------------------------------------------
set(MODULE_PYTHON_SCRIPTS
  ${MODULE_NAME}.py
  VRStageLib/__init__
  VRStageLib/Constants
  VRStageLib/ParameterNode
  VRStageLib/BakedText
  VRStageLib/Props
  VRStageLib/FramingMath
  VRStageLib/MeasurementTool
  VRStageLib/ReformatTool
  VRStageLib/SceneViews
  VRStageLib/StageLighting
  VRStageLib/OrientationLabels
  VRStageLib/WallTiles
  VRStageLib/RoomChrome
  VRStageLib/StageFraming
  VRStageLib/Logic
  )

set(MODULE_PYTHON_RESOURCES
  Resources/Icons/${MODULE_NAME}.svg
  Resources/UI/${MODULE_NAME}.ui
  Resources/AtlasThumbnails/abdominal.png
  Resources/AtlasThumbnails/brain.png
  Resources/AtlasThumbnails/knee.png
  )

#-----------------------------------------------------------------------------
slicerMacroBuildScriptedModule(
  NAME ${MODULE_NAME}
  SCRIPTS ${MODULE_PYTHON_SCRIPTS}
  RESOURCES ${MODULE_PYTHON_RESOURCES}
  WITH_GENERIC_TESTS
  )

#-----------------------------------------------------------------------------
if(BUILD_TESTING)

  # Register the unittest subclass in the main script as a ctest.
  # Note that the test will also be available at runtime.
  slicer_add_python_unittest(SCRIPT ${MODULE_NAME}.py)

  # Additional build-time testing
  add_subdirectory(Testing)
endif()
