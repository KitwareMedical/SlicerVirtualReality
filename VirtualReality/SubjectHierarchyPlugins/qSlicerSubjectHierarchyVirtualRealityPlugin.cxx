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

// SubjectHierarchy MRML includes
#include "vtkMRMLSubjectHierarchyNode.h"
#include "vtkMRMLSubjectHierarchyConstants.h"

// SubjectHierarchy Plugins includes
#include "qSlicerSubjectHierarchyPluginHandler.h"
#include "qSlicerSubjectHierarchyVirtualRealityPlugin.h"

// MRML includes
#include <vtkMRMLNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkCollection.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

// Qt includes
#include <QDebug>
#include <QAction>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SubjectHierarchy_Plugins
class qSlicerSubjectHierarchyVirtualRealityPluginPrivate: public QObject
{
  Q_DECLARE_PUBLIC(qSlicerSubjectHierarchyVirtualRealityPlugin);
protected:
  qSlicerSubjectHierarchyVirtualRealityPlugin* const q_ptr;
public:
  qSlicerSubjectHierarchyVirtualRealityPluginPrivate(qSlicerSubjectHierarchyVirtualRealityPlugin& object);
  ~qSlicerSubjectHierarchyVirtualRealityPluginPrivate();
  void init();
public:
  QAction* ToggleSelectableAction;
};

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyVirtualRealityPluginPrivate methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyVirtualRealityPluginPrivate::qSlicerSubjectHierarchyVirtualRealityPluginPrivate(qSlicerSubjectHierarchyVirtualRealityPlugin& object)
: q_ptr(&object)
, ToggleSelectableAction(NULL)
{
}

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchyVirtualRealityPluginPrivate::init()
{
  Q_Q(qSlicerSubjectHierarchyVirtualRealityPlugin);

  // Initial action
  this->ToggleSelectableAction = new QAction("Toggle selectable",q);
  this->ToggleSelectableAction->setToolTip(tr("Toggle selectable flag on object. If on, then can be manipulated through user interaction, otherwise not."));
  QObject::connect(this->ToggleSelectableAction, SIGNAL(toggled(bool)), q, SLOT(setCurrentItemSelectable(bool)));
  this->ToggleSelectableAction->setCheckable(true);
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyVirtualRealityPluginPrivate::~qSlicerSubjectHierarchyVirtualRealityPluginPrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyVirtualRealityPlugin methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyVirtualRealityPlugin::qSlicerSubjectHierarchyVirtualRealityPlugin(QObject* parent)
 : Superclass(parent)
 , d_ptr( new qSlicerSubjectHierarchyVirtualRealityPluginPrivate(*this) )
{
  this->m_Name = QString("VirtualReality");

  Q_D(qSlicerSubjectHierarchyVirtualRealityPlugin);
  d->init();
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyVirtualRealityPlugin::~qSlicerSubjectHierarchyVirtualRealityPlugin()
{
}

//---------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchyVirtualRealityPlugin::itemContextMenuActions()const
{
  Q_D(const qSlicerSubjectHierarchyVirtualRealityPlugin);

  QList<QAction*> actions;
  actions << d->ToggleSelectableAction;
  return actions;
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyVirtualRealityPlugin::showContextMenuActionsForItem(vtkIdType itemID)
{
  Q_D(qSlicerSubjectHierarchyVirtualRealityPlugin);

  if (!itemID)
    {
    // There are no scene actions in this plugin
    return;
    }

  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }

  // Patient, study, folder or data node
  if ( shNode->IsItemLevel(itemID, vtkMRMLSubjectHierarchyConstants::GetDICOMLevelPatient())
    || shNode->IsItemLevel(itemID, vtkMRMLSubjectHierarchyConstants::GetDICOMLevelStudy())
    || shNode->IsItemLevel(itemID, vtkMRMLSubjectHierarchyConstants::GetSubjectHierarchyLevelFolder())
    || shNode->GetItemDataNode(itemID) )
    {
    d->ToggleSelectableAction->setVisible(true);
    bool wasBlocked = d->ToggleSelectableAction->blockSignals(false);
    d->ToggleSelectableAction->setChecked(this->getItemSelectable(itemID));
    d->ToggleSelectableAction->blockSignals(wasBlocked);
    }
}

//---------------------------------------------------------------------------
bool qSlicerSubjectHierarchyVirtualRealityPlugin::getItemSelectable(vtkIdType itemID)
{
  Q_UNUSED(itemID);
  Q_D(qSlicerSubjectHierarchyVirtualRealityPlugin);

  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return false;
    }

  vtkIdType currentItemID = qSlicerSubjectHierarchyPluginHandler::instance()->currentItem();
  if (!currentItemID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid current item";
    return false;
    }

  // Toggle visibility in branch
  vtkSmartPointer<vtkCollection> childNodes = vtkSmartPointer<vtkCollection>::New();
  shNode->GetDataNodesInBranch(currentItemID, childNodes);
  childNodes->InitTraversal();
  for (int i=0; i<childNodes->GetNumberOfItems(); ++i)
    {
    vtkMRMLNode* node = vtkMRMLNode::SafeDownCast(childNodes->GetItemAsObject(i));
    if (node)
      {
      if (!node->GetSelectable())
        {
        return false;
        }
      }
    }

  return true;
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyVirtualRealityPlugin::setCurrentItemSelectable(bool selectable)
{
  Q_D(qSlicerSubjectHierarchyVirtualRealityPlugin);

  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }

  vtkIdType currentItemID = qSlicerSubjectHierarchyPluginHandler::instance()->currentItem();
  if (!currentItemID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid current item";
    return;
    }

  // Toggle visibility in branch
  vtkSmartPointer<vtkCollection> childNodes = vtkSmartPointer<vtkCollection>::New();
  shNode->GetDataNodesInBranch(currentItemID, childNodes);
  childNodes->InitTraversal();
  for (int i=0; i<childNodes->GetNumberOfItems(); ++i)
    {
    vtkMRMLNode* node = vtkMRMLNode::SafeDownCast(childNodes->GetItemAsObject(i));
    if (node)
      {
      node->SetSelectable(selectable);
      }
    }
}
