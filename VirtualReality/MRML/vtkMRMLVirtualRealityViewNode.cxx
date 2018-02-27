/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLVirtualRealityViewNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLScene.h"
#include "vtkMRMLVirtualRealityViewNode.h"

// VTK includes
#include <vtkObjectFactory.h>

// STD includes
#include <sstream>

const char* vtkMRMLVirtualRealityViewNode::ReferenceViewNodeReferenceRole = "ReferenceViewNodeRef";

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLVirtualRealityViewNode);

//----------------------------------------------------------------------------
vtkMRMLVirtualRealityViewNode::vtkMRMLVirtualRealityViewNode()
  : TwoSidedLighting(false)
  , BackLights(true)
{
  this->Visibility = 0; // hidden by default to not connect to the headset until it is needed
  this->BackgroundColor[0] = this->defaultBackgroundColor()[0];
  this->BackgroundColor[1] = this->defaultBackgroundColor()[1];
  this->BackgroundColor[2] = this->defaultBackgroundColor()[2];
  this->BackgroundColor2[0] = this->defaultBackgroundColor2()[0];
  this->BackgroundColor2[1] = this->defaultBackgroundColor2()[1];
  this->BackgroundColor2[2] = this->defaultBackgroundColor2()[2];
}

//----------------------------------------------------------------------------
vtkMRMLVirtualRealityViewNode::~vtkMRMLVirtualRealityViewNode()
{
}

//----------------------------------------------------------------------------
const char* vtkMRMLVirtualRealityViewNode::GetNodeTagName()
{
  return "VirtualRealityView";
}

//----------------------------------------------------------------------------
void vtkMRMLVirtualRealityViewNode::WriteXML(ostream& of, int nIndent)
{
  this->Superclass::WriteXML(of, nIndent);

  vtkMRMLWriteXMLBeginMacro(of);
  vtkMRMLWriteXMLBooleanMacro(twoSidedLighting, TwoSidedLighting);
  vtkMRMLWriteXMLBooleanMacro(backLights, BackLights);
  vtkMRMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLVirtualRealityViewNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  this->Superclass::ReadXMLAttributes(atts);

  vtkMRMLReadXMLBeginMacro(atts);
  vtkMRMLReadXMLBooleanMacro(twoSidedLighting, TwoSidedLighting);
  vtkMRMLReadXMLBooleanMacro(backLights, BackLights);
  vtkMRMLReadXMLEndMacro();

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLVirtualRealityViewNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  this->Superclass::Copy(anode);

  vtkMRMLCopyBeginMacro(anode);
  vtkMRMLCopyBooleanMacro(TwoSidedLighting);
  vtkMRMLCopyBooleanMacro(BackLights);
  vtkMRMLCopyEndMacro();

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLVirtualRealityViewNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  vtkMRMLPrintBeginMacro(os, indent);
  vtkMRMLPrintBooleanMacro(TwoSidedLighting);
  vtkMRMLPrintBooleanMacro(BackLights);
  vtkMRMLPrintEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLVirtualRealityViewNode::SetSceneReferences()
{
  if (!this->Scene)
    {
    vtkErrorMacro(<< "SetSceneReferences: Scene is expected to be non NULL.");
    return;
    }

  this->SetAndObserveParentLayoutNode(this);
}

//------------------------------------------------------------------------------
double* vtkMRMLVirtualRealityViewNode::defaultBackgroundColor()
{
  //static double backgroundColor[3] = {0.70196, 0.70196, 0.90588};
  static double backgroundColor[3] = {0.7568627450980392,
                                      0.7647058823529412,
                                      0.9098039215686275};
  return backgroundColor;
}

//------------------------------------------------------------------------------
double* vtkMRMLVirtualRealityViewNode::defaultBackgroundColor2()
{
  static double backgroundColor2[3] = {0.4549019607843137,
                                       0.4705882352941176,
                                       0.7450980392156863};
  return backgroundColor2;
}

//----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLVirtualRealityViewNode::GetReferenceViewNode()
{
  return this->GetNodeReference(this->ReferenceViewNodeReferenceRole);
}

//----------------------------------------------------------------------------
bool vtkMRMLVirtualRealityViewNode::SetAndObserveReferenceViewNodeID(const char* viewNodeId)
{
  if (!viewNodeId)
    {
    return false;
    }

  this->SetAndObserveNodeReferenceID(this->ReferenceViewNodeReferenceRole, viewNodeId);
  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLVirtualRealityViewNode::SetAndObserveReferenceViewNode(vtkMRMLNode* node)
{
  if (node && this->Scene != node->GetScene())
    {
    vtkErrorMacro("SetAndObserveReferenceViewNode: The referenced and referencing node are not in the same scene");
    return false;
    }
  if (!node->IsA("vtkMRMLViewNode"))
    {
    vtkErrorMacro("SetAndObserveReferenceViewNode: Wrong node type, vtkMRMLViewNode expected");
    return false;
    }

  return this->SetAndObserveReferenceViewNodeID(node ? node->GetID() : NULL);
}