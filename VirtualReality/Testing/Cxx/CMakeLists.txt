set(KIT qSlicer${MODULE_NAME}Module)

#-----------------------------------------------------------------------------
set(KIT_TEST_SRCS
  vtkMRMLVirtualRealityLayoutNodeTest1.cxx
  vtkMRMLVirtualRealityViewNodeTest1.cxx
  )

#-----------------------------------------------------------------------------
slicerMacroConfigureModuleCxxTestDriver(
  NAME ${KIT}
  SOURCES ${KIT_TEST_SRCS}
  WITH_VTK_DEBUG_LEAKS_CHECK
  WITH_VTK_ERROR_OUTPUT_CHECK
  )

#-----------------------------------------------------------------------------
simple_test(vtkMRMLVirtualRealityLayoutNodeTest1)
simple_test(vtkMRMLVirtualRealityViewNodeTest1)
