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

  This file was originally developed by Csaba Pinter, EBATINCA, S.L., and
  development was supported by "ICEX Espana Exportacion e Inversiones" under
  the program "Inversiones de Empresas Extranjeras en Actividades de I+D
  (Fondo Tecnologico)- Convocatoria 2021"

==============================================================================*/

#include "vtkMRMLGUIWidgetsDisplayableManager.h"

#include "vtkMRMLAbstractSliceViewDisplayableManager.h"

// GUI widgets includes
#include <vtkMRMLGUIWidgetDisplayNode.h>
#include <vtkMRMLGUIWidgetNode.h>

// MarkupsModule/Logic includes
//#include <vtkSlicerMarkupsLogic.h>

// MRMLDisplayableManager includes
#include <vtkMRMLDisplayableManagerGroup.h>
#include <vtkMRMLInteractionEventData.h>

// MRML includes
#include <vtkEventBroker.h>
#include <vtkMRMLApplicationLogic.h>
#include <vtkMRMLFolderDisplayNode.h>
#include <vtkMRMLInteractionNode.h>
#include <vtkMRMLScene.h>
//#include <vtkMRMLSelectionNode.h>
//#include <vtkMRMLSliceLogic.h>
//#include <vtkMRMLSliceNode.h>
#include <vtkMRMLTransformNode.h>
#include <vtkMRMLViewNode.h>

// VTK includes
#include <vtkAbstractWidget.h>
#include <vtkCallbackCommand.h>
//#include <vtkCamera.h>
#include <vtkEvent.h>
//#include <vtkGeneralTransform.h>
//#include <vtkMarkupsGlyphSource2D.h>
//#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
//#include <vtkPropCollection.h>
//#include <vtkProperty2D.h>
//#include <vtkRendererCollection.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
//#include <vtkRenderWindowInteractor.h>
//#include <vtkSlicerMarkupsWidgetRepresentation2D.h>
//#include <vtkSlicerMarkupsWidget.h>
//#include <vtkSphereSource.h>
//#include <vtkTextProperty.h>
//#include <vtkWidgetRepresentation.h>

// STD includes
//#include <algorithm>
//#include <map>
//#include <vector>
//#include <sstream>
//#include <string>

//typedef void (*fp)();

//#define NUMERIC_ZERO 0.001

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLGUIWidgetsDisplayableManager);

//---------------------------------------------------------------------------
class vtkMRMLGUIWidgetsDisplayableManager::vtkInternal
{
public:
  vtkInternal(vtkMRMLGUIWidgetsDisplayableManager* external);
  ~vtkInternal();

  //struct Pipeline
  //  {

  //  Pipeline()
  //    {
  //    this->Actor = vtkSmartPointer<vtkActor>::New();
  //    vtkNew<vtkPolyDataMapper> mapper;
  //    mapper->SetScalarVisibility(false); // ignore any scalars that an input mesh may contain
  //    this->Actor->SetMapper(mapper.GetPointer());
  //    this->Actor->SetVisibility(false);

  //    this->NodeToWorldTransform = vtkSmartPointer<vtkGeneralTransform>::New();
  //    this->ModelWarper = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  //    this->ModelWarper->SetTransform(this->NodeToWorldTransform);
  //    mapper->SetInputConnection(this->ModelWarper->GetOutputPort());
  //    }

  //  vtkSmartPointer<vtkActor> Actor;
  //  vtkSmartPointer<vtkGeneralTransform> NodeToWorldTransform;
  //  vtkSmartPointer<vtkTransformPolyDataFilter> ModelWarper;
  //  };

  //typedef std::map<std::string, const Pipeline*> PipelineMapType; // first: segment ID; second: display pipeline
  //typedef std::map<vtkMRMLSegmentationDisplayNode*, PipelineMapType> PipelinesCacheType;
  //PipelinesCacheType DisplayPipelines;

  //typedef std::map < vtkMRMLSegmentationNode*, std::set< vtkMRMLSegmentationDisplayNode* > > SegmentationToDisplayCacheType;
  //SegmentationToDisplayCacheType SegmentationToDisplayNodes;

  /// Get widget for a given display node
  vtkSlicerQWidgetWidget* GetWidget(vtkMRMLGUIWidgetDisplayNode* guiWidgetDisplayNode);
  /// Get first visible widget for this markup
  vtkSlicerQWidgetWidget* GetWidget(vtkMRMLGUIWidgetNode* guiWidgetNode);

  /// Remove all widgets, intersection widgets, nodes
  void RemoveAllWidgetsAndNodes();

  void AddGUIWidgetNode(vtkMRMLGUIWidgetNode* node);
  void RemoveGUIWidgetNode(vtkMRMLGUIWidgetNode* node);
  void AddDisplayNode(vtkMRMLGUIWidgetDisplayNode* displayNode);
  void RemoveDisplayNode(vtkMRMLGUIWidgetDisplayNode* displayNode);

  void DeleteWidget(vtkSlicerQWidgetWidget* widget);

  void AddObservations(vtkMRMLGUIWidgetNode* node);
  void RemoveObservations(vtkMRMLGUIWidgetNode* node);

public:
  /// Picker of segment prop in renderer
  //vtkSmartPointer<vtkCellPicker> CellPicker;

  /// Last picked segmentation display node ID
  //std::string PickedDisplayNodeID;

  /// Map of vtkWidget indexed using associated node ID
  typedef std::map<vtkSmartPointer<vtkMRMLGUIWidgetDisplayNode>, vtkSlicerQWidgetWidget* > DisplayNodeToWidgetType;
  typedef std::map<vtkSmartPointer<vtkMRMLGUIWidgetDisplayNode>, vtkSlicerQWidgetWidget* >::iterator DisplayNodeToWidgetIt;
  DisplayNodeToWidgetType GUIWidgetDisplayNodesToWidgets;  // display nodes with widgets assigned

  typedef std::set<vtkSmartPointer<vtkMRMLGUIWidgetNode> > GUIWidgetNodesType;
  typedef std::set<vtkSmartPointer<vtkMRMLGUIWidgetNode> >::iterator GUIWidgetNodesIt;
  GUIWidgetNodesType GUIWidgetNodes; // observed GUI widget nodes

private:
  vtkMRMLGUIWidgetsDisplayableManager* External;

  bool AddingGUIWidgetNode;

  std::vector<unsigned long> ObservedGuiWidgetNodeEvents;
};

