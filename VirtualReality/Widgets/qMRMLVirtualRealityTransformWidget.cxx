/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Brigham and Women's Hospital

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// VR MRML includes
#include "vtkMRMLVirtualRealityViewNode.h"

// VR Widgts includes
#include "qMRMLVirtualRealityTransformWidget.h"
#include "ui_qMRMLVirtualRealityTransformWidget.h"

// Qt includes
#include <QDebug>
#include <QIcon>
#include <QPushButton>

// MRML includes
#include <vtkMRMLLinearTransformNode.h>

// VTK includes
#include <vtkEventData.h>


namespace
{
  // Copied from https://github.com/ValveSoftware/openvr/blob/v2.0.10/headers/openvr.h
  enum ETrackingResult
  {
    TrackingResult_Uninitialized = 1,

    TrackingResult_Calibrating_InProgress	= 100,
    TrackingResult_Calibrating_OutOfRange	= 101,

    TrackingResult_Running_OK = 200,
    TrackingResult_Running_OutOfRange = 201,

    TrackingResult_Fallback_RotationOnly = 300,
  };

  ::ETrackingResult StringToPoseStatus(const char* cpose)
  {
    if (cpose == nullptr)
    {
      return ::TrackingResult_Uninitialized;
    }
    std::string pose(cpose);

    if (pose.compare("CalibratingInProgress") == 0)
    {
      return ::TrackingResult_Calibrating_InProgress;
    }
    else if (pose.compare("CalibratingOutOfRange") == 0)
    {
      return ::TrackingResult_Calibrating_OutOfRange;
    }
    else if (pose.compare("RunningOk") == 0)
    {
      return ::TrackingResult_Running_OK;
    }
    else if (pose.compare("RunningOutOfRange") == 0)
    {
      return ::TrackingResult_Running_OutOfRange;
    }
    else
    {
      return ::TrackingResult_Uninitialized;
    }
  }
}
//-----------------------------------------------------------------------------
class qMRMLVirtualRealityTransformWidgetPrivate
  : public Ui_qMRMLVirtualRealityTransformWidget
{
  Q_DECLARE_PUBLIC(qMRMLVirtualRealityTransformWidget);
protected:
  qMRMLVirtualRealityTransformWidget* const q_ptr;
public:
  qMRMLVirtualRealityTransformWidgetPrivate(qMRMLVirtualRealityTransformWidget& object);
  void init();

  vtkWeakPointer<vtkMRMLVirtualRealityViewNode> VRViewNode;
  vtkWeakPointer<vtkMRMLLinearTransformNode>    TransformNode;
  vtkEventDataDevice                            TransformType;
  ::ETrackingResult                             PreviousStatus;
};

//-----------------------------------------------------------------------------
// qMRMLVirtualRealityTransformWidgetPrivate methods

//-----------------------------------------------------------------------------
qMRMLVirtualRealityTransformWidgetPrivate::qMRMLVirtualRealityTransformWidgetPrivate(qMRMLVirtualRealityTransformWidget& object)
  : q_ptr(&object)
  , VRViewNode(nullptr)
  , TransformNode(nullptr)
  , TransformType(vtkEventDataDevice::Unknown)
  , PreviousStatus(::TrackingResult_Uninitialized)
{

}

