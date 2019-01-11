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

#include "vtkVirtualRealityViewInteractorStyle.h"

// VR MRML includes
#include "vtkMRMLVirtualRealityViewNode.h"

// MRML includes
#include "vtkMRMLScene.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLVolumeRenderingDisplayNode.h"
#include "vtkMRMLSegmentationNode.h"
#include "vtkMRMLSegmentationDisplayNode.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLAbstractThreeDViewDisplayableManager.h"
#include "vtkMRMLModelDisplayableManager.h"

// VTK includes
#include <vtkCamera.h>
#include <vtkCellPicker.h>
#include <vtkCallbackCommand.h>
#include <vtkMath.h>
#include <vtkObjectFactory.h>
#include <vtkPoints.h>
#include <vtkRenderer.h>
#include <vtkQuaternion.h>
#include <vtkTimerLog.h>
#include <vtkTransform.h>
#include <vtkWeakPointer.h>

#include "vtkOpenVRRenderWindow.h"
#include "vtkOpenVRRenderWindowInteractor.h"


vtkStandardNewMacro(vtkVirtualRealityViewInteractorStyle);

//----------------------------------------------------------------------------
class vtkVirtualRealityViewInteractorStyle::vtkInternal
{
public:
  vtkInternal();
  ~vtkInternal();

  /// Calculate the average pose of the two controllers for pinch 3D operations
  /// \return Success flag. Failure happens when the average orientation coincides
  ///         with the direction of the displacement of the two controllers
  bool CalculateCombinedControllerPose(
    vtkMatrix4x4* controller0Pose, vtkMatrix4x4* controller1Pose, vtkMatrix4x4* combinedPose);

public:
  /// Store required controllers information when performing action
  int InteractionState[vtkEventDataNumberOfDevices];

  vtkWeakPointer<vtkMRMLDisplayableNode> PickedNode[vtkEventDataNumberOfDevices];

  //vtkPlane* ClippingPlanes[vtkEventDataNumberOfDevices];

  vtkNew<vtkMatrix4x4> CombinedStartingControllerPose;
  bool StartingControllerPoseValid;
  vtkNew<vtkMatrix4x4> LastValidCombinedControllerPose;
  bool LastValidCombinedControllerPoseExists;
};

//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkVirtualRealityViewInteractorStyle::vtkInternal::vtkInternal()
{
  for (int d = 0; d < vtkEventDataNumberOfDevices; ++d)
  {
    this->InteractionState[d] = VTKIS_NONE;
    this->PickedNode[d] = nullptr;
    //this->ClippingPlanes[d] = nullptr;
  }
}

//---------------------------------------------------------------------------
vtkVirtualRealityViewInteractorStyle::vtkInternal::~vtkInternal()
{
}

