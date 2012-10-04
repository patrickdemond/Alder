/*=========================================================================

  Program:  Alder (CLSA Ultrasound Image Viewer)
  Module:   QAboutDialog.h
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>

=========================================================================*/

#ifndef __QAboutDialog_h
#define __QAboutDialog_h

#include <QDialog>

#include "Utilities.h"

class Ui_QAboutDialog;

class QAboutDialog : public QDialog
{
  Q_OBJECT

public:
  //constructor
  QAboutDialog( QWidget* parent = 0 );
  //destructor
  ~QAboutDialog();
  
public slots:

protected:

protected slots:

private:
  // Designer form
  Ui_QAboutDialog *ui;
};

#endif
