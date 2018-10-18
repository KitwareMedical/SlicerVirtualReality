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

#include "vtkSlicerQWidgetWidget.h"

#include "vtkMRMLGUIWidgetNode.h"

#include "vtkSlicerQWidgetRepresentation.h"
#include "vtkSlicerQWidgetTexture.h"

// MRML includes
#include "vtkMRMLSliceNode.h"

// Qt includes
#include <QMouseEvent>
#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPushButton>
#include <QWidget>

// VTK includes
#include "vtkCallbackCommand.h"
// #include "vtkCommand.h"
#include "vtkEvent.h"
#include "vtkEventData.h"
#include "vtkObjectFactory.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkWidgetCallbackMapper.h"
#include "vtkWidgetEvent.h"
#include "vtkWidgetEventTranslator.h"

vtkStandardNewMacro(vtkSlicerQWidgetWidget);

//------------------------------------------------------------------------------
vtkSlicerQWidgetWidget::vtkSlicerQWidgetWidget()
{
}

//------------------------------------------------------------------------------
vtkSlicerQWidgetWidget::~vtkSlicerQWidgetWidget() = default;

//------------------------------------------------------------------------------
vtkSlicerQWidgetRepresentation* vtkSlicerQWidgetWidget::GetQWidgetRepresentation()
{
  return vtkSlicerQWidgetRepresentation::SafeDownCast(this->WidgetRep);
}

//------------------------------------------------------------------------------
vtkSlicerMarkupsWidget* vtkSlicerQWidgetWidget::CreateInstance()const
{
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkSlicerQWidgetWidget");
  if(ret)
  {
    return static_cast<vtkSlicerQWidgetWidget*>(ret);
  }

  vtkSlicerQWidgetWidget* result = new vtkSlicerQWidgetWidget;
#ifdef VTK_HAS_INITIALIZE_OBJECT_BASE
  result->InitializeObjectBase();
#endif
  return result;
}

//----------------------------------------------------------------------
void vtkSlicerQWidgetWidget::CreateDefaultRepresentation(
  vtkMRMLMarkupsDisplayNode* markupsDisplayNode, vtkMRMLAbstractViewNode* viewNode, vtkRenderer* renderer)
{
  if (vtkMRMLSliceNode::SafeDownCast(viewNode))
  {
    // There is no 2D representation of the GUI widget
    return;
  }

  vtkNew<vtkSlicerQWidgetRepresentation> rep;
  this->SetRenderer(renderer);
  this->SetRepresentation(rep);
  rep->SetMarkupsDisplayNode(markupsDisplayNode);
  rep->SetViewNode(viewNode);

  rep->UpdateFromMRML(nullptr, 0); // full update
}

//------------------------------------------------------------------------------
void vtkSlicerQWidgetWidget::SetRepresentation(vtkMRMLAbstractWidgetRepresentation* rep)
{
  this->Superclass::SetRepresentation(rep);

  vtkSlicerQWidgetRepresentation* qWidgetRep = vtkSlicerQWidgetRepresentation::SafeDownCast(rep);
  if (!qWidgetRep)
  {
    vtkErrorMacro("SetRepresentation: Given representation is not a vtkSlicerQWidgetRepresentation");
  }
}

//------------------------------------------------------------------------------
void vtkSlicerQWidgetWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
