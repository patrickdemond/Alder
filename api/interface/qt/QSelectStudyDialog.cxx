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
  this->ui->studyTableWidget->horizontalHeader()->setVisible( false );
  this->ui->studyTableWidget->verticalHeader()->setVisible( false );
  this->ui->studyTableWidget->setSelectionBehavior( QAbstractItemView::SelectItems );
  this->ui->studyTableWidget->setSelectionMode( QAbstractItemView::SingleSelection );
  this->ui->studyTableWidget->setColumnCount( QSelectStudyDialog::ColumnCount );
  this->searchText = "";

  QObject::connect(
    this->ui->searchPushButton, SIGNAL( clicked( bool ) ),
    this, SLOT( slotSearch() ) );
  QObject::connect(
    this->ui->buttonBox, SIGNAL( accepted() ),
    this, SLOT( slotAccepted() ) );
  QObject::connect(
    this->ui->studyTableWidget, SIGNAL( itemSelectionChanged() ),
    this, SLOT( slotSelectionChanged() ) );

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
void QSelectStudyDialog::updateInterface()
{
  this->ui->studyTableWidget->setRowCount( 0 );
  QTableWidgetItem *item;
  vtkVariant *name, *login;
  
  int index = 0;
  std::vector< std::string > studyList = Alder::Study::GetIdentifierList();
  std::vector< std::string >::iterator it;
  for( it = studyList.begin(); it != studyList.end(); ++it )
  {
    QString identifier = tr( it->c_str() );
    // if the search text isn't empty then only add studies with a partial match
    if( this->searchText.isEmpty() || identifier.contains( this->searchText, Qt::CaseInsensitive ) )
    {
      // for every fifth study (starting with the first), add a new row
      int column = index % QSelectStudyDialog::ColumnCount;
      if( 0 == column ) this->ui->studyTableWidget->insertRow( this->ui->studyTableWidget->rowCount() );
  
      // add the study's identifier to the table
      item = new QTableWidgetItem;
      item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
      item->setText( tr( it->c_str() ) );
      this->ui->studyTableWidget->setItem( this->ui->studyTableWidget->rowCount() - 1, column, item );
  
      index++;
    }
  }
}