//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkMRMLGUIWidgetsDisplayableManager::vtkInternal::vtkInternal(vtkMRMLGUIWidgetsDisplayableManager* external)
: External(external)
, AddingGUIWidgetNode(false)
//, AddingSegmentationNode(false)
{
  //this->CellPicker = vtkSmartPointer<vtkCellPicker>::New();
  //this->CellPicker->SetTolerance(0.00001);
}

//---------------------------------------------------------------------------
vtkMRMLGUIWidgetsDisplayableManager::vtkInternal::~vtkInternal()
{
//  this->ClearDisplayableNodes();
}

//---------------------------------------------------------------------------
vtkSlicerQWidgetWidget* vtkMRMLGUIWidgetsDisplayableManager::vtkInternal::GetWidget(vtkMRMLGUIWidgetNode* guiWidgetNode)
{
  if (!guiWidgetNode)
  {
    return nullptr;
  }
  vtkMRMLGUIWidgetsDisplayableManager::vtkInternal::GUIWidgetNodesIt displayableIt = this->GUIWidgetNodes.find(guiWidgetNode);
  if (displayableIt == this->GUIWidgetNodes.end())
  {
    // we do not manage this markup
    return nullptr;
  }

  // Return first widget found for a GUI widget node
  for (vtkMRMLGUIWidgetsDisplayableManager::vtkInternal::DisplayNodeToWidgetIt widgetIterator = this->GUIWidgetDisplayNodesToWidgets.begin();
    widgetIterator != this->GUIWidgetDisplayNodesToWidgets.end(); ++widgetIterator)
  {
    vtkMRMLGUIWidgetDisplayNode* guiWidgetDisplayNode = widgetIterator->first;
    if (guiWidgetDisplayNode->GetDisplayableNode() == guiWidgetNode)
    {
      return widgetIterator->second;
    }
  }

  return nullptr;
}

//---------------------------------------------------------------------------
vtkSlicerQWidgetWidget* vtkMRMLGUIWidgetsDisplayableManager::vtkInternal::GetWidget(vtkMRMLGUIWidgetDisplayNode* node)
{
  if (!node)
  {
    return nullptr;
  }

  // Make sure the map contains a vtkWidget associated with this node
  DisplayNodeToWidgetIt it = this->GUIWidgetDisplayNodesToWidgets.find(node);
  if (it == this->GUIWidgetDisplayNodesToWidgets.end())
  {
    return nullptr;
  }

  return it->second;
}

//---------------------------------------------------------------------------
void vtkMRMLGUIWidgetsDisplayableManager::vtkInternal::AddGUIWidgetNode(vtkMRMLGUIWidgetNode* node)
{
  if (!node)
  {
    return;
  }
  vtkMRMLAbstractViewNode* viewNode = vtkMRMLAbstractViewNode::SafeDownCast(this->External->GetMRMLDisplayableNode());
  if (!viewNode)
  {
    return;
  }

  if (this->AddingGUIWidgetNode)
  {
    return;
  }
  this->AddingGUIWidgetNode = true;

  this->AddObservations(node);
  this->GUIWidgetNodes.insert(node);

  // Add Display Nodes
  int nnodes = node->GetNumberOfDisplayNodes();
  for (int i = 0; i<nnodes; i++)
  {
    vtkMRMLGUIWidgetDisplayNode* displayNode = vtkMRMLGUIWidgetDisplayNode::SafeDownCast(node->GetNthDisplayNode(i));

    // Check whether DisplayNode should be shown in this view
    if (!displayNode
      || !displayNode->IsA("vtkMRMLGUIWidgetDisplayNode")
      || !displayNode->IsDisplayableInView(viewNode->GetID()))
    {
      continue;
    }

    this->AddDisplayNode(displayNode);
  }

  this->AddingGUIWidgetNode = false;
}

//---------------------------------------------------------------------------
void vtkMRMLGUIWidgetsDisplayableManager::vtkInternal::RemoveGUIWidgetNode(vtkMRMLGUIWidgetNode* node)
{
  if (!node)
  {
    return;
  }

  vtkMRMLGUIWidgetsDisplayableManager::vtkInternal::GUIWidgetNodesIt displayableIt =
    this->GUIWidgetNodes.find(node);

  if (displayableIt == this->GUIWidgetNodes.end())
  {
    // we do not manage this markup
    return;
  }

  // Remove display nodes corresponding to this GUI widget node
  for (vtkMRMLGUIWidgetsDisplayableManager::vtkInternal::DisplayNodeToWidgetIt widgetIterator = this->GUIWidgetDisplayNodesToWidgets.begin();
    widgetIterator != this->GUIWidgetDisplayNodesToWidgets.end();
    /*upon deletion the increment is done already, so don't increment here*/)
  {
    vtkMRMLGUIWidgetDisplayNode* guiWidgetDisplayNode = widgetIterator->first;
    if (guiWidgetDisplayNode->GetDisplayableNode() != node)
    {
      ++widgetIterator;
    }
    else
    {
      // display node of the node that is being removed
      vtkMRMLGUIWidgetsDisplayableManager::vtkInternal::DisplayNodeToWidgetIt widgetIteratorToRemove = widgetIterator;
      ++widgetIterator;
      vtkSlicerQWidgetWidget* widgetToRemove = widgetIteratorToRemove->second;
      this->DeleteWidget(widgetToRemove);
      this->GUIWidgetDisplayNodesToWidgets.erase(widgetIteratorToRemove);
    }
  }

  this->RemoveObservations(node);
  this->GUIWidgetNodes.erase(displayableIt);
}

