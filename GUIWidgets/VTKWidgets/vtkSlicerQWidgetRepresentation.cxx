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

#include "vtkSlicerQWidgetRepresentation.h"

#include "vtkSlicerQWidgetTexture.h"

// GUI Widget includes
#include "vtkMRMLGUIWidgetNode.h"
#include "vtkMRMLGUIWidgetDisplayNode.h"

// VTK includes
#include <vtkActor.h>
#include <vtkCallbackCommand.h>
#include <vtkEventData.h>
#include <vtkObjectFactory.h>
#include <vtkOpenGLRenderWindow.h>
#include <vtkOpenGLTexture.h>
#include <vtkPlaneSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>

// Qt includes
#include <QRect>
#include <QWidget>

vtkStandardNewMacro(vtkSlicerQWidgetRepresentation);

//------------------------------------------------------------------------------
vtkSlicerQWidgetRepresentation::vtkSlicerQWidgetRepresentation()
{
  this->PlaneSource = vtkPlaneSource::New();
  this->PlaneSource->SetOutputPointsPrecision(vtkAlgorithm::DOUBLE_PRECISION);

  this->PlaneMapper = vtkPolyDataMapper::New();
  this->PlaneMapper->SetInputConnection(this->PlaneSource->GetOutputPort());

  this->TextureCallbackCommand = vtkCallbackCommand::New();
  this->TextureCallbackCommand->SetClientData( reinterpret_cast<void *>(this) );
  this->TextureCallbackCommand->SetCallback( vtkSlicerQWidgetRepresentation::OnTextureModified ); 

  this->QWidgetTexture = vtkSlicerQWidgetTexture::New();
  this->QWidgetTexture->AddObserver(vtkCommand::ModifiedEvent, this->TextureCallbackCommand);

  this->PlaneActor = vtkActor::New();
  this->PlaneActor->SetMapper(this->PlaneMapper);
  this->PlaneActor->SetTexture(this->QWidgetTexture);

  this->PlaneActor->GetProperty()->SetAmbient(1.0);
  this->PlaneActor->GetProperty()->SetDiffuse(0.0);

  // Define the point coordinates
  double bounds[6] = {-80, 80, -0.5, 0.5, -50, 50 }; // Width, Depth, Height

  // Initial creation of the widget, serves to initialize it
  this->PlaceWidget(bounds);
}

//------------------------------------------------------------------------------
vtkSlicerQWidgetRepresentation::~vtkSlicerQWidgetRepresentation()
{
  if (this->PlaneSource)
  {
    this->PlaneSource->Delete();
    this->PlaneSource = nullptr;
  }
  if (this->PlaneMapper)
  {
    this->PlaneMapper->Delete();
    this->PlaneMapper = nullptr;
  }
  if (this->PlaneActor)
  {
    this->PlaneActor->Delete();
    this->PlaneActor = nullptr;
  }

  if (this->QWidgetTexture)
  {
    this->QWidgetTexture->RemoveObserver(this->TextureCallbackCommand);
    this->QWidgetTexture->Delete();
    this->QWidgetTexture = nullptr;
  }
  if (this->TextureCallbackCommand)
  {
    this->TextureCallbackCommand->SetClientData(nullptr);
    this->TextureCallbackCommand->Delete();
    this->TextureCallbackCommand = nullptr;
  }
}

//------------------------------------------------------------------------------
void vtkSlicerQWidgetRepresentation::SetWidget(QWidget* w)
{
  // just pass down to the QWidgetTexture
  this->QWidgetTexture->SetWidget(w);
  this->Modified();
}

//------------------------------------------------------------------------------
double* vtkSlicerQWidgetRepresentation::GetBounds()
{
  //this->BuildRepresentation();
  return this->PlaneActor->GetBounds();
}

//------------------------------------------------------------------------------
void vtkSlicerQWidgetRepresentation::GetActors(vtkPropCollection* pc)
{
  this->Superclass::GetActors(pc);
  this->PlaneActor->GetActors(pc);
}

//------------------------------------------------------------------------------
void vtkSlicerQWidgetRepresentation::ReleaseGraphicsResources(vtkWindow* w)
{
  this->Superclass::ReleaseGraphicsResources(w);
  this->PlaneActor->ReleaseGraphicsResources(w);
  this->PlaneMapper->ReleaseGraphicsResources(w);
  this->QWidgetTexture->ReleaseGraphicsResources(w);
}

