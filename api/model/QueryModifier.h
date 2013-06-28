/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QueryModifier.h
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/

/**
 * @class QueryModifier
 * @namespace Alder
 * 
 * @author Patrick Emond <emondpd@mcmaster.ca>
 * @author Dean Inglis <inglisd@mcmaster.ca>
 * 
 * @brief A class used to modifier a database query.
 * 
 * QueryModifier provides a way to modify a database query including where, limit and offset keywords.
 * This object can be passed to the ActiveRecord class when querying records.
 */

#ifndef __QueryModifier_h
#define __QueryModifier_h

#include "ModelObject.h"

#include "vtkVariant.h"

#include <map>
#include <vector>

/**
 * @addtogroup Alder
 * @{
 */

namespace Alder
{
  class QueryModifier : public ModelObject
  {
    enum BracketType
    {
      NONE,
      OPEN,
      CLOSE
    };

    struct WhereParameter
    {
      WhereParameter() : format( true ), logicalOr( false ), bracket( QueryModifier::NONE ) {}
      std::string column;
      std::string oper;
      vtkVariant value;
      bool format;
      bool logicalOr;
      BracketType bracket;
    };

  public:
    static QueryModifier *New();
    vtkTypeMacro( QueryModifier, ModelObject );

    /**
     * Add a where statement to the modifier.
     * This method appends where clauses onto the end of already existing where clauses.
     */    
    virtual void Where(
      std::string column, std::string oper, vtkVariant value, bool format = true, bool logicalOr = false );

    /**
     * Add where statement which will be "or" combined to the modifier.
     * This is a convenience method which makes where() calls more readable.
     */
    virtual void OrWhere( std::string column, std::string oper, vtkVariant value, bool format = true )
    { this->Where( column, oper, value, format, true ); }

    /**
     * Add a bracket to a where statement
     */
    virtual void WhereBracket( bool open, bool logicalOr = false );

    /**
     * Add a group by statement to the modifier.
     * This method appends group by clauses onto the end of already existing group by clauses.
     */
    virtual void Group( std::string column );

    /**
     * Adds an order statement to the modifier.
     * This method appends order clauses onto the end of already existing order clauses.
     */
    virtual void Order( std::string column, bool desc = false );

    /**
     * Add order descending statement to the modifier.
     * This is a convenience method which makes order() calls more readable.
     */
    virtual void OrderDesc( std::string column )
    { this->Order( column, true ); }

    /**
     * Sets a limit to how many rows are returned.
     * This method sets the total number of rows and offset to begin selecting by.
     */
    virtual void Limit( int count, int offset = 0 );

    /**
     * Returns the modifier as an SQL statement (same as calling each individual get_*() method.
     */
    virtual std::string GetSql( bool appending = false );

    /**
     * Returns an SQL where statement.
     * This method should only be called by an record class and only after all modifications
     * have been set.
     */
    virtual std::string GetWhere( bool appending = false );
    
    /**
     * Returns an SQL group statement.
     * This method should only be called by an record class and only after all modifications
     * have been set.
     */
    virtual std::string GetGroup();
    
    /**
     * Returns an SQL order statement.
     * This method should only be called by an record class and only after all modifications
     * have been set.
     */
    virtual std::string GetOrder();
    
    /**
     * Returns an SQL limit statement.
     * This method should only be called by an record class and only after all modifications
     * have been set.
     */
    virtual std::string GetLimit();

  protected:
    QueryModifier();
    ~QueryModifier() {}

    std::vector<WhereParameter> WhereList;
    std::map<std::string,bool> OrderList;
    std::vector<std::string> GroupList;
    int LimitCount;
    int LimitOffset;

  private:
    QueryModifier( const QueryModifier& ); // Not implemented
    void operator=( const QueryModifier& ); // Not implemented
  };
}

/** @} end of doxygen group */

#endif
