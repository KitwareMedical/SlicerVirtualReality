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

// .NAME vtkMRMLGUIWidgetDisplayNode - MRML node to represent display properties for GUI widget
// .SECTION Description
// Adjusts default display parameters for ROI such as fill opacity.
//

#ifndef __vtkMRMLGUIWidgetDisplayNode_h
#define __vtkMRMLGUIWidgetDisplayNode_h

#include "vtkSlicerGUIWidgetsModuleMRMLExport.h"
#include "vtkMRMLMarkupsDisplayNode.h"

class VTK_SLICER_GUIWIDGETS_MODULE_MRML_EXPORT vtkMRMLGUIWidgetDisplayNode : public vtkMRMLMarkupsDisplayNode
{
public:
  static vtkMRMLGUIWidgetDisplayNode *New();
  vtkTypeMacro(vtkMRMLGUIWidgetDisplayNode, vtkMRMLMarkupsDisplayNode);

  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  vtkMRMLNode* CreateNodeInstance (  ) override;

  // Get node XML tag name (like Volume, Markups)
  const char* GetNodeTagName() override {return "GUIWidgetDisplay";};

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkMRMLNode::CopyContent
  vtkMRMLCopyContentDefaultMacro(vtkMRMLGUIWidgetDisplayNode);

protected:
  vtkMRMLGUIWidgetDisplayNode();
  ~vtkMRMLGUIWidgetDisplayNode() override;
  vtkMRMLGUIWidgetDisplayNode( const vtkMRMLGUIWidgetDisplayNode& );
  void operator= ( const vtkMRMLGUIWidgetDisplayNode& );
};
#endif