//---------------------------------------------------------------------------
void vtkMRMLGUIWidgetsDisplayableManager::vtkInternal::AddDisplayNode(vtkMRMLGUIWidgetDisplayNode* guiWidgetDisplayNode)
{
  if (!guiWidgetDisplayNode)
  {
    return;
  }

  // Do not add the display node if displayNodeIt is already associated with a widget object.
  // This happens when a segmentation node already associated with a display node
  // is copied into an other (using vtkMRMLNode::Copy()) and is added to the scene afterward.
  // Related issue are #3428 and #2608
  vtkMRMLGUIWidgetsDisplayableManager::vtkInternal::DisplayNodeToWidgetIt displayNodeIt
    = this->GUIWidgetDisplayNodesToWidgets.find(guiWidgetDisplayNode);
  if (displayNodeIt != this->GUIWidgetDisplayNodesToWidgets.end())
  {
    return;
  }

  // There should not be a widget for the new node
  if (this->GetWidget(guiWidgetDisplayNode) != nullptr)
  {
    vtkErrorWithObjectMacro(this->External, "vtkMRMLGUIWidgetsDisplayableManager: A widget is already associated to this node");
    return;
  }

  vtkSlicerQWidgetWidget* newWidget = this->External->CreateWidget(guiWidgetDisplayNode);
  if (!newWidget)
  {
    vtkErrorWithObjectMacro(this->External, "vtkMRMLGUIWidgetsDisplayableManager: Failed to create widget");
    return;
  }

  // record the mapping between node and widget in the helper
  this->GUIWidgetDisplayNodesToWidgets[guiWidgetDisplayNode] = newWidget;

  // Build representation
  newWidget->UpdateFromMRML(guiWidgetDisplayNode, 0); // no specific event triggers full rebuild

  this->External->RequestRender();

  // Update cached matrices. Calls UpdateWidget
  //this->UpdateDisplayableTransforms(mNode);
}

//---------------------------------------------------------------------------
void vtkMRMLGUIWidgetsDisplayableManager::vtkInternal::RemoveDisplayNode(vtkMRMLGUIWidgetDisplayNode* guiWidgetDisplayNode)
{
  if (!guiWidgetDisplayNode)
  {
    return;
  }

  vtkMRMLGUIWidgetsDisplayableManager::vtkInternal::DisplayNodeToWidgetIt displayNodeIt
    = this->GUIWidgetDisplayNodesToWidgets.find(guiWidgetDisplayNode);
  if (displayNodeIt == this->GUIWidgetDisplayNodesToWidgets.end())
  {
    // no widget found for this display node
    return;
  }

  vtkSlicerQWidgetWidget* widget = (displayNodeIt->second);
  this->DeleteWidget(widget);

  this->GUIWidgetDisplayNodesToWidgets.erase(guiWidgetDisplayNode);
}

//---------------------------------------------------------------------------
void vtkMRMLGUIWidgetsDisplayableManager::vtkInternal::RemoveAllWidgetsAndNodes()
{
  DisplayNodeToWidgetIt guiWidgetIterator = this->GUIWidgetDisplayNodesToWidgets.begin();
  for (guiWidgetIterator = this->GUIWidgetDisplayNodesToWidgets.begin();
    guiWidgetIterator != this->GUIWidgetDisplayNodesToWidgets.end(); ++guiWidgetIterator)
  {
    guiWidgetIterator->second->Delete();
  }
  this->GUIWidgetDisplayNodesToWidgets.clear();

  GUIWidgetNodesIt guiWidgetNodeIterator = this->GUIWidgetNodes.begin();
  for (guiWidgetNodeIterator = this->GUIWidgetNodes.begin();
    guiWidgetNodeIterator != this->GUIWidgetNodes.end(); ++guiWidgetNodeIterator)
  {
    this->RemoveObservations(*guiWidgetNodeIterator);
  }
}

//---------------------------------------------------------------------------
void vtkMRMLGUIWidgetsDisplayableManager::vtkInternal::DeleteWidget(vtkSlicerQWidgetWidget* widget)
{
  if (!widget)
  {
    return;
  }
  widget->SetRenderer(nullptr);
  widget->SetRepresentation(nullptr);
  widget->Delete();
}

//---------------------------------------------------------------------------
void vtkMRMLGUIWidgetsDisplayableManager::vtkInternal::AddObservations(vtkMRMLGUIWidgetNode* node)
{
  vtkCallbackCommand* callbackCommand = this->External->GetMRMLNodesCallbackCommand();
  vtkEventBroker* broker = vtkEventBroker::GetInstance();
  for (auto observedMarkupNodeEvent : this->ObservedGuiWidgetNodeEvents)
  {
    if (!broker->GetObservationExist(node, observedMarkupNodeEvent, this->External, callbackCommand))
    {
      broker->AddObservation(node, observedMarkupNodeEvent, this->External, callbackCommand);
    }
  }
}

//---------------------------------------------------------------------------
void vtkMRMLGUIWidgetsDisplayableManager::vtkInternal::RemoveObservations(vtkMRMLGUIWidgetNode* node)
{
  vtkCallbackCommand* callbackCommand = this->External->GetMRMLNodesCallbackCommand();
  vtkEventBroker* broker = vtkEventBroker::GetInstance();
  for (auto observedMarkupNodeEvent : this->ObservedGuiWidgetNodeEvents)
  {
    vtkEventBroker::ObservationVector observations;
    observations = broker->GetObservations(node, observedMarkupNodeEvent, this->External, callbackCommand);
    broker->RemoveObservations(observations);
  }
}


//---------------------------------------------------------------------------
// vtkMRMLGUIWidgetsDisplayableManager methods

//---------------------------------------------------------------------------
vtkMRMLGUIWidgetsDisplayableManager::vtkMRMLGUIWidgetsDisplayableManager()
{
  this->Internal = new vtkInternal(this);

  /*
  this->Internal = vtkSmartPointer<vtkMRMLGUIWidgetsDisplayableManagerHelper>::New();
  this->Internal->SetDisplayableManager(this);
  this->DisableInteractorStyleEventsProcessing = 0;

  this->LastClickWorldCoordinates[0]=0.0;
  this->LastClickWorldCoordinates[1]=0.0;
  this->LastClickWorldCoordinates[2]=0.0;
  this->LastClickWorldCoordinates[3]=1.0;
  */
}

//---------------------------------------------------------------------------
vtkMRMLGUIWidgetsDisplayableManager::~vtkMRMLGUIWidgetsDisplayableManager()
{
  //this->DisableInteractorStyleEventsProcessing = 0;

  delete this->Internal;
  this->Internal = nullptr;
}

//---------------------------------------------------------------------------
void vtkMRMLGUIWidgetsDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  /*
  os << indent << "DisableInteractorStyleEventsProcessing = " << this->DisableInteractorStyleEventsProcessing << std::endl;
  if (this->SliceNode &&
      this->SliceNode->GetID())
    {
    os << indent << "Slice node id = " << this->SliceNode->GetID() << std::endl;
    }
  else
    {
    os << indent << "No slice node" << std::endl;
    }
  */
}
/*
//---------------------------------------------------------------------------
vtkMRMLSliceNode * vtkMRMLGUIWidgetsDisplayableManager::GetMRMLSliceNode()
{
  return vtkMRMLSliceNode::SafeDownCast(this->GetMRMLDisplayableNode());
}

//---------------------------------------------------------------------------
bool vtkMRMLGUIWidgetsDisplayableManager::Is2DDisplayableManager()
{
  return this->GetMRMLSliceNode() != nullptr;
}
*/
//---------------------------------------------------------------------------
void vtkMRMLGUIWidgetsDisplayableManager::RequestRender()
{
  if (!this->GetMRMLScene())
    {
    return;
    }
  if (!this->GetMRMLScene()->IsBatchProcessing())
    {
    this->Superclass::RequestRender();
    }
}

