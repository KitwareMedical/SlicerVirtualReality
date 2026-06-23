/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLVirtualRealityViewNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/

#ifndef __vtkMRMLVirtualRealityViewNode_h
#define __vtkMRMLVirtualRealityViewNode_h

// MRML includes
#include <vtkMRMLViewNode.h>
#include <vtkMRMLLinearTransformNode.h>

// VTK includes
#include <vtkEventData.h>

// VR MRML includes
#include "vtkSlicerVirtualRealityModuleMRMLExport.h"

/// \brief MRML node to represent a 3D view.
///
/// View node contains view parameters.
class VTK_SLICER_VIRTUALREALITY_MODULE_MRML_EXPORT vtkMRMLVirtualRealityViewNode
  : public vtkMRMLViewNode
{
public:
  static const char* GetVirtualRealityInteractionTransformAttributeName() { return "VirtualReality.InteractionTransform"; };

  static vtkMRMLVirtualRealityViewNode* New();
  vtkTypeMacro(vtkMRMLVirtualRealityViewNode, vtkMRMLViewNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  enum XRBackendType : int
    {
    UndefinedXRBackend,
    OpenVR,
    OpenXR,
    XRBackend_Last // must be last
    };

  //--------------------------------------------------------------------------
  /// MRMLNode methods
  //--------------------------------------------------------------------------

  vtkMRMLNode* CreateNodeInstance() override;

  /// Read node attributes from XML file
  void ReadXMLAttributes(const char** atts) override;

  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy the node's attributes to this object
  void Copy(vtkMRMLNode* node) override;

  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override;

  /// Update the references of the node to the scene.
  void SetSceneReferences() override;

  ///@{
  /// Return the color the view nodes have for the background by default.
  static double* defaultBackgroundColor();
  static double* defaultBackgroundColor2();
  ///}@

  /// Get reference view node.
  /// Reference view node is a regular 3D view node, which content
  /// or view may be synchronized with the virtual reality camera view.
  vtkMRMLViewNode* GetReferenceViewNode();
  /// Set reference view node.
  /// \sa GetReferenceViewNode
  void SetAndObserveReferenceViewNodeID(const char* layoutNodeId);
  /// Set reference view node.
  /// \sa GetReferenceViewNode
  bool SetAndObserveReferenceViewNode(vtkMRMLViewNode* node);

  /// Create controller transform nodes if not set already.
  void CreateDefaultControllerTransformNodes();

  /// Create camera transform node if not set already.
  void CreateDefaultHMDTransformNode();

  /// Create generic tracker transform node if not set already.
  vtkMRMLLinearTransformNode* CreateDefaultTrackerTransformNode(uint32_t deviceHandle);

  /// Get controller node by device identifier
  vtkMRMLLinearTransformNode* GetControllerTransformNode(vtkEventDataDevice device);

  /// Get left controller transform node.
  /// Left controller transform node contains the 3D pose of the left controller
  vtkMRMLLinearTransformNode* GetLeftControllerTransformNode();
  const char* GetLeftControllerTransformNodeID();
  /// Set left controller transform node.
  /// \sa GetLeftControllerTransformNode
  void SetAndObserveLeftControllerTransformNodeID(const char* nodeId);
  /// Set left controller transform node.
  /// \sa GetLeftControllerTransformNode
  bool SetAndObserveLeftControllerTransformNode(vtkMRMLLinearTransformNode* node);

  /// Get right controller transform node.
  /// Right controller transform node contains the 3D pose of the right controller
  vtkMRMLLinearTransformNode* GetRightControllerTransformNode();
  const char* GetRightControllerTransformNodeID();
  /// Set right controller transform node.
  /// \sa GetRightControllerTransformNode
  void SetAndObserveRightControllerTransformNodeID(const char* nodeId);
  /// Set right controller transform node.
  /// \sa GetRightControllerTransformNode
  bool SetAndObserveRightControllerTransformNode(vtkMRMLLinearTransformNode* node);

  /// Get HMD transform node
  vtkMRMLLinearTransformNode* GetHMDTransformNode();
  const char* GetHMDTransformNodeID();
  /// Set HMD transform node.
  /// \sa GetHMDTransformNode
  void SetAndObserveHMDTransformNodeID(const char* nodeId);
  /// Set HMD transform node.
  /// \sa GetHMDTransformNode
  bool SetAndObserveHMDTransformNode(vtkMRMLLinearTransformNode* node);

  /// Get generic tracker transform node
  std::vector<vtkMRMLLinearTransformNode*> GetTrackerTransformNodes();
  vtkMRMLLinearTransformNode* GetTrackerTransformNode(uint32_t deviceHandle);
  const char* GetTrackerTransformNodeID(uint32_t deviceHandle);
  /// Set tracker transform node.
  /// \sa GetTrackerTransformNode
  vtkMRMLLinearTransformNode* SetAndObserveTrackerTransformNodeID(const char* nodeId, uint32_t deviceHandle);
  /// Set tracker transform node.
  /// \sa GetTrackerTransformNode
  vtkMRMLLinearTransformNode* SetAndObserveTrackerTransformNode(vtkMRMLLinearTransformNode* node, uint32_t deviceHandle);
  /// Remove a tracker transform node.
  /// \sa SetAndObserveTrackerTransformNode
  void RemoveTrackerTransformNode(uint32_t deviceHandle);
  /// Remove all tracker transform node.
  /// \sa SetAndObserveTrackerTransformNode
  void RemoveAllTrackerTransformNodes();

  ///@{
  /// Controls two-sided lighting property of the renderer
  vtkGetMacro(TwoSidedLighting, bool);
  vtkSetMacro(TwoSidedLighting, bool);
  vtkBooleanMacro(TwoSidedLighting, bool);
  ///}@

  ///@{
  /// If enabled then 4 lights are used, otherwise just 2.
  vtkGetMacro(BackLights, bool);
  vtkSetMacro(BackLights, bool);
  vtkBooleanMacro(BackLights, bool);
  ///}@

  ///@{
  /// Desired frame rate. Volume renderer may use this information
  /// for determining sampling distances (and other LOD actors, to
  /// determine display quality).
  vtkGetMacro(DesiredUpdateRate, double);
  vtkSetMacro(DesiredUpdateRate, double);
  ///}@

  ///@{
  /// Magnification of world [0.01, 100].
  /// Value greater than 1 means that objects appear larger in VR than their real world size.
  /// Translated to physical scale of the VR render window
  vtkGetMacro(Magnification, double);
  vtkSetMacro(Magnification, double);
  ///}@

  ///@{
  /// Motion speed of fly (i.e. dolly) in meters per second.
  /// Default is walking speed: 6 km/h = 1.66 m/s
  vtkGetMacro(MotionSpeed, double);
  vtkSetMacro(MotionSpeed, double);
  ///}@

  /// Motion sensitivity (between 0.0 and 1.0).
  /// If virtual reality headset is not moving then update rate
  /// is decreased to allow higher quality rendering.
  vtkGetMacro(MotionSensitivity, double);
  vtkSetMacro(MotionSensitivity, double);
  ///}@

  ///@{
  /// If enabled then pose of controllers are saved in the scene as transforms.
  vtkGetMacro(ControllerTransformsUpdate, bool);
  void SetControllerTransformsUpdate(bool enable);
  vtkBooleanMacro(ControllerTransformsUpdate, bool);
  ///}@

  ///@{
  vtkGetMacro(HMDTransformUpdate, bool);
  void SetHMDTransformUpdate(bool enable);
  vtkBooleanMacro(HMDTransformUpdate, bool);
  ///}@

  ///@{
  vtkGetMacro(TrackerTransformUpdate, bool);
  void SetTrackerTransformUpdate(bool enable);
  vtkBooleanMacro(TrackerTransformUpdate, bool);
  ///}@

  ///@{
  /// If set to true then controllers are visible in virtual reality view.
  vtkGetMacro(ControllerModelsVisible, bool);
  vtkSetMacro(ControllerModelsVisible, bool);
  vtkBooleanMacro(ControllerModelsVisible, bool);
  ///}@

  ///@{
  /// If set to true then tracking references (Lighthouses) are visible in virtual reality view.
  vtkGetMacro(LighthouseModelsVisible, bool);
  vtkSetMacro(LighthouseModelsVisible, bool);
  vtkBooleanMacro(LighthouseModelsVisible, bool);
  ///}@

  ///@{
  /// Get/Set the XR backend.
  vtkGetMacro(XRBackend, XRBackendType);
  vtkSetMacro(XRBackend, XRBackendType);
  ///@}

#ifndef __WRAP__
  /// Set the XR backend.
  ///
  /// Excluded from wrapping to avoid the following error:
  /// `TypeError: "ambuguous call, multiple overloaded methods match the arguments`
  void SetXRBackend(int id);
#endif

  ///@{
  /// Convert between XR backend identifier and name
  static const char* GetXRBackendAsString(int id);
  static int GetXRBackendFromString(const char* name);
  ///@}

  ///@{
  /// Get/Set if remoting is enabled.
  vtkGetMacro(Remoting, bool);
  vtkSetMacro(Remoting, bool);
  vtkBooleanMacro(Remoting, bool);
  ///@}

  ///@{
  /// OpenXR remoting IP address to connect to.
  vtkSetMacro(PlayerIPAddress, const std::string);
  vtkGetMacro(PlayerIPAddress, std::string);
  ///@}

  ///@{
  /// Enable camera passthrough (mixed reality / AR mode) for OpenXR.
  ///
  /// When enabled, the real-world view seen through the headset cameras is
  /// blended with the rendered 3D scene using
  /// XR_ENVIRONMENT_BLEND_MODE_ALPHA_BLEND.  The scene background is rendered
  /// fully transparent so the camera feed shows through.
  ///
  /// This option only has an effect when the XR backend is OpenXR and the
  /// runtime supports XR_ENVIRONMENT_BLEND_MODE_ALPHA_BLEND (e.g. Meta Quest 3
  /// via Quest Link / Air Link).
  ///
  /// Changing this setting requires reconnecting to the headset.
  vtkGetMacro(Passthrough, bool);
  vtkSetMacro(Passthrough, bool);
  vtkBooleanMacro(Passthrough, bool);
  ///@}

  ///@{
  /// Opacity applied to virtual geometry that is occluded by real-world
  /// surfaces when XR_META_environment_depth occlusion is active.
  ///
  ///  0.0 (default) – fully occluded (hard depth pre-pass).
  ///  0.0 < x < 1.0 – partial occlusion (post-pass alpha blend).
  ///  1.0            – no depth composition (bypass entirely).
  ///
  /// Has no effect when env-depth is unavailable or inactive.
  vtkGetMacro(OccludedOpacity, double);
  vtkSetClampMacro(OccludedOpacity, double, 0.0, 1.0);
  ///@}

  ///@{
  /// Show a false-colour overlay of the real-world depth texture for debugging.
  /// Red = near (0 m), blue = far (~5 m).
  /// Has no effect when XR_META_environment_depth is unavailable or inactive.
  vtkGetMacro(EnvDepthDebugVisualization, bool);
  vtkSetMacro(EnvDepthDebugVisualization, bool);
  vtkBooleanMacro(EnvDepthDebugVisualization, bool);
  ///@}

  ///@{
  /// If enabled, the rendered left-eye VR scene is captured each render tick
  /// and written to a vtkMRMLVectorVolumeNode named "VR Scene (Left Eye)"
  /// (RGB, unsigned char).
  ///
  /// The captured image is the rendered VR scene for the left eye as submitted
  /// to the display. This is not the raw headset camera feed; it is the
  /// composited output of the VR renderer (geometry, overlays, etc.).
  /// When XR_ENVIRONMENT_BLEND_MODE_ALPHA_BLEND (passthrough) is active,
  /// transparent areas will include the real-world camera feed composited by
  /// the runtime, but the primary content is the rendered scene.
  vtkGetMacro(VRSceneColorVolumeEnabled, bool);
  vtkSetMacro(VRSceneColorVolumeEnabled, bool);
  vtkBooleanMacro(VRSceneColorVolumeEnabled, bool);
  ///@}

  ///@{
  /// If enabled, the real-world environment depth is captured each render tick
  /// (via XR_META_environment_depth) and written to a vtkMRMLScalarVolumeNode
  /// named "Passthrough Depth".  Scalar values are in millimetres (float).
  ///
  /// Has no effect if XR_META_environment_depth is unavailable or inactive.
  vtkGetMacro(PassthroughDepthVolumeEnabled, bool);
  vtkSetMacro(PassthroughDepthVolumeEnabled, bool);
  vtkBooleanMacro(PassthroughDepthVolumeEnabled, bool);
  ///@}

  /// Return true if an error has occurred.
  /// "Connected" member requests connection but this method can tell if the
  /// hardware connection has been actually successfully established.
  bool HasError();

  /// Clear error state.
  void ClearError();

  /// Set error message. Non-empty string means that an error has occurred.
  void SetError(const std::string& errorText);

  /// Get error message. Non-empty string means that an error has occurred.
  std::string GetError() const;

protected:
  XRBackendType XRBackend{vtkMRMLVirtualRealityViewNode::UndefinedXRBackend};

  bool TwoSidedLighting;
  bool BackLights;
  double DesiredUpdateRate;
  double Magnification;
  double MotionSpeed;
  double MotionSensitivity;
  bool ControllerTransformsUpdate;
  bool HMDTransformUpdate;
  bool ControllerModelsVisible;
  bool LighthouseModelsVisible;
  bool TrackerTransformUpdate;

  std::string LastErrorMessage;

  // OpenXRRemoting
  bool Remoting{false};
  std::string PlayerIPAddress;

  // Passthrough (OpenXR alpha blend mode)
  bool Passthrough{false};

  // Environment-depth occlusion (XR_META_environment_depth)
  double OccludedOpacity{1.0};
  bool EnvDepthDebugVisualization{false};

  // VR scene / passthrough volume capture
  bool VRSceneColorVolumeEnabled{false};
  bool PassthroughDepthVolumeEnabled{false};

  vtkMRMLVirtualRealityViewNode();
  ~vtkMRMLVirtualRealityViewNode() override;
  vtkMRMLVirtualRealityViewNode(const vtkMRMLVirtualRealityViewNode&);
  void operator=(const vtkMRMLVirtualRealityViewNode&);

  static const char* ReferenceViewNodeReferenceRole;
  static const char* LeftControllerTransformRole;
  static const char* RightControllerTransformRole;
  static const char* HMDTransformRole;
  static const char* TrackerTransformRole;
};

#endif
