/*==============================================================================

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was supported through the University of Western Ontario.

==============================================================================*/

#ifndef __vtkVirtualRealityViewInteractorObserver_h
#define __vtkVirtualRealityViewInteractorObserver_h

// MRML includes
#include "vtkMRMLViewInteractorStyle.h"

#include "vtkSlicerVirtualRealityModuleMRMLExport.h"

class VTK_SLICER_VIRTUALREALITY_MODULE_MRML_EXPORT vtkVirtualRealityViewInteractorObserver :
    public vtkMRMLViewInteractorStyle
{
public:
  static vtkVirtualRealityViewInteractorObserver *New();
  vtkTypeMacro(vtkVirtualRealityViewInteractorObserver,vtkMRMLViewInteractorStyle);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  using vtkMRMLViewInteractorStyle::DelegateInteractionEventToDisplayableManagers;

  /// Give a chance to displayable managers to process the event.
  /// Based on the return of vtkCommand::EventHasData(event), it either casts the
  /// calldata to a vtkEventData or just creates a one of type vtkMRMLInteractionEventData.
  /// In both case, it calls DelegateInteractionEventDataToDisplayableManagers
  /// passing a vtkEventData object.
  /// Return true if the event is processed.
  virtual bool DelegateInteractionEventToDisplayableManagers(unsigned long event, void* calldata);

  /// Give a chance to displayable managers to process the event.
  /// It creates vtkMRMLInteractionEventData and calls
  /// DelegateInteractionEventDataToDisplayableManagers.
  /// Return true if the event is processed.
  bool DelegateInteractionEventToDisplayableManagers(vtkEventData* inputEventData) override;

  /// Give a chance to displayable managers to process the event.
  /// Return true if the event is processed.
  using vtkMRMLViewInteractorStyle::DelegateInteractionEventDataToDisplayableManagers;
  virtual bool DelegateInteractionEventDataToDisplayableManagers(vtkMRMLInteractionEventData* eventData) override;

  /// Set the Interactor wrapper being controlled by this object. (Satisfy superclass API.)
  void SetInteractor(vtkRenderWindowInteractor *interactor) override;

  /// Main process event method.
  ///
  /// If calling DelegateInteractionEventToDisplayableManagers() returns false or if the current motion flag
  /// returned by vtkInteractorStyle::GetState() is VTKIS_NONE, call ProcessEvents().
  static void CustomProcessEvents(vtkObject* object, unsigned long event, void* clientdata, void* calldata);

  /// Process events not already delegated to displayable managers by CustomProcessEvents().
  static void ProcessEvents(vtkObject* object, unsigned long event, void* clientdata, void* calldata);

  /// 3D event bindings
  // OnMove3D: Already implemented in vtkMRMLViewInteractorStyle
  // OnButton3D: Already implemented in vtkMRMLViewInteractorStyle
  virtual void OnMenu3D(vtkEventData *edata);
  virtual void OnSelect3D(vtkEventData *edata);
  virtual void OnNextPose3D(vtkEventData *edata);
  virtual void OnViewerMovement3D(vtkEventData *edata);
  virtual void OnPick3D(vtkEventData *edata);
  virtual void OnPositionProp3D(vtkEventData *edata);
  virtual void OnClip3D(vtkEventData *edata);
  virtual void OnElevation3D(vtkEventData *edata);

protected:
  vtkVirtualRealityViewInteractorObserver();
  ~vtkVirtualRealityViewInteractorObserver() override;

private:
  vtkVirtualRealityViewInteractorObserver(const vtkVirtualRealityViewInteractorObserver&) = delete;
  void operator=(const vtkVirtualRealityViewInteractorObserver&) = delete;

  class vtkInternal;
  vtkInternal* Internal;
};

#endif