//---------------------------------------------------------------------------
bool vtkVirtualRealityViewInteractorStyle::vtkInternal::CalculateCombinedControllerPose(
  vtkMatrix4x4* controller0Pose, vtkMatrix4x4* controller1Pose, vtkMatrix4x4* combinedPose )
{
  if (!controller0Pose || !controller1Pose || !combinedPose)
  {
    return false;
  }

  // The position will be the average position
  double controllerCenterPos[3] = {
    (controller0Pose->GetElement(0,3) + controller1Pose->GetElement(0,3)) / 2.0,
    (controller0Pose->GetElement(1,3) + controller1Pose->GetElement(1,3)) / 2.0,
    (controller0Pose->GetElement(2,3) + controller1Pose->GetElement(2,3)) / 2.0 };

  // Scaling will be the distance between the two controllers
  double controllerDistance = sqrt(
    (controller0Pose->GetElement(0,3) - controller1Pose->GetElement(0,3))
      * (controller0Pose->GetElement(0,3) - controller1Pose->GetElement(0,3)) +
    (controller0Pose->GetElement(1,3) - controller1Pose->GetElement(1,3))
      * (controller0Pose->GetElement(1,3) - controller1Pose->GetElement(1,3)) +
    (controller0Pose->GetElement(2,3) - controller1Pose->GetElement(2,3))
      * (controller0Pose->GetElement(2,3) - controller1Pose->GetElement(2,3)) );

  // X axis is the displacement vector from controller 0 to 1
  double xAxis[3] = {
    controller1Pose->GetElement(0,3) - controller0Pose->GetElement(0,3),
    controller1Pose->GetElement(1,3) - controller0Pose->GetElement(1,3),
    controller1Pose->GetElement(2,3) - controller0Pose->GetElement(2,3) };
  vtkMath::Normalize(xAxis);

  // Y axis is calculated from a Y' and Z.
  // 1. Y' is the average orientation of the two controller directions
  // 2. If X and Y' are almost parallel, then return failure
  // 3. Z is the cross product of X and Y'
  // 4. Y is then the cross product of Z and X
  double yAxisPrime[3] = {
    controller0Pose->GetElement(0,1) + controller1Pose->GetElement(0,1),
    controller0Pose->GetElement(1,1) + controller1Pose->GetElement(1,1),
    controller0Pose->GetElement(2,1) + controller1Pose->GetElement(2,1) };
  vtkMath::Normalize(yAxisPrime);

  if (fabs(vtkMath::Dot(xAxis, yAxisPrime)) > 0.99)
  {
    // The two axes are almost parallel
    return false;
  }

  double zAxis[3] = {0.0};
  vtkMath::Cross(xAxis, yAxisPrime, zAxis);
  vtkMath::Normalize(zAxis);

  double yAxis[3] = {0.0};
  vtkMath::Cross(zAxis, xAxis, yAxis);
  vtkMath::Normalize(yAxis);

  // Assemble matrix from the axes, the scaling, and the position
  for (int row=0;row<3;++row)
  {
    combinedPose->SetElement(row, 0, xAxis[row]*controllerDistance);
    combinedPose->SetElement(row, 1, yAxis[row]*controllerDistance);
    combinedPose->SetElement(row, 2, zAxis[row]*controllerDistance);
    combinedPose->SetElement(row, 3, controllerCenterPos[row]);
  }

  return true;
}

//---------------------------------------------------------------------------
// vtkVirtualRealityViewInteractorStyle methods

//----------------------------------------------------------------------------
vtkVirtualRealityViewInteractorStyle::vtkVirtualRealityViewInteractorStyle()
  : DisplayableManagerGroup(nullptr)
{
  this->Internal = new vtkInternal();

  for (int d = 0; d < vtkEventDataNumberOfDevices; ++d)
  {
    this->Internal->InteractionState[d] = VTKIS_NONE;

    for (int i = 0; i < vtkEventDataNumberOfInputs; i++)
    {
      this->InputMap[d][i] = -1;
    }
  }

  // Create default inputs mapping
  this->MapInputToAction(vtkEventDataDevice::RightController,
    vtkEventDataDeviceInput::Grip, VTKIS_POSITION_PROP);
  this->MapInputToAction(vtkEventDataDevice::RightController,
    vtkEventDataDeviceInput::TrackPad, VTKIS_DOLLY);

  this->MapInputToAction(vtkEventDataDevice::LeftController,
    vtkEventDataDeviceInput::Grip, VTKIS_POSITION_PROP);

  this->EventCallbackCommand->SetCallback(vtkVirtualRealityViewInteractorStyle::ProcessEvents);
}

//----------------------------------------------------------------------------
vtkVirtualRealityViewInteractorStyle::~vtkVirtualRealityViewInteractorStyle()
{
  this->SetDisplayableManagerGroup(nullptr);
  delete this->Internal;
}

//----------------------------------------------------------------------------
void vtkVirtualRealityViewInteractorStyle::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
} 

//----------------------------------------------------------------------------
void vtkVirtualRealityViewInteractorStyle::SetInteractor(vtkRenderWindowInteractor *i)
{
  if (i == this->Interactor)
  {
    return;
  }

  this->Superclass::SetInteractor(i);

  // add observers for each of the events handled in ProcessEvents
  if (i)
  {
    i->AddObserver(vtkCommand::StartPinchEvent,
                   this->EventCallbackCommand,
                   this->Priority);

    i->AddObserver(vtkCommand::StartRotateEvent,
                   this->EventCallbackCommand,
                   this->Priority);

    i->AddObserver(vtkCommand::StartPanEvent,
                   this->EventCallbackCommand,
                   this->Priority);

    i->AddObserver(vtkCommand::EndPinchEvent,
                   this->EventCallbackCommand,
                   this->Priority);

    i->AddObserver(vtkCommand::EndRotateEvent,
                   this->EventCallbackCommand,
                   this->Priority);

    i->AddObserver(vtkCommand::EndPanEvent,
                   this->EventCallbackCommand,
                   this->Priority);
  }
}

