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

vtkStandardNewMacro(vtkVirtualRealityViewInteractor);

//----------------------------------------------------------------------------
vtkVirtualRealityViewInteractor::vtkVirtualRealityViewInteractor()
{
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
      int iInput = static_cast<int>(vtkEventDataDeviceInput::Grip);

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
      if (this->DeviceInputDown[iInput][0] && this->DeviceInputDown[iInput][1])
      {
        this->CurrentGesture = vtkCommand::PinchEvent;
        this->StartPinchEvent();
      }
    }

    // End the gesture if needed
    if (edata->GetAction() == vtkEventDataAction::Release)
    {
      if (edata->GetInput() == vtkEventDataDeviceInput::Grip)
      {
        if (this->CurrentGesture == vtkCommand::PinchEvent)
        {
          this->EndPinchEvent();
        }
        this->CurrentGesture = vtkCommand::NoEvent;
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
