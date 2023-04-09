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

// .NAME vtkSlicerGUIWidgetsLogic - slicer logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes


#ifndef __vtkSlicerGUIWidgetsLogic_h
#define __vtkSlicerGUIWidgetsLogic_h

// Slicer includes
#include <vtkSlicerMarkupsLogic.h>

// MRML includes

#include "vtkSlicerGUIWidgetsModuleLogicExport.h"

/// \ingroup Slicer_QtModules_ExtensionTemplate
class VTK_SLICER_GUIWIDGETS_MODULE_LOGIC_EXPORT vtkSlicerGUIWidgetsLogic :
  public vtkSlicerMarkupsLogic
{
public:

  static vtkSlicerGUIWidgetsLogic *New();
  vtkTypeMacro(vtkSlicerGUIWidgetsLogic, vtkSlicerMarkupsLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

protected:
  vtkSlicerGUIWidgetsLogic();
  virtual ~vtkSlicerGUIWidgetsLogic();

  /// Initialize listening to MRML events
  void SetMRMLSceneInternal(vtkMRMLScene* newScene) override;
  void ObserveMRMLScene() override;

  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  void RegisterNodes() override;
  void UpdateFromMRMLScene() override;
  void OnMRMLSceneNodeAdded(vtkMRMLNode* node) override;
  void OnMRMLSceneNodeRemoved(vtkMRMLNode* node) override;
private:

  vtkSlicerGUIWidgetsLogic(const vtkSlicerGUIWidgetsLogic&); // Not implemented
  void operator=(const vtkSlicerGUIWidgetsLogic&); // Not implemented
};

#endif
