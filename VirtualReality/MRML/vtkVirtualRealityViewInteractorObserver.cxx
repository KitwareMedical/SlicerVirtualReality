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

#include "vtkVirtualRealityViewInteractorObserver.h"

// SlicerVirtualReality includes
#include "vtkVirtualRealityViewInteractor.h"
#include "vtkVirtualRealityViewInteractorStyle.h"

// MRML includes
#include "vtkMRMLInteractionEventData.h"

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkEventData.h>
#include <vtkInteractorStyle.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkObjectFactory.h>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkVirtualRealityViewInteractorObserver);

//---------------------------------------------------------------------------
// vtkVirtualRealityViewInteractorObserver methods

//----------------------------------------------------------------------------
vtkVirtualRealityViewInteractorObserver::vtkVirtualRealityViewInteractorObserver()
{
  this->EventCallbackCommand->SetCallback(vtkVirtualRealityViewInteractorObserver::CustomProcessEvents);
}

//----------------------------------------------------------------------------
vtkVirtualRealityViewInteractorObserver::~vtkVirtualRealityViewInteractorObserver()
{
}

//----------------------------------------------------------------------------
void vtkVirtualRealityViewInteractorObserver::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkVirtualRealityViewInteractorObserver::SetInteractor(vtkRenderWindowInteractor *interactor)
{
  if (interactor == this->Interactor)
    {
    return;
    }

  this->Superclass::SetInteractor(interactor);

  if (interactor)
    {
    float priority = 0.0f;

    /// 3D event bindings
    // Move3DEvent: Already observed in vtkMRMLViewInteractorStyle
    // Button3DEvent: Already observed in vtkMRMLViewInteractorStyle
    interactor->AddObserver(vtkCommand::Menu3DEvent, this->EventCallbackCommand, priority);
    interactor->AddObserver(vtkCommand::Select3DEvent, this->EventCallbackCommand, priority);
    interactor->AddObserver(vtkCommand::NextPose3DEvent, this->EventCallbackCommand, priority);
    interactor->AddObserver(vtkCommand::ViewerMovement3DEvent, this->EventCallbackCommand, priority);
    interactor->AddObserver(vtkCommand::Pick3DEvent, this->EventCallbackCommand, priority);
    interactor->AddObserver(vtkCommand::PositionProp3DEvent, this->EventCallbackCommand, priority);
    interactor->AddObserver(vtkCommand::Clip3DEvent, this->EventCallbackCommand, priority);
    interactor->AddObserver(vtkCommand::Elevation3DEvent, this->EventCallbackCommand, priority);

    /// Touch gesture interaction events
    // Already observed in vtkMRMLViewInteractorStyle
    }
}

//----------------------------------------------------------------------------
void vtkVirtualRealityViewInteractorObserver::CustomProcessEvents(vtkObject* object,
  unsigned long event, void* clientdata, void* calldata)
{
  vtkVirtualRealityViewInteractorObserver* self
    = reinterpret_cast<vtkVirtualRealityViewInteractorObserver *>(clientdata);

  if (!self->DelegateInteractionEventToDisplayableManagers(event, calldata))
    {
    // Displayable managers did not process it
    vtkVirtualRealityViewInteractorObserver::ProcessEvents(object, event, clientdata, calldata);
    }
}

//----------------------------------------------------------------------------
void vtkVirtualRealityViewInteractorObserver::ProcessEvents(
  vtkObject* object, unsigned long event, void* clientdata, void* calldata)
{
  vtkVirtualRealityViewInteractorObserver* self =
    reinterpret_cast<vtkVirtualRealityViewInteractorObserver*>(clientdata);

  switch (event)
    {

    case vtkCommand::StartPinchEvent:
    case vtkCommand::StartRotateEvent:
    case vtkCommand::StartPanEvent:
      self->OnStartGesture();
      break;
    case vtkCommand::EndPinchEvent:
    case vtkCommand::EndRotateEvent:
    case vtkCommand::EndPanEvent:
      self->OnEndGesture();
      break;

    /// 3D event bindings
    // Move3DEvent: Already observed in vtkMRMLViewInteractorStyle
    // Button3DEvent: Already observed in vtkMRMLViewInteractorStyle
    case vtkCommand::Menu3DEvent:
      self->OnMenu3D(static_cast<vtkEventData*>(calldata));
      break;
    case vtkCommand::Select3DEvent:
      self->OnSelect3D(static_cast<vtkEventData*>(calldata));
      break;
    case vtkCommand::NextPose3DEvent:
      self->OnNextPose3D(static_cast<vtkEventData*>(calldata));
      break;
    case vtkCommand::ViewerMovement3DEvent:
      self->OnViewerMovement3D(static_cast<vtkEventData*>(calldata));
      break;
    case vtkCommand::Pick3DEvent:
      self->OnPick3D(static_cast<vtkEventData*>(calldata));
      break;
    case vtkCommand::PositionProp3DEvent:
      self->OnPositionProp3D(static_cast<vtkEventData*>(calldata));
      break;
    case vtkCommand::Clip3DEvent:
      self->OnClip3D(static_cast<vtkEventData*>(calldata));
      break;
    case vtkCommand::Elevation3DEvent:
      self->OnElevation3D(static_cast<vtkEventData*>(calldata));
      break;

    default:
      Superclass::ProcessEvents(object, event, clientdata, calldata);
      break;
    }
}


