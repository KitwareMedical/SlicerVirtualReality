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

#ifndef __qMRMLVirtualRealityView_p_h
#define __qMRMLVirtualRealityView_p_h

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Slicer API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

// VR MRML includes
class vtkMRMLVirtualRealityViewNode;

// VR MRMLDM includes
class vtkVirtualRealityViewInteractorStyleDelegate;
class vtkVirtualRealityViewInteractorObserver;

// VR Widgets includes
#include "qMRMLVirtualRealityView.h"

// MRML includes
class vtkMRMLDisplayableManagerGroup;
class vtkMRMLTransformNode;

// VTK Rendering/VR includes
class vtkVRCamera;
class vtkVRInteractorStyle;
class vtkVRRenderer;
class vtkVRRenderWindow;
class vtkVRRenderWindowInteractor;

// VTK includes
#include <vtkEventData.h>
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>
class vtkLightCollection;
class vtkObject;
class vtkTimerLog;

// CTK includes
#include <ctkVTKObject.h>

// Qt includes
#include <QObject>
#include <QString>
#include <QTimer>

//-----------------------------------------------------------------------------
class qMRMLVirtualRealityViewPrivate: public QObject
{
  Q_OBJECT
  QVTK_OBJECT
  Q_DECLARE_PUBLIC(qMRMLVirtualRealityView);
protected:
  qMRMLVirtualRealityView* const q_ptr;
public:
  qMRMLVirtualRealityViewPrivate(qMRMLVirtualRealityView& object);
  ~qMRMLVirtualRealityViewPrivate() override;

  virtual void init();

  void startVirtualReality();
  void stopVirtualReality();

  double desiredUpdateRate();
  double stillUpdateRate();

public slots:
  void updateWidgetFromMRML();
  void doOpenVirtualReality();

protected:
  void updateTransformNodeWithControllerPose(vtkEventDataDevice device);
  void updateTransformNodeWithHMDPose();
  void updateTransformNodesWithTrackerPoses();

  void updateTransformNodeFromDevice(vtkMRMLTransformNode* node, vtkEventDataDevice device, uint32_t index=0);
  void updateTransformNodeAttributesFromDevice(vtkMRMLTransformNode* node, vtkEventDataDevice device, uint32_t index=0);

  void createRenderWindow();
  void destroyRenderWindow();

  vtkSlicerCamerasModuleLogic* CamerasLogic;

  vtkSmartPointer<vtkMRMLDisplayableManagerGroup> DisplayableManagerGroup;
  vtkSmartPointer<vtkVirtualRealityViewInteractorObserver>  InteractorObserver;
  vtkSmartPointer<vtkVirtualRealityViewInteractorStyleDelegate> InteractorStyleDelegate;
  vtkWeakPointer<vtkMRMLVirtualRealityViewNode> MRMLVirtualRealityViewNode;
  vtkSmartPointer<vtkVRRenderer> Renderer;
  vtkSmartPointer<vtkVRRenderWindow> RenderWindow;
  vtkSmartPointer<vtkVRRenderWindowInteractor> Interactor;
  vtkSmartPointer<vtkVRInteractorStyle> InteractorStyle;
  vtkSmartPointer<vtkVRCamera> Camera;
  vtkSmartPointer<vtkLightCollection> Lights;

  vtkSmartPointer<vtkTimerLog> LastViewUpdateTime;
  double LastViewDirection[3];
  double LastViewUp[3];
  double LastViewPosition[3];

  QString ActionManifestPath;

  QTimer VirtualRealityLoopTimer;
};

#endif
