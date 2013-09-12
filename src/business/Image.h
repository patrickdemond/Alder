/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   Image.h
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/

/**
 * @class Image
 * @namespace Alder
 * 
 * @author Patrick Emond <emondpd AT mcmaster DOT ca>
 * @author Dean Inglis <inglisd AT mcmaster DOT ca>
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
    std::string GetName() const { return "Image"; }

    /**
     * Returns the image's code (used to determine the image's path in the image data directory)
     */
    virtual std::string GetCode();

    /**
     * Get the full path to where the image associated with this record belongs.
     */
    std::string GetFilePath();

    /**
     * Create the path to the file name that this record represents (including path and provided suffix)
     * and returns the result (with full path, file name and suffix)
     * NOTE: this method does not depend on the file already existing, it simply uses
     * the image's path and the provided extension to create an empty file.
     */
    std::string CreateFile( const std::string suffix );

    /**
     * Once the file is written to the disk this method validates it.  It will unzip gzipped files
     * and if the file is empty or unreadable it will delete the file.
     * @return bool Whether the file is valid
     */
    bool ValidateFile();

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

    /**
     *  Is this a dicom image?
     */
    bool IsDICOM();

    /**
     * Get arbitrary DICOM tag value.  Works only for dicom images.
     */
    std::string GetDICOMTag( const std::string tagName );

    /**
     * Get the acquisition date time.  Works only for dicom images.
     */
    std::string GetDICOMAcquisitionDateTime();

    /**
     * Get the number of rows, columns and frames.  Works only for dicom images.
     */
    std::vector<int> GetDICOMDimensions();

    /**
     * Anonymize a dicom image by clearing the PatientsName tag.
     */
    void AnonymizeDICOM();

    /**
     * Returns the neighbouring interview in UId/VisitDate order.
     * The rating must be provided since an image may have more than one rating
     */
    vtkSmartPointer<Image> GetNeighbourAtlasImage( const int rating, const bool forward );
    vtkSmartPointer<Image> GetNextAtlasImage( const int rating )
    { return this->GetNeighbourAtlasImage( rating, true ); }
    vtkSmartPointer<Image> GetPreviousAtlasImage( const int rating )
    { return this->GetNeighbourAtlasImage( rating, false ); }

    /**
     * Returns an atlas image matching the current image type
     */
    vtkSmartPointer<Image> GetAtlasImage( const int rating );

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
