/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QAlderAtlasWidget.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include "QAlderAtlasWidget.h"
#include "ui_QAlderAtlasWidget.h"

#include "Application.h"
#include "Exam.h"
#include "Image.h"
#include "Interview.h"
#include "Modality.h"
#include "Rating.h"
#include "User.h"

#include "vtkNew.h"

#include "QProgressDialog.h"

#include <QMessageBox>

#include <stdexcept>

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAlderAtlasWidget::QAlderAtlasWidget( QWidget* parent )
  : QWidget( parent )
{
  Alder::Application *app = Alder::Application::GetInstance();
  
  this->ui = new Ui_QAlderAtlasWidget;
  this->ui->setupUi( this );

  QObject::connect(
    this->ui->previousPushButton, SIGNAL( clicked() ),
    this, SLOT( slotPrevious() ) );
  QObject::connect(
    this->ui->nextPushButton, SIGNAL( clicked() ),
    this, SLOT( slotNext() ) );
  QObject::connect(
    this->ui->ratingComboBox, SIGNAL( valueChanged( int ) ),
    this, SLOT( slotRatingChanged( int ) ) );
  QObject::connect(
    this->ui->noteTextEdit, SIGNAL( textChanged() ),
    this, SLOT( slotNoteChanged() ) );

  this->updateInterface();

  // give a bit more room to the help edit
  double total = this->ui->helpTextEdit->height() + this->ui->noteTextEdit->height();
  QList<int> sizeList;
  sizeList.append( floor( 2 * total / 3 ) );
  sizeList.append( total - sizeList[0] );
  this->ui->splitter->setSizes( sizeList );
};

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAlderAtlasWidget::~QAlderAtlasWidget()
{
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderAtlasWidget::slotPrevious()
{
  Alder::Application *app = Alder::Application::GetInstance();
  Alder::Image *image = app->GetActiveAtlasImage();
  
  if( NULL != image )
  {
    app->SetActiveAtlasImage( image->GetPreviousAtlasImage() );
    this->updateInterface();
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderAtlasWidget::slotNext()
{
  Alder::Application *app = Alder::Application::GetInstance();
  Alder::Image *image = app->GetActiveAtlasImage();
  
  if( NULL != image )
  {
    app->SetActiveAtlasImage( image->GetNextAtlasImage() );
    emit this->activeImageChanged();
    this->updateInterface();
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderAtlasWidget::slotRatingChanged( int value )
{
  //TODO: implement
  // when the rating changes for the current interview's image
  // find another expert rated image of the same modality and exam type
  
  Alder::Application *app = Alder::Application::GetInstance();

  // make sure we have an active user and image
  Alder::User *user = app->GetActiveUser();
  if( !user ) throw std::runtime_error( "Rating slider modified without an active user" );
  Alder::Image *image = app->GetActiveImage();
  if( !image ) throw std::runtime_error( "Rating slider modified without an active image" );

  // See if we have an atlas entry for this kind of image at the requested rating

  // if we do, update the current atlas image and display
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderAtlasWidget::updateInfo()
{
  QString helpString = "";
  QString noteString = "";
  QString interviewerString = tr( "N/A" );
  QString siteString = tr( "N/A" );
  QString dateString = tr( "N/A" );
  QString uidString = tr( "N/A" );

  Alder::Application *app = Alder::Application::GetInstance();
  Alder::Interview *interview = app->GetActiveInterview();
  if( interview )
  {
    Alder::Image *image = app->GetActiveAtlasImage();
    if( image )
    {
      vtkSmartPointer< Alder::Exam > exam;
      if( image->GetRecord( exam ) )
      {
        noteString = exam->Get( "Note" ).ToString().c_str();
        interviewerString = exam->Get( "Interviewer" ).ToString().c_str();
        siteString = interview->Get( "Site" ).ToString().c_str();
        dateString = exam->Get( "DatetimeAcquired" ).ToString().c_str();
        uidString = ""; // ->Get( "UId" );

        vtkSmartPointer< Alder::Modality > modality;
        exam->GetRecord( modality );
        helpString = modality->Get( "Help" ).ToString();
      }
    }
  }

  this->ui->helpTextEdit->setPlainText( helpString );
  this->ui->noteTextEdit->setPlainText( noteString );
  this->ui->infoInterviewerValueLabel->setText( interviewerString );
  this->ui->infoSiteValueLabel->setText( siteString );
  this->ui->infoDateValueLabel->setText( dateString );
  this->ui->infoUIdValueLabel->setText( uidString );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderAtlasWidget::updateInterface()
{
  Alder::Application *app = Alder::Application::GetInstance();
  Alder::Image *image = app->GetActiveAtlasImage();

  // set all widget enable states
  this->ui->previousPushButton->setEnabled( image );
  this->ui->nextPushButton->setEnabled( image );
  this->ui->noteTextEdit->setEnabled( image );
  this->ui->helpTextEdit->setEnabled( image );
  this->ui->ratingComboBox->setEnabled( image );

  this->updateInfo();
}
