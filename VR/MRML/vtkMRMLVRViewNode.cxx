/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLVRViewNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLScene.h"
#include "vtkMRMLVRViewNode.h"

// VTK includes
#include <vtkObjectFactory.h>

// STD includes
#include <sstream>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLVRViewNode);


//----------------------------------------------------------------------------
vtkMRMLVRViewNode::vtkMRMLVRViewNode()
{
  this->BackgroundColor[0] = this->defaultBackgroundColor()[0];
  this->BackgroundColor[1] = this->defaultBackgroundColor()[1];
  this->BackgroundColor[2] = this->defaultBackgroundColor()[2];
  this->BackgroundColor2[0] = this->defaultBackgroundColor2()[0];
  this->BackgroundColor2[1] = this->defaultBackgroundColor2()[1];
  this->BackgroundColor2[2] = this->defaultBackgroundColor2()[2];
 }

//----------------------------------------------------------------------------
vtkMRMLVRViewNode::~vtkMRMLVRViewNode()
{
}

//----------------------------------------------------------------------------
const char* vtkMRMLVRViewNode::GetNodeTagName()
{
  return "VRView";
}

//----------------------------------------------------------------------------
void vtkMRMLVRViewNode::WriteXML(ostream& of, int nIndent)
{
	// Write all attributes not equal to their defaults

	this->Superclass::WriteXML(of, nIndent);

}

//----------------------------------------------------------------------------
void vtkMRMLVRViewNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  this->Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL)
  {
	  attName = *(atts++);
	  attValue = *(atts++);

    (void)(attName);
    (void)(attValue);
  }
  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLVRViewNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  this->Superclass::Copy(anode);
  vtkMRMLVRViewNode *node = (vtkMRMLVRViewNode *) anode;

  (void)(node);

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLVRViewNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

}

//------------------------------------------------------------------------------
double* vtkMRMLVRViewNode::defaultBackgroundColor()
{
  //static double backgroundColor[3] = {0.70196, 0.70196, 0.90588};
  static double backgroundColor[3] = {0.7568627450980392,
                                      0.7647058823529412,
                                      0.9098039215686275};
  return backgroundColor;
}

//------------------------------------------------------------------------------
double* vtkMRMLVRViewNode::defaultBackgroundColor2()
{
  static double backgroundColor2[3] = {0.4549019607843137,
                                       0.4705882352941176,
                                       0.7450980392156863};
  return backgroundColor2;
}
