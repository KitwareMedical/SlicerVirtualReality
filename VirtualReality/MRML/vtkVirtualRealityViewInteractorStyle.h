/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, and Cancer
  Care Ontario.

==============================================================================*/

#ifndef __vtkVirtualRealityViewInteractorStyle_h
#define __vtkVirtualRealityViewInteractorStyle_h

// MRML includes
#include "vtkMRMLViewInteractorStyle.h"

// VTK includes
#include <vtkCellPicker.h>
#include <vtkOpenVRInteractorStyle.h>
#include "vtkObject.h"
#include "vtkEventData.h"
#include <vtkSmartPointer.h>

#include "vtkSlicerVirtualRealityModuleMRMLExport.h"

class vtkMRMLScene;
class vtkMRMLDisplayableManagerGroup;
class vtkCellPicker;
class vtkWorldPointPicker;

/// \brief Virtual reality interactions
///
/// TODO:
///
class VTK_SLICER_VIRTUALREALITY_MODULE_MRML_EXPORT vtkVirtualRealityViewInteractorStyle :
    public vtkOpenVRInteractorStyle
{
public:
  static vtkVirtualRealityViewInteractorStyle *New();
  vtkTypeMacro(vtkVirtualRealityViewInteractorStyle,vtkOpenVRInteractorStyle);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Set the Interactor wrapper being controlled by this object. (Satisfy superclass API.)
  void SetInteractor(vtkRenderWindowInteractor *interactor) override;

  void SetDisplayableManagers(vtkMRMLDisplayableManagerGroup* displayableManagers);

  /// Get MRML scene from the displayable manager group (the first displayable manager's if any)
  vtkMRMLScene* GetMRMLScene();

  //@{
  /// Override generic event bindings to call the corresponding action.
  void OnSelect3D(vtkEventData *edata) override;
  void OnMove3D(vtkEventData *edata) override;
  void OnViewerMovement3D(vtkEventData* edata) override;
  //@}

  //@{
  /**
  * Interaction mode entry points.
  */
  virtual void StartPositionProp(vtkEventDataDevice3D *);
  virtual void EndPositionProp(vtkEventDataDevice3D *);
  //@}

  //@{
  /**
  * Multitouch events binding.
  */
  void OnPan() override;
  void OnPinch() override;
  void OnRotate() override;
  void OnPinch3D();
  void StartGesture() override;
  void EndGesture() override;
  //@}

  //@{
  /**
  * Methods for interaction.
  */
  void PositionProp(vtkEventData *, double* lwpos = nullptr, double* lwori = nullptr) override;
  //@}

  //@{
  /**
  * Map controller inputs to actions.
  * Actions are defined by a VTKIS_*STATE*, interaction entry points,
  * and the corresponding method for interaction.
  */
  int GetMappedAction(vtkCommand::EventIds eid);
  //@}
  vtkSetClampMacro(GrabEnabled, int, 0, 1);
  vtkGetMacro(GrabEnabled, int);
  vtkBooleanMacro(GrabEnabled, int);

  //// allow the user to add options to the menu
  //vtkOpenVRMenuWidget *GetMenu() {
  //  return this->Menu.Get(); }

  vtkCellPicker* GetAccuratePicker();
  
  /// Set physical to world magnification. Valid value range is [0.01, 100].
  /// Note: Conversion is physicalScale = 1000 / magnification
  void SetMagnification(double magnification);
  double GetMagnification();

protected:

  // Utility routines
  void StartAction(int VTKIS_STATE, vtkEventDataDevice3D *edata);
  void EndAction(int VTKIS_STATE, vtkEventDataDevice3D *edata);

  bool QuickPick(int x, int y, double pickPoint[3]);

protected:

  /// For jump to slice feature (when mouse is moved while shift key is pressed)
  vtkSmartPointer<vtkCellPicker> AccuratePicker;
  vtkSmartPointer<vtkWorldPointPicker> QuickPicker;

  int GrabEnabled;

protected:
  vtkVirtualRealityViewInteractorStyle();
  ~vtkVirtualRealityViewInteractorStyle() override;

  /**
   * Update the 3D movement according to the given interaction state.
   */
  void Movement3D(int interactionState, vtkEventData* edata);

  vtkWeakPointer<vtkMRMLDisplayableManagerGroup> DisplayableManagers;

private:
  vtkVirtualRealityViewInteractorStyle(const vtkVirtualRealityViewInteractorStyle&);  /// Not implemented.
  void operator=(const vtkVirtualRealityViewInteractorStyle&);  /// Not implemented.

  class vtkInternal;
  vtkInternal* Internal;
};

#endif
