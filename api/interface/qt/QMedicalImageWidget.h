/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QMedicalImageWidget.h
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/

#ifndef __QMedicalImageWidget_h
#define __QMedicalImageWidget_h

#include <QWidget>
#include <vtkSmartPointer.h>

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

protected:
  void updateInterface();

  vtkSmartPointer<vtkMedicalImageViewer> viewer;

protected slots:

private:
  // Designer form
  Ui_QMedicalImageWidget *ui;
};

#endif
