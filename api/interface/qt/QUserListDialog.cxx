/*=========================================================================

  Program:  CLSAVis (Canadian Longitudinal Study on Aging Visualizer)
  Module:   QUserListDialog.cxx
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/
#include "QUserListDialog.h"
#include "ui_QUserListDialog.h"

#include "Application.h"
#include "Database.h"
#include "User.h"
#include "Utilities.h"

#include "vtkSmartPointer.h"

#include <QErrorMessage>
#include <QFile>
#include <QInputDialog>
#include <QList>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextStream>

#include <stdexcept>
#include <vector>

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QUserListDialog::QUserListDialog( QWidget* parent )
  : QDialog( parent )
{
  this->ui = new Ui_QUserListDialog;
  this->ui->setupUi( this );
  this->ui->userTableWidget->horizontalHeader()->setResizeMode( QHeaderView::Stretch );
  this->ui->userTableWidget->horizontalHeader()->setClickable( true );
  this->ui->userTableWidget->verticalHeader()->setVisible( false );
  this->ui->userTableWidget->setSelectionBehavior( QAbstractItemView::SelectRows );
  this->ui->userTableWidget->setSelectionMode( QAbstractItemView::SingleSelection );

  this->sortColumn = 0;
  this->sortOrder = Qt::AscendingOrder;

  QObject::connect(
    this->ui->addPushButton, SIGNAL( clicked( bool ) ),
    this, SLOT( slotAdd() ) );
  QObject::connect(
    this->ui->removePushButton, SIGNAL( clicked( bool ) ),
    this, SLOT( slotRemove() ) );
  QObject::connect(
    this->ui->resetPasswordPushButton, SIGNAL( clicked( bool ) ),
    this, SLOT( slotResetPassword() ) );
  QObject::connect(
    this->ui->closePushButton, SIGNAL( clicked( bool ) ),
    this, SLOT( slotClose() ) );
  QObject::connect(
    this->ui->userTableWidget, SIGNAL( itemSelectionChanged() ),
    this, SLOT( slotSelectionChanged() ) );
  QObject::connect(
    this->ui->userTableWidget->horizontalHeader(), SIGNAL( sectionClicked( int ) ),
    this, SLOT( slotHeaderClicked( int ) ) );
  
  this->updateInterface();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QUserListDialog::~QUserListDialog()
{
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QUserListDialog::slotAdd()
{
  // get the new user's name
  QString text = QInputDialog::getText(
    this,
    QObject::tr( "Create User" ),
    QObject::tr( "New user's name:" ),
    QLineEdit::Normal );
  
  if( !text.isEmpty() )
  {
    vtkSmartPointer< Alder::User > user = vtkSmartPointer< Alder::User >::New();
    user->Set( "name", text.toStdString() );
    user->ResetPassword();
    user->Save();
    this->updateInterface();
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QUserListDialog::slotRemove()
{
  QTableWidgetItem* item;
  QList<QTableWidgetItem *> list = this->ui->userTableWidget->selectedItems();
  if( 0 == list.size() ) return;
  for( int i = 0; i < list.size(); ++i )
  {
    item = list.at( i );
    if( 0 == item->column() )
    {
      vtkSmartPointer< Alder::User > user = vtkSmartPointer< Alder::User >::New();
      user->Load( "name", item->text().toStdString() );
      user->Remove();
    }
  }
  this->updateInterface();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QUserListDialog::slotResetPassword()
{
  QTableWidgetItem* item;
  QList<QTableWidgetItem *> list = this->ui->userTableWidget->selectedItems();
  if( 0 == list.size() ) return;

  for( int i = 0; i < list.size(); ++i )
  {
    item = list.at( i );
    if( 0 == item->column() )
    {
      vtkSmartPointer< Alder::User > user = vtkSmartPointer< Alder::User >::New();
      user->Load( "name", item->text().toStdString() );
      user->ResetPassword();
      user->Save();
    }
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QUserListDialog::slotClose()
{
  this->accept();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QUserListDialog::slotSelectionChanged()
{
  QList<QTableWidgetItem *> list = this->ui->userTableWidget->selectedItems();
  this->ui->removePushButton->setEnabled( 0 != list.size() );
  this->ui->resetPasswordPushButton->setEnabled( 0 != list.size() );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QUserListDialog::slotHeaderClicked( int index )
{
  // reverse order if already sorted
  if( this->sortColumn == index )
    this->sortOrder = Qt::AscendingOrder == this->sortOrder ? Qt::DescendingOrder : Qt::AscendingOrder;
  this->sortColumn = index;
  this->updateInterface();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QUserListDialog::updateInterface()
{
  this->ui->userTableWidget->setRowCount( 0 );
  QTableWidgetItem *item;
  
  std::vector< vtkSmartPointer< Alder::User > > userList;
  Alder::User::GetAll( &userList );
  std::vector< vtkSmartPointer< Alder::User > >::iterator it;
  for( it = userList.begin(); it != userList.end(); ++it )
  { // for every user, add a new row
    Alder::User *user = (*it);
    this->ui->userTableWidget->insertRow( 0 );

    // add name to row
    item = new QTableWidgetItem;
    item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
    item->setText( QString( user->Get( "name" ).ToString().c_str() ) );
    this->ui->userTableWidget->setItem( 0, 0, item );

  // add last login to row
    item = new QTableWidgetItem;
    item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
    item->setText( QString( user->Get( "last_login" ).ToString().c_str() ) );
    this->ui->userTableWidget->setItem( 0, 1, item );
  }

  this->ui->userTableWidget->sortItems( this->sortColumn, this->sortOrder );
}
