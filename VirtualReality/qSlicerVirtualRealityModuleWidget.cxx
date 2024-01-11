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

// For:
//  - SlicerVirtualReality_HAS_OPENVR_SUPPORT
//  - SlicerVirtualReality_HAS_OPENXR_SUPPORT
#include "vtkMRMLVirtualRealityConfigure.h"

// Qt includes
#include <QDebug>

// CTK includes
#include <ctkDoubleSpinBox.h>

// VR includes
#include "qSlicerVirtualRealityModule.h"
#include "qSlicerVirtualRealityModuleWidget.h"
#include "ui_qSlicerVirtualRealityModuleWidget.h"

// VR Logic includes
#include "vtkSlicerVirtualRealityLogic.h"

// VR MRML includes
#include "vtkMRMLVirtualRealityViewNode.h"

// VR widgets includes
#include "qMRMLVirtualRealityView.h"

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
  : Superclass(_parent)
  , d_ptr(new qSlicerVirtualRealityModuleWidgetPrivate)
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

  d->XRRuntimeComboBox->addItem(
        vtkMRMLVirtualRealityViewNode::GetXRRuntimeAsString(vtkMRMLVirtualRealityViewNode::UndefinedXRRuntime),
        vtkMRMLVirtualRealityViewNode::UndefinedXRRuntime);
#if defined(SlicerVirtualReality_HAS_OPENVR_SUPPORT)
  d->XRRuntimeComboBox->addItem(
        vtkMRMLVirtualRealityViewNode::GetXRRuntimeAsString(vtkMRMLVirtualRealityViewNode::OpenVR),
        vtkMRMLVirtualRealityViewNode::OpenVR);
#endif
#if defined(SlicerVirtualReality_HAS_OPENXR_SUPPORT)
  d->XRRuntimeComboBox->addItem(
        vtkMRMLVirtualRealityViewNode::GetXRRuntimeAsString(vtkMRMLVirtualRealityViewNode::OpenXR),
        vtkMRMLVirtualRealityViewNode::OpenXR);
#endif

  connect(d->XRRuntimeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setVirtualRealityXRRuntime(int)));

  connect(d->ConnectCheckBox, SIGNAL(toggled(bool)), this, SLOT(setVirtualRealityConnected(bool)));
  connect(d->RenderingEnabledCheckBox, SIGNAL(toggled(bool)), this, SLOT(setVirtualRealityActive(bool)));
  connect(d->TwoSidedLightingCheckBox, SIGNAL(toggled(bool)), this, SLOT(setTwoSidedLighting(bool)));
  connect(d->BackLightsCheckBox, SIGNAL(toggled(bool)), this, SLOT(setBackLights(bool)));
  connect(d->ControllerModelsVisibleCheckBox, SIGNAL(toggled(bool)), this, SLOT(setControllerModelsVisible(bool)));
  connect(d->LighthousesVisibleCheckBox, SIGNAL(toggled(bool)), this, SLOT(setTrackingReferenceModelsVisible(bool)));
  connect(d->ReferenceViewNodeComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(setReferenceViewNode(vtkMRMLNode*)));
  connect(d->UpdateViewFromReferenceViewCameraButton, SIGNAL(clicked()), this, SLOT(updateViewFromReferenceViewCamera()));

  connect(d->DesiredUpdateRateSlider, SIGNAL(valueChanged(double)), this, SLOT(onDesiredUpdateRateChanged(double)));
  connect(d->MotionSensitivitySlider, SIGNAL(valueChanged(double)), this, SLOT(onMotionSensitivityChanged(double)));
  connect(d->MagnificationSlider, SIGNAL(valueChanged(double)), this, SLOT(onMagnificationChanged(double)));
  connect(d->MotionSpeedSlider, SIGNAL(valueChanged(double)), this, SLOT(onMotionSpeedChanged(double)));
  connect(d->ControllerTransformsUpdateCheckBox, SIGNAL(toggled(bool)), this, SLOT(setControllerTransformsUpdate(bool)));
  connect(d->HMDTransformCheckBox, SIGNAL(toggled(bool)), this, SLOT(setHMDTransformUpdate(bool)));
  connect(d->TrackerTransformsCheckBox, SIGNAL(toggled(bool)), this, SLOT(setTrackerTransformsUpdate(bool)));


  // Make magnification label same width as motion sensitivity spinbox
  ctkDoubleSpinBox* motionSpeedSpinBox = d->MotionSpeedSlider->findChild<ctkDoubleSpinBox*>("SpinBox");
  d->MagnificationValueLabel->setMinimumWidth(motionSpeedSpinBox->sizeHint().width());

  this->updateWidgetFromMRML();

  // If virtual reality logic is modified it indicates that the view node may have changed
  qvtkConnect(logic(), vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromMRML()));
}

