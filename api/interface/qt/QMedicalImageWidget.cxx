/*=========================================================================

  Program:  CLSAVis (Canadian Longitudinal Study on Aging Visualizer)
  Module:   QMedicalImageWidget.cxx
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/
#include "QMedicalImageWidget.h"

#include "ui_QMedicalImageWidget.h"

#include "vtkMedicalImageViewer.h"

#include <QScrollBar>

#include <sstream>
#include <stdexcept>

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QMedicalImageWidget::QMedicalImageWidget( QWidget* parent )
  : QWidget( parent )
{
  this->ui = new Ui_QMedicalImageWidget;
  this->ui->setupUi( this );
  this->viewer = vtkMedicalImageViewer::New();
  this->viewer->SetRenderWindow( this->ui->vtkWidget->GetRenderWindow() );
  this->resetImage();

  QObject::connect(
    this->ui->scrollBar, SIGNAL( valueChanged( int ) ),
    this, SLOT( slotSliceChanged( int ) ) );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QMedicalImageWidget::~QMedicalImageWidget()
{
  if( NULL != this->viewer )
  {
    this->viewer->Delete();
    this->viewer = NULL;
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMedicalImageWidget::resetImage()
{
  this->viewer->SetImageToSinusoid();
  this->updateInterface();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMedicalImageWidget::loadImage( QString filename )
{
  if( !this->viewer->Load( filename.toStdString() ) )
  {
    std::stringstream stream;
    stream << "Unable to load image file \"" << filename.toStdString() << "\"";
    throw std::runtime_error( stream.str() );
  }

  this->updateInterface();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMedicalImageWidget::slotSliceChanged( int slice )
{
  this->viewer->SetSlice( slice );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMedicalImageWidget::updateInterface()
{
  // update the slider
  int dims = this->viewer->GetImageDimensionality();
  if( 3 > dims )
  { // hide the slider, we don't need it
    this->ui->scrollBar->setVisible( false );
  }
  else
  { // show the slider and update the its range
    this->ui->scrollBar->setVisible( true );
    
    int min = this->viewer->GetSliceMin();
    int max = this->viewer->GetSliceMax();
    this->ui->scrollBar->setMinimum( min );
    this->ui->scrollBar->setMaximum( max );
    this->ui->scrollBar->setSliderPosition( static_cast<int>( 0.5 * ( min + max ) ) );
  }
}
