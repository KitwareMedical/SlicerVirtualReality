/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// VR Logic includes
#include "vtkSlicerVirtualRealityLogic.h"

// VR MRML includes
#include "vtkMRMLVirtualRealityViewNode.h"

// MRML includes
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLModelDisplayNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSegmentationDisplayNode.h>

// Slicer includes
#include <vtkSlicerVolumeRenderingLogic.h>
#include <vtkSlicerConfigure.h> // For Slicer_THIRDPARTY_LIB_DIR

// VTK/Rendering/VR includes
#include <vtkVRInteractorStyle.h>
#include <vtkVRRenderWindowInteractor.h>

// VTK includes
#include <vtkIntArray.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// STD includes
#include <cassert>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerVirtualRealityLogic);
vtkCxxSetObjectMacro(vtkSlicerVirtualRealityLogic, VolumeRenderingLogic, vtkSlicerVolumeRenderingLogic);

//----------------------------------------------------------------------------
vtkSlicerVirtualRealityLogic::vtkSlicerVirtualRealityLogic()
  : ActiveViewNode(nullptr)
  , VolumeRenderingLogic(nullptr)
{
}

//----------------------------------------------------------------------------
vtkSlicerVirtualRealityLogic::~vtkSlicerVirtualRealityLogic()
{
  this->SetActiveViewNode(nullptr);
  this->SetVolumeRenderingLogic(nullptr);
}

//----------------------------------------------------------------------------
void vtkSlicerVirtualRealityLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkSlicerVirtualRealityLogic::SetMRMLSceneInternal(vtkMRMLScene* newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  events->InsertNextValue(vtkMRMLScene::EndImportEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkSlicerVirtualRealityLogic::RegisterNodes()
{
  assert(this->GetMRMLScene() != 0);
  // Register VirtualReality view node class
  this->GetMRMLScene()->RegisterNodeClass((vtkSmartPointer<vtkMRMLVirtualRealityViewNode>::New()));
}

//---------------------------------------------------------------------------
void vtkSlicerVirtualRealityLogic::UpdateFromMRMLScene()
{
  vtkMRMLVirtualRealityViewNode* vrViewNode = nullptr;
  if (this->GetMRMLScene())
  {
    vrViewNode = vtkMRMLVirtualRealityViewNode::SafeDownCast(
                   this->GetMRMLScene()->GetSingletonNode("Active", "vtkMRMLVirtualRealityViewNode"));
  }
  this->SetActiveViewNode(vrViewNode);
}

//---------------------------------------------------------------------------
void vtkSlicerVirtualRealityLogic::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  vtkMRMLVirtualRealityViewNode* vrViewNode = vtkMRMLVirtualRealityViewNode::SafeDownCast(node);
  if (!vrViewNode)
  {
    return;
  }
  // If a new active VR view node added then use it automatically.
  if (vrViewNode->GetSingletonTag() &&
      strcmp(vrViewNode->GetSingletonTag(), "Active") == 0)
  {
    this->SetActiveViewNode(vrViewNode);
  }
}

//---------------------------------------------------------------------------
void vtkSlicerVirtualRealityLogic::OnMRMLSceneNodeRemoved(vtkMRMLNode* node)
{
  vtkMRMLVirtualRealityViewNode* deletedVrViewNode = vtkMRMLVirtualRealityViewNode::SafeDownCast(node);
  if (!deletedVrViewNode)
  {
    return;
  }
  if (deletedVrViewNode == this->ActiveViewNode)
  {
    this->SetActiveViewNode(nullptr);
  }
}

//----------------------------------------------------------------------------
void vtkSlicerVirtualRealityLogic::OnMRMLSceneEndImport()
{
  if (this->ActiveViewNode != nullptr && this->ActiveViewNode->GetActive())
  {
    // Override the active flag and visibility flags, as VR connection is not restored on scene load
    this->ActiveViewNode->SetActive(0);
    this->ActiveViewNode->SetVisibility(0);
  }

  this->Modified();
}

//----------------------------------------------------------------------------
vtkMRMLVirtualRealityViewNode* vtkSlicerVirtualRealityLogic::GetVirtualRealityViewNode()
{
  return this->ActiveViewNode;
}

//---------------------------------------------------------------------------
vtkMRMLVirtualRealityViewNode* vtkSlicerVirtualRealityLogic::AddVirtualRealityViewNode()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("AddVirtualRealityViewNode: Invalid MRML scene");
    return nullptr;
  }

  if (this->ActiveViewNode)
  {
    // There is already a usable VR node, return that
    return this->ActiveViewNode;
  }

  // Create VirtualReality view node. Use CreateNodeByClass so that node properties
  // can be overridden with default node properties defined in the scene.
  vtkSmartPointer<vtkMRMLVirtualRealityViewNode> vrViewNode = vtkSmartPointer<vtkMRMLVirtualRealityViewNode>::Take(
        vtkMRMLVirtualRealityViewNode::SafeDownCast(scene->CreateNodeByClass("vtkMRMLVirtualRealityViewNode")));
  // We create the node as a singleton to make sure there is only one VR view node in the scene.
  vrViewNode->SetSingletonTag("Active");
  // If a singleton node by that name exists already then it is overwritten
  // and pointer of that node is returned.
  vrViewNode = vtkMRMLVirtualRealityViewNode::SafeDownCast(scene->AddNode(vrViewNode));

  return vrViewNode;
}

