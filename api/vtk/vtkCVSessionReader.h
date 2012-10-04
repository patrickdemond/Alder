/*=========================================================================

  Program:  CLSAVis (Canadian Longitudinal Study on Aging Visualizer)
  Module:   vtkCVSessionReader.h
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>

=========================================================================*/
//
// .NAME vtkCVSessionReader - Reads CLSAVis session files
//
// .SECTION Description
// This is a source object that reads CLSAVis session files.  The output of
// this reader is a single CVSession data object.
//
// .SECTION See Also
// CVSession vtkXMLFileReader
// 

#ifndef __vtkCVSessionReader_h
#define __vtkCVSessionReader_h

#include "vtkXMLFileReader.h"

#include "CVUtilities.h"

class CVSession;
class vtkCamera;

class vtkCVSessionReader : public vtkXMLFileReader
{
public:
  static vtkCVSessionReader *New();
  vtkTypeMacro( vtkCVSessionReader, vtkXMLFileReader );
  
  // Description:
  // Get the output of this reader.
  CVSession *GetOutput() { return this->GetOutput( 0 ); }
  CVSession *GetOutput( int idx );
  void SetOutput( CVSession *output );

protected:
  vtkCVSessionReader() {}
  ~vtkCVSessionReader() {}

  virtual int ProcessRequest(
    vtkInformation *, vtkInformationVector **, vtkInformationVector * );

  virtual int RequestDataObject(
    vtkInformation *, vtkInformationVector **, vtkInformationVector * );

  virtual int FillOutputPortInformation( int, vtkInformation* );

  virtual void Read( vtkCamera* );

private:
  vtkCVSessionReader( const vtkCVSessionReader& );  // Not implemented.
  void operator=( const vtkCVSessionReader& );  // Not implemented.
};

#endif
