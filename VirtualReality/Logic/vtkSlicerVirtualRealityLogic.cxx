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
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// STD includes
#include <cassert>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerVirtualRealityLogic);

//----------------------------------------------------------------------------
vtkSlicerVirtualRealityLogic::vtkSlicerVirtualRealityLogic()
  : ActiveViewNode(NULL)
{
}

//----------------------------------------------------------------------------
vtkSlicerVirtualRealityLogic::~vtkSlicerVirtualRealityLogic()
{
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
  if (this->ActiveViewNode)
  {
    // There is already a usable VR node, return that
    return this->ActiveViewNode;
  }

  // Create VirtualReality view node. Use CreateNodeByClass so that node properties
  // can be overridden with default node properties defined in the scene.
  vtkSmartPointer<vtkMRMLVirtualRealityViewNode> vrViewNode = vtkSmartPointer<vtkMRMLVirtualRealityViewNode>::Take(
    vtkMRMLVirtualRealityViewNode::SafeDownCast(this->GetMRMLScene()->CreateNodeByClass("vtkMRMLVirtualRealityViewNode")) );
  // We create the node as a singleton to make sure there is only one VR view node in the scene.
  vrViewNode->SetSingletonTag("Active");
  // If a singleton node by that name exists already then it is overwritten
  // and pointer of that node is returned.
  vrViewNode = vtkMRMLVirtualRealityViewNode::SafeDownCast(this->GetMRMLScene()->AddNode(vrViewNode));

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

  // We create or get and update the linear transform nodes that correspond to left and right controllers
  vtkSmartPointer<vtkMRMLLinearTransformNode> linearTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(this->GetMRMLScene()->GetSingletonNode("VirtualReality.LeftController", "vtkMRMLLinearTransformNode"));
  if (linearTransformNode == NULL)
  {
    linearTransformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::Take(
      vtkMRMLLinearTransformNode::SafeDownCast(this->GetMRMLScene()->CreateNodeByClass("vtkMRMLLinearTransformNode")));
    linearTransformNode->SetSingletonTag("VirtualReality.LeftController");
    linearTransformNode->SetName("VirtualReality.LeftController");
    this->GetMRMLScene()->AddNode(linearTransformNode);
  }
  this->ActiveViewNode->SetAndObserveLeftControllerTransformNode(linearTransformNode);

  // Right
  linearTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(this->GetMRMLScene()->GetSingletonNode("VirtualReality.RightController", "vtkMRMLLinearTransformNode"));
  if (linearTransformNode == NULL)
  {
    linearTransformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::Take(
      vtkMRMLLinearTransformNode::SafeDownCast(this->GetMRMLScene()->CreateNodeByClass("vtkMRMLLinearTransformNode")));
    linearTransformNode->SetSingletonTag("VirtualReality.RightController");
    linearTransformNode->SetName("VirtualReality.RightController");
    this->GetMRMLScene()->AddNode(linearTransformNode);
  }
  this->ActiveViewNode->SetAndObserveRightControllerTransformNode(linearTransformNode);

  this->Modified();
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
  if (connect)
  {
    if (!this->ActiveViewNode)
    {
      vtkMRMLVirtualRealityViewNode* newViewNode = this->AddVirtualRealityViewNode();
      this->SetActiveViewNode(newViewNode);
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
void vtkSlicerVirtualRealityLogic::SetVirtualRealityActive(bool activate)
{
  if (activate)
  {
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
