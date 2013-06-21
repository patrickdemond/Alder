/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QSelectInterviewDialog.h
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/

#ifndef __QSelectInterviewDialog_h
#define __QSelectInterviewDialog_h

#include <QDialog>

namespace Alder { class Interview; };
class Ui_QSelectInterviewDialog;

class QSelectInterviewDialog : public QDialog
{
  Q_OBJECT

public:
  //constructor
  QSelectInterviewDialog( QWidget* parent = 0 );
  //destructor
  ~QSelectInterviewDialog();
  
public slots:
  virtual void slotSearch();
  virtual void slotAccepted();
  virtual void slotSelectionChanged();
  virtual void slotHeaderClicked( int index );

protected:
  void updateRow( int, Alder::Interview* );
  void updateInterface();
  QString searchText;
  int sortColumn;
  Qt::SortOrder sortOrder;

protected slots:

private:
  // Designer form
  Ui_QSelectInterviewDialog *ui;
};

#endif
