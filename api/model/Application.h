/*=========================================================================

  Program:  Alder (CLSA Ultrasound Image Viewer)
  Module:   Application.h
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/

/**
 * @class Application
 * @namespace Alder
 * 
 * @author Patrick Emond <emondpd@mcmaster.ca>
 * @author Dean Inglis <inglisd@mcmaster.ca>
 * 
 * @brief Central class used to share information throughout the application
 * 
 * This class is a singleton which is meant to be used anywhere throughout
 * the application as a means of accessing global application information.
 * It includes links to the image viewer, configuration, database, connection
 * to Opal and tracks the state of the application such as active user and
 * study.
 */

#ifndef __Application_h
#define __Application_h

#include "ModelObject.h"

#include "Utilities.h"

#include <iostream>
#include <stdexcept>

class vtkMedicalImageViewer;

/**
 * @addtogroup Alder
 * @{
 */

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
    
    /**
     * Reads configuration variables from a given file
     * @param filename string The file to read the configuration from
     */
    bool ReadConfiguration( std::string filename );
    
    /**
     * Uses database values in the configuration to connect to a database
     */
    bool ConnectToDatabase();
    
    /**
     * Uses opal values in the configuration to set up a connection to Opal
     */
    void SetupOpalService();
    
    /**
     * Resets the state of the application to its initial state
     */
    void ResetApplication();

    vtkGetObjectMacro( Viewer, vtkMedicalImageViewer );
    vtkGetObjectMacro( Config, Configuration );
    vtkGetObjectMacro( DB, Database );
    vtkGetObjectMacro( Opal, OpalService );
    vtkGetObjectMacro( ActiveUser, User );
    virtual void SetActiveUser( User* );
    vtkGetObjectMacro( ActiveStudy, Study );
    virtual void SetActiveStudy( Study* );
    
    /**
     * Creates a new instance of a model object given its class name
     * @param className string
     * @throws runtime_error
     */
    ModelObject* Create( std::string className )
    {
      // make sure the constructor registry has the class name being asked for
      std::map< std::string, ModelObject*(*)() >::iterator pair;
      pair = this->ConstructorRegistry.find( className );
      if( pair == this->ConstructorRegistry.end() )
      {
        std::stringstream stream;
        stream << "Tried to create object of type \""
               << className << "\" which doesn't exist in the constructor registry";
        throw std::runtime_error( stream.str() );
      }
      return pair->second();
    }
    
    /**
     * Compilers mangle class names at compile time.  This method provides the
     * unmangled name (without namespace).  In order for this to work all classes
     * must be registered in this class' constructor.
     * @throws runtime_error
     */
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

/** @} end of doxygen group */

#endif
