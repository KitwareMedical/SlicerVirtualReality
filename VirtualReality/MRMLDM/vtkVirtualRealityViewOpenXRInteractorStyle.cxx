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

// VR MRMLDM includes
#include "vtkVirtualRealityViewOpenXRInteractorStyle.h"

// VTK Rendering/OpenXR includes
#include <vtkOpenXRRenderWindowInteractor.h>

// VTK includes
#include <vtkObjectFactory.h>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkVirtualRealityViewOpenXRInteractorStyle);

//----------------------------------------------------------------------------
vtkVirtualRealityViewOpenXRInteractorStyle::vtkVirtualRealityViewOpenXRInteractorStyle()
{
  this->ControllerEventCallbackCommand = vtkSmartPointer<vtkCallbackCommand>::New();
  this->ControllerEventCallbackCommand->SetClientData(this);
  this->ControllerEventCallbackCommand->SetCallback(
    vtkVirtualRealityViewOpenXRInteractorStyle::ProcessControllerEvents);
}

//----------------------------------------------------------------------------
void vtkVirtualRealityViewOpenXRInteractorStyle::SetupActions(vtkRenderWindowInteractor* iren)
{
  // Intentionally does not call Superclass::SetupActions(): the base class
  // registers the legacy curated action set (elevation, movement, showmenu,
  // triggeraction, ...), none of which are declared in this module's own
  // vtk_openxr_actions.json manifest.
  vtkOpenXRRenderWindowInteractor* oiren = vtkOpenXRRenderWindowInteractor::SafeDownCast(iren);
  if (!oiren)
  {
    return;
  }

  // Action -> event ID bindings, in the same order as the ControllerEvents enum (see the
  // header for details).
  oiren->AddAction("left_grip_pose", static_cast<vtkCommand::EventIds>(LeftGripPoseEvent));
  oiren->AddAction("right_grip_pose", static_cast<vtkCommand::EventIds>(RightGripPoseEvent));
  oiren->AddAction("left_aim_pose", static_cast<vtkCommand::EventIds>(LeftAimPoseEvent));
  oiren->AddAction("right_aim_pose", static_cast<vtkCommand::EventIds>(RightAimPoseEvent));

  oiren->AddAction("left_grip_value", static_cast<vtkCommand::EventIds>(LeftGripValueEvent));
  oiren->AddAction("right_grip_value", static_cast<vtkCommand::EventIds>(RightGripValueEvent));
  oiren->AddAction("left_grip_click", static_cast<vtkCommand::EventIds>(LeftGripClickEvent));
  oiren->AddAction("right_grip_click", static_cast<vtkCommand::EventIds>(RightGripClickEvent));
  oiren->AddAction("left_trigger_value", static_cast<vtkCommand::EventIds>(LeftTriggerValueEvent));
  oiren->AddAction("right_trigger_value", static_cast<vtkCommand::EventIds>(RightTriggerValueEvent));
  oiren->AddAction("left_trigger_click", static_cast<vtkCommand::EventIds>(LeftTriggerClickEvent));
  oiren->AddAction("right_trigger_click", static_cast<vtkCommand::EventIds>(RightTriggerClickEvent));
  oiren->AddAction("left_trigger_touch", static_cast<vtkCommand::EventIds>(LeftTriggerTouchEvent));
  oiren->AddAction("right_trigger_touch", static_cast<vtkCommand::EventIds>(RightTriggerTouchEvent));

  oiren->AddAction("left_thumbstick", static_cast<vtkCommand::EventIds>(LeftThumbstickEvent));
  oiren->AddAction("right_thumbstick", static_cast<vtkCommand::EventIds>(RightThumbstickEvent));
  oiren->AddAction("left_thumbstick_click", static_cast<vtkCommand::EventIds>(LeftThumbstickClickEvent));
  oiren->AddAction("right_thumbstick_click", static_cast<vtkCommand::EventIds>(RightThumbstickClickEvent));
  oiren->AddAction("left_thumbstick_touch", static_cast<vtkCommand::EventIds>(LeftThumbstickTouchEvent));
  oiren->AddAction("right_thumbstick_touch", static_cast<vtkCommand::EventIds>(RightThumbstickTouchEvent));

  oiren->AddAction("left_thumbrest_touch", static_cast<vtkCommand::EventIds>(LeftThumbrestTouchEvent));
  oiren->AddAction("right_thumbrest_touch", static_cast<vtkCommand::EventIds>(RightThumbrestTouchEvent));

  oiren->AddAction("left_button1_click", static_cast<vtkCommand::EventIds>(LeftButton1ClickEvent));
  oiren->AddAction("left_button1_touch", static_cast<vtkCommand::EventIds>(LeftButton1TouchEvent));
  oiren->AddAction("left_button2_click", static_cast<vtkCommand::EventIds>(LeftButton2ClickEvent));
  oiren->AddAction("left_button2_touch", static_cast<vtkCommand::EventIds>(LeftButton2TouchEvent));
  oiren->AddAction("left_menu_click", static_cast<vtkCommand::EventIds>(LeftMenuClickEvent));

  oiren->AddAction("right_button1_click", static_cast<vtkCommand::EventIds>(RightButton1ClickEvent));
  oiren->AddAction("right_button1_touch", static_cast<vtkCommand::EventIds>(RightButton1TouchEvent));
  oiren->AddAction("right_button2_click", static_cast<vtkCommand::EventIds>(RightButton2ClickEvent));
  oiren->AddAction("right_button2_touch", static_cast<vtkCommand::EventIds>(RightButton2TouchEvent));
  oiren->AddAction("right_system_click", static_cast<vtkCommand::EventIds>(RightSystemClickEvent));

  // Observe exactly the ControllerEvents that ProcessControllerEvents() translates into a
  // default VTK 3D event (annotated "also translated" above) -- keep this list in sync with
  // that function's switch.
  oiren->AddObserver(
    static_cast<unsigned long>(RightThumbstickEvent), this->ControllerEventCallbackCommand, this->Priority);
  oiren->AddObserver(
    static_cast<unsigned long>(RightThumbstickTouchEvent), this->ControllerEventCallbackCommand, this->Priority);
  oiren->AddObserver(
    static_cast<unsigned long>(LeftGripClickEvent), this->ControllerEventCallbackCommand, this->Priority);
  oiren->AddObserver(
    static_cast<unsigned long>(RightGripClickEvent), this->ControllerEventCallbackCommand, this->Priority);
}

