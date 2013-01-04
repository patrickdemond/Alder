/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   Image.h
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/

/**
 * @class Image
 * @namespace Alder
 * 
 * @author Patrick Emond <emondpd@mcmaster.ca>
 * @author Dean Inglis <inglisd@mcmaster.ca>
 * 
 * @brief An active record for the Image table
 */

#ifndef __Image_h
#define __Image_h

#include "ActiveRecordWithFile.h"

/**
 * @addtogroup Alder
 * @{
 */

namespace Alder
{
  class User;
  class Image : public ActiveRecordWithFile
  {
  public:
    static Image *New();
    vtkTypeMacro( Image, ActiveRecordWithFile );
    std::string GetName() { return "Image"; }

  protected:
    Image() {}
    ~Image() {}

  private:
    Image( const Image& ); // Not implemented
    void operator=( const Image& ); // Not implemented
  };
}

/** @} end of doxygen group */

#endif
