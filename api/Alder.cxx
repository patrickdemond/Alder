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
#include "Utilities.h"

#include "QMainAlderWindow.h"
#include "QAlderApplication.h"
#include <QInputDialog>
#include <QObject>
#include <QString>

#include <stdexcept>

using namespace Alder;

// main function
int main( int argc, char** argv )
{
  int status = EXIT_FAILURE;

  try
  {
    // Make sure that the database can be connected to
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

    // create the qt application object and set some defaults
    QAlderApplication qapp( argc, argv );
    
    // set main widget for the application to the main window
    QMainAlderWindow mainWindow;

    // check to see if an admin user exists, create if not
    if( !app->HasAdministrator() )
    {
      QString text = QInputDialog::getText(
        &mainWindow,
        QObject::tr( "Administrator Password" ),
        QObject::tr( "Please provide a password for the mandatory administrator account:" ),
        QLineEdit::Password );
      if( !text.isEmpty() ) app->SetAdministratorPassword( text.toStdString().c_str() );
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
