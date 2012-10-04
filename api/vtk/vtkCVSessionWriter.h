/*=========================================================================

  Program:  CLSAVis (Canadian Longitudinal Study on Aging Visualizer)
  Module:   vtkCVSessionWriter.h
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>

=========================================================================*/
//
// .NAME vtkCVSessionWriter - Writes CLSAVis session files
//
// .SECTION Description
// This is a source object that reads CLSAVis session files.  The output of
// this reader is a single CVSession data object.
//
// .SECTION See Also
// CVSession vtkXMLFileWriter
// 

#ifndef __vtkCVSessionWriter_h
#define __vtkCVSessionWriter_h

#include "vtkXMLFileWriter.h"

#include "CVUtilities.h"

class CVSession;
class vtkCamera;

class vtkCVSessionWriter : public vtkXMLFileWriter
{
public:
  static vtkCVSessionWriter *New();
  vtkTypeMacro( vtkCVSessionWriter, vtkXMLFileWriter );
  
  virtual void WriteContents();

protected:
  vtkCVSessionWriter() {}
  ~vtkCVSessionWriter() {}

  // Description:
  // Write a camera element
  virtual void Write( const char*, vtkCamera* );

private:
  vtkCVSessionWriter( const vtkCVSessionWriter& );  // Not implemented.
  void operator=( const vtkCVSessionWriter& );  // Not implemented.
};

#endif
