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

// GUIWidgets MRML includes
#include <vtkMRMLGUIWidgetNode.h>

// GUIWidgets Logic includes
#include <vtkSlicerGUIWidgetsLogic.h>

// GUIWidgets VTKWidgets includes
#include <vtkSlicerQWidgetWidget.h>

// GUIWidgets includes
#include "qSlicerGUIWidgetsModule.h"
#include "qSlicerGUIWidgetsModuleWidget.h"

// Markups Logic includes
#include <vtkSlicerMarkupsLogic.h>

// Markups Widgets includes
#include "qMRMLMarkupsOptionsWidgetsFactory.h"

// Qt includes
#include <QDebug>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerGUIWidgetsModulePrivate
{
public:
  qSlicerGUIWidgetsModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerGUIWidgetsModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerGUIWidgetsModulePrivate::qSlicerGUIWidgetsModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerGUIWidgetsModule methods

//-----------------------------------------------------------------------------
qSlicerGUIWidgetsModule::qSlicerGUIWidgetsModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerGUIWidgetsModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerGUIWidgetsModule::~qSlicerGUIWidgetsModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerGUIWidgetsModule::helpText() const
{
  return "This is a loadable module that can be bundled in an extension";
}

//-----------------------------------------------------------------------------
QString qSlicerGUIWidgetsModule::acknowledgementText() const
{
  return "This work was partially funded by the grant 'ICEX Espana Exportacion e Inversiones' under\
 the program 'Inversiones de Empresas Extranjeras en Actividades de I+D\
 (Fondo Tecnologico)- Convocatoria 2021'";
}

//-----------------------------------------------------------------------------
QStringList qSlicerGUIWidgetsModule::contributors() const
{
  QStringList moduleContributors;
  moduleContributors << QString("Csaba Pinter (Ebatinca)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerGUIWidgetsModule::icon() const
{
  return QIcon(":/Icons/GUIWidgets.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerGUIWidgetsModule::categories() const
{
  return QStringList() << "Virtual Reality";
}

//-----------------------------------------------------------------------------
QStringList qSlicerGUIWidgetsModule::dependencies() const
{
  return QStringList() << "Markups";
}

//-----------------------------------------------------------------------------
void qSlicerGUIWidgetsModule::setup()
{
  this->Superclass::setup();

  // Register displayable managers (same displayable manager handles both slice and 3D views)
  //TODO: We do not register it because since the MRML node is a subclass of the plane markup, the markups DM takes over
  //vtkMRMLThreeDViewDisplayableManagerFactory::GetInstance()->RegisterDisplayableManager("vtkMRMLGUIWidgetsDisplayableManager");

  // Register markups
  vtkSlicerApplicationLogic* appLogic = this->appLogic();
  if (!appLogic)
  {
    qCritical() << Q_FUNC_INFO << " : invalid application logic.";
    return;
  }
  vtkSlicerMarkupsLogic* markupsLogic = vtkSlicerMarkupsLogic::SafeDownCast(appLogic->GetModuleLogic("Markups"));
  if (!markupsLogic)
  {
    qCritical() << Q_FUNC_INFO << " : invalid markups logic.";
    return;
  }
  vtkNew<vtkMRMLGUIWidgetNode> guiWidgetNode;
  vtkNew<vtkSlicerQWidgetWidget> vtkQWidgetWidget;
  markupsLogic->RegisterMarkupsNode(guiWidgetNode, vtkQWidgetWidget);

  // Create and configure the additional widgets
  //auto optionsWidgetFactory = qSlicerMarkupsAdditionalOptionsWidgetsFactory::instance();
  //optionsWidgetFactory->registerAdditionalOptionsWidget(new qSlicerMarkupsGUIWidget()); 
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerGUIWidgetsModule::createWidgetRepresentation()
{
  return new qSlicerGUIWidgetsModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerGUIWidgetsModule::createLogic()
{
  return vtkSlicerGUIWidgetsLogic::New();
}
