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
#include <vtkSlicerCamerasModuleLogic.h>

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
#include <vtkLightCollection.h>
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
  , CamerasLogic(NULL)
{
  this->MRMLScene = 0;
  this->MRMLVirtualRealityViewNode = 0;
}

//---------------------------------------------------------------------------
qMRMLVirtualRealityViewPrivate::~qMRMLVirtualRealityViewPrivate()
{
}

//---------------------------------------------------------------------------
void qMRMLVirtualRealityViewPrivate::init()
{
  QObject::connect(&this->VirtualRealityLoopTimer, SIGNAL(timeout()), this, SLOT(doOpenVirtualReality()));
}

//----------------------------------------------------------------------------
CTK_SET_CPP(qMRMLVirtualRealityView, vtkSlicerCamerasModuleLogic*, setCamerasLogic, CamerasLogic);
CTK_GET_CPP(qMRMLVirtualRealityView, vtkSlicerCamerasModuleLogic*, camerasLogic, CamerasLogic);

//----------------------------------------------------------------------------
CTK_GET_CPP(qMRMLVirtualRealityView, vtkOpenVRRenderer*, renderer, Renderer);

//----------------------------------------------------------------------------
CTK_GET_CPP(qMRMLVirtualRealityView, vtkOpenVRRenderWindow*, renderWindow, RenderWindow);

//----------------------------------------------------------------------------
CTK_GET_CPP(qMRMLVirtualRealityView, vtkOpenVRRenderWindowInteractor*, interactor, Interactor);

//---------------------------------------------------------------------------
void qMRMLVirtualRealityViewPrivate::createRenderWindow()
{
  Q_Q(qMRMLVirtualRealityView);

  this->RenderWindow = vtkSmartPointer<vtkOpenVRRenderWindow>::New();
  this->Renderer = vtkSmartPointer<vtkOpenVRRenderer>::New();
  this->Interactor = vtkSmartPointer<vtkOpenVRRenderWindowInteractor>::New();
  this->Camera = vtkSmartPointer<vtkOpenVRCamera>::New();
  this->Renderer->SetActiveCamera(this->Camera);

  this->RenderWindow->SetMultiSamples(0);
  this->RenderWindow->AddRenderer(this->Renderer);
  this->RenderWindow->SetInteractor(this->Interactor);

  vtkMRMLVirtualRealityViewDisplayableManagerFactory* factory
    = vtkMRMLVirtualRealityViewDisplayableManagerFactory::GetInstance();

  QStringList displayableManagers;
  displayableManagers //<< "vtkMRMLCameraDisplayableManager"
                      //<< "vtkMRMLViewDisplayableManager"
                      << "vtkMRMLModelDisplayableManager"
                      << "vtkMRMLThreeDReformatDisplayableManager"
                      << "vtkMRMLCrosshairDisplayableManager3D"
                      //<< "vtkMRMLOrientationMarkerDisplayableManager" // Not supported in VR
                      //<< "vtkMRMLRulerDisplayableManager" // Not supported in VR
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

  this->DisplayableManagerGroup = vtkSmartPointer<vtkMRMLDisplayableManagerGroup>::Take(
    factory->InstantiateDisplayableManagers(q->renderer()));
  this->DisplayableManagerGroup->SetMRMLDisplayableNode(this->MRMLVirtualRealityViewNode);

  qDebug() << "this->DisplayableManagerGroup" << this->DisplayableManagerGroup->GetDisplayableManagerCount();

  ///CONFIGURATION OF THE OPENVR ENVIRONEMENT

  this->Renderer->RemoveCuller(this->Renderer->GetCullers()->GetLastItem());
  this->Renderer->SetBackground(0.7, 0.7, 0.7);

  // Create 4 lights for even lighting
  // without this, one side of models may be very dark.
  this->Lights = vtkSmartPointer<vtkLightCollection>::New();
  {
    vtkNew<vtkLight> light;
    light->SetPosition(0.0, 1.0, 0.0);
    light->SetIntensity(1.0);
    light->SetLightTypeToSceneLight();
    this->Lights->AddItem(light);
  }
  {
    vtkNew<vtkLight> light;
    light->SetPosition(0.8, -0.2, 0.0);
    light->SetIntensity(0.8);
    light->SetLightTypeToSceneLight();
    this->Lights->AddItem(light);
  }
  {
    vtkNew<vtkLight> light;
    light->SetPosition(-0.3, -0.2, 0.7);
    light->SetIntensity(0.6);
    light->SetLightTypeToSceneLight();
    this->Lights->AddItem(light);
  }
  {
    vtkNew<vtkLight> light;
    light->SetPosition(-0.3, -0.2, -0.7);
    light->SetIntensity(0.4);
    light->SetLightTypeToSceneLight();
    this->Lights->AddItem(light);
  }
  this->Renderer->SetLightCollection(this->Lights);

  q->updateViewFromReferenceViewCamera();

  this->RenderWindow->Initialize();
  if (!this->RenderWindow->GetHMD())
    {
    qWarning() << "Failed to initialize OpenVR RenderWindow";
    return;
    }
}

