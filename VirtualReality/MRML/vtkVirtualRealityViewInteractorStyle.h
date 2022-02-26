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
#include "vtkMRMLDisplayableManagerGroup.h"
#include "vtkMRMLViewInteractorStyle.h"

// VTK includes
#include "vtkObject.h"
#include "vtkOpenVRRenderWindow.h" // for enums
#include "vtkEventData.h"

#include "vtkSlicerVirtualRealityModuleMRMLExport.h"

class vtkMRMLScene;
class vtkCellPicker;
class vtkWorldPointPicker;

/// \brief Virtual reality interactions
///
/// TODO:
///
class VTK_SLICER_VIRTUALREALITY_MODULE_MRML_EXPORT vtkVirtualRealityViewInteractorStyle :
    public vtkMRMLViewInteractorStyle
{
public:
  static vtkVirtualRealityViewInteractorStyle *New();
  vtkTypeMacro(vtkVirtualRealityViewInteractorStyle,vtkMRMLViewInteractorStyle);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  
  /// Give a chance to displayable managers to process the event.
  /// Return true if the event is processed.
  using vtkMRMLViewInteractorStyle::DelegateInteractionEventToDisplayableManagers;
  bool DelegateInteractionEventToDisplayableManagers(vtkEventData* inputEventData) override;

  /// Set the Interactor wrapper being controlled by this object. (Satisfy superclass API.)
  void SetInteractor(vtkRenderWindowInteractor *interactor) override;

  /// Main process event method
  static void ProcessEvents(vtkObject* object, unsigned long event, void* clientdata, void* calldata);

  /// Get MRML scene from the displayable manager group (the first displayable manager's if any)
  vtkMRMLScene* GetMRMLScene();

  //@{
  /// Override generic event bindings to call the corresponding action.
  void OnButton3D(vtkEventData *edata) override;
  void OnMove3D(vtkEventData *edata) override;
  //@}

  //@{
  /**
  * Interaction mode entry points.
  */
  //virtual void StartPick(vtkEventDataDevice3D *);
  //virtual void EndPick(vtkEventDataDevice3D *);
  //virtual void StartLoadCamPose(vtkEventDataDevice3D *);
  //virtual void EndLoadCamPose(vtkEventDataDevice3D *);
  virtual void StartPositionProp(vtkEventDataDevice3D *);
  virtual void EndPositionProp(vtkEventDataDevice3D *);
  //virtual void StartClip(vtkEventDataDevice3D *);
  //virtual void EndClip(vtkEventDataDevice3D *);
  virtual void StartDolly3D(vtkEventDataDevice3D *);
  virtual void EndDolly3D(vtkEventDataDevice3D *);
  //@}

  //@{
  /**
  * Multitouch events binding.
  */
  void OnPan() override;
  void OnPinch() override;
  void OnRotate() override;
  void OnPinch3D();
  void OnStartGesture();
  void OnEndGesture();
  //@}

  //@{
  /**
  * Methods for interaction.
  */
  //void ProbeData(vtkEventDataDevice controller);
  //void LoadNextCameraPose();
  virtual void PositionProp(vtkEventData *, double* lwpos = nullptr, double* lwori = nullptr);
  //virtual void Clip(vtkEventDataDevice3D *);
  //@}

  //@{
  /**
  * Map controller inputs to actions.
  * Actions are defined by a VTKIS_*STATE*, interaction entry points,
  * and the corresponding method for interaction.
  */
  void MapInputToAction(vtkEventDataDevice device,
    vtkEventDataDeviceInput input, int state);
  int GetMappedAction(vtkEventDataDevice device, vtkEventDataDeviceInput input);
  //@}

  ////@{
  ///**
  //* Define the helper text that goes with an input
  //*/
  //void AddTooltipForInput(vtkEventDataDevice device,
  //  vtkEventDataDeviceInput input, const std::string &text);
  ////@}

  //vtkSetClampMacro(HoverPick, int, 0, 1);
  //vtkGetMacro(HoverPick, int);
  //vtkBooleanMacro(HoverPick, int);

  vtkSetClampMacro(GrabEnabled, int, 0, 1);
  vtkGetMacro(GrabEnabled, int);
  vtkBooleanMacro(GrabEnabled, int);

  //int GetInteractionState(vtkEventDataDevice device) {
  //  return this->InteractionState[static_cast<int>(device)]; }

  //void ShowRay(vtkEventDataDevice controller);
  //void HideRay(vtkEventDataDevice controller);

  //void ShowBillboard(const std::string &text);
  //void HideBillboard();

  //void ShowPickSphere(double *pos, double radius, vtkProp3D *);
  //void ShowPickCell(vtkCell *cell, vtkProp3D *);
  //void HidePickActor();

  //void ToggleDrawControls();

  //// allow the user to add options to the menu
  //vtkOpenVRMenuWidget *GetMenu() {
  //  return this->Menu.Get(); }
  
  /// Set physical to world magnification. Valid value range is [0.01, 100].
  /// Note: Conversion is physicalScale = 1000 / magnification
  void SetMagnification(double magnification);
  double GetMagnification();

protected:
  //void EndPickCallback(vtkSelection *sel);

  ////Ray drawing
  //void UpdateRay(vtkEventDataDevice controller);

  //vtkNew<vtkOpenVRMenuWidget> Menu;
  //vtkNew<vtkOpenVRMenuRepresentation> MenuRepresentation;
  //vtkCallbackCommand* MenuCommand;
  //static void MenuCallback(vtkObject* object,
  //                  unsigned long event,
  //                  void* clientdata,
  //                  void* calldata);

  //vtkNew<vtkTextActor3D> TextActor3D;
  //vtkNew<vtkActor> PickActor;
  //vtkNew<vtkSphereSource> Sphere;

  // Device input to interaction state mapping
  int InputMap[vtkEventDataNumberOfDevices][vtkEventDataNumberOfInputs];
  //vtkOpenVRControlsHelper* ControlsHelpers[vtkEventDataNumberOfDevices][vtkEventDataNumberOfInputs];

  // Utility routines
  void StartAction(int VTKIS_STATE, vtkEventDataDevice3D *edata);
  void EndAction(int VTKIS_STATE, vtkEventDataDevice3D *edata);

  ///**
  //* Controls helpers drawing
  //*/
  //void AddTooltipForInput(vtkEventDataDevice device, vtkEventDataDeviceInput input);

  bool QuickPick(int x, int y, double pickPoint[3]);

protected:
  ///**
  //* Indicates if picking should be updated every frame. If so, the interaction
  //* picker will try to pick a prop and rays will be updated accordingly.
  //* Default is set to off.
  //*/
  //int HoverPick;

  //vtkNew<vtkOpenVRHardwarePicker> HardwarePicker;

  /// For jump to slice feature (when mouse is moved while shift key is pressed)
  vtkSmartPointer<vtkCellPicker> AccuratePicker;
  vtkSmartPointer<vtkWorldPointPicker> QuickPicker;

  int GrabEnabled;

protected:
  vtkVirtualRealityViewInteractorStyle();
  ~vtkVirtualRealityViewInteractorStyle() override;

private:
  vtkVirtualRealityViewInteractorStyle(const vtkVirtualRealityViewInteractorStyle&);  /// Not implemented.
  void operator=(const vtkVirtualRealityViewInteractorStyle&);  /// Not implemented.

  class vtkInternal;
  vtkInternal* Internal;
};

#endif
