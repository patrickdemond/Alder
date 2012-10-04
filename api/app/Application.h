/*=========================================================================

  Program:  Alder (CLSA Ultrasound Image Viewer)
  Module:   Application.h
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/
//
// .NAME Application - Application or "central" object
//
// .SECTION Description
//
// .SECTION See Also
// ApplicationReader ApplicationWriter
// 

#ifndef __Application_h
#define __Application_h

#include "vtkObject.h"

#include "Utilities.h"

class vtkAlderDatabase;
class vtkView;

namespace Alder
{
//  class Session;
  class Application : public vtkObject
  {
  public:
    static Application *GetInstance();
    static void DeleteInstance();

    vtkTypeMacro( Application, vtkObject );
    void PrintSelf( ostream& os, vtkIndent indent );

//    vtkGetObjectMacro( CurrentSession, Session );
    vtkGetObjectMacro( Database, vtkAlderDatabase );
    vtkGetObjectMacro( View, vtkView );
    
  protected:
    Application();
    ~Application();

    static Application *New();
    static Application *Instance;

//    Session *CurrentSession;
    vtkAlderDatabase *Database;
    vtkView *View;
    
  private:
    Application( const Application& );  // Not implemented.
    void operator=( const Application& );  // Not implemented.
  };
}

#endif