//--------------------------------------------------------------------------
void qSlicerVirtualRealityModuleWidget::updateWidgetFromMRML()
{
  Q_D(qSlicerVirtualRealityModuleWidget);
  vtkSlicerVirtualRealityLogic* vrLogic = vtkSlicerVirtualRealityLogic::SafeDownCast(this->logic());
  vtkMRMLVirtualRealityViewNode* vrViewNode = vrLogic->GetVirtualRealityViewNode();

  bool wasBlocked = d->ConnectCheckBox->blockSignals(true);
  d->ConnectCheckBox->setChecked(vrViewNode != nullptr && vrViewNode->GetVisibility());
  d->ConnectCheckBox->blockSignals(wasBlocked);

  wasBlocked = d->XRRuntimeComboBox->blockSignals(true);
  d->XRRuntimeComboBox->setCurrentIndex(
        d->XRRuntimeComboBox->findData(
          vrViewNode != nullptr ? vrViewNode->GetXRRuntime() : vtkMRMLVirtualRealityViewNode::UndefinedXRRuntime));
  d->XRRuntimeComboBox->setEnabled(
        (vrViewNode != nullptr)
        && !vrLogic->GetVirtualRealityConnected());
  d->XRRuntimeComboBox->blockSignals(wasBlocked);

  QString errorText;
  if (vrViewNode && vrViewNode->HasError())
  {
    errorText = vrViewNode->GetError().c_str();
  }
  d->ConnectionStatusLabel->setText(errorText);

  wasBlocked = d->RenderingEnabledCheckBox->blockSignals(true);
  d->RenderingEnabledCheckBox->setChecked(vrViewNode != nullptr && vrViewNode->GetActive());
  d->RenderingEnabledCheckBox->blockSignals(wasBlocked);

  wasBlocked = d->DesiredUpdateRateSlider->blockSignals(true);
  d->DesiredUpdateRateSlider->setValue(vrViewNode != nullptr ? vrViewNode->GetDesiredUpdateRate() : 0);
  d->DesiredUpdateRateSlider->setEnabled(vrViewNode != nullptr);
  d->DesiredUpdateRateSlider->blockSignals(wasBlocked);

  wasBlocked = d->MagnificationSlider->blockSignals(true);
  d->MagnificationSlider->setValue(vrViewNode != nullptr
                                   ? this->getPowerFromMagnification(vrViewNode->GetMagnification())
                                   : 1.0);
  d->MagnificationValueLabel->setText(vrViewNode != nullptr
                                      ? QString("%1x").arg(vrViewNode->GetMagnification(), 3, 'f', 2)
                                      : "10.0");
  d->MagnificationSlider->setEnabled(vrViewNode != nullptr);
  d->MagnificationSlider->blockSignals(wasBlocked);

  wasBlocked = d->MotionSensitivitySlider->blockSignals(true);
  d->MotionSensitivitySlider->setValue(vrViewNode != nullptr ? vrViewNode->GetMotionSensitivity() * 100.0 : 0);
  d->MotionSensitivitySlider->setEnabled(vrViewNode != nullptr);
  d->MotionSensitivitySlider->blockSignals(wasBlocked);

  wasBlocked = d->MotionSpeedSlider->blockSignals(true);
  d->MotionSpeedSlider->setValue(vrViewNode != nullptr ? vrViewNode->GetMotionSpeed() : 1.6666);
  d->MotionSpeedSlider->setEnabled(vrViewNode != nullptr);
  d->MotionSpeedSlider->blockSignals(wasBlocked);

  wasBlocked = d->TwoSidedLightingCheckBox->blockSignals(true);
  d->TwoSidedLightingCheckBox->setChecked(vrViewNode != nullptr && vrViewNode->GetTwoSidedLighting());
  d->TwoSidedLightingCheckBox->setEnabled(vrViewNode != nullptr);
  d->TwoSidedLightingCheckBox->blockSignals(wasBlocked);

  wasBlocked = d->BackLightsCheckBox->blockSignals(true);
  d->BackLightsCheckBox->setChecked(vrViewNode != nullptr && vrViewNode->GetBackLights());
  d->BackLightsCheckBox->setEnabled(vrViewNode != nullptr);
  d->BackLightsCheckBox->blockSignals(wasBlocked);

  wasBlocked = d->ControllerModelsVisibleCheckBox->blockSignals(true);
  d->ControllerModelsVisibleCheckBox->setChecked(vrViewNode != nullptr && vrViewNode->GetControllerModelsVisible());
  d->ControllerModelsVisibleCheckBox->setEnabled(vrViewNode != nullptr);
  d->ControllerModelsVisibleCheckBox->blockSignals(wasBlocked);

  wasBlocked = d->LighthousesVisibleCheckBox->blockSignals(true);
  d->LighthousesVisibleCheckBox->setChecked(vrViewNode != nullptr && vrViewNode->GetLighthouseModelsVisible());
  d->LighthousesVisibleCheckBox->setEnabled(vrViewNode != nullptr);
  d->LighthousesVisibleCheckBox->blockSignals(wasBlocked);

  wasBlocked = d->ReferenceViewNodeComboBox->blockSignals(true);
  d->ReferenceViewNodeComboBox->setCurrentNode(vrViewNode != nullptr ? vrViewNode->GetReferenceViewNode() : nullptr);
  d->ReferenceViewNodeComboBox->blockSignals(wasBlocked);
  d->ReferenceViewNodeComboBox->setEnabled(vrViewNode != nullptr);

  wasBlocked = d->ControllerTransformsUpdateCheckBox->blockSignals(true);
  d->ControllerTransformsUpdateCheckBox->setChecked(vrViewNode != nullptr && vrViewNode->GetControllerTransformsUpdate());
  d->ControllerTransformsUpdateCheckBox->setEnabled(vrViewNode != nullptr);
  d->ControllerTransformsUpdateCheckBox->blockSignals(wasBlocked);

  wasBlocked = d->HMDTransformCheckBox->blockSignals(true);
  d->HMDTransformCheckBox->setChecked(vrViewNode != nullptr && vrViewNode->GetHMDTransformUpdate());
  d->HMDTransformCheckBox->setEnabled(vrViewNode != nullptr);
  d->HMDTransformCheckBox->blockSignals(wasBlocked);

  wasBlocked = d->TrackerTransformsCheckBox->blockSignals(true);
  d->TrackerTransformsCheckBox->setChecked(vrViewNode != nullptr && vrViewNode->GetTrackerTransformUpdate());
  d->TrackerTransformsCheckBox->setEnabled(vrViewNode != nullptr);
  d->TrackerTransformsCheckBox->blockSignals(wasBlocked);

  d->UpdateViewFromReferenceViewCameraButton->setEnabled(vrViewNode != nullptr
      && vrViewNode->GetReferenceViewNode() != nullptr);
}


