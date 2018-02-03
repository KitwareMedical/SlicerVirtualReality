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

#ifndef __qSlicerVRModuleWidget_h
#define __qSlicerVRModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerVirtualRealityModuleExport.h"

class qSlicerVRModuleWidgetPrivate;
class vtkMRMLNode;
class qMRMLVRView;

/// \ingroup Slicer_QtModules_ExtensionTemplate
class Q_SLICER_QTMODULES_VIRTUALREALITY_EXPORT qSlicerVRModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:
  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerVRModuleWidget(QWidget *parent=0);
  virtual ~qSlicerVRModuleWidget();
  
  Q_INVOKABLE qMRMLVRView* vrWidget() const;

public slots:
  void initializeVR();
  void startVR();
  void stopVR();

protected:
  QScopedPointer<qSlicerVRModuleWidgetPrivate> d_ptr;
  qMRMLVRView* m_vrWidget;

  virtual void setup();

private:
  Q_DECLARE_PRIVATE(qSlicerVRModuleWidget);
  Q_DISABLE_COPY(qSlicerVRModuleWidget);
};

#endif
