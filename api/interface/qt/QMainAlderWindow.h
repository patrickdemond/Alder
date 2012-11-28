/*=========================================================================

  Program:  Alder (CLSA Ultrasound Image Viewer)
  Module:   QMainAlderWindow.h
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/

#ifndef __QMainAlderWindow_h
#define __QMainAlderWindow_h

#include <QMainWindow>

#include "Utilities.h"

#include "vtkSmartPointer.h"

#include <map>

namespace Alder { class ActiveRecord; };
class Ui_QMainAlderWindow;
class QTreeWidgetItem;

class QMainAlderWindow : public QMainWindow
{
  Q_OBJECT

public:
  QMainAlderWindow( QWidget* parent = 0 );
  ~QMainAlderWindow();

  
public slots:
  // action event functions
  virtual void slotOpenStudy();
  virtual void slotPreviousStudy();
  virtual void slotNextStudy();
  virtual void slotLogin();
  virtual void slotUserManagement();
  virtual void slotUpdateStudyDatabase();
  virtual void slotTreeSelectionChanged();
  virtual void slotRatingSliderChanged( int );

  // help event functions
  virtual void slotAbout();
  virtual void slotManual();

protected:
  // called whenever the main window is closed
  virtual void closeEvent( QCloseEvent *event );

  // read/write application GUI settings
  virtual void readSettings();
  virtual void writeSettings();
  virtual void updateStudyInformation();
  virtual void updateStudyTreeWidget();
  virtual void updateMedicalImageWidget();
  virtual void updateRating();
  virtual void updateInterface();

  std::map< QTreeWidgetItem*, vtkSmartPointer<Alder::ActiveRecord> > treeModelMap;

protected slots:

private:
  // Designer form
  Ui_QMainAlderWindow *ui;
};

#endif
