/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QStudyNoteDialog.h
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/

#ifndef __QStudyNoteDialog_h
#define __QStudyNoteDialog_h

#include <QDialog>

class Ui_QStudyNoteDialog;

class QAbstractButton;

class QStudyNoteDialog : public QDialog
{
  Q_OBJECT

public:
  //constructor
  QStudyNoteDialog( QWidget* parent = 0 );
  //destructor
  ~QStudyNoteDialog();
  
public slots:
  void close();

protected:
  void updateInterface();

protected slots:

private:
  // Designer form
  Ui_QStudyNoteDialog *ui;
};

#endif
