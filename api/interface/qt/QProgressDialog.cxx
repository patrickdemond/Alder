/*=========================================================================

  Program:  CLSAVis (Canadian Longitudinal Study on Aging Visualizer)
  Module:   QProgressDialog.cxx
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/
#include "QProgressDialog.h"
#include "ui_QProgressDialog.h"

#include "Application.h"

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QProgressDialog::QProgressDialog( QWidget* parent )
  : QDialog( parent )
{
  this->ui = new Ui_QProgressDialog;
  this->ui->setupUi( this );
  
  this->observer = vtkSmartPointer< Command >::New();
  this->observer->ui = this->ui;
  Alder::Application::GetInstance()->AddObserver( vtkCommand::ProgressEvent, this->observer );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QProgressDialog::~QProgressDialog()
{
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QProgressDialog::setMessage( QString message )
{
  this->ui->label->setText( message );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QProgressDialog::Command::Execute(
  vtkObject *caller, unsigned long eventId, void *callData )
{
  if( this->ui )
  {
    // display the progress
    double progress = *( static_cast<double*>( callData ) );
    int value = (int)( 100 * progress ) + 1;
    if( 100 < value ) value = 100;
    this->ui->progressBar->setValue( value );

    this->ui->label->repaint();
    this->ui->progressBar->repaint();
    this->ui->buttonBox->repaint();
  }
}
