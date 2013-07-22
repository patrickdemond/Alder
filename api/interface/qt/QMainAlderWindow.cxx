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
#include "QExamNoteDialog.h"
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
  this->ui->interviewTreeWidget->header()->hide();

  // connect the menu items
  QObject::connect(
    this->ui->actionOpenInterview, SIGNAL( triggered() ),
    this, SLOT( slotOpenInterview() ) );
  QObject::connect(
    this->ui->actionPreviousInterview, SIGNAL( triggered() ),
    this, SLOT( slotPreviousInterview() ) );
  QObject::connect(
    this->ui->actionNextInterview, SIGNAL( triggered() ),
    this, SLOT( slotNextInterview() ) );
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

  QObject::connect(
    this->ui->previousInterviewPushButton, SIGNAL( clicked() ),
    this, SLOT( slotPreviousInterview() ) );
  QObject::connect(
    this->ui->nextInterviewPushButton, SIGNAL( clicked() ),
    this, SLOT( slotNextInterview() ) );
  QObject::connect(
    this->ui->interviewTreeWidget, SIGNAL( itemSelectionChanged() ),
    this, SLOT( slotTreeSelectionChanged() ) );
  QObject::connect(
    this->ui->ratingSlider, SIGNAL( valueChanged( int ) ),
    this, SLOT( slotRatingSliderChanged( int ) ) );
  QObject::connect(
    this->ui->notePushButton, SIGNAL( clicked() ),
    this, SLOT( slotOpenNote() ) );


  this->Viewer = vtkSmartPointer<vtkMedicalImageViewer>::New();
  vtkRenderWindow* renwin = this->ui->medicalImageWidget->GetRenderWindow();
  vtkRenderer* renderer = this->Viewer->GetRenderer();
  renderer->GradientBackgroundOn();
  renderer->SetBackground( 0, 0, 0 );
  renderer->SetBackground2( 0, 0, 1 );
  this->Viewer->SetRenderWindow( renwin  );  
  this->Viewer->InterpolateOff();
  
  this->ui->framePlayerWidget->setViewer(this->Viewer);
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
  this->ui->framePlayerWidget->setViewer(0);
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
void QMainAlderWindow::slotPreviousInterview()
{
  Alder::Application *app = Alder::Application::GetInstance();
  Alder::User *user = app->GetActiveUser();
  Alder::Interview *activeInterview = app->GetActiveInterview();
  vtkSmartPointer< Alder::Interview > interview;
  if( user && activeInterview )
  {
    interview = activeInterview->GetPrevious(
      this->ui->loadedCheckBox->isChecked(),
      this->ui->unratedCheckBox->isChecked() );

    // warn user if the neighbouring interview is an empty record (ie: no neighbour found)
    vtkVariant vId = interview->Get( "Id" );
    if( !vId.IsValid() || 0 == vId.ToInt() )
    {
      QMessageBox errorMessage( this );
      errorMessage.setWindowModality( Qt::WindowModal );
      errorMessage.setIcon( QMessageBox::Warning );
      errorMessage.setText( tr( "There are no remaining studies available which meet your criteria." ) );
      errorMessage.exec();
    }
    else
    {
      if( !interview->HasExamData() || !interview->HasImageData() )
      {
        // create a progress dialog to observe the progress of the update
        QProgressDialog dialog( this );
        dialog.setModal( true );
        dialog.setWindowTitle( tr( "Downloading Exam Images" ) );
        dialog.setMessage( tr( "Please wait while the interview's images are downloaded." ) );
        dialog.open();
        interview->UpdateExamData();
        interview->UpdateImageData();
        dialog.accept();
      }

      app->SetActiveInterview( interview );
      this->updateInterface();
    }
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::slotNextInterview()
{
  Alder::Application *app = Alder::Application::GetInstance();
  Alder::User *user = app->GetActiveUser();
  Alder::Interview *activeInterview = app->GetActiveInterview();
  vtkSmartPointer< Alder::Interview > interview;
  if( user && activeInterview )
  {
    interview = activeInterview->GetNext(
      this->ui->loadedCheckBox->isChecked(),
      this->ui->unratedCheckBox->isChecked() );

    // warn user if the neighbouring interview is an empty record (ie: no neighbour found)
    vtkVariant vId = interview->Get( "Id" );
    if( !vId.IsValid() || 0 == vId.ToInt() )
    {
      QMessageBox errorMessage( this );
      errorMessage.setWindowModality( Qt::WindowModal );
      errorMessage.setIcon( QMessageBox::Warning );
      errorMessage.setText( tr( "There are no remaining studies available which meet your criteria." ) );
      errorMessage.exec();
    }
    else
    {
      if( !interview->HasExamData() || !interview->HasImageData() )
      {
        // create a progress dialog to observe the progress of the update
        QProgressDialog dialog( this );
        dialog.setModal( true );
        dialog.setWindowTitle( tr( "Downloading Exam Images" ) );
        dialog.setMessage( tr( "Please wait while the interview's images are downloaded." ) );
        dialog.open();
        interview->UpdateExamData();
        interview->UpdateImageData();
        dialog.accept();
      }

      app->SetActiveInterview( interview );
      this->updateInterface();
    }
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
  
  QList<QTreeWidgetItem*> list = this->ui->interviewTreeWidget->selectedItems();
  if( 0 < list.size() )
  {
    std::map< QTreeWidgetItem*, vtkSmartPointer<Alder::ActiveRecord> >::iterator it;
    it = this->treeModelMap.find( list.at( 0 ) );
    if( it != this->treeModelMap.end() )
    {
      Alder::ActiveRecord *record = it->second;
      app->SetActiveImage( Alder::Image::SafeDownCast( record ) );
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
  map["UserId"] = user->Get( "Id" ).ToString();
  map["ImageId"] = image->Get( "Id" ).ToString();
  vtkNew< Alder::Rating > rating;
  if( !rating->Load( map ) )
  { // no record exists, set the user and image ids
    rating->Set( "UserId", user->Get( "Id" ).ToInt() );
    rating->Set( "ImageId", image->Get( "Id" ).ToInt() );
  }

  if( 0 == value ) rating->SetNull( "Rating" );
  else rating->Set( "Rating", value );

  rating->Save();
  this->updateRating();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::slotOpenNote()
{
  Alder::Application *app = Alder::Application::GetInstance();

  // make sure we have an active image
  Alder::Image *image = app->GetActiveImage();
  if( !image ) throw std::runtime_error( "Open note button cliecked without an active image" );

  QExamNoteDialog dialog( this );
  dialog.setModal( true );
  dialog.setWindowTitle( tr( "Exam Note" ) );
  dialog.exec();
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
void QMainAlderWindow::updateInformation()
{
  QString interviewerString = tr( "N/A" );
  QString siteString = tr( "N/A" );
  QString dateString = tr( "N/A" );

  // fill in the active exam information
  Alder::Interview *interview = Alder::Application::GetInstance()->GetActiveInterview();
  if( interview )
  {
    // get exam from active image
    Alder::Image *image = Alder::Application::GetInstance()->GetActiveImage();

    if( image )
    {
      vtkSmartPointer< Alder::Exam > exam;
      if( image->GetRecord( exam ) )
      {
        interviewerString = exam->Get( "Interviewer" ).ToString().c_str();
        siteString = interview->Get( "Site" ).ToString().c_str();
        dateString = exam->Get( "DatetimeAcquired" ).ToString().c_str();
      }
    }
  }

  this->ui->interviewerValueLabel->setText( interviewerString );
  this->ui->siteValueLabel->setText( siteString );
  this->ui->dateValueLabel->setText( dateString );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::updateInterviewTreeWidget()
{
  Alder::Interview *interview = Alder::Application::GetInstance()->GetActiveInterview();

  // stop the tree's signals until we are done
  bool oldSignalState = this->ui->interviewTreeWidget->blockSignals( true );

  // if a interview is open then populate the interview tree
  this->treeModelMap.clear();
  this->ui->interviewTreeWidget->clear();
  if( interview )
  {
    // get the active image so that we can highlight it
    Alder::Application *app = Alder::Application::GetInstance();
    Alder::User *user = app->GetActiveUser();
    Alder::Image *activeImage = app->GetActiveImage();
    QTreeWidgetItem *selectedItem = NULL, *dexaItem = NULL, *retinalItem = NULL, *ultrasoundItem = NULL;

    // make root the interview's UID and date
    QString name = tr( "Interview: " );
    name += interview->Get( "UId" ).ToString().c_str();
    name += " (";
    name += interview->Get( "VisitDate" ).ToString().c_str();
    name += ")";
    QTreeWidgetItem *root = new QTreeWidgetItem( this->ui->interviewTreeWidget );
    root->setText( 0, name );
    root->setExpanded( true );
    root->setFlags( Qt::ItemIsEnabled );
    this->ui->interviewTreeWidget->addTopLevelItem( root );

    // make each modality type a child of the root
    if( 0 < user->Get( "RateDexa" ).ToInt() )
    {
      dexaItem = new QTreeWidgetItem( root );
      dexaItem->setText( 0, "Dexa" );
      dexaItem->setExpanded( false );
      dexaItem->setFlags( Qt::ItemIsEnabled );
    }
    
    if( 0 < user->Get( "RateRetinal" ).ToInt() )
    {
      retinalItem = new QTreeWidgetItem( root );
      retinalItem->setText( 0, "Retinal" );
      retinalItem->setExpanded( false );
      retinalItem->setFlags( Qt::ItemIsEnabled );
    }
    
    if( 0 < user->Get( "RateUltrasound" ).ToInt() )
    {
      ultrasoundItem = new QTreeWidgetItem( root );
      ultrasoundItem->setText( 0, "Ultrasound" );
      ultrasoundItem->setExpanded( false );
      ultrasoundItem->setFlags( Qt::ItemIsEnabled );
    }

    std::vector< vtkSmartPointer< Alder::Exam > > examList;
    std::vector< vtkSmartPointer< Alder::Exam > >::iterator examIt;
    interview->GetList( &examList );
    for( examIt = examList.begin(); examIt != examList.end(); ++examIt )
    {
      Alder::Exam *exam = examIt->GetPointer();

      // figure out which parent to add this exam to based on modality
      std::string modality = exam->Get( "Modality" ).ToString();
      QTreeWidgetItem *parentItem;
      if( 0 == modality.compare( "Dexa" ) )
      {
        if( 0 == user->Get( "RateDexa" ).ToInt() ) continue;
        parentItem = dexaItem;
      }
      else if( 0 == modality.compare( "Retinal" ) )
      {
        if( 0 == user->Get( "RateRetinal" ).ToInt() ) continue;
        parentItem = retinalItem;
      }
      else if( 0 == modality.compare( "Ultrasound" ) )
      {
        if( 0 == user->Get( "RateUltrasound" ).ToInt() ) continue;
        parentItem = ultrasoundItem;
      }

      name = tr( "Exam" ) + ": ";
      std::string laterality = exam->Get( "Laterality" ).ToString();
      if( "none" != laterality )
      {
        name += laterality.c_str();
        name += " ";
      }

      std::string examType = exam->Get( "Type" ).ToString();
      name += examType.c_str();

      QTreeWidgetItem *examItem = new QTreeWidgetItem( parentItem );
      this->treeModelMap[examItem] = *examIt;
      examItem->setText( 0, name );
      examItem->setIcon(0, QIcon(":/icons/eye-visible-icon" ) );
      examItem->setExpanded( true );
      examItem->setFlags( Qt::ItemIsEnabled );

      // add the images for this exam
      std::vector< vtkSmartPointer< Alder::Image > > imageList;
      std::vector< vtkSmartPointer< Alder::Image > >::iterator imageIt;
      exam->GetList( &imageList );

      // if the exam has no images, display the status of the exam
      if( imageList.empty() )
      {
        examItem->setIcon(0, QIcon(":/icons/x-icon" ) );        
      }

      for( imageIt = imageList.begin(); imageIt != imageList.end(); ++imageIt )
      {
        Alder::Image *image = imageIt->GetPointer();
        
        // don't show images with parents (that happens in the loop below instead)
        if( !image->Get( "ParentImageId" ).IsValid() )
        {
          name = tr( "Image #" );
          name += image->Get( "Acquisition" ).ToString().c_str();
          QTreeWidgetItem *imageItem = new QTreeWidgetItem( examItem );
          this->treeModelMap[imageItem] = *imageIt;
          imageItem->setText( 0, name );

          if( examType.compare( "CarotidIntima" ) == 0 ||
              examType.compare( "Plaque" ) == 0 )
          {
            std::vector<int> dims = image->GetDICOMDimensions();
            if ( dims.size() > 2 )
              imageItem->setIcon(0, QIcon(":/icons/movie-icon" ) );             
          }
           
          imageItem->setExpanded( true );
          imageItem->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
          if( activeImage && activeImage->Get( "Id" ).ToInt() == image->Get( "Id" ).ToInt() )
            selectedItem = imageItem;

          // add child images for this image
          std::vector< vtkSmartPointer< Alder::Image > > childImageList;
          std::vector< vtkSmartPointer< Alder::Image > >::iterator childImageIt;
          image->GetList( &childImageList, "ParentImageId" );
          for( childImageIt = childImageList.begin(); childImageIt != childImageList.end(); ++childImageIt )
          {
            Alder::Image *childImage = childImageIt->GetPointer();
            name = tr( "Image #" );
            name += childImage->Get( "Acquisition" ).ToString().c_str();
            QTreeWidgetItem *childImageItem = new QTreeWidgetItem( imageItem );
            this->treeModelMap[childImageItem] = *childImageIt;
            childImageItem->setText( 0, name );
            childImageItem->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
            if( activeImage && activeImage->Get( "Id" ).ToInt() == childImage->Get( "Id" ).ToInt() )
              selectedItem = childImageItem;
          }
        }
      }
    }
      
    if( selectedItem ) this->ui->interviewTreeWidget->setCurrentItem( selectedItem );
  }

  // re-enable the tree's signals
  this->ui->interviewTreeWidget->blockSignals( oldSignalState );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::updateMedicalImageWidget()
{
  Alder::Image *image = Alder::Application::GetInstance()->GetActiveImage();

  if( image )
  {
    this->Viewer->Load( image->GetFileName().c_str() );
  }
  else
  {
    this->Viewer->SetImageToSinusoid();
  }
  this->ui->framePlayerWidget->updateFromViewer();
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
    map["UserId"] = user->Get( "Id" ).ToString();
    map["ImageId"] = image->Get( "Id" ).ToString();
    vtkNew< Alder::Rating > rating;
    if( rating->Load( map ) )
    {
      vtkVariant v = rating->Get( "Rating" );
      if( v.IsValid() ) ratingValue = v.ToInt();
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
  Alder::Interview *interview = app->GetActiveInterview();
  Alder::Image *image = app->GetActiveImage();
  bool loggedIn = NULL != app->GetActiveUser();

  // login button (login/logout)
  this->ui->actionLogin->setText( tr( loggedIn ? "Logout" : "Login" ) );

  // set all widget enable states
  this->ui->actionOpenInterview->setEnabled( loggedIn );
  this->ui->unratedCheckBox->setEnabled( interview );
  this->ui->actionPreviousInterview->setEnabled( interview );
  this->ui->actionNextInterview->setEnabled( interview );
  this->ui->previousInterviewPushButton->setEnabled( interview );
  this->ui->nextInterviewPushButton->setEnabled( interview );
  this->ui->ratingSlider->setEnabled( image );
  this->ui->notePushButton->setEnabled( image );
  this->ui->interviewTreeWidget->setEnabled( interview );
  this->ui->medicalImageWidget->setEnabled( loggedIn );
  this->ui->framePlayerWidget->setEnabled( loggedIn );

  this->updateInterviewTreeWidget();
  this->updateInformation();
  this->updateMedicalImageWidget();
  this->updateRating();
}
