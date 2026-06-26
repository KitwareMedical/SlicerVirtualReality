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

#ifndef vtkVirtualRealityViewOpenXRInteractorStyle_h
#define vtkVirtualRealityViewOpenXRInteractorStyle_h

// VR MRMLDM includes
#include "vtkSlicerVirtualRealityModuleMRMLDisplayableManagerExport.h"
#include "vtkVirtualRealityViewInteractorStyleDelegate.h"

// VTK Rendering/OpenXR includes
#include <vtkOpenXRInteractorStyle.h>

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkObject.h>
#include <vtkCommand.h>
#include <vtkEventData.h>
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>

class vtkMRMLScene;
class vtkMRMLDisplayableManagerGroup;
class vtkWorldPointPicker;


class VTK_SLICER_VIRTUALREALITY_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT vtkVirtualRealityViewOpenXRInteractorStyle
  : public vtkOpenXRInteractorStyle
{
public:
  static vtkVirtualRealityViewOpenXRInteractorStyle *New();
  vtkTypeMacro(vtkVirtualRealityViewOpenXRInteractorStyle,vtkOpenXRInteractorStyle);

  /// Generic, per-physical-control event IDs. One event ID per literal action
  /// output name declared in Resources/Bindings/vtk_openxr_actions.json and
  /// vtk_openxr_binding_oculus_touch_controller.json.
  ///
  /// All of these are dispatched directly by AddAction() in SetupActions()
  /// below and are independently observable by any code that observes the
  /// interactor. A curated subset (RightButton1ClickEvent,
  /// RightButton2ClickEvent, LeftMenuClickEvent, RightThumbstickEvent,
  /// RightThumbstickTouchEvent) is additionally translated by
  /// ProcessControllerEvents() into a legacy VTK 3D event (Select3DEvent,
  /// Menu3DEvent, NextPose3DEvent, ViewerMovement3DEvent) invoked on the
  /// interactor, to preserve existing end-user behavior.
  ///
  /// To customize which VTK event a given action invokes (e.g. remap a button
  /// to a different event, or move movement from the right to the left
  /// thumbstick), call vtkSlicerVirtualRealityLogic::AddAction() from Python
  /// with the action's name (e.g. "right_button1_click") and the desired
  /// vtkCommand event; see the "Low-level interception of events" section of
  /// DeveloperGuide.md.
  ///
  /// \warning LeftGripValueEvent, RightGripValueEvent, LeftTriggerValueEvent and
  /// RightTriggerValueEvent correspond to OpenXR "float" actions. As of this
  /// writing, vtkOpenXRRenderWindowInteractor::HandleAction() does not implement
  /// the XR_ACTION_TYPE_FLOAT_INPUT case, so these four events are registered
  /// and bound correctly but will never actually be invoked until VTK adds
  /// float-action dispatch support.
  enum ControllerEvents
  {
    FIRST_CONTROLLER_EVENT = vtkCommand::UserEvent + 1000,
    LeftGripPoseEvent = FIRST_CONTROLLER_EVENT,
    RightGripPoseEvent,
    LeftAimPoseEvent,
    RightAimPoseEvent,

    LeftGripValueEvent,
    RightGripValueEvent,
    LeftTriggerValueEvent,
    RightTriggerValueEvent,
    LeftTriggerTouchEvent,
    RightTriggerTouchEvent,

    LeftThumbstickEvent,
    RightThumbstickEvent,
    LeftThumbstickClickEvent,
    RightThumbstickClickEvent,
    LeftThumbstickTouchEvent,
    RightThumbstickTouchEvent,

    LeftThumbrestTouchEvent,
    RightThumbrestTouchEvent,

    LeftButton1ClickEvent,
    LeftButton1TouchEvent,
    LeftButton2ClickEvent,
    LeftButton2TouchEvent,
    LeftMenuClickEvent,

    RightButton1ClickEvent,
    RightButton1TouchEvent,
    RightButton2ClickEvent,
    RightButton2TouchEvent,
    RightSystemClickEvent,

    LAST_CONTROLLER_EVENT
  };

  /// Register the 28 generic per-control Oculus Touch actions with the interactor.
  /// Overrides vtkOpenXRInteractorStyle::SetupActions(), which otherwise registers
  /// the legacy curated action set (elevation, movement, nextcamerapose,
  /// positionprop, showmenu, startelevation, startmovement, triggeraction) that
  /// is no longer declared in this module's own action manifest.
  void SetupActions(vtkRenderWindowInteractor* iren) override;

  ///@{
  /// Set/get delegate
  void SetInteractorStyleDelegate(vtkVirtualRealityViewInteractorStyleDelegate* delegate)
  {
    vtkSetSmartPointerBodyMacro(InteractorStyleDelegate, vtkVirtualRealityViewInteractorStyleDelegate, delegate);
    if (delegate != nullptr)
      {
      delegate->SetInteractorStyle(this);
      }
  }
  vtkGetSmartPointerMacro(InteractorStyleDelegate, vtkVirtualRealityViewInteractorStyleDelegate);
  ///}@

  //@{
  /**
  * Interaction mode entry points.
  */
  void StartPositionProp(vtkEventDataDevice3D * edata) override { this->InteractorStyleDelegate->StartPositionProp(edata); }
  void EndPositionProp(vtkEventDataDevice3D * edata) override { this->InteractorStyleDelegate->EndPositionProp(edata); }
  //@}

  //@{
  /**
  * Multitouch events binding.
  */
  void StartGesture() override { this->InteractorStyleDelegate->StartGesture(); }
  void EndGesture() override { this->InteractorStyleDelegate->EndGesture(); }
  void OnPan() override { this->InteractorStyleDelegate->OnPan(); }
  void OnPinch() override { this->InteractorStyleDelegate->OnPinch(); }
  void OnRotate() override { this->InteractorStyleDelegate->OnRotate(); }
  //@}

  //@{
  /**
  * Methods for interaction.
  */
  void PositionProp(vtkEventData* ed, double* lwpos = nullptr, double* lwori = nullptr) override
  {
    this->InteractorStyleDelegate->PositionProp(ed, lwpos, lwori);
  }
  //@}

protected:
  vtkVirtualRealityViewOpenXRInteractorStyle();
  ~vtkVirtualRealityViewOpenXRInteractorStyle() override = default;

  /// Callback invoked for every ControllerEvents enum value observed on the interactor; see SetupActions().
  static void ProcessControllerEvents(
    vtkObject* object, unsigned long event, void* clientData, void* callData);

  vtkSmartPointer<vtkVirtualRealityViewInteractorStyleDelegate> InteractorStyleDelegate;
  vtkSmartPointer<vtkCallbackCommand> ControllerEventCallbackCommand;

private:
  vtkVirtualRealityViewOpenXRInteractorStyle(const vtkVirtualRealityViewOpenXRInteractorStyle&) = delete;
  void operator=(const vtkVirtualRealityViewOpenXRInteractorStyle&) = delete;
};

#endif
