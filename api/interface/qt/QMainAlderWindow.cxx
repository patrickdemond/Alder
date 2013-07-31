/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QMainAlderWindow.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include "QMainAlderWindow.h"
#include "ui_QMainAlderWindow.h"

#include "Application.h"
#include "Exam.h"
#include "Image.h"
#include "Interview.h"
#include "Modality.h"
#include "Rating.h"
#include "User.h"

#include "vtkMedicalImageViewer.h"
#include "vtkNew.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"

#include "QAboutDialog.h"
#include "QLoginDialog.h"
#include "QProgressDialog.h"
#include "QSelectInterviewDialog.h"
#include "QUserListDialog.h"

#include <QCloseEvent>
#include <QInputDialog>
#include <QMessageBox>
#include <QSettings>

#include <stdexcept>

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QMainAlderWindow::QMainAlderWindow( QWidget* parent )
  : QMainWindow( parent )
{
  Alder::Application *app = Alder::Application::GetInstance();
  QMenu *menu;
  this->atlasVisible = false;
  
  this->ui = new Ui_QMainAlderWindow;
  this->ui->setupUi( this );
  
  // connect the menu items
  QObject::connect(
    this->ui->actionOpenInterview, SIGNAL( triggered() ),
    this, SLOT( slotOpenInterview() ) );
  QObject::connect(
    this->ui->actionShowAtlas, SIGNAL( triggered() ),
    this, SLOT( slotShowAtlas() ) );
  QObject::connect(
    this->ui->actionLogin, SIGNAL( triggered() ),
    this, SLOT( slotLogin() ) );
  QObject::connect(
    this->ui->actionUserManagement, SIGNAL( triggered() ),
    this, SLOT( slotUserManagement() ) );
  QObject::connect(
    this->ui->actionUpdateDatabase, SIGNAL( triggered() ),
    this, SLOT( slotUpdateDatabase() ) );
  QObject::connect(
    this->ui->actionExit, SIGNAL( triggered() ),
    qApp, SLOT( closeAllWindows() ) );
  
  // connect the help menu items
  QObject::connect(
    this->ui->actionAbout, SIGNAL( triggered() ),
    this, SLOT( slotAbout() ) );
  QObject::connect(
    this->ui->actionManual, SIGNAL( triggered() ),
    this, SLOT( slotManual() ) );

  // connect the interview widget signals
  QObject::connect(
    this->ui->interviewWidget, SIGNAL( activeInterviewChanged() ),
    this, SLOT( slotActiveInterviewChanged() ) );
  QObject::connect(
    this->ui->interviewWidget, SIGNAL( activeImageChanged() ),
    this, SLOT( slotImageChanged() ) );

  QObject::connect(
    this->ui->atlasPreviousPushButton, SIGNAL( clicked() ),
    this, SLOT( slotPreviousAtlasImage() ) );
  QObject::connect(
    this->ui->atlasNextPushButton, SIGNAL( clicked() ),
    this, SLOT( slotNextAtlasImage() ) );
  QObject::connect(
    this->ui->atlasRatingComboBox, SIGNAL( currentIndexChanged ( int ) ),
    this, SLOT( slotAtlasRatingChanged( int ) ) );

  this->InterviewViewer = vtkSmartPointer<vtkMedicalImageViewer>::New();
  vtkRenderWindow* interviewRenwin = this->ui->interviewImageWidget->GetRenderWindow();
  vtkRenderer* interviewRenderer = this->InterviewViewer->GetRenderer();
  interviewRenderer->GradientBackgroundOn();
  interviewRenderer->SetBackground( 0, 0, 0 );
  interviewRenderer->SetBackground2( 0, 0, 1 );
  this->InterviewViewer->SetRenderWindow( interviewRenwin );
  this->InterviewViewer->InterpolateOff();
  this->InterviewViewer->SetImageToSinusoid();
    
  this->AtlasViewer = vtkSmartPointer<vtkMedicalImageViewer>::New();
  vtkRenderWindow* atlasRenwin = this->ui->atlasImageWidget->GetRenderWindow();
  vtkRenderer* atlasRenderer = this->AtlasViewer->GetRenderer();
  atlasRenderer->GradientBackgroundOn();
  atlasRenderer->SetBackground( 0, 0, 0 );
  atlasRenderer->SetBackground2( 0, 0, 1 );
  this->AtlasViewer->SetRenderWindow( atlasRenwin );
  this->AtlasViewer->InterpolateOff();
  this->AtlasViewer->SetImageToSinusoid();
  this->ui->atlasImageWidget->setParent( NULL ); // initial state is invisible
  
  this->ui->framePlayerWidget->setViewer( this->InterviewViewer );
  this->setCorner( Qt::BottomLeftCorner, Qt::BottomDockWidgetArea );
  this->setCorner( Qt::BottomRightCorner, Qt::RightDockWidgetArea );

  this->readSettings();
  this->updateInterface();
};

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QMainAlderWindow::~QMainAlderWindow()
{
  this->ui->framePlayerWidget->play(false);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::closeEvent( QCloseEvent *event )
{
  this->ui->framePlayerWidget->setViewer( 0 );
  this->writeSettings();
  event->accept();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::slotOpenInterview()
{
  bool loggedIn = NULL != Alder::Application::GetInstance()->GetActiveUser();

  if( loggedIn )
  {
    QSelectInterviewDialog dialog( this );
    dialog.setModal( true );
    dialog.setWindowTitle( tr( "Select Interview" ) );
    dialog.exec();

    // update the interview's exams
    Alder::Application *app = Alder::Application::GetInstance();
    Alder::Interview *activeInterview = app->GetActiveInterview();
    if( activeInterview && !activeInterview->HasImageData() )
    {
      // create a progress dialog to observe the progress of the update
      QProgressDialog dialog( this );
      dialog.setModal( true );
      dialog.setWindowTitle( tr( "Downloading Exam Images" ) );
      dialog.setMessage( tr( "Please wait while the interview's images are downloaded." ) );
      dialog.open();
      activeInterview->UpdateImageData();
      dialog.accept();
    }

    // active interview may have changed so update the interface
    this->updateInterface();
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::slotLogin()
{
  bool loggedIn = NULL != Alder::Application::GetInstance()->GetActiveUser();

  if( loggedIn )
  {
    Alder::Application::GetInstance()->ResetApplication();
  }
  else
  {
    QLoginDialog dialog( this );
    dialog.setModal( true );
    dialog.setWindowTitle( tr( "Login" ) );
    dialog.exec();
  }

  // active user may have changed so update the interface
  this->updateInterface();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::slotShowAtlas()
{
  this->atlasVisible = !this->atlasVisible;

  Alder::Application *app = Alder::Application::GetInstance();
  Alder::Image *atlasImage = app->GetActiveAtlasImage();

  if( this->atlasVisible )
  {
    Alder::Image *image = app->GetActiveImage();

    // select an appropriate atlas image, if necessary
    if( image )
    {
      int rating = this->ui->atlasRatingComboBox->currentIndex() + 1;
      bool getNewAtlasImage = false;

      vtkSmartPointer< Alder::Exam > exam;
      image->GetRecord( exam );

      if( NULL == atlasImage ) getNewAtlasImage = true;
      else
      {
        vtkSmartPointer< Alder::Exam > atlasExam;
        atlasImage->GetRecord( atlasExam );
        if( exam->Get( "Type" ) != atlasExam->Get( "Type" ) ) getNewAtlasImage = true;
      }

      if( getNewAtlasImage )
      {
        vtkSmartPointer<Alder::Image> newAtlasImage = 
          Alder::Image::GetAtlasImage( exam->Get( "Type" ).ToString(), rating );
        if( 0 < newAtlasImage->Get( "Id" ).ToInt() ) app->SetActiveAtlasImage( newAtlasImage );
      }
    }

    // add the widget to the splitter
    this->ui->imageWidgetSplitter->insertWidget( 0, this->ui->atlasImageWidget );

    QList<int> sizeList = this->ui->imageWidgetSplitter->sizes();
    int total = sizeList[0] + sizeList[1];
    sizeList[0] = floor( total / 2 );
    sizeList[1] = sizeList[0];
    this->ui->imageWidgetSplitter->setSizes( sizeList );
  }
  else if( !this->atlasVisible )
  {
    // remove the widget from the splitter
    this->ui->atlasImageWidget->setParent( NULL );
    app->SetActiveAtlasImage( NULL );
  }

  this->updateInterface();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::slotUserManagement()
{
  int attempt = 1;

  while( attempt < 4 )
  {
    // check for admin password
    QString text = QInputDialog::getText(
      this,
      QObject::tr( "User Management" ),
      QObject::tr( attempt > 1 ? "Wrong password, try again:" : "Administrator password:" ),
      QLineEdit::Password );
    
    // do nothing if the user hit the cancel button
    if( text.isEmpty() ) break;

    vtkNew< Alder::User > user;
    user->Load( "Name", "administrator" );
    if( user->IsPassword( text.toStdString().c_str() ) )
    {
      // load the users dialog
      QUserListDialog usersDialog( this );
      usersDialog.setModal( true );
      usersDialog.setWindowTitle( tr( "User Management" ) );
      usersDialog.exec();
      break;
    }
    attempt++;
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::slotUpdateDatabase()
{
  int attempt = 1;

  while( attempt < 4 )
  {
    // check for admin password
    QString text = QInputDialog::getText(
      this,
      QObject::tr( "User Management" ),
      QObject::tr( attempt > 1 ? "Wrong password, try again:" : "Administrator password:" ),
      QLineEdit::Password );
    
    // do nothing if the user hit the cancel button
    if( text.isEmpty() ) break;

    vtkNew< Alder::User > user;
    user->Load( "name", "administrator" );
    if( user->IsPassword( text.toStdString().c_str() ) )
    {
      // create a progress dialog to observe the progress of the update
      QProgressDialog dialog( this );
      dialog.setModal( true );
      dialog.setWindowTitle( tr( "Updating Database" ) );
      dialog.setMessage( tr( "Please wait while the database is updated." ) );
      dialog.open();
      Alder::Interview::UpdateInterviewData();
      dialog.accept();
      break;
    }
    attempt++;
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::slotActiveInterviewChanged()
{
  this->updateInterface();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::slotActiveImageChanged()
{
  this->updateInterface();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::slotAbout()
{
  QAboutDialog dialog( this );
  dialog.setModal( true );
  dialog.setWindowTitle( tr( "About Alder" ) );
  dialog.exec();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::slotManual()
{
  // TODO: open link to Alder manual
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::readSettings()
{
  QSettings settings( "CLSA", "Alder" );
  
  settings.beginGroup( "MainAlderWindow" );
  if( settings.contains( "size" ) ) this->resize( settings.value( "size" ).toSize() );
  if( settings.contains( "pos" ) ) this->move( settings.value( "pos" ).toPoint() );
  if( settings.contains( "maximized" ) && settings.value( "maximized" ).toBool() )
    this->showMaximized();
  settings.endGroup();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::slotAtlasRatingChanged( int index )
{
  int rating = index + 1;

  // TODO: implement
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::slotPreviousAtlasImage()
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
void QMainAlderWindow::slotNextAtlasImage()
{
  Alder::Application *app = Alder::Application::GetInstance();
  Alder::Image *image = app->GetActiveAtlasImage();
  
  if( NULL != image )
  {
    app->SetActiveAtlasImage( image->GetNextAtlasImage() );
    this->updateInterface();
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::writeSettings()
{
  QSettings settings( "CLSA", "Alder" );
  
  settings.beginGroup( "MainAlderWindow" );
  settings.setValue( "size", this->size() );
  settings.setValue( "pos", this->pos() );
  settings.setValue( "maximized", this->isMaximized() );
  settings.endGroup();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::updateInterviewImageWidget()
{
  Alder::Image *image = Alder::Application::GetInstance()->GetActiveImage();

  if( image ) this->InterviewViewer->Load( image->GetFileName().c_str() );
  else this->InterviewViewer->SetImageToSinusoid();

  this->ui->framePlayerWidget->updateFromViewer();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::updateAtlasInformation()
{
  QString helpString = "";
  QString noteString = "";
  QString uidString = tr( "N/A" );
  QString interviewerString = tr( "N/A" );
  QString siteString = tr( "N/A" );
  QString dateString = tr( "N/A" );

  // fill in the active exam information
  if( this->atlasVisible )
  {
    // get exam from active atlas image
    Alder::Image *atlasImage = Alder::Application::GetInstance()->GetActiveAtlasImage();

    if( atlasImage )
    {
      vtkSmartPointer< Alder::Exam > atlasExam;
      if( atlasImage->GetRecord( atlasExam ) )
      {
        vtkSmartPointer< Alder::Interview > interview;
        if( atlasExam->GetRecord( interview ) ) uidString = interview->Get( "UId" ).ToString().c_str();
        noteString = atlasExam->Get( "Note" ).ToString().c_str();
        interviewerString = atlasExam->Get( "Interviewer" ).ToString().c_str();
        siteString = interview->Get( "Site" ).ToString().c_str();
        dateString = atlasExam->Get( "DatetimeAcquired" ).ToString().c_str();
      }
    }

    // get the modality help text based on the current image's exam's modality
    Alder::Image *image = Alder::Application::GetInstance()->GetActiveAtlasImage();

    if( image )
    {
      vtkSmartPointer< Alder::Exam > exam;
      if( image->GetRecord( exam ) )
      {
        vtkSmartPointer< Alder::Modality > modality;
        exam->GetRecord( modality );
        helpString = modality->Get( "Help" ).ToString();
      }
    }
    
  }

  this->ui->atlasHelpTextEdit->setPlainText( helpString );
  this->ui->atlasNoteTextEdit->setPlainText( noteString );
  this->ui->atlasUIdValueLabel->setText( uidString );
  this->ui->atlasInterviewerValueLabel->setText( interviewerString );
  this->ui->atlasSiteValueLabel->setText( siteString );
  this->ui->atlasDateValueLabel->setText( dateString );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::updateAtlasImageWidget()
{
  Alder::Image *image = Alder::Application::GetInstance()->GetActiveAtlasImage();

  if( image ) this->AtlasViewer->Load( image->GetFileName().c_str() );
  else this->AtlasViewer->SetImageToSinusoid();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::updateInterface()
{
  Alder::Application *app = Alder::Application::GetInstance();
  Alder::Image *atlasImage = app->GetActiveAtlasImage();
  bool loggedIn = NULL != app->GetActiveUser();

  // dynamic menu action names
  this->ui->actionLogin->setText( tr( loggedIn ? "Logout" : "Login" ) );
  this->ui->actionShowAtlas->setText( tr( this->atlasVisible ? "Hide Atlas" : "Show Atlas" ) );

  // display or hide the image atlas widgets
  this->ui->atlasDockWidget->setVisible( this->atlasVisible );
  this->ui->interviewDockWidget->setVisible( !this->atlasVisible );

  // set all widget enable states
  this->ui->actionOpenInterview->setEnabled( loggedIn );
  this->ui->actionShowAtlas->setEnabled( loggedIn );

  this->ui->atlasRatingComboBox->setEnabled( atlasImage );
  this->ui->atlasPreviousPushButton->setEnabled( atlasImage );
  this->ui->atlasNextPushButton->setEnabled( atlasImage );
  this->ui->atlasHelpTextEdit->setEnabled( this->atlasVisible );
  this->ui->atlasNoteTextEdit->setEnabled( atlasImage && this->atlasVisible );

  this->ui->framePlayerWidget->setEnabled( loggedIn );
  this->ui->imageWidgetSplitter->setEnabled( loggedIn );
  this->ui->interviewImageWidget->setEnabled( loggedIn );
  this->ui->atlasImageWidget->setEnabled( loggedIn );

  this->updateInterviewImageWidget();

  if( this->atlasVisible )
  {
    this->updateAtlasInformation();
    this->updateAtlasImageWidget();
  }

  this->ui->interviewWidget->updateInterface();
}
