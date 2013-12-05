/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QChangePasswordDialog.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include <QChangePasswordDialog.h>
#include <ui_QChangePasswordDialog.h>

#include <Utilities.h>
#include <iostream>

#include <QKeyEvent>
#include <QPushButton>
#include <QMessageBox>

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QChangePasswordDialog::QChangePasswordDialog( QString const &pwd, QWidget* parent )
  : QDialog( parent )
{
  this->originalPassword = pwd; // the hashed password
  this->newPassword = "";

  this->ui = new Ui_QChangePasswordDialog;
  this->ui->setupUi( this );

  QObject::connect(
    this->ui->okButton, SIGNAL( clicked() ),
    this, SLOT( slotAccepted() ) );

  QObject::connect(
    this->ui->cancelButton, SIGNAL( clicked() ),
    this, SLOT( reject() ) );

  this->ui->newPasswordLineEdit->installEventFilter( this );   
  this->ui->confirmPasswordLineEdit->installEventFilter( this );   

  this->ui->newPasswordLineEdit->setFocus( Qt::PopupFocusReason );
  this->ui->newPasswordLineEdit->setEchoMode( QLineEdit::Password );
  this->ui->confirmPasswordLineEdit->setEchoMode( QLineEdit::Password );
  this->ui->confirmPasswordLineEdit->setDisabled( true );
  this->setWindowTitle( QString("Set Password") );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QChangePasswordDialog::~QChangePasswordDialog()
{
}

bool QChangePasswordDialog::eventFilter( QObject* watched, QEvent* event )
{
  if( watched == this->ui->newPasswordLineEdit )
  {
    if( event->type() == QEvent::KeyPress ) 
    {
      QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
      if( keyEvent->key() == Qt::Key_Return ||
          keyEvent->key() == Qt::Key_Enter ||
          keyEvent->key() == Qt::Key_Tab )
      {
        if( this->checkNewPassword() )
        {
          this->ui->confirmPasswordLineEdit->setDisabled( false );
          this->ui->confirmPasswordLineEdit->setFocus();
          return true;
        }
        else
        {
          this->ui->confirmPasswordLineEdit->setDisabled( true );
          return true;
        }  
      }  
      else
      {
        this->ui->confirmPasswordLineEdit->setDisabled( true );
        return false;
      }
    }
    else if( event->type() == QEvent::FocusIn )
    {
      this->ui->confirmPasswordLineEdit->setDisabled( true );
      return false;  
    }
    else
    {
      return false;
    }
  }
  else if( watched == this->ui->confirmPasswordLineEdit &&
           event->type() == QEvent::KeyPress )
  {
    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
    if( keyEvent->key() == Qt::Key_Return ||
        keyEvent->key() == Qt::Key_Enter ||
        keyEvent->key() == Qt::Key_Tab )
    {
      if( this->confirmNewPassword() )
      {
        this->slotAccepted();
      }
      return true;
    }
    return false;
  }
  else
    return false;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QChangePasswordDialog::checkNewPassword()
{
  std::string password = this->ui->newPasswordLineEdit->text().toStdString();
  bool noError = true;
  std::string errStr;
  if( password.empty() ) 
  {
    errStr = "empty";
    noError = false;
  }
  else if( password.size() < 6 )
  {
    errStr = "too short";
    noError = false;
  }
  else if( password == "password" )
  {
    errStr = "illegal";
    noError = false;
  }
  else
  {
    std::string hashed;
    Alder::Utilities::hashString( password, hashed );

    if( this->originalPassword.toStdString() == hashed )
    {
      errStr = "no change";
      noError = false;
    }
  }
  if( !noError )
  {
    QMessageBox errorMessage( this );
    errorMessage.setWindowModality( Qt::WindowModal );
    errorMessage.setIcon( QMessageBox::Warning );
    std::string msg = "Ivalid password (";
    msg += errStr;
    msg += "), please try again.";
    errorMessage.setText( tr( msg.c_str() ) );
    errorMessage.exec();
  }
  return noError;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QChangePasswordDialog::confirmNewPassword()
{
  QString password1 = this->ui->newPasswordLineEdit->text();
  QString password2 = this->ui->confirmPasswordLineEdit->text();
  if( password1 != password2 )
  {
    QMessageBox errorMessage( this );
    errorMessage.setWindowModality( Qt::WindowModal );
    errorMessage.setIcon( QMessageBox::Warning );
    errorMessage.setText( tr( "Confirmation does not match password, please try again." ) );
    errorMessage.exec();
    this->ui->confirmPasswordLineEdit->setFocus();
    this->newPassword = "";
    return false;
  }
  else
  {
    this->newPassword = password1;
    return true;
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QChangePasswordDialog::slotAccepted()
{
  if( !this->newPassword.isEmpty() ) 
  {
    emit passwordChange( this->newPassword  );
  }
  accept();
}