//----------------------------------------------------------------------------
void vtkVirtualRealityViewInteractorStyle::ProcessEvents(
  vtkObject* object, unsigned long event, void* clientdata, void* calldata)
{
  Superclass::ProcessEvents(object, event, clientdata, calldata);

  vtkVirtualRealityViewInteractorStyle* self =
    reinterpret_cast<vtkVirtualRealityViewInteractorStyle*>(clientdata);

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
  }
}

//----------------------------------------------------------------------------
// Generic events binding
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
void vtkVirtualRealityViewInteractorStyle::OnMove3D(vtkEventData* edata)
{
  vtkEventDataDevice3D *edd = edata->GetAsEventDataDevice3D();
  if (!edd)
  {
    return;
  }
  int idev = static_cast<int>(edd->GetDevice());

  // Update current state
  int x = this->Interactor->GetEventPosition()[0];
  int y = this->Interactor->GetEventPosition()[1];

  switch (this->Internal->InteractionState[idev])
  {
    case VTKIS_POSITION_PROP:
      this->FindPokedRenderer(x, y);
      this->PositionProp(edd);
      this->InvokeEvent(vtkCommand::InteractionEvent, nullptr);
      break;
    case VTKIS_DOLLY:
      this->FindPokedRenderer(x, y);
      this->Dolly3D(edata);
      this->InvokeEvent(vtkCommand::InteractionEvent, nullptr);
      break;
    //case VTKIS_CLIP:
    //  this->FindPokedRenderer(x, y);
    //  this->Clip(edd);
    //  this->InvokeEvent(vtkCommand::InteractionEvent, nullptr);
    //  break;
  }

  //// Update rays
  //if (this->HoverPick)
  //{
  //  this->UpdateRay(edd->GetDevice());
  //}
}

//----------------------------------------------------------------------------
void vtkVirtualRealityViewInteractorStyle::OnButton3D(vtkEventData* edata)
{
  vtkEventDataDevice3D *bd = edata->GetAsEventDataDevice3D();
  if (!bd)
  {
    return;
  }

  int x = this->Interactor->GetEventPosition()[0];
  int y = this->Interactor->GetEventPosition()[1];
  this->FindPokedRenderer(x, y);

  int state = this->InputMap[static_cast<int>(bd->GetDevice())][static_cast<int>(bd->GetInput())];
  if (state == -1)
  {
    return;
  }

  // Right trigger press/release
  if (bd->GetAction() == vtkEventDataAction::Press)
  {
    this->StartAction(state, bd);
  }
  if (bd->GetAction() == vtkEventDataAction::Release)
  {
    this->EndAction(state, bd);
  }
}

