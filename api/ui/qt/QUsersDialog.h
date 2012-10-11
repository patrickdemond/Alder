/*=========================================================================

  Program:  Alder (CLSA Ultrasound Image Viewer)
  Module:   QUsersDialog.h
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/

#ifndef __QUsersDialog_h
#define __QUsersDialog_h

#include <QDialog>

#include "Utilities.h"

class Ui_QUsersDialog;

class QUsersDialog : public QDialog
{
  Q_OBJECT

public:
  //constructor
  QUsersDialog( QWidget* parent = 0 );
  //destructor
  ~QUsersDialog();
  
public slots:
  virtual void slotAdd();
  virtual void slotRemove();
  virtual void slotResetPassword();
  virtual void slotClose();

protected:
  void PopulateUsersTable();

protected slots:

private:
  // Designer form
  Ui_QUsersDialog *ui;
};

#endif
