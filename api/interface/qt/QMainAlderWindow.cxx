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
#include "Exam.h"
#include "Image.h"
#include "Rating.h"
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

#include <stdexcept>

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
    this->ui->previousStudyPushButton, SIGNAL( clicked() ),
    this, SLOT( slotPreviousStudy() ) );
  QObject::connect(
    this->ui->nextStudyPushButton, SIGNAL( clicked() ),
    this, SLOT( slotNextStudy() ) );
  QObject::connect(
    this->ui->studyTreeWidget, SIGNAL( itemSelectionChanged() ),
    this, SLOT( slotTreeSelectionChanged() ) );
  QObject::connect(
    this->ui->ratingSlider, SIGNAL( valueChanged( int ) ),
    this, SLOT( slotRatingSliderChanged( int ) ) );

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
  bool found = false;
  Alder::Application *app = Alder::Application::GetInstance();
  Alder::User *user = app->GetActiveUser();
  Alder::Study *activeStudy = app->GetActiveStudy();
  vtkSmartPointer< Alder::Study > study;
  if( user && activeStudy )
  {
    // check if unrated checkbox is pressed, keep searching for an unrated study
    if( this->ui->unratedCheckBox->isChecked() )
    {
      int currentStudyId = activeStudy->Get( "id" )->ToInt();

      // keep getting the previous study until we find one that has images which are not rated
      study = activeStudy->GetPrevious();
      while( study->Get( "id" )->ToInt() != currentStudyId )
      {
        if( 0 < study->GetImageCount() && !study->IsRatedBy( user ) )
        {
          found = true;
          break;
        }
        study = study->GetPrevious();
      }

      // warn user if no unrated studies left
      if( study->Get( "id" )->ToInt() == currentStudyId )
      {
        QMessageBox errorMessage( this );
        errorMessage.setWindowModality( Qt::WindowModal );
        errorMessage.setIcon( QMessageBox::Warning );
        errorMessage.setText( tr( "There are no remaining unrated studies available at this time." ) );
        errorMessage.exec();
      }
    }
    else
    {
      study = activeStudy->GetPrevious();
      found = true;
    }
  }

  if( found )
  {
    app->SetActiveStudy( study );
    this->updateInterface();
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::slotNextStudy()
{
  bool found = false;
  Alder::Application *app = Alder::Application::GetInstance();
  Alder::User *user = app->GetActiveUser();
  Alder::Study *activeStudy = app->GetActiveStudy();
  vtkSmartPointer< Alder::Study > study;
  if( user && activeStudy )
  {
    // check if unrated checkbox is pressed, keep searching for an unrated study
    if( this->ui->unratedCheckBox->isChecked() )
    {
      int currentStudyId = activeStudy->Get( "id" )->ToInt();

      // keep getting the previous study until we find one that has images which are not rated
      study = activeStudy->GetNext();
      while( study->Get( "id" )->ToInt() != currentStudyId )
      {
        if( 0 < study->GetImageCount() && !study->IsRatedBy( user ) )
        {
          found = true;
          break;
        }
        study = study->GetNext();
      }

      // warn user if no unrated studies left
      if( study->Get( "id" )->ToInt() == currentStudyId )
      {
        QMessageBox errorMessage( this );
        errorMessage.setWindowModality( Qt::WindowModal );
        errorMessage.setIcon( QMessageBox::Warning );
        errorMessage.setText( tr( "There are no remaining unrated studies available at this time." ) );
        errorMessage.exec();
      }
    }
    else
    {
      study = activeStudy->GetNext();
      found = true;
    }
  }

  if( found )
  {
    app->SetActiveStudy( study );
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
/* TODO: implement once Opal is ready
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
  Alder::Application *app = Alder::Application::GetInstance();
  
  QList<QTreeWidgetItem*> list = this->ui->studyTreeWidget->selectedItems();
  if( 0 < list.size() )
  {
    std::map< QTreeWidgetItem*, vtkSmartPointer<Alder::ActiveRecord> >::iterator it;
    it = this->treeModelMap.find( list.at( 0 ) );
    if( it != this->treeModelMap.end() )
    {
      Alder::ActiveRecord *record = it->second;
      app->SetActiveImage( Alder::Image::SafeDownCast( record ) );
      app->SetActiveCineloop( Alder::Cineloop::SafeDownCast( record ) );
    }
  }

  this->updateInterface();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::slotRatingSliderChanged( int value )
{
  Alder::Application *app = Alder::Application::GetInstance();

  // make sure we have an active image
  Alder::User *user = app->GetActiveUser();
  if( !user ) throw std::runtime_error( "Rating slider modified without an active user" );
  Alder::Image *image = app->GetActiveImage();
  if( !image ) throw std::runtime_error( "Rating slider modified without an active image" );

  // See if we have a rating for this user and image
  std::map< std::string, std::string > map;
  map["user_id"] = user->Get( "id" )->ToString();
  map["image_id"] = image->Get( "id" )->ToString();
  vtkSmartPointer< Alder::Rating > rating = vtkSmartPointer< Alder::Rating >::New();
  if( !rating->Load( map ) )
  { // no record exists, set the user and image ids
    rating->Set( "user_id", user->Get( "id" )->ToInt() );
    rating->Set( "image_id", image->Get( "id" )->ToInt() );
  }

  if( 0 == value ) rating->SetNull( "rating" );
  else rating->Set( "rating", value );

  rating->Save();
  this->updateRating();
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

  // fill in the active image information (if one is selected)
  Alder::Image *image = Alder::Application::GetInstance()->GetActiveImage();
  if( image )
  {
    if( image->Get( "min" ) ) minString = image->Get( "min" )->ToString().c_str();
    if( image->Get( "max" ) ) maxString = image->Get( "max" )->ToString().c_str();
    if( image->Get( "mean" ) ) meanString = image->Get( "mean" )->ToString().c_str();
    if( image->Get( "sd" ) ) sdString = image->Get( "sd" )->ToString().c_str();
    if( image->Get( "n" ) ) nString = image->Get( "n" )->ToString().c_str();
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

  // stop the tree's signals until we are done
  bool oldSignalState = this->ui->studyTreeWidget->blockSignals( true );

  // if a study is open then populate the study tree
  this->treeModelMap.clear();
  this->ui->studyTreeWidget->clear();
  if( study )
  {
    // get the active image/cineloop so that we can highlight it
    Alder::Image *activeImage = Alder::Application::GetInstance()->GetActiveImage();
    Alder::Cineloop *activeCineloop = Alder::Application::GetInstance()->GetActiveCineloop();
    QTreeWidgetItem *selectedItem = NULL;

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
        if( activeCineloop && activeCineloop->Get( "id" )->ToInt() == cineloop->Get( "id" )->ToInt() )
          selectedItem = cineloopItem;

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
          if( activeImage && activeImage->Get( "id" )->ToInt() == image->Get( "id" )->ToInt() )
            selectedItem = imageItem;
        }
      }
    }
      
    if( selectedItem ) this->ui->studyTreeWidget->setCurrentItem( selectedItem );
  }

  // re-enable the tree's signals
  this->ui->studyTreeWidget->blockSignals( oldSignalState );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::updateMedicalImageWidget()
{
  Alder::Image *image = Alder::Application::GetInstance()->GetActiveImage();
  Alder::Cineloop *cineloop = Alder::Application::GetInstance()->GetActiveCineloop();

  if( image )
  {
    this->ui->medicalImageWidget->loadImage( QString( image->GetFileName().c_str() ) );
    // TODO: in some situations we may not want to display the static images
    // for example, when a reader should be blinded to any previous slice selections
  }
  else if( cineloop )
  {
    this->ui->medicalImageWidget->loadImage( QString( cineloop->GetFileName().c_str() ) );
  }
  else
  {
    this->ui->medicalImageWidget->resetImage();
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::updateRating()
{
  // stop the rating slider's signals until we are done
  bool oldSignalState = this->ui->ratingSlider->blockSignals( true );

  int ratingValue = 0;
  Alder::Application *app = Alder::Application::GetInstance();

  // make sure we have an active image
  Alder::User *user = app->GetActiveUser();
  Alder::Image *image = app->GetActiveImage();

  if( user && image )
  {
    std::map< std::string, std::string > map;
    map["user_id"] = user->Get( "id" )->ToString();
    map["image_id"] = image->Get( "id" )->ToString();
    vtkSmartPointer< Alder::Rating > rating = vtkSmartPointer< Alder::Rating >::New();
    
    if( rating->Load( map ) )
    {
      vtkVariant *v = rating->Get( "rating" );
      if( v ) ratingValue = v->ToInt();
    }
  }

  this->ui->ratingSlider->setValue( ratingValue );
  this->ui->ratingValueLabel->setText( 0 == ratingValue ? tr( "N/A" ) : QString::number( ratingValue ) );

  // re-enable the rating slider's signals
  this->ui->ratingSlider->blockSignals( oldSignalState );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::updateInterface()
{
  Alder::Application *app = Alder::Application::GetInstance();
  Alder::Study *study = app->GetActiveStudy();
  Alder::Cineloop *cineloop = app->GetActiveCineloop();
  Alder::Image *image = app->GetActiveImage();
  bool loggedIn = NULL != app->GetActiveUser();

  // login button (login/logout)
  this->ui->actionLogin->setText( tr( loggedIn ? "Logout" : "Login" ) );

  // set all widget enable states
  this->ui->actionOpenStudy->setEnabled( loggedIn );
  this->ui->unratedCheckBox->setEnabled( study );
  this->ui->actionPreviousStudy->setEnabled( study );
  this->ui->actionNextStudy->setEnabled( study );
  this->ui->previousStudyPushButton->setEnabled( study );
  this->ui->nextStudyPushButton->setEnabled( study );
  this->ui->addImagePushButton->setEnabled( cineloop );
  this->ui->removeImagePushButton->setEnabled( image );
  this->ui->ratingSlider->setEnabled( image );
  this->ui->notePushButton->setEnabled( false ); // TODO: notes aren't implemented
  this->ui->studyTreeWidget->setEnabled( study );
  this->ui->medicalImageWidget->setEnabled( loggedIn );

  this->updateStudyTreeWidget();
  this->updateStudyInformation();
  this->updateMedicalImageWidget();
  this->updateRating();
}
