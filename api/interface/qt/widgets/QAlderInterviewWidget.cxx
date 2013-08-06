/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QAlderInterviewWidget.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include "QAlderInterviewWidget.h"
#include "ui_QAlderInterviewWidget.h"

#include "Application.h"
#include "Exam.h"
#include "Image.h"
#include "Interview.h"
#include "Modality.h"
#include "Rating.h"
#include "User.h"

#include "vtkEventQtSlotConnect.h"
#include "vtkMedicalImageViewer.h"
#include "vtkNew.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"

#include "QProgressDialog.h"

#include <QMessageBox>
#include <QTreeWidgetItem>

#include <stdexcept>

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAlderInterviewWidget::QAlderInterviewWidget( QWidget* parent )
  : QWidget( parent )
{
  Alder::Application *app = Alder::Application::GetInstance();
  
  this->ui = new Ui_QAlderInterviewWidget;
  this->ui->setupUi( this );
  
  // set up child widgets
  this->ui->examTreeWidget->header()->hide();

  QObject::connect(
    this->ui->previousPushButton, SIGNAL( clicked() ),
    this, SLOT( slotPrevious() ) );
  QObject::connect(
    this->ui->nextPushButton, SIGNAL( clicked() ),
    this, SLOT( slotNext() ) );
  QObject::connect(
    this->ui->examTreeWidget, SIGNAL( itemSelectionChanged() ),
    this, SLOT( slotTreeSelectionChanged() ) );
  QObject::connect(
    this->ui->ratingSlider, SIGNAL( valueChanged( int ) ),
    this, SLOT( slotRatingChanged( int ) ) );
  QObject::connect(
    this->ui->noteTextEdit, SIGNAL( textChanged() ),
    this, SLOT( slotNoteChanged() ) );

  this->Connections = vtkSmartPointer<vtkEventQtSlotConnect>::New();
  this->Connections->Connect( Alder::Application::GetInstance(),
    Alder::Application::ActiveInterviewEvent,
    this,
    SLOT(updateExamTreeWidget()));
  this->Connections->Connect( Alder::Application::GetInstance(),
    Alder::Application::ActiveImageEvent,
    this,
    SLOT(updateInfo()));
  this->Connections->Connect( Alder::Application::GetInstance(),
    Alder::Application::ActiveImageEvent,
    this,
    SLOT(updateRating()));
  this->Connections->Connect( Alder::Application::GetInstance(),
    Alder::Application::ActiveImageEvent,
    this,
    SLOT(updateViewer()));

  this->Viewer = vtkSmartPointer<vtkMedicalImageViewer>::New();
  vtkRenderWindow* renwin = this->ui->imageWidget->GetRenderWindow();
  vtkRenderer* renderer = this->Viewer->GetRenderer();
  renderer->GradientBackgroundOn();
  renderer->SetBackground( 0, 0, 0 );
  renderer->SetBackground2( 0, 0, 1 );
  this->Viewer->SetRenderWindow( renwin );
  this->Viewer->InterpolateOff();
  this->Viewer->SetImageToSinusoid();

  // give a bit more room to the tree
  double total = this->ui->examTreeWidget->height() + this->ui->noteTextEdit->height();
  QList<int> sizeList;
  sizeList.append( floor( 2 * total / 3 ) );
  sizeList.append( total - sizeList[0] );
  this->ui->splitter->setSizes( sizeList );

  this->updateEnabled();  
};

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAlderInterviewWidget::~QAlderInterviewWidget()
{
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
vtkMedicalImageViewer* QAlderInterviewWidget::GetViewer()
{
  return static_cast<vtkMedicalImageViewer*>(this->Viewer.GetPointer());
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidget::slotPrevious()
{
  Alder::Application *app = Alder::Application::GetInstance();
  Alder::Interview *activeInterview = app->GetActiveInterview();
  vtkSmartPointer< Alder::Interview > interview;
  if( activeInterview )
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
    }
  }
  this->updateEnabled();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidget::slotNext()
{
  Alder::Application *app = Alder::Application::GetInstance();
  Alder::Interview *activeInterview = app->GetActiveInterview();
  vtkSmartPointer< Alder::Interview > interview;
  if( activeInterview )
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
    }
  }
  this->updateEnabled();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidget::slotTreeSelectionChanged()
{
  QList<QTreeWidgetItem*> list = this->ui->examTreeWidget->selectedItems();
  if( 0 < list.size() )
  {
    std::map< QTreeWidgetItem*, vtkSmartPointer<Alder::ActiveRecord> >::iterator it;
    it = this->treeModelMap.find( list.at( 0 ) );
    if( it != this->treeModelMap.end() )
    {
      Alder::ActiveRecord *record = it->second;
      Alder::Application::GetInstance()->SetActiveImage( Alder::Image::SafeDownCast( record ) );
    }
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidget::slotRatingChanged( int value )
{
  Alder::Application *app = Alder::Application::GetInstance();

  // make sure we have an active user and image
  Alder::User *user = app->GetActiveUser();
  Alder::Image *image = app->GetActiveImage();
  if( user && image )
  {
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
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidget::slotHideControls( bool hide )
{
  // hide/show all the widgets to show only the image viewer
  QList<QLayout*> layouts;
  layouts.append( &*this->ui->verticalLayout );
  layouts.append( &*this->ui->infoLayout );
  layouts.append( &*this->ui->ratingLayout );
  layouts.append( &*this->ui->buttonLayout );

  QList<QWidget*> widgets;
  for( int i = 0; i < layouts.count(); ++i )
  {
     QLayout* layout = layouts.at( i );
    for( int j = 0; j < layout->count(); ++j )
    {
      if( QWidgetItem* item = dynamic_cast<QWidgetItem*>(
          layout->itemAt( j ) ) )
      {    
        widgets.append( &*item->widget() );
      }  
    }  
  }
  for( int i = 0; i < widgets.count(); ++i )
  {
    QWidget* item = qobject_cast<QWidget*>( widgets.at( i ) );
    if( hide ) item->hide();
    else item->show();
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidget::slotNoteChanged()
{
  Alder::Application *app = Alder::Application::GetInstance();

  // make sure we have an active user and image, and the image has an exam
  Alder::User *user = app->GetActiveUser();
  Alder::Image *image = app->GetActiveImage();
  if( user && image )
  {
    vtkSmartPointer< Alder::Exam > exam;
    if( image->GetRecord( exam ) )
    {
      exam->Set( "Note", this->ui->noteTextEdit->toPlainText().toStdString() );
      exam->Save();
    }
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidget::updateInfo()
{
  QString noteString = "";
  QString interviewerString = tr( "N/A" );
  QString siteString = tr( "N/A" );
  QString dateString = tr( "N/A" );

  // fill in the active exam information
  Alder::Application *app = Alder::Application::GetInstance();

  Alder::Interview *interview = app->GetActiveInterview();
  Alder::Image *image = app->GetActiveImage();
  if( interview && image )
  {
    // get exam from active image
    vtkSmartPointer< Alder::Exam > exam;
    if( image->GetRecord( exam ) )
    {
      noteString = exam->Get( "Note" ).ToString().c_str();
      interviewerString = exam->Get( "Interviewer" ).ToString().c_str();
      siteString = interview->Get( "Site" ).ToString().c_str();
      dateString = exam->Get( "DatetimeAcquired" ).ToString().c_str();
    }
  }

  // set the text edit content from the exam note
  bool oldSignalState = this->ui->noteTextEdit->blockSignals( true );
  this->ui->noteTextEdit->setPlainText( noteString );
  this->ui->noteTextEdit->blockSignals( oldSignalState );

  this->ui->infoInterviewerValueLabel->setText( interviewerString );
  this->ui->infoSiteValueLabel->setText( siteString );
  this->ui->infoDateValueLabel->setText( dateString );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidget::updateExamTreeWidget()
{
  Alder::Application *app = Alder::Application::GetInstance();
  Alder::Interview *interview = app->GetActiveInterview();

  // stop the tree's signals until we are done
  bool oldSignalState = this->ui->examTreeWidget->blockSignals( true );

  // if a interview is open then populate the interview tree
  this->treeModelMap.clear();
  this->ui->examTreeWidget->clear();
  if( interview )
  {
    // get the active image so that we can highlight it
    Alder::User *user = app->GetActiveUser();
    Alder::Image *activeImage = app->GetActiveImage();
    QTreeWidgetItem *selectedItem = NULL, *item = NULL;
    std::map< std::string, QTreeWidgetItem* > modalityLookup;
    std::map< std::string, QTreeWidgetItem* >::iterator modalityLookupIt;

    // make root the interview's UID and date
    QString name = tr( "Interview: " );
    name += interview->Get( "UId" ).ToString().c_str();
    name += " (";
    name += interview->Get( "VisitDate" ).ToString().c_str();
    name += ")";
    QTreeWidgetItem *root = new QTreeWidgetItem( this->ui->examTreeWidget );
    root->setText( 0, name );
    root->setExpanded( true );
    root->setFlags( Qt::ItemIsEnabled );
    this->ui->examTreeWidget->addTopLevelItem( root );

    // make each modality type a child of the root
    std::vector< vtkSmartPointer< Alder::Modality > > modalityList;
    std::vector< vtkSmartPointer< Alder::Modality > >::iterator modalityIt;
    user->GetList( &modalityList );
    for( modalityIt = modalityList.begin(); modalityIt != modalityList.end(); ++modalityIt )
    {
      Alder::Modality *modality = modalityIt->GetPointer();
      std::string name = modality->Get( "Name" ).ToString();
      item = new QTreeWidgetItem( root );
      item->setText( 0, name.c_str() );
      item->setExpanded( false );
      item->setFlags( Qt::ItemIsEnabled );
      modalityLookup[name] = item;
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
      modalityLookupIt = modalityLookup.find( modality->Get( "Name" ).ToString() );
      if( modalityLookup.end() == modalityLookupIt ) continue;
      QTreeWidgetItem *parentItem = modalityLookupIt->second;

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
      examItem->setExpanded( true );
      examItem->setFlags( Qt::ItemIsEnabled );

      // add the images for this exam
      std::vector< vtkSmartPointer< Alder::Image > > imageList;
      std::vector< vtkSmartPointer< Alder::Image > >::iterator imageIt;
      exam->GetList( &imageList );

      // display the status of the exam
      examItem->setIcon( 0,
        imageList.empty() ? 
        QIcon(":/icons/x-icon" ) : 
        QIcon(":/icons/eye-visible-icon" ) );

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

          if( "CarotidIntima" == examType || "Plaque" == examType )
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
          for( childImageIt = childImageList.begin();
               childImageIt != childImageList.end(); ++childImageIt )
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
      
    if( selectedItem ) this->ui->examTreeWidget->setCurrentItem( selectedItem );
  }

  // re-enable the tree's signals
  this->ui->examTreeWidget->blockSignals( oldSignalState );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidget::updateRating()
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
  this->ui->ratingValueLabel->setText( 0 == ratingValue ? 
    tr( "N/A" ) : 
    QString::number( ratingValue ) );

  // re-enable the rating slider's signals
  this->ui->ratingSlider->blockSignals( oldSignalState );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidget::updateViewer()
{
  Alder::Image *image = Alder::Application::GetInstance()->GetActiveImage();
  if( image ) this->Viewer->Load( image->GetFileName().c_str() );
  else this->Viewer->SetImageToSinusoid();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QAlderInterviewWidget::updateEnabled()
{
  Alder::Application *app = Alder::Application::GetInstance();
  Alder::Interview *interview = app->GetActiveInterview();
  Alder::Image *image = app->GetActiveImage();

  // set all widget enable states
  this->ui->unratedCheckBox->setEnabled( interview );
  this->ui->loadedCheckBox->setEnabled( interview );
  this->ui->previousPushButton->setEnabled( interview );
  this->ui->nextPushButton->setEnabled( interview );
  this->ui->examTreeWidget->setEnabled( interview );

  this->ui->ratingSlider->setEnabled( image );
  this->ui->noteTextEdit->setEnabled( image );
}
