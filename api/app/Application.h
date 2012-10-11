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

#include "ModelObject.h"

#include "Utilities.h"

#include <iostream>

class vtkView;

namespace Alder
{
  class Configuration;
  class Database;
  class Application : public ModelObject
  {
  public:
    vtkTypeMacro( Application, ModelObject );
    static Application *GetInstance();
    static void DeleteInstance();
    
    bool ReadConfiguration( std::string filename );
    bool ConnectToDatabase();

    bool HasAdministrator();
    bool IsAdministratorPassword( std::string );
    void SetAdministratorPassword( std::string );

    vtkGetObjectMacro( View, vtkView );
    vtkGetObjectMacro( Config, Configuration );
    vtkGetObjectMacro( DB, Database );
    
  protected:
    Application();
    ~Application();

    static Application *New();
    static Application *Instance;

    vtkView *View;
    Configuration *Config;
    Database *DB;
    
  private:
    Application( const Application& );  // Not implemented.
    void operator=( const Application& );  // Not implemented.
  };
}

#endif
