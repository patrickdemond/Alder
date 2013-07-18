/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
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
  // define the column indeces
  this->columnIndex["Name"] = 0;
  this->columnIndex["RateDexa"] = 1;
  this->columnIndex["RateRetinal"] = 2;
  this->columnIndex["RateUltrasound"] = 3;
  this->columnIndex["LastLogin"] = 4;

  this->ui = new Ui_QUserListDialog;
  this->ui->setupUi( this );
  this->ui->userTableWidget->horizontalHeader()->setResizeMode(
    this->columnIndex["Name"], QHeaderView::Stretch );
  this->ui->userTableWidget->horizontalHeader()->setResizeMode(
    this->columnIndex["RateDexa"], QHeaderView::ResizeToContents );
  this->ui->userTableWidget->horizontalHeader()->setResizeMode(
    this->columnIndex["RateRetinal"], QHeaderView::ResizeToContents );
  this->ui->userTableWidget->horizontalHeader()->setResizeMode(
    this->columnIndex["RateUltrasound"], QHeaderView::ResizeToContents );
  this->ui->userTableWidget->horizontalHeader()->setResizeMode(
    this->columnIndex["LastLogin"], QHeaderView::Stretch );
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
  QObject::connect(
    this->ui->userTableWidget, SIGNAL( itemChanged( QTableWidgetItem* ) ),
    this, SLOT( slotItemChanged( QTableWidgetItem* ) ) );

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
  std::string name = QInputDialog::getText(
    this,
    QObject::tr( "Create User" ),
    QObject::tr( "New user's name:" ),
    QLineEdit::Normal ).toStdString();

  if( 0 < name.length() )
  {
    // make sure the user name doesn't already exist
    vtkSmartPointer< Alder::User > user = vtkSmartPointer< Alder::User >::New();
    if( user->Load( "Name", name ) )
    {
      std::stringstream stream;
      stream << "Unable to create new user \"" << name << "\", name already in use.";
      QErrorMessage *dialog = new QErrorMessage( this );
      dialog->setModal( true );
      dialog->showMessage( tr( stream.str().c_str() ) );
    }
    else
    {
      user->Set( "Name", name );
      user->ResetPassword();
      user->Save();
      this->updateInterface();
    }
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
    if( this->columnIndex["Name"] == item->column() )
    {
      vtkSmartPointer< Alder::User > user = vtkSmartPointer< Alder::User >::New();
      user->Load( "Name", item->text().toStdString() );
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
    if( this->columnIndex["Name"] == item->column() )
    {
      vtkSmartPointer< Alder::User > user = vtkSmartPointer< Alder::User >::New();
      user->Load( "Name", item->text().toStdString() );
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

  // do not allow resetting the password to the admin account
  if( 0 != list.size() )
  {
    QTableWidgetItem *item = list.at( this->columnIndex["Name"] );
    if( 0 == item->column() && "administrator" == item->text() )
      this->ui->resetPasswordPushButton->setEnabled( false );
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QUserListDialog::slotHeaderClicked( int index )
{
  // NOTE: currently the columns with checkboxes cannot be sorted.  In order to do this we would need
  // to either override QSortFilterProxyModel::lessThan() or QAbstractTableModel::sort()
  // For now we'll just ignore requests to sort by these columns
  if( this->columnIndex["RateDexa"] != index &&
      this->columnIndex["RateRetinal"] != index &&
      this->columnIndex["RateUltrasound"] != index )
  {
    // reverse order if already sorted
    if( this->sortColumn == index )
      this->sortOrder = Qt::AscendingOrder == this->sortOrder ? Qt::DescendingOrder : Qt::AscendingOrder;
    this->sortColumn = index;
    this->ui->userTableWidget->sortItems( this->sortColumn, this->sortOrder );
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QUserListDialog::slotItemChanged( QTableWidgetItem *item )
{
  // get the user
  vtkSmartPointer< Alder::User > user = vtkSmartPointer< Alder::User >::New();
  user->Load( "Name",
    this->ui->userTableWidget->item( item->row(), this->columnIndex["Name"] )->text().toStdString() );

  // update the user's rate settings
  if( this->columnIndex["RateDexa"] == item->column() )
    user->Set( "RateDexa", Qt::Checked == item->checkState() ? 1 : 0 );
  else if( this->columnIndex["RateRetinal"] == item->column() )
    user->Set( "RateRetinal", Qt::Checked == item->checkState() ? 1 : 0 );
  else if( this->columnIndex["RateUltrasound"] == item->column() )
    user->Set( "RateUltrasound", Qt::Checked == item->checkState() ? 1 : 0 );

  user->Save();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QUserListDialog::updateInterface()
{
  this->ui->userTableWidget->blockSignals( true );

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
    item->setText( QString( user->Get( "Name" ).ToString().c_str() ) );
    this->ui->userTableWidget->setItem( 0, this->columnIndex["Name"], item );

    // add rate dexa to row
    item = new QTableWidgetItem;
    item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable );
    item->setData( Qt::EditRole, 0 < user->Get( "RateDexa" ).ToInt() ? "1" : "0" );
    item->setCheckState( 0 < user->Get( "RateDexa" ).ToInt() ? Qt::Checked : Qt::Unchecked );
    this->ui->userTableWidget->setItem( 0, this->columnIndex["RateDexa"], item );

    // add rate retinal to row
    item = new QTableWidgetItem;
    item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable );
    item->setData( Qt::EditRole, 0 < user->Get( "RateRetinal" ).ToInt() ? "1" : "0" );
    item->setCheckState( 0 < user->Get( "RateRetinal" ).ToInt() ? Qt::Checked : Qt::Unchecked );
    this->ui->userTableWidget->setItem( 0, this->columnIndex["RateRetinal"], item );

    // add rate ultrasound to row
    item = new QTableWidgetItem;
    item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable );
    item->setData( Qt::EditRole, 0 < user->Get( "RateUltrasound" ).ToInt() ? "1" : "0" );
    item->setCheckState( 0 < user->Get( "RateUltrasound" ).ToInt() ? Qt::Checked : Qt::Unchecked );
    this->ui->userTableWidget->setItem( 0, this->columnIndex["RateUltrasound"], item );

    // add last login to row
    item = new QTableWidgetItem;
    item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
    item->setText( QString( user->Get( "LastLogin" ).ToString().c_str() ) );
    this->ui->userTableWidget->setItem( 0, this->columnIndex["LastLogin"], item );
  }

  this->ui->userTableWidget->sortItems( this->sortColumn, this->sortOrder );

  this->ui->userTableWidget->blockSignals( false );
}
