/*=========================================================================

  Program:  Alder (CLSA Ultrasound Image Viewer)
  Module:   QMedicalImageWidget.h
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/

#ifndef __QMedicalImageWidget_h
#define __QMedicalImageWidget_h

#include <QWidget>

class vtkMedicalImageViewer;
class Ui_QMedicalImageWidget;

class QMedicalImageWidget : public QWidget
{
  Q_OBJECT

public:
  //constructor
  QMedicalImageWidget( QWidget* parent = 0 );
  //destructor
  ~QMedicalImageWidget();

  void resetImage();
  void loadImage( QString filename );

public slots:
  virtual void slotSliceChanged( int );

protected:
  void updateInterface();

  vtkMedicalImageViewer *viewer;

protected slots:

private:
  // Designer form
  Ui_QMedicalImageWidget *ui;
};

#endif
