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

// For:
//  - SlicerVirtualReality_HAS_OPENVR_SUPPORT
//  - SlicerVirtualReality_HAS_OPENXR_SUPPORT
//  - SlicerVirtualReality_HAS_OPENXRREMOTING_SUPPORT
#include "vtkMRMLVirtualRealityConfigure.h"

// For:
//  - Slicer_VERSION_MAJOR
//  - Slicer_VERSION_MINOR
#include <vtkSlicerVersionConfigureMinimal.h>

// VR Logic includes
#include "vtkSlicerVirtualRealityLogic.h"

// VR MRML includes
#include "vtkMRMLVirtualRealityViewNode.h"

// VR MRMLDM includes
#include "vtkVirtualRealityViewInteractorObserver.h"
#include "vtkVirtualRealityViewInteractorStyleDelegate.h"
#if defined(SlicerVirtualReality_HAS_OPENVR_SUPPORT)
#include "vtkVirtualRealityViewOpenVRInteractor.h"
#include "vtkVirtualRealityViewOpenVRInteractorStyle.h"
#endif
#if defined(SlicerVirtualReality_HAS_OPENXR_SUPPORT)
#include "vtkVirtualRealityViewOpenXRInteractor.h"
#include "vtkVirtualRealityViewOpenXRInteractorStyle.h"
#endif

// VR Widgets includes
#include "qMRMLVirtualRealityView_p.h"

// Qt includes
#include <QCoreApplication>
#include <QDebug>
#include <QEvent>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStringList>
#include <QToolButton>
#include <QTimer>
#include <QSettings>

// CTK includes
#include <ctkPimpl.h> // For CTK_GET_CPP, CTK_SET_CPP

// Slicer includes
#include <qSlicerApplication.h>
#include <vtkSlicerCamerasModuleLogic.h>

// MRMLDisplayableManager includes
#include <vtkMRMLAbstractDisplayableManager.h>
#include <vtkMRMLDisplayableManagerGroup.h>
#include <vtkMRMLVirtualRealityViewDisplayableManagerFactory.h>

// MRML includes
#include <vtkMRMLCameraNode.h>
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLVectorVolumeNode.h>

#if defined(SlicerVirtualReality_HAS_OPENVR_SUPPORT)
// VTK Rendering/OpenVR includes
#include <vtkOpenVRCamera.h>
#include <vtkOpenVRModel.h>
#include <vtkOpenVRRenderWindow.h>
#include <vtkOpenVRRenderer.h>

// OpenVR includes
#include <openvr.h>
#endif

#if defined(SlicerVirtualReality_HAS_OPENXR_SUPPORT)
// VTK Rendering/OpenXR includes
#include <vtkOpenXRCamera.h>
#include <vtkOpenXRManager.h>
#include <vtkOpenXRModel.h>
#include <vtkOpenXRRenderWindow.h>
#include <vtkOpenXRRenderer.h>
#endif

#if defined(SlicerVirtualReality_HAS_OPENXRREMOTING_SUPPORT)
// VTK Rendering/OpenXRRemoting includes
#include <vtkOpenXRRemotingRenderWindow.h>
#endif

// VTK Rendering/VR includes
#include <vtkVRCamera.h>
#include <vtkVRRenderer.h>
#include <vtkVRRenderWindow.h>
#include <vtkVRRenderWindowInteractor.h>

// VTK OpenGL loader (provides raw OpenGL 4.x function pointers)
#include <vtk_glad.h>

// VTK includes
#include <vtkCollection.h>
#include <vtkCullerCollection.h>
#include <vtkImageData.h>
#include <vtkLight.h>
#include <vtkLightCollection.h>
#include <vtkNew.h>
#include <vtkOpenGLFramebufferObject.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkSmartPointer.h>
#include <vtkTimerLog.h>
#include <vtkTransform.h>

namespace
{
#if defined(SlicerVirtualReality_HAS_OPENVR_SUPPORT)
  //--------------------------------------------------------------------------
  std::string PoseStatusToString(vr::ETrackingResult result)
  {
    switch (result)
    {
    case vr::TrackingResult_Calibrating_InProgress:
      return "CalibratingInProgress";
    case vr::TrackingResult_Calibrating_OutOfRange:
      return "CalibratingOutOfRange";
    case vr::TrackingResult_Running_OK:
      return "RunningOk";
    case vr::TrackingResult_Running_OutOfRange:
      return "RunningOutOfRange";
    case vr::TrackingResult_Uninitialized:
    default:
      return "Uninitialized";
    }
  }
#endif
}

//--------------------------------------------------------------------------
// qMRMLVirtualRealityViewPrivate methods

