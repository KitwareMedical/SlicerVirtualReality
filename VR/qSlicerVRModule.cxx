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

// VR Logic includes
#include <vtkSlicerVRLogic.h>

// VR includes
#include "qSlicerVRModule.h"
#include "qSlicerVRModuleWidget.h"

//-----------------------------------------------------------------------------
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QtPlugin>
Q_EXPORT_PLUGIN2(qSlicerVRModule, qSlicerVRModule);
#endif

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerVRModulePrivate
{
public:
  qSlicerVRModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerVRModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerVRModulePrivate::qSlicerVRModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerVRModule methods

//-----------------------------------------------------------------------------
qSlicerVRModule::qSlicerVRModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerVRModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerVRModule::~qSlicerVRModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerVRModule::helpText() const
{
  return "This is a loadable module that can be bundled in an extension";
}

//-----------------------------------------------------------------------------
QString qSlicerVRModule::acknowledgementText() const
{
  return "This work was partially funded by NIH grant NXNNXXNNNNNN-NNXN";
}

//-----------------------------------------------------------------------------
QStringList qSlicerVRModule::contributors() const
{
  QStringList moduleContributors;
  moduleContributors << QString("John Doe (AnyWare Corp.)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerVRModule::icon() const
{
  return QIcon(":/Icons/VR.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerVRModule::categories() const
{
  return QStringList() << "Virtual Reality";
}

//-----------------------------------------------------------------------------
QStringList qSlicerVRModule::dependencies() const
{
  return QStringList();
}

//-----------------------------------------------------------------------------
void qSlicerVRModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerVRModule::createWidgetRepresentation()
{
  return new qSlicerVRModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerVRModule::createLogic()
{
  return vtkSlicerVRLogic::New();
}
