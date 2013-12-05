/*==============================================================================

  Module:    QAlderFramePlayerWidget.h
  Program:   Alder (CLSA Medical Image Quality Assessment Tool)
  Language:  C++
  Author:    Patrick Emond <emondpd AT mcmaster DOT ca>
  Author:    Dean Inglis <inglisd AT mcmaster DOT ca>

  Library: MSVTK

  Copyright (c) Kitware Inc.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/
#include <QAlderFramePlayerWidget.h>
#include <ui_QAlderFramePlayerWidget.h>

// Qt includes
#include <QIcon>
#include <QTime>
#include <QTimer>

// VTK includes
#include <vtkMath.h>
#include <vtkMedicalImageViewer.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
class QAlderFramePlayerWidgetPrivate : public Ui_QAlderFramePlayerWidget
{
  Q_DECLARE_PUBLIC(QAlderFramePlayerWidget);
protected:
  QAlderFramePlayerWidget* const q_ptr;

  vtkSmartPointer<vtkMedicalImageViewer> viewer;

  double maxFrameRate;                    // Time Playing speed factor.
  QTimer* timer;                          // Timer to process the player.
  QTime realTime;                         // Time to reference the real one.
  QAbstractAnimation::Direction direction;// Sense of direction

public:
  QAlderFramePlayerWidgetPrivate(QAlderFramePlayerWidget& object);
  virtual ~QAlderFramePlayerWidgetPrivate();

  virtual void setupUi(QWidget*);
  virtual void updateUi();

  struct PipelineInfoType
    {
    PipelineInfoType();

    bool isConnected;
    unsigned int numberOfFrames;
    double frameRange[2];
    double currentFrame;
    int maxFrameRate;

    void printSelf()const;
    double clampTimeInterval(double, double) const; // Transform a frameRate into a time interval
    double validateFrame(double) const;    // Validate a frame
    double nextFrame() const;     // Get the next frame.
    double previousFrame() const; // Get the previous frame.
    };
  PipelineInfoType retrievePipelineInfo();            // Get pipeline information.
  virtual void processRequest(double);                // Request Data and Update
  virtual bool isConnected();                         // Check if the pipeline is ready
  virtual void processRequest(const PipelineInfoType&, double); // Request Data and Update
  virtual void requestData(const PipelineInfoType&, double);    // Request Data by time
  virtual void updateUi(const PipelineInfoType&);               // Update the widget giving pipeline statut
};

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
//
// QAlderFramePlayerWidgetPrivate methods
//
//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAlderFramePlayerWidgetPrivate::QAlderFramePlayerWidgetPrivate
(QAlderFramePlayerWidget& object)
  : q_ptr(&object)
{
  this->maxFrameRate = 60;          // 60 FPS by default
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAlderFramePlayerWidgetPrivate::~QAlderFramePlayerWidgetPrivate()
{
  this->viewer = NULL;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAlderFramePlayerWidgetPrivate::PipelineInfoType::PipelineInfoType()
  : isConnected(false)
  , numberOfFrames(0)
  , currentFrame(0)
  , maxFrameRate(60)
{
  this->frameRange[0] = 0;
  this->frameRange[1] = 0;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidgetPrivate::PipelineInfoType::printSelf()const
{
  std::cout << "---------------------------------------------------------------" << std::endl
            << "Pipeline info: " << this << std::endl
            << "Number of image frames: " << this->numberOfFrames << std::endl
            << "Frame range: " << this->frameRange[0] << " " << this->frameRange[1] << std::endl
            << "Last frame request: " << this->currentFrame << std::endl
            << "Maximum frame rate: " << this->maxFrameRate << std::endl
            << "Is connected: " << this->isConnected << std::endl;

}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAlderFramePlayerWidgetPrivate::PipelineInfoType
QAlderFramePlayerWidgetPrivate::retrievePipelineInfo()
{
  PipelineInfoType pipeInfo;

  pipeInfo.isConnected = this->isConnected();
  if (!pipeInfo.isConnected)
    return pipeInfo;
  if( this->viewer->GetImageDimensionality() < 3 ) 
    return pipeInfo;

  pipeInfo.numberOfFrames =
    this->viewer->GetNumberOfSlices();
  pipeInfo.frameRange[0] = this->viewer->GetSliceMin();
  pipeInfo.frameRange[1] = this->viewer->GetSliceMax();
  
  pipeInfo.currentFrame = this->viewer->GetSlice();
  pipeInfo.maxFrameRate = this->viewer->GetMaxFrameRate();

  return pipeInfo;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QAlderFramePlayerWidgetPrivate::PipelineInfoType::
clampTimeInterval(double playbackSpeed, double maxFrameRate) const
{
  Q_ASSERT(playbackSpeed > 0.);

  // the time interval is the time between QTimer emitting
  // timeout signals, which in turn fires onTick, wherein the
  // frame is selected and displayed by the viewer
  // the playback speed is set in frames per second: eg., 60 FPS

  // Clamp the frame rate
  double rate = qMin( playbackSpeed, maxFrameRate);
  
  // return the time interval
  return  1000. / rate;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QAlderFramePlayerWidgetPrivate::PipelineInfoType::validateFrame(double frame) const
{
  if (this->numberOfFrames == 0)
    return vtkMath::Nan();
  else if (this->numberOfFrames == 1)
    return this->frameRange[0];
  return frame;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QAlderFramePlayerWidgetPrivate::PipelineInfoType::previousFrame() const
{
  return this->validateFrame(this->currentFrame-1);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QAlderFramePlayerWidgetPrivate::PipelineInfoType::nextFrame() const
{
  return this->validateFrame(this->currentFrame+1);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidgetPrivate::setupUi(QWidget* widget)
{
  Q_Q(QAlderFramePlayerWidget);

  this->Ui_QAlderFramePlayerWidget::setupUi(widget);
  this->timer = new QTimer(widget);

  // Connect Menu ToolBars actions
  q->connect(this->firstFrameButton, SIGNAL(pressed()), q, SLOT(goToFirstFrame()));
  q->connect(this->previousFrameButton, SIGNAL(pressed()), q, SLOT(goToPreviousFrame()));
  q->connect(this->playButton, SIGNAL(toggled(bool)), q, SLOT(playForward(bool)));
  q->connect(this->playReverseButton, SIGNAL(toggled(bool)), q, SLOT(playBackward(bool)));
  q->connect(this->nextFrameButton, SIGNAL(pressed()), q, SLOT(goToNextFrame()));
  q->connect(this->lastFrameButton, SIGNAL(pressed()), q, SLOT(goToLastFrame()));
  q->connect(this->speedFactorSpinBox, SIGNAL(valueChanged(double)), q, SLOT(setPlaySpeed(double)));

  // Connect the time slider
  q->connect(this->frameSlider, SIGNAL(valueChanged(double)), q, SLOT(setCurrentFrame(double)));
  this->frameSlider->setSuffix("");
  this->frameSlider->setDecimals(0);
  this->frameSlider->setSingleStep(1);

  // Connect the Timer for animation
  q->connect(this->timer, SIGNAL(timeout()), q, SLOT(onTick()));
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidgetPrivate::updateUi()
{
  PipelineInfoType pipeInfo = this->retrievePipelineInfo();
  this->updateUi(pipeInfo);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidgetPrivate::updateUi(const PipelineInfoType& pipeInfo)
{
  // Buttons
  this->firstFrameButton->setEnabled((pipeInfo.currentFrame > pipeInfo.frameRange[0]));
  this->previousFrameButton->setEnabled((pipeInfo.currentFrame > pipeInfo.frameRange[0]));
  this->playButton->setEnabled((pipeInfo.numberOfFrames > 1));
  this->playReverseButton->setEnabled((pipeInfo.numberOfFrames > 1));
  this->nextFrameButton->setEnabled((pipeInfo.currentFrame < pipeInfo.frameRange[1]));
  this->lastFrameButton->setEnabled((pipeInfo.currentFrame < pipeInfo.frameRange[1]));
  this->repeatButton->setEnabled((pipeInfo.numberOfFrames > 1));
  this->speedFactorSpinBox->setEnabled((pipeInfo.numberOfFrames > 1));

  // Slider
  this->frameSlider->blockSignals(true);
  this->frameSlider->setEnabled((pipeInfo.frameRange[0]!=pipeInfo.frameRange[1]));
  this->frameSlider->setRange(pipeInfo.frameRange[0], pipeInfo.frameRange[1]);
  this->frameSlider->setValue(pipeInfo.currentFrame);
  this->frameSlider->blockSignals(false);

  // SpinBox
  // the max frame rate from the pipeinfo object is set fom the viewer's information
  // about frame rate.  The value of the speed factor spin box set here is a suggested
  // value.  The speed can be set and is clamped between 1 and whatever the max frame
  // rate set through the QAlderFramePlayerWidget's maxFrameRate property.
  this->speedFactorSpinBox->blockSignals( true );
  this->speedFactorSpinBox->setValue(
    qMin( this->speedFactorSpinBox->value(), this->maxFrameRate ) );
  this->speedFactorSpinBox->blockSignals( false );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidgetPrivate::requestData(const PipelineInfoType& pipeInfo,
                                              double frame)
{
  Q_Q(QAlderFramePlayerWidget);

  // We clamp the time requested
  frame = qBound( pipeInfo.frameRange[0], 
            static_cast<double>(vtkMath::Round(frame)), 
            pipeInfo.frameRange[1] );

  // Abort the request
  if (!pipeInfo.isConnected || frame == pipeInfo.currentFrame)
    return;

  this->viewer->SetSlice( frame );
  emit q->currentFrameChanged(frame); // Emit the change
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidgetPrivate::processRequest(double frame)
{
  PipelineInfoType pipeInfo = this->retrievePipelineInfo();
  this->processRequest(pipeInfo, frame);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidgetPrivate::processRequest(const PipelineInfoType& pipeInfo,
                                                 double frame)
{
  if (vtkMath::IsNan(frame))
    return;

  this->requestData(pipeInfo, frame);
  this->updateUi();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QAlderFramePlayerWidgetPrivate::isConnected()
{
  return this->viewer && this->viewer->GetInput();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
//
// QAlderFramePlayerWidget methods
//
//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAlderFramePlayerWidget::QAlderFramePlayerWidget(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new QAlderFramePlayerWidgetPrivate(*this))
{
  Q_D(QAlderFramePlayerWidget);
  d->setupUi(this);
  d->updateUi();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAlderFramePlayerWidget::~QAlderFramePlayerWidget()
{
  Q_D(QAlderFramePlayerWidget);
  this->stop();
  d->viewer = 0;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::setViewer(vtkMedicalImageViewer* viewer)
{
  Q_D(QAlderFramePlayerWidget);

  d->viewer = viewer;
  d->updateUi();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
vtkMedicalImageViewer* QAlderFramePlayerWidget::viewer() const
{
  Q_D(const QAlderFramePlayerWidget);
  return d->viewer;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::updateFromViewer()
{
  Q_D(QAlderFramePlayerWidget);
  d->updateUi();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::goToFirstFrame()
{
  Q_D(QAlderFramePlayerWidget);

  // Fetch pipeline information
  QAlderFramePlayerWidgetPrivate::PipelineInfoType
    pipeInfo = d->retrievePipelineInfo();

  d->processRequest(pipeInfo, pipeInfo.frameRange[0]);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::goToPreviousFrame()
{
  Q_D(QAlderFramePlayerWidget);

  // Fetch pipeline information
  QAlderFramePlayerWidgetPrivate::PipelineInfoType
    pipeInfo = d->retrievePipelineInfo();

  d->processRequest(pipeInfo, pipeInfo.previousFrame());
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::goToNextFrame()
{
  Q_D(QAlderFramePlayerWidget);

  // Fetch pipeline information
  QAlderFramePlayerWidgetPrivate::PipelineInfoType
    pipeInfo = d->retrievePipelineInfo();

  d->processRequest(pipeInfo, pipeInfo.nextFrame());
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::goToLastFrame()
{
  Q_D(QAlderFramePlayerWidget);

  // Fetch pipeline information
  QAlderFramePlayerWidgetPrivate::PipelineInfoType
    pipeInfo = d->retrievePipelineInfo();

  d->processRequest(pipeInfo, pipeInfo.frameRange[1]);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::play(bool playPause)
{
  if (!playPause)
    this->pause();
  if (playPause)
    this->play();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::play()
{
  Q_D(QAlderFramePlayerWidget);

  // Fetch pipeline information
  QAlderFramePlayerWidgetPrivate::PipelineInfoType
    pipeInfo = d->retrievePipelineInfo();
  double period = pipeInfo.frameRange[1] - pipeInfo.frameRange[0];

  if (!d->viewer || period == 0)
    return;

  if (d->direction == QAbstractAnimation::Forward) {
      d->playReverseButton->blockSignals(true);
      d->playReverseButton->setChecked(false);
      d->playReverseButton->blockSignals(false);

      // Use when set the play by script
      if (!d->playButton->isChecked()) {
        d->playButton->blockSignals(true);
        d->playButton->setChecked(true);
        d->playButton->blockSignals(false);
      }

    // We reset the Slider to the initial value if we play from the end
    if (pipeInfo.currentFrame == pipeInfo.frameRange[1])
      d->frameSlider->setValue(pipeInfo.frameRange[0]);
  }
  else if (d->direction == QAbstractAnimation::Backward) {
      d->playButton->blockSignals(true);
      d->playButton->setChecked(false);
      d->playButton->blockSignals(false);

      // Use when set the play by script
      if (!d->playReverseButton->isChecked()) {
        d->playReverseButton->blockSignals(true);
        d->playReverseButton->setChecked(true);
        d->playReverseButton->blockSignals(false);
      }

    // We reset the Slider to the initial value if we play from the beginning
    if (pipeInfo.currentFrame == pipeInfo.frameRange[0])
      d->frameSlider->setValue(pipeInfo.frameRange[1]);
  }

  double timeInterval =
    pipeInfo.clampTimeInterval(d->speedFactorSpinBox->value(), d->maxFrameRate);

  d->realTime.start();
  d->timer->start(timeInterval);
  emit this->playing(true);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::pause()
{
  Q_D(QAlderFramePlayerWidget);

  if (d->direction == QAbstractAnimation::Forward)
    d->playButton->setChecked(false);
  else if (d->direction == QAbstractAnimation::Backward)
    d->playReverseButton->setChecked(false);

  if (d->timer->isActive()) {
    d->timer->stop();
    emit this->playing(false);
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::stop()
{
  this->pause();
  this->goToFirstFrame();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::playForward(bool play)
{
  this->setDirection(QAbstractAnimation::Forward);
  this->play(play);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::playBackward(bool play)
{
  this->setDirection(QAbstractAnimation::Backward);
  this->play(play);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::onTick()
{
  Q_D(QAlderFramePlayerWidget);

  // Forward the internal timer timeout signal
  emit this->onTimeout();

  // Fetch pipeline information
  QAlderFramePlayerWidgetPrivate::PipelineInfoType
    pipeInfo = d->retrievePipelineInfo();

  // currentFrame + number of milliseconds since starting x speed x direction  
  double sec = d->realTime.restart() / 1000.;
  double frameRequest = pipeInfo.currentFrame + sec *
                       d->speedFactorSpinBox->value() *
                       ((d->direction == QAbstractAnimation::Forward) ? 1 : -1);

  if (d->playButton->isChecked() && !d->playReverseButton->isChecked()) {
    if (frameRequest > pipeInfo.frameRange[1] && !d->repeatButton->isChecked()) {
      d->processRequest(pipeInfo, frameRequest);
      this->playForward(false);
      return;
    }
    else if (frameRequest > pipeInfo.frameRange[1] &&
             d->repeatButton->isChecked()) { // We Loop
      frameRequest = pipeInfo.frameRange[0];
      emit this->loop();
    }
  }
  else if (!d->playButton->isChecked() && d->playReverseButton->isChecked()) {
    if (frameRequest < pipeInfo.frameRange[0] && !d->repeatButton->isChecked()) {
      d->processRequest(pipeInfo, frameRequest);
      this->playBackward(false);
      return;
    }
    else if (frameRequest < pipeInfo.frameRange[0] &&
             d->repeatButton->isChecked()) { // We Loop
      frameRequest = pipeInfo.frameRange[1];
      emit this->loop();
    }
  }
  else
  {
    return; // Undefined status
  }

  d->processRequest(pipeInfo, frameRequest);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::setCurrentFrame(double frame)
{
  Q_D(QAlderFramePlayerWidget);
  d->processRequest(frame);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::setPlaySpeed(double speedFactor)
{
  Q_D(QAlderFramePlayerWidget);
  speedFactor = speedFactor <= 0. ? 1. : speedFactor;
  d->speedFactorSpinBox->setValue(speedFactor);

  QAlderFramePlayerWidgetPrivate::PipelineInfoType
    pipeInfo = d->retrievePipelineInfo();

  double timeInterval =
    pipeInfo.clampTimeInterval(speedFactor, d->maxFrameRate);
  d->timer->setInterval(timeInterval);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
//
// QAlderFramePlayerWidget methods -- Widgets Interface
//
//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::setFirstFrameIcon(const QIcon& ico)
{
  Q_D(QAlderFramePlayerWidget);
  d->firstFrameButton->setIcon(ico);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::setPreviousFrameIcon(const QIcon& ico)
{
  Q_D(QAlderFramePlayerWidget);
  d->previousFrameButton->setIcon(ico);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::setPlayIcon(const QIcon& ico)
{
  Q_D(QAlderFramePlayerWidget);
  d->playButton->setIcon(ico);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::setPlayReverseIcon(const QIcon& ico)
{
  Q_D(QAlderFramePlayerWidget);
  d->playReverseButton->setIcon(ico);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::setNextFrameIcon(const QIcon& ico)
{
  Q_D(QAlderFramePlayerWidget);
  d->nextFrameButton->setIcon(ico);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::setLastFrameIcon(const QIcon& ico)
{
  Q_D(QAlderFramePlayerWidget);
  d->lastFrameButton->setIcon(ico);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::setRepeatIcon(const QIcon& ico)
{
  Q_D(QAlderFramePlayerWidget);
  d->repeatButton->setIcon(ico);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QIcon QAlderFramePlayerWidget::firstFrameIcon() const
{
  Q_D(const QAlderFramePlayerWidget);
  return d->firstFrameButton->icon();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QIcon QAlderFramePlayerWidget::previousFrameIcon() const
{
  Q_D(const QAlderFramePlayerWidget);
  return d->previousFrameButton->icon();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QIcon QAlderFramePlayerWidget::playIcon() const
{
  Q_D(const QAlderFramePlayerWidget);
  return d->playButton->icon();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QIcon QAlderFramePlayerWidget::playReverseIcon() const
{
  Q_D(const QAlderFramePlayerWidget);
  return d->playReverseButton->icon();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QIcon QAlderFramePlayerWidget::nextFrameIcon() const
{
  Q_D(const QAlderFramePlayerWidget);
  return d->nextFrameButton->icon();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QIcon QAlderFramePlayerWidget::lastFrameIcon() const
{
  Q_D(const QAlderFramePlayerWidget);
  return d->lastFrameButton->icon();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QIcon QAlderFramePlayerWidget::repeatIcon() const
{
  Q_D(const QAlderFramePlayerWidget);
  return d->repeatButton->icon();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::setPlayReverseVisibility(bool visible)
{
  Q_D(QAlderFramePlayerWidget);
  d->playReverseButton->setVisible(visible);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::setBoundFramesVisibility(bool visible)
{
  Q_D(QAlderFramePlayerWidget);

  d->firstFrameButton->setVisible(visible);
  d->lastFrameButton->setVisible(visible);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::setGoToVisibility(bool visible)
{
  Q_D(QAlderFramePlayerWidget);

  d->previousFrameButton->setVisible(visible);
  d->nextFrameButton->setVisible(visible);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::setFrameSpinBoxVisibility(bool visible)
{
  Q_D(QAlderFramePlayerWidget);
  d->frameSlider->setSpinBoxVisible(visible);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QAlderFramePlayerWidget::playReverseVisibility() const
{
  Q_D(const QAlderFramePlayerWidget);
  return d->playReverseButton->isVisibleTo(
    const_cast<QAlderFramePlayerWidget*>(this));
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QAlderFramePlayerWidget::boundFramesVisibility() const
{
  Q_D(const QAlderFramePlayerWidget);
  return (d->firstFrameButton->isVisibleTo(
            const_cast<QAlderFramePlayerWidget*>(this)) &&
          d->lastFrameButton->isVisibleTo(
            const_cast<QAlderFramePlayerWidget*>(this)));
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QAlderFramePlayerWidget::goToVisibility() const
{
  Q_D(const QAlderFramePlayerWidget);
  return (d->previousFrameButton->isVisibleTo(
            const_cast<QAlderFramePlayerWidget*>(this)) &&
          d->nextFrameButton->isVisibleTo(
            const_cast<QAlderFramePlayerWidget*>(this)));
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QAlderFramePlayerWidget::frameSpinBoxVisibility() const
{
  Q_D(const QAlderFramePlayerWidget);
  return d->frameSlider->spinBox()->isVisibleTo(
    const_cast<QAlderFramePlayerWidget*>(this));
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::setSliderDecimals(int decimals)
{
  Q_D(QAlderFramePlayerWidget);
  d->frameSlider->setDecimals(decimals);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::setSliderPageStep(double pageStep)
{
  Q_D(QAlderFramePlayerWidget);
  d->frameSlider->setPageStep(pageStep);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::setSliderSingleStep(double singleStep)
{
  Q_D(QAlderFramePlayerWidget);

  if (singleStep < 0.) {
    return;
  }

  d->frameSlider->setSingleStep(singleStep);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
int QAlderFramePlayerWidget::sliderDecimals() const
{
  Q_D(const QAlderFramePlayerWidget);
  return d->frameSlider->decimals();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QAlderFramePlayerWidget::sliderPageStep() const
{
  Q_D(const QAlderFramePlayerWidget);
  return d->frameSlider->pageStep();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QAlderFramePlayerWidget::sliderSingleStep() const
{
  Q_D(const QAlderFramePlayerWidget);
  return d->frameSlider->singleStep();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::setDirection(QAbstractAnimation::Direction direction)
{
  Q_D(QAlderFramePlayerWidget);

  if (d->direction != direction) {
    d->direction = direction;
    emit this->directionChanged(direction);
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAbstractAnimation::Direction QAlderFramePlayerWidget::direction() const
{
  Q_D(const QAlderFramePlayerWidget);
  return d->direction;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::setRepeat(bool repeat)
{
  Q_D(const QAlderFramePlayerWidget);
  d->repeatButton->setChecked(repeat);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QAlderFramePlayerWidget::repeat() const
{
  Q_D(const QAlderFramePlayerWidget);
  return d->repeatButton->isChecked();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderFramePlayerWidget::setMaxFramerate(double frameRate)
{
  Q_D(QAlderFramePlayerWidget);
  // Clamp frameRate min value
  frameRate = (frameRate <= 0) ? 60 : frameRate;
  d->maxFrameRate = frameRate;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QAlderFramePlayerWidget::maxFramerate() const
{
  Q_D(const QAlderFramePlayerWidget);
  return d->maxFrameRate;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QAlderFramePlayerWidget::currentFrame() const
{
  Q_D(const QAlderFramePlayerWidget);
  return d->frameSlider->value();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double QAlderFramePlayerWidget::playSpeed() const
{
  Q_D(const QAlderFramePlayerWidget);
  return d->speedFactorSpinBox->value();
}
