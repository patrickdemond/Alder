/*=========================================================================

  Program:  Alder (CLSA Ultrasound Image Viewer)
  Module:   vtkAlderDatabase.h
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/
//
// .NAME vtkAlderDatabase - The applications connection to the database
//
// .SECTION Description
//

#ifndef __vtkAlderDatabase_h
#define __vtkAlderDatabase_h

#include "vtkMySQLDatabase.h"

#include "Utilities.h"

class vtkAlderDatabase : public vtkMySQLDatabase
{
public:
  static vtkAlderDatabase *New();
  vtkTypeMacro( vtkAlderDatabase, vtkMySQLDatabase );
  void PrintSelf( ostream& os, vtkIndent indent );
  bool Open( const char *password = 0 );
  bool HasAdministrator();
  void SetAdministratorPassword( const char* password );

protected:
  vtkAlderDatabase() {}
  ~vtkAlderDatabase() {}
  
private:
  vtkAlderDatabase( const vtkAlderDatabase& );  // Not implemented.
  void operator=( const vtkAlderDatabase& );  // Not implemented.
};

#endif
