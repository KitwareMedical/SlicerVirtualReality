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

#ifndef __qMRMLVRView_h
#define __qMRMLVRView_h

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKRenderView.h>

// Qt includes
#include <QWidget>

#include "qSlicerVRModuleWidgetsExport.h"

class qMRMLVRViewPrivate;
class vtkMRMLScene;
class vtkMRMLVRViewNode;
class vtkCollection;
class vtkGenericOpenGLRenderWindow;
class vtkRenderWindowInteractor;

class vtkOpenVRRenderer;
class vtkOpenVRRenderWindow;
class vtkOpenVRRenderWindowInteractor;
class vtkOpenVRCamera;

/// \brief 3D view for view nodes.
/// For performance reasons, the view block refreshs when the scene is in
/// batch process state.
/// \sa qMRMLVRWidget, qMRMLVRViewControllerWidget, qMRMLSliceView
class Q_SLICER_QTMODULES_VR_WIDGETS_EXPORT qMRMLVRView : public QWidget
{
  Q_OBJECT
public:
  /// Superclass typedef
  typedef QWidget Superclass;

  /// Constructors
  explicit qMRMLVRView(QWidget* parent = 0);
  virtual ~qMRMLVRView();

  /// Add a displayable manager to the view,
  /// the displayable manager is proper to the 3D view and is not shared
  /// with other views.
  /// If you want to register a displayable manager with all the 3D
  /// views (existing or future), you need to do it via
  /// vtkMRMLVRViewDisplayableManagerFactory::RegisterDisplayableManager()
  /// By default: vtkMRMLCameraDisplayableManager,
  /// vtkMRMLViewDisplayableManager and vtkMRMLModelDisplayableManager are
  /// already registered.
  void addDisplayableManager(const QString& displayableManager);
  Q_INVOKABLE void getDisplayableManagers(vtkCollection *displayableManagers);

  /// Get the 3D View node observed by view.
  Q_INVOKABLE vtkMRMLVRViewNode* mrmlVRViewNode()const;

  /// Return if rendering is enabled
  bool renderEnabled() const;

  /// Get a reference to the associated vtkRenderer
  vtkOpenVRRenderer* renderer()const;

  /// Get underlying RenderWindow
  Q_INVOKABLE vtkOpenVRRenderWindow* renderWindow()const;

  /// Get underlying RenderWindow
  Q_INVOKABLE vtkOpenVRRenderWindowInteractor* interactor()const;

public slots:

  void startVR();
  void stopVR();
  /// Set the MRML \a scene that should be listened for events
  /// When the scene is in batch process state, the view blocks all refresh.
  /// \sa renderEnabled
  void setMRMLScene(vtkMRMLScene* newScene);

  /// Set the current \a viewNode to observe
  void setMRMLVRViewNode(vtkMRMLVRViewNode* newViewNode);

  /// Enable/Disable rendering
  void setRenderEnabled(bool value);


protected:
  QScopedPointer<qMRMLVRViewPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLVRView);
  Q_DISABLE_COPY(qMRMLVRView);
};

#endif
