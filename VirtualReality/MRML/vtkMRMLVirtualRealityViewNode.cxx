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
#include "vtkMRMLViewNode.h"
#include "vtkMRMLVirtualRealityViewNode.h"

// VTK includes
#include <vtkObjectFactory.h>

// STD includes
#include <sstream>

const char* vtkMRMLVirtualRealityViewNode::ReferenceViewNodeReferenceRole = "ReferenceViewNodeRef";
const char* vtkMRMLVirtualRealityViewNode::LeftControllerTransformRole = "LeftController";
const char* vtkMRMLVirtualRealityViewNode::RightControllerTransformRole = "RightController";

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLVirtualRealityViewNode);

//----------------------------------------------------------------------------
vtkMRMLVirtualRealityViewNode::vtkMRMLVirtualRealityViewNode()
  : TwoSidedLighting(false)
  , BackLights(true)
  , DesiredUpdateRate(60.0)
  , MotionSensitivity(0.5)
  , Magnification(10.0)
  , MotionSpeed(1.6666)
  , ControllerTransformsUpdate(false)
  , ControllerModelsVisible(true)
  , TrackHMD(true)
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
  vtkMRMLWriteXMLFloatMacro(desiredUpdateRate, DesiredUpdateRate);
  vtkMRMLWriteXMLFloatMacro(magnification, Magnification);
  vtkMRMLWriteXMLFloatMacro(motionSpeed,MotionSpeed);
  vtkMRMLWriteXMLFloatMacro(motionSensitivity, MotionSensitivity);
  vtkMRMLWriteXMLBooleanMacro(controllerTransformsUpdate, ControllerTransformsUpdate);
  vtkMRMLWriteXMLBooleanMacro(controllerModelsVisible, ControllerModelsVisible);
  vtkMRMLWriteXMLBooleanMacro(trackHMD, TrackHMD);
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
  vtkMRMLReadXMLFloatMacro(desiredUpdateRate, DesiredUpdateRate);
  vtkMRMLReadXMLFloatMacro(magnification, Magnification);
  vtkMRMLReadXMLFloatMacro(motionSpeed, MotionSpeed);
  vtkMRMLReadXMLFloatMacro(motionSensitivity, MotionSensitivity);
  vtkMRMLReadXMLBooleanMacro(controllerTransformsUpdate, ControllerTransformsUpdate);
  vtkMRMLReadXMLBooleanMacro(controllerModelsVisible, ControllerModelsVisible);
  vtkMRMLReadXMLBooleanMacro(trackHMD, TrackHMD);
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
  vtkMRMLCopyFloatMacro(DesiredUpdateRate);
  vtkMRMLCopyFloatMacro(Magnification);
  vtkMRMLCopyFloatMacro(MotionSpeed);
  vtkMRMLCopyFloatMacro(MotionSensitivity);
  vtkMRMLCopyBooleanMacro(ControllerTransformsUpdate);
  vtkMRMLCopyBooleanMacro(ControllerModelsVisible);
  vtkMRMLCopyBooleanMacro(TrackHMD);
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
  vtkMRMLPrintFloatMacro(DesiredUpdateRate);
  vtkMRMLPrintFloatMacro(Magnification);
  vtkMRMLPrintFloatMacro(MotionSpeed);
  vtkMRMLPrintFloatMacro(MotionSensitivity);
  vtkMRMLPrintBooleanMacro(ControllerTransformsUpdate);
  vtkMRMLPrintBooleanMacro(ControllerModelsVisible);
  vtkMRMLPrintBooleanMacro(TrackHMD);
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
vtkMRMLViewNode* vtkMRMLVirtualRealityViewNode::GetReferenceViewNode()
{
  return vtkMRMLViewNode::SafeDownCast(this->GetNodeReference(vtkMRMLVirtualRealityViewNode::ReferenceViewNodeReferenceRole));
}

//----------------------------------------------------------------------------
void vtkMRMLVirtualRealityViewNode::SetAndObserveReferenceViewNodeID(const char* viewNodeId)
{
  this->SetAndObserveNodeReferenceID(vtkMRMLVirtualRealityViewNode::ReferenceViewNodeReferenceRole, viewNodeId);
}

//----------------------------------------------------------------------------
bool vtkMRMLVirtualRealityViewNode::SetAndObserveReferenceViewNode(vtkMRMLViewNode* node)
{
  if (node == NULL)
    {
    this->SetAndObserveReferenceViewNodeID(NULL);
    return true;
    }
  if (this->Scene != node->GetScene() || node->GetID() == NULL)
    {
    vtkErrorMacro("SetAndObserveReferenceViewNode: The referenced and referencing node are not in the same scene");
    return false;
    }
   this->SetAndObserveReferenceViewNodeID(node->GetID());
   return true;
}

//----------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkMRMLVirtualRealityViewNode::GetControllerTransformNode(vtkEventDataDevice device)
{
  if (device == vtkEventDataDevice::LeftController)
  {
    return this->GetLeftControllerTransformNode();
  }
  else if(device == vtkEventDataDevice::RightController)
  {
    return this->GetRightControllerTransformNode();
  }
  else
  {
    return NULL;
  }
}

//----------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkMRMLVirtualRealityViewNode::GetLeftControllerTransformNode()
{
  return vtkMRMLLinearTransformNode::SafeDownCast(this->GetNodeReference(vtkMRMLVirtualRealityViewNode::LeftControllerTransformRole));
}

//----------------------------------------------------------------------------
const char* vtkMRMLVirtualRealityViewNode::GetLeftControllerTransformNodeID()
{
  return this->GetNodeReferenceID(vtkMRMLVirtualRealityViewNode::LeftControllerTransformRole);
}