//---------------------------------------------------------------------------
qMRMLVirtualRealityViewPrivate::qMRMLVirtualRealityViewPrivate(qMRMLVirtualRealityView& object)
  : q_ptr(&object)
  , CamerasLogic(nullptr)
{
  this->MRMLVirtualRealityViewNode = nullptr;
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
CTK_SET_CPP(qMRMLVirtualRealityView, vtkSlicerVirtualRealityLogic*, setVirtualRealityLogic, VirtualRealityLogic);
CTK_GET_CPP(qMRMLVirtualRealityView, vtkSlicerVirtualRealityLogic*, virtualRealityLogic, VirtualRealityLogic);

//----------------------------------------------------------------------------
CTK_GET_CPP(qMRMLVirtualRealityView, vtkVRRenderer*, renderer, Renderer);

//----------------------------------------------------------------------------
CTK_GET_CPP(qMRMLVirtualRealityView, vtkVRRenderWindow*, renderWindow, RenderWindow);

//----------------------------------------------------------------------------
CTK_GET_CPP(qMRMLVirtualRealityView, vtkVRRenderWindowInteractor*, interactor, Interactor);

// --------------------------------------------------------------------------
int qMRMLVirtualRealityView::currentXRBackend() const
{
  Q_D(const qMRMLVirtualRealityView);
  return static_cast<int>(d->currentXRBackend());
}

//---------------------------------------------------------------------------
void qMRMLVirtualRealityViewPrivate::createRenderWindow(vtkMRMLVirtualRealityViewNode::XRBackendType xrBackend)
{
  Q_Q(qMRMLVirtualRealityView);

  if (this->MRMLVirtualRealityViewNode == nullptr)
  {
    qCritical() << Q_FUNC_INFO << " failed: MRMLVirtualRealityViewNode is not set";
    return;
  }
  if (this->VirtualRealityLogic == nullptr)
  {
    qCritical() << Q_FUNC_INFO << " failed: VirtualRealityLogic is not set";
    return;
  }
  vtkSlicerApplicationLogic* appLogic = qSlicerApplication::application()->applicationLogic();
  if (!appLogic)
  {
    qCritical() << Q_FUNC_INFO << " failed: Application logic not available";
    return;
  }

  this->LastViewUpdateTime = vtkSmartPointer<vtkTimerLog>::New();
  this->LastViewUpdateTime->StartTimer();
  this->LastViewUpdateTime->StopTimer();
  this->LastViewDirection[0] = 0.0;
  this->LastViewDirection[1] = 0.0;
  this->LastViewDirection[2] = 1.0;
  this->LastViewUp[0] = 0.0;
  this->LastViewUp[1] = 1.0;
  this->LastViewUp[2] = 0.0;
  this->LastViewPosition[0] = 0.0;
  this->LastViewPosition[1] = 0.0;
  this->LastViewPosition[2] = 0.0;

  // InteractorStyleDelegate
  this->InteractorStyleDelegate = vtkSmartPointer<vtkVirtualRealityViewInteractorStyleDelegate>::New();

  // XRBackend
#if defined(SlicerVirtualReality_HAS_OPENVR_SUPPORT)
  if (xrBackend == vtkMRMLVirtualRealityViewNode::OpenVR)
  {
    vtkNew<vtkVirtualRealityViewOpenVRInteractorStyle> interactorStyle;
    interactorStyle->SetInteractorStyleDelegate(this->InteractorStyleDelegate);

    this->RenderWindow = vtkSmartPointer<vtkOpenVRRenderWindow>::New();
    this->Renderer = vtkSmartPointer<vtkOpenVRRenderer>::New();
    this->InteractorStyle = interactorStyle;
    this->Interactor = vtkSmartPointer<vtkVirtualRealityViewOpenVRInteractor>::New();
    this->Camera = vtkSmartPointer<vtkOpenVRCamera>::New();
  }
  else
#endif
#if defined(SlicerVirtualReality_HAS_OPENXR_SUPPORT)
    if (xrBackend == vtkMRMLVirtualRealityViewNode::OpenXR)
    {
      vtkNew<vtkVirtualRealityViewOpenXRInteractorStyle> interactorStyle;
      interactorStyle->SetInteractorStyleDelegate(this->InteractorStyleDelegate);

#if defined(SlicerVirtualReality_HAS_OPENXRREMOTING_SUPPORT)
      if (this->MRMLVirtualRealityViewNode->GetRemoting())
      {
        vtkSmartPointer<vtkOpenXRRemotingRenderWindow> xrRemotingRenderWindow = vtkSmartPointer<vtkOpenXRRemotingRenderWindow>::New();
        xrRemotingRenderWindow->SetRemotingIPAddress(this->MRMLVirtualRealityViewNode->GetPlayerIPAddress().c_str());
        // Address "wglDXRegisterObjectNV failed in RegisterSharedTexture()" reported when using "OpenXRRemoting"
        xrRemotingRenderWindow->GetHelperWindow()->SetMultiSamples(0);
        this->RenderWindow = xrRemotingRenderWindow;
      }
      else
#endif
      {
        this->RenderWindow = vtkSmartPointer<vtkOpenXRRenderWindow>::New();
      }
      this->Renderer = vtkSmartPointer<vtkOpenXRRenderer>::New();
      this->InteractorStyle = interactorStyle;
      this->Interactor = vtkSmartPointer<vtkVirtualRealityViewOpenXRInteractor>::New();
      this->Camera = vtkSmartPointer<vtkOpenXRCamera>::New();

      // Configure passthrough and occlusion before Initialize() is called.
      // SelectExtensions() (called during Initialize) checks these values to decide
      // which XR extensions to load; they cannot be changed without a reconnect.
      {
        vtkOpenXRRenderWindow* xrRenderWindow = vtkOpenXRRenderWindow::SafeDownCast(this->RenderWindow);
        if (xrRenderWindow)
        {
          if (this->MRMLVirtualRealityViewNode->GetPassthrough())
          {
            xrRenderWindow->SetUsePassthrough(true);
          }
          // OccludedOpacity < 1.0 requires XR_META_environment_depth to be loaded at Init time.
          xrRenderWindow->SetOccludedOpacity(
            0.0);
          // Also request env depth when depth volume capture is enabled, even if
          // depth occlusion is not active (OccludedOpacity == 1.0).
          xrRenderWindow->SetCaptureEnvironmentDepth(
            this->MRMLVirtualRealityViewNode->GetPassthroughDepthVolumeEnabled());

          // OpenXR has no API for rendering controller models, so vtkRenderingOpenXR falls back
          // to a profile-to-asset JSON lookup table; point it at this module's own copy instead
          // of relying on one being manually dropped next to the VTK build/install tree.
          xrRenderWindow->SetModelsManifestDirectory(this->VirtualRealityLogic->ComputeControllerModelsPath());
        }
      }
    }
    else
#endif
    {
      this->destroyRenderWindow();
      qWarning() << "No XR backend initialized";
      this->MRMLVirtualRealityViewNode->SetError("Connection failed: No XR backend initialized");
      return;
    }

  // InteractorStyle
  this->InteractorStyle->SetCurrentRenderer(this->Renderer);

  // Interactor
  this->Interactor->SetActionManifestDirectory(this->VirtualRealityLogic->ComputeActionManifestPath(xrBackend));
  this->Interactor->SetInteractorStyle(this->InteractorStyle);

  // InteractorObserver
  this->InteractorObserver = vtkSmartPointer<vtkVirtualRealityViewInteractorObserver>::New();
  this->InteractorObserver->SetInteractor(this->Interactor);

  // Camera
  this->Renderer->SetActiveCamera(this->Camera);

  // RenderWindow
  this->RenderWindow->SetMultiSamples(0);
  this->RenderWindow->AddRenderer(this->Renderer);
  this->RenderWindow->SetInteractor(this->Interactor);
  // Set default 10x magnification (conversion: PhysicalScale = 1000 / Magnification)
  this->RenderWindow->SetPhysicalScale(100.0);

  //
  // Connections
  //

  // Observe VR render window event
  qvtkReconnect(this->RenderWindow, vtkVRRenderWindow::PhysicalToWorldMatrixModified,
    q, SLOT(onPhysicalToWorldMatrixModified()));

  // Observe renderer EndEvent to capture passthrough volume data while the
  // rendering FBO is still valid (before swapchain release).
  qvtkReconnect(this->Renderer, vtkCommand::EndEvent,
    this, SLOT(onRendererEndEvent()));

  // Observe button press event
  qvtkReconnect(this->Interactor, vtkCommand::Button3DEvent, q, SLOT(onButton3DEvent(vtkObject*, void*, unsigned long, void*)));

  //
  // DisplayableManager registration
  //
  vtkMRMLVirtualRealityViewDisplayableManagerFactory* factory
    = vtkMRMLVirtualRealityViewDisplayableManagerFactory::GetInstance();
  factory->SetMRMLApplicationLogic(appLogic);

  QStringList displayableManagers;
  displayableManagers
    // Slicer
    //<< "vtkMRMLCameraDisplayableManager" // Not supported in VR, vtkVRCamera has no MRML node counterpart
    //<< "vtkMRMLViewDisplayableManager" // Not supported in VR, require a vtkMRMLCameraNode
    << "vtkMRMLModelDisplayableManager"
    << "vtkMRMLThreeDReformatDisplayableManager"
    << "vtkMRMLCrosshairDisplayableManager3D"
    //<< "vtkMRMLOrientationMarkerDisplayableManager" // Not supported in VR
    //<< "vtkMRMLRulerDisplayableManager" // Not supported in VR
    //<< "vtkMRMLAnnotationDisplayableManager" // Not supported in VR
    << "vtkMRMLMarkupsDisplayableManager"
    << "vtkMRMLSegmentationsDisplayableManager3D"
    << "vtkMRMLTransformsDisplayableManager3D"
#if ((Slicer_VERSION_MAJOR == 5 && Slicer_VERSION_MINOR >= 7) || (Slicer_VERSION_MAJOR > 5))
    << "vtkMRMLLinearTransformsDisplayableManager"
#else
    << "vtkMRMLLinearTransformsDisplayableManager3D"
#endif
    << "vtkMRMLVolumeRenderingDisplayableManager"
    ;
  foreach(const QString & displayableManager, displayableManagers)
  {
    if (!factory->IsDisplayableManagerRegistered(displayableManager.toLatin1()))
    {
      factory->RegisterDisplayableManager(displayableManager.toLatin1());
    }
  }

  this->DisplayableManagerGroup = vtkSmartPointer<vtkMRMLDisplayableManagerGroup>::Take(
    factory->InstantiateDisplayableManagers(q->renderer()));
  this->DisplayableManagerGroup->SetMRMLDisplayableNode(this->MRMLVirtualRealityViewNode);
  this->InteractorStyleDelegate->SetDisplayableManagers(this->DisplayableManagerGroup);
  this->InteractorObserver->SetDisplayableManagers(this->DisplayableManagerGroup);

  // Default inputs mapping
  vtkSlicerVirtualRealityLogic::SetTriggerButtonFunction(
    this->Interactor, vtkSlicerVirtualRealityLogic::GetButtonFunctionIdForGrabObjectsAndWorld());

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

  QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
  this->RenderWindow->Initialize();
  QApplication::restoreOverrideCursor();

  const char* xrBackendAsStr = vtkMRMLVirtualRealityViewNode::GetXRBackendAsString(xrBackend);

  if (!this->RenderWindow->GetVRInitialized())
  {
    this->MRMLVirtualRealityViewNode->SetError("Connection failed");
    qWarning() << Q_FUNC_INFO << ": Failed to initialize " << xrBackendAsStr << " RenderWindow";
    return;
  }

  // If passthrough was requested, report the outcome.
  // XR_ENVIRONMENT_BLEND_MODE_ALPHA_BLEND is active immediately after Initialize();
  // XR_FB_passthrough objects are created during Initialize() but xrPassthroughStartFB()
  // is deferred to BeginSession() (first render tick), so IsFBPassthroughActive() is
  // still false here — use IsFBPassthroughSupported() to report that it is pending.
  // If neither mechanism is available, warn the user.
#if defined(SlicerVirtualReality_HAS_OPENXR_SUPPORT)
  if (this->MRMLVirtualRealityViewNode->GetPassthrough())
  {
    vtkOpenXRRenderWindow* xrRenderWindow = vtkOpenXRRenderWindow::SafeDownCast(this->RenderWindow);
    if (xrRenderWindow)
    {
      if (xrRenderWindow->IsPassthroughActive())
      {
        // XR_ENVIRONMENT_BLEND_MODE_ALPHA_BLEND is active right now.
        qDebug() << "SlicerVirtualReality: Passthrough active (ALPHA_BLEND blend mode).";
      }
      else if (vtkOpenXRManager::GetInstance().IsFBPassthroughSupported())
      {
        // XR_FB_passthrough extension found and objects created; passthrough will
        // activate on the first render tick when BeginSession() is called.
        qDebug() << "SlicerVirtualReality: XR_FB_passthrough objects created; "
          "passthrough will activate when the XR session starts.";
      }
      else
      {
        qWarning() << "SlicerVirtualReality: Passthrough requested but no passthrough "
          "mechanism is available (neither XR_FB_passthrough nor "
          "XR_ENVIRONMENT_BLEND_MODE_ALPHA_BLEND is supported by the runtime). "
          "VR rendering continues with normal background. "
          "For Meta Quest 3 via Quest Link: enable developer mode on the headset "
          "and ensure 'Allow apps to access passthrough' is enabled in the "
          "Meta XR PC app (Settings > General).";
      }
    }
  }
#endif

  // Keep track of last valid parameters in the settings
  QSettings().setValue("VirtualReality/DefaultXRBackend", xrBackendAsStr);
#if defined(SlicerVirtualReality_HAS_OPENXRREMOTING_SUPPORT)
  QSettings().setValue("VirtualReality/DefaultRemotingEnabled", this->MRMLVirtualRealityViewNode->GetRemoting());
  QSettings().setValue("VirtualReality/DefaultPlayerIPAddress", QString::fromStdString(this->MRMLVirtualRealityViewNode->GetPlayerIPAddress()));
#endif

  qDebug() << "";
  qDebug() << "XR backend \"" << xrBackendAsStr << "\" initialized";
  qDebug() << "";
  qDebug() << "ActionManifestPath:" << q->actionManifestPath();
  qDebug() << "Number of registered displayable manager:" << this->DisplayableManagerGroup->GetDisplayableManagerCount();
  qDebug() << "Registered displayable managers:";
  for (int idx = 0; idx < this->DisplayableManagerGroup->GetDisplayableManagerCount(); idx++)
  {
    qDebug() << " " << this->DisplayableManagerGroup->GetNthDisplayableManager(idx)->GetClassName();
  }

}

//---------------------------------------------------------------------------
void qMRMLVirtualRealityViewPrivate::destroyRenderWindow()
{
  this->VirtualRealityLoopTimer.stop();

  // Release OpenGL resources that we own while the render window's
  // GL context is still valid.
  if (this->RenderWindow != nullptr)
  {
    this->RenderWindow->MakeCurrent();
    if (this->DepthBlitFBO != 0)
    {
      glDeleteFramebuffers(1, &this->DepthBlitFBO);
      this->DepthBlitFBO = 0;
    }
    if (this->DepthBlitTexture != 0)
    {
      glDeleteTextures(1, &this->DepthBlitTexture);
      this->DepthBlitTexture = 0;
    }
  }

  // Must break the connection between interactor and render window,
  // otherwise they would circularly refer to each other and would not
  // be deleted.
  if (this->Interactor != nullptr)
  {
    this->Interactor->SetRenderWindow(nullptr);
  }
  this->Interactor = nullptr;
  this->InteractorStyle = nullptr;
  this->DisplayableManagerGroup = nullptr;
  this->Renderer = nullptr;
  this->Camera = nullptr;
  this->Lights = nullptr;
  this->RenderWindow = nullptr;
}

// --------------------------------------------------------------------------
vtkMRMLVirtualRealityViewNode::XRBackendType qMRMLVirtualRealityViewPrivate::currentXRBackend() const
{
  if (this->RenderWindow == nullptr)
  {
    return vtkMRMLVirtualRealityViewNode::UndefinedXRBackend;
  }
#if defined(SlicerVirtualReality_HAS_OPENVR_SUPPORT)
  if (vtkOpenVRRenderWindow::SafeDownCast(this->RenderWindow) != nullptr)
  {
    return vtkMRMLVirtualRealityViewNode::OpenVR;
  }
#endif
#if defined(SlicerVirtualReality_HAS_OPENXR_SUPPORT)
  if (vtkOpenXRRenderWindow::SafeDownCast(this->RenderWindow) != nullptr
#if defined(SlicerVirtualReality_HAS_OPENXRREMOTING_SUPPORT)
    || vtkOpenXRRemotingRenderWindow::SafeDownCast(this->RenderWindow) != nullptr
#endif
    )
  {
    return vtkMRMLVirtualRealityViewNode::OpenXR;
  }
#endif
  qCritical() << Q_FUNC_INFO << " failed: RenderWindow is not a supported type: "
    << this->RenderWindow->GetClassName();
  return vtkMRMLVirtualRealityViewNode::UndefinedXRBackend;
}

// --------------------------------------------------------------------------
bool qMRMLVirtualRealityViewPrivate::currentXRBackendRemotingEnabled() const
{
#if defined(SlicerVirtualReality_HAS_OPENXRREMOTING_SUPPORT)
  return vtkOpenXRRemotingRenderWindow::SafeDownCast(this->RenderWindow) != nullptr;
#else
  return false;
#endif
}

// --------------------------------------------------------------------------
std::string qMRMLVirtualRealityViewPrivate::currentXRBackendRemotingIPAddress() const
{
#if defined(SlicerVirtualReality_HAS_OPENXRREMOTING_SUPPORT)
  return vtkOpenXRManager::GetInstance().GetConnectionStrategy()->GetIPAddress();
#else
  return std::string();
#endif
}

// --------------------------------------------------------------------------
bool qMRMLVirtualRealityViewPrivate::currentXRBackendPassthroughEnabled() const
{
#if defined(SlicerVirtualReality_HAS_OPENXR_SUPPORT)
  vtkOpenXRRenderWindow* xrRenderWindow = vtkOpenXRRenderWindow::SafeDownCast(this->RenderWindow);
  if (xrRenderWindow != nullptr)
  {
    return xrRenderWindow->GetUsePassthrough();
  }
#endif
  return false;
}

// --------------------------------------------------------------------------
void qMRMLVirtualRealityViewPrivate::updateWidgetFromMRML()
{
  if (this->IsUpdatingWidgetFromMRML)
  {
    // Updating widget from MRML is already in progress
    return;
  }
  this->IsUpdatingWidgetFromMRML = true;

  this->updateWidgetFromMRMLNoModify();

  this->IsUpdatingWidgetFromMRML = false;
}

// --------------------------------------------------------------------------
void qMRMLVirtualRealityViewPrivate::updateWidgetFromMRMLNoModify()
{
  // Finalize XR backend if the view node is not present or visibility is turned off
  // (i.e., disconnected from hardware)
  if (!this->MRMLVirtualRealityViewNode || !this->MRMLVirtualRealityViewNode->GetVisibility())
  {
    this->destroyRenderWindow();
    this->InitializationAttempts = 0;
    this->MRMLVirtualRealityViewNode->ClearError();
    return;
  }

  // Reset initialization attempts and clear errors if the XR backend has changed
  if (this->currentXRBackend() != this->MRMLVirtualRealityViewNode->GetXRBackend()
    || this->currentXRBackendRemotingEnabled() != this->MRMLVirtualRealityViewNode->GetRemoting()
    || this->currentXRBackendRemotingIPAddress() != this->MRMLVirtualRealityViewNode->GetPlayerIPAddress()
    || this->currentXRBackendPassthroughEnabled() != this->MRMLVirtualRealityViewNode->GetPassthrough())
  {
    this->InitializationAttempts = 0;
    this->MRMLVirtualRealityViewNode->ClearError();
  }

  // Attempt to initialize XR backend if the current backend differs or is undefined, and
  // the view is visible (i.e., connected to the hardware)
  if ((this->currentXRBackend() != this->MRMLVirtualRealityViewNode->GetXRBackend()
    || this->currentXRBackendRemotingEnabled() != this->MRMLVirtualRealityViewNode->GetRemoting()
    || (this->MRMLVirtualRealityViewNode->GetRemoting()
      && this->currentXRBackendRemotingIPAddress() != this->MRMLVirtualRealityViewNode->GetPlayerIPAddress())
    || this->currentXRBackendPassthroughEnabled() != this->MRMLVirtualRealityViewNode->GetPassthrough()
    || this->currentXRBackend() == vtkMRMLVirtualRealityViewNode::UndefinedXRBackend)
    && this->MRMLVirtualRealityViewNode->GetVisibility())
  {
    // Bail if there are no more attempts left
    constexpr int maximumNumberOfAttempts = 1;
    if (this->InitializationAttempts >= maximumNumberOfAttempts)
    {
      return;
    }
    this->InitializationAttempts++;
    this->MRMLVirtualRealityViewNode->ClearError();

    vtkMRMLVirtualRealityViewNode::XRBackendType xrBackend = this->MRMLVirtualRealityViewNode->GetXRBackend();
    const char* xrBackendAsStr = vtkMRMLVirtualRealityViewNode::GetXRBackendAsString(xrBackend);

    // Log the initialization attempt
    qDebug().noquote().nospace()
      << "Initializing \"" << xrBackendAsStr << "\" XR backend "
      << QString("(%1/%2)").arg(this->InitializationAttempts).arg(maximumNumberOfAttempts);

    // Destroy and recreate the render window
    this->destroyRenderWindow();
    this->createRenderWindow(xrBackend);
  }

  // Skip further updates if the XR backend is undefined or if the view node has an error
  if (this->currentXRBackend() == vtkMRMLVirtualRealityViewNode::UndefinedXRBackend
    || this->MRMLVirtualRealityViewNode->HasError())
  {
    return;
  }

  if (this->DisplayableManagerGroup->GetMRMLDisplayableNode() != this->MRMLVirtualRealityViewNode.GetPointer())
  {
    this->DisplayableManagerGroup->SetMRMLDisplayableNode(this->MRMLVirtualRealityViewNode);
  }

  // Renderer properties
  // Use a transparent background only when the runtime has actually granted
  // XR_ENVIRONMENT_BLEND_MODE_ALPHA_BLEND (passthrough/AR mode).  If passthrough
  // was requested but the runtime fell back to OPAQUE, use the normal gradient
  // background so the user doesn't see an unwanted black screen.
  // For XR_FB_passthrough, IsPassthroughActive() returns false until BeginSession()
  // runs on the first render tick, so also check IsFBPassthroughSupported() — if
  // handles were successfully created, passthrough will be active imminently and
  // we should set the transparent background now.
  bool passthroughActive = false;
#if defined(SlicerVirtualReality_HAS_OPENXR_SUPPORT)
  if (this->MRMLVirtualRealityViewNode->GetPassthrough())
  {
    vtkOpenXRRenderWindow* xrRenderWindow = vtkOpenXRRenderWindow::SafeDownCast(this->RenderWindow);
    passthroughActive = xrRenderWindow && xrRenderWindow->GetVRInitialized() &&
      (xrRenderWindow->IsPassthroughActive() ||
        vtkOpenXRManager::GetInstance().IsFBPassthroughSupported());
  }
#endif

  if (this->MRMLVirtualRealityViewNode->GetRemoting() || passthroughActive)
  {
    // Use a transparent background so the real-world passthrough (or remoting
    // AR background) shows through wherever no 3D content is rendered.
    this->Renderer->SetGradientBackground(0);
    this->Renderer->SetBackground(0.0, 0.0, 0.0);
    this->Renderer->SetBackgroundAlpha(0.0);
  }
  else
  {
    this->Renderer->SetGradientBackground(1);
    this->Renderer->SetBackground(this->MRMLVirtualRealityViewNode->GetBackgroundColor());
    this->Renderer->SetBackground2(this->MRMLVirtualRealityViewNode->GetBackgroundColor2());
  }

  this->Renderer->SetTwoSidedLighting(this->MRMLVirtualRealityViewNode->GetTwoSidedLighting());

  bool switchOnAllLights = this->MRMLVirtualRealityViewNode->GetBackLights();
  for (int i = 2; i < this->Lights->GetNumberOfItems(); i++)
  {
    vtkLight* light = vtkLight::SafeDownCast(this->Lights->GetItemAsObject(i));
    light->SetSwitch(switchOnAllLights);
  }

  this->Renderer->SetUseDepthPeeling(this->MRMLVirtualRealityViewNode->GetUseDepthPeeling() != 0);
  this->Renderer->SetUseDepthPeelingForVolumes(this->MRMLVirtualRealityViewNode->GetUseDepthPeeling() != 0);

  // Render window properties
  if (this->RenderWindow)
  {
    // Desired update rate
    this->RenderWindow->SetDesiredUpdateRate(this->desiredUpdateRate());

    // Magnification
    double magnification = this->MRMLVirtualRealityViewNode->GetMagnification();
    if (magnification < 0.01)
    {
      magnification = 0.01;
    }
    else if (magnification > 100.0)
    {
      magnification = 100.0;
    }
    this->InteractorStyleDelegate->SetMagnification(magnification);

    // Dolly physical speed
    double dollyPhysicalSpeedMps = this->MRMLVirtualRealityViewNode->GetMotionSpeed();

    // 1.6666 m/s is walking speed (= 6 km/h), which is the default. We multiply it with the factor
    this->InteractorStyle->SetDollyPhysicalSpeed(dollyPhysicalSpeedMps);

#if defined(SlicerVirtualReality_HAS_OPENVR_SUPPORT)
    vtkOpenVRRenderWindow* vrRenderWindow = vtkOpenVRRenderWindow::SafeDownCast(this->RenderWindow);
    if (vrRenderWindow != nullptr && vrRenderWindow->GetHMD() != nullptr)
    {
      vtkEventDataDevice deviceIdsToUpdate[] = { vtkEventDataDevice::RightController, vtkEventDataDevice::LeftController, vtkEventDataDevice::Unknown };
      for (int deviceIdIndex = 0; deviceIdsToUpdate[deviceIdIndex] != vtkEventDataDevice::Unknown; deviceIdIndex++)
      {
        vtkVRModel* model = vtkVRModel::SafeDownCast(vrRenderWindow->GetModelForDevice(deviceIdsToUpdate[deviceIdIndex]));
        if (!model)
        {
          continue;
        }
        model->SetVisibility(this->MRMLVirtualRealityViewNode->GetControllerModelsVisible());
      }

      // Update tracking reference visibility
      for (uint32_t deviceIdIndex = 0; deviceIdIndex < vr::k_unMaxTrackedDeviceCount; ++deviceIdIndex)
      {
        if (vrRenderWindow->GetHMD()->GetTrackedDeviceClass(deviceIdIndex) == vr::TrackedDeviceClass_TrackingReference)
        {
          vtkVRModel* model = vtkVRModel::SafeDownCast(vrRenderWindow->GetModelForDevice(
            vrRenderWindow->GetDeviceForOpenVRHandle(deviceIdIndex)));
          if (!model)
          {
            continue;
          }
          model->SetVisibility(this->MRMLVirtualRealityViewNode->GetLighthouseModelsVisible());
        }
      }
    }
#endif

#if defined(SlicerVirtualReality_HAS_OPENXR_SUPPORT)
    {
      vtkOpenXRRenderWindow* xrRenderWindow = vtkOpenXRRenderWindow::SafeDownCast(this->RenderWindow);
      if (xrRenderWindow)
      {
        xrRenderWindow->SetOccludedOpacity(
          static_cast<float>(this->MRMLVirtualRealityViewNode->GetOccludedOpacity()));
        xrRenderWindow->SetShowEnvDepthDebugVisualization(
          this->MRMLVirtualRealityViewNode->GetEnvDepthDebugVisualization());
        // Note: CaptureEnvironmentDepth must be set before session creation (in createRenderWindow).
        // Updating it here has no effect on an already-running session.
      }
    }
#endif
  }

  if (this->MRMLVirtualRealityViewNode->GetActive())
  {
    this->VirtualRealityLoopTimer.start(0);
  }
  else
  {
    this->VirtualRealityLoopTimer.stop();
  }

  // Ensure volume nodes exist as soon as the capture flags are toggled on,
  // regardless of whether data is currently flowing.
  if (this->MRMLVirtualRealityViewNode->GetPassthroughDepthVolumeEnabled())
  {
    this->VirtualRealityLogic->GetOrCreatePassthroughDepthVolumeNode();
  }
  if (this->MRMLVirtualRealityViewNode->GetVRSceneColorVolumeEnabled())
  {
    this->VirtualRealityLogic->GetOrCreateVRSceneColorVolumeNode();
  }
}

//---------------------------------------------------------------------------
double qMRMLVirtualRealityViewPrivate::desiredUpdateRate()
{
  double rate = this->MRMLVirtualRealityViewNode->GetDesiredUpdateRate();

  // enforce non-zero frame rate to avoid division by zero errors
  const double defaultStaticViewUpdateRate = 0.0001;
  if (rate < defaultStaticViewUpdateRate)
  {
    rate = defaultStaticViewUpdateRate;
  }

  return rate;
}

//---------------------------------------------------------------------------
double qMRMLVirtualRealityViewPrivate::stillUpdateRate()
{
  return 0.0001;
}

// --------------------------------------------------------------------------
void qMRMLVirtualRealityViewPrivate::doOpenVirtualReality()
{
  if (this->Interactor == nullptr)
  {
    qCritical() << Q_FUNC_INFO << "failed: Interactor is not set";
    return;
  }
  if (this->Renderer == nullptr)
  {
    qCritical() << Q_FUNC_INFO << "failed: Renderer is not set";
    return;
  }
  if (this->RenderWindow == nullptr)
  {
    qCritical() << Q_FUNC_INFO << "failed: RenderWindow is not set";
    return;
  }

  bool hmdConnected = true;
#if defined(SlicerVirtualReality_HAS_OPENVR_SUPPORT)
  vtkOpenVRRenderWindow* vrRenderWindow = vtkOpenVRRenderWindow::SafeDownCast(this->RenderWindow);
  if (vrRenderWindow != nullptr)
  {
    hmdConnected = vrRenderWindow->GetHMD() != nullptr;
  }
#endif
  if (this->RenderWindow->GetVRInitialized() && hmdConnected)
  {
    // Reset staging flags so onRendererEndEvent() captures fresh data this frame.
    this->ColorStagingHasData = false;
    this->DepthStagingHasData = false;

    this->Interactor->DoOneEvent(this->RenderWindow, this->Renderer);

    this->LastViewUpdateTime->StopTimer();
    if (this->LastViewUpdateTime->GetElapsedTime() > 0.0)
    {
      bool quickViewMotion =
        vtkSlicerVirtualRealityLogic::ShouldConsiderQuickViewMotion(
          this->MRMLVirtualRealityViewNode->GetMotionSensitivity(),
          this->RenderWindow->GetPhysicalScale(),
          this->LastViewUpdateTime->GetElapsedTime(),
          // position and orientation since last view update
          this->LastViewPosition,
          this->LastViewDirection,
          this->LastViewUp,
          // current view position and orientation
          this->Camera->GetPosition(),
          this->Camera->GetViewPlaneNormal(),
          this->Camera->GetViewUp());

      double updateRate = quickViewMotion ? this->desiredUpdateRate() : this->stillUpdateRate();
      this->RenderWindow->SetDesiredUpdateRate(updateRate);

      // Save current view position and orientation
      this->Camera->GetViewPlaneNormal(this->LastViewDirection);
      this->Camera->GetViewUp(this->LastViewUp);
      this->Camera->GetPosition(this->LastViewPosition);

      if (this->MRMLVirtualRealityViewNode->GetControllerTransformsUpdate())
      {
        this->MRMLVirtualRealityViewNode->CreateDefaultControllerTransformNodes();
        updateTransformNodeWithControllerPose(vtkEventDataDevice::LeftController);
        updateTransformNodeWithControllerPose(vtkEventDataDevice::RightController);
      }
      if (this->MRMLVirtualRealityViewNode->GetHMDTransformUpdate())
      {
        this->MRMLVirtualRealityViewNode->CreateDefaultHMDTransformNode();
        updateTransformNodeWithHMDPose();
      }
      if (this->MRMLVirtualRealityViewNode->GetTrackerTransformUpdate())
      {
        updateTransformNodesWithTrackerPoses();
      }

      if (this->MRMLVirtualRealityViewNode->GetVRSceneColorVolumeEnabled() ||
        this->MRMLVirtualRealityViewNode->GetPassthroughDepthVolumeEnabled())
      {
        this->updatePassthroughVolumeNodes();
      }

      this->LastViewUpdateTime->StartTimer();
    }
  }
}

// --------------------------------------------------------------------------
void qMRMLVirtualRealityViewPrivate::updateTransformNodeWithControllerPose(vtkEventDataDevice device)
{
  vtkMRMLLinearTransformNode* node = this->MRMLVirtualRealityViewNode->GetControllerTransformNode(device);

  if (node == nullptr)
  {
    qCritical() << Q_FUNC_INFO << ": Unable to retrieve linear transform node for device: " << (int)device;
    return;
  }

  int disabledModify = node->StartModify();
  this->updateTransformNodeFromDevice(node, device);
  this->updateTransformNodeAttributesFromDevice(node, device);
  node->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void qMRMLVirtualRealityViewPrivate::updateTransformNodeWithHMDPose()
{
  vtkMRMLLinearTransformNode* node = this->MRMLVirtualRealityViewNode->GetHMDTransformNode();

  if (node == nullptr)
  {
    qCritical() << Q_FUNC_INFO << ": Unable to retrieve linear transform node for HMD";
    return;
  }

  int disabledModify = node->StartModify();
  this->updateTransformNodeFromDevice(node, vtkEventDataDevice::HeadMountedDisplay);
  this->updateTransformNodeAttributesFromDevice(node, vtkEventDataDevice::HeadMountedDisplay);
  node->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void qMRMLVirtualRealityViewPrivate::updateTransformNodesWithTrackerPoses()
{
  for (uint32_t i = 0; i < this->RenderWindow->GetNumberOfDeviceHandlesForDevice(vtkEventDataDevice::GenericTracker); ++i)
  {
    uint32_t handle = this->RenderWindow->GetDeviceHandleForDevice(vtkEventDataDevice::GenericTracker, i);
    vtkMRMLLinearTransformNode* node = this->MRMLVirtualRealityViewNode->CreateDefaultTrackerTransformNode(handle);

    int disabledModify = node->StartModify();
    this->updateTransformNodeFromDevice(node, vtkEventDataDevice::GenericTracker, i);
    this->updateTransformNodeAttributesFromDevice(node, vtkEventDataDevice::GenericTracker, i);
    node->EndModify(disabledModify);
  }
}

//----------------------------------------------------------------------------
void qMRMLVirtualRealityViewPrivate::onRendererEndEvent()
{
  if (!this->MRMLVirtualRealityViewNode)
  {
    return;
  }

  // --- Color (left-eye rendered frame) ---
  if (this->MRMLVirtualRealityViewNode->GetVRSceneColorVolumeEnabled()
    && !this->ColorStagingHasData)
  {
    GLint viewport[4] = { 0, 0, 0, 0 };
    glGetIntegerv(GL_VIEWPORT, viewport);
    int w = viewport[2];
    int h = viewport[3];
    if (w > 0 && h > 0)
    {
      this->ColorStaging.resize(static_cast<size_t>(w) * h * 3);
      glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, this->ColorStaging.data());
      this->ColorStagingWidth = w;
      this->ColorStagingHeight = h;
      this->ColorStagingHasData = true;
    }
  }

  // --- Depth (XR_META_environment_depth, left eye, layer 0) ---
#if defined(SlicerVirtualReality_HAS_OPENXR_SUPPORT)
  if (this->MRMLVirtualRealityViewNode->GetPassthroughDepthVolumeEnabled()
    && !this->DepthStagingHasData)
  {
#ifdef XR_META_environment_depth
    vtkOpenXRManager& mgr = vtkOpenXRManager::GetInstance();
    if (mgr.IsEnvironmentDepthActive())
    {
      uint32_t texId = mgr.GetEnvDepthTextureId();
      if (texId != 0)
      {
        // DSA query — no bind needed, works regardless of texture target.
        GLint w = 0, h = 0;
        glGetTextureLevelParameteriv(texId, 0, GL_TEXTURE_WIDTH, &w);
        glGetTextureLevelParameteriv(texId, 0, GL_TEXTURE_HEIGHT, &h);

        if (w > 0 && h > 0)
        {
          // Build or reuse the staging FBO.
          // No staging texture needed — we read directly from the depth
          // attachment via GL_DEPTH_COMPONENT.
          if (this->DepthBlitFBO == 0)
          {
            glGenFramebuffers(1, &this->DepthBlitFBO);
          }

          // Save current FBO state before touching anything
          GLint prevFBO = 0;
          glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFBO);

          glBindFramebuffer(GL_FRAMEBUFFER, this->DepthBlitFBO);

          // Attach layer 0 (left eye) of the XR depth texture as the
          // depth attachment — this is the only attach mode that works
          // for a depth-format swapchain texture on this runtime.
          glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
            texId, 0 /*mip*/, 0 /*layer=left eye*/);

          GLenum fbStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
          if (fbStatus != GL_FRAMEBUFFER_COMPLETE)
          {
            qWarning() << "EnvDepth: FBO incomplete: 0x" << Qt::hex << fbStatus;
          }
          else
          {
            // Flush any pre-existing GL errors before our readback
            while (glGetError() != GL_NO_ERROR) {}

            this->DepthStaging.resize(static_cast<size_t>(w) * h);
            glReadPixels(0, 0, w, h, GL_DEPTH_COMPONENT, GL_FLOAT,
              this->DepthStaging.data());

            GLenum err = glGetError();
            if (err != GL_NO_ERROR)
            {
              qWarning() << "EnvDepth glReadPixels failed: 0x" << Qt::hex << err;
            }
            else
            {
              // Convert metres → millimetres.
              for (float& v : this->DepthStaging)
              {
                v *= 1000.0f;
              }
              this->DepthStagingWidth = w;
              this->DepthStagingHeight = h;
              this->DepthStagingHasData = true;
            }
          }

          // Detach and restore XR texture
          glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 0, 0, 0);
          glBindFramebuffer(GL_FRAMEBUFFER, prevFBO);
        }
      }
    }
#endif // XR_META_environment_depth
  }
