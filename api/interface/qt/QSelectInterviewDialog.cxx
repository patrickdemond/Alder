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
#include "Interview.h"
#include "Study.h"

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
    QTableWidgetItem *item = list.first();
    interview = vtkSmartPointer< Alder::Interview >::New();
    interview->Load( "UId", item->text().toStdString() );
  }

  Alder::Application::GetInstance()->SetActiveInterview( interview );
  this->accept();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QSelectInterviewDialog::slotSelectionChanged()
{
  QList<QTableWidgetItem *> list = this->ui->interviewTableWidget->selectedItems();
  this->ui->buttonBox->button( QDialogButtonBox::Ok )->setEnabled( 0 != list.size() );
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
  std::vector< vtkSmartPointer< Alder::Study > > studyList;
  std::vector< vtkSmartPointer< Alder::Study > >::iterator studyIt;
  Alder::Study *study;
  
  std::vector< vtkSmartPointer< Alder::Interview > > interviewList;
  Alder::Interview::GetAll( &interviewList );
  std::vector< vtkSmartPointer< Alder::Interview > >::iterator it;
  for( it = interviewList.begin(); it != interviewList.end(); ++it )
  { // for every interview, add a new row
    Alder::Interview *interview = *it;
    QString UId = QString( interview->Get( "UId" ).ToString().c_str() );
    
    // get the list of studies associated with this interview
    QString date = tr( "N/A" );
    QString dexa = tr( "missing" );
    QString ultrasound = tr( "missing" );
    QString retinal = tr( "missing" );
    interview->GetList( &studyList );

    // find the date of any study and whether each modality was acquired
    for( studyIt = studyList.begin(); studyIt != studyList.end(); ++studyIt )
    {
      study = studyIt->GetPointer();
      std::string modality = study->Get( "Modality" ).ToString();
      if( "Dexa" == modality ) dexa = study->IsRatedBy( user ) ? "rated" : "available";
      else if( "Ultrasound" == modality ) ultrasound = study->IsRatedBy( user ) ? "rated" : "available";
      else if( "Retinal" == modality ) retinal = study->IsRatedBy( user ) ? "rated" : "available";
      // TODO: log if an unknown modality is found
    }

    if( this->searchText.isEmpty() || UId.contains( this->searchText, Qt::CaseInsensitive ) )
    {
      this->ui->interviewTableWidget->insertRow( 0 );

      // add UId to row
      item = new QTableWidgetItem;
      item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
      item->setText( UId );
      this->ui->interviewTableWidget->setItem( 0, 0, item );

      // add dexa interview to row
      item = new QTableWidgetItem;
      item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
      item->setText( dexa );
      this->ui->interviewTableWidget->setItem( 0, 1, item );

      // add ultrasound interview to row
      item = new QTableWidgetItem;
      item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
      item->setText( ultrasound );
      this->ui->interviewTableWidget->setItem( 0, 2, item );

      // add retinal interview to row
      item = new QTableWidgetItem;
      item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
      item->setText( retinal );
      this->ui->interviewTableWidget->setItem( 0, 3, item );

      // add visit date to row
      item = new QTableWidgetItem;
      item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
      item->setText( QString( interview->Get( "VisitDate" ).ToString().c_str() ) );
      this->ui->interviewTableWidget->setItem( 0, 4, item );
    }
  }

  this->ui->interviewTableWidget->sortItems( this->sortColumn, this->sortOrder );
}
