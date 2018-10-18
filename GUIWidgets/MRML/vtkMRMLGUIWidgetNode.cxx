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

  This file was originally developed by Csaba Pinter, EBATINCA, S.L., and
  development was supported by "ICEX Espana Exportacion e Inversiones" under
  the program "Inversiones de Empresas Extranjeras en Actividades de I+D
  (Fondo Tecnologico)- Convocatoria 2021"

==============================================================================*/

// GUI Widgets includes
#include "vtkMRMLGUIWidgetNode.h"
#include "vtkMRMLGUIWidgetDisplayNode.h"

// MRML includes
#include "vtkMRMLMeasurementArea.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLStorageNode.h"
#include "vtkMRMLTransformNode.h"

// VTK includes
#include <vtkCollection.h>
#include <vtkGeneralTransform.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkTransform.h>
#include <vtkTriangle.h>

// STD includes
#include <sstream>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLGUIWidgetNode);

//----------------------------------------------------------------------------
vtkMRMLGUIWidgetNode::vtkMRMLGUIWidgetNode()
{
}

//----------------------------------------------------------------------------
vtkMRMLGUIWidgetNode::~vtkMRMLGUIWidgetNode() = default;

//----------------------------------------------------------------------------
void vtkMRMLGUIWidgetNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of,nIndent);
  //vtkMRMLWriteXMLBeginMacro(of);
  //vtkMRMLWriteXMLEnumMacro(sizeMode, SizeMode);
  //vtkMRMLWriteXMLVectorMacro(size, Size, double, 2);
  //vtkMRMLWriteXMLFloatMacro(autoSizeScalingFactor, AutoSizeScalingFactor);
  //vtkMRMLWriteXMLMatrix4x4Macro(objectToBaseMatrix, ObjectToBaseMatrix);
  //vtkMRMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLGUIWidgetNode::ReadXMLAttributes(const char** atts)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::ReadXMLAttributes(atts);
  //vtkMRMLReadXMLBeginMacro(atts);
  //vtkMRMLReadXMLEnumMacro(sizeMode, SizeMode);
  //vtkMRMLReadXMLVectorMacro(size, Size, double, 2);
  //vtkMRMLReadXMLFloatMacro(autoSizeScalingFactor, AutoSizeScalingFactor);
  //vtkMRMLReadXMLOwnedMatrix4x4Macro(planeTobaseMatrix, ObjectToBaseMatrix); // Backwards compatible with old name
  //vtkMRMLReadXMLOwnedMatrix4x4Macro(objectToBaseMatrix, ObjectToBaseMatrix);
  //vtkMRMLReadXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLGUIWidgetNode::CopyContent(vtkMRMLNode* anode, bool deepCopy/*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);
  //vtkMRMLCopyBeginMacro(anode);
  //vtkMRMLCopyEnumMacro(SizeMode);
  //vtkMRMLCopyVectorMacro(Size, double, 2);
  //vtkMRMLCopyFloatMacro(AutoSizeScalingFactor);
  //vtkMRMLCopyOwnedMatrix4x4Macro(ObjectToBaseMatrix);
  //vtkMRMLCopyEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLGUIWidgetNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
  //vtkMRMLPrintBeginMacro(os, indent);
  //vtkMRMLPrintEnumMacro(SizeMode);
  //vtkMRMLPrintVectorMacro(Size, double, 2);
  //vtkMRMLPrintFloatMacro(AutoSizeScalingFactor);
  //vtkMRMLPrintMatrix4x4Macro(ObjectToBaseMatrix);
  //vtkMRMLPrintEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLGUIWidgetNode::CreateDefaultDisplayNodes()
{
  if (this->GetDisplayNode() != nullptr && vtkMRMLGUIWidgetDisplayNode::SafeDownCast(this->GetDisplayNode()) != nullptr)
  {
    // Display node already exists
    return;
  }
  if (this->GetScene() == nullptr)
  {
    vtkErrorMacro("vtkMRMLGUIWidgetNode::CreateDefaultDisplayNodes failed: scene is invalid");
    return;
  }
  vtkMRMLGUIWidgetDisplayNode* displayNode = vtkMRMLGUIWidgetDisplayNode::SafeDownCast(
    this->GetScene()->AddNewNodeByClass("vtkMRMLGUIWidgetDisplayNode"));
  if (!displayNode)
  {
    vtkErrorMacro("vtkMRMLGUIWidgetNode::CreateDefaultDisplayNodes failed: scene failed to instantiate a vtkMRMLGUIWidgetDisplayNode node");
    return;
  }
  this->SetAndObserveDisplayNodeID(displayNode->GetID());
}

//----------------------------------------------------------------------------
void vtkMRMLGUIWidgetNode::SetWidget(void* w)
{
  if (this->Widget == w)
  {
    return;
  }

  this->Widget = w;
  this->Modified();
}
