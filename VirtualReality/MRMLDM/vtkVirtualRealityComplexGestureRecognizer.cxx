/*==============================================================================

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was supported through the University of Western Ontario.

==============================================================================*/

#include "vtkVirtualRealityComplexGestureRecognizer.h"

// VTK/Rendering/VR includes
#include <vtkVRRenderWindow.h>
#include <vtkVRRenderWindowInteractor.h>

// VTK includes
#include <vtkEventData.h>
#include <vtkInteractorStyle.h>
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkVirtualRealityComplexGestureRecognizer);

//----------------------------------------------------------------------------
void vtkVirtualRealityComplexGestureRecognizer::SetInteractor(vtkVRRenderWindowInteractor* interactor)
{
  vtkSetSmartPointerBodyMacro(Interactor, vtkVRRenderWindowInteractor, interactor);
}

//----------------------------------------------------------------------------
vtkVRRenderWindowInteractor* vtkVirtualRealityComplexGestureRecognizer::GetInteractor() const
{
  return this->Interactor;
}

//------------------------------------------------------------------------------
void vtkVirtualRealityComplexGestureRecognizer::HandleComplexGestureEvents(vtkEventData* ed)
{
  //
  // SlicerVirtualReality implementation details:
  //
  // * The implementation of both HandleComplexGestureEvents() and RecognizeComplexGesture()
  //   was updated by removing the handling specific to Rotate and Pan to only keep
  //   he pinch event.
  //
  // * The update of the Scale was removed.
  //
  // * Update of "StartingPhysicalEventPoses" instead of "StartingPhysicalEventPositions":
  //   As originally described in SlicerVirtualReality@b6815f1cb, while the controllers
  //   move, the purpose is to have the PhysicalToWorld matrix modified so that the controller
  //   physical pose corresponds to the same rendering world pose.
  //   For reference, the "StartingPhysicalEventPoses" ivar was originally introduced in upstream
  //   VTK as VTK@803d3a327.
  //

  vtkVRRenderWindowInteractor* rwi = this->Interactor;
  if (rwi == nullptr)
  {
    vtkErrorMacro("HandleComplexGestureEvents failed: Interactor is not set");
    return;
  }

  vtkEventDataDevice3D* edata = ed->GetAsEventDataDevice3D();
  if (edata == nullptr)
  {
    return;
  }

  rwi->SetPointerIndex(static_cast<int>(edata->GetDevice()));
  int pointerIndex = rwi->GetPointerIndex();

  if (edata->GetAction() == vtkEventDataAction::Press)
  {
    rwi->SetDeviceInputDownCount(edata->GetDevice(), 1);

    // StartingPhysicalEventPose
    vtkNew<vtkMatrix4x4> physicalEventPose;
    rwi->GetPhysicalEventPose(physicalEventPose, pointerIndex);
    rwi->SetStartingPhysicalEventPose(physicalEventPose, edata->GetDevice());

    // StartingPhysicalToWorldMatrix
    vtkVRRenderWindow* renWin = vtkVRRenderWindow::SafeDownCast(rwi->GetRenderWindow());
    vtkNew<vtkMatrix4x4> physicalToWorldMatrix;
    renWin->GetPhysicalToWorldMatrix(physicalToWorldMatrix);
    rwi->SetStartingPhysicalToWorldMatrix(physicalToWorldMatrix);

    // Both controllers have the grip down, start multitouch
    if (rwi->GetDeviceInputDownCount(vtkEventDataDevice::LeftController) &&
      rwi->GetDeviceInputDownCount(vtkEventDataDevice::RightController))
    {
      // The gesture is still unknown
      rwi->SetCurrentGesture(vtkCommand::StartEvent);
    }
  }

  // End the gesture if needed
  if (edata->GetAction() == vtkEventDataAction::Release)
  {
    rwi->SetDeviceInputDownCount(edata->GetDevice(), 0);

    if (rwi->GetCurrentGesture() == vtkCommand::PinchEvent)
    {
      rwi->EndPinchEvent();
      vtkInteractorStyle* interactorStyle = vtkInteractorStyle::SafeDownCast(rwi->GetInteractorStyle());
      if (interactorStyle)
        {
        interactorStyle->EndGesture();
        }
    }
    rwi->SetCurrentGesture(vtkCommand::NoEvent);

    return;
  }
}

//----------------------------------------------------------------------------
void vtkVirtualRealityComplexGestureRecognizer::RecognizeComplexGesture(vtkEventDataDevice3D* vtkNotUsed(edata))
{
  vtkVRRenderWindowInteractor* rwi = this->Interactor;
  if (rwi == nullptr)
  {
    vtkErrorMacro("RecognizeComplexGesture failed: Interactor is not set");
    return;
  }

  // Recognize gesture only if one button is pressed per controller
  vtkEventDataDevice lhand = vtkEventDataDevice::LeftController;
  vtkEventDataDevice rhand = vtkEventDataDevice::RightController;

  if (rwi->GetDeviceInputDownCount(lhand) > 1 || rwi->GetDeviceInputDownCount(lhand) == 0 ||
    rwi->GetDeviceInputDownCount(rhand) > 1 || rwi->GetDeviceInputDownCount(rhand) == 0)
  {
    rwi->SetCurrentGesture(vtkCommand::NoEvent);
    return;
  }

  if (rwi->GetCurrentGesture() != vtkCommand::NoEvent)
  {
    if (rwi->GetCurrentGesture() == vtkCommand::StartEvent)
    {
      rwi->SetCurrentGesture(vtkCommand::PinchEvent);

      rwi->StartPinchEvent();
      vtkInteractorStyle* interactorStyle = vtkInteractorStyle::SafeDownCast(rwi->GetInteractorStyle());
      if (interactorStyle)
        {
        interactorStyle->StartGesture();
        }
    }

    // If we have found a specific type of movement then handle it
    if (rwi->GetCurrentGesture() == vtkCommand::PinchEvent)
    {
      rwi->PinchEvent();
    }
  }
}
