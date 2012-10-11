/*=========================================================================

  Program:  Alder (CLSA Ultrasound Image Viewer)
  Module:   Configuration.h
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/

#ifndef __Configuration_h
#define __Configuration_h

#include "ModelObject.h"

#include "vtkSmartPointer.h"

#include <iostream>
#include <map>

class vtkXMLConfigurationFileReader;

namespace Alder
{
//  class Session;
  class Configuration : public ModelObject
  {
  public:
    static Configuration *New();
    vtkTypeMacro( Configuration, ModelObject );

    std::string GetValue( std::string category, std::string key );
    bool Read( std::string filename );

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

#endif
