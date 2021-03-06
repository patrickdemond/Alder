/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   Application.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/

#include "Application.h"

#include "Configuration.h"
#include "Database.h"
#include "Exam.h"
#include "Image.h"
#include "Interview.h"
#include "Modality.h"
#include "OpalService.h"
#include "Rating.h"
#include "User.h"

#include "vtkDirectory.h"
#include "vtkObjectFactory.h"
#include "vtkVariant.h"

#include <stdexcept>

namespace Alder
{
  Application* Application::Instance = NULL; // set the initial application

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  Application::Application()
  {
    this->AbortFlag = false;
    this->Config = Configuration::New();
    this->DB = Database::New();
    this->Opal = OpalService::New();
    this->ActiveUser = NULL;
    this->ActiveInterview = NULL;
    this->ActiveImage = NULL;
    this->ActiveAtlasImage = NULL;

    // populate the constructor and class name registries with all active record classes
    this->ConstructorRegistry["Exam"] = &createInstance<Exam>;
    this->ClassNameRegistry["Exam"] = typeid(Exam).name();
    this->ConstructorRegistry["Image"] = &createInstance<Image>;
    this->ClassNameRegistry["Image"] = typeid(Image).name();
    this->ConstructorRegistry["Interview"] = &createInstance<Interview>;
    this->ClassNameRegistry["Interview"] = typeid(Interview).name();
    this->ConstructorRegistry["Modality"] = &createInstance<Modality>;
    this->ClassNameRegistry["Modality"] = typeid(Modality).name();
    this->ConstructorRegistry["Rating"] = &createInstance<Rating>;
    this->ClassNameRegistry["Rating"] = typeid(Rating).name();
    this->ConstructorRegistry["User"] = &createInstance<User>;
    this->ClassNameRegistry["User"] = typeid(User).name();
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  Application::~Application()
  {
    if( NULL != this->Config )
    {
      this->Config->Delete();
      this->Config = NULL;
    }

    if( NULL != this->DB )
    {
      this->DB->Delete();
      this->DB = NULL;
    }

    if( NULL != this->Opal )
    {
      this->Opal->Delete();
      this->Opal = NULL;
    }

    if( NULL != this->ActiveUser )
    {
      this->ActiveUser->Delete();
      this->ActiveUser = NULL;
    }

    if( NULL != this->ActiveInterview )
    {
      this->ActiveInterview->Delete();
      this->ActiveInterview = NULL;
    }

    if( NULL != this->ActiveInterview )
    {
      this->ActiveInterview->Delete();
      this->ActiveInterview = NULL;
    }

    if( NULL != this->ActiveImage )
    {
      this->ActiveImage->Delete();
      this->ActiveImage = NULL;
    }

    if( NULL != this->ActiveAtlasImage )
    {
      this->ActiveAtlasImage->Delete();
      this->ActiveAtlasImage = NULL;
    }
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  // this must be implemented instead of the standard new macro since the New()
  // method is protected (ie: we do not want an instantiator new function)
  Application* Application::New()
  {
    vtkObject* ret = vtkObjectFactory::CreateInstance( "Application" );
    return ret ? static_cast< Application* >( ret ) : new Application;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  Application* Application::GetInstance()
  {
    if( NULL == Application::Instance ) Application::Instance = Application::New();
    return Application::Instance;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Application::DeleteInstance()
  {
    if( NULL != Application::Instance )
    {
      Application::Instance->Delete();
      Application::Instance = NULL;
    }
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::string Application::GetUnmangledClassName( const std::string mangledName ) const
  {
    for( auto it = this->ClassNameRegistry.begin(); it != this->ClassNameRegistry.end(); ++it )
      if( it->second == mangledName ) return it->first;
    
    throw std::runtime_error( "Tried to unmangle class name which isn't registered." );
    return ""; // this will never happen because of the throw
  }
  
  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool Application::ReadConfiguration( std::string filename )
  {
    // make sure the file exists
    ifstream ifile( filename.c_str() );
    if( !ifile ) return false;
    return this->Config->Read( filename );
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool Application::ConnectToDatabase()
  {
    std::string name = this->Config->GetValue( "Database", "Name" );
    std::string user = this->Config->GetValue( "Database", "Username" );
    std::string pass = this->Config->GetValue( "Database", "Password" );
    std::string host = this->Config->GetValue( "Database", "Host" );
    std::string port = this->Config->GetValue( "Database", "Port" );

    // make sure the database and user names are provided
    if( 0 == name.length() || 0 == user.length() )
    {
      cerr << "ERROR: database name and database user name must be included in "
           << "configuration file" << endl;
      return false;
    }

    // defaint host and port
    if( 0 == host.length() ) host = "localhost";
    if( 0 == port.length() ) port = "3306";

    return this->DB->Connect( name, user, pass, host, vtkVariant( port ).ToInt() );
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Application::SetupOpalService()
  {
    // setup the opal service
    std::string user = this->Config->GetValue( "Opal", "Username" );
    std::string pass = this->Config->GetValue( "Opal", "Password" );
    std::string host = this->Config->GetValue( "Opal", "Host" );
    std::string port = this->Config->GetValue( "Opal", "Port" );
    std::string timeout = this->Config->GetValue( "Opal", "Timeout" );
    this->Opal->Setup( user, pass, host );
    if( 0 < port.length() ) this->Opal->SetPort( vtkVariant( port ).ToInt() );
    if( 0 < timeout.length() ) this->Opal->SetTimeout( vtkVariant( timeout ).ToInt() );
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Application::ResetApplication()
  {
    this->SetActiveUser( NULL );
    this->SetActiveInterview( NULL );
    this->SetActiveImage( NULL );
    this->SetActiveAtlasImage( NULL );
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Application::SetActiveUser( User *user )
  {
    if( user != this->ActiveUser )
    {
      if( this->ActiveUser ) this->ActiveUser->UnRegister( this );
      this->ActiveUser = user;
      if( this->ActiveUser ) 
      {
        this->ActiveUser->Register( this );

        // get the user's last active interview
        vtkSmartPointer< Interview > interview;
        if( this->ActiveUser->GetRecord( interview ) ) this->SetActiveInterview( interview );
      }
      this->Modified();
      this->InvokeEvent( Application::ActiveUserEvent );
    }
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Application::SetActiveInterview( Interview *interview )
  {
    if( interview != this->ActiveInterview )
    {
      if( this->ActiveInterview )
      {
        this->ActiveInterview->UnRegister( this );
      }

      this->ActiveInterview = interview;

      if( this->ActiveInterview )
      {
        this->ActiveInterview->Register( this );

        std::string lastId;
        if( this->ActiveImage ) lastId = this->ActiveImage->Get( "Id" ).ToString();
        std::string similar = interview->GetSimilarImage( lastId );
        if( !similar.empty() )
        {
          vtkSmartPointer<Image> image = vtkSmartPointer<Image>::New();
          image->Load( "Id", similar );
          this->SetActiveImage( image );
        }
        else
        {
          this->SetActiveImage( NULL );
        }  
        this->SetActiveAtlasImage( NULL );
      }

      // if there is an active user, save the active interview
      if( this->ActiveUser )
      {
        if( interview ) this->ActiveUser->Set( "InterviewId", interview->Get( "Id" ).ToInt() );
        else this->ActiveUser->SetNull( "InterviewId" );
        this->ActiveUser->Save();
      }
      this->Modified();
      this->InvokeEvent( Application::ActiveInterviewEvent );
    }
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Application::SetActiveImage( Image* image )
  {
    vtkSetObjectBodyMacro( ActiveImage, Image, image);
    this->InvokeEvent( Application::ActiveImageEvent );
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Application::SetActiveAtlasImage( Image* image )
  {
    vtkSetObjectBodyMacro( ActiveAtlasImage, Image, image);
    this->InvokeEvent( Application::ActiveAtlasImageEvent );
  }
  
  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Application::UpdateActiveInterviewImageData()
  {
    if( this->ActiveInterview )
    {
      this->ActiveInterview->UpdateImageData();
      this->InvokeEvent( Application::ActiveInterviewUpdateImageDataEvent );
    }  
  }
}
