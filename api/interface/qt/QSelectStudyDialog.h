/*=========================================================================

  Program:  Alder (CLSA Ultrasound Image Viewer)
  Module:   QSelectStudyDialog.h
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/

#ifndef __QSelectStudyDialog_h
#define __QSelectStudyDialog_h

#include <QDialog>

#include "Utilities.h"

class Ui_QSelectStudyDialog;

class QSelectStudyDialog : public QDialog
{
  Q_OBJECT

public:
  //constructor
  QSelectStudyDialog( QWidget* parent = 0 );
  //destructor
  ~QSelectStudyDialog();
  
public slots:
  virtual void slotSearch();
  virtual void slotAccepted();
  virtual void slotSelectionChanged();

protected:
  void UpdateInterface();
  QString searchText;

protected slots:

private:
  // Designer form
  Ui_QSelectStudyDialog *ui;
};

#endif
