/*=========================================================================

  Program:  Alder (CLSA Ultrasound Image Viewer)
  Module:   QMainAlderWindow.cxx
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/
#include "QMainAlderWindow.h"
#include "ui_QMainAlderWindow.h"

#include "Application.h"
#include "Cineloop.h"
#include "Image.h"
#include "Exam.h"
#include "Study.h"
#include "User.h"

#include "vtkMedicalImageViewer.h"

#include "QAboutDialog.h"
#include "QLoginDialog.h"
#include "QProgressDialog.h"
#include "QSelectStudyDialog.h"
#include "QUserListDialog.h"

#include <QCloseEvent>
#include <QInputDialog>
#include <QMessageBox>
#include <QSettings>
#include <QTreeWidgetItem>

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QMainAlderWindow::QMainAlderWindow( QWidget* parent )
  : QMainWindow( parent )
{
  Alder::Application *app = Alder::Application::GetInstance();
  QMenu *menu;
  
  this->ui = new Ui_QMainAlderWindow;
  this->ui->setupUi( this );
  
  // set up child widgets
  this->ui->studyTreeWidget->header()->hide();

  // connect the menu items
  QObject::connect(
    this->ui->actionOpenStudy, SIGNAL( triggered() ),
    this, SLOT( slotOpenStudy() ) );
  QObject::connect(
    this->ui->actionPreviousStudy, SIGNAL( triggered() ),
    this, SLOT( slotPreviousStudy() ) );
  QObject::connect(
    this->ui->actionNextStudy, SIGNAL( triggered() ),
    this, SLOT( slotNextStudy() ) );
  QObject::connect(
    this->ui->actionLogin, SIGNAL( triggered() ),
    this, SLOT( slotLogin() ) );
  QObject::connect(
    this->ui->actionUserManagement, SIGNAL( triggered() ),
    this, SLOT( slotUserManagement() ) );
  QObject::connect(
    this->ui->actionUpdateStudyDatabase, SIGNAL( triggered() ),
    this, SLOT( slotUpdateStudyDatabase() ) );
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
    this->ui->studyTreeWidget, SIGNAL( itemSelectionChanged() ),
    this, SLOT( slotTreeSelectionChanged() ) );

  // link the view and the qt render widget
  app->GetViewer()->SetInteractor( this->ui->renderWidget->GetInteractor() );
  this->ui->renderWidget->SetRenderWindow( app->GetViewer()->GetRenderWindow() );

  this->readSettings();
  this->updateInterface();
};

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QMainAlderWindow::~QMainAlderWindow()
{
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::closeEvent( QCloseEvent *event )
{
  this->writeSettings();
  event->accept();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::slotOpenStudy()
{
  bool loggedIn = NULL != Alder::Application::GetInstance()->GetActiveUser();

  if( loggedIn )
  {
    QSelectStudyDialog dialog( this );
    dialog.setModal( true );
    dialog.setWindowTitle( tr( "Select Study" ) );
    dialog.exec();

    // active study may have changed so update the interface
    this->updateInterface();
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::slotPreviousStudy()
{
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::slotNextStudy()
{
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

    vtkSmartPointer< Alder::User > user = vtkSmartPointer< Alder::User >::New();
    user->Load( "name", "administrator" );
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
void QMainAlderWindow::slotUpdateStudyDatabase()
{
/*
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

    vtkSmartPointer< Alder::User > user = vtkSmartPointer< Alder::User >::New();
    user->Load( "name", "administrator" );
    if( user->IsPassword( text.toStdString().c_str() ) )
    {
      // create a progress dialog to observe the progress of the update
      QProgressDialog dialog( this );
      dialog.setModal( true );
      dialog.setWindowTitle( tr( "Updating Study Database" ) );
      dialog.setMessage( tr( "Please wait while the study database is updated." ) );
      dialog.open();
      Alder::Study::UpdateData();
      dialog.accept();
      break;
    }
    attempt++;
  }
*/
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
void QMainAlderWindow::slotTreeSelectionChanged()
{
  this->updateStudyInformation();
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
void QMainAlderWindow::updateStudyInformation()
{
  QString interviewerString = tr( "N/A" );
  QString siteString = tr( "N/A" );
  QString dateString = tr( "N/A" );
  QString minString = tr( "N/A" );
  QString maxString = tr( "N/A" );
  QString meanString = tr( "N/A" );
  QString sdString = tr( "N/A" );
  QString nString = tr( "N/A" );

  // fill in the active study information
  Alder::Study *study = Alder::Application::GetInstance()->GetActiveStudy();
  if( study )
  {
    interviewerString = study->Get( "interviewer" )->ToString().c_str();
    siteString = study->Get( "site" )->ToString().c_str();
    dateString = study->Get( "datetime_acquired" )->ToString().c_str();
  }

  // define currently selected item
  QList<QTreeWidgetItem*> list = this->ui->studyTreeWidget->selectedItems();

  if( 0 < list.size() )
  {
    std::map< QTreeWidgetItem*, vtkSmartPointer<Alder::ActiveRecord> >::iterator it;
    it = this->treeModelMap.find( list.at( 0 ) );
    if( it != this->treeModelMap.end() )
    {
      Alder::ActiveRecord *record = it->second;
      if( 0 == record->GetName().compare( "Image" ) )
      {
        if( record->Get( "min" ) ) minString = record->Get( "min" )->ToString().c_str();
        if( record->Get( "max" ) ) maxString = record->Get( "max" )->ToString().c_str();
        if( record->Get( "mean" ) ) meanString = record->Get( "mean" )->ToString().c_str();
        if( record->Get( "sd" ) ) sdString = record->Get( "sd" )->ToString().c_str();
        if( record->Get( "n" ) ) nString = record->Get( "n" )->ToString().c_str();
      }
    }
  }

  this->ui->interviewerValueLabel->setText( interviewerString );
  this->ui->siteValueLabel->setText( siteString );
  this->ui->dateValueLabel->setText( dateString );
  this->ui->minValueLabel->setText( minString );
  this->ui->maxValueLabel->setText( maxString );
  this->ui->meanValueLabel->setText( meanString );
  this->ui->sdValueLabel->setText( sdString );
  this->ui->nValueLabel->setText( nString );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::updateStudyTreeWidget()
{
  Alder::Study *study = Alder::Application::GetInstance()->GetActiveStudy();

  // if a study is open then populate the study tree
  this->treeModelMap.clear();
  this->ui->studyTreeWidget->clear();
  if( study )
  {
    // make root the study's UID
    QString name = tr( "Study: " );
    name += study->Get( "uid" )->ToString().c_str();
    QTreeWidgetItem *root = new QTreeWidgetItem( this->ui->studyTreeWidget );
    root->setText( 0, name );
    root->setExpanded( true );
    root->setFlags( Qt::ItemIsEnabled );
    this->ui->studyTreeWidget->addTopLevelItem( root );

    // make each exam a child of the root
    std::vector< vtkSmartPointer< Alder::Exam > > examList;
    std::vector< vtkSmartPointer< Alder::Exam > >::iterator examIt;
    study->GetList( &examList );
    for( examIt = examList.begin(); examIt != examList.end(); ++examIt )
    {
      Alder::Exam *exam = examIt->GetPointer();
      name = tr( "Exam: " );
      name += exam->Get( "laterality" )->ToString().c_str();
      QTreeWidgetItem *examItem = new QTreeWidgetItem( root );
      this->treeModelMap[examItem] = *examIt;
      examItem->setText( 0, name );
      examItem->setExpanded( true );
      examItem->setFlags( Qt::ItemIsEnabled );

      // add the cineloops for this exam
      std::vector< vtkSmartPointer< Alder::Cineloop > > cineloopList;
      std::vector< vtkSmartPointer< Alder::Cineloop > >::iterator cineloopIt;
      exam->GetList( &cineloopList );
      for( cineloopIt = cineloopList.begin(); cineloopIt != cineloopList.end(); ++cineloopIt )
      {
        Alder::Cineloop *cineloop = cineloopIt->GetPointer();
        name = tr( "Cineloop #" );
        name += cineloop->Get( "number" )->ToString().c_str();
        QTreeWidgetItem *cineloopItem = new QTreeWidgetItem( examItem );
        this->treeModelMap[cineloopItem] = *cineloopIt;
        cineloopItem->setText( 0, name );
        cineloopItem->setExpanded( true );
        cineloopItem->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );

        // add the images for this cineloop
        std::vector< vtkSmartPointer< Alder::Image > > imageList;
        std::vector< vtkSmartPointer< Alder::Image > >::iterator imageIt;
        cineloop->GetList( &imageList );
        for( imageIt = imageList.begin(); imageIt != imageList.end(); ++imageIt )
        {
          Alder::Image *image = imageIt->GetPointer();
          name = tr( "Frame #" );
          name += image->Get( "frame" )->ToString().c_str();
          QTreeWidgetItem *imageItem = new QTreeWidgetItem( cineloopItem );
          this->treeModelMap[imageItem] = *imageIt;
          imageItem->setText( 0, name );
          imageItem->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
        }
      }
    }
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::updateInterface()
{
  Alder::Application *app = Alder::Application::GetInstance();
  Alder::Study *study = app->GetActiveStudy();
  bool loggedIn = NULL != app->GetActiveUser();

  // login button (login/logout)
  this->ui->actionLogin->setText( tr( loggedIn ? "Logout" : "Login" ) );

  // only allow study operations when logged in
  this->ui->actionOpenStudy->setEnabled( loggedIn );
  this->ui->actionPreviousStudy->setEnabled( loggedIn );
  this->ui->actionNextStudy->setEnabled( loggedIn );
  this->ui->previousStudyPushButton->setEnabled( loggedIn );
  this->ui->nextStudyPushButton->setEnabled( loggedIn );
  this->ui->addImagePushButton->setEnabled( false );
  this->ui->removeImagePushButton->setEnabled( false );
  this->ui->ratingSlider->setEnabled( false );
  this->ui->notePushButton->setEnabled( false );
  this->ui->studyTreeWidget->setEnabled( loggedIn );

  this->updateStudyTreeWidget();
  this->updateStudyInformation();
}
