/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Need to be included before qMRMLVRView_p
#include <vtkOpenVRRenderer.h>
#include <vtkOpenVRRenderWindow.h>
#include <vtkOpenVRRenderWindowInteractor.h>
#include <vtkOpenVRCamera.h>

#include "qMRMLVirtualRealityView_p.h"

// Qt includes
#include <QCoreApplication>
#include <QDebug>
#include <QEvent>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QTimer>

// CTK includes
#include <ctkAxesWidget.h>
#include <ctkPimpl.h>

// qMRML includes
#include "qMRMLColors.h"
#include "qMRMLThreeDView.h"
#include "qMRMLThreeDWidget.h"

// Slicer includes
#include "qSlicerApplication.h"
#include "qSlicerLayoutManager.h"
#include "vtkSlicerConfigure.h" // For Slicer_USE_OpenVR

// MRMLDisplayableManager includes
#include <vtkMRMLAbstractDisplayableManager.h>
#include <vtkMRMLDisplayableManagerGroup.h>
#include <vtkMRMLVirtualRealityViewDisplayableManagerFactory.h>
#include <vtkThreeDViewInteractorStyle.h>

// MRML includes
#include <vtkMRMLVirtualRealityViewNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLCameraNode.h>

// VTK includes
#include <vtkCollection.h>
#include <vtkCullerCollection.h>
#include <vtkLight.h>
#include <vtkNew.h>
#include <vtkOpenGLFramebufferObject.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkSmartPointer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>

#include "qSlicerApplication.h"

//--------------------------------------------------------------------------
// qMRMLVirtualRealityViewPrivate methods

//---------------------------------------------------------------------------
qMRMLVirtualRealityViewPrivate::qMRMLVirtualRealityViewPrivate(qMRMLVirtualRealityView& object)
  : q_ptr(&object)
{
  this->DisplayableManagerGroup = 0;
  this->MRMLScene = 0;
  this->MRMLVirtualRealityViewNode = 0;

//#ifdef Slicer_USE_OpenVR
  this->Renderer = vtkSmartPointer<vtkOpenVRRenderer>::New();
  this->RenderWindow = vtkSmartPointer<vtkOpenVRRenderWindow>::New();
  this->Interactor = vtkSmartPointer<vtkOpenVRRenderWindowInteractor>::New();
  this->Camera = vtkSmartPointer<vtkOpenVRCamera>::New();
  this->RenderWindow->AddRenderer(this->Renderer);
  this->RenderWindow->SetInteractor(this->Interactor);
  this->Renderer->SetActiveCamera(this->Camera);
//#endif
}

//---------------------------------------------------------------------------
qMRMLVirtualRealityViewPrivate::~qMRMLVirtualRealityViewPrivate()
{
  if (this->DisplayableManagerGroup)
    {
    this->DisplayableManagerGroup->Delete();
    }
}

//---------------------------------------------------------------------------
void qMRMLVirtualRealityViewPrivate::init()
{
  this->initDisplayableManagers();
}

//----------------------------------------------------------------------------
CTK_SET_CPP(qMRMLVirtualRealityView, bool, setRenderEnabled, RenderEnabled);
CTK_GET_CPP(qMRMLVirtualRealityView, bool, renderEnabled, RenderEnabled);

//----------------------------------------------------------------------------
CTK_GET_CPP(qMRMLVirtualRealityView, vtkOpenVRRenderer*, renderer, Renderer);

//----------------------------------------------------------------------------
CTK_GET_CPP(qMRMLVirtualRealityView, vtkOpenVRRenderWindow*, renderWindow, RenderWindow);

//----------------------------------------------------------------------------
CTK_GET_CPP(qMRMLVirtualRealityView, vtkOpenVRRenderWindowInteractor*, interactor, Interactor);

