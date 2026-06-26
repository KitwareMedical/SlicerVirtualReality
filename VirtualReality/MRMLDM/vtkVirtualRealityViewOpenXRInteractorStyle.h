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
  /// interactor. A curated subset (RightThumbstickEvent,
  /// RightThumbstickTouchEvent, LeftGripClickEvent, RightGripClickEvent) is
  /// additionally translated by ProcessControllerEvents() into a default VTK
  /// 3D event (ViewerMovement3DEvent, PositionProp3DEvent) invoked on the
  /// interactor, to preserve/add the corresponding end-user behavior.
  ///
  /// RightThumbstickEvent (continuous position) and RightThumbstickTouchEvent
  /// (touch down/lift off) are BOTH translated into ViewerMovement3DEvent,
  /// mirroring VTK's own stock Oculus Touch binding ("movement" and
  /// "startmovement" in VTK's vtk_openxr_actions.json), not an oversight:
  /// vtkOpenXRRenderWindowInteractor::HandleVector2fAction() never sets the
  /// event's Press/Release action, so vtkVRInteractorStyle::Movement3D()
  /// would otherwise only start/stop dolly movement based on a deflection
  /// threshold (fabs(pos[1]) crossing 0.1), which can lag the stick's
  /// physical spring-return. The touch event's Press/Release gives an
  /// immediate, deflection-independent start-on-touch / stop-on-release
  /// signal.
  ///
  /// Several other events are deliberately NOT translated into a default VTK
  /// 3D event, despite VTK's stock Oculus Touch binding doing so:
  /// - RightButton2ClickEvent -> Menu3DEvent would show VTK's built-in 3D
  ///   menu (vtkVRInteractorStyle::OnMenu3D()/vtkVRMenuWidget), which has no
  ///   reliable way to dismiss it if the controller ray misses a menu item,
  ///   leaving the user stuck.
  /// - LeftMenuClickEvent -> NextPose3DEvent would call
  ///   vtkVRInteractorStyle::OnNextPose3D()'s LoadNextCameraPose(), which is
  ///   a pure virtual implemented as an empty no-op by
  ///   vtkOpenXRInteractorStyle (it only does something for OpenVR).
  /// - RightButton1ClickEvent -> Select3DEvent would grab/move props (same
  ///   as LeftGripClickEvent/RightGripClickEvent do via PositionProp3DEvent
  ///   below), but is redundant now that grip-click covers that, so the
  ///   right A button is left raw.
  /// Any of these can still be wired up explicitly from Python, by observing the
  /// corresponding ControllerEvents value and forwarding it to the desired vtkCommand
  /// event via vtkSlicerVirtualRealityLogic::InvokeEvent().
  ///
  /// LeftGripClickEvent and RightGripClickEvent are translated into
  /// PositionProp3DEvent, which OnPositionProp3D() below drives into
  /// VTKIS_POSITION_PROP via StartAction()/EndAction(), the same way
  /// vtkVRInteractorStyle::OnSelect3D() does for Select3DEvent, so that
  /// squeezing either controller's grip grabs/moves props. The actual
  /// grabbing/moving logic is implemented in
  /// vtkVirtualRealityViewInteractorStyleDelegate, via this style's
  /// StartPositionProp()/EndPositionProp()/PositionProp() overrides below.
  ///
  /// To customize which VTK event a given control drives (e.g. move movement from the
  /// right to the left thumbstick), observe the corresponding ControllerEvents value from
  /// Python and forward it to the desired vtkCommand event via
  /// vtkSlicerVirtualRealityLogic::InvokeEvent(); see the "Low-level interception of
  /// events" section of DeveloperGuide.md.
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
    LeftGripClickEvent,
    RightGripClickEvent,
    LeftTriggerValueEvent,
    RightTriggerValueEvent,
    LeftTriggerClickEvent,
    RightTriggerClickEvent,
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

  /// Register the 32 generic per-control Oculus Touch actions with the interactor.
  /// Overrides vtkOpenXRInteractorStyle::SetupActions(), which otherwise registers
  /// the legacy curated action set (elevation, movement, nextcamerapose,
  /// positionprop, showmenu, startelevation, startmovement, triggeraction) that
  /// is no longer declared in this module's own action manifest.
  void SetupActions(vtkRenderWindowInteractor* iren) override;

  /// Map PositionProp3DEvent to VTKIS_POSITION_PROP (via StartAction()/EndAction()), the same
  /// way vtkVRInteractorStyle::OnSelect3D() does it for Select3DEvent. vtkInteractorStyle's
  /// own OnPositionProp3D() is an empty stub, but vtkVirtualRealityViewInteractorObserver
  /// forwards PositionProp3DEvent (received on the interactor) to this override, so it is the
  /// entry point that lets ProcessControllerEvents() route LeftGripClickEvent/
  /// RightGripClickEvent into a grab/move action.
  void OnPositionProp3D(vtkEventData* edata) override;

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

  /// Callback invoked for the ControllerEvents registered as observers in SetupActions().
  static void ProcessControllerEvents(
    vtkObject* object, unsigned long event, void* clientData, void* callData);

  vtkSmartPointer<vtkVirtualRealityViewInteractorStyleDelegate> InteractorStyleDelegate;
  vtkSmartPointer<vtkCallbackCommand> ControllerEventCallbackCommand;

private:
  vtkVirtualRealityViewOpenXRInteractorStyle(const vtkVirtualRealityViewOpenXRInteractorStyle&) = delete;
  void operator=(const vtkVirtualRealityViewOpenXRInteractorStyle&) = delete;
};

#endif
