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

#include <qdialog.h>
#include "qSlicerHomeVirtualReality.h"

// Qt includes
#include <QWidget>
#include <QAction>
#include <QDebug>
#include <QMainWindow>
#include <QMenu>
#include <QSettings>
#include <QtPlugin>
#include <QPushButton>
#include <QButtonGroup>
#include <QFormLayout>
#include <QSlider>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpacerItem>

//-----------------------------------------------------------------------------
//   qSlicerHomeVirtualWidget Methods

//-----------------------------------------------------------------------------
qSlicerHomeVirtualWidget::qSlicerHomeVirtualWidget(QWidget *parent)
{
  this->minimumHeight(650);
  this->minimumWidth(980);
  this->resize(dimensions::height, dimensions::width);

  // Create all the Qlabels
  QLabel *flySpeedLabel      = new QLabel("Fly Speed:", this);
  QLabel *magnificationLabel = new QLabel("Magnification:", this);
  QLabel *motionSenLabel     = new QLabel("Motion Sensitivity:", this;
  QLabel *lightingLabel      = new QLabel("Lighting:", this);

  // Create all the push buttons
  QPushButton *syncView = new QPushButton("Sync view to Reference View", this);
  QPushButton *magnificationButton1 = new QPushButton("0.5x", this);
  QPushButton *magnificationButton2 = new QPushButton("1x",   this);
  QPushButton *magnificationButton3 = new QPushButton("2x",   this);
  QPushButton *magnificationButton4 = new QPushButton("40x",  this);
  QPushButton *twoSidedLighting     = new QPushButton("Two-sided Lighting", this);
  QPushButton *backLighting         = new QPushButton("Back Lighting", this);

  // Create all sliders
  QSlider *flySpeedSlider  = new QSlider(Qt::Horizontal, this);
  QSlider *motionSenSlider = new QSlider(Qt::Horizontal, this);

  // Create the layouts for the UI
  QFormLayout  *menuLayout                = new QFormLayout(this);
  QHBoxLayout  *magnificationButtonLayout = new QHBoxLayout(this);
  QHBoxLayout  *lightingButtonLayout      = new QHBoxLayout(this);

  // Place Qwidgets in appropriate layouts
  magnificationButtonLayout->addWidget(magnificationButton1);
  magnificationButtonLayout->addWidget(magnificationButton2);
  magnificationButtonLayout->addWidget(magnificationButton3);
  magnificationButtonLayout->addWidget(magnificationButton4);
  lightingButtonLayout->addWidget(twoSidedLighting);
  lightingButtonLayout->addWidget(backLighting);

  // Set up the main form layout
  menuLayout->addRow(flySpeedLabel, flySpeedSlider);
  menuLayout->addRow(magnificationLabel, magnificationButtonLayout);
  menuLayout->addRow(motionSenLabel, motionSenSlider);
  menuLayout->addRow(lightingLabel, lightingButtonLayout);

  // Spacing and positioning
  menuLayout->setHorizontalSpacing(20);
  menuLayout->setVerticleSpacing(99);
}
//---------------------------------------------------------------
qSlicerHomeVirtualWidget::~qSlicerHomeVirtualWidget()
{
};
