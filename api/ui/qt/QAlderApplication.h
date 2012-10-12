/*=========================================================================

  Program:  Alder (CLSA Ultrasound Image Viewer)
  Module:   QAlderApplication.h
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/

#ifndef __QAlderApplication_h
#define __QAlderApplication_h

#include <QApplication>

#include "Utilities.h"

class Ui_QAlderApplication;

class QAlderApplication : public QApplication
{
public:
  QAlderApplication( int &argc, char **argv ) : QApplication( argc, argv ) {}
  bool notify( QObject *pObject, QEvent *pEvent );
};

#endif
