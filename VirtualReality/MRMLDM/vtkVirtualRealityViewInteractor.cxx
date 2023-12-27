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

#include "vtkVirtualRealityViewInteractor.h"

// VTK includes
#include "vtkMatrix4x4.h"
#include "vtkObjectFactory.h"
#include "vtkVRRenderWindow.h"

// SlicerVR includes
#include "vtkVirtualRealityViewInteractorStyle.h"

vtkStandardNewMacro(vtkVirtualRealityViewInteractor);

//----------------------------------------------------------------------------
vtkVirtualRealityViewInteractor::vtkVirtualRealityViewInteractor()
{
}

//----------------------------------------------------------------------------
vtkVirtualRealityViewInteractor::~vtkVirtualRealityViewInteractor()
{
}

//------------------------------------------------------------------------------
vtkCommand::EventIds vtkVirtualRealityViewInteractor::GetCurrentGesture() const
{
  return this->CurrentGesture;
}

//------------------------------------------------------------------------------
void vtkVirtualRealityViewInteractor::HandleComplexGestureEvents(vtkEventData* ed)
{
  // [SlicerVirtualReality]
  //
  // SlicerVirtualReality implementation details:
  //
  // * The implementation of both HandleGripEvents() and RecognizeComplexGesture()
  //   was updated by removing the handling specific to Rotate and Pan to only keep
  //   he pinch event.
  //
  // * The update of the Scale was removed.
  //
  // * The lines either enclosed in `[SlicerVirtualReality]` or associated with a
  //   `// SlicerVirtualReality` comment are specific to this extension.
  //
  // [/SlicerVirtualReality]

  vtkEventDataDevice3D* edata = ed->GetAsEventDataDevice3D();
  if (!edata)
  {
    return;
  }

  this->PointerIndex = static_cast<int>(edata->GetDevice());
  if (edata->GetAction() == vtkEventDataAction::Press)
  {
    this->DeviceInputDownCount[this->PointerIndex] = 1;

    this->StartingPhysicalEventPositions[this->PointerIndex][0] =
      this->PhysicalEventPositions[this->PointerIndex][0];
    this->StartingPhysicalEventPositions[this->PointerIndex][1] =
      this->PhysicalEventPositions[this->PointerIndex][1];
    this->StartingPhysicalEventPositions[this->PointerIndex][2] =
      this->PhysicalEventPositions[this->PointerIndex][2];

    // [SlicerVirtualReality]
    // As originally described in SlicerVirtualReality@b6815f1cb, while the controllers
    // move, the purpose is to have the PhysicalToWorld matrix modified so that the controller
    // physical pose corresponds to the same rendering world pose.
    //
    // For reference, the "StartingPhysicalEventPoses" ivar was originally introduced in upstream
    // VTK as VTK@803d3a327.
    this->StartingPhysicalEventPoses[this->PointerIndex]->DeepCopy(
      this->PhysicalEventPoses[this->PointerIndex]);
    // [/SlicerVirtualReality]

    vtkVRRenderWindow* renWin = vtkVRRenderWindow::SafeDownCast(this->RenderWindow);
    renWin->GetPhysicalToWorldMatrix(this->StartingPhysicalToWorldMatrix);

    // Both controllers have the grip down, start multitouch
    if (this->DeviceInputDownCount[static_cast<int>(vtkEventDataDevice::LeftController)] &&
      this->DeviceInputDownCount[static_cast<int>(vtkEventDataDevice::RightController)])
    {
      // The gesture is still unknown
      this->CurrentGesture = vtkCommand::StartEvent;
    }
  }

  // End the gesture if needed
  if (edata->GetAction() == vtkEventDataAction::Release)
  {
    this->DeviceInputDownCount[this->PointerIndex] = 0;

    if (this->CurrentGesture == vtkCommand::PinchEvent)
    {
      this->EndPinchEvent();
      vtkInteractorStyle* interactorStyle = vtkInteractorStyle::SafeDownCast(this->InteractorStyle);
      if (interactorStyle)
        {
        interactorStyle->EndGesture();
        }
    }
    this->CurrentGesture = vtkCommand::NoEvent;

    return;
  }
}

