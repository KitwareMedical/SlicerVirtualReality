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
#include "qMRMLVirtualRealityHomeWidget.h"
#include "ui_qMRMLVirtualRealityHomeWidget.h"
#include "qMRMLVirtualRealityDataModuleWidget.h"
#include "qMRMLVirtualRealitySegmentEditorWidget.h"

// VirtualReality MRML includes
#include "vtkMRMLVirtualRealityViewNode.h"

// Segmentations includes
#include "vtkMRMLSegmentEditorNode.h"

// VTK includes
#include <vtkWeakPointer.h>

// Qt includes
#include <QDebug>
#include <QIcon>

//-----------------------------------------------------------------------------
class qMRMLVirtualRealityHomeWidgetPrivate : public Ui_qMRMLVirtualRealityHomeWidget
{
  Q_DECLARE_PUBLIC(qMRMLVirtualRealityHomeWidget);

protected:
  qMRMLVirtualRealityHomeWidget* const q_ptr;

public:
  qMRMLVirtualRealityHomeWidgetPrivate(qMRMLVirtualRealityHomeWidget& object);
  virtual ~qMRMLVirtualRealityHomeWidgetPrivate();

  void init();

  /// Association between VR module buttons and the corresponding widgets
  QMap<QPushButton*, QWidget*> ModuleWidgetsMap;

public:
  /// Virtual reality view MRML node
  vtkWeakPointer<vtkMRMLVirtualRealityViewNode> VirtualRealityViewNode;
  qMRMLVirtualRealityDataModuleWidget* DataModuleWidget;
  qMRMLVirtualRealitySegmentEditorWidget* SegmentEditorWidget;
};

//-----------------------------------------------------------------------------
qMRMLVirtualRealityHomeWidgetPrivate::qMRMLVirtualRealityHomeWidgetPrivate(qMRMLVirtualRealityHomeWidget& object)
  : q_ptr(&object)
{
  this->VirtualRealityViewNode = nullptr;
  this->DataModuleWidget = nullptr;
  this->SegmentEditorWidget = nullptr;
}

//-----------------------------------------------------------------------------
qMRMLVirtualRealityHomeWidgetPrivate::~qMRMLVirtualRealityHomeWidgetPrivate()
{
  this->VirtualRealityViewNode = nullptr;
}

//-----------------------------------------------------------------------------
void qMRMLVirtualRealityHomeWidgetPrivate::init()
{
  Q_Q(qMRMLVirtualRealityHomeWidget);
  this->setupUi(q);

  this->backButton->setVisible(false);

  QObject::connect(this->MotionSensitivitySliderWidget, SIGNAL(valueChanged(double)), q, SLOT(onMotionSensitivityChanged(double)));
  QObject::connect(this->FlySpeedSliderWidget, SIGNAL(valueChanged(double)), q, SLOT(onFlySpeedChanged(double)));
  QObject::connect(this->Magnification001xButton, SIGNAL(clicked()), q, SLOT(onMagnification001xPressed()));
  QObject::connect(this->Magnification01xButton, SIGNAL(clicked()), q, SLOT(onMagnification01xPressed()));
  QObject::connect(this->Magnification1xButton, SIGNAL(clicked()), q, SLOT(onMagnification1xPressed()));
  QObject::connect(this->Magnification10xButton, SIGNAL(clicked()), q, SLOT(onMagnification10xPressed()));
  QObject::connect(this->Magnification100xButton, SIGNAL(clicked()), q, SLOT(onMagnification100xPressed()));
  QObject::connect(this->SyncViewToReferenceViewButton, SIGNAL(clicked()), q, SLOT(updateViewFromReferenceViewCamera()));

  //QObject::connect(this->LockMagnificationCheckBox, SIGNAL(toggled(bool)), q, SLOT(setMagnificationLock(bool)));
  //TODO: Magnification lock of view node not implemented yet

  // Hide module widget frame. It appears with the module when a module button is clicked
  this->ModuleWidgetFrame->setVisible(false);
  // Setup module widget frame layout
  QVBoxLayout* moduleWidgetFrameLayout = new QVBoxLayout(this->ModuleWidgetFrame);

  // Register default VR modules
  q->registerDefaultModules();
}

//-----------------------------------------------------------------------------
// qMRMLVirtualRealityHomeWidget methods