//----------------------------------------------------------------------------
// Generic events binding
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
bool vtkVirtualRealityViewInteractorObserver::DelegateInteractionEventToDisplayableManagers(unsigned long event, void* calldata)
{
  vtkSmartPointer<vtkEventData> ed;
  if (vtkCommand::EventHasData(event))
    {
    ed = vtkSmartPointer(static_cast<vtkEventData*>(calldata));
    }
  else
    {
    ed = vtkSmartPointer<vtkMRMLInteractionEventData>::New();
    ed->SetType(event);
    }

  bool delegated = this->DelegateInteractionEventToDisplayableManagers(ed);
  if (delegated)
    {
    this->EventCallbackCommand->SetAbortFlag(1);
    }
  return delegated;
}

//----------------------------------------------------------------------------
bool vtkVirtualRealityViewInteractorObserver::DelegateInteractionEventToDisplayableManagers(vtkEventData* inputEventData)
{
  // Get display and world position
  if (!inputEventData)
    {
    return false;
    }
  int* displayPositionInt = this->GetInteractor()->GetEventPosition();
  vtkRenderer* pokedRenderer = this->GetInteractor()->FindPokedRenderer(displayPositionInt[0], displayPositionInt[1]);
  if (!pokedRenderer)
    {
    // can happen during application shutdown
    return false;
    }

  vtkEventDataDevice3D* inputEventDataDevice3D = inputEventData->GetAsEventDataDevice3D();
  if (!inputEventDataDevice3D)
    {
    vtkErrorMacro("DelegateInteractionEventToDisplayableManagers: Invalid event data type");
    return false;
    }

  return this->Superclass::DelegateInteractionEventToDisplayableManagers(inputEventData);
}

//----------------------------------------------------------------------------
bool vtkVirtualRealityViewInteractorObserver::DelegateInteractionEventDataToDisplayableManagers(vtkMRMLInteractionEventData* ed)
{

  vtkVirtualRealityViewInteractorStyle* vrViewInteractorStyle =
      vtkVirtualRealityViewInteractorStyle::SafeDownCast(this->GetInteractorStyle());
  if (vrViewInteractorStyle)
    {
    ed->SetWorldToPhysicalScale(vrViewInteractorStyle->GetMagnification());
    ed->SetAccuratePicker(vrViewInteractorStyle->GetAccuratePicker());
    }

  vtkRenderer* currentRenderer = this->GetInteractorStyle()->GetCurrentRenderer();
  ed->SetRenderer(currentRenderer);

  vtkVirtualRealityViewInteractor* vrViewInteractor =
      vtkVirtualRealityViewInteractor::SafeDownCast(this->GetInteractor());

  std::string interactionContextName;
  if (ed->GetDevice() == vtkEventDataDevice::LeftController)
    {
    interactionContextName = "LeftController"; //TODO: Store these elsewhere
    }
  else if (ed->GetDevice() == vtkEventDataDevice::RightController)
    {
    interactionContextName = "RightController"; //TODO: Store these elsewhere
    }
  else if (ed->GetDevice() == vtkEventDataDevice::HeadMountedDisplay)
    {
      interactionContextName = "HeadMountedDisplay";
    }
  else if (vrViewInteractor && vrViewInteractor->GetCurrentGesture() == vtkCommand::NoEvent)
    {
    // Report an error message only if the interactor is not processing a complex gesture.
    vtkErrorMacro("DelegateInteractionEventDataToDisplayableManagers: Unrecognized device");
    }
  ed->SetInteractionContextName(interactionContextName);

  return this->Superclass::DelegateInteractionEventDataToDisplayableManagers(ed);
}

//----------------------------------------------------------------------------
void vtkVirtualRealityViewInteractorObserver::OnStartGesture()
{
  vtkVirtualRealityViewInteractorStyle* vrInteractorStyle =
      vtkVirtualRealityViewInteractorStyle::SafeDownCast(this->GetInteractorStyle());
  vrInteractorStyle->OnStartGesture();
}

//----------------------------------------------------------------------------
void vtkVirtualRealityViewInteractorObserver::OnEndGesture()
{
  vtkVirtualRealityViewInteractorStyle* vrInteractorStyle =
      vtkVirtualRealityViewInteractorStyle::SafeDownCast(this->GetInteractorStyle());
  vrInteractorStyle->OnEndGesture();
}

//----------------------------------------------------------------------------
void vtkVirtualRealityViewInteractorObserver::OnMenu3D(vtkEventData *edata)
{
  this->GetInteractorStyle()->OnMenu3D(edata);
}

//----------------------------------------------------------------------------
void vtkVirtualRealityViewInteractorObserver::OnSelect3D(vtkEventData* edata)
{
  this->GetInteractorStyle()->OnSelect3D(edata);
}

//------------------------------------------------------------------------------
void vtkVirtualRealityViewInteractorObserver::OnNextPose3D(vtkEventData *edata)
{
  this->GetInteractorStyle()->OnNextPose3D(edata);
}

//------------------------------------------------------------------------------
void vtkVirtualRealityViewInteractorObserver::OnViewerMovement3D(vtkEventData* edata)
{
  this->GetInteractorStyle()->OnViewerMovement3D(edata);
}

//------------------------------------------------------------------------------
void vtkVirtualRealityViewInteractorObserver::OnPick3D(vtkEventData *edata)
{
  this->GetInteractorStyle()->OnPick3D(edata);
}

//------------------------------------------------------------------------------
void vtkVirtualRealityViewInteractorObserver::OnPositionProp3D(vtkEventData *edata)
{
  this->GetInteractorStyle()->OnPositionProp3D(edata);
}

//------------------------------------------------------------------------------
void vtkVirtualRealityViewInteractorObserver::OnClip3D(vtkEventData *edata)
{
  this->GetInteractorStyle()->OnClip3D(edata);
}

//------------------------------------------------------------------------------
void vtkVirtualRealityViewInteractorObserver::OnElevation3D(vtkEventData *edata)
{
  this->GetInteractorStyle()->OnElevation3D(edata);
}
