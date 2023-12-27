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

#include "vtkVirtualRealityViewInteractorStyleDelegate.h"

// VirtualRealityModule includes

// VR Logic includes
#include "vtkSlicerVirtualRealityLogic.h"

// VR MRML includes
#include "vtkMRMLVirtualRealityViewNode.h"

// MRML includes
#include <vtkMRMLDisplayableNode.h>
#include <vtkMRMLDisplayNode.h>
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLScene.h>

// MRMLDM includes
#include <vtkMRMLAbstractThreeDViewDisplayableManager.h>

// VTK/Rendering/VR includes
#include <vtkVRRenderWindow.h>
#include <vtkVRRenderWindowInteractor.h>

// VTK includes
#include <vtkInteractorStyle.h>
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>
#include <vtkQuaternion.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor3D.h>
#include <vtkTransform.h>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkVirtualRealityViewInteractorStyleDelegate);

//----------------------------------------------------------------------------
void vtkVirtualRealityViewInteractorStyleDelegate::SetInteractorStyle(vtkVRInteractorStyle* interactorStyle)
{
  vtkSetSmartPointerBodyMacro(InteractorStyle, vtkVRInteractorStyle, interactorStyle);

  if (interactorStyle != nullptr)
    {
    // Create default inputs mapping

    // The "eventId to state" mapping (see call to `MapInputToAction` below) applies to right and left
    // controller buttons because they are bound to the same eventId:
    // - `vtk_openvr_binding_*.json` files define the "button to action" mapping
    // - `vtkOpenVRInteractorStyle()` contructor defines the "action to eventId" mapping

    interactorStyle->MapInputToAction(vtkCommand::ViewerMovement3DEvent, VTKIS_DOLLY);
    interactorStyle->MapInputToAction(vtkCommand::Select3DEvent, VTKIS_POSITION_PROP);
    }
}

//---------------------------------------------------------------------------
vtkMRMLScene* vtkVirtualRealityViewInteractorStyleDelegate::GetMRMLScene() const
{
  if (!this->DisplayableManagers
    || this->DisplayableManagers->GetDisplayableManagerCount() == 0)
  {
    vtkErrorMacro("GetMRMLScene failed: DisplayableManagers is not set");
    return nullptr;
  }

  return this->DisplayableManagers->GetNthDisplayableManager(0)->GetMRMLScene();
}

//----------------------------------------------------------------------------
void vtkVirtualRealityViewInteractorStyleDelegate::OnPan()
{
  this->OnPinch3D();
}

//----------------------------------------------------------------------------
void vtkVirtualRealityViewInteractorStyleDelegate::OnPinch()
{
  this->OnPinch3D();
}

//----------------------------------------------------------------------------
void vtkVirtualRealityViewInteractorStyleDelegate::OnRotate()
{
  this->OnPinch3D();
}