//----------------------------------------------------------------------------
// Interaction methods
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
void vtkVirtualRealityViewInteractorStyle::PositionProp(vtkEventData* ed)
{
  vtkEventDataDevice3D *edd = ed->GetAsEventDataDevice3D();
  if (!edd)
  {
    return;
  }
  int deviceIndex = static_cast<int>(edd->GetDevice());
  vtkMRMLDisplayableNode* pickedNode = this->Internal->PickedNode[deviceIndex];

  if ( pickedNode == nullptr || !pickedNode->GetSelectable()
    || !this->CurrentRenderer || !this->DisplayableManagerGroup )
  {
    return;
  }
  if (ed->GetType() != vtkCommand::Move3DEvent)
  {
    return;
  }

  // Get positions and orientations
  vtkRenderWindowInteractor3D* rwi = static_cast<vtkRenderWindowInteractor3D*>(this->Interactor);
  double worldPos[3] = {0.0};
  edd->GetWorldPosition(worldPos);
  double* lastWorldPos = rwi->GetLastWorldEventPosition(rwi->GetPointerIndex());
  double* worldOrientation = rwi->GetWorldEventOrientation(rwi->GetPointerIndex());
  double* lastWorldOrientation = rwi->GetLastWorldEventOrientation(rwi->GetPointerIndex());

  // Calculate transform
  vtkNew<vtkTransform> interactionTransform;
  interactionTransform->PreMultiply();

  double translation[3] = {0.0};
  for (int i = 0; i < 3; i++)
  {
    translation[i] = worldPos[i] - lastWorldPos[i];
  }
  vtkQuaternion<double> q1;
  q1.SetRotationAngleAndAxis(vtkMath::RadiansFromDegrees(
    lastWorldOrientation[0]), lastWorldOrientation[1], lastWorldOrientation[2], lastWorldOrientation[3]);
  vtkQuaternion<double> q2;
  q2.SetRotationAngleAndAxis(vtkMath::RadiansFromDegrees(
    worldOrientation[0]), worldOrientation[1], worldOrientation[2], worldOrientation[3]);
  q1.Conjugate();
  q2 = q2*q1;
  double axis[4] = {0.0};
  axis[0] = vtkMath::DegreesFromRadians(q2.GetRotationAngleAndAxis(axis+1));
  interactionTransform->Translate(worldPos[0], worldPos[1], worldPos[2]);
  interactionTransform->RotateWXYZ(axis[0], axis[1], axis[2], axis[3]);
  interactionTransform->Translate(-worldPos[0], -worldPos[1], -worldPos[2]);
  interactionTransform->Translate(translation[0], translation[1], translation[2]);

  // Make sure that the topmost parent transform is the VR interaction transform
  vtkMRMLTransformNode* topTransformNode = pickedNode->GetParentTransformNode();
  while (topTransformNode && topTransformNode->GetParentTransformNode())
  {
    topTransformNode = topTransformNode->GetParentTransformNode();
  }
  vtkMRMLTransformNode* vrTransformNode = NULL;
  if (!topTransformNode)
  {
    // Create interaction transform if no transform found
    vtkNew<vtkMRMLLinearTransformNode> newTransformNode;
    std::string vrTransformNodeName(pickedNode->GetName());
    vrTransformNodeName.append("_VR_Interaction_Transform");
    newTransformNode->SetName(vrTransformNodeName.c_str());
    newTransformNode->SetAttribute(
      vtkMRMLVirtualRealityViewNode::GetVirtualRealityInteractionTransformAttributeName(), "1");
    pickedNode->GetScene()->AddNode(newTransformNode);
    pickedNode->SetAndObserveTransformNodeID(newTransformNode->GetID());
    vrTransformNode = newTransformNode.GetPointer();
  }
  else
  {
    // Make top transform the VR interaction transform
    if (!topTransformNode->GetAttribute(
      vtkMRMLVirtualRealityViewNode::GetVirtualRealityInteractionTransformAttributeName()) )
    {
      vtkInfoMacro("PositionProp: Transform " << topTransformNode->GetName() <<
        " is now used as VR interaction transform for node " << pickedNode->GetName());
      topTransformNode->SetAttribute(
        vtkMRMLVirtualRealityViewNode::GetVirtualRealityInteractionTransformAttributeName(), "1");
    }
    // VR interaction transform found (i.e. the top transform has the VR interaction transform attribute)
    vrTransformNode = topTransformNode;
  }

  // Apply interaction transform
  vtkTransform* lastVrInteractionTransform = vtkTransform::SafeDownCast(vrTransformNode->GetTransformToParent());
  if (vrTransformNode->GetTransformToParent() && !lastVrInteractionTransform)
  {
    //TODO: Remove constraint
    vtkErrorMacro("PositionProp: Node " << pickedNode->GetName() << " contains non-linear transform");
    return;
  }
  // Use the interaction transform with the flattened matrix to prevent concatenation
  // of potentially thousands of transforms
  if (lastVrInteractionTransform)
  {
    interactionTransform->Concatenate(lastVrInteractionTransform->GetMatrix());
    lastVrInteractionTransform->DeepCopy(interactionTransform);
  }
  else
  {
    vrTransformNode->SetAndObserveTransformToParent(interactionTransform);
  }

  if (this->AutoAdjustCameraClippingRange)
  {
    this->CurrentRenderer->ResetCameraClippingRange();
  }
}

