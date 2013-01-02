/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QAboutDialog.h
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

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
