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

#ifndef __vtkMRMLGUIWidgetsDisplayableManager_h
#define __vtkMRMLGUIWidgetsDisplayableManager_h

// GUIWidget includes
#include "vtkSlicerGUIWidgetsModuleMRMLDisplayableManagerExport.h"

// GUIWidgetsModule/MRMLDisplayableManager includes
//#include "vtkMRMLGUIWidgetsDisplayableManagerHelper.h"

// MRMLDisplayableManager includes
#include <vtkMRMLAbstractDisplayableManager.h>

#include <vtkSlicerQWidgetWidget.h>

// STD includes
#include <map>

class vtkMRMLGUIWidgetNode;
class vtkSlicerViewerWidget;
class vtkMRMLGUIWidgetDisplayNode;
class vtkAbstractWidget;

/// \ingroup Slicer_QtModules_GUIWidgets
class  VTK_SLICER_GUIWIDGETS_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT vtkMRMLGUIWidgetsDisplayableManager
  : public vtkMRMLAbstractDisplayableManager
{
public:

  // Allow the helper to call protected methods of displayable manager
  //friend class vtkMRMLGUIWidgetsDisplayableManagerHelper;

  static vtkMRMLGUIWidgetsDisplayableManager *New();
  vtkTypeMacro(vtkMRMLGUIWidgetsDisplayableManager, vtkMRMLAbstractDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  /*
  /// Check if this is a 2d SliceView displayable manager, returns true if so,
  /// false otherwise. Checks return from GetSliceNode for non null, which means
  /// it's a 2d displayable manager
  virtual bool Is2DDisplayableManager();
  
  /// Get the sliceNode, if registered. This would mean it is a 2D SliceView displayableManager.
  vtkMRMLSliceNode * GetMRMLSliceNode();

  vtkMRMLGUIWidgetsDisplayableManagerHelper *  GetHelper() { return this->Helper; };

  bool CanProcessInteractionEvent(vtkMRMLInteractionEventData* eventData, double &closestDistance2) override;
  bool ProcessInteractionEvent(vtkMRMLInteractionEventData* eventData) override;

  void SetHasFocus(bool hasFocus) override;
  bool GetGrabFocus() override;
  bool GetInteractive() override;
  int GetMouseCursor() override;

  // Updates markup point preview position.
  // Returns true if the event is processed.
  vtkSlicerMarkupsWidget* GetWidgetForPlacement();

  vtkMRMLMarkupsNode* GetActiveMarkupsNodeForPlacement();

  int GetCurrentInteractionMode();

  // Methods from vtkMRMLAbstractSliceViewDisplayableManager

  /// Convert device coordinates (display) to XYZ coordinates (viewport).
  /// Parameter \a xyz is double[3]
  /// \sa ConvertDeviceToXYZ(vtkRenderWindowInteractor *, vtkMRMLSliceNode *, double x, double y, double xyz[3])
  void ConvertDeviceToXYZ(double x, double y, double xyz[3]);

  /// Get the widget of a node.
  vtkSlicerMarkupsWidget* GetWidget(vtkMRMLMarkupsDisplayNode * node);
  */
protected:

  vtkMRMLGUIWidgetsDisplayableManager();
  ~vtkMRMLGUIWidgetsDisplayableManager() override;
  /*
  vtkSlicerMarkupsWidget* FindClosestWidget(vtkMRMLInteractionEventData *callData, double &closestDistance2);
  */
  void ProcessMRMLNodesEvents(vtkObject *caller, unsigned long event, void *callData) override;

  /// Wrap the superclass render request in a check for batch processing
  virtual void RequestRender();

  /// Called from RequestRender method if UpdateFromMRMLRequested is true
  /// \sa RequestRender() SetUpdateFromMRMLRequested()
  void UpdateFromMRML() override;

  void SetMRMLSceneInternal(vtkMRMLScene* newScene) override;

  /// Called after the corresponding MRML event is triggered, from AbstractDisplayableManager
  /// \sa ProcessMRMLSceneEvents
  void UpdateFromMRMLScene() override;
  void OnMRMLSceneEndClose() override;
  void OnMRMLSceneEndImport() override;
  void OnMRMLSceneNodeAdded(vtkMRMLNode* node) override;
  void OnMRMLSceneNodeRemoved(vtkMRMLNode* node) override;

  /// Create a widget.
  vtkSlicerQWidgetWidget* CreateWidget(vtkMRMLGUIWidgetDisplayNode* node);

  /// Called after the corresponding MRML View container was modified
  void OnMRMLDisplayableNodeModifiedEvent(vtkObject* caller) override;
  /*
  /// Handler for specific SliceView actions, iterate over the widgets in the helper
  virtual void OnMRMLSliceNodeModifiedEvent();

  /// Observe the interaction node.
  void AddObserversToInteractionNode();
  void RemoveObserversFromInteractionNode();

  /// Check if it is the right displayManager
  virtual bool IsCorrectDisplayableManager();

  /// Return true if this displayable manager supports(can manage) that node,
  /// false otherwise.
  /// Can be reimplemented to add more conditions.
  /// \sa IsManageable(const char*), IsCorrectDisplayableManager()
  virtual bool IsManageable(vtkMRMLNode* node);
  /// Return true if this displayable manager supports(can manage) that node class,
  /// false otherwise.
  /// Can be reimplemented to add more conditions.
  /// \sa IsManageable(vtkMRMLNode*), IsCorrectDisplayableManager()
  virtual bool IsManageable(const char* nodeClassName);

  /// Respond to interactor style events
  void OnInteractorStyleEvent(int eventid) override;

  /// Accessor for internal flag that disables interactor style event processing
  vtkGetMacro(DisableInteractorStyleEventsProcessing, int);

  vtkSmartPointer<vtkMRMLGUIWidgetsDisplayableManagerHelper> Helper;

  double LastClickWorldCoordinates[4];

  vtkMRMLMarkupsNode* CreateNewMarkupsNode(const std::string &markupsNodeClassName);

  vtkWeakPointer<vtkSlicerMarkupsWidget> LastActiveWidget;
  */
private:
  vtkMRMLGUIWidgetsDisplayableManager(const vtkMRMLGUIWidgetsDisplayableManager&) = delete;
  void operator=(const vtkMRMLGUIWidgetsDisplayableManager&) = delete;
  /*
  int DisableInteractorStyleEventsProcessing;

  // by default, this displayableManager handles a 2d view, so the SliceNode
  // must be set when it's assigned to a viewer
  vtkWeakPointer<vtkMRMLSliceNode> SliceNode;
  */

  class vtkInternal;
  vtkInternal* Internal;
  friend class vtkInternal;
};

#endif
