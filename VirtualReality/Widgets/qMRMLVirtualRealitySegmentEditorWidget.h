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
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care
  and CANARIE.

==============================================================================*/

#ifndef __qMRMLVirtualRealitySegmentEditorWidget_h
#define __qMRMLVirtualRealitySegmentEditorWidget_h

// VirtualReality Widgets includes
#include "qSlicerVirtualRealityModuleWidgetsExport.h"

// MRMLWidgets includes
#include "qMRMLWidget.h"

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

class qMRMLVirtualRealitySegmentEditorWidgetPrivate;
class vtkMRMLSegmentEditorNode;

/// \ingroup SlicerVirtualReality_Widgets
class Q_SLICER_QTMODULES_VIRTUALREALITY_WIDGETS_EXPORT qMRMLVirtualRealitySegmentEditorWidget : public qMRMLWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef qMRMLWidget Superclass;
  /// Constructor
  explicit qMRMLVirtualRealitySegmentEditorWidget(QWidget* parent = nullptr);
  /// Destructor
  ~qMRMLVirtualRealitySegmentEditorWidget() override;

  void setMRMLSegmentEditorNode(vtkMRMLSegmentEditorNode* newSegmentEditorNode);

public slots:

protected slots:
  /// Update widgets from the MRML node
  void updateWidgetFromMRML();

protected:
  QScopedPointer<qMRMLVirtualRealitySegmentEditorWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLVirtualRealitySegmentEditorWidget);
  Q_DISABLE_COPY(qMRMLVirtualRealitySegmentEditorWidget);
};

#endif
