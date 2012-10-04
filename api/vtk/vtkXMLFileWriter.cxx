/*=========================================================================

  Program:  CLSAVis (CLSA Ultrasound Image Viewer)
  Module:   vtkXMLFileWriter.cpp
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>

=========================================================================*/
#include "vtkXMLFileWriter.h"

#include "vtkInformation.h"
#include "vtkObjectFactory.h"

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
vtkXMLFileWriter::vtkXMLFileWriter()
{
  this->FileName = "";
  this->Writer = NULL;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
vtkXMLFileWriter::~vtkXMLFileWriter()
{
  if( NULL != this->Writer )
  {
    this->FreeWriter();
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkXMLFileWriter::SetFileName( const CVString &fileName )
{
  vtkDebugMacro( << this->GetClassName() << " (" << this << "): setting "
                 << "FileName to " << fileName.c_str() );

  if( fileName != this->FileName )
  {
    this->FileName = fileName;
    this->Modified();
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
int vtkXMLFileWriter::FillInputPortInformation( int, vtkInformation *info )
{
  info->Set( vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "CVSession" );
  return 1;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkXMLFileWriter::WriteData()
{
  try
  {
    this->CreateWriter();
    this->WriteContents();
    this->FreeWriter();
  }
  catch( std::exception &e )
  {
    if( this->Writer ) this->FreeWriter();
    throw e;
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkXMLFileWriter::CreateWriter()
{
  this->Writer = xmlNewTextWriterFilename( this->FileName.c_str(), 0 );
  if( NULL == this->Writer )
  {
    throw std::runtime_error( "Unable to open file." );
  }

  // Start the document with the xml default for the version, encoding UTF-8
  // and the default for the standalone declaration.
  if( xmlTextWriterStartDocument( this->Writer, NULL, "UTF-8", NULL ) < 0 )
  {
    throw std::runtime_error( "Unable to start document." );
  }

  // Format the XML file to be indented
  std::runtime_error e( "Error writing to CLSAVis session file." );
  if( 0 > xmlTextWriterSetIndent( this->Writer, 1 ) ) throw( e );
  if( 0 > xmlTextWriterSetIndentString( this->Writer, BAD_CAST "  " ) ) throw( e );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkXMLFileWriter::FreeWriter()
{
  if( NULL == this->Writer )
  {
    throw std::runtime_error( "No file opened." );
  }
  xmlFreeTextWriter( this->Writer );
  this->Writer = NULL;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkXMLFileWriter::StartElement( const char* name )
{
  std::runtime_error e( "Error when starting element." );
  if( 0 > xmlTextWriterStartElement( this->Writer, BAD_CAST name ) ) throw( e );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkXMLFileWriter::EndElement()
{
  std::runtime_error e( "Error when ending element." );
  if( 0 > xmlTextWriterEndElement( this->Writer ) ) throw( e );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkXMLFileWriter::WriteAttribute( const char* name, const char* value )
{
  std::runtime_error e( "Error when writing attribute." );
  if( 0 > xmlTextWriterWriteAttribute( this->Writer, BAD_CAST name, BAD_CAST value ) ) throw( e );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkXMLFileWriter::WriteValue( const char* name, const char* value )
{
  std::runtime_error e( "Error when writing value." );
  this->StartElement( name );
  if( 0 > xmlTextWriterWriteAttribute( this->Writer, BAD_CAST "type", BAD_CAST "string" ) ) throw( e );
  if( 0 > xmlTextWriterWriteString( this->Writer, BAD_CAST value ) ) throw( e );
  this->EndElement();
}
