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

#ifndef __qMRMLVirtualRealityHomeWidget_h
#define __qMRMLVirtualRealityHomeWidget_h

// VirtualReality Widgets includes
#include "qSlicerVirtualRealityModuleWidgetsExport.h"

// MRMLWidgets includes
#include "qMRMLWidget.h"

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// Qt includes
#include <QIcon>
#include <QPushButton>

class vtkMRMLVirtualRealityViewNode;
class qMRMLVirtualRealityHomeWidgetPrivate;
class vtkMRMLSegmentEditorNode;

/// \ingroup SlicerVirtualReality_Widgets
class Q_SLICER_QTMODULES_VIRTUALREALITY_WIDGETS_EXPORT qMRMLVirtualRealityHomeWidget : public qMRMLWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef qMRMLWidget Superclass;
  /// Constructor
  explicit qMRMLVirtualRealityHomeWidget(QWidget* parent = nullptr);
  /// Destructor
  ~qMRMLVirtualRealityHomeWidget() override;

  /// Get virtual reality view MRML node
  Q_INVOKABLE vtkMRMLVirtualRealityViewNode* virtualRealityViewNode()const;
  /// Get virtual reality view MRML node ID
  Q_INVOKABLE QString virtualRealityViewNodeID()const;

  /// Add new button for a given VR module widget
  /// \param moduleWidget The widget that appears in place of the home widget when its button is pressed
  /// \param icon The icon that appears on the button in the home widget
  Q_INVOKABLE void addModuleButton(QWidget* moduleWidget, QIcon& icon);

  /// Register default modules: Data, Segment Editor
  Q_INVOKABLE void registerDefaultModules();

  /// Set the segment editor node for the segment editor widget
  Q_INVOKABLE void setMRMLSegmentEditorNode(vtkMRMLSegmentEditorNode* newSegmentEditorNode);

public slots:
  /// Set virtual reality view MRML node
  void setVirtualRealityViewNode(vtkMRMLVirtualRealityViewNode* node);

  void onMotionSensitivityChanged(double);
  void onFlySpeedChanged(double);
  void onMagnification001xPressed();
  void onMagnification01xPressed();
  void onMagnification1xPressed();
  void onMagnification10xPressed();
  void onMagnification100xPressed();
  //void updateViewFromReferenceViewCamera();
  //void setMagnificationLock(bool);

  void onModuleButtonClicked();
  void onBackButtonClicked();

protected slots:
  /// Update widgets from the MRML node
  void updateWidgetFromMRML();

protected:
  QScopedPointer<qMRMLVirtualRealityHomeWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLVirtualRealityHomeWidget);
  Q_DISABLE_COPY(qMRMLVirtualRealityHomeWidget);
};

#endif
