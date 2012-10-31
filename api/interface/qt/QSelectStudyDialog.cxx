/*=========================================================================

  Program:  CLSAVis (Canadian Longitudinal Study on Aging Visualizer)
  Module:   QSelectStudyDialog.cxx
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/
#include "QSelectStudyDialog.h"
#include "ui_QSelectStudyDialog.h"

#include "Application.h"
#include "Database.h"
#include "Study.h"
#include "Utilities.h"

#include "vtkSmartPointer.h"

#include <QInputDialog>
#include <QList>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>

#include <vector>

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QSelectStudyDialog::QSelectStudyDialog( QWidget* parent )
  : QDialog( parent )
{
  this->ui = new Ui_QSelectStudyDialog;
  this->ui->setupUi( this );
  this->ui->studyTableWidget->horizontalHeader()->setResizeMode( QHeaderView::Stretch );
  this->ui->studyTableWidget->horizontalHeader()->setClickable( true );
  this->ui->studyTableWidget->verticalHeader()->setVisible( false );
  this->ui->studyTableWidget->setSelectionBehavior( QAbstractItemView::SelectRows );
  this->ui->studyTableWidget->setSelectionMode( QAbstractItemView::SingleSelection );

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
    this->ui->studyTableWidget, SIGNAL( itemSelectionChanged() ),
    this, SLOT( slotSelectionChanged() ) );
  QObject::connect(
    this->ui->studyTableWidget->horizontalHeader(), SIGNAL( sectionClicked( int ) ),
    this, SLOT( slotHeaderClicked( int ) ) );

  this->updateInterface();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QSelectStudyDialog::~QSelectStudyDialog()
{
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QSelectStudyDialog::slotSearch()
{
  bool ok;
  QString text = QInputDialog::getText(
    this,
    QObject::tr( "Search Term" ),
    QObject::tr( "Provide some or all of the study to search for:" ),
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
void QSelectStudyDialog::slotAccepted()
{
  vtkSmartPointer< Alder::Study > study = vtkSmartPointer< Alder::Study >::New();
  // study->Load( "uid", TODO: get study UID from UI
  Alder::Application::GetInstance()->SetActiveStudy( study );
  this->accept();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QSelectStudyDialog::slotSelectionChanged()
{
  QList<QTableWidgetItem *> list = this->ui->studyTableWidget->selectedItems();
  this->ui->buttonBox->button( QDialogButtonBox::Ok )->setEnabled( 0 != list.size() );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QSelectStudyDialog::slotHeaderClicked( int index )
{
  // reverse order if already sorted
  if( this->sortColumn == index )
    this->sortOrder = Qt::AscendingOrder == this->sortOrder ? Qt::DescendingOrder : Qt::AscendingOrder;
  this->sortColumn = index;
  this->updateInterface();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QSelectStudyDialog::updateInterface()
{
  this->ui->studyTableWidget->setRowCount( 0 );
  QTableWidgetItem *item;
  vtkVariant *v;
  
  std::vector< vtkSmartPointer< Alder::Study > > studyList;
  Alder::Study::GetAll( &studyList );
  std::vector< vtkSmartPointer< Alder::Study > >::iterator it;
  for( it = studyList.begin(); it != studyList.end(); ++it )
  { // for every study, add a new row
    Alder::Study *study = (*it);
    QString uid = tr( study->Get( "uid" )->ToString().c_str() );

    if( this->searchText.isEmpty() || uid.contains( this->searchText, Qt::CaseInsensitive ) )
    {
      this->ui->studyTableWidget->insertRow( 0 );

      // add uid to row
      v = study->Get( "uid" );
      item = new QTableWidgetItem;
      item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
      item->setText( uid );
      this->ui->studyTableWidget->setItem( 0, 0, item );

      // add site to row
      v = study->Get( "site" );
      item = new QTableWidgetItem;
      item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
      item->setText( tr( v ? v->ToString().c_str() : "" ) );
      this->ui->studyTableWidget->setItem( 0, 1, item );

      // add interviewer to row
      v = study->Get( "interviewer" );
      item = new QTableWidgetItem;
      item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
      item->setText( tr( v ? v->ToString().c_str() : "" ) );
      this->ui->studyTableWidget->setItem( 0, 2, item );

      // add datetime_acquired to row
      v = study->Get( "datetime_acquired" );
      item = new QTableWidgetItem;
      item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
      item->setText( tr( v ? v->ToString().c_str() : "" ) );
      this->ui->studyTableWidget->setItem( 0, 3, item );
    }
  }

  this->ui->studyTableWidget->sortItems( this->sortColumn, this->sortOrder );
}
