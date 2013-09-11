#include <Application.h>
#include <Configuration.h>
#include <Exam.h>
#include <Interview.h>
#include <Modality.h>
#include <User.h>

#include <vtkNew.h>
#include <vtkSmartPointer.h>

#include <stdexcept>

bool ApplicationInit();

int main( int argc, char** argv )
{
  // stand alone console application that makes use of the
  // Alder business logic

  if( argc != 2 )
  {
    std::cout << "Usage: DexaClean <Alder admin pwd>" << std::endl;
    return EXIT_FAILURE;
  }

  if( !ApplicationInit() )
  {
    return EXIT_FAILURE;
  } 

  vtkNew< Alder::User > user;
  user->Load( "Name", "administrator" );
  std::string pwd = argv[1];
  if( !user->IsPassword( pwd ) )
  {
    std::cout << "Error: wrong administrator password" << std::endl;
    Alder::Application::DeleteInstance();
    return EXIT_FAILURE;
  }

  Alder::Application *app = Alder::Application::GetInstance();

  // search the Alder db for dexa exams of type Hip, Forearm and WholeBody
  // process the images report which ones had the dicom Patient Name tag populated

  std::vector< vtkSmartPointer< Alder::Interview > > interviewList;
  Alder::Interview::GetAll( &interviewList );
  if( !interviewList.empty() )
  {    
    for( auto interviewIt = interviewList.begin(); interviewIt != interviewList.end(); ++interviewIt )
    {
      Alder::Interview *interview = interviewIt->GetPointer();
      std::vector< vtkSmartPointer< Alder::Exam > > examList;
      interview->GetList( &examList );
      for( auto examIt = examList.begin(); examIt != examList.end(); ++examIt )
      {
        Alder::Exam *exam = examIt->GetPointer();
        vtkSmartPointer< Alder::Modality > modality;
        exam->GetRecord( modality );
        if( modality->Get( "Name" ).ToString() != "Dexa" ) continue;
        if( exam->Get( "Type" ).ToString() == "LateralBoneDensity" ) continue;

        std::vector< vtkSmartPointer< Alder::Image > > imageList;
        exam->GetList( &imageList );
        for( auto imageIt = imageList.begin(); imageIt != imageList.end(); ++imageIt )
        {
          Alder::Image *image = imageIt->GetPointer();
          std::cout << "Inter: " << exam->Get( "Interviewer" ).ToString() << ", "
           << "Site: " << interview->Get( "Site" ).ToString() << ", "
           << "Date: " << exam->Get( "DatetimeAcquired" ).ToString() << ", "
           << "Mod: " << modality->Get( "Name" ).ToString() << ", "
           << "Type: " << exam->Get( "Type" ).ToString() << ", "
           << "Lat: " << exam->Get( "Laterality" ) << ", "
           << "Dl: " << exam->Get( "Downloaded" ) << ", "
           << "Path: " << image->GetFileName() << std::endl;
        }
      }
    }
  }

  Alder::Application::DeleteInstance();
  return EXIT_SUCCESS;
}

bool ApplicationInit()
{
  bool status = true;
  Alder::Application *app = Alder::Application::GetInstance();
  if( !app->ReadConfiguration( ALDER_CONFIG_FILE ) ) 
  {   
    std::cout << "ERROR: error while reading configuration file \"" 
              << ALDER_CONFIG_FILE << "\"" << std::endl;
    status = false;          
  }   
  else if( !app->OpenLogFile() )
  {   
    std::string logPath = app->GetConfig()->GetValue( "Path", "Log" );
    std::cout << "ERROR: unable to open log file \"" 
              << logPath << "\"" << std::endl;
    status = false;          
  }   
  else if( !app->TestImageDataPath() )
  {   
    std::string imageDataPath = app->GetConfig()->GetValue( "Path", "ImageData" );
    std::cout << "ERROR: no write access to image data directory \"" 
              << imageDataPath << "\"" << std::endl;
    status = false;          
  }   
  else if( !app->ConnectToDatabase() )
  {   
    std::cout << "ERROR: error while connecting to the database" << std::endl;
    status = false;          
  }
  if( !status )
  {
    Alder::Application::DeleteInstance();
  }
  return status;
}
