/*=========================================================================

  Program:  Alder (CLSA Ultrasound Image Viewer)
  Module:   hash.cxx
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/
//
// .SECTION Description
// Encrypts a string using the whirlpool hash algorithm
//

#include <iostream>
#include <sha.h>
#include <base64.h>
#include "Utilities.h"

using namespace std;

// main function
int main( int argc, char** argv )
{
  std::string message = ALDER_SALT_STRING;
  if( 2 == argc )
  { // message provided from command line
    message += argv[1];
  }
  else if( 1 == argc )
  { // prompt user for message
    std::string temp;
    cout << "Provide message to hash: ";
    cin >> temp;
    message += temp;
  }
  else
  { // invalid number of arguments, bail
    cout << "Usage: hash <string to encrypt>" << endl;
    return EXIT_FAILURE;
  }

  // build the hash and output it
  std::string digest;
  CryptoPP::SHA256 hash;
  CryptoPP::StringSource foo1(
    message.c_str(),
    true,
    new CryptoPP::HashFilter( hash,
      new CryptoPP::Base64Encoder(
        new CryptoPP::StringSink( digest ) ) ) );
  cout << digest;

  // return the result of the executed application
  return EXIT_SUCCESS;
}
