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

  This file was originally developed by Csaba Pinter, EBATINCA, S.L., and
  development was supported by "ICEX Espana Exportacion e Inversiones" under
  the program "Inversiones de Empresas Extranjeras en Actividades de I+D
  (Fondo Tecnologico)- Convocatoria 2021"

==============================================================================*/

#ifndef __qSlicerGUIWidgetsModuleWidget_h
#define __qSlicerGUIWidgetsModuleWidget_h

// Slicer includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerGUIWidgetsModuleExport.h"

// Qt includes
#include <QMap>

class qSlicerGUIWidgetsModuleWidgetPrivate;
class vtkMRMLGUIWidgetNode;

/// \ingroup Slicer_QtModules_ExtensionTemplate
class Q_SLICER_QTMODULES_GUIWIDGETS_EXPORT qSlicerGUIWidgetsModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerGUIWidgetsModuleWidget(QWidget *parent=0);
  virtual ~qSlicerGUIWidgetsModuleWidget();

public slots:
  QWidget* onAddHelloWorldNodeClicked();
  void onUpdateButtonLabelButtonClicked();

  void onAddHomeWidgetButtonClicked();
  void onAddDataModuleWidgetButtonClicked();
  void onAddSegmentEditorWidgetButtonClicked();
  void onAddTransformWidgetButtonClicked();
  void onSetUpInteractionButtonClicked();
  void onStartInteractionButtonClicked();

  /// Assign widget to a GUIWidget markups node
  void setWidgetToGUIWidgetMarkupsNode(vtkMRMLGUIWidgetNode* node, QWidget* widget);

protected:
  QScopedPointer<qSlicerGUIWidgetsModuleWidgetPrivate> d_ptr;

  virtual void setup();

protected:
  QMap<vtkMRMLGUIWidgetNode*, QWidget*> GUIWidgetsMap;

private:
  Q_DECLARE_PRIVATE(qSlicerGUIWidgetsModuleWidget);
  Q_DISABLE_COPY(qSlicerGUIWidgetsModuleWidget);
};

#endif
