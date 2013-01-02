/*=========================================================================

  Program:  Alder (CLSA Ultrasound Image Viewer)
  Module:   Cineloop.cxx
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/
#include "Cineloop.h"

#include "Configuration.h"
#include "Exam.h"
#include "Study.h"
#include "Utilities.h"

#include "vtkObjectFactory.h"

namespace Alder
{
  vtkStandardNewMacro( Cineloop );

  std::string Cineloop::GetFileName()
  {
    this->AssertPrimaryId();

    // get the study and exam for this record
    Exam *exam = Exam::SafeDownCast( this->GetRecord( "Exam" ) );
    Study *study = Study::SafeDownCast( exam->GetRecord( "Study" ) );

    std::stringstream stream;
    // start with the base image directory
    stream << Application::GetInstance()->GetConfig()->GetValue( "Path", "ImageData" )
           << "/" << study->Get( "UId" ).ToString()
           << "/" << exam->Get( "Id" ).ToString()
           << "/Cineloop/" << this->Get( "Id" ).ToString() << ".dcm";

    exam->Delete();
    study->Delete();

    return stream.str();
  }
}
