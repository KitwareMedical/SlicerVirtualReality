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

==============================================================================*/

// Qt includes
#include <QtPlugin>

// VirtualReality Logic includes
#include <vtkSlicerVirtualRealityLogic.h>

// VirtualReality includes
#include "qSlicerVirtualRealityModule.h"
#include "qSlicerVirtualRealityModuleWidget.h"

//-----------------------------------------------------------------------------
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QtPlugin>
Q_EXPORT_PLUGIN2(qSlicerVirtualRealityModule, qSlicerVirtualRealityModule);
#endif

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerVirtualRealityModulePrivate
{
public:
  qSlicerVirtualRealityModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerVirtualRealityModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerVirtualRealityModulePrivate::qSlicerVirtualRealityModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerVirtualRealityModule methods

//-----------------------------------------------------------------------------
qSlicerVirtualRealityModule::qSlicerVirtualRealityModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerVirtualRealityModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerVirtualRealityModule::~qSlicerVirtualRealityModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerVirtualRealityModule::helpText() const
{
  return "This is a loadable module that can be bundled in an extension";
}

//-----------------------------------------------------------------------------
QString qSlicerVirtualRealityModule::acknowledgementText() const
{
  return "This work was partially funded by NIH grant NXNNXXNNNNNN-NNXN";
}

//-----------------------------------------------------------------------------
QStringList qSlicerVirtualRealityModule::contributors() const
{
  QStringList moduleContributors;
  moduleContributors << QString("John Doe (AnyWare Corp.)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerVirtualRealityModule::icon() const
{
  return QIcon(":/Icons/VirtualReality.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerVirtualRealityModule::categories() const
{
  return QStringList() << "Virtual Reality";
}

//-----------------------------------------------------------------------------
QStringList qSlicerVirtualRealityModule::dependencies() const
{
  return QStringList();
}

//-----------------------------------------------------------------------------
void qSlicerVirtualRealityModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerVirtualRealityModule::createWidgetRepresentation()
{
  return new qSlicerVirtualRealityModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerVirtualRealityModule::createLogic()
{
  return vtkSlicerVirtualRealityLogic::New();
}
