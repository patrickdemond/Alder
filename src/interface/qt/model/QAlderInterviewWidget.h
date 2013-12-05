/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QAlderInterviewWidget.h
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/

#ifndef __QAlderInterviewWidget_h
#define __QAlderInterviewWidget_h

#include <QWidget>

#include <vtkSmartPointer.h>

#include <map>

namespace Alder { 
class ActiveRecord; 
class Interview;
};

class vtkEventQtSlotConnect;
class vtkMedicalImageViewer;
class Ui_QAlderInterviewWidget;
class QTreeWidgetItem;

class QAlderInterviewWidget : public QWidget
{
  Q_OBJECT

public:
  QAlderInterviewWidget( QWidget* parent = 0 );
  ~QAlderInterviewWidget();

  vtkMedicalImageViewer *GetViewer();

public slots:
  virtual void slotPrevious();
  virtual void slotNext();
  virtual void slotTreeSelectionChanged();
  virtual void slotRatingChanged( int );
  virtual void slotNoteChanged();
  virtual void slotHideControls( bool );
  virtual void updateInfo();
  virtual void updateExamTreeWidget();
  virtual void updateRating();
  virtual void updateViewer();
  virtual void updateEnabled();

protected:

  std::map< QTreeWidgetItem*, vtkSmartPointer<Alder::ActiveRecord> > treeModelMap;

protected slots:

private:
  // Designer form
  Ui_QAlderInterviewWidget *ui;

  vtkSmartPointer<vtkMedicalImageViewer> Viewer;
  vtkSmartPointer<vtkEventQtSlotConnect> Connections;

  /**
   * Internal update method used in slotPrevious, slotNext
   */
  void updateActiveInterview( Alder::Interview* );

  void initializeTreeWidget();
  std::map< std::string, QTreeWidgetItem* > ModalityLookup;
};

#endif
