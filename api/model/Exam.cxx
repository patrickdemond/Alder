/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   Exam.cxx
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/
#include "Exam.h"

#include "Application.h"
#include "Image.h"
#include "Interview.h"
#include "OpalService.h"
#include "Utilities.h"

#include "vtkObjectFactory.h"

namespace Alder
{
  vtkStandardNewMacro( Exam );

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Exam::Update()
  {
    Application *app = Application::GetInstance();
    OpalService *opal = app->GetOpal();
    std::vector< std::string > sideList;
    std::vector< std::string >::iterator sideListIt;
    vtkSmartPointer< Interview > interview;

    // start by getting the UId
    this->GetRecord( interview );
    std::string UId = interview->Get( "UId" ).ToString();

    // only update the images if the stage is complete and they have never been downloaded
    // NOTE: it is possible that an exam with state "Ready" has valid data, but we are leaving
    // those exams out for now since we don't know for sure whether they are always valid
    if( 0 == this->Get( "Stage" ).ToString().compare( "Completed" ) &&
        0 == this->Get( "Downloaded" ).ToInt() )
    {
      // determine which Opal table to fetch from based on exam modality
      std::string type = this->Get( "Type" ).ToString();
      std::string laterality = this->Get( "Laterality" ).ToString();

      if( 0 == type.compare( "CarotidIntima" ) )
      {
        // see if the laterality exists
        sideList = opal->GetValues( "clsa-dcs-images", "CarotidIntima", UId, "Measure.SIDE" );
        
        bool found = false;
        int sideIndex = 0;
        for( sideListIt = sideList.begin(); sideListIt != sideList.end(); ++sideListIt )
        {
          if( 0 == Utilities::toLower( *sideListIt ).compare( laterality ) )
          {
            found = true;
            break;
          }
          sideIndex++;
        }

        // only get the image if its laterality is found
        if( found )
        {
          // write cineloops 1, 2 and 3
          for( int i = 1; i <= 3; i++ )
          {
            std::string variable = "Measure.CINELOOP_";
            variable += vtkVariant( i ).ToString();

            std::stringstream log;
            log << "Adding " << variable << " to database for UId \"" << UId << "\"";
            Utilities::log( log.str() );

            // add a new entry in the image table
            vtkSmartPointer< Image > image = vtkSmartPointer< Image >::New();
            image->Set( "ExamId", this->Get( "Id" ) );
            image->Set( "Acquisition", i );
            image->Save();

            // now write the file and validate it
            std::string fileName = image->CreateFile( ".dcm.gz" );
            opal->SaveFile( fileName, "clsa-dcs-images", "CarotidIntima", UId, variable, sideIndex );
            if( !image->ValidateFile() )
            {
              log.str( "" );
              log << "Removing " << variable << " from database (invalid)";
              Utilities::log( log.str() );
              image->Remove();
            }
          }

          // TODO: STILL_IMAGE and SR files still need to be downloaded
        }
      }
      else if( 0 == type.compare( "DualHipBoneDensity" ) )
      {
      }
      else if( 0 == type.compare( "ForearmBoneDensity" ) )
      {
      }
      else if( 0 == type.compare( "LateralBoneDensity" ) )
      {
      }
      else if( 0 == type.compare( "Plaque" ) )
      {
      }
      else if( 0 == type.compare( "RetinalScan" ) )
      {
        // see if the laterality exists
        sideList = opal->GetValues( "clsa-dcs-images", "RetinalScan", UId, "Measure.SIDE" );
        
        bool found = false;
        int sideIndex = 0;
        for( sideListIt = sideList.begin(); sideListIt != sideList.end(); ++sideListIt )
        {
          if( 0 == Utilities::toLower( *sideListIt ).compare( laterality ) )
          {
            found = true;
            break;
          }
          sideIndex++;
        }

        // only get the image if its laterality is found
        if( found )
        {
          std::string variable = "Measure.EYE";

          std::stringstream log;
          log << "Adding " << variable << " to database for UId \"" << UId << "\"";
          Utilities::log( log.str() );

          // add a new entry in the image table
          vtkSmartPointer< Image > image = vtkSmartPointer< Image >::New();
          image->Set( "ExamId", this->Get( "Id" ) );
          image->Set( "Acquisition", 1 );
          image->Save();

          // now write the file and validate it
          std::string fileName = image->CreateFile( ".jpg" );
          opal->SaveFile( fileName, "clsa-dcs-images", "RetinalScan", UId, variable, sideIndex );
          if( !image->ValidateFile() )
          {
            log.str( "" );
            log << "Removing " << variable << " from database (invalid)";
            Utilities::log( log.str() );
            image->Remove();
          }
        }
      }
      else if( 0 == type.compare( "WholeBodyBoneDensity" ) )
      {
      }

      // now set that we have downloaded the images
      this->Set( "Downloaded", 1 );
      this->Save();
    }
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool Exam::IsRatedBy( User* user )
  {
    this->AssertPrimaryId();

    // make sure the user is not null
    if( !user ) throw std::runtime_error( "Tried to get rating for null user" );

    // loop through all images
    std::vector< vtkSmartPointer< Image > > imageList;
    std::vector< vtkSmartPointer< Image > >::iterator imageIt;
    this->GetList( &imageList );
    for( imageIt = imageList.begin(); imageIt != imageList.end(); ++imageIt )
    {
      Image *image = *(imageIt);
      if( !image->IsRatedBy( user ) ) return false;
    }

    // only return true if there was at least one image rated
    return 0 < imageList.size();
  }
}