//----------------------------------------------------------------------------
void vtkVirtualRealityViewInteractor::RecognizeComplexGesture(vtkEventDataDevice3D* vtkNotUsed(edata))
{
  // Recognize gesture only if one button is pressed per controller
  int lhand = static_cast<int>(vtkEventDataDevice::LeftController);
  int rhand = static_cast<int>(vtkEventDataDevice::RightController);

  if (this->DeviceInputDownCount[lhand] > 1 || this->DeviceInputDownCount[lhand] == 0 ||
    this->DeviceInputDownCount[rhand] > 1 || this->DeviceInputDownCount[rhand] == 0)
  {
    this->CurrentGesture = vtkCommand::NoEvent;
    return;
  }

  // [SlicerVirtualReality]
  // double* posVals[2];
  // double* startVals[2];
  // posVals[0] = this->PhysicalEventPositions[lhand];
  // posVals[1] = this->PhysicalEventPositions[rhand];

  // startVals[0] = this->StartingPhysicalEventPositions[lhand];
  // startVals[1] = this->StartingPhysicalEventPositions[rhand];
  // [/SlicerVirtualReality]

  if (this->CurrentGesture != vtkCommand::NoEvent)
  {

    // [SlicerVirtualReality]
    // Calculate the distances
    // double originalDistance = sqrt(vtkMath::Distance2BetweenPoints(startVals[0], startVals[1]));
    // double newDistance = sqrt(vtkMath::Distance2BetweenPoints(posVals[0], posVals[1]));
    // [/SlicerVirtualReality]

    if (this->CurrentGesture == vtkCommand::StartEvent)
    {
      this->CurrentGesture = vtkCommand::PinchEvent;
      // this->Scale = 1.0; // SlicerVirtualReality
      this->StartPinchEvent();
      vtkInteractorStyle* interactorStyle = vtkInteractorStyle::SafeDownCast(this->InteractorStyle);
      if (interactorStyle)
        {
        interactorStyle->StartGesture();
        }
    }

    // If we have found a specific type of movement then handle it
    if (this->CurrentGesture == vtkCommand::PinchEvent)
    {
      // this->SetScale(newDistance / originalDistance); // SlicerVirtualReality
      this->PinchEvent();
    }
  }
}

//----------------------------------------------------------------------------
void vtkVirtualRealityViewInteractor::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "StartedMessageLoop: " << this->StartedMessageLoop << endl;
}

//----------------------------------------------------------------------
void vtkVirtualRealityViewInteractor::SetInteractorStyle(vtkInteractorObserver* style)
{
  Superclass::SetInteractorStyle(style);

  // Set default trigger button function "grab objects and world"
  this->SetTriggerButtonFunction(vtkVirtualRealityViewInteractor::GetButtonFunctionIdForGrabObjectsAndWorld());
}

//---------------------------------------------------------------------------
void vtkVirtualRealityViewInteractor::SetTriggerButtonFunction(std::string functionId)
{
  vtkVirtualRealityViewInteractorStyle* vrInteractorStyle = vtkVirtualRealityViewInteractorStyle::SafeDownCast(this->InteractorStyle);
  if (!vrInteractorStyle)
  {
    vtkWarningMacro("SetTriggerButtonFunction: Current interactor style is not a VR interactor style");
    return;
  }

  // The "eventId to state" mapping (see call to `MapInputToAction` below) applies to right and left
  // controller buttons because they are bound to the same eventId:
  // - `vtk_openvr_binding_*.json` files define the "button to action" mapping
  // - `vtkOpenVRInteractorStyle()` contructor defines the "action to eventId" mapping

  if (functionId.empty())
  {
    vrInteractorStyle->MapInputToAction(vtkCommand::Select3DEvent, VTKIS_NONE);
  }
  else if (!functionId.compare(vtkVirtualRealityViewInteractor::GetButtonFunctionIdForGrabObjectsAndWorld()))
  {
    vrInteractorStyle->MapInputToAction(vtkCommand::Select3DEvent, VTKIS_POSITION_PROP);
  }
  else
  {
    vtkErrorMacro("SetTriggerButtonFunction: Unknown function identifier '" << functionId << "'");
  }
}

