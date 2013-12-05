/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QMainAlderWindow.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include <QMainAlderWindow.h>
#include <ui_QMainAlderWindow.h>

#include <Application.h>
#include <Interview.h>
#include <User.h>

#include <vtkEventQtSlotConnect.h>
#include <vtkMedicalImageViewer.h>
#include <vtkNew.h>

#include <QAboutDialog.h>
#include <QAlderDicomTagWidget.h>
#include <QChangePasswordDialog.h>
#include <QLoginDialog.h>
#include <QSelectInterviewDialog.h>
#include <QUserListDialog.h>
#include <QVTKProgressDialog.h>

#include <QCloseEvent>
#include <QInputDialog>
#include <QMessageBox>
#include <QSettings>

#include <stdexcept>

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QMainAlderWindow::QMainAlderWindow( QWidget* parent )
  : QMainWindow( parent )
{
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
    this->ui->actionShowDicomTags, SIGNAL( triggered() ),
    this, SLOT( slotShowDicomTags() ) );
  QObject::connect(
    this->ui->actionLogin, SIGNAL( triggered() ),
    this, SLOT( slotLogin() ) );
  QObject::connect(
    this->ui->actionChangePassword, SIGNAL( triggered() ),
    this, SLOT( slotChangePassword() ) );
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

  QObject::connect( 
    this->ui->atlasWidget, SIGNAL( showing( bool ) ),
    this->ui->interviewWidget, SLOT( slotHideControls( bool ) ) );

  // TODO: need to connect atlasWidget to the frame player as well
  this->ui->framePlayerWidget->setViewer( this->ui->interviewWidget->GetViewer() );

  this->Connections = vtkSmartPointer<vtkEventQtSlotConnect>::New();
  this->Connections->Connect( Alder::Application::GetInstance(),
    Alder::Application::ActiveImageEvent,
    this->ui->framePlayerWidget, SLOT( updateFromViewer() ) );

  this->readSettings();

  // toggle visibility of the atlas widget
  this->atlasVisible = true;
  this->slotShowAtlas();
  this->DicomTagWidget = new QAlderDicomTagWidget( this );
  this->Connections->Connect( Alder::Application::GetInstance(),
    Alder::Application::ActiveImageEvent,
    this, SLOT( updateDicomTagWidget() ) );

  this->dicomTagsVisible = false;
  this->DicomTagWidget->hide();

  this->updateInterface();
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
      QVTKProgressDialog dialog( this );
      dialog.setModal( true );
      dialog.setWindowTitle( tr( "Downloading Exam Images" ) );
      dialog.setMessage( tr( "Please wait while the interview's images are downloaded." ) );
      dialog.open();
      app->UpdateActiveInterviewImageData();
      dialog.accept();
    }
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
  bool lastVisible = this->atlasVisible;

  if( !lastVisible && this->dicomTagsVisible ) this->slotShowDicomTags();

  this->atlasVisible = !this->atlasVisible;
  this->ui->actionShowAtlas->setText( tr( this->atlasVisible ? "Hide Atlas" : "Show Atlas" ) );

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

    this->ui->atlasWidget->updateViewer();
  }
  else
  {
    // remove the widget from the splitter
    this->ui->atlasWidget->setVisible( false );
    this->ui->atlasWidget->setParent( this );

    Alder::Application::GetInstance()->SetActiveAtlasImage( NULL );
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::slotShowDicomTags()
{
  if( this->atlasVisible ) return;

  this->dicomTagsVisible = !this->dicomTagsVisible;

  this->ui->actionShowDicomTags->setText( 
    tr( this->dicomTagsVisible ? "Hide Dicom Tags" : "Show Dicom Tags" ) );

  if( this->dicomTagsVisible )
  {
    this->ui->splitter->insertWidget( 0, qobject_cast<QWidget*>(this->DicomTagWidget));
    this->DicomTagWidget->show();
  }
  else
  {
    this->DicomTagWidget->hide();
    this->DicomTagWidget->setParent( this );
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::slotUserManagement()
{
  this->adminLoginDo( &QMainAlderWindow::adminUserManagement );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::slotUpdateDatabase()
{
  this->adminLoginDo( &QMainAlderWindow::adminUpdateDatabase );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::adminLoginDo( void (QMainAlderWindow::*fn)() )
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
      user->Set( "LastLogin", Alder::Utilities::getTime( "%Y-%m-%d %H:%M:%S" ) );
      user->Save();

      (this->*fn)();

      break;
    }
    attempt++;
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::adminUpdateDatabase()
{
  // create a progress dialog to observe the progress of the update
  QVTKProgressDialog dialog( this );
  dialog.setModal( true );
  dialog.setWindowTitle( tr( "Updating Database" ) );
  dialog.setMessage( tr( "Please wait while the database is updated." ) );
  dialog.open();
  Alder::Interview::UpdateInterviewData();
  dialog.accept();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::adminUserManagement( )
{
  // load the users dialog
  QUserListDialog usersDialog( this );
  usersDialog.setModal( true );
  usersDialog.setWindowTitle( tr( "User Management" ) );

  Alder::Application *app = Alder::Application::GetInstance();
  if(  NULL != app->GetActiveUser() )
  {
    QObject::connect( 
      &usersDialog , SIGNAL( userModalityChanged() ), 
     this->ui->interviewWidget, SLOT( updateExamTreeWidget() ));
  }   
  usersDialog.exec();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::slotChangePassword()
{
  Alder::Application *app = Alder::Application::GetInstance();
  Alder::User* user;
  if(  NULL != ( user = app->GetActiveUser() ) )
  {
    QString password = user->Get("Password").ToString().c_str();
    QChangePasswordDialog dialog( password, this );
    dialog.setModal( true );
    QObject::connect( 
      &dialog , SIGNAL( passwordChange( QString ) ), 
     this, SLOT( changeActiveUserPassword( QString ) ));
    dialog.exec();
  }   
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::changeActiveUserPassword( QString password )
{
  if( password.isEmpty() ) return;
  Alder::Application *app = Alder::Application::GetInstance();
  Alder::User* user;
  if(  NULL != ( user = app->GetActiveUser() ) )
  {
    user->Set( "Password", password.toStdString() );
    user->Save();
  }
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
void QMainAlderWindow::updateInterface()
{
  Alder::Application *app = Alder::Application::GetInstance();
  bool loggedIn = NULL != app->GetActiveUser();

  // dynamic menu action names
  this->ui->actionLogin->setText( tr( loggedIn ? "Logout" : "Login" ) );

  // set all widget enable states
  this->ui->actionOpenInterview->setEnabled( loggedIn );
  this->ui->actionChangePassword->setEnabled( loggedIn );
  this->ui->actionShowAtlas->setEnabled( loggedIn );

  this->ui->framePlayerWidget->setEnabled( loggedIn );
  this->ui->splitter->setEnabled( loggedIn );

  this->ui->interviewWidget->setEnabled( loggedIn );
  this->ui->interviewWidget->updateEnabled();
  this->ui->atlasWidget->setEnabled( loggedIn );
  this->ui->atlasWidget->updateEnabled();

  this->ui->actionShowDicomTags->setEnabled( loggedIn );
  this->DicomTagWidget->setEnabled( loggedIn );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::updateDicomTagWidget()
{
  Alder::Application *app = Alder::Application::GetInstance();
  Alder::Image *image = app->GetActiveImage();
  QString fileName = image ? image->GetFileName().c_str() : "";
  this->DicomTagWidget->updateTableWidget( fileName );
}
