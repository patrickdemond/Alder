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
#include <QTreeWidgetItem>

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
  
  // set up child widgets
  this->ui->interviewTreeWidget->header()->hide();

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

  QObject::connect(
    this->ui->interviewPreviousPushButton, SIGNAL( clicked() ),
    this, SLOT( slotPreviousInterview() ) );
  QObject::connect(
    this->ui->interviewNextPushButton, SIGNAL( clicked() ),
    this, SLOT( slotNextInterview() ) );
  QObject::connect(
    this->ui->interviewTreeWidget, SIGNAL( itemSelectionChanged() ),
    this, SLOT( slotTreeSelectionChanged() ) );
  QObject::connect(
    this->ui->interviewRatingSlider, SIGNAL( valueChanged( int ) ),
    this, SLOT( slotInterviewRatingChanged( int ) ) );
  QObject::connect(
    this->ui->interviewNoteTextEdit, SIGNAL( textChanged() ),
    this, SLOT( slotInterviewNoteChanged() ) );

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
  
  this->ui->framePlayerWidget->setViewer( this->InterviewViewer );
  this->setCorner( Qt::BottomLeftCorner, Qt::BottomDockWidgetArea );
  this->setCorner( Qt::BottomRightCorner, Qt::RightDockWidgetArea );

  this->readSettings();
  this->updateInterface();

  // give a bit more room to the tree
  double total = this->ui->interviewTreeWidget->height() + this->ui->interviewNoteTextEdit->height();
  QList<int> sizeList;
  sizeList.append( floor( 2 * total / 3 ) );
  sizeList.append( total - sizeList[0] );
  this->ui->interviewTextSplitter->setSizes( sizeList );
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
void QMainAlderWindow::slotPreviousInterview()
{
  Alder::Application *app = Alder::Application::GetInstance();
  Alder::User *user = app->GetActiveUser();
  Alder::Interview *activeInterview = app->GetActiveInterview();
  vtkSmartPointer< Alder::Interview > interview;
  if( user && activeInterview )
  {
    interview = activeInterview->GetPrevious(
      this->ui->interviewLoadedCheckBox->isChecked(),
      this->ui->interviewUnratedCheckBox->isChecked() );

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
      this->ui->interviewLoadedCheckBox->isChecked(),
      this->ui->interviewUnratedCheckBox->isChecked() );

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
  }
  else if( !this->atlasVisible && NULL != atlasImage )
  {
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
void QMainAlderWindow::slotInterviewRatingChanged( int value )
{
  Alder::Application *app = Alder::Application::GetInstance();

  // make sure we have an active user and image
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
  this->updateInterviewRating();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::slotInterviewNoteChanged()
{
  vtkSmartPointer< Alder::Exam > exam;
  Alder::Application *app = Alder::Application::GetInstance();

  // make sure we have an active user and image, and the image has an exam
  Alder::User *user = app->GetActiveUser();
  if( user )
  {
    Alder::Image *image = app->GetActiveImage();
    if( image && image->GetRecord( exam ) )
    {
      exam->Set( "Note", this->ui->interviewNoteTextEdit->toPlainText().toStdString() );
      exam->Save();
    }
  }
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
void QMainAlderWindow::updateInterviewInformation()
{
  QString noteString = "";
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
        noteString = exam->Get( "Note" ).ToString().c_str();
        interviewerString = exam->Get( "Interviewer" ).ToString().c_str();
        siteString = interview->Get( "Site" ).ToString().c_str();
        dateString = exam->Get( "DatetimeAcquired" ).ToString().c_str();
      }
    }
  }

  this->ui->interviewNoteTextEdit->setPlainText( noteString );
  this->ui->interviewInterviewerValueLabel->setText( interviewerString );
  this->ui->interviewSiteValueLabel->setText( siteString );
  this->ui->interviewDateValueLabel->setText( dateString );
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
      vtkSmartPointer< Alder::Modality > modality;
      exam->GetRecord( modality );
      std::string modalityName = modality->Get( "Name" ).ToString();
      QTreeWidgetItem *parentItem;
      if( 0 == modalityName.compare( "Dexa" ) )
      {
        if( 0 == user->Get( "RateDexa" ).ToInt() ) continue;
        parentItem = dexaItem;
      }
      else if( 0 == modalityName.compare( "Retinal" ) )
      {
        if( 0 == user->Get( "RateRetinal" ).ToInt() ) continue;
        parentItem = retinalItem;
      }
      else if( 0 == modalityName.compare( "Ultrasound" ) )
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
            if ( dims.size() > 2 && dims[2] > 1 )
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
void QMainAlderWindow::updateInterviewImageWidget()
{
  Alder::Image *image = Alder::Application::GetInstance()->GetActiveImage();

  if( image ) this->InterviewViewer->Load( image->GetFileName().c_str() );
  else this->InterviewViewer->SetImageToSinusoid();

  this->ui->framePlayerWidget->updateFromViewer();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::updateInterviewRating()
{
  // stop the rating slider's signals until we are done
  bool oldSignalState = this->ui->interviewRatingSlider->blockSignals( true );

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

  this->ui->interviewRatingSlider->setValue( ratingValue );
  this->ui->interviewRatingValueLabel->setText( 0 == ratingValue ? tr( "N/A" ) : QString::number( ratingValue ) );

  // re-enable the rating slider's signals
  this->ui->interviewRatingSlider->blockSignals( oldSignalState );
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
  Alder::Interview *interview = app->GetActiveInterview();
  Alder::Image *image = app->GetActiveImage();
  Alder::Image *atlasImage = app->GetActiveAtlasImage();
  bool loggedIn = NULL != app->GetActiveUser();

  // dynamic menu action names
  this->ui->actionLogin->setText( tr( loggedIn ? "Logout" : "Login" ) );
  this->ui->actionShowAtlas->setText( tr( this->atlasVisible ? "Hide Atlas" : "Show Atlas" ) );

  // display or hide the image atlas widgets
  this->ui->imageWidgetSplitter->setChildrenCollapsible( !this->atlasVisible );
  this->ui->atlasDockWidget->setVisible( this->atlasVisible );
  this->ui->interviewDockWidget->setVisible( !this->atlasVisible );

  if( this->atlasVisible )
  {
    // make sure the atlas image widget is showing
    QList<int> sizeList = this->ui->imageWidgetSplitter->sizes();
    if( 0 == sizeList[0] )
    {
      sizeList[0] = floor( sizeList[1] / 2 );
      sizeList[1] = sizeList[0];
      this->ui->imageWidgetSplitter->setSizes( sizeList );
    }
  }
  else
  {
    // make sure the atlas image widget is not showing
    QList<int> sizeList = this->ui->imageWidgetSplitter->sizes();
    if( 0 != sizeList[0] )
    {
      sizeList[1] += sizeList[0];
      sizeList[0] = 0;
      this->ui->imageWidgetSplitter->setSizes( sizeList );
    }
  }

  // set all widget enable states
  this->ui->actionOpenInterview->setEnabled( loggedIn );
  this->ui->actionShowAtlas->setEnabled( loggedIn );
  this->ui->interviewUnratedCheckBox->setEnabled( interview );
  this->ui->interviewLoadedCheckBox->setEnabled( interview );
  this->ui->interviewPreviousPushButton->setEnabled( interview );
  this->ui->interviewNextPushButton->setEnabled( interview );
  this->ui->interviewRatingSlider->setEnabled( image );
  this->ui->interviewNoteTextEdit->setEnabled( image );
  this->ui->interviewTreeWidget->setEnabled( interview );

  this->ui->atlasRatingComboBox->setEnabled( atlasImage );
  this->ui->atlasPreviousPushButton->setEnabled( atlasImage );
  this->ui->atlasNextPushButton->setEnabled( atlasImage );
  this->ui->atlasHelpTextEdit->setEnabled( this->atlasVisible );
  this->ui->atlasNoteTextEdit->setEnabled( atlasImage && this->atlasVisible );

  this->ui->framePlayerWidget->setEnabled( loggedIn );

  this->ui->imageWidgetSplitter->setEnabled( this->atlasVisible );
  this->ui->interviewImageWidget->setEnabled( loggedIn );
  this->ui->atlasImageWidget->setEnabled( loggedIn );

  this->updateInterviewTreeWidget();
  this->updateInterviewInformation();
  this->updateInterviewImageWidget();
  this->updateInterviewRating();

  if( this->atlasVisible )
  {
    this->updateAtlasInformation();
    this->updateAtlasImageWidget();
  }
}
