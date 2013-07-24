/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QExamNoteDialog.h
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/

#ifndef __QExamNoteDialog_h
#define __QExamNoteDialog_h

#include <QDialog>

class Ui_QExamNoteDialog;

class QAbstractButton;

class QExamNoteDialog : public QDialog
{
  Q_OBJECT

public:
  //constructor
  QExamNoteDialog( QWidget* parent = 0 );
  //destructor
  ~QExamNoteDialog();
  
public slots:
  void close();

protected:
  void updateInterface();

protected slots:

private:
  // Designer form
  Ui_QExamNoteDialog *ui;
};

#endif
