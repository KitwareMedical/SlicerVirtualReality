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

// VirtualReality Logic includes
#include "vtkMRMLVirtualRealityViewNode.h"
#include "vtkSlicerVirtualRealityLogic.h"

// MRML includes
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLModelDisplayNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSegmentationDisplayNode.h>

// Slicer includes
#include "vtkSlicerVolumeRenderingLogic.h"

// VTK includes
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// STD includes
#include <cassert>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerVirtualRealityLogic);
vtkCxxSetObjectMacro(vtkSlicerVirtualRealityLogic, VolumeRenderingLogic, vtkSlicerVolumeRenderingLogic);

//----------------------------------------------------------------------------
vtkSlicerVirtualRealityLogic::vtkSlicerVirtualRealityLogic()
  : ActiveViewNode(NULL)
  , VolumeRenderingLogic(NULL)
{
}

//----------------------------------------------------------------------------
vtkSlicerVirtualRealityLogic::~vtkSlicerVirtualRealityLogic()
{
  this->SetVolumeRenderingLogic(NULL);
}

//----------------------------------------------------------------------------
void vtkSlicerVirtualRealityLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkSlicerVirtualRealityLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
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
  vtkMRMLVirtualRealityViewNode* vrViewNode = NULL;
  if (this->GetMRMLScene())
  {
    vrViewNode = vtkMRMLVirtualRealityViewNode::SafeDownCast(
      this->GetMRMLScene()->GetSingletonNode("vtkMRMLVirtualRealityViewNode", "Active"));
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
    this->SetActiveViewNode(NULL);
  }
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
    return NULL;
  }

  if (this->ActiveViewNode)
  {
    // There is already a usable VR node, return that
    return this->ActiveViewNode;
  }
  
  // Create VirtualReality view node. Use CreateNodeByClass so that node properties
  // can be overridden with default node properties defined in the scene.
  vtkSmartPointer<vtkMRMLVirtualRealityViewNode> vrViewNode = vtkSmartPointer<vtkMRMLVirtualRealityViewNode>::Take(
    vtkMRMLVirtualRealityViewNode::SafeDownCast(scene->CreateNodeByClass("vtkMRMLVirtualRealityViewNode")) );
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

  if (this->ActiveViewNode == NULL)
  {
    this->Modified();
    return;
  }
}

//-----------------------------------------------------------------------------
void vtkSlicerVirtualRealityLogic::ProcessMRMLNodesEvents(vtkObject *caller, unsigned long event, void *vtkNotUsed(callData))
{
  if (caller == this->ActiveViewNode && event == vtkCommand::ModifiedEvent)
  {
    this->Modified();
  }
}

//-----------------------------------------------------------------------------
void vtkSlicerVirtualRealityLogic::SetVirtualRealityConnected(bool connect)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("SetVirtualRealityConnected: Invalid MRML scene");
    return;
  }

  if (connect)
  {
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
    if (this->ActiveViewNode)
    {
      this->ActiveViewNode->SetVisibility(1);
    }
    else
    {
      vtkErrorMacro("Failed to create virtual reality view node");
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
    if ( this->GetVirtualRealityConnected()
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
    if (this->ActiveViewNode != NULL)
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

//---------------------------------------------------------------------------
void vtkSlicerVirtualRealityLogic::SetDefaultReferenceView()
{
  if (!this->ActiveViewNode)
  {
    return;
  }
  if (this->ActiveViewNode->GetReferenceViewNode() != NULL)
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
  vtkMRMLViewNode* viewNode = 0;
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
  vtkMRMLScene *scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("GetDefaultVirtualRealityViewNode failed: invalid scene");
    return NULL;
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
  vtkMRMLScene *scene = this->GetMRMLScene();
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
  for ( std::vector<vtkMRMLNode*>::iterator mdIt=modelDisplayNodes.begin();
    mdIt!=modelDisplayNodes.end(); ++mdIt )
  {
    vtkMRMLModelDisplayNode* modelDisplayNode = vtkMRMLModelDisplayNode::SafeDownCast(*mdIt);
    modelDisplayNode->SetBackfaceCulling(0);
    modelDisplayNode->SetSliceIntersectionVisibility(0);
  }

  std::vector<vtkMRMLNode*> segmentationDisplayNodes;
  scene->GetNodesByClass("vtkMRMLSegmentationDisplayNode", segmentationDisplayNodes);
  for ( std::vector<vtkMRMLNode*>::iterator sdIt=segmentationDisplayNodes.begin();
    sdIt!=segmentationDisplayNodes.end(); ++sdIt )
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
  vtkMRMLModelDisplayNode::SafeDownCast(defaultModelDisplayNode)->SetSliceIntersectionVisibility(0);

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
