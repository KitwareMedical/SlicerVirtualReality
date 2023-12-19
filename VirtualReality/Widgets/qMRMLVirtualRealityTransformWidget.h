/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Brigham and Women's Hospital

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Adam Rankin, Robarts Research Institute,
  Western University, London, Ontario, Canada.

==============================================================================*/

#ifndef __qMRMLVirtualRealityTransformWidget_h
#define __qMRMLVirtualRealityTransformWidget_h

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// SlicerQt includes
#include "qMRMLWidget.h"

#include "qSlicerVirtualRealityModuleWidgetsExport.h"

class qMRMLVirtualRealityTransformWidgetPrivate;
class vtkMRMLLinearTransformNode;
class vtkMRMLNode;
class vtkMRMLVirtualRealityViewNode;

/// \ingroup Slicer_QtModules_Markups
class Q_SLICER_QTMODULES_VIRTUALREALITY_WIDGETS_EXPORT qMRMLVirtualRealityTransformWidget
  : public qMRMLWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef qMRMLWidget Superclass;
  qMRMLVirtualRealityTransformWidget(vtkMRMLVirtualRealityViewNode* viewNode, QWidget* newParent = nullptr);
  ~qMRMLVirtualRealityTransformWidget() override;

public slots:
  void setMRMLLinearTransformNode(vtkMRMLLinearTransformNode* node);
  void setMRMLLinearTransformNode(vtkMRMLNode* node);
  void onButtonClicked();

protected slots:
  void updateWidgetFromMRML();

protected:
  QScopedPointer<qMRMLVirtualRealityTransformWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLVirtualRealityTransformWidget);
  Q_DISABLE_COPY(qMRMLVirtualRealityTransformWidget);

};

#endif
