/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   Alder.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
//
// .SECTION Description
// The main function which launches the application.
//

#include <Application.h>
#include <Configuration.h>
#include <User.h>
#include <Utilities.h>

#include <QMainAlderWindow.h>
#include <QAlderApplication.h>
#include <QInputDialog>
#include <QObject>
#include <QString>

#include <vtkNew.h>

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

    if( !app->OpenLogFile() )
    {
      std::string logPath = app->GetConfig()->GetValue( "Path", "Log" );
      cerr << "ERROR: unable to open log file \"" << logPath << "\"" << endl;
      Application::DeleteInstance();
      return status;
    }

    if( !app->TestImageDataPath() )
    {
      std::string imageDataPath = app->GetConfig()->GetValue( "Path", "ImageData" );
      cerr << "ERROR: no write access to image data directory \"" << imageDataPath << "\"" << endl;
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
    vtkNew< User > user; 
    if( !user->Load( "Name", "administrator" ) )
    {
      QString text = QInputDialog::getText(
        &mainWindow,
        QObject::tr( "Administrator Password" ),
        QObject::tr( "Please provide a password for the mandatory administrator account:" ),
        QLineEdit::Password );

      if( !text.isEmpty() )
      { // create an administrator with the new password
        user->Set( "Name", "administrator" );
        user->Set( "Password", text.toStdString().c_str() );
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
