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

// SlicerVR includes
#include "vtkVirtualRealityViewInteractorStyle.h"

vtkStandardNewMacro(vtkVirtualRealityViewInteractor);

//----------------------------------------------------------------------------
vtkVirtualRealityViewInteractor::vtkVirtualRealityViewInteractor()
{
  this->GestureEnabledButtons.push_back(static_cast<int>(vtkEventDataDeviceInput::Grip));
}

//----------------------------------------------------------------------------
vtkVirtualRealityViewInteractor::~vtkVirtualRealityViewInteractor()
{
}

//----------------------------------------------------------------------------
void vtkVirtualRealityViewInteractor::RecognizeComplexGesture(vtkEventDataDevice3D* edata)
{
  // Recognize gesture only if one button is pressed per controller
  if (this->DeviceInputDownCount[this->PointerIndex] > 2 ||
    this->DeviceInputDownCount[this->PointerIndex] == 0)
  {
    this->CurrentGesture = vtkCommand::NoEvent;
    return;
  }

  // Store the initial positions
  if (edata->GetType() == vtkCommand::Button3DEvent)
  {
    if (edata->GetAction() == vtkEventDataAction::Press)
    {
      this->StartingPhysicalEventPositions[this->PointerIndex][0] =
        this->PhysicalEventPositions[this->PointerIndex][0];
      this->StartingPhysicalEventPositions[this->PointerIndex][1] =
        this->PhysicalEventPositions[this->PointerIndex][1];
      this->StartingPhysicalEventPositions[this->PointerIndex][2] =
        this->PhysicalEventPositions[this->PointerIndex][2];

      this->StartingPhysicalEventPoses[this->PointerIndex]->DeepCopy(
        this->PhysicalEventPoses[this->PointerIndex] );

      vtkOpenVRRenderWindow* renWin =
        vtkOpenVRRenderWindow::SafeDownCast(this->RenderWindow);
      renWin->GetPhysicalToWorldMatrix(this->StartingPhysicalToWorldMatrix);

      // Both controllers have the grip down, start multitouch
      for (std::vector<int>::iterator buttonIt=this->GestureEnabledButtons.begin(); buttonIt!=this->GestureEnabledButtons.end(); ++buttonIt)
      {
        if (this->DeviceInputDown[*buttonIt][0] && this->DeviceInputDown[*buttonIt][1])
        {
          this->CurrentGesture = vtkCommand::PinchEvent;
          this->StartPinchEvent();
          break;
        }
      }
    }

    // End the gesture if needed
    if (edata->GetAction() == vtkEventDataAction::Release)
    {
      for (std::vector<int>::iterator buttonIt=this->GestureEnabledButtons.begin(); buttonIt!=this->GestureEnabledButtons.end(); ++buttonIt)
      {
        if (edata->GetInput() == static_cast<vtkEventDataDeviceInput>(*buttonIt))
        {
          if (this->CurrentGesture == vtkCommand::PinchEvent)
          {
            this->EndPinchEvent();
          }
          this->CurrentGesture = vtkCommand::NoEvent;
        }
      }
    }
  }

  if (edata->GetType() == vtkCommand::Move3DEvent && this->CurrentGesture != vtkCommand::NoEvent)
  {
    // Reduce computation
    if (!this->PointerIndex)
    {
      return;
    }

    if (this->CurrentGesture == vtkCommand::PinchEvent)
    {
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

  if (functionId.empty())
  {
    vrInteractorStyle->MapInputToAction(vtkEventDataDevice::RightController, vtkEventDataDeviceInput::Trigger, VTKIS_NONE);
    vrInteractorStyle->MapInputToAction(vtkEventDataDevice::LeftController,  vtkEventDataDeviceInput::Trigger, VTKIS_NONE);

    this->GestureEnabledButtons.clear();
    this->GestureEnabledButtons.push_back(static_cast<int>(vtkEventDataDeviceInput::Grip));
  }
  else if (!functionId.compare(vtkVirtualRealityViewInteractor::GetButtonFunctionIdForGrabObjectsAndWorld()))
  {
    vrInteractorStyle->MapInputToAction(vtkEventDataDevice::RightController, vtkEventDataDeviceInput::Trigger, VTKIS_POSITION_PROP);
    vrInteractorStyle->MapInputToAction(vtkEventDataDevice::LeftController,  vtkEventDataDeviceInput::Trigger, VTKIS_POSITION_PROP);

    this->GestureEnabledButtons.clear();
    this->GestureEnabledButtons.push_back(static_cast<int>(vtkEventDataDeviceInput::Grip));
    this->GestureEnabledButtons.push_back(static_cast<int>(vtkEventDataDeviceInput::Trigger));
  }
  else
  {
    vtkErrorMacro("SetTriggerButtonFunction: Unknown function identifier '" << functionId << "'");
  }
}