//----------------------------------------------------------------------------
void vtkSlicerVirtualRealityLogic::SetActiveViewNode(vtkMRMLVirtualRealityViewNode* vrViewNode)
{
  if (this->ActiveViewNode == vrViewNode)
  {
    return;
  }

  if (!this->GetMRMLScene())
  {
    return;
  }

  this->GetMRMLNodesObserverManager()->SetAndObserveObject(vtkObjectPointer(&this->ActiveViewNode), vrViewNode);

  this->Modified();
}

//-----------------------------------------------------------------------------
void vtkSlicerVirtualRealityLogic::ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void* vtkNotUsed(callData))
{
  if (caller == this->ActiveViewNode && event == vtkCommand::ModifiedEvent)
  {
    this->Modified();
  }
}

//-----------------------------------------------------------------------------
void vtkSlicerVirtualRealityLogic::SetVirtualRealityConnected(bool connect)
{
  if (connect)
  {
    this->InitializeActiveViewNode();
    if (this->ActiveViewNode)
    {
      this->ActiveViewNode->SetVisibility(1);
    }
  }
  else
  {
    if (this->ActiveViewNode)
    {
      this->ActiveViewNode->SetVisibility(0);
    }
  }
}

//-----------------------------------------------------------------------------
bool vtkSlicerVirtualRealityLogic::GetVirtualRealityConnected()
{
  if (!this->ActiveViewNode)
  {
    return false;
  }
  return (this->ActiveViewNode->GetVisibility() != 0);
}

//-----------------------------------------------------------------------------
void vtkSlicerVirtualRealityLogic::SetVirtualRealityActive(bool activate)
{
  if (activate)
  {
    if (this->GetVirtualRealityConnected()
        && this->GetVirtualRealityViewNode()
        && this->GetVirtualRealityViewNode()->HasError())
    {
      // If it is connected already but there is an error then disconnect first then reconnect
      // as the error may be resolved by reconnecting.
      this->SetVirtualRealityConnected(false);
    }
    this->SetVirtualRealityConnected(true);
    if (this->ActiveViewNode)
    {
      this->ActiveViewNode->SetActive(1);
    }
    else
    {
      vtkErrorMacro("vtkSlicerVirtualRealityLogic::SetVirtualRealityActive failed: view node is not available");
    }
  }
  else
  {
    if (this->ActiveViewNode != nullptr)
    {
      this->ActiveViewNode->SetActive(0);
    }
  }
}

//-----------------------------------------------------------------------------
bool vtkSlicerVirtualRealityLogic::GetVirtualRealityActive()
{
  if (!this->ActiveViewNode)
  {
    return false;
  }
  return (this->ActiveViewNode->GetVisibility() != 0 && this->ActiveViewNode->GetActive() != 0);
}

