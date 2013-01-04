/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   ActiveRecordWithFile.cxx
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/
#include "ActiveRecordWithFile.h"

#include "Configuration.h"
#include "Exam.h"
#include "Interview.h"
#include "Rating.h"
#include "Study.h"
#include "User.h"
#include "Utilities.h"

#include "vtkDirectory.h"

#include <stdexcept>

namespace Alder
{
  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::string ActiveRecordWithFile::GetFilePath()
  {
    this->AssertPrimaryId();

    // get the study and exam for this record
    Exam *exam = Exam::SafeDownCast( this->GetRecord( "Exam" ) );
    Study *study = Study::SafeDownCast( exam->GetRecord( "Study" ) );
    Interview *interview = Interview::SafeDownCast( study->GetRecord( "Interview" ) );

    std::stringstream stream;
    // get the path of the file (we don't know file type yet)
    stream << Application::GetInstance()->GetConfig()->GetValue( "Path", "ActiveRecordWithFileData" )
           << "/" << interview->Get( "UId" ).ToString()
           << "/" << interview->Get( "VisitDate" ).ToString()
           << "/" << study->Get( "Modality" ).ToString()
           << "/" << study->Get( "Id" ).ToString()
           << "/" << exam->Get( "Type" ).ToString()
           << "/" << exam->Get( "Laterality" ).ToString()
           << "/" << exam->Get( "Id" ).ToString()
           << "/" << this->GetName();

    exam->Delete();
    study->Delete();
    interview->Delete();

    return stream.str();
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::string ActiveRecordWithFile::GetFileName()
  {
    // make sure the path exists
    std::string path = this->GetFilePath();

    // now look for image files in that directory
    vtkSmartPointer< vtkDirectory > directory = vtkSmartPointer< vtkDirectory >::New();
    
    if( !directory->Open( path.c_str() ) )
    {
      std::stringstream error;
      error << "Tried to get file but the path \"" << path << "\" does not exist.";
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
    error << "Tried to get file in \"" << path << "\" but the file does not exist.";
    throw std::runtime_error( error.str() );
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool ActiveRecordWithFile::IsRatedBy( User* user )
  {
    this->AssertPrimaryId();
    
    // make sure the user is not null
    if( !user ) throw std::runtime_error( "Tried to get rating for null user" );

    std::map< std::string, std::string > map;
    map["UserId"] = user->Get( "Id" ).ToString();
    map[this->GetName()+"Id"] = this->Get( "Id" ).ToString();
    vtkSmartPointer< Alder::Rating > rating = vtkSmartPointer< Alder::Rating >::New();
    if( !rating->Load( map ) ) return false;

    // we have found a rating, make sure it is not null
    return rating->Get( "Rating" ).IsValid();
  }
}