//---------------------------------------------------------------------------
void qMRMLVirtualRealityViewPrivate::destroyRenderWindow()
{
  Q_Q(qMRMLVirtualRealityView);
  this->VirtualRealityLoopTimer.stop();
  this->RenderWindow = NULL;
  this->Interactor = NULL;
  this->DisplayableManagerGroup = NULL;
  this->Renderer = NULL;
  this->Camera = NULL;
  this->Lights = NULL;
}

//---------------------------------------------------------------------------
void qMRMLVirtualRealityView::updateViewFromReferenceViewCamera()
{
  Q_D(qMRMLVirtualRealityView);
  if (!d->MRMLVirtualRealityViewNode)
  {
    return;
  }
  vtkMRMLViewNode* refrenceViewNode = d->MRMLVirtualRealityViewNode->GetReferenceViewNode();
  if (!refrenceViewNode)
  {
    qWarning() << Q_FUNC_INFO << " failed: no reference view node is set";
    return;
  }
  if (!d->CamerasLogic)
  {
    qWarning() << Q_FUNC_INFO << " failed: cameras module logic is not set";
    return;
  }
  vtkMRMLCameraNode* cameraNode = d->CamerasLogic->GetViewActiveCameraNode(refrenceViewNode);
  if (!cameraNode || !cameraNode->GetCamera())
  {
    qWarning() << Q_FUNC_INFO << " failed: camera node is not found";
    return;
  }
  if (!d->RenderWindow)
  {
    qWarning() << Q_FUNC_INFO << " failed: RenderWindow has not been created";
    return;
  }

  // The following is based on d->RenderWindow->InitializeViewFromCamera(sourceCamera),
  // but that is not usable for us, as it puts the headset in the focal point (so we
  // need to step back to see the full content) and snaps view direction to the closest axis.

  vtkCamera *sourceCamera = cameraNode->GetCamera();

  vtkRenderer *ren = static_cast<vtkRenderer *>(d->RenderWindow->GetRenderers()->GetItemAsObject(0));
  if (!ren)
  {
    qWarning() << Q_FUNC_INFO << "The renderer must be set prior to calling InitializeViewFromCamera";
    return;
  }
  vtkOpenVRCamera *cam = vtkOpenVRCamera::SafeDownCast(ren->GetActiveCamera());
  if (!cam)
  {
    qWarning() << Q_FUNC_INFO << "The renderer's active camera must be set prior to calling InitializeViewFromCamera";
    return;
  }

  double distance =
    sin(vtkMath::RadiansFromDegrees(sourceCamera->GetViewAngle()) / 2.0) *
    sourceCamera->GetDistance() /
    sin(vtkMath::RadiansFromDegrees(cam->GetViewAngle()) / 2.0);

  double* sourceViewUp = sourceCamera->GetViewUp();
  cam->SetViewUp(sourceViewUp);
  d->RenderWindow->SetPhysicalViewUp(sourceViewUp);

  double* sourcePosition = sourceCamera->GetPosition();
  double* viewUp = cam->GetViewUp();
  cam->SetFocalPoint(sourcePosition);
  d->RenderWindow->SetPhysicalTranslation(
    viewUp[0] * distance - sourcePosition[0],
    viewUp[1] * distance - sourcePosition[1],
    viewUp[2] * distance - sourcePosition[2]);
  d->RenderWindow->SetPhysicalScale(distance);

  double* sourceDirectionOfProjection = sourceCamera->GetDirectionOfProjection();
  d->RenderWindow->SetPhysicalViewDirection(sourceDirectionOfProjection);
  double *idop = d->RenderWindow->GetPhysicalViewDirection();
  cam->SetPosition(
    -idop[0] * distance + sourcePosition[0],
    -idop[1] * distance + sourcePosition[1],
    -idop[2] * distance + sourcePosition[2]);

  ren->ResetCameraClippingRange();
}