//----------------------------------------------------------------------------
// Interaction entry points
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
void vtkVirtualRealityViewInteractorStyle::StartPositionProp(vtkEventDataDevice3D* edata)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    return;
  }

  int deviceIndex = static_cast<int>(edata->GetDevice());

  double pos[3] = {0.0};
  edata->GetWorldPosition(pos);

  // Get MRML node to move
  for (int i=0; i<this->DisplayableManagerGroup->GetDisplayableManagerCount(); ++i)
  {
    vtkMRMLAbstractThreeDViewDisplayableManager* currentDisplayableManager =
      vtkMRMLAbstractThreeDViewDisplayableManager::SafeDownCast(this->DisplayableManagerGroup->GetNthDisplayableManager(i));
    if (!currentDisplayableManager)
    {
      continue;
    }
    currentDisplayableManager->Pick3D(pos);
    vtkMRMLDisplayNode* displayNode = vtkMRMLDisplayNode::SafeDownCast(
      scene->GetNodeByID(currentDisplayableManager->GetPickedNodeID()) );
    if (!displayNode)
    {
      continue;
    }
    //TODO: Only the first selectable picked node in the last displayable manager will be picked
    vtkMRMLNode* pickedNode = displayNode->GetDisplayableNode();
    if (pickedNode && pickedNode->GetSelectable())
    {
      this->Internal->PickedNode[deviceIndex] = pickedNode;
    }
  }

  this->Internal->InteractionState[deviceIndex] = VTKIS_POSITION_PROP;

  // Don't start action if a controller is already positioning the prop
  int rc = static_cast<int>(vtkEventDataDevice::RightController);
  int lc = static_cast<int>(vtkEventDataDevice::LeftController);
  if (this->Internal->PickedNode[rc] == this->Internal->PickedNode[lc])
  {
    this->EndPositionProp(edata);
    return;
  }

  if (this->CurrentRenderer == nullptr || this->InteractionProp == nullptr)
  {
    return;
  }
}

//----------------------------------------------------------------------------
void vtkVirtualRealityViewInteractorStyle::EndPositionProp(vtkEventDataDevice3D* edata)
{
  int deviceIndex = static_cast<int>(edata->GetDevice());
  this->Internal->InteractionState[deviceIndex] = VTKIS_NONE;
  this->Internal->PickedNode[deviceIndex] = nullptr;
}

//----------------------------------------------------------------------------
void vtkVirtualRealityViewInteractorStyle::StartDolly3D(vtkEventDataDevice3D* ed)
{
  if (this->CurrentRenderer == nullptr)
  {
    return;
  }
  vtkEventDataDevice dev = ed->GetDevice();
  this->Internal->InteractionState[static_cast<int>(dev)] = VTKIS_DOLLY;
  this->LastDolly3DEventTime->StartTimer();

  // this->GrabFocus(this->EventCallbackCommand);
}

//----------------------------------------------------------------------------
void vtkVirtualRealityViewInteractorStyle::EndDolly3D(vtkEventDataDevice3D* ed)
{
  vtkEventDataDevice dev = ed->GetDevice();
  this->Internal->InteractionState[static_cast<int>(dev)] = VTKIS_NONE;

  this->LastDolly3DEventTime->StopTimer();
}

//----------------------------------------------------------------------------
// Multitouch interaction methods
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
void vtkVirtualRealityViewInteractorStyle::OnPan()
{
  this->OnPinch3D();
}

//----------------------------------------------------------------------------
void vtkVirtualRealityViewInteractorStyle::OnPinch()
{
  this->OnPinch3D();
}

//----------------------------------------------------------------------------
void vtkVirtualRealityViewInteractorStyle::OnRotate()
{
  this->OnPinch3D();
}

