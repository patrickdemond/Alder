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

class vtkMedicalImageViewer;

namespace Alder
{
  class Configuration;
  class Database;
  class OpalService;
  class Study;
  class User;
  class Application : public ModelObject
  {
  public:
    vtkTypeMacro( Application, ModelObject );
    static Application *GetInstance();
    static void DeleteInstance();
    
    bool ReadConfiguration( std::string filename );
    bool ConnectToDatabase();
    void SetupOpalService();
    void ResetApplication();

    vtkGetObjectMacro( Viewer, vtkMedicalImageViewer );
    vtkGetObjectMacro( Config, Configuration );
    vtkGetObjectMacro( DB, Database );
    vtkGetObjectMacro( Opal, OpalService );
    vtkGetObjectMacro( ActiveUser, User );
    virtual void SetActiveUser( User* );
    vtkGetObjectMacro( ActiveStudy, Study );
    virtual void SetActiveStudy( Study* );

    ModelObject* Create( std::string className )
    { return Application::ConstructorRegistry[className](); }
    std::string GetUnmangledClassName( std::string mangledName );
    
  protected:
    Application();
    ~Application();

    static Application *New();
    static Application *Instance;

    vtkMedicalImageViewer *Viewer;
    Configuration *Config;
    Database *DB;
    OpalService *Opal;
    User *ActiveUser;
    Study *ActiveStudy;
    
  private:
    Application( const Application& );  // Not implemented.
    void operator=( const Application& );  // Not implemented.

    std::map< std::string, ModelObject*(*)() > ConstructorRegistry;
    std::map< std::string, std::string > ClassNameRegistry;
  };

  template <class T> ModelObject* createInstance() { return T::New(); }
}

#endif