//-----------------------------------------------------------------------------

qMRMLVirtualRealityHomeWidget::qMRMLVirtualRealityHomeWidget(QWidget* _parent)
  : qMRMLWidget(_parent)
  , d_ptr(new qMRMLVirtualRealityHomeWidgetPrivate(*this))
{
  Q_D(qMRMLVirtualRealityHomeWidget);
  d->init();
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
qMRMLVirtualRealityHomeWidget::~qMRMLVirtualRealityHomeWidget()
= default;

//-----------------------------------------------------------------------------
vtkMRMLVirtualRealityViewNode* qMRMLVirtualRealityHomeWidget::virtualRealityViewNode() const
{
  Q_D(const qMRMLVirtualRealityHomeWidget);
  return d->VirtualRealityViewNode;
}

//-----------------------------------------------------------------------------
QString qMRMLVirtualRealityHomeWidget::virtualRealityViewNodeID()const
{
  Q_D(const qMRMLVirtualRealityHomeWidget);
  return (d->VirtualRealityViewNode.GetPointer() ? d->VirtualRealityViewNode->GetID() : QString());
}

//-----------------------------------------------------------------------------
void qMRMLVirtualRealityHomeWidget::setVirtualRealityViewNode(vtkMRMLVirtualRealityViewNode* node)
{
  Q_D(qMRMLVirtualRealityHomeWidget);

  if (d->VirtualRealityViewNode == node)
  {
    return;
  }

  qvtkReconnect(d->VirtualRealityViewNode, node, vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromMRML()));

  vtkMRMLVirtualRealityViewNode* vrViewNode = vtkMRMLVirtualRealityViewNode::SafeDownCast(node);
  d->VirtualRealityViewNode = vrViewNode;

  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qMRMLVirtualRealityHomeWidget::updateWidgetFromMRML()
{
  Q_D(qMRMLVirtualRealityHomeWidget);
  vtkMRMLVirtualRealityViewNode* vrViewNode = d->VirtualRealityViewNode;

  bool wasBlocked = d->MotionSensitivitySliderWidget->blockSignals(true);
  d->MotionSensitivitySliderWidget->setValue(vrViewNode != nullptr ? vrViewNode->GetMotionSensitivity() * 100.0 : 0);
  d->MotionSensitivitySliderWidget->setEnabled(vrViewNode != nullptr);
  d->MotionSensitivitySliderWidget->blockSignals(wasBlocked);

  wasBlocked = d->FlySpeedSliderWidget->blockSignals(true);
  d->FlySpeedSliderWidget->setValue(vrViewNode != nullptr ? vrViewNode->GetMotionSpeed() : 1.6666);
  d->FlySpeedSliderWidget->setEnabled(vrViewNode != nullptr);
  d->FlySpeedSliderWidget->blockSignals(wasBlocked);

  /*
  bool wasBlocked = d->LockMagnificationCheckBox->blockSignals(true);
  d->LockMagnificationCheckBox->setChecked(vrViewNode != nullptr && vrViewNode->);
  d->LockMagnificationCheckBox->setEnabled(vrViewNode != nullptr);
  d->LockMagnificationCheckBox->blockSignals(wasBlocked);
  */
  //TODO: Magnification lock of view node not implemented yet 

  d->Magnification001xButton->setEnabled(vrViewNode != nullptr && vrViewNode->GetMagnification() != NULL);
  d->Magnification01xButton->setEnabled(vrViewNode != nullptr && vrViewNode->GetMagnification() != NULL);
  d->Magnification1xButton->setEnabled(vrViewNode != nullptr && vrViewNode->GetMagnification() != NULL);
  d->Magnification10xButton->setEnabled(vrViewNode != nullptr && vrViewNode->GetMagnification() != NULL);
  d->Magnification100xButton->setEnabled(vrViewNode != nullptr && vrViewNode->GetMagnification() != NULL); 
}

//-----------------------------------------------------------------------------
void qMRMLVirtualRealityHomeWidget::onMotionSensitivityChanged(double percent)
{
  Q_D(qMRMLVirtualRealityHomeWidget);
  vtkMRMLVirtualRealityViewNode* vrViewNode = d->VirtualRealityViewNode;

  if (!vrViewNode)
  {
    qCritical() << Q_FUNC_INFO << " Failed: view node is null";
    return;
  }
  vrViewNode->SetMotionSensitivity(percent * 0.01);
}

//-----------------------------------------------------------------------------
void qMRMLVirtualRealityHomeWidget::onFlySpeedChanged(double speedMps)
{
  Q_D(qMRMLVirtualRealityHomeWidget);
  vtkMRMLVirtualRealityViewNode* vrViewNode = d->VirtualRealityViewNode;

  if (!vrViewNode)
  {
    qCritical() << Q_FUNC_INFO << " Failed: view node is null";
    return;
  }
  vrViewNode->SetMotionSpeed(speedMps);
}

//-----------------------------------------------------------------------------
void qMRMLVirtualRealityHomeWidget::onMagnification001xPressed()
{
  Q_D(qMRMLVirtualRealityHomeWidget);
  vtkMRMLVirtualRealityViewNode* vrViewNode = d->VirtualRealityViewNode;

  if (!vrViewNode)
  {
    qCritical() << Q_FUNC_INFO << " Failed: view node is null";
    return;
  }
  vrViewNode->SetMagnification(0.01);
}

//-----------------------------------------------------------------------------
void qMRMLVirtualRealityHomeWidget::onMagnification01xPressed()
{
  Q_D(qMRMLVirtualRealityHomeWidget);
  vtkMRMLVirtualRealityViewNode* vrViewNode = d->VirtualRealityViewNode;

  if (!vrViewNode)
  {
    qCritical() << Q_FUNC_INFO << " Failed: view node is null";
    return;
  }
  vrViewNode->SetMagnification(0.1);
}

//-----------------------------------------------------------------------------
void qMRMLVirtualRealityHomeWidget::onMagnification1xPressed()
{
  Q_D(qMRMLVirtualRealityHomeWidget);
  vtkMRMLVirtualRealityViewNode* vrViewNode = d->VirtualRealityViewNode;

  if (!vrViewNode)
  {
    qCritical() << Q_FUNC_INFO << " Failed: view node is null";
    return;
  }
  vrViewNode->SetMagnification(1.0);
}

//-----------------------------------------------------------------------------
void qMRMLVirtualRealityHomeWidget::onMagnification10xPressed()
{
  Q_D(qMRMLVirtualRealityHomeWidget);
  vtkMRMLVirtualRealityViewNode* vrViewNode = d->VirtualRealityViewNode;

  if (!vrViewNode)
  {
    qCritical() << Q_FUNC_INFO << " Failed: view node is null";
    return;
  }
  vrViewNode->SetMagnification(10.0);
}

//-----------------------------------------------------------------------------
void qMRMLVirtualRealityHomeWidget::onMagnification100xPressed()
{
  Q_D(qMRMLVirtualRealityHomeWidget);
  vtkMRMLVirtualRealityViewNode* vrViewNode = d->VirtualRealityViewNode;

  if (!vrViewNode)
  {
    qCritical() << Q_FUNC_INFO << " Failed: view node is null";
    return;
  }
  vrViewNode->SetMagnification(100.0);
}

//-----------------------------------------------------------------------------
/*
void qMRMLVirtualRealityHomeWidget::updateViewFromReferenceViewCamera()
{
  Q_D(qMRMLVirtualRealityHomeWidget);
  qSlicerVirtualRealityModule* vrModule = dynamic_cast<qSlicerVirtualRealityModule*>(this->module());
  if (!vrModule)
  {
    qCritical() << Q_FUNC_INFO << " Failed: vrModule is null";
    return;
  }
  qMRMLVirtualRealityView* vrView = vrModule->viewWidget();
  if (!vrView)
  {
    qCritical() << Q_FUNC_INFO << " Failed: view node is null";
    return;
  }
  vrView->updateViewFromReferenceViewCamera();
}
*/
//TODO: This member function won't work unless qSlicerVirtualRealityModule and qMRMLVirtualRealityView are included

//-----------------------------------------------------------------------------
/*
void qMRMLVirtualRealityHomeWidget::setMagnificationLock(bool active)
{
  Q_D(qMRMLVirtualRealityHomeWidget);
  vtkMRMLVirtualRealityViewNode* vrViewNode = d->VirtualRealityViewNode;

  if (!vrViewNode)
  {
   qCritical() << Q_FUNC_INFO << " Failed: view node is null";
   return;
  }
  
  vrViewNode->????(active);  //TODO: Implement magnification lock for view node
}
*/

//-----------------------------------------------------------------------------
void qMRMLVirtualRealityHomeWidget::addModuleButton(QWidget* moduleWidget, QIcon& icon)
{
  Q_D(qMRMLVirtualRealityHomeWidget);

  if (!moduleWidget)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid module widget given";
    return;
  }

  // Create button for the new module
  QPushButton* moduleButton = new QPushButton(d->ModulesGroupBox);
  d->ModulesGroupBoxLayout->addWidget(moduleButton);
  moduleButton->setIcon(icon);

  // Setup module widget within home widget
  moduleWidget->setParent(d->ModuleWidgetFrame);
  moduleWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
  moduleWidget->setVisible(false);
  d->ModuleWidgetFrame->layout()->addWidget(moduleWidget);
  d->ModuleWidgetsMap[moduleButton] = moduleWidget;

  // Handle scene if a MRML widget is given
  qMRMLWidget* moduleMrmlWidget = qobject_cast<qMRMLWidget*>(moduleWidget);
  if (moduleMrmlWidget)
  {
    moduleMrmlWidget->setMRMLScene(this->mrmlScene());
    QObject::connect(this, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)), moduleWidget, SLOT(setMRMLScene(vtkMRMLScene*)));
  }

  QObject::connect(moduleButton, SIGNAL(clicked()), this, SLOT(onModuleButtonClicked()));
  QObject::connect(d->backButton, SIGNAL(clicked()), this, SLOT(onBackButtonClicked()));

  moduleButton->setVisible(true);
  d->backButton->setVisible(false);
}

