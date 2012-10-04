/*=========================================================================

  Program:  CLSAVis (Canadian Longitudinal Study on Aging Visualizer)
  Module:   vtkXMLFileWriter.h
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/
//
// .NAME vtkXMLFileWriter - Generic XML file reader
//
// .SECTION Description
// This abstract class is to be extended by any class which writes XML files.
// The parent of this class, vtkWriter, provides many methods for controlling
// the writing of the data file, see vtkWriter for more information.
//
// .SECTION See Also
// vtkWriter

#ifndef __vtkXMLFileWriter_h
#define __vtkXMLFileWriter_h

#include "vtkWriter.h"

#include "CVUtilities.h"
#include "vtkVariant.h"

#include <libxml/xmlwriter.h>
#include <stdexcept>

class vtkXMLFileWriter : public vtkWriter
{
public:
  vtkTypeMacro( vtkXMLFileWriter, vtkWriter );

  // Description:
  // Set/get the file name
  virtual CVString GetFileName() { return this->FileName; }
  virtual void SetFileName( const CVString &fileName );

protected:
  vtkXMLFileWriter();
  ~vtkXMLFileWriter();

  void WriteData();
  virtual void WriteContents() = 0;
  virtual int FillInputPortInformation( int port, vtkInformation *info );
  
  // Description:
  // Opens a XML file for writing (truncating it)
  void CreateWriter();

  // Description:
  // Closes the current file.
  void FreeWriter();
  
  void StartElement( const char* name );
  void EndElement();
  void WriteAttribute( const char* name, const char* value );
  template<class T> void WriteAttribute( const char* name, T value );
  void WriteValue( const char* name, const char* value );
  void WriteValue( const char* name, const CVString &value )
  { this->WriteValue( name, value.c_str() ); }
  template<class T> void WriteValue( const char* name, T value, bool type = true );
  template<class T> void WriteTuple( const char* name, T* array, vtkIdType length );

  CVString FileName;
  xmlTextWriter *Writer;

private:
  vtkXMLFileWriter( const vtkXMLFileWriter& );  // Not implemented.
  void operator=( const vtkXMLFileWriter& );  // Not implemented.
};

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
template<class T> void vtkXMLFileWriter::WriteAttribute( const char* name, T value )
{
  std::runtime_error e( "Error when writing attribute." );
  const char* string = vtkVariant( value ).ToString().c_str();
  if( 0 > xmlTextWriterWriteAttribute( this->Writer, BAD_CAST name, BAD_CAST string ) ) throw( e );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
template<class T> void vtkXMLFileWriter::WriteValue( const char* name, T value, bool type )
{
  std::runtime_error e( "Error when writing value." );
  vtkVariant v = vtkVariant( value );
  this->StartElement( name );
  if( type ) this->WriteAttribute( "type", v.GetTypeAsString() );
  if( 0 > xmlTextWriterWriteString( this->Writer, BAD_CAST v.ToString().c_str() ) ) throw( e );
  this->EndElement();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
template<class T> void vtkXMLFileWriter::WriteTuple( const char* name, T* array, vtkIdType length )
{
  std::runtime_error e( "Error when writing tuple." );
  this->StartElement( name );
  this->WriteAttribute( "type", vtkVariant( *array ).GetTypeAsString() );
  this->WriteAttribute( "length", length );

  // build the tuple as a space separated list
  CVString string = "";
  for( vtkIdType i = 0; i < length; i++ )
  {
    // convert the value to a string, whatever it is
    if( 0 < string.size() ) string += " ";
    string += vtkVariant( *( array + i ) ).ToString();
  }
  if( 0 > xmlTextWriterWriteString( this->Writer, BAD_CAST string.c_str() ) ) throw( e );

  this->EndElement();
}

#endif
