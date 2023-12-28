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

  enum XRRuntimeType : int
    {
    OpenVR,
    XRRuntime_Last // must be last
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
  /// Convert between XR Runtime Interface identifier and name
  static const char* GetXRRuntimeAsString(int id);
  static int GetXRRuntimeFromString(const char* name);
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
