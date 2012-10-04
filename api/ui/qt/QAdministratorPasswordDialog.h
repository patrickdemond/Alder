/*=========================================================================

  Program:  Alder (CLSA Ultrasound Image Viewer)
  Module:   QAdministratorPasswordDialog.h
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/

#ifndef __QAdministratorPasswordDialog_h
#define __QAdministratorPasswordDialog_h

#include <QDialog>

#include "Utilities.h"

class Ui_QAdministratorPasswordDialog;

class QAdministratorPasswordDialog : public QDialog
{
  Q_OBJECT

public:
  //constructor
  QAdministratorPasswordDialog( QWidget* parent = 0 );
  //destructor
  ~QAdministratorPasswordDialog();
  
public slots:

protected:

protected slots:

private:
  // Designer form
  Ui_QAdministratorPasswordDialog *ui;
};

#endif
