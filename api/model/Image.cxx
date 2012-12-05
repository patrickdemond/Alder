/*=========================================================================

  Program:  Alder (CLSA Ultrasound Image Viewer)
  Module:   Image.cxx
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/
#include "Image.h"

#include "Configuration.h"
#include "Exam.h"
#include "Rating.h"
#include "Study.h"
#include "User.h"
#include "Utilities.h"

#include "vtkObjectFactory.h"

#include <stdexcept>

namespace Alder
{
  vtkStandardNewMacro( Image );

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::string Image::GetFileName()
  {
    this->AssertPrimaryId();

    // get the study and exam for this record
    Exam *exam = Exam::SafeDownCast( this->GetRecord( "Exam" ) );
    Study *study = Study::SafeDownCast( exam->GetRecord( "Study" ) );

    std::stringstream stream;
    // start with the base image directory
    stream << Application::GetInstance()->GetConfig()->GetValue( "Path", "ImageData" )
           << "/" << study->Get( "uid" )->ToString()
           << "/" << exam->Get( "id" )->ToString()
           << "/Image/" << this->Get( "id" )->ToString() << ".dcm";

    exam->Delete();
    study->Delete();

    return stream.str();
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool Image::IsRatedBy( User* user )
  {
    this->AssertPrimaryId();
    
    // make sure the user is not null
    if( !user ) throw std::runtime_error( "Tried to get rating for null user" );

    std::map< std::string, std::string > map;
    map["user_id"] = user->Get( "id" )->ToString();
    map["image_id"] = this->Get( "id" )->ToString();
    vtkSmartPointer< Alder::Rating > rating = vtkSmartPointer< Alder::Rating >::New();
    if( !rating->Load( map ) ) return false;

    // we have found a rating, make sure it is not null
    return NULL != rating->Get( "rating" );
  }
}