//-----------------------------------------------------------------------------
void vtkSlicerVirtualRealityLogic::InitializeActiveViewNode()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("InitializeActiveViewNode: Invalid MRML scene");
    return;
  }

  if (!this->ActiveViewNode)
  {
    // Check if there is a VirtualReality view node in the scene, in case the scene has been loaded
    // from file and VR view properties has been changed
    if (scene->GetNumberOfNodesByClass("vtkMRMLVirtualRealityViewNode") > 0)
    {
      // Use the first one if any found
      this->SetActiveViewNode(
        vtkMRMLVirtualRealityViewNode::SafeDownCast(scene->GetNthNodeByClass(0, "vtkMRMLVirtualRealityViewNode")));
    }
    else
    {
      vtkMRMLVirtualRealityViewNode* newViewNode = this->AddVirtualRealityViewNode();
      this->SetActiveViewNode(newViewNode);
    }
  }
  if (!this->ActiveViewNode)
  {
    vtkErrorMacro("Failed to create virtual reality view node");
  }
}

//---------------------------------------------------------------------------
void vtkSlicerVirtualRealityLogic::SetDefaultReferenceView()
{
  if (!this->ActiveViewNode)
  {
    return;
  }
  if (this->ActiveViewNode->GetReferenceViewNode() != nullptr)
  {
    // Reference view is already set, there is nothing to do
    return;
  }
  if (!this->GetMRMLScene())
  {
    return;
  }
  vtkSmartPointer<vtkCollection> nodes = vtkSmartPointer<vtkCollection>::Take(
      this->GetMRMLScene()->GetNodesByClass("vtkMRMLViewNode"));
  vtkMRMLViewNode* viewNode = nullptr;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it); (viewNode = vtkMRMLViewNode::SafeDownCast(
                                    nodes->GetNextItemAsObject(it)));)
  {
    if (viewNode->GetVisibility() && viewNode->IsMappedInLayout())
    {
      // Found a view node displayed in current layout, use this
      break;
    }
  }
  // Either use a view node displayed in current layout or just any 3D view node found in the scene
  this->ActiveViewNode->SetAndObserveReferenceViewNode(viewNode);
}

//-----------------------------------------------------------------------------
vtkMRMLVirtualRealityViewNode* vtkSlicerVirtualRealityLogic::GetDefaultVirtualRealityViewNode()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("GetDefaultVirtualRealityViewNode failed: invalid scene");
    return nullptr;
  }
  vtkMRMLNode* defaultNode = scene->GetDefaultNodeByClass("vtkMRMLVirtualRealityViewNode");
  if (!defaultNode)
  {
    defaultNode = scene->CreateNodeByClass("vtkMRMLVirtualRealityViewNode");
    scene->AddDefaultNode(defaultNode);
    defaultNode->Delete(); // scene owns it now
  }
  return vtkMRMLVirtualRealityViewNode::SafeDownCast(defaultNode);
}

