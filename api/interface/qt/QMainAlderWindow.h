/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QMainAlderWindow.h
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/

#ifndef __QMainAlderWindow_h
#define __QMainAlderWindow_h

#include <QMainWindow>

#include "vtkSmartPointer.h"

#include <map>

namespace Alder { class ActiveRecord; };
class Ui_QMainAlderWindow;
class vtkMedicalImageViewer;
class QTreeWidgetItem;

class QMainAlderWindow : public QMainWindow
{
  Q_OBJECT

public:
  QMainAlderWindow( QWidget* parent = 0 );
  ~QMainAlderWindow();

  
public slots:
  // action event functions
  virtual void slotOpenInterview();
  virtual void slotShowAtlas();
  virtual void slotLogin();
  virtual void slotUserManagement();
  virtual void slotUpdateDatabase();

  virtual void slotPreviousInterview();
  virtual void slotNextInterview();
  virtual void slotTreeSelectionChanged();
  virtual void slotInterviewRatingChanged( int );
  virtual void slotInterviewNoteChanged();

  virtual void slotAtlasRatingChanged( int );
  virtual void slotPreviousAtlasImage();
  virtual void slotNextAtlasImage();

  // help event functions
  virtual void slotAbout();
  virtual void slotManual();

protected:
  // called whenever the main window is closed
  virtual void closeEvent( QCloseEvent *event );

  // read/write application GUI settings
  virtual void readSettings();
  virtual void writeSettings();
  
  virtual void updateInterviewInformation();
  virtual void updateInterviewTreeWidget();
  virtual void updateInterviewImageWidget();
  virtual void updateInterviewRating();
  virtual void updateAtlasInformation();
  virtual void updateAtlasImageWidget();

  virtual void updateInterface();

  std::map< QTreeWidgetItem*, vtkSmartPointer<Alder::ActiveRecord> > treeModelMap;

protected slots:

private:
  // Designer form
  Ui_QMainAlderWindow *ui;

  vtkSmartPointer<vtkMedicalImageViewer> InterviewViewer;
  vtkSmartPointer<vtkMedicalImageViewer> AtlasViewer;
  bool atlasVisible;
};

#endif
