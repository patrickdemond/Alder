/*=========================================================================

  Program:  Alder (CLSA Ultrasound Image Viewer)
  Module:   vtkXMLSessionFileReader.h
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/
//
// .NAME vtkXMLSessionFileReader - Reads CLSAVis session files
//
// .SECTION Description
// This is a source object that reads CLSAVis session files.  The output of
// this reader is a single Session data object.
//
// .SECTION See Also
// Session vtkXMLFileReader
// 

#ifndef __vtkXMLSessionFileReader_h
#define __vtkXMLSessionFileReader_h

#include "vtkXMLFileReader.h"

#include "Utilities.h"

class Session;
class vtkCamera;

class vtkXMLSessionFileReader : public vtkXMLFileReader
{
public:
  static vtkXMLSessionFileReader *New();
  vtkTypeMacro( vtkXMLSessionFileReader, vtkXMLFileReader );
  
  // Description:
  // Get the output of this reader.
  Session *GetOutput() { return this->GetOutput( 0 ); }
  Session *GetOutput( int idx );
  void SetOutput( Session *output );

protected:
  vtkXMLSessionFileReader() {}
  ~vtkXMLSessionFileReader() {}

  virtual int ProcessRequest(
    vtkInformation *, vtkInformationVector **, vtkInformationVector * );

  virtual int RequestDataObject(
    vtkInformation *, vtkInformationVector **, vtkInformationVector * );

  virtual int FillOutputPortInformation( int, vtkInformation* );

  virtual void Read( vtkCamera* );

private:
  vtkXMLSessionFileReader( const vtkXMLSessionFileReader& );  // Not implemented.
  void operator=( const vtkXMLSessionFileReader& );  // Not implemented.
};

#endif
