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

// Qt includes
#include <QDebug>

// SlicerQt includes
#include "qSlicerVirtualRealityModuleWidget.h"
#include "ui_qSlicerVirtualRealityModuleWidget.h"

// MRML includes
#include "vtkMRMLScene.h"

// VirtualReality includes
#include "qSlicerVirtualRealityModule.h"
#include "qMRMLVirtualRealityView.h"
#include "vtkMRMLVirtualRealityViewNode.h"
#include "vtkMRMLVirtualRealityViewDisplayableManagerFactory.h"
#include "vtkSlicerVirtualRealityLogic.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerVirtualRealityModuleWidgetPrivate: public Ui_qSlicerVirtualRealityModuleWidget
{
public:
  qSlicerVirtualRealityModuleWidgetPrivate();
};

//-----------------------------------------------------------------------------
// qSlicerVirtualRealityModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerVirtualRealityModuleWidgetPrivate::qSlicerVirtualRealityModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerVirtualRealityModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerVirtualRealityModuleWidget::qSlicerVirtualRealityModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerVirtualRealityModuleWidgetPrivate )
{
}

//-----------------------------------------------------------------------------
qSlicerVirtualRealityModuleWidget::~qSlicerVirtualRealityModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerVirtualRealityModuleWidget::setup()
{
  Q_D(qSlicerVirtualRealityModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();

  connect(d->ConnectCheckBox, SIGNAL(toggled(bool)), this, SLOT(setVirtualRealityConnected(bool)));
  connect(d->RenderingEnabledCheckBox, SIGNAL(toggled(bool)), this, SLOT(setVirtualRealityActive(bool)));
  connect(d->TwoSidedLightingCheckBox, SIGNAL(toggled(bool)), this, SLOT(setTwoSidedLighting(bool)));
  connect(d->BackLightsCheckBox, SIGNAL(toggled(bool)), this, SLOT(setBackLights(bool)));

  connect(d->ReferenceViewNodeComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(setReferenceViewNode(vtkMRMLNode*)));
  connect(d->UpdateViewFromReferenceViewCameraButton, SIGNAL(clicked()), this, SLOT(updateViewFromReferenceViewCamera()));

  connect(d->DesiredUpdateRateSlider, SIGNAL(valueChanged(double)), this, SLOT(onDesiredUpdateRateChanged(double)));


  this->updateWidgetFromMRML();

  // If virtual reality logic is modified it indicates that the view node may changed
  qvtkConnect(logic(), vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromMRML()));
}

//--------------------------------------------------------------------------
void qSlicerVirtualRealityModuleWidget::updateWidgetFromMRML()
{
  Q_D(qSlicerVirtualRealityModuleWidget);
  vtkSlicerVirtualRealityLogic* vrLogic = vtkSlicerVirtualRealityLogic::SafeDownCast(this->logic());
  vtkMRMLVirtualRealityViewNode* vrViewNode = vrLogic->GetVirtualRealityViewNode();

  bool wasBlocked = d->ConnectCheckBox->blockSignals(true);
  d->ConnectCheckBox->setChecked(vrViewNode != NULL && vrViewNode->GetVisibility());
  d->ConnectCheckBox->blockSignals(wasBlocked);

  wasBlocked = d->RenderingEnabledCheckBox->blockSignals(true);
  d->RenderingEnabledCheckBox->setChecked(vrViewNode != NULL && vrViewNode->GetActive());
  d->RenderingEnabledCheckBox->blockSignals(wasBlocked);

  wasBlocked = d->DesiredUpdateRateSlider->blockSignals(true);
  d->DesiredUpdateRateSlider->setValue(vrViewNode != NULL ? vrViewNode->GetDesiredUpdateRate() : 0);
  d->DesiredUpdateRateSlider->setEnabled(vrViewNode != NULL);
  d->DesiredUpdateRateSlider->blockSignals(wasBlocked);

  wasBlocked = d->TwoSidedLightingCheckBox->blockSignals(true);
  d->TwoSidedLightingCheckBox->setChecked(vrViewNode != NULL && vrViewNode->GetTwoSidedLighting());
  d->TwoSidedLightingCheckBox->setEnabled(vrViewNode != NULL);
  d->TwoSidedLightingCheckBox->blockSignals(wasBlocked);

  wasBlocked = d->BackLightsCheckBox->blockSignals(true);
  d->BackLightsCheckBox->setChecked(vrViewNode != NULL && vrViewNode->GetBackLights());
  d->BackLightsCheckBox->setEnabled(vrViewNode != NULL);
  d->BackLightsCheckBox->blockSignals(wasBlocked);

  wasBlocked = d->ReferenceViewNodeComboBox->blockSignals(true);
  d->ReferenceViewNodeComboBox->setCurrentNode(vrViewNode != NULL ? vrViewNode->GetReferenceViewNode() : NULL);
  d->ReferenceViewNodeComboBox->blockSignals(wasBlocked);
  d->ReferenceViewNodeComboBox->setEnabled(vrViewNode != NULL);

  d->UpdateViewFromReferenceViewCameraButton->setEnabled(vrViewNode != NULL
    && vrViewNode->GetReferenceViewNode() != NULL);
}