//----------------------------------------------------------------------------
void vtkVirtualRealityViewInteractor::SetGestureButtonToTrigger()
{
  vtkVirtualRealityViewInteractorStyle* vrInteractorStyle = vtkVirtualRealityViewInteractorStyle::SafeDownCast(this->InteractorStyle);
  if (!vrInteractorStyle)
  {
    vtkWarningMacro("SetGestureButtonToTrigger: Current interactor style is not a VR interactor style");
    return;
  }

  // The update "action to (eventId|function)" mapping (see call to `AddAction` below) applies to
  // right and left controller buttons because they are bound to the same eventId:
  // - "vtk_openvr_binding_*.json" defines the "button -> action" mapping
  // - vtkOpenVRInteractorStyle defines the "action -> eventId" mapping
  this->AddAction("/actions/vtk/in/TriggerAction", /*isAnalog=*/false,
                  [this](vtkEventData* ed) { this->HandleComplexGestureEvents(ed); });
  this->AddAction("/actions/vtk/in/ComplexGestureAction", vtkCommand::Select3DEvent, /*isAnalog=*/false);
}

//----------------------------------------------------------------------------
void vtkVirtualRealityViewInteractor::SetGestureButtonToGrip()
{
  vtkVirtualRealityViewInteractorStyle* vrInteractorStyle = vtkVirtualRealityViewInteractorStyle::SafeDownCast(this->InteractorStyle);
  if (!vrInteractorStyle)
  {
    vtkWarningMacro("SetGestureButtonToGrip: Current interactor style is not a VR interactor style");
    return;
  }

  // The update "action to (eventId|function)" mapping (see call to `AddAction` below) applies to
  // right and left controller buttons because they are bound to the same eventId:
  // - "vtk_openvr_binding_*.json" defines the "button -> action" mapping
  // - vtkOpenVRInteractorStyle defines the "action -> eventId" mapping
  this->AddAction("/actions/vtk/in/TriggerAction", vtkCommand::Select3DEvent, /*isAnalog=*/false);
  this->AddAction("/actions/vtk/in/ComplexGestureAction", /*isAnalog=*/false,
                  [this](vtkEventData* ed) { this->HandleComplexGestureEvents(ed); });
}

//----------------------------------------------------------------------------
void vtkVirtualRealityViewInteractor::SetGestureButtonToTriggerAndGrip()
{
  vtkVirtualRealityViewInteractorStyle* vrInteractorStyle = vtkVirtualRealityViewInteractorStyle::SafeDownCast(this->InteractorStyle);
  if (!vrInteractorStyle)
  {
    vtkWarningMacro("SetGestureButtonToTriggerAndGrip: Current interactor style is not a VR interactor style");
    return;
  }

  // The update "action to (eventId|function)" mapping (see call to `AddAction` below) applies to
  // right and left controller buttons because they are bound to the same eventId:
  // - "vtk_openvr_binding_*.json" defines the "button -> action" mapping
  // - vtkOpenVRInteractorStyle defines the "action -> eventId" mapping
  this->AddAction("/actions/vtk/in/TriggerAction", /*isAnalog=*/false,
                  [this](vtkEventData* ed) { this->HandleComplexGestureEvents(ed); });
  this->AddAction("/actions/vtk/in/ComplexGestureAction", /*isAnalog=*/false,
                  [this](vtkEventData* ed) { this->HandleComplexGestureEvents(ed); });
}

//----------------------------------------------------------------------------
void vtkVirtualRealityViewInteractor::SetGestureButtonToNone()
{
  vtkVirtualRealityViewInteractorStyle* vrInteractorStyle = vtkVirtualRealityViewInteractorStyle::SafeDownCast(this->InteractorStyle);
  if (!vrInteractorStyle)
  {
    vtkWarningMacro("SetGestureButtonToNone: Current interactor style is not a VR interactor style");
    return;
  }

  // The update "action to (eventId|function)" mapping (see call to `AddAction` below) applies to
  // right and left controller buttons because they are bound to the same eventId:
  // - "vtk_openvr_binding_*.json" defines the "button -> action" mapping
  // - vtkOpenVRInteractorStyle defines the "action -> eventId" mapping
  this->AddAction("/actions/vtk/in/TriggerAction", /*isAnalog=*/false,
                  [](vtkEventData* vtkNotUsed(ed)) { });
  this->AddAction("/actions/vtk/in/ComplexGestureAction", /*isAnalog=*/false,
                  [](vtkEventData* vtkNotUsed(ed)) { });
}
