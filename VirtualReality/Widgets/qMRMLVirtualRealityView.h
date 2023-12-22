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

#ifndef __qMRMLVirtualRealityView_h
#define __qMRMLVirtualRealityView_h

// VR Logic includes
class vtkSlicerVirtualRealityLogic;

// VR MRML includes
class vtkMRMLVirtualRealityViewNode;

// VR MRMLDM includes
class vtkVirtualRealityViewInteractorObserver;

// VR Widgets includes
#include "qSlicerVirtualRealityModuleWidgetsExport.h"
class qMRMLVirtualRealityViewPrivate;

// Qt includes
#include <QString>
#include <QWidget>

// CTK includes
#include <ctkVTKObject.h>

// Slicer includes
class vtkSlicerCamerasModuleLogic;

// VTK Rendering/VR includes
class vtkVRRenderer;
class vtkVRRenderWindow;
class vtkVRRenderWindowInteractor;

// VTK includes
class vtkCollection;
class vtkGenericOpenGLRenderWindow;
class vtkObject;
class vtkRenderWindowInteractor;

/// \brief 3D view for view nodes.
/// For performance reasons, the view block refreshes when the scene is in
/// batch process state.
/// VR hardware connection state is controlled by associated view node's properties:
/// - Visible: connection is made with OpenVR.
/// - Active: scene is rendered in the VR headset.
/// \sa qMRMLVRWidget, qMRMLVRViewControllerWidget, qMRMLSliceView
class Q_SLICER_QTMODULES_VIRTUALREALITY_WIDGETS_EXPORT qMRMLVirtualRealityView : public QWidget
{
  Q_OBJECT
  QVTK_OBJECT
public:
  /// Superclass typedef
  typedef QWidget Superclass;

  /// Constructors
  explicit qMRMLVirtualRealityView(QWidget* parent = nullptr);
  ~qMRMLVirtualRealityView() override;

  /// Returns the interactor observer of the view
  Q_INVOKABLE vtkVirtualRealityViewInteractorObserver* interactorObserver()const;

  /// Add a displayable manager to the view,
  /// the displayable manager is proper to the 3D view and is not shared
  /// with other views.
  /// If you want to register a displayable manager with all the 3D
  /// views (existing or future), you need to do it via
  /// vtkMRMLVirtualRealityViewDisplayableManagerFactory::RegisterDisplayableManager()
  /// By default: vtkMRMLCameraDisplayableManager,
  /// vtkMRMLViewDisplayableManager and vtkMRMLModelDisplayableManager are
  /// already registered.
  void addDisplayableManager(const QString& displayableManager);
  Q_INVOKABLE void getDisplayableManagers(vtkCollection *displayableManagers);

  ///@{
  /// Set Cameras module logic.
  /// Required for updating camera from reference view node.
  void setCamerasLogic(vtkSlicerCamerasModuleLogic* camerasLogic);
  vtkSlicerCamerasModuleLogic* camerasLogic()const;
  ///}@

  ///@{
  /// VirtualReality module logic.
  void setVirtualRealityLogic(vtkSlicerVirtualRealityLogic* camerasLogic);
  vtkSlicerVirtualRealityLogic* virtualRealityLogic()const;
  ///}@

  /// Get the 3D View node observed by view.
  Q_INVOKABLE vtkMRMLVirtualRealityViewNode* mrmlVirtualRealityViewNode()const;

  /// Get a reference to the associated vtkRenderer
  vtkVRRenderer* renderer()const;

  /// Get underlying RenderWindow
  Q_INVOKABLE vtkVRRenderWindow* renderWindow()const;

  /// Get underlying RenderWindow
  Q_INVOKABLE vtkVRRenderWindowInteractor* interactor()const;

  /// Initialize the virtual reality view to most closely
  /// matched the camera of the reference view camera.
  Q_INVOKABLE void updateViewFromReferenceViewCamera();

  /// Get underlying RenderWindow
  Q_INVOKABLE bool isHardwareConnected()const;

  /// Enable/disable grabbing and moving objects in the scene
  Q_INVOKABLE void setGrabObjectsEnabled(bool enable);
  /// Get whether grabbing and moving objects in the scene is enabled
  Q_INVOKABLE bool isGrabObjectsEnabled();

  /// Enable/disable dolly (flying in the scene with the TrackPad)
  Q_INVOKABLE void setDolly3DEnabled(bool enable);
  // Get whether dolly (flying in the scene with the TrackPad) is enabled
  Q_INVOKABLE bool isDolly3DEnabled();

  ///@{
  /// Convenience functions to easily associate grab and world functions to one or more buttons.
  /// When interaction with markups and other VTK MRML widgets will be implemented then we probably
  /// will not need these low-level event mappings anymore, but in the short term it is an effective
  /// workaround that enables prototyping of ideas.
  Q_INVOKABLE void setGestureButtonToTrigger();
  Q_INVOKABLE void setGestureButtonToGrip();
  Q_INVOKABLE void setGestureButtonToTriggerAndGrip();
  Q_INVOKABLE void setGestureButtonToNone();
  ///@}

  ///@{
  /// Path where the action manifest .json files are located.
  Q_INVOKABLE QString actionManifestPath() const;
  ///@}

signals:

  void physicalToWorldMatrixModified();

  // Controllers trigger
  void leftControllerTriggerPressed();
  void leftControllerTriggerReleased();
  void rightControllerTriggerPressed();
  void rightControllerTriggerReleased();

  // Controllers Grip
  void leftControllerGripPressed();
  void leftControllerGripReleased();
  void rightControllerGripPressed();
  void rightControllerGripReleased();

  // Controllers trackpad
  void leftControllerTrackpadPressed(double xPos, double yPos);
  void leftControllerTrackpadReleased(double xPos, double yPos);
  void rightControllerTrackpadPressed(double xPos, double yPos);
  void rightControllerTrackpadReleased(double xPos, double yPos);

public slots:
  /// Set the current \a viewNode to observe
  void setMRMLVirtualRealityViewNode(vtkMRMLVirtualRealityViewNode* newViewNode);

  void onPhysicalToWorldMatrixModified();
  void onButton3DEvent(vtkObject* caller, void* call_data, unsigned long vtk_event, void* client_data);

protected:

  QScopedPointer<qMRMLVirtualRealityViewPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLVirtualRealityView);
  Q_DISABLE_COPY(qMRMLVirtualRealityView);
};

#endif
