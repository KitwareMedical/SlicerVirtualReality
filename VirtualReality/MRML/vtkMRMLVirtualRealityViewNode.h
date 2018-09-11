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

#include "vtkSlicerVirtualRealityModuleMRMLExport.h"

/// \brief MRML node to represent a 3D view.
///
/// View node contains view parameters.
class VTK_SLICER_VIRTUALREALITY_MODULE_MRML_EXPORT vtkMRMLVirtualRealityViewNode
  : public vtkMRMLViewNode
{
public:
  static const char* GetVirtualRealityInteractionTransformAttributeName() { return "VirtualReality.InteractionTransform"; }; 

  static vtkMRMLVirtualRealityViewNode *New();
  vtkTypeMacro(vtkMRMLVirtualRealityViewNode,vtkMRMLViewNode);
  void PrintSelf(ostream& os, vtkIndent indent) VTK_OVERRIDE;

  //--------------------------------------------------------------------------
  /// MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance() VTK_OVERRIDE;

  ///
  /// Read node attributes from XML file
  virtual void ReadXMLAttributes( const char** atts) VTK_OVERRIDE;

  ///
  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent) VTK_OVERRIDE;

  ///
  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node) VTK_OVERRIDE;

  ///
  /// Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() VTK_OVERRIDE;

  /// Update the references of the node to the scene.
  virtual void SetSceneReferences();

  /// Return the color the view nodes have for the background by default.
  static double* defaultBackgroundColor();
  static double* defaultBackgroundColor2();

  /// Get reference view node.
  /// Reference view node is a regular 3D view node, which content
  /// or view may be synchronized with the virtual reality camera view.
  vtkMRMLViewNode* GetReferenceViewNode();
  /// Set reference view node.
  /// \sa GetReferenceViewNode
  void SetAndObserveReferenceViewNodeID(const char *layoutNodeId);
  /// Set reference view node.
  /// \sa GetReferenceViewNode
  bool SetAndObserveReferenceViewNode(vtkMRMLViewNode* node);

  /// Create controller transform nodes if not set already.
  void CreateDefaultControllerTransformNodes();

  /// Get controller node by device identifier
  vtkMRMLLinearTransformNode* GetControllerTransformNode(vtkEventDataDevice device);

  /// Get left controller transform node.
  /// Left controller transform node contains the 3D pose of the left controller
  vtkMRMLLinearTransformNode* GetLeftControllerTransformNode();
  const char* GetLeftControllerTransformNodeID();
  /// Set left controller transform node.
  /// \sa GetLeftControllerTransformNode
  void SetAndObserveLeftControllerTransformNodeID(const char *nodeId);
  /// Set left controller transform node.
  /// \sa GetLeftControllerTransformNode
  bool SetAndObserveLeftControllerTransformNode(vtkMRMLLinearTransformNode* node);

  /// Get right controller transform node.
  /// Right controller transform node contains the 3D pose of the right controller
  vtkMRMLLinearTransformNode* GetRightControllerTransformNode();
  const char* GetRightControllerTransformNodeID();
  /// Set right controller transform node.
  /// \sa GetRightControllerTransformNode
  void SetAndObserveRightControllerTransformNodeID(const char *nodeId);
  /// Set right controller transform node.
  /// \sa GetRightControllerTransformNode
  bool SetAndObserveRightControllerTransformNode(vtkMRMLLinearTransformNode* node);

  /// Controls two-sided lighting property of the renderer
  vtkGetMacro(TwoSidedLighting, bool);
  vtkSetMacro(TwoSidedLighting, bool);
  vtkBooleanMacro(TwoSidedLighting, bool);

  /// If enabled then 4 lights are used, otherwise just 2.
  vtkGetMacro(BackLights, bool);
  vtkSetMacro(BackLights, bool);
  vtkBooleanMacro(BackLights, bool);

  /// Desired frame rate. Volume renderer may use this information
  /// for determining sampling distances (and other LOD actors, to
  /// determine display quality).
  vtkGetMacro(DesiredUpdateRate, double);
  vtkSetMacro(DesiredUpdateRate, double);

  /// Motion sensitivity (between 0.0 and 1.0).
  /// If virtual reality headset is not moving then update rate
  /// is decreased to allow higher quality rendering.
  vtkGetMacro(MotionSensitivity, double);
  vtkSetMacro(MotionSensitivity, double);

  /// If enabled then pose of controllers are saved in the scene as transforms.
  vtkGetMacro(ControllerTransformsUpdate, bool);
  void SetControllerTransformsUpdate(bool enable);
  vtkBooleanMacro(ControllerTransformsUpdate, bool);

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
  double MotionSensitivity;
  bool ControllerTransformsUpdate;
  std::string LastErrorMessage;

  vtkMRMLVirtualRealityViewNode();
  ~vtkMRMLVirtualRealityViewNode();
  vtkMRMLVirtualRealityViewNode(const vtkMRMLVirtualRealityViewNode&);
  void operator=(const vtkMRMLVirtualRealityViewNode&);

  static const char* ReferenceViewNodeReferenceRole;
  static const char* LeftControllerTransformRole;
  static const char* RightControllerTransformRole;
};

#endif
