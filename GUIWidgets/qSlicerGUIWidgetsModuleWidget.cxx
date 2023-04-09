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
#include "qSlicerGUIWidgetsModuleWidget.h"
#include "ui_qSlicerGUIWidgetsModuleWidget.h"

#include "vtkMRMLGUIWidgetNode.h"
#include "vtkMRMLGUIWidgetDisplayNode.h"

#include "vtkSlicerQWidgetWidget.h"
#include "vtkSlicerQWidgetRepresentation.h"

#include "vtkSlicerQWidgetTexture.h"

// VirtualReality includes
#include "vtkMRMLVirtualRealityViewNode.h"
#include "vtkSlicerVirtualRealityLogic.h"

// Slicer includes
#include "qSlicerApplication.h"
#include "qSlicerLayoutManager.h"

#include "vtkSlicerApplicationLogic.h"

// qMRMLWidget includes
#include "qMRMLThreeDView.h"
#include "qMRMLThreeDWidget.h"

// Markups Logic includes
#include <vtkSlicerMarkupsLogic.h>

// Virtual Reality includes
#include "qMRMLVirtualRealityHomeWidget.h"
#include "qMRMLVirtualRealityDataModuleWidget.h"
#include "qMRMLVirtualRealitySegmentEditorWidget.h"
#include "qMRMLVirtualRealityTransformWidget.h"

// MRML includes
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLViewNode.h"

#include "vtkMRMLMarkupsDisplayableManager.h"
#include "vtkMRMLCameraDisplayableManager.h"
#include "vtkMRMLMarkupsDisplayableManagerHelper.h"
#include "vtkMRMLVirtualRealityViewDisplayableManagerFactory.h"

// VTK includes
#include "vtkRenderer.h"
#include "vtkMatrix4x4.h"
#include "vtkPlaneSource.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkDataSet.h"
#include "vtkCellLocator.h"

// Qt includes
#include <QDebug>
#include <QRect>
#include <QWidget>
#include <QEvent>
#include <QMouseEvent>
#include <QGraphicsScene>
#include <QApplication>
#include <QGraphicsSceneMouseEvent>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerGUIWidgetsModuleWidgetPrivate: public Ui_qSlicerGUIWidgetsModuleWidget
{
public:
  qSlicerGUIWidgetsModuleWidgetPrivate();
};

//-----------------------------------------------------------------------------
// qSlicerGUIWidgetsModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerGUIWidgetsModuleWidgetPrivate::qSlicerGUIWidgetsModuleWidgetPrivate()
{
}


//-----------------------------------------------------------------------------
// qSlicerGUIWidgetsModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerGUIWidgetsModuleWidget::qSlicerGUIWidgetsModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerGUIWidgetsModuleWidgetPrivate )
{
}

//-----------------------------------------------------------------------------
qSlicerGUIWidgetsModuleWidget::~qSlicerGUIWidgetsModuleWidget()
{
  this->GUIWidgetsMap.clear();
}

