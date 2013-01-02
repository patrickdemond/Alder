/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   Study.cxx
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/
#include "Study.h"

#include "Application.h"
#include "Exam.h"
#include "Image.h"
//#include "OpalService.h"
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
      study->Load( "UId", *identifier ); // may not result in loading a record
      study->Set( "UId", *identifier );
      study->Set( "Site", "unknown" ); // TODO: get from Mastodon
      value = userList.find( *identifier );
      study->Set( "Interviewer", userList.end() != value ? value->second : "unknown" );
      value = datetimeList.find( *identifier );
      study->Set( "DatetimeAcquired", datetimeList.end() != value ? value->second : "unknown" );
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
    std::string currentUid = this->Get( "UId" ).ToString();
    std::vector< std::string > list = Study::GetUIdList();
    std::vector< std::string >::reverse_iterator it;

    // the list should never be empty (since we are already an instance of study)
    if( list.empty() ) throw std::runtime_error( "Study list is empty while trying to get next study." );
    
    // find this record's UId in the list, return the next one
    std::string UId;
    for( it = list.rbegin(); it != list.rend(); it++ )
    {
      if( currentUid == *it )
      {
        if( list.rbegin() == it )
        { // first UId matches, get the last UId
          UId = list.front();
        }
        else
        { // move the iterator to the previous address, get it's value
          it--;
          UId = *it;
        }
        break;
      }
    }

    if( UId.empty() ) throw std::runtime_error( "Study list does not include current UId." );

    vtkSmartPointer<Study> study = vtkSmartPointer<Study>::New();
    study->Load( "UId", UId );
    return study;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Study::Next()
  {
    this->Load( "Id", this->GetNext()->Get( "Id" ).ToString() );
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  vtkSmartPointer<Study> Study::GetPrevious()
  {
    std::string currentUid = this->Get( "UId" ).ToString();
    std::vector< std::string > list = Study::GetUIdList();
    std::vector< std::string >::iterator it;

    // the list should never be empty (since we are already an instance of study)
    if( list.empty() ) throw std::runtime_error( "Study list is empty while trying to get next study." );
    
    // find this record's UId in the list, return the next one
    std::string UId;
    for( it = list.begin(); it != list.end(); it++ )
    {
      if( currentUid == *it )
      {
        if( list.begin() == it )
        { // first UId matches, get the last UId
          UId = list.back();
        }
        else
        { // move the iterator to the previous address, get it's value
          it--;
          UId = *it;
        }
        break;
      }
    }

    if( UId.empty() ) throw std::runtime_error( "Study list does not include current UId." );

    vtkSmartPointer<Study> study = vtkSmartPointer<Study>::New();
    study->Load( "UId", UId );
    return study;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Study::Previous()
  {
    this->Load( "Id", this->GetPrevious()->Get( "Id" ).ToString() );
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::vector< std::string > Study::GetUIdList()
  {
    Application *app = Application::GetInstance();
    vtkSmartPointer<vtkAlderMySQLQuery> query = app->GetDB()->GetQuery();
    query->SetQuery( "SELECT UId FROM Study ORDER BY UId" );
    query->Execute();

    std::vector< std::string > list;
    while( query->NextRow() ) list.push_back( query->DataValue( 0 ).ToString() );
    return list;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  int Study::GetImageCount()
  {
    // loop through all exams and sum the image count
    int total = 0;

    // loop through all exams
    std::vector< vtkSmartPointer< Exam > > examList;
    std::vector< vtkSmartPointer< Exam > >::iterator examIt;
    this->GetList( &examList );
    for( examIt = examList.begin(); examIt != examList.end(); ++examIt )
    {
      Exam *exam = *(examIt);
      total += exam->GetCount( "Image" );
    }

    return total;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool Study::IsRatedBy( User* user )
  {
    this->AssertPrimaryId();

    // make sure the user is not null
    if( !user ) throw std::runtime_error( "Tried to get rating for null user" );

    // loop through all exams
    std::vector< vtkSmartPointer< Exam > > examList;
    std::vector< vtkSmartPointer< Exam > >::iterator examIt;
    this->GetList( &examList );
    for( examIt = examList.begin(); examIt != examList.end(); ++examIt )
    {
      Exam *exam = *(examIt);
      // loop through all images
      std::vector< vtkSmartPointer< Image > > imageList;
      std::vector< vtkSmartPointer< Image > >::iterator imageIt;
      exam->GetList( &imageList );
      for( imageIt = imageList.begin(); imageIt != imageList.end(); ++imageIt )
      {
        Image *image = *(imageIt);
        if( !image->IsRatedBy( user ) ) return false;
      }
    }

    return true;
  }
}
