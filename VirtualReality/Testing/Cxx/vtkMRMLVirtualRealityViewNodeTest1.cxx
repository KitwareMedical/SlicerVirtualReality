
// VirtualReality MRML includes
#include <vtkMRMLVirtualRealityViewNode.h>

// MRML includes
#include <vtkMRMLCoreTestingMacros.h>

int vtkMRMLVirtualRealityViewNodeTest1(int , char * [])
{
  vtkNew<vtkMRMLVirtualRealityViewNode> node1;
  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());

  return EXIT_SUCCESS;
}
