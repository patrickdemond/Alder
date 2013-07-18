/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   Interview.cxx
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/
#include "Interview.h"

#include "Application.h"
#include "Exam.h"
#include "OpalService.h"
#include "User.h"
#include "Utilities.h"

#include "vtkCommand.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"

#include <map>
#include <stdexcept>

namespace Alder
{
  vtkStandardNewMacro( Interview );

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  vtkSmartPointer<Interview> Interview::GetNeighbour( bool forward, bool loaded, bool unrated )
  {
    this->AssertPrimaryId();

    Application *app = Application::GetInstance();
    std::string interviewId = this->Get( "Id" ).ToString();
    std::string uId = this->Get( "UId" ).ToString();
    std::string userId = app->GetActiveUser()->Get( "Id" ).ToString();
    std::stringstream stream;

    // use a special query to quickly get the next interview
    if( !loaded && !unrated )
    {
      stream << "SELECT Id FROM Interview ";
    }
    else if( !loaded && unrated )
    {
      stream << "SELECT Id, UId FROM ( "
             <<   "SELECT Interview.Id, UId, Rating.Rating IS NOT NULL AS Rated "
             <<   "FROM User "
             <<   "CROSS JOIN Interview "
             <<   "LEFT JOIN Exam ON Interview.Id = Exam.InterviewId "
             <<   "AND ( ( Exam.Modality = 'Dexa' AND User.RateDexa = true ) OR "
             <<         "( Exam.Modality = 'Retinal' AND User.RateRetinal = true ) OR "
             <<         "( Exam.Modality = 'Ultrasound' AND User.RateUltrasound = true ) ) "
             <<   "LEFT JOIN Image ON Exam.Id = Image.ExamId "
             <<   "LEFT JOIN Rating ON Image.Id = Rating.ImageId "
             <<   "AND User.Id = Rating.UserId "
             <<   "WHERE User.Id = " << userId << " "
             <<   "GROUP BY Interview.Id, Rating.Rating IS NOT NULL "
             << ") AS temp1 "
             << "WHERE Rated = false "
             << "AND Id NOT IN ( "
             <<   "SELECT Id FROM ( "
             <<     "SELECT Interview.Id, Rating.Rating IS NOT NULL AS Rated "
             <<     "FROM User "
             <<     "CROSS JOIN Interview "
             <<     "JOIN Exam ON Interview.Id = Exam.InterviewId "
             <<     "AND ( ( Exam.Modality = 'Dexa' AND User.RateDexa = true ) OR "
             <<           "( Exam.Modality = 'Retinal' AND User.RateRetinal = true ) OR "
             <<           "( Exam.Modality = 'Ultrasound' AND User.RateUltrasound = true ) ) "
             <<     "JOIN Image ON Exam.Id = Image.ExamId "
             <<     "JOIN Rating ON Image.Id = Rating.ImageId "
             <<     "AND User.Id = Rating.UserId "
             <<     "WHERE User.Id = " << userId << " "
             <<     "GROUP BY Interview.Id, Rating.Rating IS NOT NULL "
             <<   ") AS temp2 "
             <<   "WHERE Rated = true "
             << ") "
             << "UNION SELECT " << interviewId << ", '" << uId << "' ";
    }
    else if ( loaded && !unrated )
    {
      stream << "SELECT Id, UId FROM ( "
             <<   "SELECT Interview.Id, UId, Exam.Downloaded "
             <<   "FROM User "
             <<   "CROSS JOIN Interview "
             <<   "JOIN Exam ON Interview.Id = Exam.InterviewId "
             <<   "AND ( ( Exam.Modality = 'Dexa' AND User.RateDexa = true ) OR "
             <<         "( Exam.Modality = 'Retinal' AND User.RateRetinal = true ) OR "
             <<         "( Exam.Modality = 'Ultrasound' AND User.RateUltrasound = true ) ) "
             <<   "WHERE User.Id = " << userId << " "
             <<   "AND Stage = 'Completed' "
             <<   "GROUP BY Interview.Id, Downloaded "
             << ") AS temp1 "
             << "WHERE Downloaded = true "
             << "AND Id NOT IN ( "
             <<   "SELECT Id FROM ( "
             <<     "SELECT Interview.Id, Exam.Downloaded "
             <<     "FROM User "
             <<     "CROSS JOIN Interview "
             <<     "JOIN Exam ON Interview.Id = Exam.InterviewId "
             <<     "AND ( ( Exam.Modality = 'Dexa' AND User.RateDexa = true ) OR "
             <<           "( Exam.Modality = 'Retinal' AND User.RateRetinal = true ) OR "
             <<           "( Exam.Modality = 'Ultrasound' AND User.RateUltrasound = true ) ) "
             <<     "WHERE User.Id = " << userId << " "
             <<     "AND Stage = 'Completed' "
             <<     "GROUP BY Interview.Id, Downloaded "
             <<   ") AS temp2 "
             <<   "WHERE Downloaded = false "
             << ") "
             << "UNION SELECT " << interviewId << ", '" << uId << "' ";
    }
    else // loaded && unrated
    {
      stream << "SELECT Id, UId FROM ( "
             <<   "SELECT Interview.Id, UId, Rating.Rating IS NOT NULL AS Rated, Exam.Downloaded "
             <<   "FROM User "
             <<   "CROSS JOIN Interview "
             <<   "JOIN Exam ON Interview.Id = Exam.InterviewId "
             <<   "AND ( ( Exam.Modality = 'Dexa' AND User.RateDexa = true ) OR "
             <<         "( Exam.Modality = 'Retinal' AND User.RateRetinal = true ) OR "
             <<         "( Exam.Modality = 'Ultrasound' AND User.RateUltrasound = true ) ) "
             <<   "JOIN Image ON Exam.Id = Image.ExamId "
             <<   "LEFT JOIN Rating ON Image.Id = Rating.ImageId "
             <<   "AND User.Id = Rating.UserId "
             <<   "WHERE User.Id = " << userId << " "
             <<   "AND Stage = 'Completed' "
             <<   "GROUP BY Interview.Id, Rating.Rating IS NOT NULL, Downloaded "
             << ") AS temp1 "
             << "WHERE Rated = false "
             << "AND Downloaded = true "
             << "AND Id NOT IN ( "
             <<   "SELECT Id FROM ( "
             <<     "SELECT Interview.Id, Rating.Rating IS NOT NULL AS Rated "
             <<     "FROM User "
             <<     "CROSS JOIN Interview "
             <<     "JOIN Exam ON Interview.Id = Exam.InterviewId "
             <<     "AND ( ( Exam.Modality = 'Dexa' AND User.RateDexa = true ) OR "
             <<           "( Exam.Modality = 'Retinal' AND User.RateRetinal = true ) OR "
             <<           "( Exam.Modality = 'Ultrasound' AND User.RateUltrasound = true ) ) "
             <<     "JOIN Image ON Exam.Id = Image.ExamId "
             <<     "JOIN Rating ON Image.Id = Rating.ImageId "
             <<     "AND User.Id = Rating.UserId "
             <<     "WHERE User.Id = " << userId << " "
             <<     "GROUP BY Interview.Id, Rating.Rating IS NOT NULL "
             <<   ") AS temp2 "
             <<   "WHERE Rated = true "
             << ") "
             << "AND Id NOT IN ( "
             <<   "SELECT Id FROM ( "
             <<     "SELECT Interview.Id, Exam.Downloaded "
             <<     "FROM User "
             <<     "CROSS JOIN Interview "
             <<     "JOIN Exam ON Interview.Id = Exam.InterviewId "
             <<     "AND ( ( Exam.Modality = 'Dexa' AND User.RateDexa = true ) OR "
             <<           "( Exam.Modality = 'Retinal' AND User.RateRetinal = true ) OR "
             <<           "( Exam.Modality = 'Ultrasound' AND User.RateUltrasound = true ) ) "
             <<     "WHERE User.Id = " << userId << " "
             <<     "AND Stage = 'Completed' "
             <<     "GROUP BY Interview.Id, Downloaded "
             <<   ") AS temp2 "
             <<   "WHERE Downloaded = false "
             << ") "
             << "UNION SELECT " << interviewId << ", '" << uId << "' ";
    }

    // order the query by UId (descending if not forward)
    stream << "ORDER BY UId ";
    if( !forward ) stream << "DESC ";

    Utilities::log( "Querying Database: " + stream.str() );
    vtkSmartPointer<vtkAlderMySQLQuery> query = Application::GetInstance()->GetDB()->GetQuery();
    query->SetQuery( stream.str().c_str() );
    query->Execute();

    if( query->HasError() )
    {
      Utilities::log( query->GetLastErrorText() );
      throw std::runtime_error( "There was an error while trying to query the database." );
    }

    vtkVariant neighbourId;

    // store the first record in case we need to loop over
    if( query->NextRow() )
    {
      bool found = false;
      vtkVariant currentId = this->Get( "Id" );

      // if the current id is last in the following loop then we need the first id
      neighbourId = query->DataValue( 0 );

      do // keep looping until we find the current Id
      {
        vtkVariant id = query->DataValue( 0 );
        if( found )
        {
          neighbourId = id;
          break;
        }

        if( currentId == id ) found = true;
      }
      while( query->NextRow() );

      // we should always find the current interview id
      if( !found ) throw std::runtime_error( "Cannot find current Interview in database." );
    }

    vtkSmartPointer<Interview> interview = vtkSmartPointer<Interview>::New();
    if( neighbourId.IsValid() ) interview->Load( "Id", neighbourId.ToString() );
    return interview;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::vector< std::pair< std::string, std::string > > Interview::GetUIdVisitDateList()
  {
    Application *app = Application::GetInstance();
    vtkSmartPointer<vtkAlderMySQLQuery> query = app->GetDB()->GetQuery();
    query->SetQuery( "SELECT UId, VisitDate FROM Interview ORDER BY UId, VisitDate" );
    query->Execute();

    if( query->HasError() )
    {
      Utilities::log( query->GetLastErrorText() );
      throw std::runtime_error( "There was an error while trying to query the database." );
    }

    std::vector< std::pair< std::string, std::string > > list;
    while( query->NextRow() )
      list.push_back( std::pair< std::string, std::string >(
        query->DataValue( 0 ).ToString(), query->DataValue( 1 ).ToString() ) );
    return list;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  int Interview::GetImageCount()
  {
    // loop through all exams and sum the image count
    int total = 0;

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
  bool Interview::IsRatedBy( User* user )
  {
    this->AssertPrimaryId();

    // make sure the user is not null
    if( !user ) throw std::runtime_error( "Tried to get rating for null user" );

    std::vector< vtkSmartPointer< Exam > > examList;
    std::vector< vtkSmartPointer< Exam > >::iterator examIt;
    this->GetList( &examList );
    for( examIt = examList.begin(); examIt != examList.end(); ++examIt )
    {
      Exam *exam = *(examIt);
      if( !exam->IsRatedBy( user ) ) return false;
    }

    return true;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool Interview::HasExamData()
  {
    return 0 < this->GetCount( "Exam" );
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool Interview::HasImageData()
  {
    this->AssertPrimaryId();

    std::vector< vtkSmartPointer< Exam > > examList;
    std::vector< vtkSmartPointer< Exam > >::iterator examIt;
    this->GetList( &examList );
    for( examIt = examList.begin(); examIt != examList.end(); ++examIt )
    {
      Exam *exam = *(examIt);
      if( !exam->HasImageData() ) return false;
    }

    return true;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Interview::UpdateExamData()
  {
    Application *app = Application::GetInstance();
    OpalService *opal = app->GetOpal();

    std::map< std::string, std::string > examData;
    vtkSmartPointer<Exam> exam;

    // make sure all stages exist

    // only update the exams if there are none in the database
    if( !this->HasExamData() )
    {
      // get exam metadata from Opal for this interview
      examData = opal->GetRow( "alder", "Exam", this->Get( "UId" ).ToString() );

      // CarotidIntima
      exam = vtkSmartPointer<Exam>::New();
      exam->Set( "InterviewId", this->Get( "Id" ) );
      exam->Set( "Modality", "Ultrasound" );
      exam->Set( "Type", "CarotidIntima" );
      exam->Set( "Laterality", "left" );
      exam->Set( "Stage", examData["CarotidIntima.Stage"] );
      exam->Set( "Interviewer", examData["CarotidIntima.Interviewer"] );
      exam->Set( "DatetimeAcquired", examData["CarotidIntima.DatetimeAcquired"] );
      exam->Save();

      exam = vtkSmartPointer<Exam>::New();
      exam->Set( "InterviewId", this->Get( "Id" ) );
      exam->Set( "Modality", "Ultrasound" );
      exam->Set( "Type", "CarotidIntima" );
      exam->Set( "Laterality", "right" );
      exam->Set( "Stage", examData["CarotidIntima.Stage"] );
      exam->Set( "Interviewer", examData["CarotidIntima.Interviewer"] );
      exam->Set( "DatetimeAcquired", examData["CarotidIntima.DatetimeAcquired"] );
      exam->Save();

      // DualHipBoneDensity
      exam = vtkSmartPointer<Exam>::New();
      exam->Set( "InterviewId", this->Get( "Id" ) );
      exam->Set( "Modality", "Dexa" );
      exam->Set( "Type", "DualHipBoneDensity" );
      exam->Set( "Laterality", "left" );
      exam->Set( "Stage", examData["DualHipBoneDensity.Stage"] );
      exam->Set( "Interviewer", examData["DualHipBoneDensity.Interviewer"] );
      exam->Set( "DatetimeAcquired", examData["DualHipBoneDensity.DatetimeAcquired"] );
      exam->Save();

      exam = vtkSmartPointer<Exam>::New();
      exam->Set( "InterviewId", this->Get( "Id" ) );
      exam->Set( "Modality", "Dexa" );
      exam->Set( "Type", "DualHipBoneDensity" );
      exam->Set( "Laterality", "right" );
      exam->Set( "Stage", examData["DualHipBoneDensity.Stage"] );
      exam->Set( "Interviewer", examData["DualHipBoneDensity.Interviewer"] );
      exam->Set( "DatetimeAcquired", examData["DualHipBoneDensity.DatetimeAcquired"] );
      exam->Save();

      // ForearmBoneDensity
      exam = vtkSmartPointer<Exam>::New();
      exam->Set( "InterviewId", this->Get( "Id" ) );
      exam->Set( "Modality", "Dexa" );
      exam->Set( "Type", "ForearmBoneDensity" );
      exam->Set( "Laterality", "none" );
      exam->Set( "Stage", examData["ForearmBoneDensity.Stage"] );
      exam->Set( "Interviewer", examData["ForearmBoneDensity.Interviewer"] );
      exam->Set( "DatetimeAcquired", examData["ForearmBoneDensity.DatetimeAcquired"] );
      exam->Save();

      // LateralBoneDensity
      exam = vtkSmartPointer<Exam>::New();
      exam->Set( "InterviewId", this->Get( "Id" ) );
      exam->Set( "Modality", "Dexa" );
      exam->Set( "Type", "LateralBoneDensity" );
      exam->Set( "Laterality", "none" );
      exam->Set( "Stage", examData["LateralBoneDensity.Stage"] );
      exam->Set( "Interviewer", examData["LateralBoneDensity.Interviewer"] );
      exam->Set( "DatetimeAcquired", examData["LateralBoneDensity.DatetimeAcquired"] );
      exam->Save();

      // Plaque
      exam = vtkSmartPointer<Exam>::New();
      exam->Set( "InterviewId", this->Get( "Id" ) );
      exam->Set( "Modality", "Ultrasound" );
      exam->Set( "Type", "Plaque" );
      exam->Set( "Laterality", "left" );
      exam->Set( "Stage", examData["Plaque.Stage"] );
      exam->Set( "Interviewer", examData["Plaque.Interviewer"] );
      exam->Set( "DatetimeAcquired", examData["Plaque.DatetimeAcquired"] );
      exam->Save();

      exam = vtkSmartPointer<Exam>::New();
      exam->Set( "InterviewId", this->Get( "Id" ) );
      exam->Set( "Modality", "Ultrasound" );
      exam->Set( "Type", "Plaque" );
      exam->Set( "Laterality", "right" );
      exam->Set( "Stage", examData["Plaque.Stage"] );
      exam->Set( "Interviewer", examData["Plaque.Interviewer"] );
      exam->Set( "DatetimeAcquired", examData["Plaque.DatetimeAcquired"] );
      exam->Save();

      // RetinalScan
      exam = vtkSmartPointer<Exam>::New();
      exam->Set( "InterviewId", this->Get( "Id" ) );
      exam->Set( "Modality", "Retinal" );
      exam->Set( "Type", "RetinalScan" );
      exam->Set( "Laterality", "left" );
      exam->Set( "Stage", examData["RetinalScan.Stage"] );
      exam->Set( "Interviewer", examData["RetinalScan.Interviewer"] );
      exam->Set( "DatetimeAcquired", examData["RetinalScan.DatetimeAcquired"] );
      exam->Save();

      exam = vtkSmartPointer<Exam>::New();
      exam->Set( "InterviewId", this->Get( "Id" ) );
      exam->Set( "Modality", "Retinal" );
      exam->Set( "Type", "RetinalScan" );
      exam->Set( "Laterality", "right" );
      exam->Set( "Stage", examData["RetinalScan.Stage"] );
      exam->Set( "Interviewer", examData["RetinalScan.Interviewer"] );
      exam->Set( "DatetimeAcquired", examData["RetinalScan.DatetimeAcquired"] );
      exam->Save();

      // WholeBodyBoneDensity
      exam = vtkSmartPointer<Exam>::New();
      exam->Set( "InterviewId", this->Get( "Id" ) );
      exam->Set( "Modality", "Dexa" );
      exam->Set( "Type", "WholeBodyBoneDensity" );
      exam->Set( "Laterality", "none" );
      exam->Set( "Stage", examData["WholeBodyBoneDensity.Stage"] );
      exam->Set( "Interviewer", examData["WholeBodyBoneDensity.Interviewer"] );
      exam->Set( "DatetimeAcquired", examData["WholeBodyBoneDensity.DatetimeAcquired"] );
      exam->Save();
    }
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Interview::UpdateImageData()
  {
    std::vector< vtkSmartPointer< Exam > > examList;
    this->GetList( &examList );

    if( !examList.empty() )
    {
      double index = 0;
      bool global = true;
      std::pair<bool, double> progressConfig = std::pair<bool, double>( global, 0.0 );
      std::vector< vtkSmartPointer< Exam > >::iterator examIt;
      Application *app = Application::GetInstance();

      // we are going to be downloading file type data here, so
      // we tell opal service on the first curl callback to NOT check if the data
      // has a substantial return size, and force that we monitor all file downloads using curl progress
      OpalService::SetCurlProgressChecking( false );

      app->InvokeEvent( vtkCommand::StartEvent, static_cast<void *>( &global ) );

      for( examIt = examList.begin(); examIt != examList.end(); ++examIt, ++index )
      {
        progressConfig.second = index / examList.size();
        app->InvokeEvent( vtkCommand::ProgressEvent, static_cast<void *>( &progressConfig ) );
        if( app->GetAbortFlag() ) break;
        ( *examIt )->UpdateImageData(); // invokes progress events
      }

      if( app->GetAbortFlag() ) app->SetAbortFlag( false );
      else app->InvokeEvent( vtkCommand::EndEvent, static_cast<void *>( &global ) );
    }
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Interview::UpdateInterviewData()
  {
    Application *app = Application::GetInstance();
    OpalService *opal = app->GetOpal();

    // get a list of all interview start dates
    std::map< std::string, std::map< std::string, std::string > > list;
    std::map< std::string, std::string > map, key;
    std::map< std::string, std::map< std::string, std::string > >::iterator it;
    bool done = false;
    bool global = true;
    std::pair<bool, double> progressConfig = std::pair<bool, double>( global, 0.0 );
    int limit = 100;
    double index = 0;

    std::vector< std::string > identifierList = opal->GetIdentifiers( "alder", "Interview" );
    double size = (double) identifierList.size();

    // we are going to be downloading non file type data here, so
    // we tell opal service on the first curl callback to check if the data
    // has a substantial return size that we can monitor using curl progress
    OpalService::SetCurlProgressChecking( true );

    app->InvokeEvent( vtkCommand::StartEvent, static_cast<void *>( &global ) );

    do
    {
      progressConfig.second = index / size;
      app->InvokeEvent( vtkCommand::ProgressEvent, static_cast<void *>( &progressConfig ) );
      if( app->GetAbortFlag() ) break;
      list = opal->GetRows( "alder", "Interview", index, limit ); // invokes progress events

      for( it = list.begin(); it != list.end(); ++it )
      {
        std::string UId = it->first;
        map = it->second;

        // create a unique value map to load the interview
        key["UId"] = UId;
        key["VisitDate"] = map["VisitDate"].substr( 0, 10 );

        vtkNew< Interview > interview;
        if( !interview->Load( key ) )
        { // only write to the database if the data is missing
          interview->Set( "UId", UId );
          interview->Set( map );
          interview->Save();
        }
      }

      // prepare the next block of start dates
      index += list.size();
    } while ( !list.empty() );

    if( app->GetAbortFlag() ) app->SetAbortFlag( false );
    else app->InvokeEvent( vtkCommand::EndEvent, static_cast<void *>( &global ) );
  }
}
