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

#include <ActiveRecord.h>

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
     * @return std::string The InterviewId/ExamId/ImageId code
     * @throw std::runtime_error
     */
    virtual std::string GetCode();

    /**
     * Get the full path to where the image associated with this record belongs.
     * @return std::string The path to this image (excludes file name)
     * @throw std::runtime_error
     */
    std::string GetFilePath();

    /**
     * Create the path to the file name that this record represents (including path and provided
     * suffix) and returns the result (with full path, file name and suffix).
     * NOTE: this method does not depend on the file already existing, it simply uses
     * the image's path and the provided extension to create an empty file.
     */
    std::string CreateFile( std::string const &suffix );

    /**
     * Once the file is written to the disk this method validates it.  It will unzip gzipped files
     * and if the file is empty or unreadable it will delete the file.
     * @return bool Whether the file is valid
     */
    bool ValidateFile();

    /**
     * Get the file name that this record represents (including path).
     * NOTE: this method depends on the file already existing: if it doesn't already
     * exist, an exception is thrown.
     * @return std::string Full path and file name of this image
     * @throw std::runtime_error
     */
    std::string GetFileName();

    /**
     * Get whether a particular user has rated this image.
     * @param User The user to check against
     * @return bool Whether a user has rated this image
     * @throw std::runtime_error
     */
    bool IsRatedBy( User* user );

    /**
     * Is this a dicom image? Determined via the modality
     * record associated with this image's parent exam.
     * @return bool Whether this is a dicom image
     */
    bool IsDICOM();

    /**
     * Get a DICOM tag value. Accepted tag names are: 
     *   AcquisitionDateTime - 0x0008, 0x002a
     *   SeriesNumber        - 0x0020,0x0011
     *   PatientsName        - 0x0010, 0x0010
     *   Laterality          - 0x0020, 0x0060
     * No checking is done to ensure this is a dicom image: use
     * IsDICOM() method to check first.
     * @param tagName Description of the dicom tag
     * @return std::string Value of a dicom tag
     * @throw std::runtime_error
     */
    std::string GetDICOMTag( std::string const &tagName );

    /**
     * Get the acquisition date time.  Works only for dicom images.
     * No checking is done to ensure this is a dicom image: use
     * IsDICOM() method to check first.
     * @return std::string Value of the dicom AcquistionDateTime tag
     */
    std::string GetDICOMAcquisitionDateTime();

    /**
     * Get the number of rows, columns and frames of a dicom image.
     * No checking is done to ensure this is a dicom image: use
     * IsDICOM() method to check first.
     * @return std::vector<int> Dimensions in x, y, z: x, y only if 2D
     * @throw std::runtime_error
     */
    std::vector<int> GetDICOMDimensions();

    /**
     * Set the laterality of the image's parent exam from its dicom tag if available.
     * No checking is done to ensure this is a dicom image: use
     * IsDICOM() method to check first.
     */
    void SetLateralityFromDICOM();

    /**
     * Anonymize a dicom image by clearing the PatientsName tag.
     * No checking is done to ensure this is a dicom image: use
     * IsDICOM() method to check first.
     * @return bool Whether the file was anonymized
     * @throw std::runtime_error
     */
    bool AnonymizeDICOM();

    /**
     * Erase the rectangular Patient Name box in Hologic DEXA images.
     * This method calls AnonymizeDICOM() after cleaning the actual
     * image.
     * No checking is done to ensure this is a dicom image: use
     * IsDICOM() method to check first.
     * @return bool Success status of the clean
     * @throw std::runtime_error
     */
    bool CleanHologicDICOM();

    /**
     * Returns the neighbouring interview in UId/VisitDate order.
     * A rating must be provided since an image may have more than one rating.
     * @param rating The rating of the atlas image that matches this one
     * @param forward The order ASC (forward=true) or DESC to search by Interview.UId 
     * @return vtkSmartPointer<Image> Neigbouring atlas image
     * @throw std::runtime_error
     */
    vtkSmartPointer<Image> GetNeighbourAtlasImage( int const &rating, bool const &forward );
    vtkSmartPointer<Image> GetNextAtlasImage( int const &rating )
    { return this->GetNeighbourAtlasImage( rating, true ); }
    vtkSmartPointer<Image> GetPreviousAtlasImage( int const &rating )
    { return this->GetNeighbourAtlasImage( rating, false ); }

    /**
     * Returns an atlas image matching the current image type.
     * @param rating The rating of the atlas image that matches this one
     * @return vtkSmartPointer<Image> Atlas image matching this image
     * @throw std::runtime_error
     */
    vtkSmartPointer<Image> GetAtlasImage( int const &rating );

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
