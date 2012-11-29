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
#include <stdexcept>

namespace Alder
{
  vtkStandardNewMacro( Study );

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  /*
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
  */

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  /*
  std::vector< std::string > Study::GetIdentifierList()
  {
    return Application::GetInstance()->GetOpal()->GetIdentifiers( "clsa-dcs-images", "CarotidIntima" );
  }
  */

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  vtkSmartPointer<Study> Study::GetNext()
  {
    std::string currentUid = this->Get( "uid" )->ToString();
    std::vector< std::string > list = Study::GetUIDList();
    std::vector< std::string >::reverse_iterator it;

    // the list should never be empty (since we are already an instance of study)
    if( list.empty() ) throw std::runtime_error( "Study list is empty while trying to get next study." );
    
    // find this record's uid in the list, return the next one
    std::string uid;
    for( it = list.rbegin(); it != list.rend(); it++ )
    {
      if( currentUid == *it )
      {
        if( list.rbegin() == it )
        { // first uid matches, get the last uid
          uid = list.front();
        }
        else
        { // move the iterator to the previous address, get it's value
          it--;
          uid = *it;
        }
        break;
      }
    }

    if( uid.empty() ) throw std::runtime_error( "Study list does not include current UID." );

    vtkSmartPointer<Study> study = vtkSmartPointer<Study>::New();
    study->Load( "uid", uid );
    return study;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  vtkSmartPointer<Study> Study::GetPrevious()
  {
    std::string currentUid = this->Get( "uid" )->ToString();
    std::vector< std::string > list = Study::GetUIDList();
    std::vector< std::string >::iterator it;

    // the list should never be empty (since we are already an instance of study)
    if( list.empty() ) throw std::runtime_error( "Study list is empty while trying to get next study." );
    
    // find this record's uid in the list, return the next one
    std::string uid;
    for( it = list.begin(); it != list.end(); it++ )
    {
      if( currentUid == *it )
      {
        if( list.begin() == it )
        { // first uid matches, get the last uid
          uid = list.back();
        }
        else
        { // move the iterator to the previous address, get it's value
          it--;
          uid = *it;
        }
        break;
      }
    }

    if( uid.empty() ) throw std::runtime_error( "Study list does not include current UID." );

    vtkSmartPointer<Study> study = vtkSmartPointer<Study>::New();
    study->Load( "uid", uid );
    return study;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::vector< std::string > Study::GetUIDList()
  {
    Application *app = Application::GetInstance();
    vtkSmartPointer<vtkMySQLQuery> query = app->GetDB()->GetQuery();
    query->SetQuery( "SELECT uid FROM Study ORDER BY uid" );
    query->Execute();

    std::vector< std::string > list;
    while( query->NextRow() ) list.push_back( query->DataValue( 0 ).ToString() );
    return list;
  }
}
