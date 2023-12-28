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

// VirtualRealityModule includes
#include "vtkSlicerVirtualRealityModuleMRMLDisplayableManagerExport.h"
#include "vtkVirtualRealityComplexGestureRecognizer.h"

// VTK includes
#include <vtkNew.h>
#include <vtkOpenVRRenderWindowInteractor.h>

class VTK_SLICER_VIRTUALREALITY_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT vtkVirtualRealityViewInteractor
  : public vtkOpenVRRenderWindowInteractor
{
public:
  static vtkVirtualRealityViewInteractor *New();
  vtkTypeMacro(vtkVirtualRealityViewInteractor,vtkOpenVRRenderWindowInteractor);

  ///@{
  /// Define Slicer specific heuristic for handling complex gestures.
  virtual void HandleComplexGestureEvents(vtkEventData* ed) override
  {
    this->ComplexGestureRecognizer->HandleComplexGestureEvents(ed);
  }
  virtual void RecognizeComplexGesture(vtkEventDataDevice3D* edata) override
  {
    this->ComplexGestureRecognizer->RecognizeComplexGesture(edata);
  }
  ///@}

protected:
  vtkNew<vtkVirtualRealityComplexGestureRecognizer> ComplexGestureRecognizer;

private:
  vtkVirtualRealityViewInteractor()
  {
    this->ComplexGestureRecognizer->SetInteractor(this);
  }
  ~vtkVirtualRealityViewInteractor() override = default;

  vtkVirtualRealityViewInteractor(const vtkVirtualRealityViewInteractor&) = delete;
  void operator=(const vtkVirtualRealityViewInteractor&) = delete;
};

#endif
