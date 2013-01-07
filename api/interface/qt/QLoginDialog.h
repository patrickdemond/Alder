/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QLoginDialog.h
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/

#ifndef __QLoginDialog_h
#define __QLoginDialog_h

#include <QDialog>

class Ui_QLoginDialog;

class QLoginDialog : public QDialog
{
  Q_OBJECT

public:
  //constructor
  QLoginDialog( QWidget* parent = 0 );
  //destructor
  ~QLoginDialog();
  
public slots:
  virtual void slotAccepted();

protected:

protected slots:

private:
  // Designer form
  Ui_QLoginDialog *ui;
};

#endif
