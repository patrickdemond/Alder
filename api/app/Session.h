/*=========================================================================

  Program:  Alder (CLSA Ultrasound Image Viewer)
  Module:   Session.h
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>

=========================================================================*/
//
// .NAME Session - Session information object
//
// .SECTION Description
//
// .SECTION See Also
// SessionReader SessionWriter
// 

#ifndef __Session_h
#define __Session_h

#include "vtkDataObject.h"

#include "Utilities.h"

class vtkCamera;

namespace Alder
{
  class Session : public vtkDataObject
  {
  public:
    static Session *New();
    vtkTypeMacro( Session, vtkDataObject );
    void PrintSelf( ostream& os, vtkIndent indent );
    
    bool operator == ( const Session& ) const;
    bool operator != ( const Session &rhs ) const { return !( *this == rhs ); }
    
    virtual void DeepCopy( Session* );

    void Save( const char* fileName );
    void Load( const char* fileName );

    // Description:
    // Set/get the camera
    virtual vtkCamera* GetCamera() { return this->Camera; }
    virtual void SetCamera( vtkCamera* );
    
  protected:
    Session();
    ~Session();
    
    vtkCamera *Camera;

  private:
    Session( const Session& );  // Not implemented.
    void operator=( const Session& );  // Not implemented.
  };
}

#endif
