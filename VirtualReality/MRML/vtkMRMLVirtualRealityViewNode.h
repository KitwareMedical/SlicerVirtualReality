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

// VTK includes
#include "vtkMRMLViewNode.h"

#include "vtkSlicerVirtualRealityModuleMRMLExport.h"

/// \brief MRML node to represent a 3D view.
///
/// View node contains view parameters.
class VTK_SLICER_VIRTUALREALITY_MODULE_MRML_EXPORT vtkMRMLVirtualRealityViewNode
  : public vtkMRMLViewNode
{
public:
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

  /// Controls two-sided lighting property of the renderer
  vtkGetMacro(TwoSidedLighting, bool);
  vtkSetMacro(TwoSidedLighting, bool);
  vtkBooleanMacro(TwoSidedLighting, bool);

  /// If enabled then 4 lights are used, otherwise just 2.
  vtkGetMacro(BackLights, bool);
  vtkSetMacro(BackLights, bool);
  vtkBooleanMacro(BackLights, bool);

protected:
  bool TwoSidedLighting;
  bool BackLights;

  vtkMRMLVirtualRealityViewNode();
  ~vtkMRMLVirtualRealityViewNode();
  vtkMRMLVirtualRealityViewNode(const vtkMRMLVirtualRealityViewNode&);
  void operator=(const vtkMRMLVirtualRealityViewNode&);

  static const char* ReferenceViewNodeReferenceRole;
};

#endif
