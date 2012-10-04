/*=========================================================================

  Program:  CLSAVis (CLSA Ultrasound Image Viewer)
  Module:   vtkCVView.h
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/
//
// .NAME vtkCVView - The application's main render view.
//
// .SECTION Description
//

#ifndef __cvView_h
#define __cvView_h

#include "vtkRenderView.h"

#include "CVUtilities.h"

class vtkCVView : public vtkRenderView
{
public:
  static vtkCVView *New();
  vtkTypeMacro( vtkCVView, vtkRenderView );
  void PrintSelf( ostream& os, vtkIndent indent );
  void SaveScreenshot( const char* fileName, int magnification = 1 );
  void DefaultView();
  void SetPNGFile( const char* fileName );

protected:
  vtkCVView() { this->DefaultView(); }
  ~vtkCVView();
  
private:
  vtkCVView( const vtkCVView& );  // Not implemented.
  void operator=( const vtkCVView& );  // Not implemented.
};

#endif
