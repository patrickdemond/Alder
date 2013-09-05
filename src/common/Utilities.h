/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   Utilities.h
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/

/**
 * @class Utilities
 *
 * @author Patrick Emond <emondpd AT mcmaster DOT ca>
 * @author Dean Inglis <inglisd AT mcmaster DOT ca>
 *
 * @brief Utilities class includes typedefs, macros, global functions, etc.
 *
 * All methods in this class are static.  There is no need to instantiate
 * and instance of the Utilities class.  The class implementation file
 * exists solely for the creation of a library.
 *
 */
#ifndef __Utilities_h
#define __Utilities_h

#include <algorithm>
#include <base64.h>
#include <cctype>
#include <fstream>
//#include <json/reader.h>
#include <sha.h>
#include <sstream>
#include <sys/stat.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include <AlderConfig.h>

/**
 * @addtogroup Alder
 * @{
 */

namespace Alder
{
  class Utilities
  {
  public:

    inline static void hashString( std::string input, std::string &output )
    {
      input += ALDER_SALT_STRING;
      output = "";

      CryptoPP::SHA256 hash;
      CryptoPP::StringSource foo(
        input.c_str(),
        true,
        new CryptoPP::HashFilter( hash,
          new CryptoPP::Base64Encoder(
            new CryptoPP::StringSink( output ) ) ) );
    }

    inline static void base64String( std::string input, std::string &output )
    {
      output = "";

      CryptoPP::StringSource foo(
        input.c_str(),
        true,
        new CryptoPP::Base64Encoder(
          new CryptoPP::StringSink( output ) ) );
    }

    inline static size_t writePointerToFile( void *ptr, size_t size, size_t nmemb, FILE *stream )
    {
      size_t written = fwrite( ptr, size, nmemb, stream );
      return written;
    }

    inline static size_t writePointerToString( void *ptr, size_t size, size_t count, void *stream )
    {
      ( (std::string*) stream )->append( (char*) ptr, 0, size * count );
      return size * count;
    }

    inline static std::string exec( std::string command )
    {
      FILE* pipe = popen( command.c_str(), "r" );
      if( !pipe ) return "ERROR";
      char buffer[128];
      std::string result = "";
      while( !feof( pipe ) ) if( fgets( buffer, 128, pipe ) != NULL ) result += buffer;
      pclose( pipe );
      return result;
    } 

    inline static std::string getTime( std::string format )
    {
      char buffer[256];
      time_t rawtime;
      time( &rawtime );
      strftime( buffer, 256, format.c_str(), localtime( &rawtime ) );
      return std::string( buffer );
    }

    inline static std::string toLower( std::string str )
    {
      std::string returnString = str;
      std::transform( str.begin(), str.end(), returnString.begin(), tolower );
      return returnString;
    }

    inline static std::string toUpper( std::string str )
    {
      std::string returnString = str;
      std::transform( str.begin(), str.end(), returnString.begin(), toupper );
      return returnString;
    }

    inline static bool fileExists( std::string filename )
    {
      if( filename.empty() ) return false;
      return access( filename.c_str(), R_OK ) == 0;
    }

    inline static std::string getFileExtension( std::string filename )
    {
      std::string::size_type dot_pos = filename.rfind(".");
      std::string extension = (dot_pos == std::string::npos) ? "" :
        filename.substr( dot_pos );
      return extension;  
    }

    inline static std::string getFilenamePath( std::string filename )
    {
      std::string::size_type slash_pos = filename.rfind("/");
      if( slash_pos != std::string::npos )
      {
        std::string path = filename.substr( 0, slash_pos );
        if( path.size() == 2 && path[1] == ':' )
        {
          return path + '/';
        }
        if( path.size() == 0 )
        {
          return "/";
        }
        return path;
      }
      else
      {
        return "";
      }
    }

    inline static unsigned long getFileLength( std::string filename )
    {
      struct stat fs;
      return 0 != stat( filename.c_str(), &fs ) ? 0 : static_cast<unsigned long>( fs.st_size );
    }
    
    inline static std::string getFilenameName( std::string filename )
    {
      std::string::size_type slash_pos = filename.find_last_of("/");
      if( slash_pos != std::string::npos )
      {
        return filename.substr( slash_pos + 1 );
      }
      else
      {
        return filename;
      }
    }

    inline static std::vector< std::string > explode( std::string str, std::string separator )
    {
      std::vector< std::string > results;
      int found = str.find_first_of( separator );
      while( found != std::string::npos )
      {
        if( found > 0 ) results.push_back( str.substr( 0, found ) );
        str = str.substr( found + 1 );
        found = str.find_first_of( separator );
      }
      if( str.length() > 0 ) results.push_back( str );
      return results;
    }

  protected:
    Utilities() {}
    ~Utilities() {}

  };
}

#endif // __Utilities_h
