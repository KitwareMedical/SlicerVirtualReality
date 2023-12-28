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

#ifndef vtkVirtualRealityViewOpenVRInteractorStyle_h
#define vtkVirtualRealityViewOpenVRInteractorStyle_h

// VR MRMLDM includes
#include "vtkSlicerVirtualRealityModuleMRMLDisplayableManagerExport.h"
#include "vtkVirtualRealityViewInteractorStyleDelegate.h"

// VTK Rendering/OpenVR
#include <vtkOpenVRInteractorStyle.h>

// VTK includes
#include <vtkObject.h>
#include <vtkEventData.h>
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>

class vtkMRMLScene;
class vtkMRMLDisplayableManagerGroup;
class vtkWorldPointPicker;


class VTK_SLICER_VIRTUALREALITY_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT vtkVirtualRealityViewOpenVRInteractorStyle
  : public vtkOpenVRInteractorStyle
{
public:
  static vtkVirtualRealityViewOpenVRInteractorStyle *New();
  vtkTypeMacro(vtkVirtualRealityViewOpenVRInteractorStyle,vtkOpenVRInteractorStyle);

  ///@{
  /// Set/get delegate
  void SetInteractorStyleDelegate(vtkVirtualRealityViewInteractorStyleDelegate* delegate)
  {
    vtkSetSmartPointerBodyMacro(InteractorStyleDelegate, vtkVirtualRealityViewInteractorStyleDelegate, delegate);
    if (delegate != nullptr)
      {
      delegate->SetInteractorStyle(this);
      }
  }
  vtkGetSmartPointerMacro(InteractorStyleDelegate, vtkVirtualRealityViewInteractorStyleDelegate);
  ///}@

  //@{
  /**
  * Interaction mode entry points.
  */
  void StartPositionProp(vtkEventDataDevice3D * edata) override { this->InteractorStyleDelegate->StartPositionProp(edata); }
  void EndPositionProp(vtkEventDataDevice3D * edata) override { this->InteractorStyleDelegate->EndPositionProp(edata); }
  //@}

  //@{
  /**
  * Multitouch events binding.
  */
  void StartGesture() override { this->InteractorStyleDelegate->StartGesture(); }
  void EndGesture() override { this->InteractorStyleDelegate->EndGesture(); }
  void OnPan() override { this->InteractorStyleDelegate->OnPan(); }
  void OnPinch() override { this->InteractorStyleDelegate->OnPinch(); }
  void OnRotate() override { this->InteractorStyleDelegate->OnRotate(); }
  //@}

  //@{
  /**
  * Methods for interaction.
  */
  void PositionProp(vtkEventData* ed, double* lwpos = nullptr, double* lwori = nullptr) override
  {
    this->InteractorStyleDelegate->PositionProp(ed, lwpos, lwori);
  }
  //@}

protected:
  vtkVirtualRealityViewOpenVRInteractorStyle() = default;
  ~vtkVirtualRealityViewOpenVRInteractorStyle() override = default;

  vtkSmartPointer<vtkVirtualRealityViewInteractorStyleDelegate> InteractorStyleDelegate;

private:
  vtkVirtualRealityViewOpenVRInteractorStyle(const vtkVirtualRealityViewOpenVRInteractorStyle&) = delete;
  void operator=(const vtkVirtualRealityViewOpenVRInteractorStyle&) = delete;
};

#endif