//----------------------------------------------------------------------------
void vtkMRMLVirtualRealityViewNode::SetAndObserveLeftControllerTransformNodeID(const char *nodeId)
{
  this->SetAndObserveNodeReferenceID(vtkMRMLVirtualRealityViewNode::LeftControllerTransformRole, nodeId);
}

//----------------------------------------------------------------------------
bool vtkMRMLVirtualRealityViewNode::SetAndObserveLeftControllerTransformNode(vtkMRMLLinearTransformNode* node)
{
  if (node == NULL)
  {
    this->SetAndObserveLeftControllerTransformNodeID(NULL);
    return true;
  }
  if (this->Scene != node->GetScene() || node->GetID() == NULL)
  {
    vtkErrorMacro("SetAndObserveLeftControllerTransformNode: The referenced and referencing node are not in the same scene");
    return false;
  }
  this->SetAndObserveLeftControllerTransformNodeID(node->GetID());
  return true;
}

//----------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkMRMLVirtualRealityViewNode::GetRightControllerTransformNode()
{
  return vtkMRMLLinearTransformNode::SafeDownCast(this->GetNodeReference(vtkMRMLVirtualRealityViewNode::RightControllerTransformRole));
}

//----------------------------------------------------------------------------
const char* vtkMRMLVirtualRealityViewNode::GetRightControllerTransformNodeID()
{
  return this->GetNodeReferenceID(vtkMRMLVirtualRealityViewNode::RightControllerTransformRole);
}

//----------------------------------------------------------------------------
void vtkMRMLVirtualRealityViewNode::SetAndObserveRightControllerTransformNodeID(const char *nodeId)
{
  this->SetAndObserveNodeReferenceID(vtkMRMLVirtualRealityViewNode::RightControllerTransformRole, nodeId);
}

//----------------------------------------------------------------------------
bool vtkMRMLVirtualRealityViewNode::SetAndObserveRightControllerTransformNode(vtkMRMLLinearTransformNode* node)
{
  if (node == NULL)
  {
    this->SetAndObserveRightControllerTransformNodeID(NULL);
    return true;
  }
  if (this->Scene != node->GetScene() || node->GetID() == NULL)
  {
    vtkErrorMacro("SetAndObserveRightControllerTransformNode: The referenced and referencing node are not in the same scene");
    return false;
  }
  this->SetAndObserveRightControllerTransformNodeID(node->GetID());
  return true;
}

//----------------------------------------------------------------------------
void vtkMRMLVirtualRealityViewNode::CreateDefaultControllerTransformNodes()
{
  if (!this->GetScene())
  {
    return;
  }

  if (!this->GetLeftControllerTransformNodeID())
  {
    // We create or get and update the linear transform nodes that correspond to left and right controllers
    vtkSmartPointer<vtkMRMLLinearTransformNode> linearTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(this->GetScene()->GetSingletonNode("VirtualReality.LeftController", "vtkMRMLLinearTransformNode"));
    if (linearTransformNode == NULL)
    {
      linearTransformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::Take(
        vtkMRMLLinearTransformNode::SafeDownCast(this->GetScene()->CreateNodeByClass("vtkMRMLLinearTransformNode")));
      linearTransformNode->SetSingletonTag("VirtualReality.LeftController");
      linearTransformNode->SetName("VirtualReality.LeftController");
      this->GetScene()->AddNode(linearTransformNode);
    }
    this->SetAndObserveLeftControllerTransformNode(linearTransformNode);
  }

  if (!this->GetRightControllerTransformNodeID())
  {
    vtkSmartPointer<vtkMRMLLinearTransformNode> linearTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(this->GetScene()->GetSingletonNode("VirtualReality.RightController", "vtkMRMLLinearTransformNode"));
    if (linearTransformNode == NULL)
    {
      linearTransformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::Take(
        vtkMRMLLinearTransformNode::SafeDownCast(this->GetScene()->CreateNodeByClass("vtkMRMLLinearTransformNode")));
      linearTransformNode->SetSingletonTag("VirtualReality.RightController");
      linearTransformNode->SetName("VirtualReality.RightController");
      this->GetScene()->AddNode(linearTransformNode);
    }
    this->SetAndObserveRightControllerTransformNode(linearTransformNode);
  }
}

//----------------------------------------------------------------------------
void vtkMRMLVirtualRealityViewNode::SetControllerTransformsUpdate(bool enable)
{
  if (enable == this->ControllerTransformsUpdate)
  {
    // no change
    return;
  }

  this->ControllerTransformsUpdate = enable;

  if (!enable)
  {
    vtkMRMLLinearTransformNode* transformNode = this->GetLeftControllerTransformNode();
    if (transformNode)
    {
      transformNode->SetAttribute("VirtualReality.ControllerActive", "0");
      transformNode->SetAttribute("VirtualReality.ControllerConnected", "0");
    }
    transformNode = this->GetRightControllerTransformNode();
    if (transformNode)
    {
      transformNode->SetAttribute("VirtualReality.ControllerActive", "0");
      transformNode->SetAttribute("VirtualReality.ControllerConnected", "0");
    }
  }

  this->Modified();
}

//----------------------------------------------------------------------------
bool vtkMRMLVirtualRealityViewNode::HasError()
{
  return !this->LastErrorMessage.empty();
}

//----------------------------------------------------------------------------
void vtkMRMLVirtualRealityViewNode::ClearError()
{
  this->SetError("");
}

//----------------------------------------------------------------------------
void vtkMRMLVirtualRealityViewNode::SetError(const std::string& errorText)
{
  if (this->LastErrorMessage == errorText)
  {
    // no change
    return;
  }
  this->LastErrorMessage = errorText;
  this->Modified();
}

//----------------------------------------------------------------------------
std::string vtkMRMLVirtualRealityViewNode::GetError() const
{
  return this->LastErrorMessage;
}