//-----------------------------------------------------------------------------
void qSlicerGUIWidgetsModuleWidget::setup()
{
  Q_D(qSlicerGUIWidgetsModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();

  QObject::connect(d->AddHelloWorldGUIWidgetNodeButton, SIGNAL(clicked()), this, SLOT(onAddHelloWorldNodeClicked()));
  QObject::connect(d->UpdateButtonLabelButton, SIGNAL(clicked()), this, SLOT(onUpdateButtonLabelButtonClicked()));

  QObject::connect(d->AddHomeWidgetButton, SIGNAL(clicked()), this, SLOT(onAddHomeWidgetButtonClicked()));
  QObject::connect(d->AddDataModuleWidgetButton, SIGNAL(clicked()), this, SLOT(onAddDataModuleWidgetButtonClicked()));
  QObject::connect(d->AddSegmentEditorWidgetButton, SIGNAL(clicked()), this, SLOT(onAddSegmentEditorWidgetButtonClicked()));
  QObject::connect(d->AddTransformWidgetButton, SIGNAL(clicked()), this, SLOT(onAddTransformWidgetButtonClicked()));

  QObject::connect(d->SetUpInteractionButton, SIGNAL(clicked()), this, SLOT(onSetUpInteractionButtonClicked()));
  QObject::connect(d->StartInteractionButton, SIGNAL(clicked()), this, SLOT(onStartInteractionButtonClicked()));
}

//-----------------------------------------------------------------------------
void qSlicerGUIWidgetsModuleWidget::setWidgetToGUIWidgetMarkupsNode(vtkMRMLGUIWidgetNode* node, QWidget* widget)
{
  if (!node)
  {
    return;
  }

  node->SetWidget((void*)widget);

  this->GUIWidgetsMap[node] = widget;
}

//-----------------------------------------------------------------------------
QWidget* qSlicerGUIWidgetsModuleWidget::onAddHelloWorldNodeClicked()
{
  qSlicerApplication* app = qSlicerApplication::application();
  vtkMRMLGUIWidgetNode* widgetNode = vtkMRMLGUIWidgetNode::SafeDownCast(
    app->mrmlScene()->AddNewNodeByClass("vtkMRMLGUIWidgetNode") );
  widgetNode->SetName("TestButtonWidgetNode");

  QPushButton* newButton = new QPushButton("Hello world!");
  this->setWidgetToGUIWidgetMarkupsNode(widgetNode, newButton);

  return newButton;
}

//-----------------------------------------------------------------------------
void qSlicerGUIWidgetsModuleWidget::onUpdateButtonLabelButtonClicked()
{
  Q_D(qSlicerGUIWidgetsModuleWidget);

  // Get last widget
  QWidget* lastWidget = this->GUIWidgetsMap.last();

  QPushButton* button = qobject_cast<QPushButton*>(lastWidget);
  if (button)
  {
    button->setText(d->NewLabelLineEdit->text());
  }
  else
  {
    qCritical() << Q_FUNC_INFO << ": Widget is not a push button";
  }
}

//-----------------------------------------------------------------------------
void qSlicerGUIWidgetsModuleWidget::onAddHomeWidgetButtonClicked()
{
  Q_D(qSlicerGUIWidgetsModuleWidget);

  qSlicerApplication* app = qSlicerApplication::application();
  vtkMRMLGUIWidgetNode* widgetNode = vtkMRMLGUIWidgetNode::SafeDownCast(
    app->mrmlScene()->AddNewNodeByClass("vtkMRMLGUIWidgetNode") );
  widgetNode->SetName("HomeWidgetNode");

  qMRMLVirtualRealityHomeWidget* widget = new qMRMLVirtualRealityHomeWidget();
  widget->setMRMLScene(app->mrmlScene());
  this->setWidgetToGUIWidgetMarkupsNode(widgetNode, widget);
}

//-----------------------------------------------------------------------------
void qSlicerGUIWidgetsModuleWidget::onAddDataModuleWidgetButtonClicked()
{
  Q_D(qSlicerGUIWidgetsModuleWidget);

  qSlicerApplication* app = qSlicerApplication::application();
  vtkMRMLGUIWidgetNode* widgetNode = vtkMRMLGUIWidgetNode::SafeDownCast(
    app->mrmlScene()->AddNewNodeByClass("vtkMRMLGUIWidgetNode") );
  widgetNode->SetName("DataModuleWidgetNode");

  qMRMLVirtualRealityDataModuleWidget* widget = new qMRMLVirtualRealityDataModuleWidget();
  widget->setMRMLScene(app->mrmlScene());
  this->setWidgetToGUIWidgetMarkupsNode(widgetNode, widget);
}

//-----------------------------------------------------------------------------
void qSlicerGUIWidgetsModuleWidget::onAddSegmentEditorWidgetButtonClicked()
{
  Q_D(qSlicerGUIWidgetsModuleWidget);

  qSlicerApplication* app = qSlicerApplication::application();
  vtkMRMLGUIWidgetNode* widgetNode = vtkMRMLGUIWidgetNode::SafeDownCast(
    app->mrmlScene()->AddNewNodeByClass("vtkMRMLGUIWidgetNode") );
  widgetNode->SetName("SegmentEditorWidgetNode");

  qMRMLVirtualRealitySegmentEditorWidget* widget = new qMRMLVirtualRealitySegmentEditorWidget();
  widget->setMRMLScene(app->mrmlScene());
  this->setWidgetToGUIWidgetMarkupsNode(widgetNode, widget);
}

//-----------------------------------------------------------------------------
void qSlicerGUIWidgetsModuleWidget::onAddTransformWidgetButtonClicked()
{
  Q_D(qSlicerGUIWidgetsModuleWidget);

  qSlicerApplication* app = qSlicerApplication::application();
  vtkMRMLGUIWidgetNode* widgetNode = vtkMRMLGUIWidgetNode::SafeDownCast(
    app->mrmlScene()->AddNewNodeByClass("vtkMRMLGUIWidgetNode") );
  widgetNode->SetName("TransformWidgetNode");

  vtkSlicerVirtualRealityLogic* vrLogic =
    vtkSlicerVirtualRealityLogic::SafeDownCast(app->applicationLogic()->GetModuleLogic("VirtualReality"));
  if (!vrLogic)
  {
    qCritical() << Q_FUNC_INFO << " : invalid VR logic";
    return;
  }
 
  qMRMLVirtualRealityTransformWidget* widget = new qMRMLVirtualRealityTransformWidget(vrLogic->GetVirtualRealityViewNode());
  widget->setMRMLScene(app->mrmlScene());
  this->setWidgetToGUIWidgetMarkupsNode(widgetNode, widget);
}

//-----------------------------------------------------------------------------
void qSlicerGUIWidgetsModuleWidget::onSetUpInteractionButtonClicked()
{
  std::cout << "----- onSetUpInteractionButtonClicked ----- \n";
}
    
//-----------------------------------------------------------------------------
void qSlicerGUIWidgetsModuleWidget::onStartInteractionButtonClicked()
{
  std::cout << "\n\nqSlicerGUIWidgetsModuleWidget::onStartInteractionButtonClicked() \n";

  // Pointer transform
  qSlicerApplication* app = qSlicerApplication::application();
  vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast(app->mrmlScene()->GetFirstNodeByName("PointerTransform"));
  if (!transformNode)
    {
    std::cout << "ERROR: Pointer transform was not found in scene... \n";
    return;
    }

  // Define maximum distance for interaction
  double maxDistanceForInteraction = 2000; // mm

  // Line points
  double pointA_h[4] = { 0.0, 0.0, 0.0, 1.0 };
  double pointB_h[4] = { 0.0, 0.0, -maxDistanceForInteraction, 1.0 };

  // Get transformed line points
  double pointA_transf_h[4] = { 0.0, 0.0, 0.0, 1.0 };
  double pointB_transf_h[4] = { 0.0, 0.0, 0.0, 1.0 };
  vtkNew<vtkMatrix4x4> matrixTransformToWorld;
  transformNode->GetMatrixTransformToWorld(matrixTransformToWorld);
  matrixTransformToWorld->MultiplyPoint(pointA_h, pointA_transf_h);
  matrixTransformToWorld->MultiplyPoint(pointB_h, pointB_transf_h);
  double pointA_transf[3] = { pointA_transf_h[0], pointA_transf_h[1], pointA_transf_h[2] };
  double pointB_transf[3] = { pointB_transf_h[0], pointB_transf_h[1], pointB_transf_h[2] };

  // Get GUI widget
  vtkMRMLGUIWidgetNode* widgetNode = vtkMRMLGUIWidgetNode::SafeDownCast(app->mrmlScene()->GetFirstNodeByName("HomeWidgetNode"));

  // Get displayable manager
  qSlicerLayoutManager* layoutManager = qSlicerApplication::application()->layoutManager();
  if (!layoutManager)
    {
    // application is closing
    return;
    }
  qMRMLThreeDWidget* threeDWidget = layoutManager->threeDWidget(0);
  vtkMRMLMarkupsDisplayableManager* markupsDisplayableManager = vtkMRMLMarkupsDisplayableManager::SafeDownCast(threeDWidget->threeDView()->displayableManagerByClassName("vtkMRMLMarkupsDisplayableManager"));
  
  // Get widget representation from displayabale manager
  vtkMRMLMarkupsDisplayableManagerHelper* helper = markupsDisplayableManager->GetHelper();
  vtkSlicerQWidgetWidget* widget = vtkSlicerQWidgetWidget::SafeDownCast(helper->GetWidget(widgetNode->GetMarkupsDisplayNode()));
  vtkSlicerQWidgetRepresentation* rep = vtkSlicerQWidgetRepresentation::SafeDownCast(widget->GetRepresentation());

  // Get plane source
  vtkPlaneSource* planeSource = vtkPlaneSource::SafeDownCast(rep->GetPlaneSource());

  // Get plane normal
  double* planeNormal = planeSource->GetNormal();
  std::cout << "Plane normal: [" << planeNormal[0] << ", " << planeNormal[1] << ", " << planeNormal[2] << "] \n";

  // Get plane reference points
  double* planePointSW = planeSource->GetOrigin(); // bottom left corner
  double* planePointSE = planeSource->GetPoint1(); // bottom right corner
  double* planePointNW = planeSource->GetPoint2(); // top left corner
  double translationWtoE[3] = {0.0, 0.0, 0.0};
  vtkMath::Subtract(planePointSE, planePointSW, translationWtoE);
  double planePointNE[3] = { 0.0, 0.0, 0.0 };
  vtkMath::Add(planePointNW, translationWtoE, planePointNE);
  std::cout << "Plane point NW: [" << planePointNW[0] << ", " << planePointNW[1] << ", " << planePointNW[2] << "] \n";
  std::cout << "Plane point NE: [" << planePointNE[0] << ", " << planePointNE[1] << ", " << planePointNE[2] << "] \n";
  std::cout << "Plane point SW: [" << planePointSW[0] << ", " << planePointSW[1] << ", " << planePointSW[2] << "] \n";
  std::cout << "Plane point SE: [" << planePointSE[0] << ", " << planePointSE[1] << ", " << planePointSE[2] << "] \n";

  // Compute intersection point
  vtkNew<vtkCellLocator> cellLocator;
  cellLocator->SetDataSet(planeSource->GetOutput());
  cellLocator->BuildLocator(); 
  double tolerance = 0.001;
  double t = 0.0;
  double pcoords[3] = { 0.0 };
  int subId = 0;
  vtkIdType cellId = 0;
  vtkNew <vtkGenericCell> cell;
  double intersectionPoint[3]= { 0.0, 0.0, 0.0 };
  int foundIntersection = cellLocator->IntersectWithLine(pointA_transf, pointB_transf, tolerance, t, intersectionPoint, pcoords, subId, cellId, cell);
  if (foundIntersection)
    {
    std::cout << "Intersection point: [" << intersectionPoint[0] << ", " << intersectionPoint[1] << ", " << intersectionPoint[2] << "] \n";
    }
  else
    {
    std::cout << "No intersection was found... \n";
    return;
    }

  // Get plane dimensions
  vtkSlicerQWidgetTexture* texture = rep->GetQWidgetTexture();
  QWidget* qWidget = texture->GetWidget();
  if (!qWidget)
    {
    return;
    }
  QRect rect = qWidget->geometry();
  if (rect.width() < 2 || rect.height() < 2)
    {
    return;
    }
  std::cout << "Widget dimensions: width = " << rect.width() << " and height = " << rect.height() << "\n";  
  double spacingMmPerPixel = rep->GetSpacingMmPerPixel();
  double bounds[6] = {
    -(double)(rect.width() / 2) * spacingMmPerPixel, (double)rect.width() / 2 * spacingMmPerPixel,
    -0.5, 0.5,
    -(double)(rect.height() / 2) * spacingMmPerPixel, (double)rect.height() / 2 * spacingMmPerPixel
    };
  std::cout << "Widget bounds: [ " << bounds[0] << ", " << bounds[1] << ", " << bounds[2] << ", " << bounds[3] << ", " << bounds[4] << ", " << bounds[5] << "\n";

  // Compute pixel position
  double intersectionPointVector[3] = { intersectionPoint[0] - planePointNW[0], intersectionPoint[1] - planePointNW[1], intersectionPoint[2] - planePointNW[2] };
  double xPlaneAxis[3] = { planePointNE[0] - planePointNW[0], planePointNE[1] - planePointNW[1], planePointNE[2] - planePointNW[2] };
  double yPlaneAxis[3] = { planePointSW[0] - planePointNW[0], planePointSW[1] - planePointNW[1], planePointSW[2] - planePointNW[2] };
  vtkMath::MultiplyScalar(xPlaneAxis, vtkMath::Dot(intersectionPointVector, xPlaneAxis) / vtkMath::Dot(xPlaneAxis, xPlaneAxis));
  vtkMath::MultiplyScalar(yPlaneAxis, vtkMath::Dot(intersectionPointVector, yPlaneAxis) / vtkMath::Dot(yPlaneAxis, yPlaneAxis));
  double xIntersectionPoint[3] = { 0.0, 0.0, 0.0 };
  double yIntersectionPoint[3] = { 0.0, 0.0, 0.0 };
  vtkMath::Add(planePointNW, xPlaneAxis, xIntersectionPoint);
  vtkMath::Add(planePointNW, yPlaneAxis, yIntersectionPoint);
  vtkMath::Subtract(xIntersectionPoint, planePointNW, xIntersectionPoint); // subtract plane origin
  vtkMath::Subtract(yIntersectionPoint, planePointNW, yIntersectionPoint); // subtract plane origin
  double xPositionMm = vtkMath::Norm(xIntersectionPoint);
  double yPositionMm = vtkMath::Norm(yIntersectionPoint);
  std::cout << "Pointer intersection position (mm): [ " << xPositionMm << ", " << yPositionMm << "] \n";
  int xPositionPixels = xPositionMm / spacingMmPerPixel;
  int yPositionPixels = yPositionMm / spacingMmPerPixel;
  std::cout << "Pointer intersection position (pixels): [ " << xPositionPixels << ", " << yPositionPixels << "] \n";

  // Send press event
  QGraphicsSceneMouseEvent pressEvent(QEvent::GraphicsSceneMousePress);
  pressEvent.setScenePos(QPointF(xPositionPixels, yPositionPixels));
  pressEvent.setButton(Qt::LeftButton);
  pressEvent.setButtons(Qt::LeftButton); 
  QApplication::sendEvent(texture->GetScene(), &pressEvent);

  // Send release event  
  QGraphicsSceneMouseEvent releaseEvent(QEvent::GraphicsSceneMouseRelease);
  releaseEvent.setScenePos(QPointF(xPositionPixels, yPositionPixels));
  releaseEvent.setButton(Qt::LeftButton);
  releaseEvent.setButtons(Qt::LeftButton);
  QApplication::sendEvent(texture->GetScene(), &releaseEvent);
}
