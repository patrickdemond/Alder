/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   Exam.cxx
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/
#include "Exam.h"

#include "Image.h"
#include "Utilities.h"

#include "vtkObjectFactory.h"

namespace Alder
{
  vtkStandardNewMacro( Exam );

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
