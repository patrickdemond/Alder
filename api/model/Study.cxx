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

#include "vtkCommand.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"

#include <map>

namespace Alder
{
  vtkStandardNewMacro( Study );

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Study::UpdateData()
  {
    Application *app = Application::GetInstance();
    OpalService *opal = app->GetOpal();

    double progress = 0.0;
    app->InvokeEvent( vtkCommand::ProgressEvent, static_cast<void *>( &progress ) );

    // count the number of identifiers
    std::vector< std::string > identifierList = opal->GetIdentifiers( "clsa-dcs-images", "CarotidIntima" );
    std::map< std::string, std::string > userList;
    std::map< std::string, std::string > datetimeList;
    std::map< std::string, std::string > map;

    bool done = false;
    int limit = 100;
    int index = 0;
    
    while( userList.size() < identifierList.size() )
    {
      // get users
      map = opal->GetValueList( "clsa-dcs", "CarotidIntima", "InstrumentRun.user", index, limit );
      userList.insert( map.begin(), map.end() );

      // get datetimes
      map = opal->GetValueList( "clsa-dcs", "CarotidIntima", "InstrumentRun.timeStart", index, limit );
      datetimeList.insert( map.begin(), map.end() );
      if( 0 == map.size() ) done = true;

      index += map.size();

      progress = (double)userList.size() / identifierList.size() * 0.5;
      app->InvokeEvent( vtkCommand::ProgressEvent, static_cast<void *>( &progress ) );
    }

    // now create the records
    index = 0;
    std::vector< std::string >::iterator identifier;
    std::map< std::string, std::string >::iterator value;
    
    progress = 0.5;
    app->InvokeEvent( vtkCommand::ProgressEvent, static_cast<void *>( &progress ) );

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
      index++;

      if( 0 == index % 100 )
      {
        progress = (double)index / identifierList.size() * 0.5 + 0.5;
        app->InvokeEvent( vtkCommand::ProgressEvent, static_cast<void *>( &progress ) );
      }
    }

    progress = 1.0;
    app->InvokeEvent( vtkCommand::ProgressEvent, static_cast<void *>( &progress ) );
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::vector< std::string > Study::GetIdentifierList()
  {
    return Application::GetInstance()->GetOpal()->GetIdentifiers( "clsa-dcs-images", "CarotidIntima" );
  }
}