//------------------------------------------------------------------------------
int vtkSlicerQWidgetRepresentation::RenderOpaqueGeometry(vtkViewport* v)
{
  int count = this->Superclass::RenderOpaqueGeometry(v);

  if (this->PlaneActor->GetVisibility())
  {
    this->PlaneActor->SetPropertyKeys(this->GetPropertyKeys());

    count += this->PlaneActor->RenderOpaqueGeometry(v);
  }

  return count;
}

//------------------------------------------------------------------------------
int vtkSlicerQWidgetRepresentation::RenderTranslucentPolygonalGeometry(vtkViewport* viewport)
{
  int count=0;
  count = this->Superclass::RenderTranslucentPolygonalGeometry(viewport);
  if (this->PlaneActor->GetVisibility())
  {
    // The internal actor needs to share property keys.
    // This ensures the mapper state is consistent and allows depth peeling to work as expected.
    this->PlaneActor->SetPropertyKeys(this->GetPropertyKeys());

    count += this->PlaneActor->RenderTranslucentPolygonalGeometry(viewport);
  }
  return count;
}

//------------------------------------------------------------------------------
vtkTypeBool vtkSlicerQWidgetRepresentation::HasTranslucentPolygonalGeometry()
{
  if (this->Superclass::HasTranslucentPolygonalGeometry())
  {
    return true;
  }
  if (this->PlaneActor->GetVisibility() && this->PlaneActor->HasTranslucentPolygonalGeometry())
  {
    return true;
  }
  return false;
}

//------------------------------------------------------------------------------
void vtkSlicerQWidgetRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  // this->InteractionState is printed in superclass
  // this is commented to avoid PrintSelf errors
}

//------------------------------------------------------------------------------
void vtkSlicerQWidgetRepresentation::PlaceWidget(double bds[6])
{
  this->PlaneSource->SetOrigin(bds[0], bds[2], bds[4]);
  this->PlaneSource->SetPoint1(bds[1], bds[2], bds[4]);
  this->PlaneSource->SetPoint2(bds[0], bds[2], bds[5]);
}

//----------------------------------------------------------------------
void vtkSlicerQWidgetRepresentation::UpdateFromMRML(vtkMRMLNode* caller, unsigned long event, void *callData /*=nullptr*/)
{
  Superclass::UpdateFromMRML(caller, event, callData);

  this->NeedToRenderOn();

  vtkMRMLGUIWidgetNode* guiWidgetNode = vtkMRMLGUIWidgetNode::SafeDownCast(this->GetMarkupsNode());
  vtkMRMLGUIWidgetDisplayNode* displayNode = vtkMRMLGUIWidgetDisplayNode::SafeDownCast(this->GetMarkupsDisplayNode());
  if (!guiWidgetNode || !this->IsDisplayable() || !displayNode)
  {
    this->VisibilityOff();
    return;
  }

  if (guiWidgetNode->GetWidget() != this->GetQWidgetTexture()->GetWidget())
  {
    if (guiWidgetNode->GetWidget() == nullptr)
    {
      this->QWidgetTexture->SetWidget(nullptr);
    }
    else
    {
      QWidget* widget = reinterpret_cast<QWidget*>(guiWidgetNode->GetWidget());
      if (widget != this->QWidgetTexture->GetWidget())
      {
        this->QWidgetTexture->SetWidget(widget);
      }
    }
  }

  if (!this->QWidgetTexture->GetWidget() || !this->ViewNode)
  {
    this->VisibilityOff();
    this->PlaneActor->SetVisibility(false);
    return;
  }

  this->VisibilityOn();
  this->PlaneActor->SetVisibility(true);
}

//---------------------------------------------------------------------------
void vtkSlicerQWidgetRepresentation::OnTextureModified(
  vtkObject* vtkNotUsed(caller), unsigned long vtkNotUsed(eid), void* clientData, void* vtkNotUsed(callData))
{
  vtkSlicerQWidgetRepresentation* self = reinterpret_cast<vtkSlicerQWidgetRepresentation*>(clientData);

  // Redefine widget plane
  QWidget* widget = self->QWidgetTexture->GetWidget();
  if (!widget)
  {
    return;
  }

  QRect rect = widget->geometry();
  if (rect.width() < 2 || rect.height() < 2)
  {
    return;
  }
  double bounds[6] = {
    -(double)(rect.width()/2)*self->SpacingMmPerPixel, (double)rect.width()/2*self->SpacingMmPerPixel,
    -0.5, 0.5,
    -(double)(rect.height()/2)*self->SpacingMmPerPixel, (double)rect.height()/2*self->SpacingMmPerPixel
  };
  self->PlaceWidget(bounds);

  // Trigger rendering in view
  self->GetViewNode()->Modified();
}
