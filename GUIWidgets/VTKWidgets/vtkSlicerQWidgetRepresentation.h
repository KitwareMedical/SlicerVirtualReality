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
 * @class   vtkSlicerQWidgetRepresentation
 * @brief   a class defining the representation for a vtkSlicerQWidgetWidget
 *
 * This class renders a QWidget as a simple vtkPlaneSource with a vtkTexture
 * that contains a vtkSlicerQWidgetTexture which imports the OpenGL texture handle
 * from Qt into the vtk scene. Qt and VTK may need to be using the same
 * graphics context.
 */

#ifndef vtkSlicerQWidgetRepresentation_h
#define vtkSlicerQWidgetRepresentation_h

#include "vtkSlicerGUIWidgetsModuleVTKWidgetsExport.h"

#include "vtkSlicerMarkupsWidgetRepresentation3D.h"

class QWidget;

class vtkActor;
class vtkCallbackCommand;
class vtkOpenGLTexture;
class vtkPlaneSource;
class vtkPolyDataAlgorithm;
class vtkPolyDataMapper;
class vtkSlicerQWidgetTexture;

class VTK_SLICER_GUIWIDGETS_MODULE_VTKWIDGETS_EXPORT vtkSlicerQWidgetRepresentation : public vtkSlicerMarkupsWidgetRepresentation3D
{
public:
  /**
   * Instantiate the class.
   */
  static vtkSlicerQWidgetRepresentation* New();

  ///@{
  /// Standard methods for the class.
  vtkTypeMacro(vtkSlicerQWidgetRepresentation, vtkSlicerMarkupsWidgetRepresentation3D);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  ///@}

  /// Subclasses of vtkMRMLAbstractWidgetRepresentation must implement these methods. These
  /// are the methods that the widget and its representation use to
  /// communicate with each other.
  void UpdateFromMRML(vtkMRMLNode* caller, unsigned long event, void *callData=nullptr) override;

  /// TODO:
  void PlaceWidget(double bounds[6]);

  ///@{
  /// Methods supporting the rendering process.
  double* GetBounds() VTK_SIZEHINT(6) override;
  void GetActors(vtkPropCollection* pc) override;
  void ReleaseGraphicsResources(vtkWindow*) override;
  int RenderOpaqueGeometry(vtkViewport*) override;
  int RenderTranslucentPolygonalGeometry(vtkViewport*) override;
  vtkTypeBool HasTranslucentPolygonalGeometry() override;
  ///@}

  // Manage the state of the widget
  enum _InteractionState
  {
    Outside = 0,
    Inside
  };

  /// Set the QWidget this representation will render
  void SetWidget(QWidget* w);

  /// Get the QWidgetTexture used by the representation
  vtkGetObjectMacro(QWidgetTexture, vtkSlicerQWidgetTexture);

  /// Get the vtkPlaneSouce used by this representation. This can be useful
  /// to set the Origin, Point1, Point2 of the plane source directly.
  vtkGetObjectMacro(PlaneSource, vtkPlaneSource);

  /// Get the widget coordinates as computed in the last call to
  /// ComputeComplexInteractionState.
  //vtkGetVector2Macro(WidgetCoordinates, int);
   
  /// Get spacing of the widget (mm/pixel)
  vtkGetMacro(SpacingMmPerPixel, double);
  /// Set spacing of the widget (mm/pixel)
  vtkSetMacro(SpacingMmPerPixel, double);

protected:
  /// Callback function observing texture modified events.
  static void OnTextureModified(vtkObject* caller, unsigned long eid, void* clientData, void* callData);

protected:
  //int WidgetCoordinates[2];
  double SpacingMmPerPixel{0.5};

  vtkPlaneSource* PlaneSource;
  vtkPolyDataMapper* PlaneMapper;
  vtkActor* PlaneActor;
  vtkOpenGLTexture* PlaneTexture;
  vtkSlicerQWidgetTexture* QWidgetTexture;
  vtkCallbackCommand* TextureCallbackCommand;

protected:
  vtkSlicerQWidgetRepresentation();
  ~vtkSlicerQWidgetRepresentation() override;

private:
  vtkSlicerQWidgetRepresentation(const vtkSlicerQWidgetRepresentation&) = delete;
  void operator=(const vtkSlicerQWidgetRepresentation&) = delete;
};

#endif
