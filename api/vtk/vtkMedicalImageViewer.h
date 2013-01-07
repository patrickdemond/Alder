/*=======================================================================

  Module:    vtkMedicalImageViewer.h
  Program:   Alder (CLSA Medical Image Quality Assessment Tool)
  Language:  C++
  Author:    Patrick Emond <emondpd@mcmaster.ca>
  Author:    Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/

/** 
 * @class vtkMedicalImageViewer
 *
 * @author Patrick Emond <emondpd@mcmaster.ca>
 * @author Dean Inglis <inglisd@mcmaster.ca>
 *
 * @brief Display a 2D image.
 *
 * vtkMedicalImageViewer is a convenience class for 
 * displaying a 2D image.  It packages up the functionality found in 
 * vtkRenderWindow, vtkRenderer, vtkImageActor and 
 * vtkImageMapToWindowLevelColors into a single easy to use
 * class.  This class also creates an image interactor style
 * (vtkInteractorStyleImage) that allows zooming and panning of images, and
 * supports interactive window/level operations on the image. Note that
 * vtkMedicalImageViewer is simply a wrapper around these classes.
 *
 * vtkMedicalImageViewer uses the 3D rendering and texture mapping engine
 * to draw an image on a plane.  This allows for rapid rendering,
 * zooming, and panning. The image is placed in the 3D scene at a
 * depth based on the z-coordinate of the particular image slice. Each
 * call to SetSlice() changes the image data (slice) displayed AND
 * changes the depth of the displayed slice in the 3D scene. This can
 * be controlled by the AutoAdjustCameraClippingRange ivar of the
 * InteractorStyle member.
 *
 * It is possible to mix images and geometry, using the methods:
 * viewer->SetInput( myImage );
 * viewer->GetRenderer()->AddActor( myActor );
 *
 * This can be used to annotate an image with a PolyData of "edges" or
 * or highlight sections of an image or display a 3D isosurface
 * with a slice from the volume, etc. Any portions of your geometry
 * that are in front of the displayed slice will be visible; any
 * portions of your geometry that are behind the displayed slice will
 * be obscured. A more general framework (with respect to viewing
 * direction) for achieving this effect is provided by the
 * vtkImagePlaneWidget .
 *
 * Note that pressing 'r' will reset the window/level and pressing
 * shift+'r' or control+'r' will reset the camera.
 *
 * @see vtkRenderWindow vtkRenderer vtkImageActor vtkImageMapToWindowLevelColors
 */
#ifndef __vtkMedicalImageViewer_h
#define __vtkMedicalImageViewer_h

#include "vtkObject.h"
#include <vector>

