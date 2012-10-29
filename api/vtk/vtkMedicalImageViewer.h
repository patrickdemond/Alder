/*=========================================================================

  Program:   Alder (CLSA Ultrasound Image Viewer)
  Module:    vtkMedicalImageViewer.h
  Language:  C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/
// .NAME vtkMedicalImageViewer - Display a 2D image.
// .SECTION Description
// vtkMedicalImageViewer is a convenience class for displaying a 2D image.  It
// packages up the functionality found in vtkRenderWindow, vtkRenderer,
// vtkImageActor and vtkImageMapToWindowLevelColors into a single easy to use
// class.  This class also creates an image interactor style
// (vtkInteractorStyleImage) that allows zooming and panning of images, and
// supports interactive window/level operations on the image. Note that
// vtkMedicalImageViewer is simply a wrapper around these classes.
//
// vtkMedicalImageViewer uses the 3D rendering and texture mapping engine
// to draw an image on a plane.  This allows for rapid rendering,
// zooming, and panning. The image is placed in the 3D scene at a
// depth based on the z-coordinate of the particular image slice. Each
// call to SetSlice() changes the image data (slice) displayed AND
// changes the depth of the displayed slice in the 3D scene. This can
// be controlled by the AutoAdjustCameraClippingRange ivar of the
// InteractorStyle member.
//
// It is possible to mix images and geometry, using the methods:
//
// viewer->SetInput( myImage );
// viewer->GetRenderer()->AddActor( myActor );
//
// This can be used to annotate an image with a PolyData of "edges" or
// or highlight sections of an image or display a 3D isosurface
// with a slice from the volume, etc. Any portions of your geometry
// that are in front of the displayed slice will be visible; any
// portions of your geometry that are behind the displayed slice will
// be obscured. A more general framework (with respect to viewing
// direction) for achieving this effect is provided by the
// vtkImagePlaneWidget .
//
// Note that pressing 'r' will reset the window/level and pressing
// shift+'r' or control+'r' will reset the camera.
//
// .SECTION See Also
// vtkRenderWindow vtkRenderer vtkImageActor vtkImageMapToWindowLevelColors

#ifndef __vtkMedicalImageViewer_h
#define __vtkMedicalImageViewer_h

#include "vtkObject.h"
#include <vector>

class vtkImageActor;
class vtkImageData;
class vtkImageMapToWindowLevelColors;
class vtkInteractorStyleImage;
class vtkRenderWindow;
class vtkRenderer;
class vtkRenderWindowInteractor;

class vtkMedicalImageViewer : public vtkObject 
{
public:
  static vtkMedicalImageViewer *New();
  vtkTypeMacro(vtkMedicalImageViewer,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Render the resulting image.
  virtual void Render( void );
  
  // Description:
  // Set/Get the input image to the viewer.
  virtual void SetInput( vtkImageData* );
  virtual vtkImageData* GetInput();

  // Set/get the slice orientation
  enum
  {
    VIEW_ORIENTATION_YZ = 0,
    VIEW_ORIENTATION_XZ = 1,
    VIEW_ORIENTATION_XY = 2
  };
  vtkGetMacro( ViewOrientation, int );
  virtual void SetViewOrientation( const int& );
  virtual void SetViewOrientationToXY()
    { this->SetViewOrientation(vtkMedicalImageViewer::VIEW_ORIENTATION_XY); };
  virtual void SetViewOrientationToYZ()
    { this->SetViewOrientation(vtkMedicalImageViewer::VIEW_ORIENTATION_YZ); };
  virtual void SetViewOrientationToXZ()
    { this->SetViewOrientation(vtkMedicalImageViewer::VIEW_ORIENTATION_XZ); };

  // Description:
  // Set/Get the current slice to display (depending on the orientation
  // this can be in X, Y or Z).
  vtkGetMacro( Slice, int );
  virtual void SetSlice( int );

  // Description:
  // Get the location of the slice.
  double GetSliceLocation();

  // Description:
  // Update the display extent manually so that the proper slice for the
  // given orientation is displayed. It will also try to set a
  // reasonable camera clipping range.
  // This method is called automatically when the Input is changed, but
  // most of the time the input of this class is likely to remain the same,
  // i.e. connected to the output of a filter, or an image reader. When the
  // input of this filter or reader itself is changed, an error message might
  // be displayed since the current display extent is probably outside
  // the new whole extent. Calling this method will ensure that the display
  // extent is reset properly.
  virtual void UpdateDisplayExtent();
  
  // Description:
  // Return the minimum and maximum slice values (depending on the orientation
  // this can be in X, Y or Z).
  virtual int GetSliceMin();
  virtual int GetSliceMax();
  virtual void GetSliceRange( int range[2] )
    { this->GetSliceRange(range[0], range[1]); }
  virtual void GetSliceRange( int &min, int &max );
  virtual int* GetSliceRange();
  
  // Description:
  // Get the internal render window, renderer, image actor, and
  // image map instances.
  vtkGetObjectMacro( RenderWindow, vtkRenderWindow );
  vtkGetObjectMacro( Renderer, vtkRenderer );
  vtkGetObjectMacro( ImageActor, vtkImageActor );
  vtkGetObjectMacro( WindowLevel, vtkImageMapToWindowLevelColors );
  vtkGetObjectMacro( InteractorStyle, vtkInteractorStyleImage );
  
  // Description:
  // Set your own renderwindow and renderer.
  virtual void SetRenderWindow( vtkRenderWindow* );
  virtual void SetRenderer( vtkRenderer* );
  virtual void SetInteractor( vtkRenderWindowInteractor* );

  // Description:
  // Convenience methods to set up the underlying vtkImageMapToWindowLevelColors.
  void SetMappingToLuminance();
  void SetMappingToColorAlpha();
  void SetMappingToColor();

  // Description:
  // Methods to control window level.
  double GetColorWindow() { return this->Window; }
  double GetColorLevel() { return this->Level; }
  void SetColorWindowLevel( const double& , const double& );
  void DoStartWindowLevel();
  void DoResetWindowLevel();
  void DoWindowLevel();

  // Description:
  // Convenience to set the window and level of an image to be the 
  // same as what was used for the previously input image.  Default is off.
  vtkSetMacro( MaintainLastWindowLevel, int );
  vtkGetMacro( MaintainLastWindowLevel, int );
  vtkBooleanMacro( MaintainLastWindowLevel, int );

  // Description:
  // Get the dimensionality of the image, eg., 3D or 2D.
  int GetImageDimensionality();

  // Description:
  // Create a default 3D sinusoidal image.
  void SetImageToSinusoid();

  // Description:
  // Methods to scroll through 3D images.
  void CinePlay();
  void CineLoop();
  void CineRewind();
  void CineStop();

  enum
  {
    PLAY,
    STOP
  };


  // Description:
  // Get the event id tags for playing and stopping cine loops.
  vtkGetMacro( PlayEvent, int );
  vtkGetMacro( StopEvent, int );

protected:
  vtkMedicalImageViewer();
  ~vtkMedicalImageViewer();

  virtual void InstallPipeline();
  virtual void UnInstallPipeline();

  vtkImageMapToWindowLevelColors  *WindowLevel;
  vtkRenderWindow                 *RenderWindow;
  vtkRenderer                     *Renderer;
  vtkImageActor                   *ImageActor;
  vtkRenderWindowInteractor       *Interactor;
  vtkInteractorStyleImage         *InteractorStyle;

  int Slice;
  int LastSlice[3];

  int PlayEvent;
  int StopEvent;
  int CineState;

  int MaintainLastWindowLevel;
  double OriginalWindow;
  double OriginalLevel;
  double InitialWindow;
  double InitialLevel;
  double Window;
  double Level;
  std::vector<unsigned long> WindowLevelCallbackTags;

  void InitializeWindowLevel();

  int ViewOrientation;
  double CameraPosition[3][3];
  double CameraFocalPoint[3][3];
  double CameraViewUp[3][3];
  double CameraParallelScale[3];

  void InitializeCameraViews();
  void RecordCameraView();

private:
  vtkMedicalImageViewer(const vtkMedicalImageViewer&);  // Not implemented.
  void operator=(const vtkMedicalImageViewer&);  // Not implemented.
};

#endif
