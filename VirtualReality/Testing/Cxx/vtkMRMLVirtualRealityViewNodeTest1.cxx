
// VirtualReality MRML includes
#include <vtkMRMLVirtualRealityViewNode.h>

// MRML includes
#include <vtkMRMLCoreTestingMacros.h>

int vtkMRMLVirtualRealityViewNodeTest1(int , char * [])
{
  vtkNew<vtkMRMLVirtualRealityViewNode> node1;
  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());

  CHECK_INT(vtkMRMLVirtualRealityViewNode::GetXRBackendFromString(nullptr), -1);
  CHECK_INT(vtkMRMLVirtualRealityViewNode::GetXRBackendFromString(""), -1);
  CHECK_INT(vtkMRMLVirtualRealityViewNode::GetXRBackendFromString("any"), -1);
  CHECK_INT(vtkMRMLVirtualRealityViewNode::GetXRBackendFromString("undefined"), vtkMRMLVirtualRealityViewNode::UndefinedXRBackend);
  CHECK_INT(vtkMRMLVirtualRealityViewNode::GetXRBackendFromString("OpenVR"), vtkMRMLVirtualRealityViewNode::OpenVR);
  CHECK_INT(vtkMRMLVirtualRealityViewNode::GetXRBackendFromString("OpenXR"), vtkMRMLVirtualRealityViewNode::OpenXR);

  return EXIT_SUCCESS;
}
