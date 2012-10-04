/*=========================================================================

  Program:  CLSAVis (Canadian Longitudinal Study on Aging Visualizer)
  Module:   vtkCVView.cxx
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/

#include "vtkCVView.h"

#include "vtkActor.h"
#include "vtkCamera.h"
#include "vtkCellArray.h"
#include "vtkFloatArray.h"
#include "vtkObjectFactory.h"
#include "vtkPNGWriter.h"
#include "vtkPolyData.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPolyDataMapper.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkSmartPointer.h"
#include "vtkWindowToImageFilter.h"

#include "vtkPNGReader.h"
#include "vtkInteractorStyleImage.h"
#include "vtkImageActor.h"
#include "vtkImageMapper3D.h"

vtkStandardNewMacro( vtkCVView );

void vtkCVView::DefaultView()
{
  static float x[8][3] = { { 0, 0, 0 }, { 1, 0, 0 }, { 1, 1, 0 }, { 0, 1, 0 },
                           { 0, 0, 1 }, { 1, 0, 1 }, { 1, 1, 1 }, { 0, 1, 1 } };
  static vtkIdType pts[6][4] = { { 0, 1, 2, 3 }, { 4, 5, 6, 7 }, { 0, 1, 5, 4 },
                                 { 1, 2, 6, 5 }, { 2, 3, 7, 6 }, { 3, 0, 4, 7 } };
  
  // Load the point, cell, and data attributes.
  vtkSmartPointer< vtkPoints > points = vtkSmartPointer< vtkPoints >::New();
  for( int i = 0; i < 8; i++ ) points->InsertPoint( i, x[i] );
  vtkSmartPointer< vtkCellArray > polys = vtkSmartPointer< vtkCellArray >::New();
  for( int i = 0; i < 6; i++ ) polys->InsertNextCell( 4, pts[i] );
  vtkSmartPointer< vtkFloatArray > scalars = vtkSmartPointer< vtkFloatArray >::New();
  for( int i = 0; i < 8; i++ ) scalars->InsertTuple1( i, i );

  // We now assign the pieces to the vtkPolyData.
  vtkSmartPointer< vtkPolyData > cube = vtkSmartPointer< vtkPolyData >::New();
  cube->SetPoints( points );
  cube->SetPolys( polys );
  cube->GetPointData()->SetScalars( scalars );

  // Now we'll look at it.
  vtkSmartPointer< vtkPolyDataMapper > cubeMapper = vtkSmartPointer< vtkPolyDataMapper >::New();
  cubeMapper->SetInput( cube );
  cubeMapper->SetScalarRange( 0, 7 );
  vtkSmartPointer< vtkActor > cubeActor = vtkSmartPointer< vtkActor >::New();
  cubeActor->SetMapper( cubeMapper );

  this->GetRenderer()->GetActiveCamera()->SetPosition( 1, 1, 1 );
  this->GetRenderer()->GetActiveCamera()->SetFocalPoint( 0, 0, 0 );
  this->GetRenderer()->AddActor( cubeActor );
  this->GetRenderer()->ResetCamera();
}

vtkCVView::~vtkCVView()
{
}

void vtkCVView::SaveScreenshot( const char* fileName, int magnification )
{
  vtkSmartPointer< vtkWindowToImageFilter > filter = vtkSmartPointer< vtkWindowToImageFilter >::New();
  vtkSmartPointer< vtkPNGWriter > writer = vtkSmartPointer< vtkPNGWriter >::New();

  filter->SetInput( this->GetRenderWindow() );
  filter->SetMagnification( magnification );
  filter->Update();
  writer->SetInput( filter->GetOutput() );
  writer->SetFileName( fileName );
  writer->Write();
}

void vtkCVView::SetPNGFile( const char* fileName )
{
  vtkSmartPointer< vtkPNGReader > pngReader = vtkSmartPointer< vtkPNGReader >::New();
  pngReader->SetFileName( fileName );
 
  vtkSmartPointer<vtkImageActor> actor = vtkSmartPointer<vtkImageActor>::New();
  actor->GetMapper()->SetInputConnection(pngReader->GetOutputPort());  

  this->GetRenderer()->AddActor( actor );

  vtkSmartPointer<vtkInteractorStyleImage> style = vtkSmartPointer<vtkInteractorStyleImage>::New();
 
  this->GetRenderer()->ResetCamera();
  double *bounds = actor->GetBounds();
  this->GetRenderer()->GetActiveCamera()->SetViewUp( 0, 1, 0 );
  this->GetRenderer()->GetActiveCamera()->SetPosition(
    0.5*( bounds[0] + bounds[1] ),
    0.5*( bounds[2] + bounds[3] ),
    1 );
  this->GetRenderer()->GetActiveCamera()->SetFocalPoint(
    0.5*( bounds[0] + bounds[1] ),
    0.5*( bounds[2] + bounds[3] ),
    0 );
  this->GetRenderWindow()->GetInteractor()->SetInteractorStyle( style );
  this->GetRenderer()->ResetCamera();
}

void vtkCVView::PrintSelf( ostream& os, vtkIndent indent )
{
  Superclass::PrintSelf( os, indent );
}
