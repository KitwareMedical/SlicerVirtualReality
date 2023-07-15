/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, and Cancer
  Care Ontario.

==============================================================================*/

#ifndef vtkVirtualRealityViewInteractor_h
#define vtkVirtualRealityViewInteractor_h

#include "vtkSlicerVirtualRealityModuleMRMLExport.h"

#include "vtkOpenVRRenderWindowInteractor.h"

#include <vector>

// vtkRenderingOpenVR is not python wrapped, so wrapping New causes linking error //TODO:
#ifndef __VTK_WRAP__

class VTK_SLICER_VIRTUALREALITY_MODULE_MRML_EXPORT vtkVirtualRealityViewInteractor : public vtkOpenVRRenderWindowInteractor
{
public:
  static vtkVirtualRealityViewInteractor *New();

  typedef vtkVirtualRealityViewInteractor Self;

  vtkTypeMacro(vtkVirtualRealityViewInteractor,vtkOpenVRRenderWindowInteractor);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  virtual void SetInteractorStyle(vtkInteractorObserver*) override;

  ///@{
  /// Define Slicer specific heuristic for handling complex gestures.
  ///
  /// See https://gitlab.kitware.com/vtk/vtk/-/merge_requests/9892
  virtual void HandleComplexGestureEvents(vtkEventData* ed) override;
  virtual void RecognizeComplexGesture(vtkEventDataDevice3D* edata) override;
  ///@}

  /// Set trigger button function
  /// By default it is the same as grab (\sa GetButtonFunctionIdForGrabObjectsAndWorld)
  /// Empty string disables button
  void SetTriggerButtonFunction(std::string functionId);

  /// Get string constant corresponding to button function "grab objects and world"
  static std::string GetButtonFunctionIdForGrabObjectsAndWorld() { return "GrabObjectsAndWorld"; };

  ///@{
  /// Convenience functions to easily associate grab and world functions to one or more buttons.
  /// When interaction with markups and other VTK MRML widgets will be implemented then we probably
  /// will not need these low-level event mappings anymore, but in the short term it is an effective
  /// workaround that enables prototyping of ideas.
  void SetGestureButtonToTrigger();
  void SetGestureButtonToGrip();
  void SetGestureButtonToTriggerAndGrip();
  void SetGestureButtonToNone();
  ///@}

protected:

  /// List of buttons for which gesture recognition is enabled
  std::vector<int> GestureEnabledButtons;

private:
  vtkVirtualRealityViewInteractor();
  ~vtkVirtualRealityViewInteractor();

  vtkVirtualRealityViewInteractor(const vtkVirtualRealityViewInteractor&) = delete;
  void operator=(const vtkVirtualRealityViewInteractor&) = delete;
};

#endif // __VTK_WRAP__

#endif