//----------------------------------------------------------------------------
void vtkVirtualRealityViewInteractorStyleDelegate::OnPinch3D()
{
  vtkVRInteractorStyle* istyle = this->InteractorStyle;
  if (istyle == nullptr)
  {
    vtkErrorMacro("OnPinch3D failed: InteractorStyle is not set");
    return;
  }
  if (istyle->GetCurrentRenderer() == nullptr)
  {
    vtkErrorMacro("OnPinch3D failed: CurrentRenderer is not set");
    return;
  }

  int rc = static_cast<int>(vtkEventDataDevice::RightController);
  int lc = static_cast<int>(vtkEventDataDevice::LeftController);
  if ( (this->PickedNode[rc] && this->PickedNode[rc]->GetSelectable())
    || (this->PickedNode[lc] && this->PickedNode[lc]->GetSelectable()) )
  {
    // If a node is being picked then don't do the pinch 3D operation
    return;
  }

  istyle->SetInteractionState(vtkEventDataDevice::RightController, VTKIS_ZOOM);
  istyle->SetInteractionState(vtkEventDataDevice::LeftController, VTKIS_ZOOM);

  if (!this->StartingControllerPoseValid)
  {
    // If starting pose is not valid then cannot do the pinch 3D operation
    return;
  }

  vtkVRRenderWindowInteractor* rwi = vtkVRRenderWindowInteractor::SafeDownCast(istyle->GetInteractor());
  vtkVRRenderWindow* rw = vtkVRRenderWindow::SafeDownCast(rwi->GetRenderWindow());

  int pointer = rwi->GetPointerIndex();

  istyle->FindPokedRenderer(rwi->GetEventPositions(pointer)[0], rwi->GetEventPositions(pointer)[1]);

  // Get current controller poses
  vtkMatrix4x4* currentController0Pose_Physical =
      rw->GetDeviceToPhysicalMatrixForDevice(vtkEventDataDevice::LeftController);
  vtkMatrix4x4* currentController1Pose_Physical =
      rw->GetDeviceToPhysicalMatrixForDevice(vtkEventDataDevice::RightController);

  // Get combined current controller pose
  vtkNew<vtkMatrix4x4> combinedCurrentControllerPose;
  if ( !vtkSlicerVirtualRealityLogic::CalculateCombinedControllerPose(
    currentController0Pose_Physical, currentController1Pose_Physical, combinedCurrentControllerPose ) )
  {
    // If combined pose is invalid, then use the last valid pose if it exists
    if (this->LastValidCombinedControllerPoseExists)
    {
      combinedCurrentControllerPose->DeepCopy(this->LastValidCombinedControllerPose);
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
    this->LastValidCombinedControllerPose->DeepCopy(combinedCurrentControllerPose);
  }

  // Calculate modifier matrix
  vtkNew<vtkMatrix4x4> inverseCombinedCurrentControllerPose;
  vtkMatrix4x4::Invert(combinedCurrentControllerPose, inverseCombinedCurrentControllerPose);

  vtkNew<vtkMatrix4x4> modifyPhysicalToWorldMatrix;
  vtkMatrix4x4::Multiply4x4(
    this->CombinedStartingControllerPose, inverseCombinedCurrentControllerPose, modifyPhysicalToWorldMatrix);

  // Calculate new physical to world matrix
  vtkNew<vtkMatrix4x4> startingPhysicalToWorldMatrix;
  rwi->GetStartingPhysicalToWorldMatrix(startingPhysicalToWorldMatrix);
  vtkNew<vtkMatrix4x4> newPhysicalToWorldMatrix;
  vtkMatrix4x4::Multiply4x4(
    startingPhysicalToWorldMatrix, modifyPhysicalToWorldMatrix, newPhysicalToWorldMatrix);

  // Set new physical to world matrix
  rw->SetPhysicalToWorldMatrix(newPhysicalToWorldMatrix);
  if (istyle->GetAutoAdjustCameraClippingRange())
  {
    istyle->GetCurrentRenderer()->ResetCameraClippingRange();
  }
  if (rwi->GetLightFollowCamera())
  {
    istyle->GetCurrentRenderer()->UpdateLightsGeometryToFollowCamera();
  }
}

//----------------------------------------------------------------------------
void vtkVirtualRealityViewInteractorStyleDelegate::StartGesture()
{
  vtkVRInteractorStyle* istyle = this->InteractorStyle;
  if (istyle == nullptr)
  {
    vtkErrorMacro("StartGesture failed: InteractorStyle is not set");
    return;
  }

  // Store combined starting controller pose
  vtkVRRenderWindow* rw = vtkVRRenderWindow::SafeDownCast(istyle->GetInteractor()->GetRenderWindow());

  vtkMatrix4x4* startingController0Pose_Physical =
      rw->GetDeviceToPhysicalMatrixForDevice(vtkEventDataDevice::LeftController);
  vtkMatrix4x4* startingController1Pose_Physical =
      rw->GetDeviceToPhysicalMatrixForDevice(vtkEventDataDevice::RightController);

  if ( vtkSlicerVirtualRealityLogic::CalculateCombinedControllerPose(
    startingController0Pose_Physical, startingController1Pose_Physical, this->CombinedStartingControllerPose) )
  {
    this->StartingControllerPoseValid = true;
  }
}

//----------------------------------------------------------------------------
void vtkVirtualRealityViewInteractorStyleDelegate::EndGesture()
{
  this->StartingControllerPoseValid = false;
  this->LastValidCombinedControllerPoseExists = false;
}

//----------------------------------------------------------------------------
// Interaction methods
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
void vtkVirtualRealityViewInteractorStyleDelegate::PositionProp(
    vtkEventData* ed, double* vtkNotUsed(lwpos), double* vtkNotUsed(lwori))
{
  vtkVRInteractorStyle* istyle = this->InteractorStyle;
  if (istyle == nullptr)
  {
    vtkErrorMacro("PositionProp failed: InteractorStyle is not set");
    return;
  }
  if (istyle->GetCurrentRenderer() == nullptr)
  {
    vtkErrorMacro("PositionProp failed: CurrentRenderer is not set");
    return;
  }
  if (this->DisplayableManagers == nullptr)
  {
    vtkErrorMacro("PositionProp failed: DisplayableManagers is not set");
    return;
  }

  vtkEventDataDevice3D *edd = ed->GetAsEventDataDevice3D();
  if (edd == nullptr)
  {
    vtkErrorMacro("PositionProp failed: 3D event is expected");
    return;
  }
  int deviceIndex = static_cast<int>(edd->GetDevice());
  vtkMRMLDisplayableNode* pickedNode = this->PickedNode[deviceIndex];
  if (pickedNode == nullptr || !pickedNode->GetSelectable())
  {
    return;
  }
  if (ed->GetType() != vtkCommand::Move3DEvent)
  {
    return;
  }

  // Get positions and orientations
  vtkRenderWindowInteractor3D* rwi = vtkRenderWindowInteractor3D::SafeDownCast(istyle->GetInteractor());
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
  vtkMRMLTransformNode* vrTransformNode = nullptr;
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
    vtkErrorMacro("PositionProp failed: Node " << pickedNode->GetName() << " contains non-linear transform");
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

  if (istyle->GetAutoAdjustCameraClippingRange())
  {
    istyle->GetCurrentRenderer()->ResetCameraClippingRange();
  }
}

//----------------------------------------------------------------------------
// Interaction entry points
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
void vtkVirtualRealityViewInteractorStyleDelegate::StartPositionProp(vtkEventDataDevice3D* edata)
{
  vtkVRInteractorStyle* istyle = this->InteractorStyle;
  if (istyle == nullptr)
  {
    vtkErrorMacro("StartPositionProp failed: InteractorStyle is not set");
    return;
  }

  if (this->DisplayableManagers == nullptr)
  {
    vtkErrorMacro("StartPositionProp failed: DisplayableManagers is not set");
    return;
  }

  vtkMRMLScene* scene = this->GetMRMLScene();
  if (scene == nullptr)
  {
    vtkErrorMacro("StartPositionProp failed: scenee is not set");
    return;
  }

  vtkEventDataDevice device = edata->GetDevice();

  double pos[3] = {0.0};
  edata->GetWorldPosition(pos);

  // Get MRML node to move
  for (int i=0; i<this->DisplayableManagers->GetDisplayableManagerCount(); ++i)
  {
    vtkMRMLAbstractThreeDViewDisplayableManager* currentDisplayableManager =
      vtkMRMLAbstractThreeDViewDisplayableManager::SafeDownCast(this->DisplayableManagers->GetNthDisplayableManager(i));
    if (currentDisplayableManager == nullptr)
    {
      continue;
    }
    currentDisplayableManager->Pick3D(pos);
    vtkMRMLDisplayNode* displayNode = vtkMRMLDisplayNode::SafeDownCast(
      scene->GetNodeByID(currentDisplayableManager->GetPickedNodeID()) );
    if (displayNode == nullptr)
    {
      continue;
    }
    //TODO: Only the first selectable picked node in the last displayable manager will be picked
    vtkMRMLDisplayableNode* pickedNode = displayNode->GetDisplayableNode();
    if (pickedNode != nullptr && pickedNode->GetSelectable() && this->GrabEnabled)
    {
      this->PickedNode[static_cast<int>(device)] = pickedNode;
    }
  }

  istyle->SetInteractionState(device, VTKIS_POSITION_PROP);

  // Don't start action if a controller is already positioning the prop
  int rc = static_cast<int>(vtkEventDataDevice::RightController);
  int lc = static_cast<int>(vtkEventDataDevice::LeftController);
  if (this->PickedNode[rc] == this->PickedNode[lc])
  {
    this->EndPositionProp(edata);
    return;
  }
}

//----------------------------------------------------------------------------
void vtkVirtualRealityViewInteractorStyleDelegate::EndPositionProp(vtkEventDataDevice3D* edata)
{
  vtkVRInteractorStyle* istyle = this->InteractorStyle;
  if (istyle == nullptr)
  {
    vtkErrorMacro("EndPositionProp failed: InteractorStyle is not set");
    return;
  }

  vtkEventDataDevice device = edata->GetDevice();
  istyle->SetInteractionState(device, VTKIS_NONE);
  this->PickedNode[static_cast<int>(device)] = nullptr;
}

//----------------------------------------------------------------------------
void vtkVirtualRealityViewInteractorStyleDelegate::SetMagnification(double magnification)
{
  vtkVRInteractorStyle* istyle = this->InteractorStyle;
  if (istyle == nullptr)
  {
    vtkErrorMacro("SetMagnification failed: InteractorStyle is not set");
    return;
  }

  if (!istyle->GetCurrentRenderer())
  {
    return;
  }

  vtkVRRenderWindow* rw = vtkVRRenderWindow::SafeDownCast(istyle->GetInteractor()->GetRenderWindow());

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
  istyle->GetCurrentRenderer()->ComputeVisiblePropBounds(bounds);
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

  if (istyle->GetAutoAdjustCameraClippingRange())
  {
    istyle->GetCurrentRenderer()->ResetCameraClippingRange();
  }
  if (istyle->GetInteractor()->GetLightFollowCamera())
  {
    istyle->GetCurrentRenderer()->UpdateLightsGeometryToFollowCamera();
  }
}

//---------------------------------------------------------------------------
double vtkVirtualRealityViewInteractorStyleDelegate::GetMagnification()
{
  vtkVRInteractorStyle* istyle = this->InteractorStyle;
  if (istyle == nullptr)
  {
    vtkErrorMacro("GetMagnification failed: InteractorStyle is not set");
    return 1.0;
  }

  vtkVRRenderWindow* rw = vtkVRRenderWindow::SafeDownCast(istyle->GetInteractor()->GetRenderWindow());
  return 1000.0 / rw->GetPhysicalScale();
}
