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

//MRML includes
#include "vtkMRMLScene.h"

//VirtualReality includes
#include "vtkMRMLVirtualRealityViewNode.h"
#include "vtkMRMLVirtualRealityViewDisplayableManagerFactory.h"
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
  : Superclass( _parent )
  , d_ptr( new qSlicerVirtualRealityModuleWidgetPrivate )
  , VirtualRealityWidget(NULL)
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

  connect(d->initializePushButton, SIGNAL(clicked()), this, SLOT(initializeVirtualReality()));
  connect(d->startVirtualRealityPushButton, SIGNAL(clicked()), this, SLOT(startVirtualReality()));
  connect(d->stopVirtualRealityPushButton, SIGNAL(clicked()), this, SLOT(stopVirtualReality()));
}

//-----------------------------------------------------------------------------
qMRMLVirtualRealityView* qSlicerVirtualRealityModuleWidget::vrWidget() const
{
  return this->VirtualRealityWidget;
}

//-----------------------------------------------------------------------------
void qSlicerVirtualRealityModuleWidget::initializeVirtualReality()
{
  Q_D(qSlicerVirtualRealityModuleWidget);
  qDebug() << Q_FUNC_INFO << ": Initialize OpenVirtualReality";

  if (this->VirtualRealityWidget && this->mrmlScene()->GetNumberOfNodesByClass("vtkMRMLVirtualRealityViewNode") > 0)
    {
    qDebug() << Q_FUNC_INFO << ": OpenVirtualReality already initialized";
    return;
    }

  // Register VirtualReality view node class
  this->mrmlScene()->RegisterNodeClass((vtkSmartPointer<vtkMRMLVirtualRealityViewNode>::New()));

  // Register displayable managers to VirtualReality view displayable manager factory
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
    vtkMRMLVirtualRealityViewDisplayableManagerFactory::GetInstance()->RegisterDisplayableManager(name.toLatin1());
    }

  // Create VirtualReality view node
  vtkNew<vtkMRMLVirtualRealityViewNode> vrViewNode;
  this->mrmlScene()->AddNode(vrViewNode.GetPointer());

  // Setup VirtualReality view widget
  this->VirtualRealityWidget = new qMRMLVirtualRealityView();
  this->VirtualRealityWidget->setObjectName(QString("VirtualRealityWidget"));
  this->VirtualRealityWidget->setMRMLScene(this->mrmlScene());
  this->VirtualRealityWidget->setMRMLVirtualRealityViewNode(vrViewNode.GetPointer());
}

//-----------------------------------------------------------------------------
void qSlicerVirtualRealityModuleWidget::startVirtualReality()
{
  qDebug() << Q_FUNC_INFO << ": Start VirtualReality";
  this->VirtualRealityWidget->startVirtualReality();
}

//-----------------------------------------------------------------------------
void qSlicerVirtualRealityModuleWidget::stopVirtualReality()
{
  qDebug() << Q_FUNC_INFO << ": Stop VirtualReality";
  this->VirtualRealityWidget->stopVirtualReality();
}
