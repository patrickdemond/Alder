/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QSelectInterviewDialog.cxx
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/
#include "QSelectInterviewDialog.h"
#include "ui_QSelectInterviewDialog.h"

#include "Application.h"
#include "Database.h"
#include "Exam.h"
#include "Interview.h"

#include "vtkSmartPointer.h"

#include <QInputDialog>
#include <QList>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>

#include <vector>

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QSelectInterviewDialog::QSelectInterviewDialog( QWidget* parent )
  : QDialog( parent )
{
  this->ui = new Ui_QSelectInterviewDialog;
  this->ui->setupUi( this );
  this->ui->interviewTableWidget->horizontalHeader()->setResizeMode( QHeaderView::Stretch );
  this->ui->interviewTableWidget->horizontalHeader()->setClickable( true );
  this->ui->interviewTableWidget->verticalHeader()->setVisible( false );
  this->ui->interviewTableWidget->setSelectionBehavior( QAbstractItemView::SelectRows );
  this->ui->interviewTableWidget->setSelectionMode( QAbstractItemView::SingleSelection );

  this->searchText = "";
  this->sortColumn = 0;
  this->sortOrder = Qt::AscendingOrder;

  QObject::connect(
    this->ui->searchPushButton, SIGNAL( clicked( bool ) ),
    this, SLOT( slotSearch() ) );
  QObject::connect(
    this->ui->buttonBox, SIGNAL( accepted() ),
    this, SLOT( slotAccepted() ) );
  QObject::connect(
    this->ui->interviewTableWidget, SIGNAL( itemSelectionChanged() ),
    this, SLOT( slotSelectionChanged() ) );
  QObject::connect(
    this->ui->interviewTableWidget->horizontalHeader(), SIGNAL( sectionClicked( int ) ),
    this, SLOT( slotHeaderClicked( int ) ) );

  this->updateInterface();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QSelectInterviewDialog::~QSelectInterviewDialog()
{
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QSelectInterviewDialog::slotSearch()
{
  bool ok;
  QString text = QInputDialog::getText(
    this,
    QObject::tr( "Search Term" ),
    QObject::tr( "Provide some or all of the interviews to search for:" ),
    QLineEdit::Normal,
    QString(),
    &ok );

  if( ok )
  {
    this->searchText = text;
    this->updateInterface();
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QSelectInterviewDialog::slotAccepted()
{
  // get the UId of the selected row
  vtkSmartPointer< Alder::Interview > interview;
  QList<QTableWidgetItem *> list = this->ui->interviewTableWidget->selectedItems();
  if( list.empty() )
  {
    interview = NULL;
  }
  else
  {
    std::map< std::string, std::string > map;
    map["UId"] = list.at( 0 )->text().toStdString();
    map["VisitDate"] = list.at( 1 )->text().toStdString();
    interview = vtkSmartPointer< Alder::Interview >::New();
    interview->Load( map );
  }

  Alder::Application::GetInstance()->SetActiveInterview( interview );
  this->accept();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QSelectInterviewDialog::slotSelectionChanged()
{
  vtkSmartPointer< Alder::Interview > interview;
  QList<QTableWidgetItem *> list = this->ui->interviewTableWidget->selectedItems();
  this->ui->buttonBox->button( QDialogButtonBox::Ok )->setEnabled( !list.empty() );

  if( !list.empty() )
  {
    // update the selected interview
    std::map< std::string, std::string > map;
    map["UId"] = list.at( 0 )->text().toStdString();
    map["VisitDate"] = list.at( 1 )->text().toStdString();
    interview = vtkSmartPointer< Alder::Interview >::New();
    interview->Load( map );
    interview->Update();
  }

  this->updateInterface();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QSelectInterviewDialog::slotHeaderClicked( int index )
{
  // reverse order if already sorted
  if( this->sortColumn == index )
    this->sortOrder = Qt::AscendingOrder == this->sortOrder ? Qt::DescendingOrder : Qt::AscendingOrder;
  this->sortColumn = index;
  this->updateInterface();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QSelectInterviewDialog::updateInterface()
{
  Alder::User *user = Alder::Application::GetInstance()->GetActiveUser();
  this->ui->interviewTableWidget->setRowCount( 0 );
  QTableWidgetItem *item;
  std::vector< vtkSmartPointer< Alder::Exam > > examList;
  std::vector< vtkSmartPointer< Alder::Exam > >::iterator examIt;
  Alder::Exam *exam;
  
  std::vector< vtkSmartPointer< Alder::Interview > > interviewList;
  Alder::Interview::GetAll( &interviewList );
  std::vector< vtkSmartPointer< Alder::Interview > >::iterator it;
  for( it = interviewList.begin(); it != interviewList.end(); ++it )
  { // for every interview, add a new row
    Alder::Interview *interview = *it;
    QString UId = QString( interview->Get( "UId" ).ToString().c_str() );
    
    // get the list of studies associated with this interview
    bool dexaUpdated = false, retinalUpdated = false, ultrasoundUpdated = false;
    int dexaCount = 0, retinalCount = 0, ultrasoundCount = 0;
    int dexaRatedCount = 0, retinalRatedCount = 0, ultrasoundRatedCount = 0;
    QString dexaString = "?", retinalString = "?", ultrasoundString = "?";
    QString date = tr( "N/A" );
    interview->GetList( &examList );

    // count the number of exams of each modality and whether 
    for( examIt = examList.begin(); examIt != examList.end(); ++examIt )
    {
      exam = examIt->GetPointer();
      std::string modality = exam->Get( "Modality" ).ToString();
      std::string stage = exam->Get( "Stage" ).ToString();
      
      if( "Dexa" == modality )
      {
        dexaUpdated = true;
        if( 0 == stage.compare( "Completed" ) ) dexaCount++;
        if( exam->IsRatedBy( user ) ) dexaRatedCount++;
      }
      else if( "Retinal" == modality )
      {
        retinalUpdated = true;
        if( 0 == stage.compare( "Completed" ) ) retinalCount++;
        if( exam->IsRatedBy( user ) ) retinalRatedCount++;
      }
      else if( "Ultrasound" == modality )
      {
        ultrasoundUpdated = true;
        if( 0 == stage.compare( "Completed" ) ) ultrasoundCount++;
        if( exam->IsRatedBy( user ) ) ultrasoundRatedCount++;
      }
      // TODO: log if an unknown modality is found
    }

    // set the text, if updated
    if( dexaUpdated )
    {
      dexaString = QString::number( dexaRatedCount );
      dexaString += tr( " of " );
      dexaString += QString::number( dexaCount );
    }

    if( retinalUpdated )
    {
      retinalString = QString::number( retinalRatedCount );
      retinalString += tr( " of " );
      retinalString += QString::number( retinalCount );
    }

    if( ultrasoundUpdated )
    {
      ultrasoundString = QString::number( ultrasoundRatedCount );
      ultrasoundString += tr( " of " );
      ultrasoundString += QString::number( ultrasoundCount );
    }

    // if rated display "rated", otherwise 

    if( this->searchText.isEmpty() || UId.contains( this->searchText, Qt::CaseInsensitive ) )
    {
      this->ui->interviewTableWidget->insertRow( 0 );

      // add UId to row
      item = new QTableWidgetItem;
      item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
      item->setText( UId );
      this->ui->interviewTableWidget->setItem( 0, 0, item );

      // add visit date to row
      item = new QTableWidgetItem;
      item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
      item->setText( QString( interview->Get( "VisitDate" ).ToString().c_str() ) );
      this->ui->interviewTableWidget->setItem( 0, 1, item );

      // add dexa interview to row
      item = new QTableWidgetItem;
      item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
      item->setText( dexaString );
      this->ui->interviewTableWidget->setItem( 0, 2, item );

      // add retinal interview to row
      item = new QTableWidgetItem;
      item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
      item->setText( retinalString );
      this->ui->interviewTableWidget->setItem( 0, 4, item );

      // add ultrasound interview to row
      item = new QTableWidgetItem;
      item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
      item->setText( ultrasoundString );
      this->ui->interviewTableWidget->setItem( 0, 3, item );
    }
  }

  this->ui->interviewTableWidget->sortItems( this->sortColumn, this->sortOrder );
}
