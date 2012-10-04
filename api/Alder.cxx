/*=========================================================================

  Program:  Alder (CLSA Ultrasound Image Viewer)
  Module:   Alder.cxx
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>

=========================================================================*/
//
// .SECTION Description
// The main function which launches the application.
//

#include "Application.h"
#include "Utilities.h"
#include "QMainAlderWindow.h"
#include "vtkSmartPointer.h"
#include <QApplication>

using namespace Alder;

// main function
int main( int argc, char** argv )
{
  // create the qt application object and set some defaults
  QApplication app( argc, argv );
  
  // set main widget for the application to the main window
  QMainAlderWindow mainWindow;
  app.setMainWidget( &mainWindow );  
  mainWindow.show();

  // execute the application, then delete the application
  int status = app.exec();
  //Application::DeleteInstance();

  // return the result of the executed application
  return status;
}