//---------------------------------------------------------------------------
void vtkMRMLGUIWidgetsDisplayableManager::UpdateFromMRML()
{
  // this gets called from RequestRender, so make sure to jump out quickly if possible
  if (this->GetMRMLScene() == nullptr)
  {
    return;
  }

  // turn off update from mrml requested, as we're doing it now, and create
  // widget requests a render which checks this flag before calling update
  // from mrml again
  this->SetUpdateFromMRMLRequested(false);

  std::vector<vtkMRMLNode*> guiWidgetNodes;
  this->GetMRMLScene()->GetNodesByClass("vtkMRMLGUIWidgetNode", guiWidgetNodes);
  for (std::vector< vtkMRMLNode* >::iterator nodeIt = guiWidgetNodes.begin(); nodeIt != guiWidgetNodes.end(); ++nodeIt)
  {
    vtkMRMLGUIWidgetNode* guiWidgetNode = vtkMRMLGUIWidgetNode::SafeDownCast(*nodeIt);
    if (!guiWidgetNode)
    {
      continue;
    }
    if (this->Internal->GUIWidgetNodes.find(guiWidgetNode) != this->Internal->GUIWidgetNodes.end())
    {
      // node added already
      continue;
    }
    this->Internal->AddGUIWidgetNode(guiWidgetNode);
  }

  std::vector<vtkMRMLNode*> guiWidgetDisplayNodes;
  this->GetMRMLScene()->GetNodesByClass("vtkMRMLGUIWidgetDisplayNode", guiWidgetDisplayNodes);
  for (std::vector< vtkMRMLNode* >::iterator nodeIt = guiWidgetDisplayNodes.begin();
    nodeIt != guiWidgetDisplayNodes.end(); ++nodeIt)
  {
    vtkMRMLGUIWidgetDisplayNode* guiWidgetDisplayNode = vtkMRMLGUIWidgetDisplayNode::SafeDownCast(*nodeIt);
    if (!guiWidgetDisplayNode)
    {
      continue;
    }
    if (this->Internal->GUIWidgetDisplayNodesToWidgets.find(guiWidgetDisplayNode) != this->Internal->GUIWidgetDisplayNodesToWidgets.end())
    {
      // node added already
      continue;
    }
    this->Internal->AddDisplayNode(guiWidgetDisplayNode);
  }

  // Remove observed GUI widget nodes that have been deleted from the scene
  for (vtkMRMLGUIWidgetsDisplayableManager::vtkInternal::GUIWidgetNodesIt guiWidgetIterator =
    this->Internal->GUIWidgetNodes.begin(); guiWidgetIterator != this->Internal->GUIWidgetNodes.end(); )
  {
    vtkMRMLGUIWidgetNode* guiWidgetNode = *guiWidgetIterator;
    if (this->GetMRMLScene()->IsNodePresent(guiWidgetNode))
    {
      ++guiWidgetIterator;
    }
    else
    {
      // display node is not in the scene anymore, delete the widget
      vtkMRMLGUIWidgetsDisplayableManager::vtkInternal::GUIWidgetNodesIt guiWidgetIteratorToRemove = guiWidgetIterator;
      ++guiWidgetIterator;
      this->Internal->RemoveGUIWidgetNode(*guiWidgetIteratorToRemove);
      this->Internal->GUIWidgetNodes.erase(guiWidgetIteratorToRemove);
    }
  }

  // Remove widgets corresponding deleted display nodes
  for (vtkMRMLGUIWidgetsDisplayableManager::vtkInternal::DisplayNodeToWidgetIt widgetIterator = this->Internal->GUIWidgetDisplayNodesToWidgets.begin();
    widgetIterator != this->Internal->GUIWidgetDisplayNodesToWidgets.end(); )
  {
    vtkMRMLGUIWidgetDisplayNode* guiWidgetDisplayNode = widgetIterator->first;
    if (this->GetMRMLScene()->IsNodePresent(guiWidgetDisplayNode))
    {
      ++widgetIterator;
    }
    else
    {
      // display node is not in the scene anymore, delete the widget
      vtkMRMLGUIWidgetsDisplayableManager::vtkInternal::DisplayNodeToWidgetIt widgetIteratorToRemove = widgetIterator;
      ++widgetIterator;
      vtkSlicerQWidgetWidget* widgetToRemove = widgetIteratorToRemove->second;
      this->Internal->DeleteWidget(widgetToRemove);
      this->Internal->GUIWidgetDisplayNodesToWidgets.erase(widgetIteratorToRemove);
    }
  }
}

//---------------------------------------------------------------------------
void vtkMRMLGUIWidgetsDisplayableManager::SetMRMLSceneInternal(vtkMRMLScene* newScene)
{
  Superclass::SetMRMLSceneInternal(newScene);

  // after a new scene got associated, we want to make sure everything old is gone
  this->OnMRMLSceneEndClose();

  //if (newScene)
  //{
  //  this->AddObserversToInteractionNode();
  //}
  //else
  //{
  //  // there's no scene to get the interaction node from, so this won't do anything
  //  this->RemoveObserversFromInteractionNode();
  //}
  //vtkDebugMacro("SetMRMLSceneInternal: add observer on interaction node now?");

  // clear out the map of glyph types
  //this->Internal->ClearNodeGlyphTypes();
}

