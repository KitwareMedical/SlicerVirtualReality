
// VirtualReality MRML includes
#include <vtkMRMLVirtualRealityViewNode.h>

// MRML includes
#include <vtkMRMLCoreTestingMacros.h>

int vtkMRMLVirtualRealityViewNodeTest1(int , char * [])
{
  vtkNew<vtkMRMLVirtualRealityViewNode> node1;
  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());

  CHECK_INT(vtkMRMLVirtualRealityViewNode::GetXRRuntimeFromString(nullptr), -1);
  CHECK_INT(vtkMRMLVirtualRealityViewNode::GetXRRuntimeFromString(""), -1);
  CHECK_INT(vtkMRMLVirtualRealityViewNode::GetXRRuntimeFromString("any"), -1);
  CHECK_INT(vtkMRMLVirtualRealityViewNode::GetXRRuntimeFromString("undefined"), vtkMRMLVirtualRealityViewNode::UndefinedXRRuntime);
  CHECK_INT(vtkMRMLVirtualRealityViewNode::GetXRRuntimeFromString("OpenVR"), vtkMRMLVirtualRealityViewNode::OpenVR);
  return EXIT_SUCCESS;
}
