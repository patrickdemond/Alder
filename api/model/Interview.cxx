/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   Interview.cxx
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/
#include "Interview.h"

#include "Study.h"
#include "User.h"
#include "Utilities.h"

#include "vtkObjectFactory.h"

namespace Alder
{
  vtkStandardNewMacro( Interview );

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  vtkSmartPointer<Interview> Interview::GetNext()
  {
    std::string currentUId = this->Get( "UId" ).ToString();
    std::string currentVisitDate = this->Get( "VisitDate" ).ToString();
    std::vector< std::pair< std::string, std::string > > list = Interview::GetUIdVisitDateList();
    std::vector< std::pair< std::string, std::string > >::reverse_iterator pair;

    // the list should never be empty (since we are already an instance of interview)
    if( list.empty() )
      throw std::runtime_error( "Interview list is empty while trying to get next interview." );
    
    // find this record's UId in the list, return the next one
    for( pair = list.rbegin(); pair != list.rend(); pair++ )
    {
      if( currentUId == pair->first && currentVisitDate == pair->second )
      {
        if( list.rbegin() == pair )
        { // first UId matches, get the last pair
          pair = list.rbegin();
        }
        else
        { // move the iterator to the previous address
          pair--;
        }
        break;
      }
    }

    if( list.rend() == pair )
      throw std::runtime_error( "Interview list does not include current UId/VisitDate pair." );

    std::map< std::string, std::string > map;
    map["UId"] = pair->first;
    map["VisitDate"] = pair->second;
    vtkSmartPointer<Interview> interview = vtkSmartPointer<Interview>::New();
    interview->Load( map );
    return interview;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Interview::Next()
  {
    this->Load( "Id", this->GetNext()->Get( "Id" ).ToString() );
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  vtkSmartPointer<Interview> Interview::GetPrevious()
  {
    std::string currentUId = this->Get( "UId" ).ToString();
    std::string currentVisitDate = this->Get( "VisitDate" ).ToString();
    std::vector< std::pair< std::string, std::string > > list = Interview::GetUIdVisitDateList();
    std::vector< std::pair< std::string, std::string > >::iterator pair;

    // the list should never be empty (since we are already an instance of interview)
    if( list.empty() )
      throw std::runtime_error( "Interview list is empty while trying to get next interview." );
    
    // find this record's UId in the list, return the next one
    std::string UId;
    for( pair = list.begin(); pair != list.end(); pair++ )
    {
      if( currentUId == pair->first && currentVisitDate == pair->second )
      {
        if( list.begin() == pair )
        { // first UId matches, get the last UId
          pair = list.end();
        }
        else
        { // move the iterator to the previous address, get its value
          pair--;
        }
        break;
      }
    }

    if( list.end() == pair )
      throw std::runtime_error( "Interview list does not include current UId/VisitDate pair." );

    std::map< std::string, std::string > map;
    map["UId"] = pair->first;
    map["VisitDate"] = pair->second;
    vtkSmartPointer<Interview> interview = vtkSmartPointer<Interview>::New();
    interview->Load( map );
    return interview;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Interview::Previous()
  {
    this->Load( "Id", this->GetPrevious()->Get( "Id" ).ToString() );
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::vector< std::pair< std::string, std::string > > Interview::GetUIdVisitDateList()
  {
    Application *app = Application::GetInstance();
    vtkSmartPointer<vtkAlderMySQLQuery> query = app->GetDB()->GetQuery();
    query->SetQuery( "SELECT UId, VisitDate FROM Interview ORDER BY UId, VisitDate" );
    query->Execute();

    std::vector< std::pair< std::string, std::string > > list;
    while( query->NextRow() )
      list.push_back( std::pair< std::string, std::string >(
        query->DataValue( 0 ).ToString(), query->DataValue( 1 ).ToString() ) );
    return list;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  int Interview::GetImageCount()
  {
    // loop through all studies and sum the image count
    int total = 0;

    std::vector< vtkSmartPointer< Study > > studyList;
    std::vector< vtkSmartPointer< Study > >::iterator studyIt;
    this->GetList( &studyList );
    for( studyIt = studyList.begin(); studyIt != studyList.end(); ++studyIt )
    {
      Study *study = *(studyIt);
      total += study->GetImageCount();
    }

    return total;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool Interview::IsRatedBy( User* user )
  {
    this->AssertPrimaryId();

    // make sure the user is not null
    if( !user ) throw std::runtime_error( "Tried to get rating for null user" );

    std::vector< vtkSmartPointer< Study > > studyList;
    std::vector< vtkSmartPointer< Study > >::iterator studyIt;
    this->GetList( &studyList );
    for( studyIt = studyList.begin(); studyIt != studyList.end(); ++studyIt )
    {
      Study *study = *(studyIt);
      if( !study->IsRatedBy( user ) ) return false;
    }

    return true;
  }
}
