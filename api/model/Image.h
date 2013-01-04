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

#include "ActiveRecord.h"

/**
 * @addtogroup Alder
 * @{
 */

namespace Alder
{
  class User;
  class Image : public ActiveRecord
  {
  public:
    static Image *New();
    vtkTypeMacro( Image, ActiveRecord );
    std::string GetName() { return "Image"; }

    /**
     * Get the full path to where the image associated with this record belongs.
     */
    std::string GetFilePath();

    /**
     * Get the file name that this record represents (including path)
     * NOTE: this method depends on the file already existing, if it doesn't already
     * exist it will throw an exception
     */
    std::string GetFileName();

    /**
     * Get whether a particular user has rated this image
     */
    bool IsRatedBy( User* user );

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
