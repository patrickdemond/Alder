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

#include <QMessageBox>

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QChangePasswordDialog::QChangePasswordDialog( QString password, QWidget* parent )
  : QDialog( parent )
{
  this->originalPassword = password; // the hashed password
  this->newPassword = "";

  this->ui = new Ui_QChangePasswordDialog;
  this->ui->setupUi( this );

  QObject::connect(
    this->ui->buttonBox, SIGNAL( accepted() ),
    this, SLOT( slotAccepted() ) );

  QObject::connect( 
    this->ui->newPasswordLineEdit, SIGNAL( editingFinished() ),
    this, SLOT( slotNewPassword() ) );

  QObject::connect( 
    this->ui->confirmPasswordLineEdit, SIGNAL( editingFinished() ),
    this, SLOT( slotPasswordConfirmed() ) );

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

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QChangePasswordDialog::slotNewPassword()
{
  std::string password = this->ui->newPasswordLineEdit->text().toStdString();
  std::string hashed;
  Alder::Utilities::hashString( password, hashed );
  bool error = false;
  std::string errStr;
  if( password.empty() ) 
  {
    errStr = "empty";
    error = true;
  }
  else if( password.size() < 6 )
  {
    errStr = "too short";
    error = true;
  }
  else if( this->originalPassword.toStdString() == hashed )
  {
    errStr = "no change";
    error = true;
  }

  if( error )
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
  else
  {
    this->ui->confirmPasswordLineEdit->setDisabled( false );
    this->ui->confirmPasswordLineEdit->setFocus();
  }  
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QChangePasswordDialog::slotPasswordConfirmed()
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
    return;
  }
  else
  {
    this->newPassword = password1;
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QChangePasswordDialog::slotAccepted()
{
  emit passwordChange( this->newPassword  );
  accept();
}