//-----------------------------------------------------------------------------
void qSlicerVirtualRealityModuleWidget::setVirtualRealityXRRuntime(int index)
{
  vtkSlicerVirtualRealityLogic* vrLogic = vtkSlicerVirtualRealityLogic::SafeDownCast(this->logic());
  vrLogic->SetVirtualRealityXRRuntime(index);
}

//-----------------------------------------------------------------------------
void qSlicerVirtualRealityModuleWidget::setVirtualRealityConnected(bool connect)
{
  vtkSlicerVirtualRealityLogic* vrLogic = vtkSlicerVirtualRealityLogic::SafeDownCast(this->logic());
  vrLogic->SetVirtualRealityConnected(connect);
}

//-----------------------------------------------------------------------------
void qSlicerVirtualRealityModuleWidget::setVirtualRealityActive(bool activate)
{
  vtkSlicerVirtualRealityLogic* vrLogic = vtkSlicerVirtualRealityLogic::SafeDownCast(this->logic());
  vrLogic->SetVirtualRealityActive(activate);
}

//-----------------------------------------------------------------------------
void qSlicerVirtualRealityModuleWidget::setTwoSidedLighting(bool active)
{
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
  vtkSlicerVirtualRealityLogic* vrLogic = vtkSlicerVirtualRealityLogic::SafeDownCast(this->logic());
  vtkMRMLVirtualRealityViewNode* vrViewNode = vrLogic->GetVirtualRealityViewNode();
  if (vrViewNode)
  {
    vrViewNode->SetBackLights(active);
  }
}