class vtkCornerAnnotation;
class vtkImageActor;
class vtkImageCoordinateWidget;
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
  vtkTypeMacro( vtkMedicalImageViewer, vtkObject );
  void PrintSelf( ostream& os, vtkIndent indent );

  /**
   * Force render the display.
   * In some situations, a pipeline update will not cause a 
   * refresh render.  This method allows for forcing a render.
   */
  virtual void Render( void );
  
  //@{
  /** 
   * Set/Get the input image to the viewer.
   * @param input vtkImageData from the output of a reader
   */
  virtual void SetInput( vtkImageData* input );
  virtual vtkImageData* GetInput();
  //@}

  /**
   * Load an image from file and display.
   * 
   * If fileName is valid, load the file via vtkGDCMImageReader and display it.
   * Returns fails if image fails to load.
   * @param fileName Name of a file on disk
   * @return boolean
   */
   bool Load( std::string fileName );

  /**
   * Enum constants for orthonormal slice orientations. */
  enum
  {
    VIEW_ORIENTATION_YZ = 0,  /**< enum value VIEW_ORIENTATION_YZ. */
    VIEW_ORIENTATION_XZ = 1,  /**< enum value VIEW_ORIENTATION_XZ. */
    VIEW_ORIENTATION_XY = 2   /**< enum value VIEW_ORIENTATION_XY. */
  };

  //@{
  /**
   * Set/Get the orthonormal view orientation. */
  vtkGetMacro( ViewOrientation, int );
  virtual void SetViewOrientation( const int& );
  virtual void SetViewOrientationToXY()
    { this->SetViewOrientation(vtkMedicalImageViewer::VIEW_ORIENTATION_XY); };
  virtual void SetViewOrientationToYZ()
    { this->SetViewOrientation(vtkMedicalImageViewer::VIEW_ORIENTATION_YZ); };
  virtual void SetViewOrientationToXZ()
    { this->SetViewOrientation(vtkMedicalImageViewer::VIEW_ORIENTATION_XZ); };
  //@}

  //@{
  /**
   * Set/Get the current slice index to display.
   * Depending on the orientation, this can be an X, Y or Z slice.
   * @see GetSliceLocation()
   */
  vtkGetMacro( Slice, int );
  virtual void SetSlice( int );
  //@}

  /**
   * Get the non-indexed location of the slice from the origin.
   * @sa GetSlice(), SetSlice()
   */
  double GetSliceLocation();

  /**
   * Update the display extent.
   * Update the display extent so that the proper slice for the
   * given orientation is displayed. It will also try to set a
   * reasonable camera clipping range.
   * This method is called automatically when the Input is changed, but
   * most of the time the input of this class is likely to remain the same,
   * i.e. connected to the output of a filter, or an image reader. When the
   * input of this filter or reader itself is changed, an error message might
   * be displayed since the current display extent is probably outside
   * the new whole extent. Calling this method will ensure that the display
   * extent is reset properly.
   */
  virtual void UpdateDisplayExtent();
  
  /**
   * Get the minimum slice index. 
   * @sa GetSliceMax(), GetSliceRange()
   */
  virtual int GetSliceMin();

  /**
   * Get the maximum slice index. 
   * @sa GetSliceMin(), GetSliceRange()
   */
  virtual int GetSliceMax();

  /**
   * Return the minimum and maximum slice index values.
   * @param range int array to put the range of slices in.
   * @sa GetSliceMin(), GetSliceMax()
   */
  virtual void GetSliceRange( int range[2] )
    { this->GetSliceRange(range[0], range[1]); }
    
  /**
   * Return the minimum and maximum slice index values.
   * @param min fill the minimum slice by reference
   * @param max fill the minimum slice by reference
   * @sa GetSliceMin(), GetSliceMax()
   */
  virtual void GetSliceRange( int &min, int &max );

  /**
   * Return the minimum and maximum slice index values.
   * Warning: this method is NOT thread safe.
   * @return pointer to input image's whole extent array
   * @sa GetSliceMin(), GetSliceMax()
   */
  virtual int* GetSliceRange();
  
  //@{
  /**
   * Set/Get VTK object member ivars.
   * Class member ivars are accessible for connecting to third party
   * GUI toolkits (e.g., Qt), or for custom configuration.
   */
  vtkGetObjectMacro( RenderWindow, vtkRenderWindow );
  vtkGetObjectMacro( Renderer, vtkRenderer );
  vtkGetObjectMacro( ImageActor, vtkImageActor );
  vtkGetObjectMacro( WindowLevel, vtkImageMapToWindowLevelColors );
  vtkGetObjectMacro( InteractorStyle, vtkInteractorStyleImage );
  vtkGetObjectMacro( Annotation, vtkCornerAnnotation );
  virtual void SetRenderWindow( vtkRenderWindow* );
  virtual void SetRenderer( vtkRenderer* );
  virtual void SetInteractor( vtkRenderWindowInteractor* );
  //@}

  //@{
  /**
   * Configure the vtkImageMapToWindowLevelColors ivar.
   * Configuration is set automatically according to the number
   * of scalar components per pixel/voxel.
   * @see SetInput()
   */
  void SetMappingToLuminance();
  void SetMappingToColorAlpha();
  void SetMappingToColor();
  //@}
  
  /**
   * Get window.
   * @sa Level, SetColorWindowLevel()
   * @return current window
   */
  double GetColorWindow() { return this->Window; }

  /**
   * Get level.
   * @sa Window, SetColorWindowLevel() 
   * @return current level
   */
  double GetColorLevel() { return this->Level; }

  /**
   * Get window and level.
   * @sa  Window, Level, GetColorWindow(), GetColorLevel()
   * @return current window and level by reference
   */
  void SetColorWindowLevel( const double& , const double& );

  //@{
  /**
   * Set the window level of the next image to be the same as the last one.
   * Default is off.
   */
  vtkSetMacro( MaintainLastWindowLevel, int );
  vtkGetMacro( MaintainLastWindowLevel, int );
  vtkBooleanMacro( MaintainLastWindowLevel, int );
  //@}

  //@{
  /** Window level control for callbacks and VTK events. */
  void DoStartWindowLevel();
  void DoResetWindowLevel();
  void DoWindowLevel();
  //@}

  /**
   * Get the dimensionality of the image (e.g., 2D, 3D).
   * @return dimension of the image
   */
  int GetImageDimensionality();

  /** Create a default 3D sinusoidal image. */
  void SetImageToSinusoid();

  /**
   * Scroll once through slices.
   * Scrolling starts at the current slice and proceeds to the last slice.
   * At start, the state ivar is set to PLAY.  At each slice change the
   * PlayEvent is invoked.  Stops either when the state ivar is set to STOP
   * or when the last slice is reached at which point state is set to STOP.
   */
  void CinePlay();

  /**
   * Scroll continuously through slices.
   * Scrolling starts at the first slice, proceeds to the last slice and
   * then repeats.  State ivar is set to PLAY, invokes PlayEvent at each
   * slice change, until the state is set to STOP.
   */
  void CineLoop();

  /** Set the current slice to the first slice. */
  void CineRewind();

  /**
   * Stop Scrolling through slices.
   * State ivar is set to STOP and a StopEvent is invoked.
   */
  void CineStop();

  /** State enum for control of slice scrolling. */
  enum
  {
    PLAY,
    STOP
  };

  //@{
  /** Get event id tags for control of scrolling through 3D images. */
  vtkGetMacro( PlayEvent, int );
  vtkGetMacro( StopEvent, int );
  //@}

  /**
   * Turn cursoring on or off.  Cursoring works in concert with
   * corner annotation.  If cursoring is off, the cursor widget
   * is disabled but the vtkCornerAnnotation object could still be
   * used to display other textual overlay content.  If annotation
   * is off, cursoring can still be active with its output directed
   * via callback mechanism to another GUI element.
   */
  void SetCursor( int );
  vtkBooleanMacro(Cursor,int);
  vtkGetMacro(Cursor, int );

  /**
   * Turn annotation on or off in the render window.
   */
  void SetAnnotate( int );
  vtkBooleanMacro(Annotate,int);
  vtkGetMacro(Annotate, int );

  /**
   * Turns interpolation on or off for both the cursor widget
   * and the image actor simultaneously.  Off state sets the
   * cursor widget to use discrete cursoring mode: the cursor 
   * snaps to pixel centers and reads off discrete pixel values,
   * and the image actor show pixels with nearest neighbor interpolation.
   * On state sets the cursor widget to use continous cursoring
   * mode using interpolation of pixel values and the image actor
   * shows pixels with linear interpolation.
   */
  void SetInterpolate( int );
  vtkBooleanMacro(Interpolate,int);
  vtkGetMacro(Interpolate, int );

