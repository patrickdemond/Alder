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
    vtkSmartPointer< Interview > interview;

    // start by getting the UId
    this->GetRecord( interview );
    std::string UId = interview->Get( "UId" ).ToString();
    bool result = false;

    // only update the images if the stage is complete and they have never been downloaded
    // NOTE: it is possible that an exam with state "Ready" has valid data, but we are leaving
    // those exams out for now since we don't know for sure whether they are always valid
    if( 0 == this->Get( "Stage" ).ToString().compare( "Completed" ) &&
        0 == this->Get( "Downloaded" ).ToInt() )
    {
      // determine which Opal table to fetch from based on exam modality
      std::string type = this->Get( "Type" ).ToString();
      std::map< std::string, vtkVariant > settings;
      settings[ "ExamId" ] = this->Get("Id");
      settings[ "Acquisition" ] = vtkVariant( 1 );

      if( 0 == type.compare( "CarotidIntima" ) )
      {
        // write cineloops 1, 2 and 3
        // for now, assume that the parent image id for the still image
        // associated with one of the 3 possible cineloops is the first valid one
        // 
        bool hasValidParent = false;
        int validParentId = 0;
        int maxAcquisition = 0;
        std::string suffix = ".dcm.gz";
        std::string sideVariable = "Measure.SIDE";

        for( int acquisition = 1; acquisition <= 3; acquisition++ )
        {
          std::string variable = "Measure.CINELOOP_";
          variable += vtkVariant( acquisition ).ToString();
          settings[ "Acquisition" ] = vtkVariant(acquisition);

          result = this->RetrieveImage( type, variable, UId, settings,
                                        suffix, sideVariable );

          if( result )
          {
            hasValidParent = true;
          }
          maxAcquisition = acquisition;
        }

        // TODO: STILL_IMAGE and SR files still need to be downloaded

        if( hasValidParent )
        {
          std::string variable = "Measure.STILL_IMAGE";
          // need an Image instance to get the last cineloop's Id 
          vtkSmartPointer< Image > image = vtkSmartPointer< Image >::New(); 
          maxAcquisition++;
          settings[ "Acquisition" ] = vtkVariant(maxAcquisition);
          settings[ "ParentImageId" ] = vtkVariant( image->GetLastInsertId() );
          result = this->RetrieveImage( type, variable, UId, settings,
                                        suffix, sideVariable );
        }
      }
      else if( 0 == type.compare( "DualHipBoneDensity" ) )
      {
        std::string variable = "Measure.RES_HIP_DICOM";
        std::string sideVariable = "Measure.OUTPUT_HIP_SIDE";
        std::string suffix = ".dcm";
        result = this->RetrieveImage( type, variable, UId, settings,
                                      suffix, sideVariable );
      }
      else if( 0 == type.compare( "ForearmBoneDensity" ) )
      {
        std::string variable = "RES_FA_DICOM";
        std::string sideVariable = "OUTPUT_FA_SIDE";
        std::string suffix = ".dcm";
        result = this->RetrieveImage( type, variable, UId, settings,
                                          suffix, sideVariable );
      }
      else if( 0 == type.compare( "LateralBoneDensity" ) )
      {
        std::string variable = "RES_SEL_DICOM_MEASURE";
        std::string suffix = ".dcm";
        result = this->RetrieveImage( type, variable, UId, settings, suffix );
      }
      else if( 0 == type.compare( "Plaque" ) )
      {
        std::string variable = "Measure.CINELOOP_1";
        std::string sideVariable = "Measure.SIDE";
        std::string suffix = ".dcm.gz";
        result = this->RetrieveImage( type, variable, UId, settings,
                                      suffix, sideVariable );
      }
      else if( 0 == type.compare( "RetinalScan" ) )
      {
        std::string variable = "Measure.EYE";
        std::string sideVariable = "Measure.SIDE";
        std::string suffix = ".jpg";
        result = this->RetrieveImage( type, variable, UId, settings,
                                      suffix, sideVariable );
      }
      else if( 0 == type.compare( "WholeBodyBoneDensity" ) )
      {
        std::string variable = "RES_WB_DICOM_1";
        std::string suffix = ".dcm";
        result = this->RetrieveImage( type, variable, UId, settings, suffix );

        if( result )
        {
          variable = "RES_WB_DICOM_2";
          settings[ "Acquisition" ] = vtkVariant( 2 ); 
          result = this->RetrieveImage( type, variable, UId, settings, suffix );
        }
      }

      // now set that we have downloaded all the images
      if( result )
      {
        this->Set( "Downloaded", 1 );
      }
      this->Save();
    }
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool Exam::RetrieveImage( std::string type, std::string variable,  std::string UId,
                            std::map<std::string, vtkVariant> settings,
                            std::string suffix, std::string sideVariable )
  {
    Application *app = Application::GetInstance();
    OpalService *opal = app->GetOpal();
    bool repeatable = sideVariable.length() > 0; 
    bool result = true;
    int sideIndex = 0; 
     
    if( repeatable )
    {
      std::vector< std::string > sideList;
      std::vector< std::string >::iterator sideListIt;     
      sideList = opal->GetValues( "clsa-dcs-images", type, UId, sideVariable );
      bool found = false;
      std::string laterality = this->Get( "Laterality" ).ToString();
      //TODO: handle the case whereine the side is an empty string
      for( sideListIt = sideList.begin(); sideListIt != sideList.end(); ++sideListIt )
      {
        if( 0 == Utilities::toLower( *sideListIt ).compare( laterality ) )
        {
          found = true;
          break;
        }
        sideIndex++;
      }
      if( !found )
      {
        throw std::runtime_error( "Failed to find image laterality" );         
      } 
    }

    std::stringstream log;
    log << "Adding " << variable << " to database for UId \"" << UId << "\"";
    Utilities::log( log.str() );

    // add a new entry in the image table
    vtkSmartPointer< Image > image = vtkSmartPointer< Image >::New();
    std::map< std::string, vtkVariant >::iterator it = settings.begin();
    for( it = settings.begin(); it != settings.end(); it++ )
    { 
      image->Set( it->first,  it->second );
    }
    image->Save();

    // now write the file and validate it
    std::string fileName = image->CreateFile( suffix );
    if( repeatable )
    {
      opal->SaveFile( fileName, "clsa-dcs-images", type, UId, variable, sideIndex );
    }
    else 
    {
      opal->SaveFile( fileName, "clsa-dcs-images", type, UId, variable );
    }

    if( !image->ValidateFile() )
    {
      log.str( "" );
      log << "Removing " << variable << " from database (invalid)";
      Utilities::log( log.str() );
      image->Remove();
      result = false;
    }
    return result;   
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
