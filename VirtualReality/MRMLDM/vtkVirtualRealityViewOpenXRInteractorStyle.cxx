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

  oiren->AddAction("left_grip_pose", static_cast<vtkCommand::EventIds>(LeftGripPoseEvent));
  oiren->AddAction("right_grip_pose", static_cast<vtkCommand::EventIds>(RightGripPoseEvent));
  oiren->AddAction("left_aim_pose", static_cast<vtkCommand::EventIds>(LeftAimPoseEvent));
  oiren->AddAction("right_aim_pose", static_cast<vtkCommand::EventIds>(RightAimPoseEvent));

  oiren->AddAction("left_grip_value", static_cast<vtkCommand::EventIds>(LeftGripValueEvent));
  oiren->AddAction("right_grip_value", static_cast<vtkCommand::EventIds>(RightGripValueEvent));
  oiren->AddAction("left_trigger_value", static_cast<vtkCommand::EventIds>(LeftTriggerValueEvent));
  oiren->AddAction("right_trigger_value", static_cast<vtkCommand::EventIds>(RightTriggerValueEvent));
  oiren->AddAction("left_trigger_touch", static_cast<vtkCommand::EventIds>(LeftTriggerTouchEvent));
  oiren->AddAction("right_trigger_touch", static_cast<vtkCommand::EventIds>(RightTriggerTouchEvent));

  oiren->AddAction("left_thumbstick", static_cast<vtkCommand::EventIds>(LeftThumbstickEvent));
  oiren->AddAction("left_thumbstick_click", static_cast<vtkCommand::EventIds>(LeftThumbstickClickEvent));
  oiren->AddAction("right_thumbstick_click", static_cast<vtkCommand::EventIds>(RightThumbstickClickEvent));
  oiren->AddAction("left_thumbstick_touch", static_cast<vtkCommand::EventIds>(LeftThumbstickTouchEvent));

  oiren->AddAction("left_thumbrest_touch", static_cast<vtkCommand::EventIds>(LeftThumbrestTouchEvent));
  oiren->AddAction("right_thumbrest_touch", static_cast<vtkCommand::EventIds>(RightThumbrestTouchEvent));

  oiren->AddAction("left_button1_click", static_cast<vtkCommand::EventIds>(LeftButton1ClickEvent));
  oiren->AddAction("left_button1_touch", static_cast<vtkCommand::EventIds>(LeftButton1TouchEvent));
  oiren->AddAction("left_button2_click", static_cast<vtkCommand::EventIds>(LeftButton2ClickEvent));
  oiren->AddAction("left_button2_touch", static_cast<vtkCommand::EventIds>(LeftButton2TouchEvent));

  oiren->AddAction("right_button1_touch", static_cast<vtkCommand::EventIds>(RightButton1TouchEvent));
  oiren->AddAction("right_button2_touch", static_cast<vtkCommand::EventIds>(RightButton2TouchEvent));
  oiren->AddAction("right_system_click", static_cast<vtkCommand::EventIds>(RightSystemClickEvent));

  // The following 5 actions are bound directly to legacy VTK 3D events,
  // preserving current end-user behavior exactly (matches VTK's stock Oculus
  // Touch binding: right A button -> select, right B button -> menu, left
  // menu button -> next camera pose, right thumbstick -> movement). Customize
  // via vtkSlicerVirtualRealityLogic::AddAction() from Python, e.g. to rebind
  // movement to the left thumbstick instead (see DeveloperGuide.md).
  oiren->AddAction("right_button1_click", vtkCommand::Select3DEvent);
  oiren->AddAction("right_button2_click", vtkCommand::Menu3DEvent);
  oiren->AddAction("left_menu_click", vtkCommand::NextPose3DEvent);
  oiren->AddAction("right_thumbstick", vtkCommand::ViewerMovement3DEvent);
  oiren->AddAction("right_thumbstick_touch", vtkCommand::ViewerMovement3DEvent);
}
