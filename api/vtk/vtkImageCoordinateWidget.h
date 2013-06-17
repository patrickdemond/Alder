/*=========================================================================

  Module:    vtkImageCoordinateWidget.h
  Program:   Alder (CLSA Medical Image Quality Assessment Tool)
  Language:  C++
  Author:    Patrick Emond <emondpd@mcmaster.ca>
  Author:    Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/

/**
 * @class vtkImageCoordinateWidget 

 * @author Patrick Emond <emondpd@mcmaster.ca>
 * @author Dean Inglis <inglisd@mcmaster.ca>
 *
 * @brief 3D widget for probing image data.
 *
 * Choose between voxel centered or continuous cursor probing by listening
 * to mouse move events.  With voxel centered probing, the nearest voxel and
 * reported coordinates are extent based.  With continuous probing, voxel data
 * is interpolated using vtkDataSetAttributes' InterpolatePoint method and
 * the reported coordinates are 3D spatially continuous.
 *
 * @see vtk3DWidget vtkBoxWidget vtkLineWidget  vtkPlaneWidget vtkPointWidget
 */

#ifndef __vtkImageCoordinateWidget_h
#define __vtkImageCoordinateWidget_h

#include <vtkInteractorObserver.h>
#include <vtkSmartPointer.h>

#include <vector>
#include <string>

class vtkAbstractPropPicker;
class vtkDataSet;
class vtkHomogeneousTransform;
class vtkImageData;
class vtkProp;
class vtkPropCollection;

class vtkImageCoordinateWidget : public vtkInteractorObserver
{
public:
  static vtkImageCoordinateWidget *New(); 
  vtkTypeMacro( vtkImageCoordinateWidget,vtkInteractorObserver );
  void PrintSelf( ostream& os, vtkIndent indent );

  /** Methods that satisfy the superclass' API. */
  virtual void SetEnabled( int );

  //@{
  /** Add, remove or query props to a list. */
  virtual void AddViewProp( vtkProp* );
  virtual void RemoveProp( vtkProp* );
  virtual void RemoveAllProps();
  virtual void RemoveProps( vtkPropCollection* );
  int          HasProp( vtkProp* );
  int          GetNumberOfProps();
  vtkProp*     GetNthProp( int );
  //@}

  /**
   * Set the vtkImageData* input for probing.  Usually, since vtkImageActor
   * takes vtkImageData scaled and shifted vtkImageData to unsigned char or
   * unsigned short, we want to probe the original data which could be
   * float, short, int etc.
   */
  void SetInput( vtkDataSet* input );

  //@{
  /** Get the current image coordinate position. */
  int GetCursorPosition( double& x, double& y, double& z );
  int GetCursorPosition( double xyz[3] )
  { return this->GetCursorPosition( xyz[0], xyz[1], xyz[2] ); }
  //@} 

  //@{
  /**
   * Get the image voxel value at the current coordinate position.
   * Invalid values will be set to VTK_DOUBLE_MAX.
   */
  int GetCursorData1( double& v1 );
  int GetCursorData2( double& v1, double& v2 );
  int GetCursorData3( double& v1, double& v2, double& v3 );
  int GetCursorData4( double& v1, double& v2, double& v3, double& v4 );
  int GetCursorData9( double& v1, double& v2, double& v3, double& v4,
                      double& v5, double& v6, double& v7, double& v8, double& v9 );
  int GetCursorData2( double v[2] )
  { return this->GetCursorData2( v[0], v[1] ); }
  int GetCursorData3( double v[3] )
  { return this->GetCursorData3( v[0], v[1], v[2] ); }
  int GetCursorData4( double v[4] )
  { return this->GetCursorData4( v[0], v[1], v[2], v[3] ); }
  int GetCursorData9( double v[9] )
  { return this->GetCursorData9( v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7], v[8] ); }
  //@}

  /** Cursoring modes. */
  enum
  {
    Discrete = 0,  /**< enum value Discrete */
    Continuous     /**< enum value Continuous */
  };

  //@{
  /**
   * Set/get the interaction and readout functionality of the widget
   *  mode 0: widget cursors for all mouse move events without being started
   *          by a left or right button down/up pair and readout is interpolated
   *  mode 1: widget cursors for all mouse move events without being started
   *          by a left or right button down/up pair and readout is discrete
   */
  void SetCursoringMode( int );
  vtkGetMacro( CursoringMode, int );
  void SetCursoringModeToContinuous()
  {
    this->SetCursoringMode( vtkImageCoordinateWidget::Continuous );
  };
  void SetCursoringModeToDiscrete()
  {
    this->SetCursoringMode( vtkImageCoordinateWidget::Discrete );
  };
  //@}

  //@{
  /** Set/Get the picker. */
  void SetPicker(vtkAbstractPropPicker*);
  vtkGetObjectMacro(Picker, vtkAbstractPropPicker);
  //@}

  //@{
  /**
   * You can add a transformation for your own use to transform the coordinates
   * that are displayed.
   */
  void SetUserTransform(vtkHomogeneousTransform *transform);
  vtkGetObjectMacro(UserTransform,vtkHomogeneousTransform);
  //@}

  /** Get the readout as a char string. */
  const char* GetMessageString(){ return this->MessageString.c_str(); }

  /** Force the widget to update its message string */
  void UpdateMessageString();

protected:
  vtkImageCoordinateWidget();
  ~vtkImageCoordinateWidget();

  /** Enum to manage the state of the widget. */
  enum WidgetState
  {
    Start=0,
    Cursoring,
    Outside
  };
  int State;

  /** Handles the events. */
  static void ProcessEvents( vtkObject* object,
                            unsigned long event,
                            void* clientdata,
                            void* calldata );

  /** ProcessEvents() dispatches to these methods. */
  virtual void OnMouseMove();

  //@{
  /** Update the cursor depending on which mode the widget is in. */
  void UpdateCursor( int, int );
  void UpdateContinuousCursor( double* );
  void UpdateDiscreteCursor( double* );
  //@}

  /**
   * When using this method make sure the double vector 'argument' has at
   * least 'components' doubles allocated.
   */
  int GetCursorDataN( double *v1, int components );

  // Attributes
  int    CursoringMode;
  double CurrentCursorPosition[3];
  std::vector< double > CurrentImageValue; // empty when invalid

  // Objects

  // The prop(s) we want to pick on
  vtkSmartPointer<vtkPropCollection> PropCollection;
  // The prop's picker
  vtkAbstractPropPicker* Picker;
  // The image pertaining to but not necessarily being owned by the input prop
  vtkImageData*            ImageData;
  vtkHomogeneousTransform* UserTransform;

  std::string MessageString;

private:
  vtkImageCoordinateWidget( const vtkImageCoordinateWidget& );  /** Not implemented */
  void operator=( const vtkImageCoordinateWidget& );  /** Not implemented */
};

#endif
