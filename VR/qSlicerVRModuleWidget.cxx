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
#include "qSlicerVRModuleWidget.h"
#include "ui_qSlicerVRModuleWidget.h"

//MRML includes
#include "vtkMRMLScene.h"

//VR includes
#include "vtkMRMLVRViewNode.h"
#include "vtkMRMLVRViewDisplayableManagerFactory.h"
#include "qMRMLVRView.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerVRModuleWidgetPrivate: public Ui_qSlicerVRModuleWidget
{
public:
  qSlicerVRModuleWidgetPrivate();
};

//-----------------------------------------------------------------------------
// qSlicerVRModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerVRModuleWidgetPrivate::qSlicerVRModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerVRModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerVRModuleWidget::qSlicerVRModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerVRModuleWidgetPrivate )
  , VRWidget(NULL)
{
}

//-----------------------------------------------------------------------------
qSlicerVRModuleWidget::~qSlicerVRModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerVRModuleWidget::setup()
{
  Q_D(qSlicerVRModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();

  connect(d->initializePushButton, SIGNAL(clicked()), this, SLOT(initializeVR()));
  connect(d->startVRPushButton, SIGNAL(clicked()), this, SLOT(startVR()));
  connect(d->stopVRPushButton, SIGNAL(clicked()), this, SLOT(stopVR()));
}

//-----------------------------------------------------------------------------
qMRMLVRView* qSlicerVRModuleWidget::vrWidget() const
{
  return this->VRWidget;
}

//-----------------------------------------------------------------------------
void qSlicerVRModuleWidget::initializeVR()
{
  Q_D(qSlicerVRModuleWidget);
  qDebug() << Q_FUNC_INFO << ": Initialize OpenVR";

  // Register VR view node class
  this->mrmlScene()->RegisterNodeClass((vtkSmartPointer<vtkMRMLVRViewNode>::New()));

  // Register displayable managers to VR view displayable manager factory
  foreach(const QString& name,
    QStringList()
      << "vtkMRMLAnnotationDisplayableManager"
      << "vtkMRMLMarkupsFiducialDisplayableManager3D"
      << "vtkMRMLSegmentationsDisplayableManager3D"
      << "vtkMRMLTransformsDisplayableManager3D"
      << "vtkMRMLLinearTransformsDisplayableManager3D"
      << "vtkMRMLVolumeRenderingDisplayableManager"
  )
    {
    vtkMRMLVRViewDisplayableManagerFactory::GetInstance()->RegisterDisplayableManager(name.toLatin1());
    }

  // Create VR view node
  vtkNew<vtkMRMLVRViewNode> vrViewNode;
  this->mrmlScene()->AddNode(vrViewNode.GetPointer());

  // Setup VR view widget
  this->VRWidget = new qMRMLVRView();
  this->VRWidget->setObjectName(QString("VRWidget"));
  this->VRWidget->setMRMLScene(this->mrmlScene());
  this->VRWidget->setMRMLVRViewNode(vrViewNode.GetPointer());
}

//-----------------------------------------------------------------------------
void qSlicerVRModuleWidget::startVR()
{
  qDebug() << Q_FUNC_INFO << ": Start VR";
  this->VRWidget->startVR();
}

//-----------------------------------------------------------------------------
void qSlicerVRModuleWidget::stopVR()
{
  qDebug() << Q_FUNC_INFO << ": Stop VR";
  this->VRWidget->stopVR();
}
