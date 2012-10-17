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

    vtkGetObjectMacro( View, vtkView );
    vtkGetObjectMacro( Config, Configuration );
    vtkGetObjectMacro( DB, Database );

    ModelObject* Create( std::string className )
    { return Application::Factory[className](); }
    
  protected:
    Application();
    ~Application();

    static Application *New();
    static Application *Instance;

    std::map< std::string, ModelObject*(*)() > Factory;
    vtkView *View;
    Configuration *Config;
    Database *DB;
    
  private:
    Application( const Application& );  // Not implemented.
    void operator=( const Application& );  // Not implemented.
  };

  template <class T> ModelObject* createInstance() { return T::New(); }
}

#endif