//---------------------------------------------------------------------------
void vtkSlicerVirtualRealityLogic::OptimizeSceneForVirtualReality()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("OptimizeSceneForVirtualReality failed: Invalid scene");
    return;
  }

  // Steps to optimize MRML scene for virtual reality:
  // - Make sure volume rendering uses GPU
  // - Turn off backface culling for all existing models
  // - Turn off slice intersection visibility for all existing models and segmentations
  // - Apply settings in default display nodes

  // Set volume rendering method to "VTK GPU Ray Casting"
  if (this->VolumeRenderingLogic)
  {
    this->VolumeRenderingLogic->ChangeVolumeRenderingMethod("vtkMRMLGPURayCastVolumeRenderingDisplayNode");
  }
  else
  {
    vtkErrorMacro("OptimizeSceneForVirtualReality: Unable to access volume rendering logic");
  }

  // Set properties to existing model and segmentation display nodes
  std::vector<vtkMRMLNode*> modelDisplayNodes;
  scene->GetNodesByClass("vtkMRMLModelDisplayNode", modelDisplayNodes);
  for (std::vector<vtkMRMLNode*>::iterator mdIt = modelDisplayNodes.begin();
       mdIt != modelDisplayNodes.end(); ++mdIt)
  {
    vtkMRMLModelDisplayNode* modelDisplayNode = vtkMRMLModelDisplayNode::SafeDownCast(*mdIt);
    modelDisplayNode->SetBackfaceCulling(0);
    modelDisplayNode->SetVisibility2D(0);
  }

  std::vector<vtkMRMLNode*> segmentationDisplayNodes;
  scene->GetNodesByClass("vtkMRMLSegmentationDisplayNode", segmentationDisplayNodes);
  for (std::vector<vtkMRMLNode*>::iterator sdIt = segmentationDisplayNodes.begin();
       sdIt != segmentationDisplayNodes.end(); ++sdIt)
  {
    vtkMRMLSegmentationDisplayNode* segmentationDisplayNode = vtkMRMLSegmentationDisplayNode::SafeDownCast(*sdIt);
    segmentationDisplayNode->SetVisibility2DFill(0);
    segmentationDisplayNode->SetVisibility2DOutline(0);
  }

  // Set properties in default display nodes for models and segmentations so that they are propagated to new nodes
  vtkMRMLNode* defaultModelDisplayNode = scene->GetDefaultNodeByClass("vtkMRMLModelDisplayNode");
  if (!defaultModelDisplayNode)
  {
    defaultModelDisplayNode = scene->CreateNodeByClass("vtkMRMLModelDisplayNode");
    scene->AddDefaultNode(defaultModelDisplayNode);
    defaultModelDisplayNode->Delete(); // scene owns it now
  }
  vtkMRMLModelDisplayNode::SafeDownCast(defaultModelDisplayNode)->SetBackfaceCulling(0);
  vtkMRMLModelDisplayNode::SafeDownCast(defaultModelDisplayNode)->SetVisibility2D(0);

  vtkMRMLNode* defaultSegmentationDisplayNode = scene->GetDefaultNodeByClass("vtkMRMLSegmentationDisplayNode");
  if (!defaultSegmentationDisplayNode)
  {
    defaultSegmentationDisplayNode = scene->CreateNodeByClass("vtkMRMLSegmentationDisplayNode");
    scene->AddDefaultNode(defaultSegmentationDisplayNode);
    defaultSegmentationDisplayNode->Delete(); // scene owns it now
  }
  vtkMRMLSegmentationDisplayNode::SafeDownCast(defaultSegmentationDisplayNode)->SetVisibility2DFill(0);
  vtkMRMLSegmentationDisplayNode::SafeDownCast(defaultSegmentationDisplayNode)->SetVisibility2DOutline(0);
}

