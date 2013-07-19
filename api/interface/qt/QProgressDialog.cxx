/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QProgressDialog.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include "QProgressDialog.h"
#include "ui_QProgressDialog.h"

#include "Application.h"

#include <utility>

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QProgressDialog::QProgressDialog( QWidget* parent )
  : QDialog( parent )
{
  this->ui = new Ui_QProgressDialog;
  this->ui->setupUi( this );
  
  QObject::connect(
    this->ui->buttonBox, SIGNAL( rejected() ),
    this, SLOT( slotCancel() ) );

  this->observer = vtkSmartPointer< Command >::New();
  this->observer->ui = this->ui;

  Alder::Application *app = Alder::Application::GetInstance();
  app->AddObserver( vtkCommand::ConfigureEvent, this->observer );
  app->AddObserver( vtkCommand::StartEvent, this->observer );
  app->AddObserver( vtkCommand::ProgressEvent, this->observer );
  app->AddObserver( vtkCommand::EndEvent, this->observer );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QProgressDialog::~QProgressDialog()
{
  Alder::Application::GetInstance()->RemoveObserver( this->observer );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QProgressDialog::setMessage( QString message )
{
  this->ui->label->setText( message );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QProgressDialog::slotCancel()
{
  Alder::Application::GetInstance()->SetAbortFlag( true );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QProgressDialog::Command::Execute(
  vtkObject *caller, unsigned long eventId, void *callData )
{
  if( this->ui )
  {
    QProgressBar *progressBar;

    if( vtkCommand::ConfigureEvent == eventId )
    { // configure the progress bar
      // pair contains two bools: < which progress bar?, show as busy? >
      std::pair<bool, bool> progressConfig = *( static_cast< std::pair<bool, bool>* >( callData ) );
      progressBar = progressConfig.first ? this->ui->globalProgressBar : this->ui->localProgressBar;
      progressBar->setRange( 0, progressConfig.second ? 0 : 100 );
    }
    else if( vtkCommand::StartEvent == eventId )
    { // set the progress to 0
      // which progress bar?
      bool global = *( static_cast< bool* >( callData ) );
      progressBar = global ? this->ui->globalProgressBar : this->ui->localProgressBar;
      if( 0 < progressBar->maximum() ) progressBar->setValue( 0 );
    }
    else if( vtkCommand::ProgressEvent == eventId )
    { // set the progress to the call data
      // pair contains bool and double; < which progress bar?, progress value >
      std::pair<bool, double> progressConfig = *( static_cast< std::pair<bool, double>* >( callData ) );
      progressBar = progressConfig.first ? this->ui->globalProgressBar : this->ui->localProgressBar;
      if( 0 < progressBar->maximum() )
      {
        int percent = static_cast<int>( 100 * progressConfig.second );
        progressBar->setValue( percent );
      }
    }
    else if( vtkCommand::EndEvent == eventId )
    { // set the progress to 100
      // which progress bar?
      bool global = *( static_cast< bool* >( callData ) );
      progressBar = global ? this->ui->globalProgressBar : this->ui->localProgressBar;
      progressBar->setValue( 100 );
    }

    QApplication::processEvents();
  }
}
