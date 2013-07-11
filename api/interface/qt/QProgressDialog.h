/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QProgressDialog.h
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/

#ifndef __QProgressDialog_h
#define __QProgressDialog_h

#include <QDialog>

#include "vtkCommand.h"
#include "vtkSmartPointer.h"

class Ui_QProgressDialog;

class QProgressDialog : public QDialog
{
  Q_OBJECT
private:
  class Command : public vtkCommand
  {
  public:
    static Command *New() { return new Command; }
    void Execute( vtkObject *caller, unsigned long eventId, void *callData );
    Ui_QProgressDialog *ui;

  protected:
    Command() { this->ui = NULL; }
  };  

public:
  //constructor
  QProgressDialog( QWidget* parent = 0 );
  //destructor
  ~QProgressDialog();

  void setMessage( QString message );
  
public slots:
  virtual void slotCancel();

protected:
  vtkSmartPointer< Command > observer;

protected slots:

private:
  // Designer form
  Ui_QProgressDialog *ui;
};

#endif
