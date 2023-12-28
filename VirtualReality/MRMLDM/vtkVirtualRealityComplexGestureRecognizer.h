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

#ifndef vtkVirtualRealityComplexGestureRecognizer_h
#define vtkVirtualRealityComplexGestureRecognizer_h

// VR MRMLDM includes
#include "vtkSlicerVirtualRealityModuleMRMLDisplayableManagerExport.h"

// VTK/Rendering/VR includes
class vtkVRRenderWindowInteractor;

// VTK includes
#include <vtkObject.h>
#include <vtkSmartPointer.h>
class vtkEventData;
class vtkEventDataDevice3D;


class VTK_SLICER_VIRTUALREALITY_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT vtkVirtualRealityComplexGestureRecognizer
  : public vtkObject
{
public:
  static vtkVirtualRealityComplexGestureRecognizer *New();
  typedef vtkVirtualRealityComplexGestureRecognizer Self;
  vtkTypeMacro(vtkVirtualRealityComplexGestureRecognizer,vtkObject);

  ///@{
  /// Interactor to recognize the complex gestures from.
  void SetInteractor(vtkVRRenderWindowInteractor* interactor);
  vtkVRRenderWindowInteractor* GetInteractor() const;
  ///}@

  ///@{
  /// Define Slicer specific heuristic for handling complex gestures.
  void HandleComplexGestureEvents(vtkEventData* ed);
  void RecognizeComplexGesture(vtkEventDataDevice3D* edata);
  ///@}

protected:
  vtkSmartPointer<vtkVRRenderWindowInteractor> Interactor;

private:
  vtkVirtualRealityComplexGestureRecognizer() = default;
  ~vtkVirtualRealityComplexGestureRecognizer() override = default;

  vtkVirtualRealityComplexGestureRecognizer(const vtkVirtualRealityComplexGestureRecognizer&) = delete;
  void operator=(const vtkVirtualRealityComplexGestureRecognizer&) = delete;
};

#endif
