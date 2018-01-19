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

#include "qMRMLVRView_p.h"

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
#include <vtkMRMLVRViewDisplayableManagerFactory.h>
#include <vtkThreeDViewInteractorStyle.h>

// MRML includes
#include <vtkMRMLVRViewNode.h>
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
// qMRMLVRViewPrivate methods

//---------------------------------------------------------------------------
qMRMLVRViewPrivate::qMRMLVRViewPrivate(qMRMLVRView& object)
  : q_ptr(&object)
{
  this->DisplayableManagerGroup = 0;
  this->MRMLScene = 0;
  this->MRMLVRViewNode = 0;

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
qMRMLVRViewPrivate::~qMRMLVRViewPrivate()
{
  if (this->DisplayableManagerGroup)
    {
    this->DisplayableManagerGroup->Delete();
    }
}

//---------------------------------------------------------------------------
void qMRMLVRViewPrivate::init()
{
  this->initDisplayableManagers();
}

//----------------------------------------------------------------------------
CTK_SET_CPP(qMRMLVRView, bool, setRenderEnabled, RenderEnabled);
CTK_GET_CPP(qMRMLVRView, bool, renderEnabled, RenderEnabled);

//----------------------------------------------------------------------------
CTK_GET_CPP(qMRMLVRView, vtkOpenVRRenderer*, renderer, Renderer);

//----------------------------------------------------------------------------
CTK_GET_CPP(qMRMLVRView, vtkOpenVRRenderWindow*, renderWindow, RenderWindow);

//----------------------------------------------------------------------------
CTK_GET_CPP(qMRMLVRView, vtkOpenVRRenderWindowInteractor*, interactor, Interactor);

//---------------------------------------------------------------------------
void qMRMLVRViewPrivate::initDisplayableManagers()
{
  Q_Q(qMRMLVRView);
  vtkMRMLVRViewDisplayableManagerFactory* factory
    = vtkMRMLVRViewDisplayableManagerFactory::GetInstance();

  this->RenderWindow->SetMultiSamples(0);

  QStringList displayableManagers;
  displayableManagers //<< "vtkMRMLCameraDisplayableManager"
                      //<< "vtkMRMLViewDisplayableManager"
                      << "vtkMRMLModelDisplayableManager"
                      << "vtkMRMLThreeDReformatDisplayableManager"
                      << "vtkMRMLCrosshairDisplayableManager3D"
                      << "vtkMRMLOrientationMarkerDisplayableManager"
                      << "vtkMRMLRulerDisplayableManager"
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

  // create 2 lights
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

  this->Renderer->SetBackground(0, 0, 1);

  vtkRenderWindow* refRenderWindow = qSlicerApplication::application()->layoutManager()->threeDWidget(0)->threeDView()->renderWindow();
  this->RenderWindow->InitializeViewFromCamera(refRenderWindow->GetRenderers()->GetFirstRenderer()->GetActiveCamera());

  this->Renderer->SetGradientBackground(1);
  this->Renderer->SetBackground2(0.7, 0.7, 0.7);
  this->Renderer->SetBackground(0, 0, 1);

  this->RenderWindow->Initialize();
  if (!this->RenderWindow->GetHMD())
    {
    qWarning() << "Failed to initialize OpenVR RenderWindow";
    return;
    }
  std::cout << "Loop connection" << std::endl;
  QObject::connect(&this->VRLoopTimer, SIGNAL(timeout()), this, SLOT(doOpenVR()));
}

//---------------------------------------------------------------------------
void qMRMLVRViewPrivate::setMRMLScene(vtkMRMLScene* newScene)
{
  Q_Q(qMRMLVRView);
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
void qMRMLVRViewPrivate::onSceneStartProcessing()
{
  Q_Q(qMRMLVRView);
  q->setRenderEnabled(false);
}

//
// --------------------------------------------------------------------------
void qMRMLVRViewPrivate::onSceneEndProcessing()
{
  Q_Q(qMRMLVRView);
  q->setRenderEnabled(true);
}

// --------------------------------------------------------------------------
void qMRMLVRViewPrivate::updateWidgetFromMRML()
{
  Q_Q(qMRMLVRView);
  if (!this->MRMLVRViewNode)
    {
    return;
    }
}

// --------------------------------------------------------------------------
void qMRMLVRViewPrivate::doOpenVR()
{
  this->Interactor->DoOneEvent(this->RenderWindow, this->Renderer);
}

// --------------------------------------------------------------------------
// qMRMLVRView methods

// --------------------------------------------------------------------------
qMRMLVRView::qMRMLVRView(QWidget* _parent) : Superclass(_parent)
  , d_ptr(new qMRMLVRViewPrivate(*this))
{
  Q_D(qMRMLVRView);
  d->init();
}

// --------------------------------------------------------------------------
qMRMLVRView::~qMRMLVRView()
{
}

//------------------------------------------------------------------------------
void qMRMLVRView::addDisplayableManager(const QString& displayableManagerName)
{
  Q_D(qMRMLVRView);
  vtkSmartPointer<vtkMRMLAbstractDisplayableManager> displayableManager;
  displayableManager.TakeReference(
    vtkMRMLDisplayableManagerGroup::InstantiateDisplayableManager(
      displayableManagerName.toLatin1()));
  d->DisplayableManagerGroup->AddDisplayableManager(displayableManager);
}

//----------------------------------------------------------------------------
void qMRMLVRView::startVR()
{
  Q_D(qMRMLVRView);
  qDebug() << "startVR";
  d->VRLoopTimer.start(0);
}

//----------------------------------------------------------------------------
void qMRMLVRView::stopVR()
{
  Q_D(qMRMLVRView);
  qDebug() << "stopVR";
  d->VRLoopTimer.stop();
}

//------------------------------------------------------------------------------
void qMRMLVRView::setMRMLScene(vtkMRMLScene* newScene)
{
  Q_D(qMRMLVRView);
  d->setMRMLScene(newScene);

  if (d->MRMLVRViewNode && newScene != d->MRMLVRViewNode->GetScene())
    {
    this->setMRMLVRViewNode(0);
    }
}

//---------------------------------------------------------------------------
void qMRMLVRView::setMRMLVRViewNode(vtkMRMLVRViewNode* newViewNode)
{
  Q_D(qMRMLVRView);
  if (d->MRMLVRViewNode == newViewNode)
    {
    return;
    }


  d->qvtkReconnect(
    d->MRMLVRViewNode, newViewNode,
    vtkCommand::ModifiedEvent, d, SLOT(updateWidgetFromMRML()));

  d->MRMLVRViewNode = newViewNode;
  d->DisplayableManagerGroup->SetMRMLDisplayableNode(newViewNode);

  d->updateWidgetFromMRML();
  // Enable/disable widget
  this->setEnabled(newViewNode != 0);
}

//---------------------------------------------------------------------------
vtkMRMLVRViewNode* qMRMLVRView::mrmlVRViewNode()const
{
  Q_D(const qMRMLVRView);
  return d->MRMLVRViewNode;
}

//------------------------------------------------------------------------------
void qMRMLVRView::getDisplayableManagers(vtkCollection *displayableManagers)
{
  Q_D(qMRMLVRView);

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
