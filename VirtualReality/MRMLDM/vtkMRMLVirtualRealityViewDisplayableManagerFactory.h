/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __vtkMRMLVirtualRealityViewDisplayableManagerFactory_h
#define __vtkMRMLVirtualRealityViewDisplayableManagerFactory_h

// For:
//  - Slicer_VERSION_MAJOR
//  - Slicer_VERSION_MINOR
#include <vtkSlicerVersionConfigureMinimal.h>

// VR MRMLDM includes
#include "vtkSlicerVirtualRealityModuleMRMLDisplayableManagerExport.h"

// MRMLDisplayableManager includes
#include "vtkMRMLDisplayableManagerFactory.h"

// VTK includes
#include <vtkSingleton.h>

class vtkRenderer;

/// \brief Factory where displayable manager classes are registered.
///
/// A displayable manager class is responsible to represente a
/// MRMLDisplayable node in a renderer.
class VTK_SLICER_VIRTUALREALITY_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT vtkMRMLVirtualRealityViewDisplayableManagerFactory
  : public vtkMRMLDisplayableManagerFactory
{
public:

  vtkTypeMacro(vtkMRMLVirtualRealityViewDisplayableManagerFactory,
                       vtkMRMLDisplayableManagerFactory);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// This is a singleton pattern New.  There will only be ONE
  /// reference to a vtkMRMLVirtualRealityViewDisplayableManagerFactory object per process. Clients that
  /// call this must call Delete on the object so that the reference counting will work.
  /// The single instance will be unreferenced when the program exits.
  static vtkMRMLVirtualRealityViewDisplayableManagerFactory *New();

  /// Return the singleton instance with no reference counting.
  static vtkMRMLVirtualRealityViewDisplayableManagerFactory* GetInstance();

protected:

  vtkMRMLVirtualRealityViewDisplayableManagerFactory();
  ~vtkMRMLVirtualRealityViewDisplayableManagerFactory() override;

  VTK_SINGLETON_DECLARE(vtkMRMLVirtualRealityViewDisplayableManagerFactory);

private:

#if ((Slicer_VERSION_MAJOR == 5 && Slicer_VERSION_MINOR >= 7) || (Slicer_VERSION_MAJOR > 5))
  vtkMRMLVirtualRealityViewDisplayableManagerFactory(const vtkMRMLVirtualRealityViewDisplayableManagerFactory&) = delete;
  void operator=(const vtkMRMLVirtualRealityViewDisplayableManagerFactory&) = delete;
#else
  // legacy implementation for Slicer<5.7
  vtkMRMLVirtualRealityViewDisplayableManagerFactory(const vtkMRMLVirtualRealityViewDisplayableManagerFactory&);
  void operator=(const vtkMRMLVirtualRealityViewDisplayableManagerFactory&);
#endif

};

#ifndef __VTK_WRAP__
//BTX
VTK_SINGLETON_DECLARE_INITIALIZER(VTK_SLICER_VIRTUALREALITY_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT,
                                  vtkMRMLVirtualRealityViewDisplayableManagerFactory);
//ETX
#endif // __VTK_WRAP__

#endif