//---------------------------------------------------------------------------
void qMRMLVirtualRealityViewPrivate::initDisplayableManagers()
{
  Q_Q(qMRMLVirtualRealityView);
  vtkMRMLVirtualRealityViewDisplayableManagerFactory* factory
    = vtkMRMLVirtualRealityViewDisplayableManagerFactory::GetInstance();

  this->RenderWindow->SetMultiSamples(0);

  QStringList displayableManagers;
  displayableManagers //<< "vtkMRMLCameraDisplayableManager"
                      //<< "vtkMRMLViewDisplayableManager"
                      << "vtkMRMLModelDisplayableManager"
                      << "vtkMRMLThreeDReformatDisplayableManager"
                      << "vtkMRMLCrosshairDisplayableManager3D"
                      << "vtkMRMLOrientationMarkerDisplayableManager"
                      << "vtkMRMLRulerDisplayableManager"
                      << "vtkMRMLAnnotationDisplayableManager"
                      << "vtkMRMLMarkupsFiducialDisplayableManager3D"
                      << "vtkMRMLSegmentationsDisplayableManager3D"
                      << "vtkMRMLTransformsDisplayableManager3D"
                      << "vtkMRMLLinearTransformsDisplayableManager3D"
                      << "vtkMRMLVolumeRenderingDisplayableManager"
                      ;
  foreach(const QString& displayableManager, displayableManagers)
    {
    if(!factory->IsDisplayableManagerRegistered(displayableManager.toLatin1()))
      {
      factory->RegisterDisplayableManager(displayableManager.toLatin1());
      }
    }

  this->DisplayableManagerGroup
    = factory->InstantiateDisplayableManagers(q->renderer());

  qDebug() << "this->DisplayableManagerGroup" << this->DisplayableManagerGroup->GetDisplayableManagerCount();

  ///CONFIGURATION OF THE OPENVR ENVIRONEMENT

  this->Renderer->RemoveCuller(this->Renderer->GetCullers()->GetLastItem());
  this->Renderer->SetBackground(0.7, 0.7, 0.7);

  // create 4 lights for even lighting
  {
    vtkNew<vtkLight> light;
    light->SetPosition(0.0, 1.0, 0.0);
    light->SetIntensity(1.0);
    light->SetLightTypeToSceneLight();
    this->Renderer->AddLight(light.GetPointer());
  }
  {
    vtkNew<vtkLight> light;
    light->SetPosition(0.8, -0.2, 0.0);
    light->SetIntensity(0.8);
    light->SetLightTypeToSceneLight();
    this->Renderer->AddLight(light.GetPointer());
  }
  {
    vtkNew<vtkLight> light;
    light->SetPosition(-0.3, -0.2, 0.7);
    light->SetIntensity(0.6);
    light->SetLightTypeToSceneLight();
    this->Renderer->AddLight(light.GetPointer());
  }
  {
    vtkNew<vtkLight> light;
    light->SetPosition(-0.3, -0.2, -0.7);
    light->SetIntensity(0.4);
    light->SetLightTypeToSceneLight();
    this->Renderer->AddLight(light.GetPointer());
  }

  this->Renderer->SetBackground(0, 0, 1);

  vtkRenderWindow* refRenderWindow = qSlicerApplication::application()->layoutManager()->threeDWidget(0)->threeDView()->renderWindow();
  this->RenderWindow->InitializeViewFromCamera(refRenderWindow->GetRenderers()->GetFirstRenderer()->GetActiveCamera());

  this->Renderer->SetGradientBackground(1);
  this->Renderer->SetBackground2(0.7, 0.7, 0.7);
  this->Renderer->SetBackground(0, 0, 1);

  this->Renderer->SetTwoSidedLighting(0);

  this->RenderWindow->Initialize();
  if (!this->RenderWindow->GetHMD())
    {
    qWarning() << "Failed to initialize OpenVR RenderWindow";
    return;
    }
  std::cout << "Loop connection" << std::endl;
  QObject::connect(&this->VirtualRealityLoopTimer, SIGNAL(timeout()), this, SLOT(doOpenVirtualReality()));
}

//---------------------------------------------------------------------------
void qMRMLVirtualRealityViewPrivate::setMRMLScene(vtkMRMLScene* newScene)
{
  Q_Q(qMRMLVirtualRealityView);
  if (newScene == this->MRMLScene)
    {
    return;
    }

  this->qvtkReconnect(
    this->MRMLScene, newScene,
    vtkMRMLScene::StartBatchProcessEvent, this, SLOT(onSceneStartProcessing()));

  this->qvtkReconnect(
    this->MRMLScene, newScene,
    vtkMRMLScene::EndBatchProcessEvent, this, SLOT(onSceneEndProcessing()));

  this->MRMLScene = newScene;
  q->setRenderEnabled(
    this->MRMLScene != 0 && !this->MRMLScene->IsBatchProcessing());
}

// --------------------------------------------------------------------------
void qMRMLVirtualRealityViewPrivate::onSceneStartProcessing()
{
  Q_Q(qMRMLVirtualRealityView);
  q->setRenderEnabled(false);
}

