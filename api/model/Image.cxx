/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   Image.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include "Image.h"

#include "Configuration.h"
#include "Exam.h"
#include "Interview.h"
#include "Rating.h"
#include "User.h"
#include "Utilities.h"

#include "vtkDirectory.h"
#include "vtkImageDataReader.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"

#include "gdcmImageReader.h"
#include "gdcmDirectoryHelper.h"

#include <stdexcept>

namespace Alder
{
  vtkStandardNewMacro( Image );

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::string Image::GetFilePath()
  {
    this->AssertPrimaryId();

    // get the exam for this record
    vtkSmartPointer< Exam > exam;
    if( !this->GetRecord( exam ) )
      throw std::runtime_error( "Image has no parent exam!" );

    vtkSmartPointer< Interview > interview;
    if( !exam->GetRecord( interview ) )
      throw std::runtime_error( "Exam has no parent interview!" );

    std::stringstream stream;
    // get the path of the file (we don't know file type yet)
    stream << Application::GetInstance()->GetConfig()->GetValue( "Path", "ImageData" )
           << "/" << interview->Get( "Id" ).ToString()
           << "/" << exam->Get( "Id" ).ToString();

    return stream.str();
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::string Image::CreateFile( std::string suffix )
  {
    // first get the path and create it if it doesn't exist
    std::string path = this->GetFilePath();
    if( !Utilities::fileExists( path ) ) vtkDirectory::MakeDirectory( path.c_str() );

    return path + "/" + this->Get( "Id" ).ToString() + suffix;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool Image::ValidateFile()
  {
    bool valid;
    std::string fileName = this->GetFileName();

    // now check the file, if it is empty delete the image and the file
    if( 0 == Utilities::getFileLength( fileName ) )
    {
      valid = false;
    }
    else // file exists
    {
      // if the file has a .gz extension, unzip it
      if( ".gz" == fileName.substr( fileName.length() - 3, 3 ) )
      {
        std::string zipFileName = fileName;
        fileName = fileName.substr( 0, fileName.length() - 3 );

        std::string command = "gunzip ";
        command += zipFileName;

        // not a gz file, remove the .gz extension manually
        if( "ERROR" == Utilities::exec( command ) )
          rename( zipFileName.c_str(), fileName.c_str() );
      }

      // now see if we can read the file
      valid = vtkImageDataReader::IsValidFileName( fileName.c_str() );
    }

    // if the file isn't valid, remove it from the disk
    if( !valid ) remove( fileName.c_str() );

    return valid;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::string Image::GetFileName()
  {
    // make sure the path exists
    std::string path = this->GetFilePath();

    // now look for image files in that directory
    vtkNew< vtkDirectory > directory;
    
    if( !directory->Open( path.c_str() ) )
    {
      std::stringstream error;
      error << "Tried to get image file but the path \"" << path << "\" does not exist.";
      throw std::runtime_error( error.str() );
    }

    // we don't know the file type yet, search for all files which have our Id
    std::string id = this->Get( "Id" ).ToString();
    for( vtkIdType index = 0; index < directory->GetNumberOfFiles(); index++ )
    {
      std::string fileName = directory->GetFile( index );
      if( fileName.substr( 0, id.length() ) == id )
      {
        std::stringstream name;
        name << path << "/" << fileName;
        return name.str();
      }
    }

    // if we get here then the file was not found
    std::stringstream error;
    error << "Tried to get image file in \"" << path << "\" but the file does not exist.";
    throw std::runtime_error( error.str() );
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool Image::IsRatedBy( User* user )
  {
    this->AssertPrimaryId();
    
    // make sure the user is not null
    if( !user ) throw std::runtime_error( "Tried to get rating for null user" );

    std::map< std::string, std::string > map;
    map["UserId"] = user->Get( "Id" ).ToString();
    map["ImageId"] = this->Get( "Id" ).ToString();
    vtkNew< Alder::Rating > rating;
    if( !rating->Load( map ) ) return false;

    // we have found a rating, make sure it is not null
    return rating->Get( "Rating" ).IsValid();
  }
  
  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::string Image::GetDICOMAcquisitionDateTime()
  {
    this->AssertPrimaryId();

    gdcm::ImageReader reader;
    reader.SetFileName( this->GetFileName().c_str() );
    reader.Read();
    const gdcm::File &file = reader.GetFile();
    const gdcm::DataSet &ds = file.GetDataSet();

    return std::string( 
      gdcm::DirectoryHelper::GetStringValueFromTag( gdcm::Tag(0x0008,0x002a), ds ) );
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::vector<int> Image::GetDICOMDimensions()
  {
    this->AssertPrimaryId();

    gdcm::ImageReader reader;
    reader.SetFileName( this->GetFileName().c_str() );
    reader.Read();
    gdcm::Image &image = reader.GetImage();
    
    std::vector<int> dims;
    for( int i = 0; i < 3; ++i )
      dims.push_back( image.GetDimension(i) );
    
    return dims;  
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  vtkSmartPointer<Image> Image::GetNeighbourAtlasImage( bool forward )
  {
    this->AssertPrimaryId();
    vtkSmartPointer<Image> image = vtkSmartPointer<Image>::New();

    // TODO: get neighbouring image which matches the requested exam type and image rating (from an expert user)

    return image;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  vtkSmartPointer<Image> Image::GetAtlasImage( std::string type, int rating )
  {
    vtkSmartPointer<Image> image = vtkSmartPointer<Image>::New();

    // TODO: get any image which matches the requested exam type and image rating (from an expert user)

    return image;
  }
}
