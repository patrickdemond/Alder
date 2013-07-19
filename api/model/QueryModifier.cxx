/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QueryModifier.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/

#include "QueryModifier.h"

#include "Application.h"
#include "Database.h"

#include "vtkAlderMySQLQuery.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"

#include <sstream>
#include <vector>

namespace Alder
{
  vtkStandardNewMacro( QueryModifier );

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  QueryModifier::QueryModifier()
  {
    this->LimitCount = 0;
    this->LimitOffset = 0;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void QueryModifier::Where(
    std::string column, std::string oper, vtkVariant value, bool format, bool logicalOr )
  {
    WhereParameter p;
    p.column = column;
    p.oper = oper;
    p.value = value;
    p.format = format;
    p.logicalOr = logicalOr;
    this->WhereList.push_back( p );
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void QueryModifier::WhereBracket( bool open, bool logicalOr )
  {
    WhereParameter p;
    p.bracket = open ? QueryModifier::OPEN : QueryModifier::CLOSE;
    p.logicalOr = logicalOr;
    this->WhereList.push_back( p );
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void QueryModifier::Group( std::string column )
  {
    this->GroupList.push_back( column );
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void QueryModifier::Order( std::string column, bool desc )
  {
    this->OrderList[column] = desc;
  } 

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void QueryModifier::Limit( int count, int offset )
  {
    this->LimitCount = count;
    this->LimitOffset = offset;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::string QueryModifier::GetSql( bool appending )
  {
    std::string retVal;
    retVal  = this->GetWhere( appending );
    retVal += this->GetGroup();
    retVal += this->GetOrder();
    retVal += this->GetLimit();
    return retVal;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::string QueryModifier::GetWhere( bool appending )
  {
    bool firstItem = true, lastOpenBracket = false;
    std::string statement;
    std::stringstream stream;
    std::vector<WhereParameter>::iterator it;

    // loop through each where parameter
    for( it = this->WhereList.begin(); it != this->WhereList.end(); ++it )
    {
      if( QueryModifier::NONE != it->bracket )
      {
        statement = ( QueryModifier::OPEN == it->bracket ? "(" : ")" );
      }
      else
      {
        // format the value (unless specified not to)
        vtkVariant value = it->value;
        if( it->format && value.IsValid() )
        {
          // we need a query object to escape the sql :(
          vtkSmartPointer<vtkAlderMySQLQuery> query = Application::GetInstance()->GetDB()->GetQuery();
          value = query->EscapeString( value.ToString() );
        }

        if( !value.IsValid() )
        {
          statement = it->column;
          statement += ( 0 == it->oper.compare( "=" ) ? " IS NULL" : " IS NOT NULL" );
        }
        else
        {
          statement = it->column;
          statement += " ";
          statement += it->oper;
          statement += " ";
          statement += value.ToString();
        }
      }

      // add the logical connecting statement if necessary
      if( ( !firstItem || appending ) &&
          QueryModifier::CLOSE != it->bracket && !lastOpenBracket )
        stream << ( it->logicalOr ? " OR" : " AND" );

      stream << " " << statement;

      // keep track of whether this statement is an open bracket
      if( QueryModifier::OPEN == it->bracket ) lastOpenBracket = true;
      if( firstItem ) firstItem = false;
    }

    // add "WHERE" at the front, if necessary
    std::string sql = stream.str();
    if( !appending || 0 < sql.length() ) sql = "WHERE" + sql;

    return sql;
  }
  
  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::string QueryModifier::GetGroup()
  {
    std::stringstream sql;
    bool first = true;
    std::vector<std::string>::iterator it;

    for( it = this->GroupList.begin(); it != this->GroupList.end(); ++it )
    {
      sql << ( first ? "GROUP BY " : ", " ) << *it;
      if( first ) first = false;
    }

    return sql.str();
  }
  
  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::string QueryModifier::GetOrder()
  {
    std::stringstream sql;
    bool first = true;
    std::map<std::string,bool>::iterator it;

    for( it = this->OrderList.begin(); it != this->OrderList.end(); ++it )
    {
      sql << ( first ? "ORDER BY " : ", " ) << it->first << " " << ( it->second ? "DESC" : "" );
      if( first ) first = false;
    }

    return sql.str();
  }
  
  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::string QueryModifier::GetLimit()
  {
    std::stringstream sql;
    if( 0 < this->LimitCount )
      sql << "LIMIT " << this->LimitCount << " OFFSET " << this->LimitOffset;

    return sql.str();
  }
}
