
// VirtualReality MRML includes
#include <vtkMRMLVirtualRealityLayoutNode.h>

// MRML includes
#include <vtkMRMLCoreTestingMacros.h>

int vtkMRMLVirtualRealityLayoutNodeTest1(int , char * [])
{
  vtkNew<vtkMRMLVirtualRealityLayoutNode> node1;
  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());

  return EXIT_SUCCESS;
}
