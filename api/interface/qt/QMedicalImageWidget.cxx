/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
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
  this->viewer = vtkSmartPointer<vtkMedicalImageViewer>::New();
  this->viewer->SetRenderWindow( this->ui->vtkWidget->GetRenderWindow() );
  this->viewer->InterpolateOff();
  this->resetImage();

  this->ui->framePlayerWidget->setViewer(this->viewer);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QMedicalImageWidget::~QMedicalImageWidget()
{
  //TODO this may have to be place in the closeEvent of the parent UI object
  this->ui->framePlayerWidget->setViewer(0);
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
void QMedicalImageWidget::updateInterface()
{
  this->ui->framePlayerWidget->updateFromViewer();
}

