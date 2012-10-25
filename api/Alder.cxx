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

#include "Application.h"
#include "User.h"
#include "Utilities.h"

#include "QMainAlderWindow.h"
#include "QAlderApplication.h"
#include <QInputDialog>
#include <QObject>
#include <QString>

#include "vtkSmartPointer.h"

#include <stdexcept>

using namespace Alder;

// main function
int main( int argc, char** argv )
{
  int status = EXIT_FAILURE;

  try
  {
    // start by reading the configuration, connecting to the database and setting up the Opal service
    Application *app = Application::GetInstance();
    if( !app->ReadConfiguration( ALDER_CONFIG_FILE ) )
    {
      cerr << "ERROR: error while reading configuration file \"" << ALDER_CONFIG_FILE << "\"" << endl;
      Application::DeleteInstance();
      return status;
    }
    if( !app->ConnectToDatabase() )
    {
      cerr << "ERROR: error while connecting to the database" << endl;
      Application::DeleteInstance();
      return status;
    }
    app->SetupOpalService();

    // now create the user interface
    QAlderApplication qapp( argc, argv );
    QMainAlderWindow mainWindow;

    // check to see if an admin user exists, create if not
    vtkSmartPointer< User > user = vtkSmartPointer< User >::New();
    if( !user->Load( "name", "administrator" ) )
    {
      QString text = QInputDialog::getText(
        &mainWindow,
        QObject::tr( "Administrator Password" ),
        QObject::tr( "Please provide a password for the mandatory administrator account:" ),
        QLineEdit::Password );

      if( !text.isEmpty() )
      { // create an administrator with the new password
        user->Set( "name", "administrator" );
        user->Set( "password", text.toStdString().c_str() );
        user->Save();
      }
    }

    mainWindow.show();

    // execute the application, then delete the application
    int status = qapp.exec();
    Application::DeleteInstance();
  }
  catch( std::exception &e )
  {
    cerr << "Uncaught exception: " << e.what() << endl;
    return EXIT_FAILURE;
  }

  // return the result of the executed application
  return status;
}
