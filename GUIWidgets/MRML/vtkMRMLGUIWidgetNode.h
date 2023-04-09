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

#ifndef __vtkMRMLGUIWidgetNode_h
#define __vtkMRMLGUIWidgetNode_h

#include "vtkSlicerGUIWidgetsModuleMRMLExport.h"

// Markups includes
#include "vtkMRMLMarkupsPlaneNode.h"

/// \brief MRML node to represent a planar GUI widget displayable in the 3D view
class  VTK_SLICER_GUIWIDGETS_MODULE_MRML_EXPORT vtkMRMLGUIWidgetNode : public vtkMRMLMarkupsPlaneNode
{
public:
  static vtkMRMLGUIWidgetNode *New();
  vtkTypeMacro(vtkMRMLGUIWidgetNode, vtkMRMLMarkupsPlaneNode);
  /// Print out the node information to the output stream
  void PrintSelf(ostream& os, vtkIndent indent) override;

  //TODO:
  //const char* GetIcon() override {return ":/Icons/MarkupsPlane.png";}
  //const char* GetAddIcon() override {return ":/Icons/MarkupsPlaneMouseModePlace.png";}
  //const char* GetPlaceAddIcon() override {return ":/Icons/MarkupsPlaneMouseModePlaceAdd.png";}

  vtkMRMLNode* CreateNodeInstance() override;
  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "GUIWidget";}

  /// Get markup name
  const char* GetMarkupType() override {return "GUIWidget";};

  /// Get markup short name
  const char* GetDefaultNodeNamePrefix() override {return "W";};

  /// Read node attributes from XML file
  void ReadXMLAttributes( const char** atts) override;

  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkMRMLNode::CopyContent
  vtkMRMLCopyContentMacro(vtkMRMLGUIWidgetNode);

  /// Create default storage node or nullptr if does not have one
  void CreateDefaultDisplayNodes() override;

public:
  /// Set the handle (must contain a QWidget) that will receive the events.
  void SetWidget(void* w);
  /// Get the QWidget handle
  void* GetWidget() { return this->Widget; };

protected:
  /// Handle for the widget to be rendered in the GUI widget markup
  void* Widget{nullptr};

protected:
  vtkMRMLGUIWidgetNode();
  ~vtkMRMLGUIWidgetNode() override;
  vtkMRMLGUIWidgetNode(const vtkMRMLGUIWidgetNode&);
  void operator=(const vtkMRMLGUIWidgetNode&);
};

#endif
