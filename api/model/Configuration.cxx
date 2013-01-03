/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   Configuration.cxx
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/

#include "Configuration.h"

#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"
#include "vtkXMLConfigurationFileReader.h"

namespace Alder
{
  vtkStandardNewMacro( Configuration );

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  Configuration::Configuration()
  {
    this->Reader = vtkSmartPointer<vtkXMLConfigurationFileReader>::New();
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool Configuration::Read( std::string fileName )
  {
    this->Reader->SetFileName( fileName );
    try
    {
      this->Reader->Update();
      this->Settings = this->Reader->GetSettings();
    }
    catch( std::exception &e )
    {
      return false;
    }

    return true;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::string Configuration::GetValue( std::string category, std::string key )
  {
    return this->Settings.find( category ) != this->Settings.end() &&
           this->Settings[category].find( key ) != this->Settings[category].end() ?
           this->Settings[category][key] : "";
  }
}