//----------------------------------------------------------------------------
void vtkVirtualRealityViewOpenXRInteractorStyle::ProcessControllerEvents(
  vtkObject* vtkNotUsed(object), unsigned long event, void* clientData, void* callData)
{
  vtkVirtualRealityViewOpenXRInteractorStyle* self =
    static_cast<vtkVirtualRealityViewOpenXRInteractorStyle*>(clientData);

  // Invoke on the interactor (not directly on this style object): both this style's own
  // dispatch (vtkInteractorStyle::ProcessEvents, registered as an observer on the interactor,
  // which calls OnViewerMovement3D/OnPositionProp3D) and any other code observing these
  // default VTK 3D events on the interactor (e.g. vtkVirtualRealityViewInteractorObserver) only
  // react to events invoked on the interactor. RightButton1ClickEvent/RightButton2ClickEvent/
  // LeftMenuClickEvent are intentionally not translated into Select3DEvent/Menu3DEvent/
  // NextPose3DEvent here; see the ControllerEvents doc comment above.
  vtkRenderWindowInteractor* interactor = self->GetInteractor();
  switch (event)
  {
  case RightThumbstickEvent:
  case RightThumbstickTouchEvent:
    interactor->InvokeEvent(vtkCommand::ViewerMovement3DEvent, callData);
    break;
  case LeftGripClickEvent:
  case RightGripClickEvent:
    interactor->InvokeEvent(vtkCommand::PositionProp3DEvent, callData);
    break;
  default:
    break;
  }
}

//----------------------------------------------------------------------------
void vtkVirtualRealityViewOpenXRInteractorStyle::OnPositionProp3D(vtkEventData* edata)
{
  // Mirrors vtkVRInteractorStyle::OnSelect3D(), which is the only generic 3D event that VTK
  // wires into the StartAction()/EndAction() grab/move state machine. PositionProp3DEvent is
  // not handled by VTK itself, so this override is what allows it (and therefore
  // LeftGripClickEvent/RightGripClickEvent, translated into it by ProcessControllerEvents())
  // to also drive VTKIS_POSITION_PROP. Unlike OnSelect3D(), the state is hardcoded rather than
  // looked up via GetMappedAction(): that lookup only matters for VTK's built-in 3D menu
  // (vtkVRInteractorStyle::MenuCallback), which this module deliberately does not use (see
  // ControllerEvents doc comment), so PositionProp3DEvent is never remapped to anything other
  // than VTKIS_POSITION_PROP.
  vtkEventDataDevice3D* bd = edata->GetAsEventDataDevice3D();
  if (!bd)
  {
    return;
  }

  int x = this->Interactor->GetEventPosition()[0];
  int y = this->Interactor->GetEventPosition()[1];
  this->FindPokedRenderer(x, y);

  switch (bd->GetAction())
  {
  case vtkEventDataAction::Press:
  case vtkEventDataAction::Touch:
    this->StartAction(VTKIS_POSITION_PROP, bd);
    break;
  case vtkEventDataAction::Release:
  case vtkEventDataAction::Untouch:
    this->EndAction(VTKIS_POSITION_PROP, bd);
    break;
  default:
    break;
  }
}