#endif // SlicerVirtualReality_HAS_OPENXR_SUPPORT
}

//----------------------------------------------------------------------------
// Copy from CPU staging buffers (filled during rendering) into MRML volume
// nodes.  No OpenGL calls are made here — the swapchain images have already
// been released at this point.
void qMRMLVirtualRealityViewPrivate::updatePassthroughVolumeNodes()
{
  if (!this->MRMLVirtualRealityViewNode)
  {
    return;
  }

  // --- Depth volume ---
  if (this->MRMLVirtualRealityViewNode->GetPassthroughDepthVolumeEnabled()
    && this->DepthStagingHasData)
  {
    vtkMRMLScalarVolumeNode* volNode =
      this->VirtualRealityLogic->GetOrCreatePassthroughDepthVolumeNode();
    if (volNode)
    {
      vtkImageData* img = volNode->GetImageData();
      if (!img ||
        img->GetDimensions()[0] != this->DepthStagingWidth ||
        img->GetDimensions()[1] != this->DepthStagingHeight)
      {
        vtkNew<vtkImageData> newImg;
        newImg->SetDimensions(this->DepthStagingWidth, this->DepthStagingHeight, 1);
        newImg->AllocateScalars(VTK_FLOAT, 1);
        volNode->SetAndObserveImageData(newImg);
        img = volNode->GetImageData();
      }
      memcpy(img->GetScalarPointer(),
        this->DepthStaging.data(),
        this->DepthStaging.size() * sizeof(float));
      img->Modified();
      volNode->Modified();
    }
    this->DepthStagingHasData = false;
  }

  // --- Color volume ---
  if (this->MRMLVirtualRealityViewNode->GetVRSceneColorVolumeEnabled()
    && this->ColorStagingHasData)
  {
    vtkMRMLVectorVolumeNode* volNode =
      this->VirtualRealityLogic->GetOrCreateVRSceneColorVolumeNode();
    if (volNode)
    {
      vtkImageData* img = volNode->GetImageData();
      if (!img ||
        img->GetDimensions()[0] != this->ColorStagingWidth ||
        img->GetDimensions()[1] != this->ColorStagingHeight)
      {
        vtkNew<vtkImageData> newImg;
        newImg->SetDimensions(this->ColorStagingWidth, this->ColorStagingHeight, 1);
        newImg->AllocateScalars(VTK_UNSIGNED_CHAR, 3);
        volNode->SetAndObserveImageData(newImg);
        img = volNode->GetImageData();
      }
      memcpy(img->GetScalarPointer(),
        this->ColorStaging.data(),
        this->ColorStaging.size());
      img->Modified();
      volNode->Modified();
    }
    this->ColorStagingHasData = false;
  }
}

