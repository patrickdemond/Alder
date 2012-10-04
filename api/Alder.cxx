/*=========================================================================

  Program:  Alder (CLSA Ultrasound Image Viewer)
  Module:   Alder.cxx
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/
//
// .SECTION Description
// The main function which launches the application.
//

#include "Utilities.h"
#include "Application.h"

#include "vtkAlderDatabase.h"
#include "vtkSmartPointer.h"

#include "QMainAlderWindow.h"
#include <QApplication>
#include <QInputDialog>
#include <QObject>
#include <QString>

#include <stdexcept>

using namespace Alder;

// main function
int main( int argc, char** argv )
{
  // Make sure that the database can be connected to
  vtkAlderDatabase *db = Application::GetInstance()->GetDatabase();
  if( !db->Open() )
  {
    cout << "Cannot connect to database." << endl;
    return EXIT_FAILURE;
  }

  // create the qt application object and set some defaults
  QApplication qapp( argc, argv );
  
  // set main widget for the application to the main window
  QMainAlderWindow mainWindow;
  qapp.setMainWidget( &mainWindow );  

  // check to see if an admin user exists, create if not
  if( !db->HasAdministrator() )
  {
    QString text = QInputDialog::getText(
      &mainWindow,
      QObject::tr( "Administrator Password" ),
      QObject::tr( "Please provide a password for the mandatory administrator account:" ),
      QLineEdit::PasswordEchoOnEdit );
    db->SetAdministratorPassword( text.toStdString().c_str() );
  }

  mainWindow.show();

  // execute the application, then delete the application
  int status = qapp.exec();
  Application::DeleteInstance();

  // return the result of the executed application
  return status;
}
