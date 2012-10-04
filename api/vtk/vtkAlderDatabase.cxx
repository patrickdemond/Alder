/*=========================================================================

  Program:  Alder (CLSA Ultrasound Image Viewer)
  Module:   vtkAlderDatabase.cxx
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/

#include "vtkAlderDatabase.h"

#include "Utilities.h"

#include "vtkSQLQuery.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"
#include "vtkXMLDatabaseFileReader.h"

vtkStandardNewMacro( vtkAlderDatabase );

/**
 * Override the parent class Open method by automatically providing the database
 * credentials which are found in database.xml
 */
bool vtkAlderDatabase::Open( const char *password )
{
  vtkSmartPointer<vtkXMLDatabaseFileReader> reader = vtkSmartPointer<vtkXMLDatabaseFileReader>::New();
  reader->SetFileName( ALDER_DB_FILE );
  reader->Update();
  this->SetDatabaseName( reader->GetName() );
  this->SetUser( reader->GetUsername() );
  this->SetHostName( reader->GetServer() );
  this->SetServerPort( reader->GetPort() );
  return Superclass::Open( password ? password : reader->GetPassword() );
}

bool vtkAlderDatabase::HasAdministrator()
{
  vtkSmartPointer<vtkSQLQuery> query = vtkSmartPointer<vtkSQLQuery>::Take( this->GetQueryInstance() );
  query->SetQuery( "SELECT COUNT(*) AS total FROM user WHERE name = 'administrator'" );
  query->Execute();
  query->NextRow();
  vtkVariant v = query->DataValue( 0 );
  return 1 == v.ToInt();
}

void vtkAlderDatabase::SetAdministratorPassword( const char* password )
{
  vtkSmartPointer<vtkSQLQuery> query = vtkSmartPointer<vtkSQLQuery>::Take( this->GetQueryInstance() );
  std::string hash;
  Alder::hashString( password, hash );
  std::string sql = "REPLACE INTO user SET name = 'Administrator', password = '";
  sql += hash;
  sql += "'";
  query->SetQuery( sql.c_str() );
  query->Execute();
}

void vtkAlderDatabase::PrintSelf( ostream& os, vtkIndent indent )
{
  Superclass::PrintSelf( os, indent );
}