//----------------------------------------------------------------------------
void qMRMLVirtualRealityViewPrivate
::updateTransformNodeAttributesFromDevice(vtkMRMLTransformNode* node, vtkEventDataDevice device, uint32_t index)
{
#ifdef SlicerVirtualReality_HAS_OPENVR_SUPPORT
  vtkOpenVRRenderWindow* vrRenderWindow = vtkOpenVRRenderWindow::SafeDownCast(this->RenderWindow);
  if (vrRenderWindow == nullptr)
  {
    return;
  }

  std::string attributePrefix;

  switch (device)
  {
  case vtkEventDataDevice::HeadMountedDisplay:
    attributePrefix = "HMD";
    break;
  case vtkEventDataDevice::RightController:
    attributePrefix = "Controller";
    break;
  case vtkEventDataDevice::LeftController:
    attributePrefix = "Controller";
    break;
  case vtkEventDataDevice::GenericTracker:
    attributePrefix = "Tracker";
    break;
  default:
    attributePrefix = "Unknown";
    break;
  }

  vr::TrackedDevicePose_t* tdPose;
  vrRenderWindow->GetOpenVRPose(device, index, &tdPose);
  if (tdPose == nullptr)
  {
    auto handle = this->RenderWindow->GetDeviceHandleForDevice(device, index);
    switch (device)
    {
    case vtkEventDataDevice::HeadMountedDisplay:
      qCritical() << Q_FUNC_INFO << ": Unable to retrieve HMD pose";
      break;
    case vtkEventDataDevice::RightController:
      qCritical() << Q_FUNC_INFO << ": Unable to retrieve RightController pose";
      break;
    case vtkEventDataDevice::LeftController:
      qCritical() << Q_FUNC_INFO << ": Unable to retrieve LeftController pose";
      break;
    case vtkEventDataDevice::GenericTracker:
      qCritical() << Q_FUNC_INFO << ": Unable to retrieve pose associated with VR tracker"
        << "(index: " << index << ", handle: " << handle << ")";
      break;
    default:
      qCritical() << Q_FUNC_INFO << ": Unable to retrieve pose associated with unknown device";
      break;
    }
    return;
  }

  bool active = tdPose != nullptr && tdPose->eTrackingResult == vr::TrackingResult_Running_OK;
  std::string activeAttributeName = std::string("VirtualReality.") + attributePrefix + "Active";
  node->SetAttribute(activeAttributeName.c_str(), active ? "1" : "0");

  bool connected = tdPose != nullptr && tdPose->bDeviceIsConnected;
  std::string connectedAttributeName = std::string("VirtualReality.") + attributePrefix + "Connected";
  node->SetAttribute(connectedAttributeName.c_str(), connected ? "1" : "0");

  bool poseValid = tdPose != nullptr && tdPose->bPoseIsValid;
  node->SetAttribute("VirtualReality.PoseValid", poseValid ? "True" : "False");
  node->SetAttribute("VirtualReality.PoseStatus", tdPose ? PoseStatusToString(tdPose->eTrackingResult).c_str() : "Uninitialized");
#else
  Q_UNUSED(node);
  Q_UNUSED(device);
  Q_UNUSED(index);
#endif
}

