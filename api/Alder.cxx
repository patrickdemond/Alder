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

/*
bool adminUserExists()
{
  // url syntax:
  // mysql://'[[username[':'password]'@']hostname[':'port]]'/'[dbname]
 
  vtkSmartPointer<vtkMySQLDatabase> db =
    vtkSmartPointer<vtkMySQLDatabase>::Take(vtkMySQLDatabase::SafeDownCast(
            vtkSQLDatabase::CreateFromURL( "mysql://root@localhost/TestDatabase" ) ));
 
  bool status = db->Open();
 
  std::cout << "Database open? " << status << std::endl;
 
  if(!status)
    {
    return EXIT_FAILURE;
    }
 
  vtkSmartPointer<vtkSQLQuery> query =
    vtkSmartPointer<vtkSQLQuery>::Take(db->GetQueryInstance());
 
  std::string createQuery( "SELECT PointId FROM TestTable");
  std::cout << createQuery << std::endl;
  query->SetQuery( createQuery.c_str() );
  query->Execute();
 
  for ( int col = 0; col < query->GetNumberOfFields(); ++ col )
    {
    if ( col > 0 )
      {
      cerr << ", ";
      }
    cerr << query->GetFieldName( col );
    }
  cerr << endl;
  while ( query->NextRow() )
    {
    for ( int field = 0; field < query->GetNumberOfFields(); ++ field )
      {
      if ( field > 0 )
        {
        cerr << ", ";
        }
      cerr << query->DataValue( field ).ToString().c_str();
      }
    cerr << endl;
    }
}
*/