//---------------------------------------------------------------------------
void vtkMRMLGUIWidgetsDisplayableManager
::ProcessMRMLNodesEvents(vtkObject *caller, unsigned long event, void *callData)
{
  vtkMRMLGUIWidgetNode* guiWidgetNode = vtkMRMLGUIWidgetNode::SafeDownCast(caller);
  vtkMRMLInteractionNode* interactionNode = vtkMRMLInteractionNode::SafeDownCast(caller);
  if (guiWidgetNode)
  {
    bool renderRequested = false;

    for (int displayNodeIndex = 0; displayNodeIndex < guiWidgetNode->GetNumberOfDisplayNodes(); displayNodeIndex++)
    {
      vtkMRMLGUIWidgetDisplayNode* displayNode = vtkMRMLGUIWidgetDisplayNode::SafeDownCast(guiWidgetNode->GetNthDisplayNode(displayNodeIndex));
      vtkSlicerQWidgetWidget* widget = this->Internal->GetWidget(displayNode);
      if (!widget)
      {
        // if a new display node is added or display node view node IDs are changed then we may need to create a new widget
        this->Internal->AddDisplayNode(displayNode);
        widget = this->Internal->GetWidget(displayNode);
      }
      if (!widget)
      {
        continue;
      }
      widget->UpdateFromMRML(guiWidgetNode, event, callData);
      if (widget->GetNeedToRender())
      {
        renderRequested = true;
        widget->NeedToRenderOff();
      }
    }

    if (renderRequested)
    {
      this->RequestRender();
    }
  }
  else if (interactionNode)
  {
    if (event == vtkMRMLInteractionNode::InteractionModeChangedEvent)
    {
      // loop through all widgets and update the widget status
      for (vtkMRMLGUIWidgetsDisplayableManager::vtkInternal::DisplayNodeToWidgetIt widgetIterator = this->Internal->GUIWidgetDisplayNodesToWidgets.begin();
        widgetIterator != this->Internal->GUIWidgetDisplayNodesToWidgets.end(); ++widgetIterator)
      {
        vtkSlicerQWidgetWidget* widget = widgetIterator->second;
        if (!widget)
        {
          continue;
        }
        vtkMRMLInteractionEventData* eventData = reinterpret_cast<vtkMRMLInteractionEventData*>(callData);
        widget->Leave(eventData);
      }
    }
  }
  else
  {
    this->Superclass::ProcessMRMLNodesEvents(caller, event, callData);
  }
}

