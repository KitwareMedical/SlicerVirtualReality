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

// VirtualReality Widgets includes
#include "qMRMLVirtualRealitySegmentEditorWidget.h"
#include "ui_qMRMLVirtualRealitySegmentEditorWidget.h"

// VirtualReality MRML includes
#include "vtkMRMLVirtualRealityViewNode.h"

// Segmentations includes
#include "vtkMRMLSegmentEditorNode.h"

// Qt includes
#include <QDebug>
#include "qpushbutton.h"
#include "ctkMenuButton.h"
#include "qtoolbutton.h"

//-----------------------------------------------------------------------------
class qMRMLVirtualRealitySegmentEditorWidgetPrivate : public Ui_qMRMLVirtualRealitySegmentEditorWidget
{
  Q_DECLARE_PUBLIC(qMRMLVirtualRealitySegmentEditorWidget);

protected:
  qMRMLVirtualRealitySegmentEditorWidget* const q_ptr;

public:
  qMRMLVirtualRealitySegmentEditorWidgetPrivate(qMRMLVirtualRealitySegmentEditorWidget& object);
  virtual ~qMRMLVirtualRealitySegmentEditorWidgetPrivate();

  void init();
};

//-----------------------------------------------------------------------------
qMRMLVirtualRealitySegmentEditorWidgetPrivate::qMRMLVirtualRealitySegmentEditorWidgetPrivate(qMRMLVirtualRealitySegmentEditorWidget& object)
  : q_ptr(&object)
{
}

//-----------------------------------------------------------------------------
qMRMLVirtualRealitySegmentEditorWidgetPrivate::~qMRMLVirtualRealitySegmentEditorWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
void qMRMLVirtualRealitySegmentEditorWidgetPrivate::init()
{
  Q_Q(qMRMLVirtualRealitySegmentEditorWidget);
  this->setupUi(q);

  //List of effects to be used in VR; effects not listed here are not shown in the widget
  QStringList effectNameOrder;
  effectNameOrder.append("None");
  effectNameOrder.append("Paint");
  effectNameOrder.append("Erase");
  effectNameOrder.append("Level tracing");
  effectNameOrder.append("Grow from seeds");
  effectNameOrder.append("Smoothing");
  effectNameOrder.append("Scissors");
  effectNameOrder.append("Logical Operators");
  this->SegmentEditorWidget->setEffectNameOrder(effectNameOrder);

  //Hide unneeded effects and buttons
  this->SegmentEditorWidget->setUnorderedEffectsVisible(false);
  this->SegmentEditorWidget->setSwitchToSegmentationsButtonVisible(false);
  this->SegmentEditorWidget->findChild<QToolButton*>("SpecifyGeometryButton")->setVisible(false);
  this->SegmentEditorWidget->findChild<QToolButton*>("SliceRotateWarningButton")->setVisible(false);
  this->SegmentEditorWidget->findChild<ctkMenuButton*>("Show3DButton")->setVisible(false);
}

//-----------------------------------------------------------------------------
// qMRMLVirtualRealitySegmentEditorWidget methods

//-----------------------------------------------------------------------------
qMRMLVirtualRealitySegmentEditorWidget::qMRMLVirtualRealitySegmentEditorWidget(QWidget* _parent)
  : qMRMLWidget(_parent)
  , d_ptr(new qMRMLVirtualRealitySegmentEditorWidgetPrivate(*this))
{
  Q_D(qMRMLVirtualRealitySegmentEditorWidget);
  d->init();

  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
qMRMLVirtualRealitySegmentEditorWidget::~qMRMLVirtualRealitySegmentEditorWidget()
= default;

//-----------------------------------------------------------------------------
void qMRMLVirtualRealitySegmentEditorWidget::updateWidgetFromMRML()
{
  //Q_D(qMRMLVirtualRealitySegmentEditorWidget);
}

void qMRMLVirtualRealitySegmentEditorWidget::setMRMLSegmentEditorNode(vtkMRMLSegmentEditorNode* newSegmentEditorNode)
{
  Q_D(qMRMLVirtualRealitySegmentEditorWidget);
  d->SegmentEditorWidget->setMRMLSegmentEditorNode(newSegmentEditorNode);
}
