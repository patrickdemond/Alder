/*=========================================================================

  Program:  CLSAVis (Canadian Longitudinal Study on Aging Visualizer)
  Module:   QUsersDialog.cxx
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/
#include "QUsersDialog.h"
#include "ui_QUsersDialog.h"

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
QUsersDialog::QUsersDialog( QWidget* parent )
  : QDialog( parent )
{
  this->ui = new Ui_QUsersDialog;
  this->ui->setupUi( this );
  this->ui->usersTableWidget->horizontalHeader()->setResizeMode( QHeaderView::Stretch );
  this->ui->usersTableWidget->verticalHeader()->setVisible( false );
  this->ui->usersTableWidget->setSelectionBehavior( QAbstractItemView::SelectRows );
  this->ui->usersTableWidget->setSelectionMode( QAbstractItemView::SingleSelection );

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
    this->ui->usersTableWidget, SIGNAL( itemSelectionChanged() ),
    this, SLOT( slotSelectionChanged() ) );
  
  this->PopulateUsersTable();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QUsersDialog::~QUsersDialog()
{
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QUsersDialog::slotAdd()
{
  // get the new user's name
  QString text = QInputDialog::getText(
    this,
    QObject::tr( "Create User" ),
    QObject::tr( "New user's name:" ),
    QLineEdit::Normal );
  
  if( !text.isEmpty() )
  {
    Alder::Application::GetInstance()->GetDB()->AddUser( text.toStdString() );
    this->PopulateUsersTable();
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QUsersDialog::slotRemove()
{
  QTableWidgetItem* item;
  QList<QTableWidgetItem *> list = this->ui->usersTableWidget->selectedItems();
  if( 0 == list.size() ) return;
  for( int i = 0; i < list.size(); ++i )
  {
    item = list.at( i );
    if( 0 == item->column() )
      Alder::Application::GetInstance()->GetDB()->RemoveUser( item->text().toStdString() );
  }
  this->PopulateUsersTable();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QUsersDialog::slotResetPassword()
{
  QList<QTableWidgetItem *> list = this->ui->usersTableWidget->selectedItems();
  if( 0 == list.size() ) return;
  // TODONEXT: what would be nice is if any model object which is directly related
  // to a table in the DB woudl have a Save() method (like below) which would
  // write that object's ivars to the DB (ie: active record)
  // This should be made generic, with maybe a ActiveRecord base class

  /* for instance...
  for( int i = 0; i < list.size(); ++i )
  {
    item = list.at( i );
    if( 0 == item->column() )
    {
      vtkSmartPointer<User> user = Alder::User::GetUser( item->text().toStdString() );
      if( NULL != user )
      {
        user->SetPassword( "password" );
        user->Save();
      }
    }
  }
  */
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QUsersDialog::slotClose()
{
  this->accept();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QUsersDialog::slotSelectionChanged()
{
  QList<QTableWidgetItem *> list = this->ui->usersTableWidget->selectedItems();
  this->ui->removePushButton->setEnabled( 0 != list.size() );
  this->ui->resetPasswordPushButton->setEnabled( 0 != list.size() );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QUsersDialog::PopulateUsersTable()
{
//  this->ui->usersTableWidget->clear();
  this->ui->usersTableWidget->setRowCount( 0 );
  QTableWidgetItem *item;
  
  std::vector< vtkSmartPointer<Alder::User> > users =
    Alder::Application::GetInstance()->GetDB()->GetUsers();
  std::vector< vtkSmartPointer<Alder::User> >::iterator it;
  for( it = users.begin(); it != users.end(); ++it )
  { // for every user, add a new row
    Alder::User *user = (*it);
    this->ui->usersTableWidget->insertRow( 0 );

    // add name to row
    item = new QTableWidgetItem;
    item->setText( tr( user->name.c_str() ) );
    this->ui->usersTableWidget->setItem( 0, 0, item );

    // add last login to row
    item = new QTableWidgetItem;
    item->setText( tr( user->lastLogin.c_str() ) );
    this->ui->usersTableWidget->setItem( 0, 1, item );

    // add created tiem to row
    item = new QTableWidgetItem;
    item->setText( tr( user->createdOn.c_str() ) );
    this->ui->usersTableWidget->setItem( 0, 2, item );
  }
}
