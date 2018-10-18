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

/**
 * @class   vtkSlicerQWidgetWidget
 * @brief   3D VTK widget for a QWidget
 *
 * This 3D widget handles events between VTK and Qt for a QWidget placed
 * in a scene. It currently takes 6dof events as from VR controllers and
 * if they intersect the widget it converts them to Qt events and fires
 * them off.
 */

#ifndef vtkSlicerQWidgetWidget_h
#define vtkSlicerQWidgetWidget_h

#include "vtkSlicerGUIWidgetsModuleVTKWidgetsExport.h"

#include "vtkSlicerMarkupsWidget.h"

// Qt includes
#include <QPointF>

class vtkSlicerQWidgetRepresentation;

class VTK_SLICER_GUIWIDGETS_MODULE_VTKWIDGETS_EXPORT vtkSlicerQWidgetWidget : public vtkSlicerMarkupsWidget
{
  friend class vtkInteractionCallback;

public:
  /// Instantiate the object.
  static vtkSlicerQWidgetWidget* New();

  ///@{
  /// Standard vtkObject methods
  vtkTypeMacro(vtkSlicerQWidgetWidget, vtkSlicerMarkupsWidget);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  ///@}

  /// Create instance of the markups widget
  vtkSlicerMarkupsWidget* CreateInstance() const override;

  /// Specify an instance of vtkSlicerQWidgetRepresentation used to represent this
  /// widget in the scene. Note that the representation is a subclass of vtkProp
  /// so it can be added to the renderer independent of the widget.
  void SetRepresentation(vtkMRMLAbstractWidgetRepresentation *r) override;

  // Description:
  // Disable/Enable the widget if needed.
  // Unobserved the camera if the widget is disabled.
  //void SetEnabled(int enabling) override;

  /// Return the representation as a vtkSlicerQWidgetRepresentation
  vtkSlicerQWidgetRepresentation* GetQWidgetRepresentation();

  /// Create the default widget representation and initializes the widget and representation.
  void CreateDefaultRepresentation(
    vtkMRMLMarkupsDisplayNode* markupsDisplayNode, vtkMRMLAbstractViewNode* viewNode, vtkRenderer* renderer) override;

protected:
  vtkSlicerQWidgetWidget();
  ~vtkSlicerQWidgetWidget() override;
  /*
  // Manage the state of the widget
  int WidgetState;
  enum _WidgetState
  {
    Start = 0,
    Active
  };
  */
  QPointF LastWidgetCoordinates;

  // These methods handle events
  //static void SelectAction3D(vtkAbstractWidget*);
  //static void EndSelectAction3D(vtkAbstractWidget*);
  //static void MoveAction3D(vtkAbstractWidget*);

private:
  vtkSlicerQWidgetWidget(const vtkSlicerQWidgetWidget&) = delete;
  void operator=(const vtkSlicerQWidgetWidget&) = delete;
};

#endif
