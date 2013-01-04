/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   ActiveRecordWithFile.h
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/

/**
 * @class ActiveRecordWithFile
 * @namespace Alder
 * 
 * @author Patrick Emond <emondpd@mcmaster.ca>
 * @author Dean Inglis <inglisd@mcmaster.ca>
 * 
 * @brief An active record which has a file associated with it.
 */

#ifndef __ActiveRecordWithFile_h
#define __ActiveRecordWithFile_h

#include "ActiveRecord.h"

/**
 * @addtogroup Alder
 * @{
 */

namespace Alder
{
  class User;
  class ActiveRecordWithFile : public ActiveRecord
  {
  public:
    vtkTypeMacro( ActiveRecordWithFile, ActiveRecord );

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
    ActiveRecordWithFile() {}
    ~ActiveRecordWithFile() {}

  private:
    ActiveRecordWithFile( const ActiveRecordWithFile& ); // Not implemented
    void operator=( const ActiveRecordWithFile& ); // Not implemented
  };
}

/** @} end of doxygen group */

#endif
