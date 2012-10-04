/*=========================================================================

  Program:  CLSAVis (Canadian Longitudinal Study on Aging Visualizer)
  Module:   vtkXMLFileReader.h
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/
//
// .NAME vtkXMLFileReader - Generic XML file reader
//
// .SECTION Description
// This abstract class is to be extended by any class which reads XML files.
// The parent of this class, vtkAlgorithm, provides many methods for
// controlling the reading of the data file, see vtkAlgorithm for more
// information.
//
// .SECTION See Also
// vtkAlgorithm
// 

#ifndef __vtkXMLFileReader_h
#define __vtkXMLFileReader_h

#include "vtkAlgorithm.h"
#include "vtkVariant.h"

#include "CVUtilities.h"
#include <libxml/xmlreader.h>
#include <stdexcept>

class vtkGraph;

class vtkXMLFileReader : public vtkAlgorithm
{
public:
  vtkTypeMacro( vtkXMLFileReader, vtkAlgorithm );
  void PrintSelf( ostream& os, vtkIndent indent );

  // Description:
  // Set/get the file name
  virtual CVString GetFileName() { return this->FileName; }
  virtual void SetFileName( const CVString &fileName );
  
protected:
  // Description:
  // Internal struct for managing nodes
  struct vtkXMLFileReaderNode
  {
    vtkXMLFileReaderNode() { this->Clear(); }
    void Clear()
    {
      this->Name = NULL;
      this->Depth = 0;
      this->AttributeCount = 0;
      this->NodeType = 0;
      this->IsEmptyElement = 0;
      this->HasContent = 0;
      this->Content = NULL;
    }
    const char* GetNodeTypeName()
    {
      if( XML_READER_TYPE_NONE == this->NodeType ) return "None";
      else if( XML_READER_TYPE_ELEMENT == this->NodeType ) return "Element";
      else if( XML_READER_TYPE_ATTRIBUTE == this->NodeType ) return "Attribute";
      else if( XML_READER_TYPE_TEXT == this->NodeType ) return "Text";
      else if( XML_READER_TYPE_CDATA == this->NodeType ) return "CDATA";
      else if( XML_READER_TYPE_ENTITY_REFERENCE == this->NodeType ) return "EntityReference";
      else if( XML_READER_TYPE_ENTITY == this->NodeType ) return "Entity";
      else if( XML_READER_TYPE_PROCESSING_INSTRUCTION == this->NodeType ) return "ProcessingInstruction";
      else if( XML_READER_TYPE_COMMENT == this->NodeType ) return "Comment";
      else if( XML_READER_TYPE_DOCUMENT == this->NodeType ) return "Document";
      else if( XML_READER_TYPE_DOCUMENT_TYPE == this->NodeType ) return "DocumentType";
      else if( XML_READER_TYPE_DOCUMENT_FRAGMENT == this->NodeType ) return "DocumentFragment";
      else if( XML_READER_TYPE_NOTATION == this->NodeType ) return "Notation";
      else if( XML_READER_TYPE_WHITESPACE == this->NodeType ) return "Whitespace";
      else if( XML_READER_TYPE_SIGNIFICANT_WHITESPACE == this->NodeType ) return "SignificantWhitespace";
      else if( XML_READER_TYPE_END_ELEMENT == this->NodeType ) return "EndElement";
      else if( XML_READER_TYPE_END_ENTITY == this->NodeType ) return "EndEntity";
      else if( XML_READER_TYPE_XML_DECLARATION == this->NodeType ) return "XmlDeclaration";
      return "Unknown";
    }
    void PrintSelf( ostream &os, vtkIndent indent )
    {
      os << indent << "Name: " << ( NULL == this->Name ? "(null)" : ( char* )( this->Name ) ) << endl;
      os << indent << "NodeType: " << this->GetNodeTypeName() << " (" << this->NodeType << ")" << endl;
      os << indent << "Depth: " << this->Depth << endl;
      os << indent << "AttributeCount: " << this->AttributeCount << endl;
      os << indent << "IsEmptyElement: " << this->IsEmptyElement << endl;
      os << indent << "HasContent: " << this->HasContent << endl;
      os << indent << "Content: \"" << ( NULL == this->Content ? "(null)" : ( char* )( this->Content ) )
         << "\"" << endl;
    }

    const xmlChar* Name;
    int Depth;
    int AttributeCount;
    int NodeType;
    int IsEmptyElement;
    int HasContent;
    const xmlChar* Content;
  };

  vtkXMLFileReader();
  ~vtkXMLFileReader();

  virtual int ProcessRequest(
    vtkInformation *, vtkInformationVector **, vtkInformationVector * );

  // Description:
  // Opens and parses the current XML file.
  // An exception is thrown if the file cannot be opened.
  void CreateReader();

  // Description:
  // Closes the current file.
  void FreeReader();

  // Description:
  // Parses the next node in the XML file.  Make sure to use Open() before calling this method.
  // Returns 1 if a new node has been parsed or 0 if the end of the file has been reached.
  // An exception is thrown if there is a parsing error.
  int ParseNode();

  // Description:
  // Points the current node to the beginning of the file so that the first node in the file
  // will be parsed next time ParseNode() is called.
  void RewindReader();

  void ReadValue( CVString& value );
  template<class T> void ReadValue( T& value );
  template<class T> void ReadTuple( T array[], vtkIdType length );

  CVString FileName;
  xmlTextReader *Reader;
  vtkXMLFileReaderNode CurrentNode;

private:
  vtkXMLFileReader( const vtkXMLFileReader& );  // Not implemented.
  void operator=( const vtkXMLFileReader& );  // Not implemented.
};

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
template<class T> void vtkXMLFileReader::ReadValue( T& value )
{
  // list of expected elements
  xmlChar *read = xmlTextReaderReadString( this->Reader );
  if( NULL == read )
  {
    CVString error = "Failed to read ";
    error += vtkVariant( value ).GetTypeAsString();
    throw( std::runtime_error( error ) );
  }
  vtkVariant v = vtkVariant( ( char* ) read );
  delete [] read;
  bool valid = true;
  value = v.ToNumeric( &valid, &value );

  // TODO: warn if attribute "type" doesn't match T's type
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
template<class T> void vtkXMLFileReader::ReadTuple( T array[], vtkIdType length )
{
  CVString string;
  this->ReadValue( string );

  // split string by spaces
  vtkIdType count = 0;
  CVStringStream stream( string );
  CVString word;
  while( stream >> word )
  {
    // check for overflow
    if( count >= length )
    {
      vtkWarningMacro( "More than " << length << " elements found in tuple." );
      break;
    }

    bool valid = true;
    *( array + count ) = vtkVariant( word ).ToNumeric( &valid, array );
    count++;
  }

  // warn if not enough values were found
  if( count < length )
    vtkWarningMacro( "Expecting " << length << " elements in tuple, " << count << " found." );

  // TODO: warn if attribute "type" doesn't match T's type
}

#endif