//----------------------------------------------------------------------------
void vtkVirtualRealityViewInteractorStyle::OnPinch3D()
{
  int rc = static_cast<int>(vtkEventDataDevice::RightController);
  int lc = static_cast<int>(vtkEventDataDevice::LeftController);
  if ( (this->Internal->PickedNode[rc] && this->Internal->PickedNode[rc]->GetSelectable())
    || (this->Internal->PickedNode[lc] && this->Internal->PickedNode[lc]->GetSelectable()) )
  {
    // If a node is being picked then don't do the pinch 3D operation
    return;
  }

  this->Internal->InteractionState[rc] = VTKIS_ZOOM;
  this->Internal->InteractionState[lc] = VTKIS_ZOOM;

  if (this->CurrentRenderer == nullptr)
  {
    return;
  }
  if (!this->Internal->StartingControllerPoseValid)
  {
    // If starting pose is not valid then cannot do the pinch 3D operation
    return;
  }

  vtkOpenVRRenderWindowInteractor* rwi = static_cast<vtkOpenVRRenderWindowInteractor*>(this->Interactor);
  vtkOpenVRRenderWindow* rw = static_cast<vtkOpenVRRenderWindow*>(rwi->GetRenderWindow());

  int pointer = this->Interactor->GetPointerIndex();

  this->FindPokedRenderer(
    this->Interactor->GetEventPositions(pointer)[0], this->Interactor->GetEventPositions(pointer)[1]);

  // Get current controller poses
  vtkNew<vtkMatrix4x4> currentController0Pose_Physical;
  rwi->GetPhysicalEventPose(currentController0Pose_Physical, 0);
  vtkNew<vtkMatrix4x4> currentController1Pose_Physical;
  rwi->GetPhysicalEventPose(currentController1Pose_Physical, 1);

  // Get combined current controller pose
  vtkNew<vtkMatrix4x4> combinedCurrentControllerPose;
  if ( !this->Internal->CalculateCombinedControllerPose(
    currentController0Pose_Physical, currentController1Pose_Physical, combinedCurrentControllerPose ) )
  {
    // If combined pose is invalid, then use the last valid pose if it exists
    if (this->Internal->LastValidCombinedControllerPoseExists)
    {
      combinedCurrentControllerPose->DeepCopy(this->Internal->LastValidCombinedControllerPose);
    }
    else
    {
      // There is no valid last position so cannot do the pinch 3D operation
      return;
    }
  }
  else
  {
    // Save current as last valid pose
    this->Internal->LastValidCombinedControllerPose->DeepCopy(combinedCurrentControllerPose);
  }

  // Calculate modifier matrix
  vtkNew<vtkMatrix4x4> inverseCombinedCurrentControllerPose;
  vtkMatrix4x4::Invert(combinedCurrentControllerPose, inverseCombinedCurrentControllerPose);

  vtkNew<vtkMatrix4x4> modifyPhysicalToWorldMatrix;
  vtkMatrix4x4::Multiply4x4(
    this->Internal->CombinedStartingControllerPose, inverseCombinedCurrentControllerPose, modifyPhysicalToWorldMatrix);

  // Calculate new physical to world matrix
  vtkNew<vtkMatrix4x4> startingPhysicalToWorldMatrix;
  rwi->GetStartingPhysicalToWorldMatrix(startingPhysicalToWorldMatrix);
  vtkNew<vtkMatrix4x4> newPhysicalToWorldMatrix;
  vtkMatrix4x4::Multiply4x4(
    startingPhysicalToWorldMatrix, modifyPhysicalToWorldMatrix, newPhysicalToWorldMatrix);

  // Set new physical to world matrix
  rw->SetPhysicalToWorldMatrix(newPhysicalToWorldMatrix);
  if (this->AutoAdjustCameraClippingRange)
  {
    this->CurrentRenderer->ResetCameraClippingRange();
  }
  if (this->Interactor->GetLightFollowCamera())
  {
    this->CurrentRenderer->UpdateLightsGeometryToFollowCamera();
  }
}

