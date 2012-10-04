/*=========================================================================

  Program:  CLSAVis (Canadian Longitudinal Study on Aging Visualizer)
  Module:   QAboutDialog.cxx
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>

=========================================================================*/
#include "QAboutDialog.h"
#include "ui_QAboutDialog.h"

#include <QFile>
#include <QTextStream>

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAboutDialog::QAboutDialog( QWidget* parent )
  : QDialog( parent )
{
  this->ui = new Ui_QAboutDialog;
  this->ui->setupUi( this );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QAboutDialog::~QAboutDialog()
{
}
