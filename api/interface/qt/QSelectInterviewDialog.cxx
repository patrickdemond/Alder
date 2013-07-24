/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QSelectInterviewDialog.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include "QSelectInterviewDialog.h"
#include "ui_QSelectInterviewDialog.h"

#include "Application.h"
#include "Database.h"
#include "Exam.h"
#include "Interview.h"
#include "QueryModifier.h"

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
  // define the column indeces
  this->columnIndex["Site"] = 0;
  this->columnIndex["UId"] = 1;
  this->columnIndex["VisitDate"] = 2;
  this->columnIndex["Dexa"] = 3;
  this->columnIndex["Retinal"] = 4;
  this->columnIndex["Ultrasound"] = 5;

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
    map["UId"] = list.at( this->columnIndex["UId"] )->text().toStdString();
    map["VisitDate"] = list.at( this->columnIndex["VisitDate"] )->text().toStdString();
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
    std::map< std::string, std::string > map;
    map["UId"] = list.at( this->columnIndex["UId"] )->text().toStdString();
    map["VisitDate"] = list.at( this->columnIndex["VisitDate"] )->text().toStdString();
    interview = vtkSmartPointer< Alder::Interview >::New();
    interview->Load( map );
    if( !interview->HasExamData() ) interview->UpdateExamData();
    this->updateRow( list.at( 0 )->row(), interview );
  }
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
void QSelectInterviewDialog::updateRow( int row, Alder::Interview *interview )
{
  std::vector< vtkSmartPointer< Alder::Exam > > examList;
  std::vector< vtkSmartPointer< Alder::Exam > >::iterator examIt;
  Alder::Exam *exam;
  Alder::User *user = Alder::Application::GetInstance()->GetActiveUser();
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
    
    // NOTE: it is possible that an exam with state "Ready" has valid data, but we are leaving
    // those exams out for now since we don't know for sure whether they are always valid
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

  if( this->searchText.isEmpty() || UId.contains( this->searchText, Qt::CaseInsensitive ) )
  {
    QTableWidgetItem *item;
    item = this->ui->interviewTableWidget->item( row, this->columnIndex["Site"] );
    if( item ) item->setText( interview->Get( "Site" ).ToString().c_str() );
    item = this->ui->interviewTableWidget->item( row, this->columnIndex["UId"] );
    if( item ) item->setText( UId );
    item = this->ui->interviewTableWidget->item( row, this->columnIndex["VisitDate"] );
    if( item ) item->setText( QString( interview->Get( "VisitDate" ).ToString().c_str() ) );
    item = this->ui->interviewTableWidget->item( row, this->columnIndex["Dexa"] );
    if( item ) item->setText( dexaString );
    item = this->ui->interviewTableWidget->item( row, this->columnIndex["Retinal"] );
    if( item ) item->setText( retinalString );
    item = this->ui->interviewTableWidget->item( row, this->columnIndex["Ultrasound"] );
    if( item ) item->setText( ultrasoundString );
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QSelectInterviewDialog::updateInterface()
{
  this->ui->interviewTableWidget->setRowCount( 0 );
  QTableWidgetItem *item;
  
  if( !this->searchText.isEmpty() )
  {
    // create a modifier using the search text
    std::string where = this->searchText.toStdString();
    where += "%";
    vtkSmartPointer< Alder::QueryModifier > modifier = vtkSmartPointer< Alder::QueryModifier >::New();
    modifier->Where( "UId", "LIKE", vtkVariant( where ) );

    // now get all the interviews given the search text
    std::vector< vtkSmartPointer< Alder::Interview > > interviewList;
    Alder::Interview::GetAll( &interviewList, modifier );
    std::vector< vtkSmartPointer< Alder::Interview > >::iterator it;
    for( it = interviewList.begin(); it != interviewList.end(); ++it )
    { // for every interview, add a new row
      Alder::Interview *interview = *it;
      QString UId = QString( interview->Get( "UId" ).ToString().c_str() );
      
      if( this->searchText.isEmpty() || UId.contains( this->searchText, Qt::CaseInsensitive ) )
      {
        this->ui->interviewTableWidget->insertRow( 0 );

        // add site to row
        item = new QTableWidgetItem;
        item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
        this->ui->interviewTableWidget->setItem( 0, this->columnIndex["Site"], item );

        // add UId to row
        item = new QTableWidgetItem;
        item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
        this->ui->interviewTableWidget->setItem( 0, this->columnIndex["UId"], item );

        // add visit date to row
        item = new QTableWidgetItem;
        item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
        this->ui->interviewTableWidget->setItem( 0, this->columnIndex["VisitDate"], item );

        // add dexa interview to row
        item = new QTableWidgetItem;
        item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
        this->ui->interviewTableWidget->setItem( 0, this->columnIndex["Dexa"], item );

        // add retinal interview to row
        item = new QTableWidgetItem;
        item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
        this->ui->interviewTableWidget->setItem( 0, this->columnIndex["Retinal"], item );

        // add ultrasound interview to row
        item = new QTableWidgetItem;
        item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
        this->ui->interviewTableWidget->setItem( 0, this->columnIndex["Ultrasound"], item );

        this->updateRow( 0, interview );
      }
    }

    this->ui->interviewTableWidget->sortItems( this->sortColumn, this->sortOrder );
  }
}
