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

// Slicer includes
#include <qSlicerAbstractCoreModule.h>
#include <qSlicerApplication.h>
#include <qSlicerModuleManager.h>

// qMRML includes
#include "qMRMLVirtualRealityTransformWidget.h"
#include "ui_qMRMLVirtualRealityTransformWidget.h"

// MRML includes
#include <vtkMRMLLinearTransformNode.h>

// OpenVR includes
#include <openvr.h>

// SlicerVR includes
#include <qMRMLVirtualRealityView.h>
#include <vtkMRMLVirtualRealityViewNode.h>

// Qt includes
#include <QDebug>
#include <QPushButton>

namespace
{
  vr::ETrackingResult StringToPoseStatus(const char* cpose)
  {
    if (cpose == nullptr)
    {
      return vr::TrackingResult_Uninitialized;
    }
    std::string pose(cpose);

    if (pose.compare("CalibratingInProgress") == 0)
    {
      return vr::TrackingResult_Calibrating_InProgress;
    }
    else if (pose.compare("CalibratingOutOfRange") == 0)
    {
      return vr::TrackingResult_Calibrating_OutOfRange;
    }
    else if (pose.compare("RunningOk") == 0)
    {
      return vr::TrackingResult_Running_OK;
    }
    else if (pose.compare("RunningOutOfRange") == 0)
    {
      return vr::TrackingResult_Running_OutOfRange;
    }
    else
    {
      return vr::TrackingResult_Uninitialized;
    }
  }
}
//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Markups
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
  vr::ETrackedDeviceClass                       TransformType;
  vr::ETrackingResult                           PreviousStatus;
};

//-----------------------------------------------------------------------------
// qMRMLVirtualRealityTransformWidgetPrivate methods

//-----------------------------------------------------------------------------
qMRMLVirtualRealityTransformWidgetPrivate::qMRMLVirtualRealityTransformWidgetPrivate(qMRMLVirtualRealityTransformWidget& object)
  : q_ptr(&object)
  , VRViewNode(nullptr)
  , TransformNode(nullptr)
  , TransformType(vr::TrackedDeviceClass_Invalid)
  , PreviousStatus(vr::TrackingResult_Uninitialized)
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
    d->TransformType = vr::TrackedDeviceClass_Controller;
  }
  else
  {
    if (name.find("VirtualReality") != std::string::npos && name.find("HMD") != std::string::npos)
    {
      // It's a headset!
      d->TransformType = vr::TrackedDeviceClass_HMD;
    }
    else
    {
      if (name.find("VirtualReality") != std::string::npos && name.find("GenericTracker") != std::string::npos)
      {
        // It's a generic tracker!
        d->TransformType = vr::TrackedDeviceClass_GenericTracker;
      }
    }
  }

  if (d->TransformType == vr::TrackedDeviceClass_Invalid)
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
    case vr::TrackedDeviceClass_HMD:
      d->VRViewNode->SetHMDTransformUpdate(!d->VRViewNode->GetHMDTransformUpdate());
      break;
    case vr::TrackedDeviceClass_GenericTracker:
      d->VRViewNode->SetTrackerTransformUpdate(!d->VRViewNode->GetTrackerTransformUpdate());
      break;
    case vr::TrackedDeviceClass_Controller:
      d->VRViewNode->SetControllerTransformsUpdate(!d->VRViewNode->GetControllerTransformsUpdate());
      break;
    default:
      qCritical() << Q_FUNC_INFO << ": Unable to handle controller button click due to unknown transform type:"
        << d->TransformType;
      return;
  }

  updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qMRMLVirtualRealityTransformWidget::updateWidgetFromMRML()
{
  Q_D(qMRMLVirtualRealityTransformWidget);

  if (d->TransformType == vr::TrackedDeviceClass_Invalid ||
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

  vr::ETrackingResult status = StringToPoseStatus(d->TransformNode->GetAttribute("VirtualReality.PoseStatus"));
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
    case vr::TrackedDeviceClass_HMD:
      {
        if (!d->VRViewNode->GetHMDTransformUpdate())
        {
          d->pushButton_Transform->setIcon(QIcon(":/Icons/headset_status_off.png"));
          break;
        }
        switch (status)
        {
          case vr::TrackingResult_Running_OK:
            d->pushButton_Transform->setIcon(QIcon(":/Icons/headset_status_ready.png"));

            break;
          case vr::TrackingResult_Running_OutOfRange:
            d->pushButton_Transform->setIcon(QIcon(":/Icons/headset_status_range.png"));

            break;
          case vr::TrackingResult_Calibrating_InProgress:
            d->pushButton_Transform->setIcon(QIcon(":/Icons/headset_status_alert.png"));

            break;
          case vr::TrackingResult_Calibrating_OutOfRange:
            d->pushButton_Transform->setIcon(QIcon(":/Icons/headset_status_range.png"));
            break;
          default:
            d->pushButton_Transform->setIcon(QIcon(":/Icons/headset_status_off.png"));
        }
        break;
      }
    case vr::TrackedDeviceClass_GenericTracker:
      {
        if (!d->VRViewNode->GetTrackerTransformUpdate())
        {
          d->pushButton_Transform->setIcon(QIcon(":/Icons/tracker_status_off.png"));
          break;
        }
        switch (status)
        {
          case vr::TrackingResult_Running_OK:
            d->pushButton_Transform->setIcon(QIcon(":/Icons/tracker_status_ready.png"));

            break;
          case vr::TrackingResult_Running_OutOfRange:
            d->pushButton_Transform->setIcon(QIcon(":/Icons/tracker_status_range.png"));

            break;
          case vr::TrackingResult_Calibrating_InProgress:
            d->pushButton_Transform->setIcon(QIcon(":/Icons/tracker_status_alert.png"));

            break;
          case vr::TrackingResult_Calibrating_OutOfRange:
            d->pushButton_Transform->setIcon(QIcon(":/Icons/tracker_status_range.png"));

            break;
          default:
            d->pushButton_Transform->setIcon(QIcon(":/Icons/tracker_status_off.png"));

        }
        break;
      }
    case vr::TrackedDeviceClass_Controller:
      {
        if (!d->VRViewNode->GetControllerTransformsUpdate())
        {
          d->pushButton_Transform->setIcon(QIcon(":/Icons/controller_status_off.png"));
          break;
        }
        switch (status)
        {
          case vr::TrackingResult_Running_OK:
            d->pushButton_Transform->setIcon(QIcon(":/Icons/controller_status_ready.png"));

            break;
          case vr::TrackingResult_Running_OutOfRange:
            d->pushButton_Transform->setIcon(QIcon(":/Icons/controller_status_range.png"));

            break;
          case vr::TrackingResult_Calibrating_InProgress:
            d->pushButton_Transform->setIcon(QIcon(":/Icons/controller_status_alert.png"));

            break;
          case vr::TrackingResult_Calibrating_OutOfRange:
            d->pushButton_Transform->setIcon(QIcon(":/Icons/controller_status_range.png"));

            break;
          default:
            d->pushButton_Transform->setIcon(QIcon(":/Icons/controller_status_off.png"));

        }
      }
      break;
    default:
      qCritical() << Q_FUNC_INFO << ": Unable to update transform widget due to unknown transform type:"
        << d->TransformType;
      break;
  }
}
