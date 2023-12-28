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

#ifndef vtkVirtualRealityViewInteractorStyleDelegate_h
#define vtkVirtualRealityViewInteractorStyleDelegate_h

// VR MRMLDM includes
#include "vtkSlicerVirtualRealityModuleMRMLDisplayableManagerExport.h"

// MRML includes
class vtkMRMLDisplayableNode;
class vtkMRMLScene;

// MRMLDM includes
#include <vtkMRMLDisplayableManagerGroup.h>

// VTK/Rendering/VR includes
#include <vtkVRInteractorStyle.h>

// VTK includes
#include <vtkEventData.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObject.h>
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>


class VTK_SLICER_VIRTUALREALITY_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT vtkVirtualRealityViewInteractorStyleDelegate
  : public vtkObject
{
public:
  static vtkVirtualRealityViewInteractorStyleDelegate *New();
  typedef vtkVirtualRealityViewInteractorStyleDelegate Self;
  vtkTypeMacro(vtkVirtualRealityViewInteractorStyleDelegate,vtkObject);

  ///@{
  /// Interactor style to perform the complex gesture from.
  void SetInteractorStyle(vtkVRInteractorStyle* interactor);
  vtkGetSmartPointerMacro(InteractorStyle, vtkVRInteractorStyle);
  ///}@

  ///@{
  /// Set/get displayable managers
  vtkSetSmartPointerMacro(DisplayableManagers, vtkMRMLDisplayableManagerGroup);
  vtkGetSmartPointerMacro(DisplayableManagers, vtkMRMLDisplayableManagerGroup);
  ///}@

  /// Get MRML scene from the displayable manager group (the first displayable manager's if any)
  /// \sa GetDisplayableManagers()
  vtkMRMLScene* GetMRMLScene() const;

  ///@{
  /// Multitouch events binding delegates.
  void StartGesture();
  void EndGesture();
  void OnPan();
  void OnPinch();
  void OnRotate();
  ///@}

  ///@{
  /// Interaction mode entry points delegates.
  void StartPositionProp(vtkEventDataDevice3D *);
  void EndPositionProp(vtkEventDataDevice3D *);
  ///}@

  ///@{
  /// Interaction method delegates.
  void PositionProp(vtkEventData*, double* lwpos = nullptr, double* lwori = nullptr);
  ///@}

  ///@{
  /// Enable/disable picking.
  vtkSetMacro(GrabEnabled, bool)
  vtkGetMacro(GrabEnabled, bool);
  vtkBooleanMacro(GrabEnabled, bool);
  ///@}

  ///@{
  /// Set physical to world magnification. Valid value range is [0.01, 100].
  /// Note: Conversion is physicalScale = 1000 / magnification
  void SetMagnification(double magnification);
  double GetMagnification();
  ///}@

protected:
  vtkWeakPointer<vtkVRInteractorStyle> InteractorStyle;

  void OnPinch3D();

  vtkNew<vtkMatrix4x4> CombinedStartingControllerPose;
  bool StartingControllerPoseValid{false};

  vtkNew<vtkMatrix4x4> LastValidCombinedControllerPose;
  bool LastValidCombinedControllerPoseExists{false};

  bool GrabEnabled{true};
  vtkWeakPointer<vtkMRMLDisplayableNode> PickedNode[vtkEventDataNumberOfDevices];
  vtkWeakPointer<vtkMRMLDisplayableManagerGroup> DisplayableManagers;

private:
  vtkVirtualRealityViewInteractorStyleDelegate() = default;
  ~vtkVirtualRealityViewInteractorStyleDelegate() override = default;

  vtkVirtualRealityViewInteractorStyleDelegate(const vtkVirtualRealityViewInteractorStyleDelegate&) = delete;
  void operator=(const vtkVirtualRealityViewInteractorStyleDelegate&) = delete;
};

#endif
