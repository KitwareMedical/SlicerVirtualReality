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

#ifndef __qSlicerSubjectHierarchyVirtualRealityPlugin_h
#define __qSlicerSubjectHierarchyVirtualRealityPlugin_h

// SubjectHierarchy includes
#include "qSlicerSubjectHierarchyAbstractPlugin.h"

#include "qSlicerVirtualRealitySubjectHierarchyPluginsExport.h"

class qSlicerSubjectHierarchyVirtualRealityPluginPrivate;

/// \ingroup Slicer_QtModules_SubjectHierarchy_Plugins
class Q_SLICER_VIRTUALREALITY_SUBJECT_HIERARCHY_PLUGINS_EXPORT qSlicerSubjectHierarchyVirtualRealityPlugin : public qSlicerSubjectHierarchyAbstractPlugin
{
public:
  Q_OBJECT

public:
  typedef qSlicerSubjectHierarchyAbstractPlugin Superclass;
  qSlicerSubjectHierarchyVirtualRealityPlugin(QObject* parent = NULL);
  virtual ~qSlicerSubjectHierarchyVirtualRealityPlugin();

public:
  /// Get item context menu item actions to add to tree view
  virtual QList<QAction*> itemContextMenuActions()const;

  /// Show context menu actions valid for a given subject hierarchy item.
  /// \param itemID Subject Hierarchy item to show the context menu items for
  virtual void showContextMenuActionsForItem(vtkIdType itemID);

  /// Get selectable flag on given item's node(s).
  /// \return True if all are selectable, false if at least one of the nodes are not
  bool getItemSelectable(vtkIdType itemID);

protected slots:
  /// Toggle selectable flag on current item's node(s)
  void setCurrentItemSelectable(bool);

protected:
  QScopedPointer<qSlicerSubjectHierarchyVirtualRealityPluginPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerSubjectHierarchyVirtualRealityPlugin);
  Q_DISABLE_COPY(qSlicerSubjectHierarchyVirtualRealityPlugin);
};

#endif
