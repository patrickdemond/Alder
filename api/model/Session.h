/*=========================================================================

  Program:  Alder (CLSA Ultrasound Image Viewer)
  Module:   Session.h
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/

/**
 * @class Session
 * @namespace Alder
 * 
 * @author Patrick Emond <emondpd@mcmaster.ca>
 * @author Dean Inglis <inglisd@mcmaster.ca>
 * 
 * @brief A class containing all session information
 * 
 * TODO: document if class gets used, otherwise remove from project
 */

#ifndef __Session_h
#define __Session_h

#include "vtkDataObject.h"

#include "Utilities.h"

class vtkCamera;

/**
 * @addtogroup Alder
 * @{
 */

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

/** @} end of doxygen group */

#endif