//-----------------------------------------------------------------------------
void qMRMLVirtualRealityTransformWidgetPrivate::init()
{
  Q_Q(qMRMLVirtualRealityTransformWidget);
  this->setupUi(q);

  QObject::connect(this->pushButton_Transform, &QPushButton::clicked, q, &qMRMLVirtualRealityTransformWidget::onButtonClicked);

  q->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
// qMRMLVirtualRealityTransformWidget methods

//-----------------------------------------------------------------------------
qMRMLVirtualRealityTransformWidget::qMRMLVirtualRealityTransformWidget(vtkMRMLVirtualRealityViewNode* viewNode, QWidget* newParent)
  : Superclass(newParent)
  , d_ptr(new qMRMLVirtualRealityTransformWidgetPrivate(*this))
{
  Q_D(qMRMLVirtualRealityTransformWidget);

  d->VRViewNode = viewNode;

  Q_INIT_RESOURCE(qMRMLVirtualRealityTransformWidget);

  d->init();
}

//-----------------------------------------------------------------------------
qMRMLVirtualRealityTransformWidget::~qMRMLVirtualRealityTransformWidget()
{
}

//-----------------------------------------------------------------------------
void qMRMLVirtualRealityTransformWidget::setMRMLLinearTransformNode(vtkMRMLNode* node)
{
  setMRMLLinearTransformNode(vtkMRMLLinearTransformNode::SafeDownCast(node));
}

//-----------------------------------------------------------------------------
void qMRMLVirtualRealityTransformWidget::setMRMLLinearTransformNode(vtkMRMLLinearTransformNode* node)
{
  Q_D(qMRMLVirtualRealityTransformWidget);

  d->TransformNode = node;
  if (d->TransformNode == nullptr)
  {
    return;
  }

  qvtkReconnect(d->TransformNode, node, vtkCommand::ModifiedEvent,
                this, SLOT(updateWidgetFromMRML()));

  // decipher correct type of images to load/show
  std::string name = std::string(d->TransformNode->GetName());
  if (name.find("VirtualReality") != std::string::npos && name.find("Controller") != std::string::npos)
  {
    // It's a controller!
    d->TransformType = vtkEventDataDevice::LeftController; // Used for both Left and Right
  }
  else
  {
    if (name.find("VirtualReality") != std::string::npos && name.find("HMD") != std::string::npos)
    {
      // It's a headset!
      d->TransformType = vtkEventDataDevice::HeadMountedDisplay;
    }
    else
    {
      if (name.find("VirtualReality") != std::string::npos && name.find("GenericTracker") != std::string::npos)
      {
        // It's a generic tracker!
        d->TransformType = vtkEventDataDevice::GenericTracker;
      }
    }
  }

  if (d->TransformType == vtkEventDataDevice::Unknown)
  {
    qCritical() << "Non-VR transform sent to VRTransformWidget";
  }

  this->updateWidgetFromMRML();
}

//----------------------------------------------------------------------------
void qMRMLVirtualRealityTransformWidget::onButtonClicked()
{
  // Enable/disable Slicer node updates for transform node
  Q_D(qMRMLVirtualRealityTransformWidget);

  if (d->TransformNode == nullptr || d->VRViewNode == nullptr)
  {
    // Not initialized yet, no MRML node to work from
    return;
  }

  switch (d->TransformType)
  {
    case vtkEventDataDevice::HeadMountedDisplay:
      d->VRViewNode->SetHMDTransformUpdate(!d->VRViewNode->GetHMDTransformUpdate());
      break;
    case vtkEventDataDevice::GenericTracker:
      d->VRViewNode->SetTrackerTransformUpdate(!d->VRViewNode->GetTrackerTransformUpdate());
      break;
    case vtkEventDataDevice::LeftController:
      Q_FALLTHROUGH();
    case vtkEventDataDevice::RightController:
      d->VRViewNode->SetControllerTransformsUpdate(!d->VRViewNode->GetControllerTransformsUpdate());
      break;
    default:
      qCritical() << Q_FUNC_INFO << ": Unable to handle controller button click due to unknown transform type:"
        << static_cast<int>(d->TransformType);
      return;
  }

  updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qMRMLVirtualRealityTransformWidget::updateWidgetFromMRML()
{
  Q_D(qMRMLVirtualRealityTransformWidget);

  if (d->TransformType == vtkEventDataDevice::Unknown ||
      d->TransformNode == nullptr)
  {
    d->pushButton_Transform->setEnabled(false);
    if (d->TransformNode == nullptr)
    {
      return;
    }
  }
  else
  {
    d->pushButton_Transform->setEnabled(true);
  }

  ::ETrackingResult status = StringToPoseStatus(d->TransformNode->GetAttribute("VirtualReality.PoseStatus"));
  if (d->PreviousStatus == status)
  {
    // No change
    return;
  }

  if (d->VRViewNode == nullptr)
  {
    return;
  }

  // Choose correct icon based on node attributes
  switch (d->TransformType)
  {
    case vtkEventDataDevice::HeadMountedDisplay:
      {
        if (!d->VRViewNode->GetHMDTransformUpdate())
        {
          d->pushButton_Transform->setIcon(QIcon(":/Icons/headset_status_off.png"));
          break;
        }
        switch (status)
        {
          case ::TrackingResult_Running_OK:
            d->pushButton_Transform->setIcon(QIcon(":/Icons/headset_status_ready.png"));

            break;
          case ::TrackingResult_Running_OutOfRange:
            d->pushButton_Transform->setIcon(QIcon(":/Icons/headset_status_range.png"));

            break;
          case ::TrackingResult_Calibrating_InProgress:
            d->pushButton_Transform->setIcon(QIcon(":/Icons/headset_status_alert.png"));

            break;
          case ::TrackingResult_Calibrating_OutOfRange:
            d->pushButton_Transform->setIcon(QIcon(":/Icons/headset_status_range.png"));
            break;
          default:
            d->pushButton_Transform->setIcon(QIcon(":/Icons/headset_status_off.png"));
        }
        break;
      }
    case vtkEventDataDevice::GenericTracker:
      {
        if (!d->VRViewNode->GetTrackerTransformUpdate())
        {
          d->pushButton_Transform->setIcon(QIcon(":/Icons/tracker_status_off.png"));
          break;
        }
        switch (status)
        {
          case ::TrackingResult_Running_OK:
            d->pushButton_Transform->setIcon(QIcon(":/Icons/tracker_status_ready.png"));

            break;
          case ::TrackingResult_Running_OutOfRange:
            d->pushButton_Transform->setIcon(QIcon(":/Icons/tracker_status_range.png"));

            break;
          case ::TrackingResult_Calibrating_InProgress:
            d->pushButton_Transform->setIcon(QIcon(":/Icons/tracker_status_alert.png"));

            break;
          case ::TrackingResult_Calibrating_OutOfRange:
            d->pushButton_Transform->setIcon(QIcon(":/Icons/tracker_status_range.png"));

            break;
          default:
            d->pushButton_Transform->setIcon(QIcon(":/Icons/tracker_status_off.png"));

        }
        break;
      }
    case vtkEventDataDevice::LeftController:
      Q_FALLTHROUGH();
    case vtkEventDataDevice::RightController:
      {
        if (!d->VRViewNode->GetControllerTransformsUpdate())
        {
          d->pushButton_Transform->setIcon(QIcon(":/Icons/controller_status_off.png"));
          break;
        }
        switch (status)
        {
          case ::TrackingResult_Running_OK:
            d->pushButton_Transform->setIcon(QIcon(":/Icons/controller_status_ready.png"));

            break;
          case ::TrackingResult_Running_OutOfRange:
            d->pushButton_Transform->setIcon(QIcon(":/Icons/controller_status_range.png"));

            break;
          case ::TrackingResult_Calibrating_InProgress:
            d->pushButton_Transform->setIcon(QIcon(":/Icons/controller_status_alert.png"));

            break;
          case ::TrackingResult_Calibrating_OutOfRange:
            d->pushButton_Transform->setIcon(QIcon(":/Icons/controller_status_range.png"));

            break;
          default:
            d->pushButton_Transform->setIcon(QIcon(":/Icons/controller_status_off.png"));

        }
      }
      break;
    default:
      qCritical() << Q_FUNC_INFO << ": Unable to update transform widget due to unknown transform type:"
        << static_cast<int>(d->TransformType);
      break;
  }
}
