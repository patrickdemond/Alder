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

class Ui_QMainAlderWindow;

//class QMainAlderWindowProgressCommand;

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

  // help event functions
  virtual void slotAbout();
  virtual void slotManual();

protected:
  // called whenever the main window is closed
  virtual void closeEvent( QCloseEvent *event );

  // read/write application GUI settings
  virtual void readSettings();
  virtual void writeSettings();
  virtual void updateInterface();

  //virtual void Render( bool resetCamera );

  //vtkSmartPointer< QMainAlderWindowProgressCommand > ProgressObserver;
  //QString CurrentSessionFileName;
protected slots:

private:
  // Designer form
  Ui_QMainAlderWindow *ui;
};

#endif
