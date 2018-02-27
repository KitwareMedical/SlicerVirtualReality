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
#include "vtkSlicerVirtualRealityLogic.h"
#include "vtkMRMLVirtualRealityViewNode.h"

// MRML includes
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
  // can be overridden with deafult node properties defined in the scene.
  vtkMRMLVirtualRealityViewNode* vrViewNode = vtkMRMLVirtualRealityViewNode::SafeDownCast(
    this->GetMRMLScene()->CreateNodeByClass("vtkMRMLVirtualRealityViewNode"));
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
    this->ActiveViewNode->SetActive(1);
  }
  else
  {
    if (this->ActiveViewNode != NULL)
    {
      this->ActiveViewNode->SetActive(0);
    }
  }
}
