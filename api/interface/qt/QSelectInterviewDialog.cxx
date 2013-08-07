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
#include "Modality.h"
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
  int index = 0;
  this->ui = new Ui_QSelectInterviewDialog;
  this->ui->setupUi( this );
  QStringList labels;

  labels << "Site";
  this->columnIndex["Site"] = index++;

  labels << "UID";
  this->columnIndex["UId"] = index++;

  labels << "Visit Date";
  this->columnIndex["VisitDate"] = index++;

  // all modalities will fill up the remainder of the table
  std::vector< vtkSmartPointer< Alder::Modality > > modalityList;
  std::vector< vtkSmartPointer< Alder::Modality > >::iterator modalityListIt;
  Alder::Modality::GetAll( &modalityList );

  // make enough columns for all modalities and set their names
  this->ui->interviewTableWidget->setColumnCount( index + modalityList.size() );
  for( modalityListIt = modalityList.begin(); modalityListIt != modalityList.end(); ++modalityListIt )
  {
    std::string name = (*modalityListIt)->Get( "Name" ).ToString();
    labels << name.c_str();
    this->ui->interviewTableWidget->setVerticalHeaderItem( index, new QTableWidgetItem( name.c_str() ) );
    this->columnIndex[name] = index++;
  }

  this->ui->interviewTableWidget->setHorizontalHeaderLabels( labels );
  this->ui->interviewTableWidget->horizontalHeader()->setResizeMode( QHeaderView::Stretch );
  this->ui->interviewTableWidget->horizontalHeader()->setClickable( true );
  this->ui->interviewTableWidget->verticalHeader()->setVisible( false );
  this->ui->interviewTableWidget->setSelectionBehavior( QAbstractItemView::SelectRows );
  this->ui->interviewTableWidget->setSelectionMode( QAbstractItemView::SingleSelection );

  this->searchText = "";
  this->sortColumn = 1;
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

  // if the active interview is changed, Application invokes an ActiveInterviewEvent
  // the interview may not have downloaded images though
  // this dialog closes after accept is called and the main application window
  // slot that originally launched this dialog forces the active interview to 
  // update its image data
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
  {
    this->sortOrder = 
      Qt::AscendingOrder == this->sortOrder ? Qt::DescendingOrder : Qt::AscendingOrder;
  }

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
  
  std::map< std::string, bool > updateItemText;
  std::map< std::string, int > examCount;
  std::map< std::string, int > ratedCount;
  std::map< std::string, QString > itemText;
  std::string modalityName;
  std::vector< vtkSmartPointer< Alder::Modality > > modalityList;
  std::vector< vtkSmartPointer< Alder::Modality > >::iterator modalityListIt;

  Alder::Modality::GetAll( &modalityList );
  for( modalityListIt = modalityList.begin(); modalityListIt != modalityList.end(); ++modalityListIt )
  {
    modalityName = (*modalityListIt)->Get( "Name" ).ToString();
    updateItemText[modalityName] = false;
    examCount[modalityName] = 0;
    ratedCount[modalityName] = 0;
    itemText[modalityName] = "?";
  }

  QString date = tr( "N/A" );
  interview->GetList( &examList );

  // examCount the number of exams of each modality and whether they have been rated
  for( examIt = examList.begin(); examIt != examList.end(); ++examIt )
  {
    exam = examIt->GetPointer();
    vtkSmartPointer< Alder::Modality > modality;
    exam->GetRecord( modality );
    modalityName = modality->Get( "Name" ).ToString();
    
    // NOTE: it is possible that an exam with state "Ready" has valid data, but we are leaving
    // those exams out for now since we don't know for sure whether they are always valid
    updateItemText[modalityName] = true;
    if( "Completed" == exam->Get( "Stage" ).ToString() ) examCount[modalityName]++;
    if( exam->IsRatedBy( user ) ) ratedCount[modalityName]++;
  }

  // set the text, if updated
  std::map< std::string, bool >::iterator updateItemTextIt;
  for( updateItemTextIt = updateItemText.begin();
       updateItemTextIt != updateItemText.end();
       ++updateItemTextIt )
  {
    modalityName = updateItemTextIt->first;
    if( updateItemTextIt->second )
    {
      itemText[modalityName] = QString::number( ratedCount[modalityName] );
      itemText[modalityName] += tr( " of " );
      itemText[modalityName] += QString::number( examCount[modalityName] );
    }
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

    // add all modalities to the table
    std::map< std::string, QString >::iterator itemTextIt;
    for( itemTextIt = itemText.begin(); itemTextIt != itemText.end(); ++itemTextIt )
    {
      item = this->ui->interviewTableWidget->item( row, this->columnIndex[itemTextIt->first] );
      if( item ) item->setText( itemTextIt->second );
    }
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

        // add all modalities (one per column)
        std::vector< vtkSmartPointer< Alder::Modality > > modalityList;
        std::vector< vtkSmartPointer< Alder::Modality > >::iterator modalityListIt;
        Alder::Modality::GetAll( &modalityList );
        for( modalityListIt = modalityList.begin();
             modalityListIt != modalityList.end(); ++modalityListIt )
        {
          item = new QTableWidgetItem;
          item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
          this->ui->interviewTableWidget->setItem(
            0, this->columnIndex[(*modalityListIt)->Get( "Name" ).ToString()], item );
        }

        this->updateRow( 0, interview );
      }
    }

    this->ui->interviewTableWidget->sortItems( this->sortColumn, this->sortOrder );
  }
}