//----------------------------------------------------------------------------
void vtkVirtualRealityViewInteractorStyle::OnStartGesture()
{
  // Store combined starting controller pose
  vtkOpenVRRenderWindowInteractor* rwi = static_cast<vtkOpenVRRenderWindowInteractor*>(this->Interactor);
  vtkNew<vtkMatrix4x4> startingController0Pose_Physical;
  rwi->GetStartingPhysicalEventPose(startingController0Pose_Physical, 0);
  vtkNew<vtkMatrix4x4> startingController1Pose_Physical;
  rwi->GetStartingPhysicalEventPose(startingController1Pose_Physical, 1);

  if ( this->Internal->CalculateCombinedControllerPose(
    startingController0Pose_Physical, startingController1Pose_Physical, this->Internal->CombinedStartingControllerPose) )
  {
    this->Internal->StartingControllerPoseValid = true;
  }
}

//----------------------------------------------------------------------------
void vtkVirtualRealityViewInteractorStyle::OnEndGesture()
{
  this->Internal->StartingControllerPoseValid = false;
  this->Internal->LastValidCombinedControllerPoseExists = false;
}

//----------------------------------------------------------------------------
// Utility routines
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
void vtkVirtualRealityViewInteractorStyle::MapInputToAction(
  vtkEventDataDevice device, vtkEventDataDeviceInput input, int state)
{
  if (input >= vtkEventDataDeviceInput::NumberOfInputs || state < VTKIS_NONE)
  {
    return;
  }

  int oldstate = this->InputMap[static_cast<int>(device)][static_cast<int>(input)];
  if (oldstate == state)
  {
    return;
  }

  this->InputMap[static_cast<int>(device)][static_cast<int>(input)] = state;
  //this->AddTooltipForInput(device, input); //TODO:

  this->Modified();
}

//----------------------------------------------------------------------------
void vtkVirtualRealityViewInteractorStyle::StartAction(int state, vtkEventDataDevice3D* edata)
{
  switch (state)
  {
    case VTKIS_POSITION_PROP:
      this->StartPositionProp(edata);
      break;
    case VTKIS_DOLLY:
      this->StartDolly3D(edata);
      break;
    //case VTKIS_CLIP:
    //  this->StartClip(edata);
    //  break;
    //case VTKIS_PICK:
    //  this->StartPick(edata);
    //  break;
    //case VTKIS_LOAD_CAMERA_POSE:
    //  this->StartLoadCamPose(edata);
    //  break;
  }
}

//----------------------------------------------------------------------------
void vtkVirtualRealityViewInteractorStyle::EndAction(int state, vtkEventDataDevice3D* edata)
{
  switch (state)
  {
    case VTKIS_POSITION_PROP:
      this->EndPositionProp(edata);
      break;
    case VTKIS_DOLLY:
      this->EndDolly3D(edata);
      break;
    //case VTKIS_CLIP:
    //  this->EndClip(edata);
    //  break;
    //case VTKIS_PICK:
    //  this->EndPick(edata);
    //  break;
    //case VTKIS_MENU:
    //  this->Menu->SetInteractor(this->Interactor);
    //  this->Menu->Show(edata);
    //  break;
    //case VTKIS_LOAD_CAMERA_POSE:
    //  this->EndLoadCamPose(edata);
    //  break;
    //case VTKIS_TOGGLE_DRAW_CONTROLS:
    //  this->ToggleDrawControls();
    //  break;
    //case VTKIS_EXIT:
    //  if (this->Interactor)
    //  {
    //    this->Interactor->ExitCallback();
    //  }
    //  break;
  }

  // Reset multitouch state because a button has been released
  //for (int d = 0; d < vtkEventDataNumberOfDevices; ++d)
  //{
  //  switch (this->InteractionState[d])
  //  {
  //    case VTKIS_PAN:
  //    case VTKIS_ZOOM:
  //    case VTKIS_ROTATE:
  //      this->InteractionState[d] = VTKIS_NONE;
  //      break;
  //  }
  //}
}

//---------------------------------------------------------------------------
vtkMRMLScene* vtkVirtualRealityViewInteractorStyle::GetMRMLScene()
{
  if (!this->DisplayableManagerGroup
    || this->DisplayableManagerGroup->GetDisplayableManagerCount() == 0)
  {
    return nullptr;
  }

  return this->DisplayableManagerGroup->GetNthDisplayableManager(0)->GetMRMLScene();
}

