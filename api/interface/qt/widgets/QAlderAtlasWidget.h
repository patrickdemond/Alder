/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QAlderAtlasWidget.h
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/

#ifndef __QAlderAtlasWidget_h
#define __QAlderAtlasWidget_h

#include <QWidget>

#include "vtkSmartPointer.h"

class vtkMedicalImageViewer;
class Ui_QAlderAtlasWidget;

class QAlderAtlasWidget : public QWidget
{
  Q_OBJECT

public:
  QAlderAtlasWidget( QWidget* parent = 0 );
  ~QAlderAtlasWidget();

  virtual void updateInterface();
  vtkMedicalImageViewer *GetViewer() { return this->Viewer; }
  virtual void setVisible( bool visible );

signals:
  void activeImageChanged();
  
public slots:
  // action event functions
  virtual void slotPrevious();
  virtual void slotNext();
  virtual void slotRatingChanged( int );

protected:
  // read/write application GUI settings
  virtual void updateInfo();

protected slots:

private:
  // Designer form
  Ui_QAlderAtlasWidget *ui;

  vtkSmartPointer<vtkMedicalImageViewer> Viewer;
};

#endif