//---------------------------------------------------------------------------
void qMRMLVirtualRealityViewPrivate::setMRMLScene(vtkMRMLScene* newScene)
{
  Q_Q(qMRMLVirtualRealityView);
  if (newScene == this->MRMLScene)
    {
    return;
    }
  this->MRMLScene = newScene;
}

// --------------------------------------------------------------------------
void qMRMLVirtualRealityViewPrivate::updateWidgetFromMRML()
{
  Q_Q(qMRMLVirtualRealityView);
  if (!this->MRMLVirtualRealityViewNode
    || !this->MRMLVirtualRealityViewNode->GetVisibility())
  {
    if (this->RenderWindow != NULL)
    {
      this->destroyRenderWindow();
    }
    return;
  }

  if (!this->RenderWindow)
  {
    this->createRenderWindow();
  }

  if (this->DisplayableManagerGroup->GetMRMLDisplayableNode() != this->MRMLVirtualRealityViewNode.GetPointer())
  {
    this->DisplayableManagerGroup->SetMRMLDisplayableNode(this->MRMLVirtualRealityViewNode);
  }

  this->Renderer->SetGradientBackground(1);
  this->Renderer->SetBackground(this->MRMLVirtualRealityViewNode->GetBackgroundColor());
  this->Renderer->SetBackground2(this->MRMLVirtualRealityViewNode->GetBackgroundColor2());

  this->Renderer->SetTwoSidedLighting(this->MRMLVirtualRealityViewNode->GetTwoSidedLighting());

  bool switchOnAllLights = this->MRMLVirtualRealityViewNode->GetBackLights();
  for (int i = 2; i < this->Lights->GetNumberOfItems(); i++)
  {
    vtkLight* light = vtkLight::SafeDownCast(this->Lights->GetItemAsObject(i));
    light->SetSwitch(switchOnAllLights);
  }

  if (this->RenderWindow)
  {
    double desiredUpdateRate = this->MRMLVirtualRealityViewNode->GetDesiredUpdateRate();

    // enforce non-zero frame rate to avoid division by zero errors
    const double defaultStaticViewUpdateRate = 0.0001;
    if (desiredUpdateRate < defaultStaticViewUpdateRate)
    {
      desiredUpdateRate = defaultStaticViewUpdateRate;
    }

    this->RenderWindow->SetDesiredUpdateRate(desiredUpdateRate);
  }

  if (this->MRMLVirtualRealityViewNode->GetActive())
  {
    this->VirtualRealityLoopTimer.start(0);
  }
  else
  {
    this->VirtualRealityLoopTimer.stop();
  }
}

// --------------------------------------------------------------------------
void qMRMLVirtualRealityViewPrivate::doOpenVirtualReality()
{
  if (this->Interactor && this->RenderWindow && this->Renderer)
  {
    this->Interactor->DoOneEvent(this->RenderWindow, this->Renderer);
  }
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
