/*=========================================================================

  Program:  Alder (CLSA Ultrasound Image Viewer)
  Module:   vtkAlderDatabase.cxx
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/

#include "vtkAlderDatabase.h"

#include "Utilities.h"

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
  vtkSmartPointer< vtkXMLDatabaseFileReader > reader = vtkSmartPointer< vtkXMLDatabaseFileReader >::New();
  reader->SetFileName( ALDER_DB_FILE );
  reader->Update();
  this->SetDatabaseName( reader->GetName() );
  this->SetUser( reader->GetUsername() );
  this->SetHostName( reader->GetServer() );
  this->SetServerPort( reader->GetPort() );
  return Superclass::Open( password ? password : reader->GetPassword() );
}

void vtkAlderDatabase::PrintSelf( ostream& os, vtkIndent indent )
{
  Superclass::PrintSelf( os, indent );
}
