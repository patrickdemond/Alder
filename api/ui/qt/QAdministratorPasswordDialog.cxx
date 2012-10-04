/*=========================================================================

  Program:  CLSAVis (Canadian Longitudinal Study on Aging Visualizer)
  Module:   QAdministratorPasswordDialog.cxx
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/
#include "QAdministratorPasswordDialog.h"
#include "ui_QAdministratorPasswordDialog.h"

#include <QFile>
#include <QTextStream>

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAdministratorPasswordDialog::QAdministratorPasswordDialog( QWidget* parent )
  : QDialog( parent )
{
  this->ui = new Ui_QAdministratorPasswordDialog;
  this->ui->setupUi( this );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAdministratorPasswordDialog::~QAdministratorPasswordDialog()
{
}
