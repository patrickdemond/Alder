/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   Image.cxx
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/
#include "Image.h"

#include "Configuration.h"
#include "Exam.h"
#include "Interview.h"
#include "Rating.h"
#include "Study.h"
#include "User.h"
#include "Utilities.h"

#include "vtkDirectory.h"
#include "vtkObjectFactory.h"

#include <stdexcept>

namespace Alder
{
  vtkStandardNewMacro( Image );

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::string Image::GetFilePath()
  {
    this->AssertPrimaryId();

    // get the study and exam for this record
    Exam *exam = Exam::SafeDownCast( this->GetRecord( "Exam" ) );
    Study *study = Study::SafeDownCast( exam->GetRecord( "Study" ) );
    Interview *interview = Interview::SafeDownCast( study->GetRecord( "Interview" ) );

    std::stringstream stream;
    // get the path of the file (we don't know file type yet)
    stream << Application::GetInstance()->GetConfig()->GetValue( "Path", "ImageData" )
           << "/" << interview->Get( "Id" ).ToString()
           << "/" << study->Get( "Id" ).ToString()
           << "/" << exam->Get( "Id" ).ToString();

    exam->Delete();
    study->Delete();
    interview->Delete();

    return stream.str();
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::string Image::GetFileName()
  {
    // make sure the path exists
    std::string path = this->GetFilePath();

    // now look for image files in that directory
    vtkSmartPointer< vtkDirectory > directory = vtkSmartPointer< vtkDirectory >::New();
    
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
      std::string filename = directory->GetFile( index );
      if( filename.substr( 0, id.length() ) == id )
      {
        std::stringstream name;
        name << path << "/" << filename;
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
    vtkSmartPointer< Alder::Rating > rating = vtkSmartPointer< Alder::Rating >::New();
    if( !rating->Load( map ) ) return false;

    // we have found a rating, make sure it is not null
    return rating->Get( "Rating" ).IsValid();
  }
}
