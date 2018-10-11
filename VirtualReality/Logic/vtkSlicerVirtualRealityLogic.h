/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// .NAME vtkSlicerVirtualRealityLogic - slicer logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes


#ifndef __vtkSlicerVirtualRealityLogic_h
#define __vtkSlicerVirtualRealityLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// MRML includes

// STD includes
#include <cstdlib>

#include "vtkSlicerVirtualRealityModuleLogicExport.h"

class vtkMRMLVirtualRealityViewNode;

class VTK_SLICER_VIRTUALREALITY_MODULE_LOGIC_EXPORT vtkSlicerVirtualRealityLogic :
  public vtkSlicerModuleLogic
{
public:

  static vtkSlicerVirtualRealityLogic *New();
  vtkTypeMacro(vtkSlicerVirtualRealityLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  /// Creates a singleton virtual reality view node and adds it to the scene.
  /// If there is a virtual reality view node in the scene already then it just returns that.
  /// If current view node is created, deleted, or modified then a Modified() event is invoked
  /// for this logic class, to make it easy for modules to detect view changes.
  vtkMRMLVirtualRealityViewNode* AddVirtualRealityViewNode();

  /// Get singleton virtual reality node
  vtkMRMLVirtualRealityViewNode* GetVirtualRealityViewNode();

  /// Retrieves the default VR view node from the scene. Creates it if does not exist.
  vtkMRMLVirtualRealityViewNode* GetDefaultVirtualRealityViewNode();

  /// Connect/disconnect to headset.
  /// Adds virtual reality view node if not added yet.
  void SetVirtualRealityConnected(bool connect);
  bool GetVirtualRealityConnected();

  /// Enable rendering updates in headset.
  /// Connects to device if not yet connected.
  void SetVirtualRealityActive(bool activate);
  bool GetVirtualRealityActive();

  /// Set the first visible 3D view as reference view for
  /// virtual reality view.
  /// If a reference view has been already set then the
  /// method has no effect.
  void SetDefaultReferenceView();

protected:
  vtkSlicerVirtualRealityLogic();
  virtual ~vtkSlicerVirtualRealityLogic();

  void SetActiveViewNode(vtkMRMLVirtualRealityViewNode* vrViewNode);

  vtkMRMLVirtualRealityViewNode* ActiveViewNode;

  virtual void SetMRMLSceneInternal(vtkMRMLScene* newScene);
  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  virtual void RegisterNodes();
  virtual void UpdateFromMRMLScene();
  virtual void OnMRMLSceneNodeAdded(vtkMRMLNode* node);
  virtual void OnMRMLSceneNodeRemoved(vtkMRMLNode* node);
  virtual void ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void* callData);

private:

  vtkSlicerVirtualRealityLogic(const vtkSlicerVirtualRealityLogic&); // Not implemented
  void operator=(const vtkSlicerVirtualRealityLogic&); // Not implemented
};

#endif
