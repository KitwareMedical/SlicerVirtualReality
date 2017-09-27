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

#ifndef __qMRMLVRView_p_h
#define __qMRMLVRView_p_h

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

// VTK includes
//#include <vtkOpenGLFramebufferObject.h>

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// qMRML includes
#include "qMRMLVRView.h"

// Qt includes
#include <QTimer>

class QLabel;
class vtkMRMLDisplayableManagerGroup;
class vtkMRMLVRViewNode;
class vtkMRMLCameraNode;
class vtkObject;

//-----------------------------------------------------------------------------
class qMRMLVRViewPrivate: public QObject
{
  Q_OBJECT
  QVTK_OBJECT
  Q_DECLARE_PUBLIC(qMRMLVRView);
protected:
  qMRMLVRView* const q_ptr;
public:
  qMRMLVRViewPrivate(qMRMLVRView& object);
  ~qMRMLVRViewPrivate();

  virtual void init();

  void setMRMLScene(vtkMRMLScene* scene);
  void startVR();
  void stopVR();

public slots:
  /// Handle MRML scene event
  void onSceneStartProcessing();
  void onSceneEndProcessing();

  void updateWidgetFromMRML();


  void doOpenVR();

protected:
  void initDisplayableManagers();

  vtkMRMLDisplayableManagerGroup*    DisplayableManagerGroup;
  vtkMRMLScene*                      MRMLScene;
  vtkMRMLVRViewNode*                 MRMLVRViewNode;
  bool                               RenderEnabled;
  vtkSmartPointer<vtkOpenVRRenderer>       Renderer;
  vtkSmartPointer<vtkOpenVRRenderWindow>   RenderWindow;
  vtkSmartPointer<vtkOpenVRRenderWindowInteractor>   Interactor;
  vtkSmartPointer<vtkOpenVRCamera> Camera;

  QTimer VRLoopTimer;
};

#endif