//---------------------------------------------------------------------------
bool vtkSlicerVirtualRealityLogic::CalculateCombinedControllerPose(
  vtkMatrix4x4* controller0Pose, vtkMatrix4x4* controller1Pose, vtkMatrix4x4* combinedPose)
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
void vtkSlicerVirtualRealityLogic::SetTriggerButtonFunction(vtkVRRenderWindowInteractor* rwi, const std::string& functionId)
{
  vtkVRInteractorStyle* vrInteractorStyle = vtkVRInteractorStyle::SafeDownCast(rwi->GetInteractorStyle());
  if (!vrInteractorStyle)
  {
    vtkWarningWithObjectMacro(rwi, "SetTriggerButtonFunction: Current interactor style is not a VR interactor style");
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
  else if (!functionId.compare(vtkSlicerVirtualRealityLogic::GetButtonFunctionIdForGrabObjectsAndWorld()))
  {
    vrInteractorStyle->MapInputToAction(vtkCommand::Select3DEvent, VTKIS_POSITION_PROP);
  }
  else
  {
    vtkErrorWithObjectMacro(rwi, "SetTriggerButtonFunction: Unknown function identifier '" << functionId << "'");
  }
}

//----------------------------------------------------------------------------
void vtkSlicerVirtualRealityLogic::SetGestureButtonToTrigger(vtkVRRenderWindowInteractor* rwi)
{
  vtkVRInteractorStyle* vrInteractorStyle = vtkVRInteractorStyle::SafeDownCast(rwi->GetInteractorStyle());
  if (!vrInteractorStyle)
  {
    vtkWarningWithObjectMacro(rwi, "SetGestureButtonToTrigger: Current interactor style is not a VR interactor style");
    return;
  }

  // The update "action to (eventId|function)" mapping (see call to `AddAction` below) applies to
  // right and left controller buttons because they are bound to the same eventId:
  // - "vtk_openvr_binding_*.json" defines the "button -> action" mapping
  // - vtkOpenVRInteractorStyle defines the "action -> eventId" mapping
  rwi->AddAction("/actions/vtk/in/TriggerAction", /*isAnalog=*/false,
                 [rwi](vtkEventData* ed) { rwi->HandleComplexGestureEvents(ed); });
  rwi->AddAction("/actions/vtk/in/ComplexGestureAction", vtkCommand::Select3DEvent, /*isAnalog=*/false);
}

//----------------------------------------------------------------------------
void vtkSlicerVirtualRealityLogic::SetGestureButtonToGrip(vtkVRRenderWindowInteractor* rwi)
{
  vtkVRInteractorStyle* vrInteractorStyle = vtkVRInteractorStyle::SafeDownCast(rwi->GetInteractorStyle());
  if (!vrInteractorStyle)
  {
    vtkWarningWithObjectMacro(rwi, "SetGestureButtonToGrip: Current interactor style is not a VR interactor style");
    return;
  }

  // The update "action to (eventId|function)" mapping (see call to `AddAction` below) applies to
  // right and left controller buttons because they are bound to the same eventId:
  // - "vtk_openvr_binding_*.json" defines the "button -> action" mapping
  // - vtkOpenVRInteractorStyle defines the "action -> eventId" mapping
  rwi->AddAction("/actions/vtk/in/TriggerAction", vtkCommand::Select3DEvent, /*isAnalog=*/false);
  rwi->AddAction("/actions/vtk/in/ComplexGestureAction", /*isAnalog=*/false,
                 [rwi](vtkEventData* ed) { rwi->HandleComplexGestureEvents(ed); });
}

//----------------------------------------------------------------------------
void vtkSlicerVirtualRealityLogic::SetGestureButtonToTriggerAndGrip(vtkVRRenderWindowInteractor* rwi)
{
  vtkVRInteractorStyle* vrInteractorStyle = vtkVRInteractorStyle::SafeDownCast(rwi->GetInteractorStyle());
  if (!vrInteractorStyle)
  {
    vtkWarningWithObjectMacro(rwi, "SetGestureButtonToTriggerAndGrip: Current interactor style is not a VR interactor style");
    return;
  }

  // The update "action to (eventId|function)" mapping (see call to `AddAction` below) applies to
  // right and left controller buttons because they are bound to the same eventId:
  // - "vtk_openvr_binding_*.json" defines the "button -> action" mapping
  // - vtkOpenVRInteractorStyle defines the "action -> eventId" mapping
  rwi->AddAction("/actions/vtk/in/TriggerAction", /*isAnalog=*/false,
                 [rwi](vtkEventData* ed) { rwi->HandleComplexGestureEvents(ed); });
  rwi->AddAction("/actions/vtk/in/ComplexGestureAction", /*isAnalog=*/false,
                 [rwi](vtkEventData* ed) { rwi->HandleComplexGestureEvents(ed); });
}

//----------------------------------------------------------------------------
void vtkSlicerVirtualRealityLogic::SetGestureButtonToNone(vtkVRRenderWindowInteractor* rwi)
{
  vtkVRInteractorStyle* vrInteractorStyle = vtkVRInteractorStyle::SafeDownCast(rwi->GetInteractorStyle());
  if (!vrInteractorStyle)
  {
    vtkWarningWithObjectMacro(rwi, "SetGestureButtonToNone: Current interactor style is not a VR interactor style");
    return;
  }

  // The update "action to (eventId|function)" mapping (see call to `AddAction` below) applies to
  // right and left controller buttons because they are bound to the same eventId:
  // - "vtk_openvr_binding_*.json" defines the "button -> action" mapping
  // - vtkOpenVRInteractorStyle defines the "action -> eventId" mapping
  rwi->AddAction("/actions/vtk/in/TriggerAction", /*isAnalog=*/false,
                 [](vtkEventData* vtkNotUsed(ed)) { });
  rwi->AddAction("/actions/vtk/in/ComplexGestureAction", /*isAnalog=*/false,
                 [](vtkEventData* vtkNotUsed(ed)) { });
}

//-----------------------------------------------------------------------------
std::string vtkSlicerVirtualRealityLogic::ComputeActionManifestPath(vtkMRMLVirtualRealityViewNode::XRRuntimeType xrRuntime)
{
  std::string actionManifestPath =
      Self::ComputeActionManifestPath(this->GetModuleShareDirectory(), xrRuntime, this->ModuleInstalled);

  std::string actionManifestCollapsedPath = vtksys::SystemTools::CollapseFullPath(actionManifestPath);

  if (!vtksys::SystemTools::FileExists(actionManifestCollapsedPath))
  {
    vtkErrorMacro(<< "ComputeActionManifestPath: Action manifest path set for "
                  << vtkMRMLVirtualRealityViewNode::GetXRRuntimeAsString(xrRuntime) << " runtime does not exist\n"
                  << "           actionManifestPath: " << actionManifestPath
                  << "  actionManifestCollapsedPath: " << actionManifestCollapsedPath);
  }

  return actionManifestCollapsedPath + "/";
}

//-----------------------------------------------------------------------------
std::string vtkSlicerVirtualRealityLogic::ComputeActionManifestPath(
    const std::string& moduleShareDirectory, vtkMRMLVirtualRealityViewNode::XRRuntimeType xrRuntime, bool installed)
{
  std::string actionManifestPath;

  if(installed)
  {
    // Since the output of vtkSlicerModuleLogic::GetModuleShareDirectory() is
    //
    //   <extension-install-dir>/<extension-name>/share/Slicer-X.Y/qt-loadable-modules/<module-name>
    //
    // and the action manifest files are in this directory
    //
    //   <extension-install-dir>/<extension-name>/<thirdparty-lib-dir>/<actions-subdir>
    //
    // where
    //
    //   <actions-subdir> is the "*_actions" sub-directory hard-coded in "VTK/Rendering/(OpenVR|OpenXR)/CMakeLists.txt"
    //
    //   <thirdparty-lib-dir> corresponds to the Slicer_THIRDPARTY_LIB_DIR macro (e.g "lib/Slicer-X.Y") which
    //   corresponding CMake option is used in "External_vtkRenderingOpen(VR|XR).cmake"
    //
    // we compose the path as such:

    // First, we retrieve <module-share-dir>

    // ... then we change the directory to <extension-name>/<thirdparty-lib-dir>/<actions-subdir>/
    switch (xrRuntime)
    {
      case vtkMRMLVirtualRealityViewNode::OpenVR:
        actionManifestPath = moduleShareDirectory + "/../../../../" Slicer_THIRDPARTY_LIB_DIR "/vr_actions/";
        break;
      default:
        vtkErrorWithObjectMacro(nullptr, << "ComputeActionManifestPath: No install tree action manifest path set for"
                                << vtkMRMLVirtualRealityViewNode::GetXRRuntimeAsString(xrRuntime) << " runtime");
        break;
    }
  }
  else
  {
    // Since the output of vtkSlicerModuleLogic::GetModuleShareDirectory() is
    //
    //   <extension-build-dir>/inner-build/share/Slicer-X.Y/qt-loadable-modules/<module-name>
    //
    // and the action manifest files are in this directory
    //
    //   <extension-build-dir>/<vtk-module-name>-build/<vtk-module-name>/
    //
    // we compose the path as such:

    // First, we retrieve <module-share-dir>

    // ... then we change the directory to <vtk-module-name>-build/<vtk-module-name>/
    switch (xrRuntime)
    {
      case vtkMRMLVirtualRealityViewNode::OpenVR:
        actionManifestPath = moduleShareDirectory + "/../../../../../vtkRenderingOpenVR-build/vtkRenderingOpenVR/";
        break;
      default:
        vtkErrorWithObjectMacro(nullptr, <<"ComputeActionManifestPath: No build tree action manifest path set for"
                                << vtkMRMLVirtualRealityViewNode::GetXRRuntimeAsString(xrRuntime) << " runtime");
        break;
    }
  }

  return actionManifestPath;
}
