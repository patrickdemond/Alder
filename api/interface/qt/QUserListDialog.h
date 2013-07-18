/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QUserListDialog.h
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/

#ifndef __QUserListDialog_h
#define __QUserListDialog_h

#include <QDialog>

#include <map>

class QTableWidgetItem;
class Ui_QUserListDialog;

class QUserListDialog : public QDialog
{
  Q_OBJECT

public:
  //constructor
  QUserListDialog( QWidget* parent = 0 );
  //destructor
  ~QUserListDialog();
  
public slots:
  virtual void slotAdd();
  virtual void slotRemove();
  virtual void slotResetPassword();
  virtual void slotClose();
  virtual void slotSelectionChanged();
  virtual void slotHeaderClicked( int index );
  virtual void slotItemChanged( QTableWidgetItem* );

protected:
  void updateInterface();
  int sortColumn;
  Qt::SortOrder sortOrder;
  std::map< std::string, int > columnIndex;

protected slots:

private:
  // Designer form
  Ui_QUserListDialog *ui;
};

#endif