//---------------------------------------------------------------------------
void vtkVirtualRealityViewInteractorStyle::SetMagnification(double magnification)
{
  if (this->CurrentRenderer == nullptr)
  {
    return;
  }

  vtkOpenVRRenderWindowInteractor* rwi = static_cast<vtkOpenVRRenderWindowInteractor*>(this->Interactor);
  vtkOpenVRRenderWindow* rw = static_cast<vtkOpenVRRenderWindow*>(rwi->GetRenderWindow());

  double currentPhysicalScale = rw->GetPhysicalScale();
  double newPhysicalScale = 1000.0 / magnification;
  double scaleFactor = newPhysicalScale / currentPhysicalScale;
  if (fabs(scaleFactor - 1.0) < 0.001)
  {
    return;
  }

  // Get Physical to World_Origin matrix
  vtkNew<vtkMatrix4x4> physicalToWorldOriginMatrix;
  rw->GetPhysicalToWorldMatrix(physicalToWorldOriginMatrix);

  // Calculate World_Origin to World_FocusPoint matrix
  double bounds[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  this->CurrentRenderer->ComputeVisiblePropBounds(bounds);
  double focusPoint_World[3] = { (bounds[1] + bounds[0]) / 2.0,
                                 (bounds[3] + bounds[2]) / 2.0,
                                 (bounds[5] + bounds[4]) / 2.0 };
  vtkNew<vtkMatrix4x4> worldOriginToWorldFocusPointMatrix;
  for (int i=0; i<3; ++i)
  {
    worldOriginToWorldFocusPointMatrix->SetElement(i,3, -focusPoint_World[i]);
  }

  // Calculate World_FocusPoint to World_ScaledFocusPoint matrix
  vtkNew<vtkMatrix4x4> worldFocusPointToWorldScaledFocusPointMatrix;
  for (int i=0; i<3; ++i)
  {
    worldFocusPointToWorldScaledFocusPointMatrix->SetElement(i,i, scaleFactor);
  }

  // Calculate World_ScaledFocusPoint to World_ScaledOrigin matrix
  vtkNew<vtkMatrix4x4> worldScaledFocusPointToWorldScaledOriginMatrix;
  for (int i=0; i<3; ++i)
  {
    worldScaledFocusPointToWorldScaledOriginMatrix->SetElement(i,3, focusPoint_World[i]);
  }

  // Calculate Physical to World_ScaledOrigin matrix
  vtkNew<vtkMatrix4x4> worldFocusPointToWorldScaledOriginMatrix;
  vtkMatrix4x4::Multiply4x4( worldScaledFocusPointToWorldScaledOriginMatrix, worldFocusPointToWorldScaledFocusPointMatrix,
    worldFocusPointToWorldScaledOriginMatrix );

  vtkNew<vtkMatrix4x4> worldOriginToWorldScaledOriginMatrix;
  vtkMatrix4x4::Multiply4x4( worldFocusPointToWorldScaledOriginMatrix, worldOriginToWorldFocusPointMatrix,
    worldOriginToWorldScaledOriginMatrix );

  // Physical to World_ScaledOrigin
  vtkNew<vtkMatrix4x4> physicalToWorldScaledOriginMatrix;
  vtkMatrix4x4::Multiply4x4( worldOriginToWorldScaledOriginMatrix, physicalToWorldOriginMatrix,
    physicalToWorldScaledOriginMatrix );

  rw->SetPhysicalToWorldMatrix(physicalToWorldScaledOriginMatrix);

  if (this->AutoAdjustCameraClippingRange)
  {
    this->CurrentRenderer->ResetCameraClippingRange();
  }
  if (this->Interactor->GetLightFollowCamera())
  {
    this->CurrentRenderer->UpdateLightsGeometryToFollowCamera();
  }
}

//---------------------------------------------------------------------------
double vtkVirtualRealityViewInteractorStyle::GetMagnification()
{
  vtkOpenVRRenderWindowInteractor* rwi = static_cast<vtkOpenVRRenderWindowInteractor*>(this->Interactor);
  vtkOpenVRRenderWindow* rw = static_cast<vtkOpenVRRenderWindow*>(rwi->GetRenderWindow());

  return 1000.0 / rw->GetPhysicalScale();
}