protected:
  vtkMedicalImageViewer();
  ~vtkMedicalImageViewer();

  //@{
  /**
   * Internal methods to build or dismantle the display pipeline.
   * WARNING: do not use without carefully considering side effects!
   */
  virtual void InstallPipeline();
  virtual void UnInstallPipeline();
  void InstallCursor();
  void UnInstallCursor();
  void InstallAnnotation();
  void UnInstallAnnotation();
  //@}

  //@{
  /** VTK object ivars that constitute the visualization/interaction pipeline. */
  vtkImageMapToWindowLevelColors  *WindowLevel;
  vtkRenderWindow                 *RenderWindow;
  vtkRenderer                     *Renderer;
  vtkImageActor                   *ImageActor;
  vtkRenderWindowInteractor       *Interactor;
  vtkInteractorStyleImage         *InteractorStyle;
  vtkImageCoordinateWidget        *CursorWidget;
  vtkCornerAnnotation             *Annotation;
  //@}

  int Cursor;
  int Annotate;
  int Interpolate;

  int Slice;        /**< Current slice index */
  int LastSlice[3]; /**< Keeps track of last slice when changing orientation */

  int PlayEvent;    /**< VTK unique event id for the PlayEvent */
  int StopEvent;    /**< VTK unique event id for the StopEvent */
  int CineState;    /**< Current state of play or stop for cine methods */

  /** Maintain window level settings between image changes */
  int MaintainLastWindowLevel; 
  double OriginalWindow; /**< Original window computed from input */
  double OriginalLevel;  /**< Original level computed from input */
  double InitialWindow;  /**< Initial window at start of interaction */   
  double InitialLevel;   /**< Initial level at start of interaction */
  double Window;         /**< Current window */
  double Level;          /**< Current level */

  /** 
   * Callback ids for install and uninstall of callbacks to the interactor.
   * Callback tags are set in InstallPipeline() and used for callback
   * removal in UnInstallPipeline()
   */
  std::vector<unsigned long> WindowLevelCallbackTags;

  /** Calculate the original window and level parameters
    * @sa OriginalWindow, OriginalLevel
    */
  void InitializeWindowLevel();

  int ViewOrientation;              /**< Current view orientation: XY, XZ, YZ */
  double CameraPosition[3][3];      /**< Current camera position */
  double CameraFocalPoint[3][3];    /**< Current camera focal point */
  double CameraViewUp[3][3];        /**< Current camera view up */
  double CameraParallelScale[3];    /**< Current camera parallel scale (zoom) */

  /** Set up the default camera parameters based on input data dimensions etc. */
  void InitializeCameraViews();
  /** Record the current camera parameters */
  void RecordCameraView();

private:
  vtkMedicalImageViewer(const vtkMedicalImageViewer&);  /** Not implemented. */
  void operator=(const vtkMedicalImageViewer&);  /** Not implemented. */
};

#endif
