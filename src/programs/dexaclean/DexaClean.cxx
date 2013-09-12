#include <Application.h>
#include <Configuration.h>
#include <Exam.h>
#include <Interview.h>
#include <Modality.h>
#include <User.h>

#include <vtkImageData.h>
#include <vtkImageDataReader.h>
#include <vtkImageCanvasSource2D.h>
#include <vtkImageFlip.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>

#include <gdcmAnonymizer.h>
#include <gdcmReader.h>
#include <gdcmWriter.h>

#include <fstream>
#include <stdexcept>

bool ApplicationInit();
bool CleanImage( std::string const &fileName, int const &examType );

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

      std::string siteStr = interview->Get( "Site" ).ToString();

      for( auto examIt = examList.begin(); examIt != examList.end(); ++examIt )
      {
        Alder::Exam *exam = examIt->GetPointer();
        vtkSmartPointer< Alder::Modality > modality;
        exam->GetRecord( modality );
        std::string modStr = modality->Get( "Name" ).ToString();
        std::string typeStr = exam->Get( "Type" ).ToString();

        if( modStr != "Dexa" ) continue;
        if( typeStr == "LateralBoneDensity" ) continue;

        std::string latStr = exam->Get( "Laterality" ).ToString();

        std::vector< vtkSmartPointer< Alder::Image > > imageList;
        exam->GetList( &imageList );
        for( auto imageIt = imageList.begin(); imageIt != imageList.end(); ++imageIt )
        {
          Alder::Image *image = imageIt->GetPointer();
          std::string fileName = image->GetFileName();
          std::cout << "Inter: " << exam->Get( "Interviewer" ).ToString() << ", "
           << "Site: " << siteStr << ", "
           << "Date: " << exam->Get( "DatetimeAcquired" ).ToString() << ", "
           << "Mod: " << modStr << ", "
           << "Type: " << typeStr << ", "
           << "Lat: " << latStr << ", "
           << "Dl: " << exam->Get( "Downloaded" ).ToInt() << ", "
           << "Path: " << fileName << std::endl;

          //check the dicom tag for laterality
          if( latStr != "none" )
          {
            try{
              std::string tagStr = image->GetDICOMTag( "Laterality" );
              if( tagStr.size() > 0 )
              {
                tagStr = Alder::Utilities::toLower( tagStr ); // tagStr will either be R or L
                if( tagStr.compare(0, 1, latStr, 0, 1) != 0 )
                {
                  latStr = tagStr.compare(0, 1, "l", 0, 1) == 0 ? "left" : "right";
                  exam->Set( "Laterality", latStr );
                  exam->Save();
                }
              }            
            }
            catch(...)
            {
            }
          }

          //check the dicom tag for a patient name
          std::string nameStr = image->GetDICOMTag( "PatientsName" );
          if( !nameStr.empty() ) 
          {
            std::cout << "CONF BREECH!!! " << nameStr << std::endl;
            int examType = -1;
            if( typeStr == "DualHipBoneDensity" )
            {  
              examType = latStr == "left" ? 0 : 1;
            }
            else if( typeStr == "ForearmBoneDensity" )
            {
              examType = 2;
            }
            else if( typeStr == "WholeBodyBoneDensity" )
            {
              // check if the image has a parent, if so, it is a body composition file
              examType =( image->Get( "ParentImageId" ).IsValid() ) ? 4 : 3;
            }  
            CleanImage( fileName, examType );
          }
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


bool CleanImage( std::string const &fileName, int const &examType )
{
  if( examType < 0 || examType > 4 ) return false;

  vtkNew<vtkImageDataReader> reader;
  reader->SetFileName( fileName.c_str() );
  
  vtkImageData* image = reader->GetOutput();

  int extent[6];
  image->GetExtent( extent );
  int dims[3];
  image->GetDimensions(dims);

  // start in the middle of the left edge,
  // increment across until the color changes to 255,255,255
  
  // left edge coordinates for each DEXA exam type
  int x0[5] = { 168, 168, 168, 168, 193 };
  // bottom edge coordinates
  int y0[5] = { 1622, 1622, 1111, 1377, 1434 };
  // top edge coordinates
  int y1[5] = { 1648, 1648, 1138, 1403, 1456 };

  bool found = false;
  // start search from the middle of the left edge 
  int ix = x0[ examType ];
  int iy = y0[ examType ] + ( y1[ examType ] - y0[ examType ] )/2; 
  do  
  {
    int val = static_cast<int>( image->GetScalarComponentAsFloat( ix++, iy, 0, 0 ) );
    if( val == 255 )
    {
      found = true;
      ix--;
    }
  }while( !found && ix < extent[1] );

  if( !found )
  {
    std::cout << "ERR: edge of name field was not found" << std::endl;
    return false;
  }
  else std::cout << "edge of name field found at " << ix << std::endl;

  vtkNew<vtkImageCanvasSource2D> canvas;
  // copy the image onto the canvas
  canvas->SetNumberOfScalarComponents( image->GetNumberOfScalarComponents() );
  canvas->SetScalarType( image->GetScalarType() );
  canvas->SetExtent( extent );
  canvas->DrawImage( 0, 0, image );
  // erase the name field with its gray background color
  canvas->SetDrawColor( 222, 222, 222 );
  canvas->FillBox( x0[ examType ] , ix, y0[ examType ], y1[ examType ] );
  canvas->Update();

  // flip the canvas vertically
  vtkNew< vtkImageFlip > flip;
  flip->SetInput( canvas->GetOutput() );
  flip->SetFilteredAxis( 1 );
  flip->Update();

  unsigned pos = fileName.find_last_of( "." );
  std::string outName = fileName.substr( 0, pos );
  outName += "_repaired.dcm";

  // byte size of the original dicom file
  unsigned long flength = Alder::Utilities::getFileLength( fileName );
  // byte size of the image
  unsigned long ilength = dims[0]*dims[1]*3;
  // byte size of the dicom header
  unsigned long hlength = flength - ilength;

  std::cout << "image dims: " << dims[0] << " x " << dims[1] << " x " << dims[2] << std::endl;
  std::cout << "file size: " << flength << std::endl;
  std::cout << "image size: " << ilength << std::endl;
  std::cout << "header size: " << hlength << std::endl;

  // read in the input dicom file
  std::ifstream infs;
  infs.open( fileName.c_str(), std::fstream::binary );
  if( !infs.is_open() )
  {
    std::cout << "ERROR: failed to stream in dicom data" << std::cout;
    return false;
  }

  char* buffer = new char[flength];
  infs.read( buffer, hlength );
  infs.close();

  // output the repaired dicom file
  std::ofstream outfs;
  outfs.open( outName.c_str(), std::ofstream::binary | std::ofstream::trunc );

  if( !outfs.is_open() )
  {
    std::cout << "ERROR: failed to stream out dicom data" << std::cout;
    delete[] buffer;    
    return false;
  }

  outfs.write( buffer, hlength );
  outfs.write( (char*)(flip->GetOutput()->GetScalarPointer()), ilength );
  outfs.close();

  delete[] buffer;

  // anonymize the name field dicom tags
  gdcm::Reader gdcmRead;
  gdcmRead.SetFileName( outName.c_str() );
  if( !gdcmRead.Read() )
  {
    std::cout << "ERROR: failed to anonymize dicom data during read" << std::cout;
    return false;
  }
  gdcm::Anonymizer gdcmAnon;
  gdcmAnon.SetFile( gdcmRead.GetFile() );
  gdcmAnon.Empty( gdcm::Tag(0x10, 0x10) );

  gdcm::Writer gdcmWriter;
  gdcmWriter.SetFile( gdcmAnon.GetFile() );
  gdcmWriter.SetFileName( outName.c_str() );
  if( !gdcmWriter.Write() )
  {
    std::cout << "ERROR: failed to anonymize dicom data during write" << std::cout;
    return false;
  }
  return true;
}