//-----------------------------------------------------------------------------
void qSlicerVirtualRealityModuleWidget::setVirtualRealityConnected(bool connect)
{
  Q_D(qSlicerVirtualRealityModuleWidget);
  vtkSlicerVirtualRealityLogic* vrLogic = vtkSlicerVirtualRealityLogic::SafeDownCast(this->logic());
  vrLogic->SetVirtualRealityConnected(connect);
}

//-----------------------------------------------------------------------------
void qSlicerVirtualRealityModuleWidget::setVirtualRealityActive(bool activate)
{
  Q_D(qSlicerVirtualRealityModuleWidget);
  vtkSlicerVirtualRealityLogic* vrLogic = vtkSlicerVirtualRealityLogic::SafeDownCast(this->logic());
  vrLogic->SetVirtualRealityActive(activate);
}

//-----------------------------------------------------------------------------
void qSlicerVirtualRealityModuleWidget::setTwoSidedLighting(bool active)
{
  Q_D(qSlicerVirtualRealityModuleWidget);
  vtkSlicerVirtualRealityLogic* vrLogic = vtkSlicerVirtualRealityLogic::SafeDownCast(this->logic());
  vtkMRMLVirtualRealityViewNode* vrViewNode = vrLogic->GetVirtualRealityViewNode();
  if (vrViewNode)
  {
    vrViewNode->SetTwoSidedLighting(active);
  }
}

//-----------------------------------------------------------------------------
void qSlicerVirtualRealityModuleWidget::setBackLights(bool active)
{
  Q_D(qSlicerVirtualRealityModuleWidget);
  vtkSlicerVirtualRealityLogic* vrLogic = vtkSlicerVirtualRealityLogic::SafeDownCast(this->logic());
  vtkMRMLVirtualRealityViewNode* vrViewNode = vrLogic->GetVirtualRealityViewNode();
  if (vrViewNode)
  {
    vrViewNode->SetBackLights(active);
  }
}

//-----------------------------------------------------------------------------
void qSlicerVirtualRealityModuleWidget::setReferenceViewNode(vtkMRMLNode* node)
{
  Q_D(qSlicerVirtualRealityModuleWidget);
  vtkSlicerVirtualRealityLogic* vrLogic = vtkSlicerVirtualRealityLogic::SafeDownCast(this->logic());
  vtkMRMLVirtualRealityViewNode* vrViewNode = vrLogic->GetVirtualRealityViewNode();
  if (vrViewNode)
  {
    vtkMRMLViewNode* referenceViewNode = vtkMRMLViewNode::SafeDownCast(node);
    vrViewNode->SetAndObserveReferenceViewNode(referenceViewNode);
  }
}

//-----------------------------------------------------------------------------
void qSlicerVirtualRealityModuleWidget::updateViewFromReferenceViewCamera()
{
  Q_D(qSlicerVirtualRealityModuleWidget);
  qSlicerVirtualRealityModule* vrModule = dynamic_cast<qSlicerVirtualRealityModule*>(this->module());
  if (!vrModule)
  {
    return;
  }
  qMRMLVirtualRealityView* vrView = vrModule->viewWidget();
  if (!vrView)
  {
    return;
  }
  vrView->updateViewFromReferenceViewCamera();
}



//-----------------------------------------------------------------------------
void qSlicerVirtualRealityModuleWidget::onDesiredUpdateRateChanged(double fps)
{
  Q_D(qSlicerVirtualRealityModuleWidget);
  vtkSlicerVirtualRealityLogic* vrLogic = vtkSlicerVirtualRealityLogic::SafeDownCast(this->logic());
  vtkMRMLVirtualRealityViewNode* vrViewNode = vrLogic->GetVirtualRealityViewNode();
  if (vrViewNode)
  {
    vrViewNode->SetDesiredUpdateRate(fps);
  }
}
