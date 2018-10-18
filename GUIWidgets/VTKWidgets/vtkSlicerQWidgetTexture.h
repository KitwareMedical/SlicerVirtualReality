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

#ifndef vtkSlicerQWidgetTexture_h
#define vtkSlicerQWidgetTexture_h

#include "vtkSlicerGUIWidgetsModuleVTKWidgetsExport.h"

// VTK includes
#include <vtkImageData.h>
#include <vtkSmartPointer.h>
#include <vtkOpenGLTexture.h>
#include <vtkTrivialProducer.h>

#include <functional> // for ivar

class QGraphicsScene;
class QImage;
class QWidget;

/**
 * @class vtkSlicerQWidgetTexture
 * @brief Allows a QWidget to be used as a texture in VTK with OpenGL
 *
 * This class works by rendering the QWidget into a Framebuffer
 * and then sending the OpenGL texture handle to VTK for rendering.
 */
class VTK_SLICER_GUIWIDGETS_MODULE_VTKWIDGETS_EXPORT vtkSlicerQWidgetTexture : public vtkOpenGLTexture
{
public:
  static vtkSlicerQWidgetTexture* New();
  vtkTypeMacro(vtkSlicerQWidgetTexture, vtkOpenGLTexture);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  ///@{
  /**
   * Set/Get the QWidget that this TextureObject will render/use.
   * Just hold onto the widget until opengl context is active.
   */
  void SetWidget(QWidget* w);
  QWidget* GetWidget() { return this->Widget; }
  ///@}

  /**
   * get the QScene used for rendering, this is where events will
   * be forwarded to.
   */
  QGraphicsScene* GetScene() { return this->Scene; }

  /**
   * Free resources
   */
  void ReleaseGraphicsResources(vtkWindow* win) override;

protected:
  vtkSlicerQWidgetTexture();
  ~vtkSlicerQWidgetTexture() override;

  QGraphicsScene* Scene;
  QWidget* Widget;

  vtkSmartPointer<vtkImageData> TextureImageData;
  vtkSmartPointer<vtkTrivialProducer> TextureTrivialProducer;

  /// method called when the widget needs repainting
  std::function<void()> UpdateTextureMethod;

  /// Setup new widget with the graphics scene observation
  void SetupWidget();

private:
  vtkSlicerQWidgetTexture(const vtkSlicerQWidgetTexture&) = delete;
  void operator=(const vtkSlicerQWidgetTexture&) = delete;
};

#endif
