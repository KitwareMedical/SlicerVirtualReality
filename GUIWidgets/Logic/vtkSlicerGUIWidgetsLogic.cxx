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

// GUIWidgets Logic includes
#include "vtkSlicerGUIWidgetsLogic.h"

// GUIWidgets MRML includes
#include "vtkMRMLGUIWidgetNode.h"
#include "vtkMRMLGUIWidgetDisplayNode.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLSelectionNode.h>

// Markups logic includes
#include <vtkSlicerMarkupsLogic.h>

// VTK includes
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// STD includes
#include <cassert>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerGUIWidgetsLogic);

//----------------------------------------------------------------------------
vtkSlicerGUIWidgetsLogic::vtkSlicerGUIWidgetsLogic()
{
}

//----------------------------------------------------------------------------
vtkSlicerGUIWidgetsLogic::~vtkSlicerGUIWidgetsLogic()
{
}

//----------------------------------------------------------------------------
void vtkSlicerGUIWidgetsLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkSlicerGUIWidgetsLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}

//---------------------------------------------------------------------------
void vtkSlicerGUIWidgetsLogic::ObserveMRMLScene()
{
  if (!this->GetMRMLScene())
  {
    return;
  }

  vtkMRMLApplicationLogic* mrmlAppLogic = this->GetMRMLApplicationLogic();
  if (!mrmlAppLogic)
  {
    vtkErrorMacro("ObserveMRMLScene: invalid MRML Application Logic");
    return;
  }

  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(this->GetSelectionNodeID().c_str());
  if (!node)
  {
    vtkErrorMacro("Observe MRMLScene: invalid Selection Node");
    return;
  }

  // add known markup types to the selection node
  vtkMRMLSelectionNode* selectionNode = vtkMRMLSelectionNode::SafeDownCast(node);
  if (selectionNode)
  {
    // got into batch process mode so that an update on the mouse mode tool
    // bar is triggered when leave it
    this->GetMRMLScene()->StartState(vtkMRMLScene::BatchProcessState);

    auto guiWidgetNode = vtkSmartPointer<vtkMRMLGUIWidgetNode>::New();
    selectionNode->AddNewPlaceNodeClassNameToList(
      guiWidgetNode->GetClassName(), guiWidgetNode->GetAddIcon(), guiWidgetNode->GetMarkupType());

    // trigger an update on the mouse mode toolbar
    this->GetMRMLScene()->EndState(vtkMRMLScene::BatchProcessState);
  }

 this->Superclass::ObserveMRMLScene();
}

//-----------------------------------------------------------------------------
void vtkSlicerGUIWidgetsLogic::RegisterNodes()
{
  vtkMRMLScene* scene = this->GetMRMLScene(); 
  if (!scene)
  {
    vtkErrorMacro("RegisterNodes: Invalid MRML scene");
    return;
  }
  if (!scene->IsNodeClassRegistered("vtkMRMLGUIWidgetNode"))
  {
    scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLGUIWidgetNode>::New());
  }
  if (!scene->IsNodeClassRegistered("vtkMRMLGUIWidgetDisplayNode"))
  {
    scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLGUIWidgetDisplayNode>::New());
  }
}

//---------------------------------------------------------------------------
void vtkSlicerGUIWidgetsLogic::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);
}

//---------------------------------------------------------------------------
void vtkSlicerGUIWidgetsLogic::OnMRMLSceneNodeAdded(vtkMRMLNode* vtkNotUsed(node))
{
}

//---------------------------------------------------------------------------
void vtkSlicerGUIWidgetsLogic::OnMRMLSceneNodeRemoved(vtkMRMLNode* vtkNotUsed(node))
{
}