//-----------------------------------------------------------------------------
void qSlicerVirtualRealityModuleWidget::setControllerModelsVisible(bool visible)
{
  vtkSlicerVirtualRealityLogic* vrLogic = vtkSlicerVirtualRealityLogic::SafeDownCast(this->logic());
  vtkMRMLVirtualRealityViewNode* vrViewNode = vrLogic->GetVirtualRealityViewNode();
  if (vrViewNode)
  {
    vrViewNode->SetControllerModelsVisible(visible);
  }
}

//-----------------------------------------------------------------------------
void qSlicerVirtualRealityModuleWidget::setTrackingReferenceModelsVisible(bool visible)
{
  vtkSlicerVirtualRealityLogic* vrLogic = vtkSlicerVirtualRealityLogic::SafeDownCast(this->logic());
  vtkMRMLVirtualRealityViewNode* vrViewNode = vrLogic->GetVirtualRealityViewNode();
  if (vrViewNode)
  {
    vrViewNode->SetLighthouseModelsVisible(visible);
  }
}

//-----------------------------------------------------------------------------
void qSlicerVirtualRealityModuleWidget::setReferenceViewNode(vtkMRMLNode* node)
{
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
  vtkSlicerVirtualRealityLogic* vrLogic = vtkSlicerVirtualRealityLogic::SafeDownCast(this->logic());
  vtkMRMLVirtualRealityViewNode* vrViewNode = vrLogic->GetVirtualRealityViewNode();
  if (vrViewNode)
  {
    vrViewNode->SetDesiredUpdateRate(fps);
  }
}

//-----------------------------------------------------------------------------
void qSlicerVirtualRealityModuleWidget::onMotionSensitivityChanged(double percent)
{
  vtkSlicerVirtualRealityLogic* vrLogic = vtkSlicerVirtualRealityLogic::SafeDownCast(this->logic());
  vtkMRMLVirtualRealityViewNode* vrViewNode = vrLogic->GetVirtualRealityViewNode();
  if (vrViewNode)
  {
    vrViewNode->SetMotionSensitivity(percent * 0.01);
  }
}

//-----------------------------------------------------------------------------
void qSlicerVirtualRealityModuleWidget::onMotionSpeedChanged(double speedMps)
{
  vtkSlicerVirtualRealityLogic* vrLogic = vtkSlicerVirtualRealityLogic::SafeDownCast(this->logic());
  vtkMRMLVirtualRealityViewNode* vrViewNode = vrLogic->GetVirtualRealityViewNode();
  if (vrViewNode)
  {
    vrViewNode->SetMotionSpeed(speedMps);
  }
}

