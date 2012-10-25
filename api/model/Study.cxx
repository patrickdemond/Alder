/*=========================================================================

  Program:  Alder (CLSA Ultrasound Image Viewer)
  Module:   Study.cxx
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/
#include "Study.h"

#include "Application.h"
#include "OpalService.h"
#include "Utilities.h"

#include "vtkObjectFactory.h"

namespace Alder
{
  vtkStandardNewMacro( Study );

  std::vector< std::string > Study::GetIdentifierList()
  {
    return Application::GetInstance()->GetOpal()->GetIdentifiers( "clsa-dcs-images", "CarotidIntima" );
  }
}
