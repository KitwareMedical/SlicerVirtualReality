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

#ifndef __qSlicerVirtualRealityModule_h
#define __qSlicerVirtualRealityModule_h

// SlicerQt includes
#include "qSlicerLoadableModule.h"

#include "qSlicerVirtualRealityModuleExport.h"

#include <ctkVTKObject.h>

class vtkMRMLVirtualRealityViewNode;
class qSlicerVirtualRealityModulePrivate;
class qMRMLVirtualRealityView;
class QToolBar;

class Q_SLICER_QTMODULES_VIRTUALREALITY_EXPORT
  qSlicerVirtualRealityModule : public qSlicerLoadableModule
{
  Q_OBJECT;
  QVTK_OBJECT;
#ifdef Slicer_HAVE_QT5
  Q_PLUGIN_METADATA(IID "org.slicer.modules.loadable.qSlicerLoadableModule/1.0");
#endif
  Q_INTERFACES(qSlicerLoadableModule);

  /// Visibility of the Virtual Reality toolbar
  Q_PROPERTY(bool toolBarVisible READ isToolBarVisible WRITE setToolBarVisible)

public:

  typedef qSlicerLoadableModule Superclass;
  explicit qSlicerVirtualRealityModule(QObject* parent = nullptr);
  ~qSlicerVirtualRealityModule() override;

  qSlicerGetTitleMacro(QTMODULE_TITLE);

  QString helpText()const override;
  QString acknowledgementText()const override;
  QStringList contributors()const override;

  QIcon icon()const override;

  QStringList categories()const override;
  QStringList dependencies() const override;

  Q_INVOKABLE bool isToolBarVisible();
  Q_INVOKABLE QToolBar* toolBar();

  Q_INVOKABLE virtual qMRMLVirtualRealityView* viewWidget();

public slots:
  void setToolBarVisible(bool visible);
  void enableVirtualReality(bool);
  void updateViewFromReferenceViewCamera();
  void switchToVirtualRealityModule();
  void optimizeSceneForVirtualReality();

  /// Set MRML scene for the module. Updates the default view settings based on
  /// the application settings.
  void setMRMLScene(vtkMRMLScene* scene) override;

protected slots:
  void onViewNodeModified();

protected:

  /// Initialize the module
  void setup() override;

  /// Create and return the widget representation associated to this module
  qSlicerAbstractModuleRepresentation* createWidgetRepresentation() override;

  /// Create and return the logic associated to this module
  vtkMRMLAbstractLogic* createLogic() override;

protected:
  QScopedPointer<qSlicerVirtualRealityModulePrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerVirtualRealityModule);
  Q_DISABLE_COPY(qSlicerVirtualRealityModule);

};

#endif
