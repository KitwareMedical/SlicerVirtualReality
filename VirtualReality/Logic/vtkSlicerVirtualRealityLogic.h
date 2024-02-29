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

// .NAME vtkSlicerVirtualRealityLogic - slicer logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes


#ifndef __vtkSlicerVirtualRealityLogic_h
#define __vtkSlicerVirtualRealityLogic_h

// VR Logic includes
#include "vtkSlicerVirtualRealityModuleLogicExport.h"

// VR MRML includes
#include "vtkMRMLVirtualRealityViewNode.h"

// Slicer includes
#include <vtkSlicerModuleLogic.h>
class vtkSlicerVolumeRenderingLogic;

// VTK/Rendering/VR includes
class vtkVRRenderWindowInteractor;

// VTK includes
class vtkMatrix4x4;

// STD includes
#include <cstdlib>


class VTK_SLICER_VIRTUALREALITY_MODULE_LOGIC_EXPORT vtkSlicerVirtualRealityLogic :
  public vtkSlicerModuleLogic
{
public:
  static vtkSlicerVirtualRealityLogic* New();
  vtkTypeMacro(vtkSlicerVirtualRealityLogic, vtkSlicerModuleLogic);
  typedef vtkSlicerVirtualRealityLogic Self;
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Creates a singleton virtual reality view node and adds it to the scene.
  /// If there is a virtual reality view node in the scene already then it just returns that.
  /// If current view node is created, deleted, or modified then a Modified() event is invoked
  /// for this logic class, to make it easy for modules to detect view changes.
  vtkMRMLVirtualRealityViewNode* AddVirtualRealityViewNode();

  /// Get active singleton virtual reality node
  vtkMRMLVirtualRealityViewNode* GetVirtualRealityViewNode();

  /// Retrieves the default VR view node from the scene. Creates it if does not exist.
  vtkMRMLVirtualRealityViewNode* GetDefaultVirtualRealityViewNode();

  ///@{
  /// Set/get the XR backend.
  void SetVirtualRealityXRBackend(vtkMRMLVirtualRealityViewNode::XRBackendType id);
  vtkMRMLVirtualRealityViewNode::XRBackendType GetVirtualRealityXRBackend();
  ///}@

#ifndef __WRAP__
  /// Set the XR backend.
  ///
  /// Excluded from wrapping to avoid the following error:
  /// `TypeError: "ambuguous call, multiple overloaded methods match the arguments`
  void SetVirtualRealityXRBackend(int id);
#endif

  ///@{
  /// Connect/disconnect to headset.
  void SetVirtualRealityConnected(bool connect);
  bool GetVirtualRealityConnected();
  ///}@

  /// Enable rendering updates in headset.
  /// Connects to device if not yet connected.
  void SetVirtualRealityActive(bool activate);
  bool GetVirtualRealityActive();

  /// Set the first visible 3D view as reference view for
  /// virtual reality view.
  /// If a reference view has been already set then the
  /// method has no effect.
  void SetDefaultReferenceView();

  /// Modify MRML scene to be better suited for virtual reality, e.g.
  /// - Make sure volume rendering uses GPU
  /// - Turn off backface culling for all models for better visibility when clipping,
  ///   which occurs on head movement
  /// - Turn off slice intersection visibility for all models and segmentations
  ///   for performance improvement
  void OptimizeSceneForVirtualReality();

  /// Set volume rendering logic
  void SetVolumeRenderingLogic(vtkSlicerVolumeRenderingLogic* volumeRenderingLogic);

  /// Determines whether rendering should occur as quick view motion.
  ///
  /// This function evaluates the motion sensitivity and elapsed time to decide
  /// whether rendering should be considered as quick view motion. It computes
  /// limits based on the motion sensitivity and checks the change speed of
  /// view position, direction, and up vector to determine if it qualifies as
  /// quick view motion.
  ///
  /// \param motionSensitivity The sensitivity value (0.0 to 1.0) indicating the user's preference for motion.
  /// \param physicalScale The physical scale of the rendering.
  /// \param elapsedTimeInSec The time elapsed in seconds since the last view update.
  /// \param lastViewPos The position in the last view.
  /// \param lastViewDir The direction in the last view.
  /// \param lastViewUp The up vector in the last view.
  /// \param viewPos The current position.
  /// \param viewDir The current direction.
  /// \param viewUp The current up vector.
  ///
  /// \return True if rendering should be considered as quick view motion, false otherwise.
  ///
  /// \sa vtkMRMLVirtualRealityViewNode::MotionSensitivity()
  /// \sa vtkVRRenderWindow::GetPhysicalScale()
  static bool ShouldConsiderQuickViewMotion(
      double motionSensitivity, double physicalScale, double elapsedTimeInSec,
      double lastViewPos[3], double lastViewDir[3], double lastViewUp[3],
      double viewPos[3], double viewDir[3], double viewUp[3]);

  /// Calculate the average pose of the two controllers for pinch 3D operations
  ///
  /// \return Success flag. Failure happens when the average orientation coincides
  ///         with the direction of the displacement of the two controllers
  static bool CalculateCombinedControllerPose(
      vtkMatrix4x4* controller0Pose, vtkMatrix4x4* controller1Pose, vtkMatrix4x4* combinedPose);

  /// Set trigger button function
  /// By default it is the same as grab (\sa GetButtonFunctionIdForGrabObjectsAndWorld)
  /// Empty string disables button
  static void SetTriggerButtonFunction(vtkVRRenderWindowInteractor* rwi, const std::string& functionId);

  /// Get string constant corresponding to button function "grab objects and world"
  static std::string GetButtonFunctionIdForGrabObjectsAndWorld() { return "GrabObjectsAndWorld"; };

  ///@{
  /// Convenience functions to easily associate grab and world functions to one or more buttons.
  /// When interaction with markups and other VTK MRML widgets will be implemented then we probably
  /// will not need these low-level event mappings anymore, but in the short term it is an effective
  /// workaround that enables prototyping of ideas.
  static void SetGestureButtonToTrigger(vtkVRRenderWindowInteractor* rwi);
  static void SetGestureButtonToGrip(vtkVRRenderWindowInteractor* rwi);
  static void SetGestureButtonToTriggerAndGrip(vtkVRRenderWindowInteractor* rwi);
  static void SetGestureButtonToNone(vtkVRRenderWindowInteractor* rwi);
  ///@}

  ///@{
  /// Whether the module associated with this logic is loaded from an install tree or not.
  vtkSetMacro(ModuleInstalled, bool);
  vtkGetMacro(ModuleInstalled, bool);
  ///@}

  ///@{
  /// Utility functions for computing the ActionManifestPath based on the module share directory,
  /// XR backend and module install state.
  ///
  /// \sa vtkVRRenderWindowInteractor::SetActionManifestDirectory()
  std::string ComputeActionManifestPath(vtkMRMLVirtualRealityViewNode::XRBackendType xrBackend);
  static std::string ComputeActionManifestPath(
      const std::string& moduleShareDirectory, vtkMRMLVirtualRealityViewNode::XRBackendType xrBackend, bool installed);
  ///@}

  /// Initialize the active Virtual Reality view node.
  ///
  /// Checks if there is already an active Virtual Reality view node.
  /// If not, it looks for an existing node in the MRML scene.
  /// If no node is found, a new Virtual Reality view node is added to
  /// the scene and set as the active node.
  /// If the creation or retrieval fails, an error message is logged.
  ///
  /// The active view node is set internally for future use and can
  /// be obtained using GetActiveViewNode().
  void InitializeActiveViewNode();

protected:
  vtkSlicerVirtualRealityLogic();
  ~vtkSlicerVirtualRealityLogic() override;

  void SetActiveViewNode(vtkMRMLVirtualRealityViewNode* vrViewNode);

  void SetMRMLSceneInternal(vtkMRMLScene* newScene) override;
  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  void RegisterNodes() override;
  void UpdateFromMRMLScene() override;
  void OnMRMLSceneNodeAdded(vtkMRMLNode* node) override;
  void OnMRMLSceneNodeRemoved(vtkMRMLNode* node) override;
  void OnMRMLSceneEndImport() override;
  void ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void* callData) override;

protected:
  /// Active VR view node
  vtkMRMLVirtualRealityViewNode* ActiveViewNode;

  /// Volume rendering logic
  vtkSlicerVolumeRenderingLogic* VolumeRenderingLogic;

  bool ModuleInstalled{false};

private:
  vtkSlicerVirtualRealityLogic(const vtkSlicerVirtualRealityLogic&); // Not implemented
  void operator=(const vtkSlicerVirtualRealityLogic&); // Not implemented
};

#endif
