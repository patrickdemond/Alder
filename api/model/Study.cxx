/*=========================================================================

  Program:  Alder (CLSA Ultrasound Image Viewer)
  Module:   Study.cxx
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/
#include "Study.h"

#include "Utilities.h"

#include "vtkObjectFactory.h"

namespace Alder
{
  vtkStandardNewMacro( Study );

  std::vector< std::string > Study::GetIdentifierList()
  {
    // TODO: implement
    std::vector< std::string > list;
    list.push_back( "A123123" );
    list.push_back( "B234234" );
    list.push_back( "C123123" );
    list.push_back( "D234234" );
    list.push_back( "E123123" );
    list.push_back( "F234234" );
    list.push_back( "G123123" );
    return list;
  }
}
