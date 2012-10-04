/*=========================================================================

  Program:  Alder (CLSA Ultrasound Image Viewer)
  Module:   vtkXMLDatabaseFileReader.cpp
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/
#include "vtkXMLDatabaseFileReader.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkVariantArray.h"

#include <algorithm>
#include <stdexcept>

vtkStandardNewMacro( vtkXMLDatabaseFileReader );

// this undef is required on the hp. vtkMutexLock ends up including
// /usr/inclue/dce/cma_ux.h which has the gall to #define read as cma_read
#ifdef read
#undef read
#endif

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
int vtkXMLDatabaseFileReader::ProcessRequest(
  vtkInformation *request,
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  if( request->Has( vtkDemandDrivenPipeline::REQUEST_DATA_OBJECT() ) )
  {
    return this->RequestDataObject( request, inputVector, outputVector );
  }
  else if( request->Has( vtkStreamingDemandDrivenPipeline::REQUEST_UPDATE_EXTENT() ) )
  {
    return 1;
  }
  else if( request->Has( vtkDemandDrivenPipeline::REQUEST_INFORMATION() ) )
  {
    return 1;
  }
  if( request->Has( vtkDemandDrivenPipeline::REQUEST_DATA() ) )
  {
    vtkDebugMacro( << "Reading database file ...");

    try
    {
      std::runtime_error e( "Error reading database file." );

      this->CreateReader();
      
      // parse until we find the session element
      while( this->ParseNode() )
        if( XML_READER_TYPE_ELEMENT == this->CurrentNode.NodeType &&
            0 == xmlStrcmp( BAD_CAST "Database", this->CurrentNode.Name ) ) break;

      // if we never found the session element then throw an exception
      if( XML_READER_TYPE_ELEMENT != this->CurrentNode.NodeType ||
          0 != xmlStrcmp( BAD_CAST "Database", this->CurrentNode.Name ) )
        throw std::runtime_error( "File does not contain a Database element." );

      std::string str;

      // list of expected elements
      bool nameFound = false;
      bool usernameFound = false;
      bool passwordFound = false;
      bool serverFound = false;
      bool portFound = false;

      int depth = this->CurrentNode.Depth;
      while( this->ParseNode() )
      {
        // loop until we find the closing element at the same depth
        if( XML_READER_TYPE_END_ELEMENT == this->CurrentNode.NodeType &&
            depth == this->CurrentNode.Depth ) break;

        // only process opening elements
        if( XML_READER_TYPE_ELEMENT != this->CurrentNode.NodeType ) continue;

        if( 0 == xmlStrcmp( BAD_CAST "Name", this->CurrentNode.Name ) )
        {
          this->ReadValue( str );
          this->SetName( str.c_str() );
          nameFound = true;
        }
        else if( 0 == xmlStrcmp( BAD_CAST "Username", this->CurrentNode.Name ) )
        {
          this->ReadValue( str );
          this->SetUsername( str.c_str() );
          usernameFound = true;
        }
        else if( 0 == xmlStrcmp( BAD_CAST "Password", this->CurrentNode.Name ) )
        {
          this->ReadValue( str );
          this->SetPassword( str.c_str() );
          passwordFound = true;
        }
        else if( 0 == xmlStrcmp( BAD_CAST "Server", this->CurrentNode.Name ) )
        {
          this->ReadValue( str );
          this->SetServer( str.c_str() );
          serverFound = true;
        }
        else if( 0 == xmlStrcmp( BAD_CAST "Port", this->CurrentNode.Name ) )
        {
          this->ReadValue( this->Port );
          portFound = true;
        }
        else
        {
          vtkWarningMacro(
            "Found unexpected element \"" << ( char* )( this->CurrentNode.Name ) <<
            "\" while reading Database." );
        }
      }

      // warn if any expected elements were not found
      if( !nameFound )
        vtkWarningMacro( "Expected element \"Name\" was not found while reading Database." );
      if( !usernameFound )
        vtkWarningMacro( "Expected element \"Username\" was not found while reading Database." );
      if( !passwordFound )
        vtkWarningMacro( "Expected element \"Password\" was not found while reading Database." );

      this->FreeReader();
    }
    catch( std::exception &e )
    {
      vtkErrorMacro( << e.what() );
      return 0;
    }
  }

  return this->Superclass::ProcessRequest( request, inputVector, outputVector );
}
