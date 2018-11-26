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

#ifndef __qSlicerVirtualRealityModuleWidget_h
#define __qSlicerVirtualRealityModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerVirtualRealityModuleExport.h"

class qSlicerVirtualRealityModuleWidgetPrivate;
class vtkMRMLNode;

class Q_SLICER_QTMODULES_VIRTUALREALITY_EXPORT qSlicerVirtualRealityModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:
  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerVirtualRealityModuleWidget(QWidget *parent=0);
  virtual ~qSlicerVirtualRealityModuleWidget();

public slots:
  void setVirtualRealityConnected(bool connect);
  void setVirtualRealityActive(bool activate);
  void setTwoSidedLighting(bool);
  void setBackLights(bool);
  void setControllerModelsVisible(bool);
  void setReferenceViewNode(vtkMRMLNode*);
  void updateViewFromReferenceViewCamera();
  void onDesiredUpdateRateChanged(double);
  void onMotionSensitivityChanged(double);
  void onMotionSpeedChanged(double);
  void onMagnificationChanged(double); 
  void setControllerTransformsUpdate(bool);

protected slots:
  void updateWidgetFromMRML();

  void onPhysicalToWorldMatrixModified();

protected:
  QScopedPointer<qSlicerVirtualRealityModuleWidgetPrivate> d_ptr;

  virtual void setup();

  double getMagnificationFromPower(double powerOfTen);
  double getPowerFromMagnification(double magnification);

private:
  Q_DECLARE_PRIVATE(qSlicerVirtualRealityModuleWidget);
  Q_DISABLE_COPY(qSlicerVirtualRealityModuleWidget);
};

#endif
