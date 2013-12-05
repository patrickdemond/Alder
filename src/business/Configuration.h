/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   Configuration.h
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/

/**
 * @class Configuration
 * @namespace Alder
 * 
 * @author Patrick Emond <emondpd AT mcmaster DOT ca>
 * @author Dean Inglis <inglisd AT mcmaster DOT ca>
 * 
 * @brief Class which contains all configuration values
 * 
 * This is an object representation of the config.xml document.  It contains
 * information such as the database and Opal connection parameters.  A single
 * instance of this class is created and managed by the Application singleton.
 */

#ifndef __Configuration_h
#define __Configuration_h

#include <ModelObject.h>

#include <vtkSmartPointer.h>

#include <iostream>
#include <map>

class vtkXMLConfigurationFileReader;

/**
 * @addtogroup Alder
 * @{
 */

namespace Alder
{
  class Configuration : public ModelObject
  {
  public:
    static Configuration *New();
    vtkTypeMacro( Configuration, ModelObject );

    /**
     * Gets a value from the configuration given a category and key
     * @param category string
     * @param key string
     */
    std::string GetValue( const std::string category, const std::string key ) const;

    /**
     * Reads a configuration file given a filename
     * @filename string
     */
    bool Read( const std::string filename );

  protected:
    Configuration();
    ~Configuration() {}
    std::map< std::string, std::map< std::string, std::string > > Settings;
    vtkSmartPointer<vtkXMLConfigurationFileReader> Reader;
  
  private:
    Configuration( const Configuration& ); // Not implemented
    void operator=( const Configuration& ); // Not implemented
  };
}

/** @} end of doxygen group */

#endif