//----------------------------------------------------------------------------
void qMRMLVirtualRealityViewPrivate
::updateTransformNodeFromDevice(vtkMRMLTransformNode* node, vtkEventDataDevice device, uint32_t index)
{
  auto deviceHandle = this->RenderWindow->GetDeviceHandleForDevice(device, index);
  if (deviceHandle == UINT32_MAX /* InvalidDeviceIndex or vr::k_unTrackedDeviceIndexInvalid */)
  {
    return;
  }
  vtkMatrix4x4* pose = this->RenderWindow->GetDeviceToPhysicalMatrixForDeviceHandle(deviceHandle);
  if (!pose)
  {
    return;
  }

  double pos[3] = { 0. };
  double ppos[3] = { 0. };
  double wxyz[4] = { 1., 0., 0., 0. };
  double wdir[3] = { 1., 0., 0. };

  // Convert device pose to world coordinates
  this->Interactor->ConvertPoseToWorldCoordinates(pose, pos, wxyz, ppos, wdir);

  vtkNew<vtkTransform> transform;
  transform->Translate(pos);
  transform->RotateWXYZ(wxyz[0], wxyz[1], wxyz[2], wxyz[3]);

  node->SetMatrixTransformToParent(transform->GetMatrix());
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
vtkVirtualRealityViewInteractorObserver* qMRMLVirtualRealityView::interactorObserver()const
{
  Q_D(const qMRMLVirtualRealityView);
  return d->InteractorObserver;
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
  this->setEnabled(newViewNode != nullptr);
}

//---------------------------------------------------------------------------
vtkMRMLVirtualRealityViewNode* qMRMLVirtualRealityView::mrmlVirtualRealityViewNode()const
{
  Q_D(const qMRMLVirtualRealityView);
  return d->MRMLVirtualRealityViewNode;
}

//------------------------------------------------------------------------------
void qMRMLVirtualRealityView::getDisplayableManagers(vtkCollection* displayableManagers)
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

//------------------------------------------------------------------------------
void qMRMLVirtualRealityView::setGrabObjectsEnabled(bool enable)
{
  Q_D(qMRMLVirtualRealityView);
  d->InteractorStyleDelegate->SetGrabEnabled(enable);
}

//------------------------------------------------------------------------------
bool qMRMLVirtualRealityView::isGrabObjectsEnabled()
{
  Q_D(qMRMLVirtualRealityView);
  return d->InteractorStyleDelegate->GetGrabEnabled();
}

//------------------------------------------------------------------------------
void qMRMLVirtualRealityView::setDolly3DEnabled(bool enable)
{
  Q_D(qMRMLVirtualRealityView);

  // The "eventId to state" mapping (see call to `MapInputToAction` below) applies to right and left
  // controller buttons because they are bound to the same eventId:
  // - `vtk_openvr_binding_*.json` files define the "button to action" mapping
  // - `vtkOpenVRInteractorStyle()` contructor defines the "action to eventId" mapping

  if (enable)
  {
    d->InteractorStyle->MapInputToAction(vtkCommand::ViewerMovement3DEvent, VTKIS_DOLLY);
  }
  else
  {
    d->InteractorStyle->MapInputToAction(vtkCommand::ViewerMovement3DEvent, VTKIS_NONE);
  }
}

//------------------------------------------------------------------------------
bool qMRMLVirtualRealityView::isDolly3DEnabled()
{
  Q_D(qMRMLVirtualRealityView);

  return d->InteractorStyle->GetMappedAction(vtkCommand::ViewerMovement3DEvent) == VTKIS_DOLLY;
}

//------------------------------------------------------------------------------
void qMRMLVirtualRealityView::setGestureButtonToTrigger()
{
  Q_D(qMRMLVirtualRealityView);
  vtkSlicerVirtualRealityLogic::SetGestureButtonToTrigger(d->Interactor);
}

//------------------------------------------------------------------------------
void qMRMLVirtualRealityView::setGestureButtonToGrip()
{
  Q_D(qMRMLVirtualRealityView);
  vtkSlicerVirtualRealityLogic::SetGestureButtonToGrip(d->Interactor);
}

//------------------------------------------------------------------------------
void qMRMLVirtualRealityView::setGestureButtonToTriggerAndGrip()
{
  Q_D(qMRMLVirtualRealityView);
  vtkSlicerVirtualRealityLogic::SetGestureButtonToTriggerAndGrip(d->Interactor);
}

//------------------------------------------------------------------------------
void qMRMLVirtualRealityView::setGestureButtonToNone()
{
  Q_D(qMRMLVirtualRealityView);
  vtkSlicerVirtualRealityLogic::SetGestureButtonToNone(d->Interactor);
}

//------------------------------------------------------------------------------
QString qMRMLVirtualRealityView::actionManifestPath() const
{
  Q_D(const qMRMLVirtualRealityView);
  return QString::fromStdString(d->Interactor->GetActionManifestDirectory());
}

//------------------------------------------------------------------------------
void qMRMLVirtualRealityView::onPhysicalToWorldMatrixModified()
{
  Q_D(qMRMLVirtualRealityView);

  d->MRMLVirtualRealityViewNode->SetMagnification(d->InteractorStyleDelegate->GetMagnification());

  emit physicalToWorldMatrixModified();
}

//------------------------------------------------------------------------------
void qMRMLVirtualRealityView::onButton3DEvent(vtkObject* caller, void* call_data, unsigned long vtk_event, void* client_data)
{
  Q_UNUSED(caller);
  Q_UNUSED(vtk_event);
  Q_UNUSED(client_data);

  vtkEventDataDevice3D* ed = reinterpret_cast<vtkEventDataDevice3D*>(call_data);

  if (ed->GetInput() == vtkEventDataDeviceInput::Trigger)
  {
    if (ed->GetDevice() == vtkEventDataDevice::LeftController)
    {
      if (ed->GetAction() == vtkEventDataAction::Press)
      {
        emit leftControllerTriggerPressed();
      }
      else if (ed->GetAction() == vtkEventDataAction::Release)
      {
        emit leftControllerTriggerReleased();
      }
    }
    else if (ed->GetDevice() == vtkEventDataDevice::RightController)
    {
      if (ed->GetAction() == vtkEventDataAction::Press)
      {
        emit rightControllerTriggerPressed();
      }
      else if (ed->GetAction() == vtkEventDataAction::Release)
      {
        emit rightControllerTriggerReleased();
      }
    }
  }
  else if (ed->GetInput() == vtkEventDataDeviceInput::Grip)
  {
    if (ed->GetDevice() == vtkEventDataDevice::LeftController)
    {
      if (ed->GetAction() == vtkEventDataAction::Press)
      {
        emit leftControllerGripPressed();
      }
      else if (ed->GetAction() == vtkEventDataAction::Release)
      {
        emit leftControllerGripReleased();
      }
    }
    else if (ed->GetDevice() == vtkEventDataDevice::RightController)
    {
      if (ed->GetAction() == vtkEventDataAction::Press)
      {
        emit rightControllerGripPressed();
      }
      else if (ed->GetAction() == vtkEventDataAction::Release)
      {
        emit rightControllerGripReleased();
      }
    }
  }
  else if (ed->GetInput() == vtkEventDataDeviceInput::TrackPad)
  {
    if (ed->GetDevice() == vtkEventDataDevice::LeftController)
    {
      if (ed->GetAction() == vtkEventDataAction::Press)
      {
        emit leftControllerTrackpadPressed(ed->GetTrackPadPosition()[0], ed->GetTrackPadPosition()[1]);
      }
      else if (ed->GetAction() == vtkEventDataAction::Release)
      {
        emit leftControllerTrackpadReleased(ed->GetTrackPadPosition()[0], ed->GetTrackPadPosition()[1]);
      }
    }
    else if (ed->GetDevice() == vtkEventDataDevice::RightController)
    {
      if (ed->GetAction() == vtkEventDataAction::Press)
      {
        emit rightControllerTrackpadPressed(ed->GetTrackPadPosition()[0], ed->GetTrackPadPosition()[1]);
      }
      else if (ed->GetAction() == vtkEventDataAction::Release)
      {
        emit rightControllerTrackpadReleased(ed->GetTrackPadPosition()[0], ed->GetTrackPadPosition()[1]);
      }
    }
  }
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

  vtkCamera* sourceCamera = cameraNode->GetCamera();

  vtkRenderer* ren = static_cast<vtkRenderer*>(d->RenderWindow->GetRenderers()->GetItemAsObject(0));
  if (!ren)
  {
    qWarning() << Q_FUNC_INFO << ": The renderer must be set prior to calling InitializeViewFromCamera";
    return;
  }
  vtkVRCamera* cam = vtkVRCamera::SafeDownCast(ren->GetActiveCamera());
  if (!cam)
  {
    qWarning() << Q_FUNC_INFO << ": The renderer's active camera must be set prior to calling InitializeViewFromCamera";
    return;
  }

  double newPhysicalScale = 100.0; // Default 10x magnification

  double* sourceViewUp = sourceCamera->GetViewUp();
  cam->SetViewUp(sourceViewUp);
  d->RenderWindow->SetPhysicalViewUp(sourceViewUp);

  double* sourcePosition = sourceCamera->GetPosition();
  double* viewUp = cam->GetViewUp();
  cam->SetFocalPoint(sourcePosition);
  d->RenderWindow->SetPhysicalTranslation(
    viewUp[0] * newPhysicalScale - sourcePosition[0],
    viewUp[1] * newPhysicalScale - sourcePosition[1],
    viewUp[2] * newPhysicalScale - sourcePosition[2]);

  double* sourceDirectionOfProjection = sourceCamera->GetDirectionOfProjection();
  d->RenderWindow->SetPhysicalViewDirection(sourceDirectionOfProjection);
  double* idop = d->RenderWindow->GetPhysicalViewDirection();
  cam->SetPosition(
    -idop[0] * newPhysicalScale + sourcePosition[0],
    -idop[1] * newPhysicalScale + sourcePosition[1],
    -idop[2] * newPhysicalScale + sourcePosition[2]);

  d->RenderWindow->SetPhysicalScale(newPhysicalScale);

  ren->ResetCameraClippingRange();
}
