/*=========================================================================

  Program:  Alder (CLSA Ultrasound Image Viewer)
  Module:   vtkXMLDatabaseFileReader.h
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/
//
// .NAME vtkXMLDatabaseFileReader - Reads database.xml files
//
// .SECTION Description
// This is a source object that reads database session files in XML format.
// There is no output, instead use the GetX() methods to get the read information.
//
// .SECTION See Also
// Database vtkXMLFileReader
// 

#ifndef __vtkXMLDatabaseFileReader_h
#define __vtkXMLDatabaseFileReader_h

#include "vtkXMLFileReader.h"

#include "Utilities.h"

class vtkXMLDatabaseFileReader : public vtkXMLFileReader
{
public:
  static vtkXMLDatabaseFileReader *New();
  vtkTypeMacro( vtkXMLDatabaseFileReader, vtkXMLFileReader );
  
  vtkGetStringMacro( Name );
  vtkGetStringMacro( Username );
  vtkGetStringMacro( Password );
  vtkGetStringMacro( Server );
  vtkGetMacro( Port, unsigned int );

  /*
  // Description:
  // Get the output of this reader.
  Database *GetOutput() { return this->GetOutput( 0 ); }
  Database *GetOutput( int idx );
  void SetOutput( Database *output );
  */

protected:
  vtkXMLDatabaseFileReader()
  {
    this->Name = NULL;
    this->SetName( "alder" );
    this->Username = NULL;
    this->SetUsername( "alder" );
    this->Password = NULL;
    this->Server = NULL;
    this->SetServer( "localhost" );
    this->Port = 3306;
    this->SetNumberOfOutputPorts( 0 );
  }
  ~vtkXMLDatabaseFileReader()
  {
    this->SetName( NULL );
    this->SetUsername( NULL );
    this->SetPassword( NULL );
    this->SetServer( NULL );
  }

  vtkSetStringMacro( Name );
  vtkSetStringMacro( Username );
  vtkSetStringMacro( Password );
  vtkSetStringMacro( Server );

  virtual int ProcessRequest( vtkInformation *, vtkInformationVector **, vtkInformationVector * );
  virtual int FillOutputPortInformation( int, vtkInformation* ) { return 1; }
  virtual int RequestDataObject( vtkInformation *, vtkInformationVector **, vtkInformationVector * )
  { return 1; }

  char* Name;
  char* Username;
  char* Password;
  char* Server;
  unsigned int Port;

private:
  vtkXMLDatabaseFileReader( const vtkXMLDatabaseFileReader& );  // Not implemented.
  void operator=( const vtkXMLDatabaseFileReader& );  // Not implemented.
};

#endif
