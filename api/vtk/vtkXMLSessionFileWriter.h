/*=========================================================================

  Program:  Alder (CLSA Ultrasound Image Viewer)
  Module:   vtkXMLSessionFileWriter.h
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/
//
// .NAME vtkXMLSessionFileWriter - Writes CLSAVis session files
//
// .SECTION Description
// This is a source object that reads CLSAVis session files.  The output of
// this reader is a single Session data object.
//
// .SECTION See Also
// Session vtkXMLFileWriter
// 

#ifndef __vtkXMLSessionFileWriter_h
#define __vtkXMLSessionFileWriter_h

#include "vtkXMLFileWriter.h"

#include "Utilities.h"

class Session;
class vtkCamera;

class vtkXMLSessionFileWriter : public vtkXMLFileWriter
{
public:
  static vtkXMLSessionFileWriter *New();
  vtkTypeMacro( vtkXMLSessionFileWriter, vtkXMLFileWriter );
  
  virtual void WriteContents();

protected:
  vtkXMLSessionFileWriter() {}
  ~vtkXMLSessionFileWriter() {}

  // Description:
  // Write a camera element
  virtual void Write( const char*, vtkCamera* );

private:
  vtkXMLSessionFileWriter( const vtkXMLSessionFileWriter& );  // Not implemented.
  void operator=( const vtkXMLSessionFileWriter& );  // Not implemented.
};

#endif
