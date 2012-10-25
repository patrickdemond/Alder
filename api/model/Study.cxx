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
#include "vtkSmartPointer.h"

#include <map>

namespace Alder
{
  vtkStandardNewMacro( Study );

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Study::UpdateData()
  {
    OpalService *opal = Application::GetInstance()->GetOpal();
    // count the number of identifiers
    std::vector< std::string > identifierList = opal->GetIdentifiers( "clsa-dcs-images", "CarotidIntima" );
    std::map< std::string, std::string > userList;
    std::map< std::string, std::string > datetimeList;
    std::map< std::string, std::string > map;

    int limit = 100;
    int index = 0;
    while( index < identifierList.size() )
    {
      // get users
      map = opal->GetValueList( "clsa-dcs", "CarotidIntima", "InstrumentRun.user", index, limit );
      userList.insert( map.begin(), map.end() );

      // get datetimes
      map = opal->GetValueList( "clsa-dcs", "CarotidIntima", "InstrumentRun.timeStart", index, limit );
      datetimeList.insert( map.begin(), map.end() );

      index += map.size();
    }

    // now create the records
    std::vector< std::string >::iterator identifier;
    std::map< std::string, std::string >::iterator value;
    for( identifier = identifierList.begin(); identifier != identifierList.end(); ++identifier )
    {
      vtkSmartPointer< Study > study = vtkSmartPointer< Study >::New();
      study->Load( "uid", *identifier ); // may not result in loading a record
      study->Set( "uid", *identifier );
      study->Set( "site", "unknown" ); // TODO: get from Mastodon
      value = userList.find( *identifier );
      study->Set( "interviewer", userList.end() != value ? value->second : "unknown" );
      value = datetimeList.find( *identifier );
      study->Set( "datetime_acquired", datetimeList.end() != value ? value->second : "unknown" );
      study->Save();
    }
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::vector< std::string > Study::GetIdentifierList()
  {
    return Application::GetInstance()->GetOpal()->GetIdentifiers( "clsa-dcs-images", "CarotidIntima" );
  }
}
