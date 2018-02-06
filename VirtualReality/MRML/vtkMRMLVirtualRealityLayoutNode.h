/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, and Cancer
  Care Ontario.

==============================================================================*/

#ifndef __vtkMRMLVirtualRealityLayoutNode_h
#define __vtkMRMLVirtualRealityLayoutNode_h

// MRML includes
#include "vtkMRMLAbstractLayoutNode.h"

#include "vtkSlicerVirtualRealityModuleMRMLExport.h"

/// \brief Node that describes the virtual reality layout of the application.
class VTK_SLICER_VIRTUALREALITY_MODULE_MRML_EXPORT vtkMRMLVirtualRealityLayoutNode : public vtkMRMLAbstractLayoutNode
{
public:
  static vtkMRMLVirtualRealityLayoutNode *New();
  vtkTypeMacro(vtkMRMLVirtualRealityLayoutNode,vtkMRMLAbstractLayoutNode);
  virtual vtkMRMLNode* CreateNodeInstance() VTK_OVERRIDE;
  void PrintSelf(ostream& os, vtkIndent indent) VTK_OVERRIDE;

  //--------------------------------------------------------------------------
  /// MRML methods
  //--------------------------------------------------------------------------

  /// Set node attributes
  virtual void ReadXMLAttributes( const char** atts) VTK_OVERRIDE;

  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent) VTK_OVERRIDE;

  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node) VTK_OVERRIDE;

  /// Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() VTK_OVERRIDE {return "VirtualRealityLayout";}

  /// Adds a layout description with integer identifier
  /// "layout". Returns false without making any modifications if the
  /// integer identifier "layout" has already been added.
  bool AddLayoutDescription(int layout, const char* layoutDescription);

  /// Modifies a layout description for integer identifier
  /// "layout". Returns false without making any modifications if the
  /// integer identifier "layout" has NOT already been added.
  bool SetLayoutDescription(int layout, const char* layoutDescription);

  /// Query whether a layout exists with a specified integer identifier
  bool IsLayoutDescription(int layout);

  /// Get the layout description associated with a specified integer
  /// identifier. The empty string is returned if the layout does not exist.
  std::string GetLayoutDescription(int layout);

  // Get the layout description currently displayed. Used
  // internally. This is XML description corresponding to the ivar
  // ViewArrangement which is the integer identifier for the
  // layout. ViewArrangement and CurrentViewDescription may not
  // correspond while a view is being switched.
  vtkGetStringMacro(CurrentLayoutDescription);

  //TODO: Add features to handle heads-up display. It is unlikely that the VirtualReality layouts can use
  // the same API as the Slicer layouts, because they are not a Qt layouts, just stationary
  // 2D panels in the 3D scene.
  // A new displayable manager will be needed to handle these HUD views

protected:
  void UpdateCurrentLayoutDescription();
  void SetAndParseCurrentLayoutDescription(const char* description);
  vtkSetStringMacro(CurrentLayoutDescription);

protected:
  vtkMRMLVirtualRealityLayoutNode();
  ~vtkMRMLVirtualRealityLayoutNode();
  vtkMRMLVirtualRealityLayoutNode(const vtkMRMLVirtualRealityLayoutNode&);
  void operator=(const vtkMRMLVirtualRealityLayoutNode&);

  //TODO: Are layout descriptions needed like this? If so, sink this into abstract layout
  std::map<int, std::string> Layouts;
  char*                      CurrentLayoutDescription;
};

#endif
