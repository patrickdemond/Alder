/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QLoginDialog.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include "QLoginDialog.h"
#include "ui_QLoginDialog.h"

#include "Application.h"
#include "Modality.h"
#include "User.h"

#include "vtkSmartPointer.h"

#include <QFile>
#include <QInputDialog>
#include <QMessageBox>
#include <QTextStream>

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QLoginDialog::QLoginDialog( QWidget* parent )
  : QDialog( parent )
{
  this->ui = new Ui_QLoginDialog;
  this->ui->setupUi( this );

  QObject::connect(
    this->ui->buttonBox, SIGNAL( accepted() ),
    this, SLOT( slotAccepted() ) );

  this->ui->passwordLineEdit->setEchoMode( QLineEdit::Password );
  this->ui->usernameLineEdit->setFocus( Qt::PopupFocusReason ); // make sure username box is focused
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QLoginDialog::~QLoginDialog()
{
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QLoginDialog::slotAccepted()
{
  std::string password = this->ui->passwordLineEdit->text().toStdString();

  vtkSmartPointer< Alder::User > user = vtkSmartPointer< Alder::User >::New();
  if( user->Load( "Name", this->ui->usernameLineEdit->text().toStdString() ) && user->IsPassword( password ) )
  { // login successful
    // if the password matches the default password, force the user to change it
    while( 0 == password.compare( Alder::User::GetDefaultPassword() ) )
    {
      // prompt for new password
      QString password1 = QInputDialog::getText(
        this,
        QObject::tr( "Change Password" ),
        QObject::tr( "Please provide a new password (cannot be \"password\") for your account:" ),
        QLineEdit::Password );

      if( !password1.isEmpty() && password1 != QString( Alder::User::GetDefaultPassword().c_str() ) )
      {
        // re-prompt to repeat password
        QString password2 = QInputDialog::getText(
          this,
          QObject::tr( "Re-type Password" ),
          QObject::tr( "Please verify your new password by typing it again:" ),
          QLineEdit::Password );

        if( password1 == password2 )
        {
          // set the replacement password
          password = password1.toStdString();
          user->Set( "Password", password );
          user->Save();
        }
      }
    }

    // log in the user and mark login time
    Alder::Application::GetInstance()->SetActiveUser( user );
    user->Set( "LastLogin", Alder::Utilities::getTime( "%Y-%m-%d %H:%M:%S" ) );
    user->Save();
    this->accept();
  }
  else
  { // login failed
    QMessageBox errorMessage( this );
    errorMessage.setWindowModality( Qt::WindowModal );
    errorMessage.setIcon( QMessageBox::Warning );
    errorMessage.setText( tr( "Invalid username or password, please try again." ) );
    errorMessage.exec();
  }
}
