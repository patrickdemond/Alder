/*=========================================================================

  Program:  Alder (CLSA Ultrasound Image Viewer)
  Module:   vtkXMLSessionFileReader.cpp
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/
#include "vtkXMLSessionFileReader.h"

#include "Session.h"
#include "vtkCamera.h"
#include "vtkCommand.h"
#include "vtkDataSetAttributes.h"
#include "vtkDoubleArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkIntArray.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkVariantArray.h"

#include <algorithm>
#include <stdexcept>

vtkStandardNewMacro( vtkXMLSessionFileReader );

// this undef is required on the hp. vtkMutexLock ends up including
// /usr/inclue/dce/cma_ux.h which has the gall to #define read as cma_read
#ifdef read
#undef read
#endif

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
Session* vtkXMLSessionFileReader::GetOutput(int idx)
{
  return Session::SafeDownCast( this->GetOutputDataObject( idx ) );
} 

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkXMLSessionFileReader::SetOutput( Session *output )
{
  this->GetExecutive()->SetOutputData( 0, output );
} 

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
int vtkXMLSessionFileReader::RequestDataObject(
  vtkInformation* request,
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  Session *output = Session::New();
  this->SetOutput( output );
  output->Delete();
  
  return 1;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
int vtkXMLSessionFileReader::ProcessRequest(
  vtkInformation *request,
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  vtkInformation *outInfo = outputVector->GetInformationObject( 0 );

  if( request->Has( vtkDemandDrivenPipeline::REQUEST_DATA_OBJECT() ) )
  {
    return this->RequestDataObject( request, inputVector, outputVector );
  }

  if( request->Has( vtkStreamingDemandDrivenPipeline::REQUEST_UPDATE_EXTENT() ) )
  {
    return 1;
  }

  if( request->Has( vtkDemandDrivenPipeline::REQUEST_INFORMATION() ) )
  {
    return 1;
  }

  if( request->Has( vtkDemandDrivenPipeline::REQUEST_DATA() ) &&
      0 == outInfo->Get( vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER() ) )
  {
    vtkDebugMacro( << "Reading CLSAVis session ...");

    Session* output = Session::SafeDownCast( outInfo->Get( vtkDataObject::DATA_OBJECT() ) );

    try
    {
      std::runtime_error e( "Error reading CLSAVis session file." );

      this->CreateReader();
      
      // parse until we find the session element
      while( this->ParseNode() )
        if( XML_READER_TYPE_ELEMENT == this->CurrentNode.NodeType &&
            0 == xmlStrcasecmp( BAD_CAST "Session", this->CurrentNode.Name ) ) break;

      // if we never found the session element then throw an exception
      if( XML_READER_TYPE_ELEMENT != this->CurrentNode.NodeType ||
          0 != xmlStrcasecmp( BAD_CAST "Session", this->CurrentNode.Name ) )
        throw std::runtime_error( "File does not contain a Session element." );

      // list of expected elements
      bool cameraFound = false;

      int depth = this->CurrentNode.Depth;
      while( this->ParseNode() )
      {
        // loop until we find the closing element at the same depth
        if( XML_READER_TYPE_END_ELEMENT == this->CurrentNode.NodeType &&
            depth == this->CurrentNode.Depth ) break;

        // only process opening elements
        if( XML_READER_TYPE_ELEMENT != this->CurrentNode.NodeType ) continue;

        if( 0 == xmlStrcasecmp( BAD_CAST "Camera", this->CurrentNode.Name ) )
        {
          vtkSmartPointer< vtkCamera > camera = vtkSmartPointer< vtkCamera >::New();
          this->Read( camera );
          output->GetCamera()->DeepCopy( camera );
          cameraFound = true;
        }
        else
        {
          vtkWarningMacro(
            "Found unexpected element \"" << ( char* )( this->CurrentNode.Name ) <<
            "\" while reading Session." );
        }
      }

      // warn if any expected elements were not found
      if( !cameraFound )
        vtkWarningMacro( "Expected element \"Camera\" was not found while reading Session." );

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

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
int vtkXMLSessionFileReader::FillOutputPortInformation( int, vtkInformation* info )
{
  info->Set( vtkDataObject::DATA_TYPE_NAME(), "Session" );
  return 1;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkXMLSessionFileReader::Read( vtkCamera* camera )
{
  // list of expected elements
  bool isClippingRangeFound = false;
  bool isDistanceFound = false;
  bool isFocalDiskFound = false;
  bool isFocalPointFound = false;
  bool isViewShearFound = false;
  bool isParallelProjectionFound = false;
  bool isParallelScaleFound = false;
  bool isPositionFound = false;
  bool isLeftEyeFound = false;
  bool isThicknessFound = false;
  bool isViewAngleFound = false;
  bool isUseHorizontalViewAngleFound = false;
  bool isViewUpFound = false;
  bool isWindowCenterFound = false;
  bool isScreenBottomLeftFound = false;
  bool isScreenBottomRightFound = false;
  bool isScreenTopRightFound = false;

  int depth = this->CurrentNode.Depth;
  while( this->ParseNode() )
  {
    // loop until we find the closing element at the same depth
    if( XML_READER_TYPE_END_ELEMENT == this->CurrentNode.NodeType &&
        depth == this->CurrentNode.Depth ) break;

    // only process opening elements
    if( XML_READER_TYPE_ELEMENT != this->CurrentNode.NodeType ) continue;

    if( 0 == xmlStrcasecmp( BAD_CAST "ClippingRange", this->CurrentNode.Name ) )
    {
      double tuple[2];
      camera->GetClippingRange( tuple );
      this->ReadTuple( tuple, 2 );
      camera->SetClippingRange( tuple );
      isClippingRangeFound = true;
    }
    else if( 0 == xmlStrcasecmp( BAD_CAST "Distance", this->CurrentNode.Name ) )
    {
      double value = camera->GetDistance();
      this->ReadValue( value );
      camera->SetDistance( value );
      isDistanceFound = true;
    }
    else if( 0 == xmlStrcasecmp( BAD_CAST "FocalDisk", this->CurrentNode.Name ) )
    {
      double value = camera->GetFocalDisk();
      this->ReadValue( value );
      camera->SetFocalDisk( value );
      isFocalDiskFound = true;
    }
    else if( 0 == xmlStrcasecmp( BAD_CAST "FocalPoint", this->CurrentNode.Name ) )
    {
      double tuple[3];
      camera->GetFocalPoint( tuple );
      this->ReadTuple( tuple, 3 );
      camera->SetFocalPoint( tuple );
      isFocalPointFound = true;
    }
    else if( 0 == xmlStrcasecmp( BAD_CAST "ViewShear", this->CurrentNode.Name ) )
    {
      double tuple[3];
      camera->GetViewShear( tuple );
      this->ReadTuple( tuple, 3 );
      camera->SetViewShear( tuple );
      isViewShearFound = true;
    }
    else if( 0 == xmlStrcasecmp( BAD_CAST "ParallelProjection", this->CurrentNode.Name ) )
    {
      int value = camera->GetParallelProjection();
      this->ReadValue( value );
      camera->SetParallelProjection( value );
      isParallelProjectionFound = true;
    }
    else if( 0 == xmlStrcasecmp( BAD_CAST "ParallelScale", this->CurrentNode.Name ) )
    {
      double value = camera->GetParallelScale();
      this->ReadValue( value );
      camera->SetParallelScale( value );
      isParallelScaleFound = true;
    }
    else if( 0 == xmlStrcasecmp( BAD_CAST "Position", this->CurrentNode.Name ) )
    {
      double tuple[3];
      camera->GetPosition( tuple );
      this->ReadTuple( tuple, 3 );
      camera->SetPosition( tuple );
      isPositionFound = true;
    }
    else if( 0 == xmlStrcasecmp( BAD_CAST "LeftEye", this->CurrentNode.Name ) )
    {
      int value = camera->GetLeftEye();
      this->ReadValue( value );
      camera->SetLeftEye( value );
      isLeftEyeFound = true;
    }
    else if( 0 == xmlStrcasecmp( BAD_CAST "Thickness", this->CurrentNode.Name ) )
    {
      double value = camera->GetThickness();
      this->ReadValue( value );
      camera->SetThickness( value );
      isThicknessFound = true;
    }
    else if( 0 == xmlStrcasecmp( BAD_CAST "ViewAngle", this->CurrentNode.Name ) )
    {
      double value = camera->GetViewAngle();
      this->ReadValue( value );
      camera->SetViewAngle( value );
      isViewAngleFound = true;
    }
    else if( 0 == xmlStrcasecmp( BAD_CAST "UseHorizontalViewAngle", this->CurrentNode.Name ) )
    {
      int value = camera->GetUseHorizontalViewAngle();
      this->ReadValue( value );
      camera->SetUseHorizontalViewAngle( value );
      isUseHorizontalViewAngleFound = true;
    }
    else if( 0 == xmlStrcasecmp( BAD_CAST "ViewUp", this->CurrentNode.Name ) )
    {
      double tuple[3];
      camera->GetViewUp( tuple );
      this->ReadTuple( tuple, 3 );
      camera->SetViewUp( tuple );
      isViewUpFound = true;
    }
    else if( 0 == xmlStrcasecmp( BAD_CAST "WindowCenter", this->CurrentNode.Name ) )
    {
      double tuple[2];
      camera->GetWindowCenter( tuple );
      this->ReadTuple( tuple, 2 );
      camera->SetWindowCenter( tuple[0], tuple[1] );
      isWindowCenterFound = true;
    }
    else if( 0 == xmlStrcasecmp( BAD_CAST "ScreenBottomLeft", this->CurrentNode.Name ) )
    {
      double tuple[3];
      camera->GetScreenBottomLeft( tuple );
      this->ReadTuple( tuple, 3 );
      camera->SetScreenBottomLeft( tuple );
      isScreenBottomLeftFound = true;
    }
    else if( 0 == xmlStrcasecmp( BAD_CAST "ScreenBottomRight", this->CurrentNode.Name ) )
    {
      double tuple[3];
      camera->GetScreenBottomRight( tuple );
      this->ReadTuple( tuple, 3 );
      camera->SetScreenBottomRight( tuple );
      isScreenBottomRightFound = true;
    }
    else if( 0 == xmlStrcasecmp( BAD_CAST "ScreenTopRight", this->CurrentNode.Name ) )
    {
      double tuple[3];
      camera->GetScreenTopRight( tuple );
      this->ReadTuple( tuple, 3 );
      camera->SetScreenTopRight( tuple );
      isScreenTopRightFound = true;
    }
    else
    {
      vtkWarningMacro(
        "Found unexpected element \"" << ( char* )( this->CurrentNode.Name ) <<
        "\" while reading Camera." );
    }
  }

  // warn if any expected elements were not found
  if( !isClippingRangeFound )
    vtkWarningMacro( "Expected element \"ClippingRange\" was not found while reading Camera." );
  if( !isDistanceFound )
    vtkWarningMacro( "Expected element \"Distance\" was not found while reading Camera." );
  if( !isFocalDiskFound )
    vtkWarningMacro( "Expected element \"FocalDisk\" was not found while reading Camera." );
  if( !isFocalPointFound )
    vtkWarningMacro( "Expected element \"FocalPoint\" was not found while reading Camera." );
  if( !isViewShearFound )
    vtkWarningMacro( "Expected element \"ViewShear\" was not found while reading Camera." );
  if( !isParallelProjectionFound )
    vtkWarningMacro( "Expected element \"ParallelProjection\" was not found while reading Camera." );
  if( !isParallelScaleFound )
    vtkWarningMacro( "Expected element \"ParallelScale\" was not found while reading Camera." );
  if( !isPositionFound )
    vtkWarningMacro( "Expected element \"Position\" was not found while reading Camera." );
  if( !isLeftEyeFound )
    vtkWarningMacro( "Expected element \"LeftEye\" was not found while reading Camera." );
  if( !isThicknessFound )
    vtkWarningMacro( "Expected element \"Thickness\" was not found while reading Camera." );
  if( !isViewAngleFound )
    vtkWarningMacro( "Expected element \"ViewAngle\" was not found while reading Camera." );
  if( !isUseHorizontalViewAngleFound )
    vtkWarningMacro( "Expected element \"UseHorizontalViewAngle\" was not found while reading Camera." );
  if( !isViewUpFound )
    vtkWarningMacro( "Expected element \"ViewUp\" was not found while reading Camera." );
  if( !isWindowCenterFound )
    vtkWarningMacro( "Expected element \"WindowCenter\" was not found while reading Camera." );
  if( !isScreenBottomLeftFound )
    vtkWarningMacro( "Expected element \"ScreenBottomLeft\" was not found while reading Camera." );
  if( !isScreenBottomRightFound )
    vtkWarningMacro( "Expected element \"ScreenBottomRight\" was not found while reading Camera." );
  if( !isScreenTopRightFound )
    vtkWarningMacro( "Expected element \"ScreenTopRight\" was not found while reading Camera." );
}