//-----------------------------------------------------------------------------
void qMRMLVirtualRealityHomeWidget::onModuleButtonClicked()
{
  Q_D(qMRMLVirtualRealityHomeWidget);

  QPushButton* moduleButton = qobject_cast<QPushButton*>(sender());
  if (!moduleButton)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to access clicked button";
    return;
  }

  d->HomeWidgetFrame->setVisible(false);
  d->ModulesGroupBox->setVisible(false);

  d->ModuleWidgetFrame->setVisible(true);
  d->ModuleWidgetsMap[moduleButton]->setVisible(true);
  d->backButton->setVisible(true);
}

//-----------------------------------------------------------------------------
void  qMRMLVirtualRealityHomeWidget::onBackButtonClicked()
{
  Q_D(qMRMLVirtualRealityHomeWidget);

  QMap<QPushButton*, QWidget*>::const_iterator buttonIt;
  for (buttonIt = d->ModuleWidgetsMap.constBegin(); buttonIt != d->ModuleWidgetsMap.constEnd(); ++buttonIt)
  {
    buttonIt.value()->setVisible(false);
  }
  d->ModuleWidgetFrame->setVisible(false);
  d->backButton->setVisible(false);

  d->HomeWidgetFrame->setVisible(true);
  d->ModulesGroupBox->setVisible(true);
}

//-----------------------------------------------------------------------------
void qMRMLVirtualRealityHomeWidget::registerDefaultModules()
{
  Q_D(qMRMLVirtualRealityHomeWidget);

  d->DataModuleWidget = new qMRMLVirtualRealityDataModuleWidget(this);
  d->DataModuleWidget->setMRMLScene(this->mrmlScene());
  d->SegmentEditorWidget = new qMRMLVirtualRealitySegmentEditorWidget(this);
  d->SegmentEditorWidget->setMRMLScene(this->mrmlScene());

  QIcon dataIcon(QPixmap(":/Icons/SubjectHierarchy.png"));
  QIcon segmentEditorIcon(QPixmap(":/Icons/SegmentEditor.png"));

  this->addModuleButton(d->DataModuleWidget, dataIcon);
  this->addModuleButton(d->SegmentEditorWidget, segmentEditorIcon);
}

void qMRMLVirtualRealityHomeWidget::setMRMLSegmentEditorNode(vtkMRMLSegmentEditorNode* newSegmentEditorNode)
{
  Q_D(qMRMLVirtualRealityHomeWidget);
  d->SegmentEditorWidget->setMRMLSegmentEditorNode(newSegmentEditorNode);
}
