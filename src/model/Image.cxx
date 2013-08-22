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
  std::string Image::GetCode()
  {
    this->AssertPrimaryId();

    // get the exam's code and add append the image id
    vtkSmartPointer< Exam > exam;
    if( !this->GetRecord( exam ) )
      throw std::runtime_error( "Image has no parent exam!" );

    std::stringstream stream;
    stream << exam->GetCode() << "/" << this->Get( "Id" ).ToString();
    return stream.str();
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::string Image::GetFilePath()
  {
    this->AssertPrimaryId();

    vtkSmartPointer< Exam > exam;
    if( !this->GetRecord( exam ) )
      throw std::runtime_error( "Image has no parent exam!" );

    // the image file's directory is simply the image data path and the exam code
    std::stringstream stream;
    stream << Application::GetInstance()->GetConfig()->GetValue( "Path", "ImageData" )
           << "/" << exam->GetCode();

    return stream.str();
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::string Image::CreateFile( const std::string suffix )
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
    Application *app = Application::GetInstance();

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
        app->Log( std::string( "Unzipping file: " ) + fileName );
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
  std::string Image::GetDICOMTag( const std::string tagName )
  {
    this->AssertPrimaryId();

    // get the name of the unzipped file
    std::string fileName = this->GetFileName();
    if( ".gz" == fileName.substr( fileName.length() - 3, 3 ) )
      fileName = fileName.substr( 0, fileName.length() - 3 );

    gdcm::ImageReader reader;
    reader.SetFileName( fileName.c_str() );
    reader.Read();
    const gdcm::File &file = reader.GetFile();
    const gdcm::DataSet &ds = file.GetDataSet();

    // TODO: use GDCM to get the correct tags
    gdcm::Tag tag;
    if( "AcquisitionDateTime" == tagName ) tag = gdcm::Tag( 0x0008, 0x002a );
    else if( "SeriesNumber" == tagName ) tag = gdcm::Tag( 0x0020,0x0011 );
    else throw std::runtime_error( "Unknown DICOM tag name." );

    if( !ds.FindDataElement( tag ) )
      throw std::runtime_error( "Unknown DICOM tag with name " + tagName );

    return std::string( 
      gdcm::DirectoryHelper::GetStringValueFromTag( tag, ds ) );
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::vector<int> Image::GetDICOMDimensions()
  {
    this->AssertPrimaryId();

    // get the name of the unzipped file
    std::string fileName = this->GetFileName();
    if( ".gz" == fileName.substr( fileName.length() - 3, 3 ) )
      fileName = fileName.substr( 0, fileName.length() - 3 );

    gdcm::ImageReader reader;
    reader.SetFileName( fileName.c_str() );
    reader.Read();
    gdcm::Image &image = reader.GetImage();
    
    std::vector<int> dims;
    for( int i = 0; i < 3; ++i )
      dims.push_back( image.GetDimension(i) );
    
    return dims;  
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  vtkSmartPointer<Image> Image::GetNeighbourAtlasImage( const int rating, const bool forward )
  {
    this->AssertPrimaryId();
    Application *app = Application::GetInstance();
    Image *activeImage = app->GetActiveImage();
    bool hasParent = this->Get( "ParentImageId" ).IsValid();

    // get neighbouring image which matches this image's exam type and the given rating
    std::stringstream stream;
    stream << "SELECT Image.Id "
           << "FROM Image "
           << "JOIN Exam ON Image.ExamId = Exam.Id "
           << "JOIN Interview ON Exam.InterviewId = Interview.Id "
           << "JOIN Rating ON Image.Id = Rating.ImageId "
           << "JOIN User ON Rating.UserId = User.Id "
           << "WHERE Exam.Type = ( "
           <<   "SELECT Exam.Type "
           <<   "FROM Exam "
           <<   "JOIN Image ON Exam.Id = Image.ExamId "
           <<   "WHERE Image.Id = " << this->Get( "Id" ).ToString() << " "
           << ") "
           << "AND Image.ParentImageId IS " << ( hasParent ? "NOT" : "" ) << " NULL "
           << "AND Rating = " << rating << " "
           << "AND User.Expert = true ";

    // do not show the active image
    if( NULL != activeImage ) stream << "AND Image.Id != " << activeImage->Get( "Id" ).ToString() << " ";

    // order the query by UId (descending if not forward)
    stream << "ORDER BY Interview.UId ";
    if( !forward ) stream << "DESC ";

    app->Log( "Querying Database: " + stream.str() );
    vtkSmartPointer<vtkAlderMySQLQuery> query = app->GetDB()->GetQuery();
    query->SetQuery( stream.str().c_str() );
    query->Execute();

    if( query->HasError() )
    {
      app->Log( query->GetLastErrorText() );
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

      // we should always find the current image id
      if( !found ) throw std::runtime_error( "Cannot find current atlas image in database." );
    }

    vtkSmartPointer<Image> image = vtkSmartPointer<Image>::New();
    if( neighbourId.IsValid() ) image->Load( "Id", neighbourId.ToString() );
    return image;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  vtkSmartPointer<Image> Image::GetAtlasImage( const int rating )
  {
    Application *app = Application::GetInstance();
    vtkSmartPointer<vtkAlderMySQLQuery> query = app->GetDB()->GetQuery();

    vtkSmartPointer<Exam> exam;
    this->GetRecord( exam );
    bool hasParent = this->Get( "ParentImageId" ).IsValid();

    // get any image rated by an expert user having the given exam type and rating score
    std::stringstream stream;
    stream << "SELECT Image.Id "
           << "FROM Image "
           << "JOIN Exam ON Image.ExamId = Exam.Id "
           << "JOIN Rating ON Image.Id = Rating.ImageId "
           << "JOIN User ON Rating.UserId = User.Id "
           << "WHERE Exam.Type = " << query->EscapeString( exam->Get( "Type" ).ToString() ) << " "
           << "AND Image.ParentImageId IS " << ( hasParent ? "NOT" : "" ) << " NULL "
           << "AND Rating = " << rating << " "
           << "AND User.Expert = true "
           << "AND Image.Id != " << this->Get( "Id" ).ToString() << " "
           << "LIMIT 1";

    app->Log( "Querying Database: " + stream.str() );
    query->SetQuery( stream.str().c_str() );
    query->Execute();

    if( query->HasError() )
    {   
      app->Log( query->GetLastErrorText() );
      throw std::runtime_error( "There was an error while trying to query the database." );
    }   

    vtkSmartPointer<Image> image = vtkSmartPointer<Image>::New();
    if( query->NextRow() ) image->Load( "Id", query->DataValue( 0 ).ToString() );
    return image;
  }
}
