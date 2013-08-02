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
  this->atlasVisible = true; // this will be toggled to false at the end of this method
  
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
    this, SLOT( this->ui->atlasWidget->slotActiveInterviewChanged() ) );
  QObject::connect(
    this->ui->interviewWidget, SIGNAL( activeImageChanged() ),
    this, SLOT( this->ui->atlasWidget->slotActiveImageChanged() ) );

  // TODO: need to connect atlasWidget to the frame player as well
  this->ui->framePlayerWidget->setViewer( this->ui->interviewWidget->GetViewer() );

  this->readSettings();
  this->slotShowAtlas(); // this actually hides the atlas
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QMainAlderWindow::~QMainAlderWindow()
{
  this->ui->framePlayerWidget->play( false );
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
  Alder::Application *app = Alder::Application::GetInstance();
  bool loggedIn = NULL != app->GetActiveUser();

  if( loggedIn )
  {
    QSelectInterviewDialog dialog( this );
    dialog.setModal( true );
    dialog.setWindowTitle( tr( "Select Interview" ) );
    dialog.exec();

    // update the interview's exams
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
  Alder::Application *app = Alder::Application::GetInstance();
  bool loggedIn = NULL != app->GetActiveUser();

  if( loggedIn )
  {
    app->ResetApplication();
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

  if( this->atlasVisible )
  {
    // add the widget to the splitter
    this->ui->splitter->insertWidget( 0, this->ui->atlasWidget );
    this->ui->atlasWidget->setVisible( true );

    QList<int> sizeList = this->ui->splitter->sizes();
    int total = sizeList[0] + sizeList[1];
    sizeList[0] = floor( total / 2 );
    sizeList[1] = sizeList[0];
    this->ui->splitter->setSizes( sizeList );
  }
  else if( !this->atlasVisible )
  {
    // remove the widget from the splitter
    this->ui->atlasWidget->setVisible( false );
    this->ui->atlasWidget->setParent( this );

    Alder::Application::GetInstance()->SetActiveAtlasImage( NULL );
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
  this->updateInterviewImageWidget();
  //TODO: if the current interview image changes, then the atlas
  // should change accordingly too
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

  this->ui->framePlayerWidget->setEnabled( loggedIn );
  this->ui->splitter->setEnabled( loggedIn );

  //TODO: interviewImageWidget probably only needs to self->updateInterface
  // when setEnabled emits a signal
  this->ui->interviewImageWidget->setEnabled( loggedIn );
  this->ui->atlasImageWidget->setEnabled( loggedIn );

  this->updateInterviewImageWidget();

  if( this->atlasVisible )
  {
    this->updateAtlasImageWidget();
  }

  this->ui->interviewWidget->updateInterface();
  this->ui->atlasWidget->updateInterface();
}