//---------------------------------------------------------------------------
void vtkMRMLGUIWidgetsDisplayableManager::OnMRMLSceneEndClose()
{
  //vtkDebugMacro("OnMRMLSceneEndClose: remove observers?");
  // run through all nodes and remove node and widget
  this->Internal->RemoveAllWidgetsAndNodes();

  this->SetUpdateFromMRMLRequested(true);
  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkMRMLGUIWidgetsDisplayableManager::OnMRMLSceneEndImport()
{
  this->SetUpdateFromMRMLRequested(true);
  this->UpdateFromMRMLScene();
  //this->Internal->SetAllWidgetsToManipulate();
  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkMRMLGUIWidgetsDisplayableManager::UpdateFromMRMLScene()
{
  if (this->GetMRMLDisplayableNode())
  {
    this->UpdateFromMRML();
  }
}

//---------------------------------------------------------------------------
void vtkMRMLGUIWidgetsDisplayableManager::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  if (!node || !this->GetMRMLScene())
  {
    return;
  }

  // if the scene is still updating, jump out
  if (this->GetMRMLScene()->IsBatchProcessing())
  {
    this->SetUpdateFromMRMLRequested(true);
    return;
  }

  if (node->IsA("vtkMRMLInteractionNode"))
  {
    //this->AddObserversToInteractionNode();
    return;
  }

  if (node->IsA("vtkMRMLGUIWidgetNode"))
  {
    this->Internal->AddGUIWidgetNode(vtkMRMLGUIWidgetNode::SafeDownCast(node));

    // and render again
    this->RequestRender();
  }
}
/*
//---------------------------------------------------------------------------
void vtkMRMLGUIWidgetsDisplayableManager::AddObserversToInteractionNode()
{
  if (!this->GetMRMLScene())
    {
    return;
    }
  // also observe the interaction node for changes
  vtkMRMLInteractionNode *interactionNode = this->GetInteractionNode();
  if (interactionNode)
    {
    vtkDebugMacro("AddObserversToInteractionNode: interactionNode found");
    vtkNew<vtkIntArray> interactionEvents;
    interactionEvents->InsertNextValue(vtkMRMLInteractionNode::InteractionModeChangedEvent);
    interactionEvents->InsertNextValue(vtkMRMLInteractionNode::InteractionModePersistenceChangedEvent);
    interactionEvents->InsertNextValue(vtkMRMLInteractionNode::EndPlacementEvent);
    vtkObserveMRMLNodeEventsMacro(interactionNode, interactionEvents.GetPointer());
    }
  else { vtkDebugMacro("AddObserversToInteractionNode: No interaction node!"); }
}

//---------------------------------------------------------------------------
void vtkMRMLGUIWidgetsDisplayableManager::RemoveObserversFromInteractionNode()
{
  if (!this->GetMRMLScene())
    {
    return;
    }

  // find the interaction node
  vtkMRMLInteractionNode *interactionNode =  this->GetInteractionNode();
  if (interactionNode)
    {
    vtkUnObserveMRMLNodeMacro(interactionNode);
    }
}
*/
//---------------------------------------------------------------------------
void vtkMRMLGUIWidgetsDisplayableManager::OnMRMLSceneNodeRemoved(vtkMRMLNode* node)
{
  bool modified = false;

  vtkMRMLGUIWidgetNode* guiWidgetNode = vtkMRMLGUIWidgetNode::SafeDownCast(node);
  if (guiWidgetNode)
  {
    this->Internal->RemoveGUIWidgetNode(guiWidgetNode);
    modified = true;
  }

  vtkMRMLGUIWidgetDisplayNode* guiWidgetDisplayNode = vtkMRMLGUIWidgetDisplayNode::SafeDownCast(node);
  if (guiWidgetDisplayNode)
  {
    this->Internal->RemoveDisplayNode(guiWidgetDisplayNode);
    modified = true;
  }

  if (modified)
  {
    this->RequestRender();
  }
}

//---------------------------------------------------------------------------
void vtkMRMLGUIWidgetsDisplayableManager::OnMRMLDisplayableNodeModifiedEvent(vtkObject* caller)
{
  vtkDebugMacro("OnMRMLDisplayableNodeModifiedEvent");

  if (!caller)
  {
    vtkErrorMacro("OnMRMLDisplayableNodeModifiedEvent: Could not get caller.");
    return;
  }

  //vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(caller);
  //if (sliceNode)
  //{
  //  // the associated renderWindow is a 2D SliceView
  //  // this is the entry point for all events fired by one of the three sliceviews
  //  // (e.g. change slice number, zoom etc.)

  //  // we remember that this instance of the displayableManager deals with 2D
  //  // this is important for widget creation etc. and save the actual SliceNode
  //  // because during Slicer startup the SliceViews fire events, it will be always set correctly
  //  this->SliceNode = sliceNode;

  //  // now we call the handle for specific sliceNode actions
  //  this->OnMRMLSliceNodeModifiedEvent();

  //  // and exit
  //  return;
  //}

  //TODO: This is a no-op. Needed?
  vtkMRMLViewNode* viewNode = vtkMRMLViewNode::SafeDownCast(caller);
  if (viewNode)
  {
    // the associated renderWindow is a 3D View
    vtkDebugMacro("OnMRMLDisplayableNodeModifiedEvent: This displayableManager handles a ThreeD view.");
    return;
  }
}
/*
//---------------------------------------------------------------------------
void vtkMRMLGUIWidgetsDisplayableManager::OnMRMLSliceNodeModifiedEvent()
{
  bool renderRequested = false;

  // run through all markup nodes in the helper
  vtkMRMLGUIWidgetsDisplayableManagerHelper::DisplayNodeToWidgetIt it
    = this->Internal->MarkupsDisplayNodesToWidgets.begin();
  while(it != this->Internal->MarkupsDisplayNodesToWidgets.end())
    {
    // we loop through all widgets
    vtkSlicerMarkupsWidget* widget = (it->second);
    widget->UpdateFromMRML(this->SliceNode, vtkCommand::ModifiedEvent);
    if (widget->GetNeedToRender())
      {
      renderRequested = true;
      widget->NeedToRenderOff();
      }
    ++it;
    }

  if (renderRequested)
  {
    this->RequestRender();
  }
}

//---------------------------------------------------------------------------
void vtkMRMLGUIWidgetsDisplayableManager::OnInteractorStyleEvent(int eventid)
{
  Superclass::OnInteractorStyleEvent(eventid);

}

//---------------------------------------------------------------------------
vtkSlicerMarkupsWidget* vtkMRMLGUIWidgetsDisplayableManager::GetWidget(vtkMRMLMarkupsDisplayNode * node)
{
  return this->Internal->GetWidget(node);
}

//---------------------------------------------------------------------------
/// Check if it is the correct displayableManager
//---------------------------------------------------------------------------
bool vtkMRMLGUIWidgetsDisplayableManager::IsCorrectDisplayableManager()
{
  vtkMRMLSelectionNode *selectionNode = this->GetMRMLApplicationLogic()->GetSelectionNode();
  if (selectionNode == nullptr)
    {
    vtkErrorMacro ("IsCorrectDisplayableManager: No selection node in the scene.");
    return false;
    }
  const char* placeNodeClassName = selectionNode->GetActivePlaceNodeClassName();
  if (!placeNodeClassName)
    {
    return false;
    }

  vtkSmartPointer<vtkMRMLNode> node =
    vtkSmartPointer<vtkMRMLNode>::Take(this->GetMRMLScene()->CreateNodeByClass(placeNodeClassName));
  vtkMRMLMarkupsNode* markupsNode = vtkMRMLMarkupsNode::SafeDownCast(node);
  if (!markupsNode)
    {
    return false;
    }

  // the purpose of the displayableManager is hardcoded
  return this->IsManageable(placeNodeClassName);

}
//---------------------------------------------------------------------------
bool vtkMRMLGUIWidgetsDisplayableManager::IsManageable(vtkMRMLNode* node)
{
  if (node == nullptr)
    {
    vtkErrorMacro("Is Manageable: invalid node.");
    return false;
    }

  return this->IsManageable(node->GetClassName());
}

//---------------------------------------------------------------------------
bool vtkMRMLGUIWidgetsDisplayableManager::IsManageable(const char* nodeClassName)
{
  if (nodeClassName == nullptr)
    {
   return false;
    }

  vtkSlicerMarkupsLogic* markupsLogic =
    vtkSlicerMarkupsLogic::SafeDownCast(this->GetMRMLApplicationLogic()->GetModuleLogic("Markups"));
  if (!markupsLogic)
    {
    vtkErrorMacro("Is Manageable: invalid Markups logic.");
    return false;
    }

  vtkSmartPointer<vtkMRMLNode> node =
    vtkSmartPointer<vtkMRMLNode>::Take(this->GetMRMLScene()->CreateNodeByClass(nodeClassName));
  vtkMRMLMarkupsNode* markupsNode = vtkMRMLMarkupsNode::SafeDownCast(node);
  if (!markupsNode)
    {
    return false;
    }

  return markupsLogic->GetWidgetByMarkupsType(markupsNode->GetMarkupType()) ? true : false;
}

//---------------------------------------------------------------------------
vtkMRMLMarkupsNode* vtkMRMLGUIWidgetsDisplayableManager::CreateNewMarkupsNode(
  const std::string &markupsNodeClassName)
{
  vtkMRMLMarkupsNode* markupsNode = vtkMRMLMarkupsNode::SafeDownCast(
    this->GetMRMLScene()->AddNewNodeByClass(markupsNodeClassName));

  std::string nodeName =
    this->GetMRMLScene()->GenerateUniqueName(markupsNode->GetDefaultNodeNamePrefix());
  markupsNode->SetName(nodeName.c_str());
  markupsNode->AddDefaultStorageNode();
  markupsNode->CreateDefaultDisplayNodes();

  return markupsNode;
}

//---------------------------------------------------------------------------
vtkSlicerMarkupsWidget* vtkMRMLGUIWidgetsDisplayableManager::FindClosestWidget(vtkMRMLInteractionEventData *callData, double &closestDistance2)
{
  vtkSlicerMarkupsWidget* closestWidget = nullptr;
  closestDistance2 = VTK_DOUBLE_MAX;

  for (vtkMRMLGUIWidgetsDisplayableManagerHelper::DisplayNodeToWidgetIt widgetIterator = this->Internal->MarkupsDisplayNodesToWidgets.begin();
    widgetIterator != this->Internal->MarkupsDisplayNodesToWidgets.end(); ++widgetIterator)
    {
    vtkSlicerMarkupsWidget* widget = widgetIterator->second;
    if (!widget)
      {
      continue;
      }
    double distance2FromWidget = VTK_DOUBLE_MAX;
    if (widget->CanProcessInteractionEvent(callData, distance2FromWidget))
      {
      if (!closestWidget || distance2FromWidget < closestDistance2)
        {
        closestDistance2 = distance2FromWidget;
        closestWidget = widget;
        }
      }
    }
  return closestWidget;
}

//---------------------------------------------------------------------------
bool vtkMRMLGUIWidgetsDisplayableManager::CanProcessInteractionEvent(vtkMRMLInteractionEventData* eventData, double &closestDistance2)
{
  vtkMRMLInteractionNode* interactionNode = this->GetInteractionNode();
  // New point can be placed anywhere
  int eventid = eventData->GetType();
  // We allow mouse move with the shift modifier to be processed while in place mode so that we can continue to update the
  // preview positionm, even when using shift + mouse-move to adjust the crosshair position.
  if ((eventid == vtkCommand::MouseMoveEvent
       && (eventData->GetModifiers() == vtkEvent::NoModifier ||
          (eventData->GetModifiers() & vtkEvent::ShiftModifier &&
           interactionNode && interactionNode->GetCurrentInteractionMode() == vtkMRMLInteractionNode::Place)))
    || eventid == vtkCommand::Move3DEvent
    //|| (eventid == vtkCommand::LeftButtonPressEvent && eventData->GetModifiers() == vtkEvent::NoModifier)
    //|| eventid == vtkCommand::LeftButtonReleaseEvent
    //|| eventid == vtkCommand::RightButtonReleaseEvent
    //|| eventid == vtkCommand::EnterEvent
    //|| eventid == vtkCommand::LeaveEvent
    )
    {
    vtkMRMLSelectionNode *selectionNode = this->GetSelectionNode();
    if (!interactionNode || !selectionNode)
      {
      return false;
      }
    if (interactionNode->GetCurrentInteractionMode() == vtkMRMLInteractionNode::Place
      && this->IsManageable(selectionNode->GetActivePlaceNodeClassName()))
      {

      // If there is a suitable markups node for placement but it is not available in current view
      // then we do not allow placement (placement would create a new markup node for this view,
      // which would probably not what users want - they would like to place using the current markups node)
      bool canPlaceInThisView = false;
      vtkMRMLMarkupsNode* markupsNode = this->GetActiveMarkupsNodeForPlacement();
      if (markupsNode)
        {
        int numberOfDisplayNodes = markupsNode->GetNumberOfDisplayNodes();
        vtkMRMLAbstractViewNode* viewNode = vtkMRMLAbstractViewNode::SafeDownCast(this->GetMRMLDisplayableNode());
        for (int displayNodeIndex = 0; displayNodeIndex < numberOfDisplayNodes; displayNodeIndex++)
          {
          vtkMRMLDisplayNode* displayNode = markupsNode->GetNthDisplayNode(displayNodeIndex);
          if (displayNode && displayNode->IsDisplayableInView(viewNode->GetID()))
            {
            canPlaceInThisView = true;
            break;
            }
          }
        }
      else
        {
        // a new markups node will be created
        canPlaceInThisView = true;
        }
      if (canPlaceInThisView)
        {
        closestDistance2 = 0.0;
        return true;
        }
      }
    }

  if (eventid == vtkCommand::LeaveEvent && this->LastActiveWidget != nullptr)
    {
    if (this->LastActiveWidget->GetMarkupsDisplayNode() && this->LastActiveWidget->GetMarkupsDisplayNode()->HasActiveComponent())
      {
      // this widget has active component, therefore leave event is relevant
      closestDistance2 = 0.0;
      return this->LastActiveWidget;
      }
    }

  // Other interactions
  bool canProcess = (this->FindClosestWidget(eventData, closestDistance2) != nullptr);

  if (!canProcess && this->LastActiveWidget != nullptr
    && (eventid == vtkCommand::MouseMoveEvent || eventid == vtkCommand::Move3DEvent) )
    {
    // interaction context (e.g. mouse) is moved away from the widget -> deactivate if it's the same context that activated it
    std::vector<std::string> contextsWithActiveComponents =
      this->LastActiveWidget->GetMarkupsDisplayNode()->GetActiveComponentInteractionContexts();
    if (std::find(contextsWithActiveComponents.begin(), contextsWithActiveComponents.end(), eventData->GetInteractionContextName())
        != contextsWithActiveComponents.end() )
      {
      this->LastActiveWidget->Leave(eventData);
      this->LastActiveWidget = nullptr;
      }
    }

  return canProcess;
}

//---------------------------------------------------------------------------
bool vtkMRMLGUIWidgetsDisplayableManager::ProcessInteractionEvent(vtkMRMLInteractionEventData* eventData)
{
  if (this->GetDisableInteractorStyleEventsProcessing())
    {
    return false;
    }
  int eventid = eventData->GetType();

  if (eventid == vtkCommand::LeaveEvent)
    {
    if (this->LastActiveWidget != nullptr)
      {
      this->LastActiveWidget->Leave(eventData);
      this->LastActiveWidget = nullptr;
      }
    }

  // Find/create active widget
  vtkSlicerMarkupsWidget* activeWidget = nullptr;
  if (this->GetInteractionNode()->GetCurrentInteractionMode() == vtkMRMLInteractionNode::Place)
    {
    activeWidget = this->GetWidgetForPlacement();
    if (activeWidget)
      {
      activeWidget->SetWidgetState(vtkSlicerMarkupsWidget::WidgetStateDefine);
      }
    }
  else
    {
    double closestDistance2 = VTK_DOUBLE_MAX;
    activeWidget = this->FindClosestWidget(eventData, closestDistance2);
    }

  // Deactivate previous widget
  if (this->LastActiveWidget != nullptr && this->LastActiveWidget != activeWidget)
    {
    this->LastActiveWidget->Leave(eventData);
    }
  this->LastActiveWidget = activeWidget;
  if (!activeWidget)
    {
    // deactivate widget if we move far from it
    if (eventid == vtkCommand::MouseMoveEvent && this->LastActiveWidget != nullptr)
      {
      this->LastActiveWidget->Leave(eventData);
      this->LastActiveWidget = nullptr;
      }
    return false;
    }

  // Pass on the interaction event to the active widget
  return activeWidget->ProcessInteractionEvent(eventData);
}

//---------------------------------------------------------------------------
vtkMRMLMarkupsNode* vtkMRMLGUIWidgetsDisplayableManager::GetActiveMarkupsNodeForPlacement()
{
  vtkMRMLSelectionNode *selectionNode = this->GetSelectionNode();
  if (!selectionNode)
    {
    return nullptr;
    }
  const char *activeMarkupsID = selectionNode->GetActivePlaceNodeID();
  vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(activeMarkupsID));
  if (!markupsNode)
    {
    return nullptr;
    }
  // Additional checks for placement
  const char* placeNodeClassName = selectionNode->GetActivePlaceNodeClassName();
  if (!placeNodeClassName)
    {
    return nullptr;
    }
  if (!this->IsManageable(placeNodeClassName))
    {
    return nullptr;
    }
  if (std::string(markupsNode->GetClassName()) != placeNodeClassName)
    {
    return nullptr;
    }
  return markupsNode;
}

//---------------------------------------------------------------------------
int vtkMRMLGUIWidgetsDisplayableManager::GetCurrentInteractionMode()
{
  vtkMRMLInteractionNode *interactionNode = this->GetInteractionNode();
  if (!interactionNode)
    {
    return 0;
    }
  return interactionNode->GetCurrentInteractionMode();
}

//---------------------------------------------------------------------------
vtkSlicerMarkupsWidget* vtkMRMLGUIWidgetsDisplayableManager::GetWidgetForPlacement()
{
  if (this->GetCurrentInteractionMode() != vtkMRMLInteractionNode::Place)
    {
    return nullptr;
    }
  vtkMRMLSelectionNode *selectionNode = this->GetSelectionNode();
  if (!selectionNode)
    {
    return nullptr;
    }
  std::string placeNodeClassName = (selectionNode->GetActivePlaceNodeClassName() ? selectionNode->GetActivePlaceNodeClassName() : nullptr);
  if (!this->IsManageable(placeNodeClassName.c_str()))
    {
    return nullptr;
    }

  // Check if the active markups node is already the right class, and if yes then use that
  vtkMRMLMarkupsNode *activeMarkupsNode = this->GetActiveMarkupsNodeForPlacement();

  // Do not create a new widget if the markup is not displayable in this view
  if (activeMarkupsNode)
    {
    bool canPlaceInThisView = false;
    int numberOfDisplayNodes = activeMarkupsNode->GetNumberOfDisplayNodes();
    vtkMRMLAbstractViewNode* viewNode = vtkMRMLAbstractViewNode::SafeDownCast(this->GetMRMLDisplayableNode());
    for (int displayNodeIndex = 0; displayNodeIndex < numberOfDisplayNodes; displayNodeIndex++)
      {
        vtkMRMLDisplayNode* displayNode = activeMarkupsNode->GetNthDisplayNode(displayNodeIndex);
        if (displayNode && displayNode->IsDisplayableInView(viewNode->GetID()))
        {
        canPlaceInThisView = true;
        break;
        }
      }
    if (!canPlaceInThisView)
      {
      return nullptr;
      }
    }

  if (activeMarkupsNode && activeMarkupsNode->GetMaximumNumberOfControlPoints() > 0
    && activeMarkupsNode->GetNumberOfControlPoints() >= activeMarkupsNode->GetMaximumNumberOfControlPoints())
    {
    // maybe reached maximum number of points - if yes, then create a new widget
    if (activeMarkupsNode->GetNumberOfControlPoints() == activeMarkupsNode->GetMaximumNumberOfControlPoints())
      {
      // one more point than the maximum
      vtkSlicerMarkupsWidget *slicerWidget = this->Internal->GetWidget(activeMarkupsNode);
      if (slicerWidget && !slicerWidget->IsPointPreviewed())
        {
        // no preview is shown, so the widget is actually complete
        activeMarkupsNode = nullptr;
        }
      }
    else
      {
      // clearly over the maximum number of points
      activeMarkupsNode = nullptr;
      }
    }

  // If there is no active markups node then create a new one
  if (!activeMarkupsNode)
    {
    activeMarkupsNode = this->CreateNewMarkupsNode(placeNodeClassName);
    selectionNode->SetReferenceActivePlaceNodeID(activeMarkupsNode->GetID());
    }

  if (!activeMarkupsNode)
    {
    return nullptr;
    }
  vtkSlicerMarkupsWidget *slicerWidget = this->Internal->GetWidget(activeMarkupsNode);
  return slicerWidget;
}

//---------------------------------------------------------------------------
void vtkMRMLGUIWidgetsDisplayableManager::SetHasFocus(bool hasFocus)
{
  if (!hasFocus && this->LastActiveWidget!=nullptr)
    {
    this->LastActiveWidget->Leave(nullptr);
    this->LastActiveWidget = nullptr;
    }
}

//---------------------------------------------------------------------------
bool vtkMRMLGUIWidgetsDisplayableManager::GetGrabFocus()
{
  return (this->LastActiveWidget != nullptr && this->LastActiveWidget->GetGrabFocus());
}

//---------------------------------------------------------------------------
bool vtkMRMLGUIWidgetsDisplayableManager::GetInteractive()
{
  return (this->LastActiveWidget != nullptr && this->LastActiveWidget->GetInteractive());
}

//---------------------------------------------------------------------------
int vtkMRMLGUIWidgetsDisplayableManager::GetMouseCursor()
{
  if (!this->LastActiveWidget)
    {
    return VTK_CURSOR_DEFAULT;
    }
  return this->LastActiveWidget->GetMouseCursor();
}
*/
//---------------------------------------------------------------------------
vtkSlicerQWidgetWidget* vtkMRMLGUIWidgetsDisplayableManager::CreateWidget(vtkMRMLGUIWidgetDisplayNode* widgetDisplayNode)
{
  vtkSlicerQWidgetWidget* widget = vtkSlicerQWidgetWidget::New();

  //TODO: Display node is not used, remove from attribute?

  // If the widget was successfully created
  if (widget)
  {
    vtkMRMLAbstractViewNode* viewNode = vtkMRMLAbstractViewNode::SafeDownCast(this->GetMRMLDisplayableNode());
    vtkRenderer* renderer = this->GetRenderer();
    widget->SetMRMLApplicationLogic(this->GetMRMLApplicationLogic());
    widget->CreateDefaultRepresentation(/*widgetDisplayNode, */viewNode, renderer);
  }

  return widget;
}
/*
//---------------------------------------------------------------------------
void vtkMRMLGUIWidgetsDisplayableManager::ConvertDeviceToXYZ(double x, double y, double xyz[3])
{
  vtkMRMLAbstractSliceViewDisplayableManager::ConvertDeviceToXYZ(this->GetInteractor(), this->GetMRMLSliceNode(), x, y, xyz);
}
*/
