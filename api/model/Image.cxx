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
#include "Study.h"
#include "Utilities.h"

#include "vtkObjectFactory.h"

namespace Alder
{
  vtkStandardNewMacro( Image );

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
}