//
// --------------------------------------------------------------------------
void qMRMLVirtualRealityViewPrivate::onSceneEndProcessing()
{
  Q_Q(qMRMLVirtualRealityView);
  q->setRenderEnabled(true);
}

// --------------------------------------------------------------------------
void qMRMLVirtualRealityViewPrivate::updateWidgetFromMRML()
{
  Q_Q(qMRMLVirtualRealityView);
  if (!this->MRMLVirtualRealityViewNode)
    {
    return;
    }
}

// --------------------------------------------------------------------------
void qMRMLVirtualRealityViewPrivate::doOpenVirtualReality()
{
  this->Interactor->DoOneEvent(this->RenderWindow, this->Renderer);
}

// --------------------------------------------------------------------------
// qMRMLVirtualRealityView methods

// --------------------------------------------------------------------------
qMRMLVirtualRealityView::qMRMLVirtualRealityView(QWidget* _parent) : Superclass(_parent)
  , d_ptr(new qMRMLVirtualRealityViewPrivate(*this))
{
  Q_D(qMRMLVirtualRealityView);
  d->init();
}

// --------------------------------------------------------------------------
qMRMLVirtualRealityView::~qMRMLVirtualRealityView()
{
}

//------------------------------------------------------------------------------
void qMRMLVirtualRealityView::addDisplayableManager(const QString& displayableManagerName)
{
  Q_D(qMRMLVirtualRealityView);
  vtkSmartPointer<vtkMRMLAbstractDisplayableManager> displayableManager;
  displayableManager.TakeReference(
    vtkMRMLDisplayableManagerGroup::InstantiateDisplayableManager(
      displayableManagerName.toLatin1()));
  d->DisplayableManagerGroup->AddDisplayableManager(displayableManager);
}

//----------------------------------------------------------------------------
void qMRMLVirtualRealityView::startVirtualReality()
{
  Q_D(qMRMLVirtualRealityView);
  qDebug() << Q_FUNC_INFO << "Start VirtualReality";
  d->VirtualRealityLoopTimer.start(0);
}

//----------------------------------------------------------------------------
void qMRMLVirtualRealityView::stopVirtualReality()
{
  Q_D(qMRMLVirtualRealityView);
  qDebug() << Q_FUNC_INFO << "Stop VirtualReality";
  d->VirtualRealityLoopTimer.stop();
}

//------------------------------------------------------------------------------
void qMRMLVirtualRealityView::setMRMLScene(vtkMRMLScene* newScene)
{
  Q_D(qMRMLVirtualRealityView);
  d->setMRMLScene(newScene);

  if (d->MRMLVirtualRealityViewNode && newScene != d->MRMLVirtualRealityViewNode->GetScene())
    {
    this->setMRMLVirtualRealityViewNode(0);
    }
}

//---------------------------------------------------------------------------
void qMRMLVirtualRealityView::setMRMLVirtualRealityViewNode(vtkMRMLVirtualRealityViewNode* newViewNode)
{
  Q_D(qMRMLVirtualRealityView);
  if (d->MRMLVirtualRealityViewNode == newViewNode)
    {
    return;
    }

  d->qvtkReconnect(
    d->MRMLVirtualRealityViewNode, newViewNode,
    vtkCommand::ModifiedEvent, d, SLOT(updateWidgetFromMRML()));

  d->MRMLVirtualRealityViewNode = newViewNode;
  d->DisplayableManagerGroup->SetMRMLDisplayableNode(newViewNode);

  d->updateWidgetFromMRML();

  // Enable/disable widget
  this->setEnabled(newViewNode != 0);
}

//---------------------------------------------------------------------------
vtkMRMLVirtualRealityViewNode* qMRMLVirtualRealityView::mrmlVirtualRealityViewNode()const
{
  Q_D(const qMRMLVirtualRealityView);
  return d->MRMLVirtualRealityViewNode;
}

//------------------------------------------------------------------------------
void qMRMLVirtualRealityView::getDisplayableManagers(vtkCollection *displayableManagers)
{
  Q_D(qMRMLVirtualRealityView);

  if (!displayableManagers || !d->DisplayableManagerGroup)
    {
    return;
    }
  int num = d->DisplayableManagerGroup->GetDisplayableManagerCount();
  for (int n = 0; n < num; n++)
    {
    displayableManagers->AddItem(d->DisplayableManagerGroup->GetNthDisplayableManager(n));
    }
}