//----------------------------------------------------------------------------------
void qSlicerVirtualRealityModuleWidget::onMagnificationChanged(double powerOfTen)
{
  Q_D(qSlicerVirtualRealityModuleWidget);

  double magnification = this->getMagnificationFromPower(powerOfTen);

  vtkSlicerVirtualRealityLogic* vrLogic = vtkSlicerVirtualRealityLogic::SafeDownCast(this->logic());
  vtkMRMLVirtualRealityViewNode* vrViewNode = vrLogic->GetVirtualRealityViewNode();
  if (vrViewNode)
  {
    vrViewNode->SetMagnification(magnification);
  }

  d->MagnificationValueLabel->setText(QString("%1x").arg(magnification, 3, 'f', 2));
}

//----------------------------------------------------------------------------------
void qSlicerVirtualRealityModuleWidget::onPhysicalToWorldMatrixModified()
{
  Q_D(qSlicerVirtualRealityModuleWidget);

  vtkSlicerVirtualRealityLogic* vrLogic = vtkSlicerVirtualRealityLogic::SafeDownCast(this->logic());
  vtkMRMLVirtualRealityViewNode* vrViewNode = vrLogic->GetVirtualRealityViewNode();
  if (vrViewNode)
  {
    double magnification = vrViewNode->GetMagnification();
    bool wasBlocked = d->MagnificationSlider->blockSignals(true);
    d->MagnificationSlider->setValue(this->getPowerFromMagnification(magnification));
    d->MagnificationSlider->blockSignals(wasBlocked);
    d->MagnificationValueLabel->setText(QString("%1x").arg(magnification, 3, 'f', 2));
  }
}

//----------------------------------------------------------------------------------
double qSlicerVirtualRealityModuleWidget::getMagnificationFromPower(double powerOfTen)
{
  if (powerOfTen < -2.0)
  {
    powerOfTen = -2.0;
  }
  else if (powerOfTen > 2.0)
  {
    powerOfTen = 2.0;
  }

  return pow(10.0, powerOfTen);
}

//----------------------------------------------------------------------------------
double qSlicerVirtualRealityModuleWidget::getPowerFromMagnification(double magnification)
{
  if (magnification < 0.01)
  {
    magnification = 0.01;
  }
  else if (magnification > 100.0)
  {
    magnification = 100.0;
  }

  return log10(magnification);
}

//-----------------------------------------------------------------------------
void qSlicerVirtualRealityModuleWidget::setControllerTransformsUpdate(bool active)
{
  vtkSlicerVirtualRealityLogic* vrLogic = vtkSlicerVirtualRealityLogic::SafeDownCast(this->logic());
  vtkMRMLVirtualRealityViewNode* vrViewNode = vrLogic->GetVirtualRealityViewNode();
  if (vrViewNode)
  {
    vrViewNode->SetControllerTransformsUpdate(active);
  }
}

//-----------------------------------------------------------------------------
void qSlicerVirtualRealityModuleWidget::setHMDTransformUpdate(bool active)
{
  vtkSlicerVirtualRealityLogic* vrLogic = vtkSlicerVirtualRealityLogic::SafeDownCast(this->logic());
  vtkMRMLVirtualRealityViewNode* vrViewNode = vrLogic->GetVirtualRealityViewNode();
  if (vrViewNode)
  {
    vrViewNode->SetHMDTransformUpdate(active);
  }
}

//----------------------------------------------------------------------------
void qSlicerVirtualRealityModuleWidget::setTrackerTransformsUpdate(bool active)
{
  vtkSlicerVirtualRealityLogic* vrLogic = vtkSlicerVirtualRealityLogic::SafeDownCast(this->logic());
  vtkMRMLVirtualRealityViewNode* vrViewNode = vrLogic->GetVirtualRealityViewNode();
  if (vrViewNode)
  {
    vrViewNode->SetTrackerTransformUpdate(active);
  }
}
