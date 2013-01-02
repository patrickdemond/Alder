/*=========================================================================

  Program:   Alder ( CLSA Ultrasound Image Viewer )
  Module:    vtkImageCoordinateWidget.cxx
  Language:  C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/
#include "vtkImageCoordinateWidget.h"

#include "vtkActor.h"
#include "vtkAssemblyNode.h"
#include "vtkAssemblyPath.h"
#include "vtkCallbackCommand.h"
#include "vtkCamera.h"
#include "vtkCellArray.h"
#include "vtkHomogeneousTransform.h"
#include "vtkImageActor.h"
#include "vtkImageData.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPropPicker.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"

vtkStandardNewMacro( vtkImageCoordinateWidget );

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
vtkImageCoordinateWidget::vtkImageCoordinateWidget()
{
  this->EventCallbackCommand->SetCallback(
    vtkImageCoordinateWidget::ProcessEvents );
  this->EventCallbackCommand->SetPassiveObserver( 1 ); // get events first

  this->State = vtkImageCoordinateWidget::Start;
  this->CursoringMode = vtkImageCoordinateWidget::Continuous;

  this->CurrentCursorPosition[0] = 0;
  this->CurrentCursorPosition[1] = 0;
  this->CurrentCursorPosition[2] = 0;
  this->CurrentImageValue.empty();

  this->PropCollection = vtkPropCollection::New();
  this->ImageData = NULL;

  this->Picker = vtkPropPicker::New();
  this->Picker->PickFromListOn();

  this->MessageString = "NA";

  this->UserTransform = NULL;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
vtkImageCoordinateWidget::~vtkImageCoordinateWidget()
{
  if( this->ImageData )
  {
    this->ImageData = NULL;
  }

  this->RemoveAllProps();
  if ( this->Picker )
  {
    this->Picker->Delete();
    this->Picker = NULL;
  }  
  if ( this->PropCollection )
  {
    this->PropCollection->Delete();
    this->PropCollection = NULL;
  }
  if ( this->UserTransform )
  {
    this->UserTransform->UnRegister( this );
    this->UserTransform = NULL;
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkImageCoordinateWidget::SetPicker( vtkAbstractPropPicker* picker )
{
  // we have to have a picker for the cursor to work
  if( this->Picker != picker && picker != NULL )
  {
    // to avoid destructor recursion
    vtkAbstractPropPicker *temp = this->Picker;
    this->Picker = picker;
    if( temp )
    {
      temp->UnRegister( this );
    }
    if( this->Picker )
    {
      this->Picker->Register( this );
      this->Picker->PickFromListOn();
      for( int i = 0; i < this->GetNumberOfProps(); ++i )
      {
        this->Picker->AddPickList( this->GetNthProp( i ) );
      }
    }
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkImageCoordinateWidget::SetUserTransform( vtkHomogeneousTransform *transform )
{
  if( transform == this->UserTransform )
  {
    return;
  }
  if( this->UserTransform )
  {
    this->UserTransform->Delete();
    this->UserTransform = NULL;
  }
  if( transform )
  {
    this->UserTransform = transform;
    this->UserTransform->Register( this );
  }
  this->Modified();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkImageCoordinateWidget::AddViewProp( vtkProp *prop )
{
  this->PropCollection->AddItem( prop );
  this->Picker->AddPickList( prop );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkImageCoordinateWidget::RemoveProp( vtkProp *prop )
{
  this->PropCollection->RemoveItem( prop );
  this->Picker->DeletePickList( prop );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkImageCoordinateWidget::RemoveAllProps()
{
  this->PropCollection->RemoveAllItems();
  this->Picker->InitializePickList();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkImageCoordinateWidget::RemoveProps( vtkPropCollection* props )
{
  if( !props ) return;
  vtkProp *aProp;
  vtkCollectionSimpleIterator pit;
  for( props->InitTraversal( pit );
      ( aProp = props->GetNextProp( pit ) ); )
  {
     this->RemoveProp( aProp );
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
int vtkImageCoordinateWidget::HasProp( vtkProp *prop )
{
  return this->PropCollection->IsItemPresent( prop );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
int vtkImageCoordinateWidget::GetNumberOfProps()
{
  return this->PropCollection->GetNumberOfItems();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
vtkProp* vtkImageCoordinateWidget::GetNthProp( int id )
{
  return vtkProp::SafeDownCast( this->PropCollection->GetItemAsObject( id ) );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkImageCoordinateWidget::SetCursoringMode( int mode )
{
  if( this->CursoringMode == mode )
  {
    return;
  }

  this->CursoringMode = mode < 0 ? 0 : mode > 1 ? 1 : mode;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkImageCoordinateWidget::SetInput( vtkDataSet* input )
{
  this->ImageData = vtkImageData::SafeDownCast( input );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkImageCoordinateWidget::SetEnabled( int enabling )
{
  if( !this->Interactor )
  {
    return;
  }

  if( enabling ) //----------------------------------------------------------
  {
    vtkDebugMacro( <<"Enabling vtkImageCoordinateWidget" );

    if( this->Enabled ) //already enabled, just return
    {
      return;
    }

    if( !this->CurrentRenderer )
    {
      this->SetCurrentRenderer( this->Interactor->FindPokedRenderer(
        this->Interactor->GetLastEventPosition()[0],
        this->Interactor->GetLastEventPosition()[1] ) );
      if( this->CurrentRenderer == NULL )
      {
        return;
      }
    }

    this->Enabled = 1;

    // we have to honour this ivar: it could be that this->Interaction was
    // set to off when we were disabled

    this->Interactor->AddObserver( vtkCommand::MouseMoveEvent,
      this->EventCallbackCommand, this->Priority );

    this->InvokeEvent( vtkCommand::EnableEvent, NULL );

    this->EventCallbackCommand->SetAbortFlag( 1 );
    this->StartInteraction();
    this->InvokeEvent( vtkCommand::StartInteractionEvent, NULL );
    this->Interactor->Render();
    this->State = vtkImageCoordinateWidget::Cursoring;
  }
  else //disabling----------------------------------------------------------
  {
    vtkDebugMacro( <<"Disabling vtkImageCoordinateWidget" );

    if( !this->Enabled ) //already disabled, just return
    {
      return;
    }

    this->State = vtkImageCoordinateWidget::Start;

    this->EventCallbackCommand->SetAbortFlag( 1 );
    this->EndInteraction();
    this->InvokeEvent( vtkCommand::EndInteractionEvent, NULL );
    this->Interactor->Render();

    this->Enabled = 0;

    // don't listen for events any more
    this->Interactor->RemoveObserver( this->EventCallbackCommand );

    this->InvokeEvent( vtkCommand::DisableEvent, NULL );
    this->SetCurrentRenderer( NULL );
  }

  this->Interactor->Render();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkImageCoordinateWidget::ProcessEvents( vtkObject* vtkNotUsed( object ),
                                        unsigned long event,
                                        void* clientdata,
                                        void* vtkNotUsed( calldata ) )
{
  vtkImageCoordinateWidget* self =
    reinterpret_cast<vtkImageCoordinateWidget *>( clientdata );

  if( event == vtkCommand::MouseMoveEvent )
  {
    self->OnMouseMove();
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkImageCoordinateWidget::OnMouseMove()
{
  // See whether we're active
  //
  if( this->State == vtkImageCoordinateWidget::Outside ||
       this->State == vtkImageCoordinateWidget::Start )
  {
    return;
  }

  int X = this->Interactor->GetEventPosition()[0];
  int Y = this->Interactor->GetEventPosition()[1];

  if( this->State == vtkImageCoordinateWidget::Cursoring )
  {
    this->UpdateCursor( X, Y );
  }

  // Interact, if desired
  //
  this->EventCallbackCommand->SetAbortFlag( 1 );
  this->InvokeEvent( vtkCommand::InteractionEvent, NULL );
  this->Interactor->Render();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
int vtkImageCoordinateWidget::GetCursorPosition( double& x, double& y, double& z )
{
  if( this->State != vtkImageCoordinateWidget::Cursoring )
  {
    return 0;
  }

  x = this->CurrentCursorPosition[0];
  y = this->CurrentCursorPosition[1];
  z = this->CurrentCursorPosition[2];

  return 1;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
int vtkImageCoordinateWidget::GetCursorDataN( double *v, int components )
{
  if( this->State != vtkImageCoordinateWidget::Cursoring )
  {
    return 0;
  }

  int c;
  int currSize = static_cast<int>(this->CurrentImageValue.size());

  for( c = 0; c < components; ++c )
  {
    if( c < currSize )
    {
      v[c] = this->CurrentImageValue[ c ];
    }
    else
    {
      // anything out of range gets the max double value
      v[c] = VTK_DOUBLE_MAX;
    }
  }

  return 1;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
int vtkImageCoordinateWidget::GetCursorData1( double& v1 )
{
  int retVal;
  double v[1];

  retVal = this->GetCursorDataN( v, 1 );
  if( retVal )
  {
    v1 = v[0];
  }

  return retVal;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
int vtkImageCoordinateWidget::GetCursorData2( double& v1, double& v2 )
{
  int retVal;
  double v[2];

  retVal = this->GetCursorDataN( v, 2 );
  if( retVal )
  {
    v1 = v[0];
    v2 = v[1];
  }

  return retVal;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
int vtkImageCoordinateWidget::GetCursorData3( double& v1, double& v2,
  double& v3 )
{
  int retVal;
  double v[3];

  retVal = this->GetCursorDataN( v, 3 );
  if( retVal )
  {
    v1 = v[0];
    v2 = v[1];
    v3 = v[2];
  }

  return retVal;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
int vtkImageCoordinateWidget::GetCursorData4( double& v1, double& v2,
  double& v3, double& v4 )
{
  int retVal;
  double v[4];

  retVal = this->GetCursorDataN( v, 4 );
  if( retVal )
  {
    v1 = v[0];
    v2 = v[1];
    v3 = v[2];
    v4 = v[3];
  }

  return retVal;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
int vtkImageCoordinateWidget::GetCursorData9(
  double& v1, double& v2, double& v3, double& v4,
  double& v5, double& v6, double& v7, double& v8, double& v9 )
{
  int retVal;
  double v[9];

  retVal = this->GetCursorDataN( v, 9 );
  if( retVal )
  {
    v1 = v[0];
    v2 = v[1];
    v3 = v[2];
    v4 = v[3];
    v5 = v[4];
    v6 = v[5];
    v7 = v[6];
    v8 = v[7];
    v9 = v[8];
  }

  return retVal;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkImageCoordinateWidget::UpdateCursor( int X, int Y )
{
  this->MessageString = "Off Image";

  if( this->GetNumberOfProps() == 0 ) return;

  // try to use the actor's image data if it is a single vtkImageActor
  if( !this->ImageData )
  {
    vtkImageActor* actor = vtkImageActor::SafeDownCast( this->GetNthProp( 0 ) );
    if( !actor ||  !(this->ImageData = actor->GetInput()) )
    {
      return;
    }
  }

  // We're going to be extracting values with GetScalarComponentAsDouble(),
  // we might as well make sure that the data is there.  If the data is
  // up to date already, this call doesn't cost very much.  If we don't make
  // this call and the data is not up to date, the GetScalar() call will
  // cause a segfault.
  this->ImageData->Update();

  this->Picker->Pick( X, Y, 0.0, this->CurrentRenderer );
  vtkAssemblyPath* path = this->Picker->GetPath();

  vtkProp* pickedProp = NULL;
  if( path  )
  {
    // Deal with the possibility that we may be using a shared picker
    vtkCollectionSimpleIterator sit;
    path->InitTraversal( sit );
    vtkAssemblyNode *node;
    for( int i = 0; i < path->GetNumberOfItems(); ++i )
    {
      node = path->GetNextNode( sit );
      pickedProp = node->GetViewProp();
      if( this->HasProp( pickedProp ) )
      {
        break;
      }
    }
  }

  if( !pickedProp ) return;

  double q[3];
  this->Picker->GetPickPosition( q );
  double* bounds = pickedProp->GetBounds();

  if( bounds[0] == bounds[1] )      //YZ
  {
    q[0] = bounds[0];   // fixes some numerical problems with continuous cursoring
  }
  else if( bounds[2] == bounds[3] ) //XZ
  {
    q[1] = bounds[2];
  }
  else if( bounds[4] == bounds[5] ) //XY
  {
    q[2] = bounds[4];
  }

  if( this->UserTransform )
  {
    this->UserTransform->TransformPoint( q, q );
  }

  if( this->CursoringMode == vtkImageCoordinateWidget::Continuous )
  {
    this->UpdateContinuousCursor( q );
  }
  else
  {
    this->UpdateDiscreteCursor( q );
  }
  
  this->MessageString = "Location: (";
  for ( int i = 0 ; i < 3; ++i )
  {
     this->MessageString += vtkVariant( this->CurrentCursorPosition[i] ).ToString();
     if( i < 2 )  this->MessageString += ", ";
  }
  this->MessageString += ")";

  int components = this->CurrentImageValue.size();
  if( 0 < components )
  {
    double v[9];
    this->MessageString += " Value: ";
    // if we only have one component then do not use round brackets
    if( 1 == components )
    {
      this->MessageString += 
        vtkVariant( this->CurrentImageValue[0] ).ToString();
    }
    else
    {
      this->MessageString += "( ";
      for( int c = 0; c < components; ++c )
      {
        this->MessageString += 
          vtkVariant( this->CurrentImageValue[c] ).ToString();
        if( (c + 1) < components ) this->MessageString += ", ";
      }
      this->MessageString += " )";
    }
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkImageCoordinateWidget::UpdateContinuousCursor( double *q )
{
  // empty the current image value
  this->CurrentImageValue.empty();

  this->CurrentCursorPosition[0] = q[0];
  this->CurrentCursorPosition[1] = q[1];
  this->CurrentCursorPosition[2] = q[2];

  vtkPointData* pd = this->ImageData->GetPointData();
  if( !pd ) return;

  vtkPointData* outPD = vtkPointData::New();
  outPD->InterpolateAllocate( pd, 1, 1 );

  // Use tolerance as a function of size of source data
  //
  double tol2 = this->ImageData->GetLength();
  tol2 = tol2 ? tol2*tol2 / 1000.0 : 0.001;

  // Find the cell that contains q and get it
  //
  int subId;
  double pcoords[3], weights[8];
  vtkCell* cell = this->ImageData->FindAndGetCell(
    q, NULL, -1, tol2, subId, pcoords, weights );
  if( cell )
  {
    int components;
    double* tuple;

    // Interpolate the point data
    //
    outPD->InterpolatePoint( pd, 0, cell->PointIds, weights );
    components = outPD->GetScalars()->GetNumberOfComponents();
    tuple = outPD->GetScalars()->GetTuple( 0 );

    this->CurrentImageValue.resize( components );
    for( int c = 0; c < components; ++c )
    {
      this->CurrentImageValue[ c ] = tuple[ c ];
    }
  }

  outPD->Delete();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkImageCoordinateWidget::UpdateDiscreteCursor( double *q )
{
  // empty the current image value
  this->CurrentImageValue.empty();

  // vtkImageData will find the nearest implicit point to q
  //
  vtkIdType ptId = this->ImageData->FindPoint( q );

  if( ptId == -1 ) return;

  double closestPt[3];
  this->ImageData->GetPoint( ptId, closestPt );

  double* origin = this->ImageData->GetOrigin();
  double* spacing = this->ImageData->GetSpacing();
  int* extent = this->ImageData->GetExtent();

  int iq[3];
  int iqtemp;
  for( int i = 0; i < 3; ++i )
  {
    // compute world to image coords
    iqtemp = static_cast<int>( (( closestPt[i] - origin[i] ) / spacing[i]) + 0.5 );

    // we have a valid pick already, just enforce bounds check
    iq[i] = ( iqtemp < extent[2*i] ) ? extent[2*i] : \
          ( ( iqtemp > extent[2*i+1] ) ? extent[2*i+1] : iqtemp );

    // compute image to world coords
    q[i] = iq[i]*spacing[i] + origin[i]; 

    this->CurrentCursorPosition[i] = iq[i];
  }

  int components = this->ImageData->GetNumberOfScalarComponents();
  this->CurrentImageValue.resize( components );
  int x = static_cast<int>( this->CurrentCursorPosition[0] );
  int y = static_cast<int>( this->CurrentCursorPosition[1] );
  int z = static_cast<int>( this->CurrentCursorPosition[2] );
  for( int c = 0; c < components; ++c )
  {
    this->CurrentImageValue[c] = 
      this->ImageData->GetScalarComponentAsDouble( x, y, z, c );
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkImageCoordinateWidget::PrintSelf( ostream& os, vtkIndent indent )
{
  this->Superclass::PrintSelf( os,indent );
  os << indent << "CursoringMode: " << this->CursoringMode << endl;
}
