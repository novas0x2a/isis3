#if !defined(DbPvlMapUtil_h)
#define DbPvlMapUtil_h
/**                                                                       
 * @file                                                                  
 * $Revision: 1.2 $
 * $Date: 2007/10/31 20:52:04 $
 * 
 *   Unless noted otherwise, the portions of Isis written by the USGS are 
 *   public domain. See individual third-party library and package descriptions 
 *   for intellectual property information, user agreements, and related  
 *   information.                                                         
 *                                                                        
 *   Although Isis has been used by the USGS, no warranty, expressed or   
 *   implied, is made by the USGS as to the accuracy and functioning of such 
 *   software and related material nor shall the fact of distribution     
 *   constitute any such warranty, and no responsibility is assumed by the
 *   USGS in connection therewith.                                        
 *                                                                        
 *   For additional information, launch                                   
 *   $ISISROOT/doc//documents/Disclaimers/Disclaimers.html                
 *   in a browser or see the Privacy &amp; Disclaimers page on the Isis website,
 *   http://isis.astrogeology.usgs.gov, and the USGS privacy and disclaimers on
 *   http://www.usgs.gov/privacy.html.                                    
 */                                                                       

#include <string>

#include "CollectorMap.h"
#include "Pvl.h"
#include "MapFunctor.h"

namespace Isis {
/** Define the container for the variable key references */
typedef CollectorMap<std::string, std::string, NoCaseStringCompare> Variables;

class DbResource {
  public:
    DbResource(Pvl &source, Variables &vars, FunctionList &funcs) : 
               _source(source), _vars(vars), _functors(funcs) { }

    bool exists(const std::string &var) const { return (_vars.exists(var)); }
    std::string get(const std::string &var) const { return (_vars.get(var)); }
    void add(const std::string &key, const std::string &value) {
      _vars.add(key,value);
    }

    Pvl &getSource() { return (_source); }
    const Variables &getVariables() const { return (_vars); }
    MapFunctor *getFunction(const std::string &name) {
      if (_functors.exists(name)) { return (_functors.get(name)); }
      return (0);
    }

    protected:
      Pvl           &_source;
      Variables     &_vars;
      FunctionList  &_functors;
};


struct DbColumn {
  DbColumn() : _name(), _value() { }
  DbColumn(const std::string &colname) : _name(colname), _value() { }
  DbColumn(const std::string &colname, const std::string &val) : _name(colname),
                                                                 _value(val) { }

  bool isValid() const { return (!_name.empty()); }
  bool isMissing() const { return (_value.empty()); }

  std::string name() const { return (_name); }
  std::string value() const { return (_value); }
  void setValue(const std::string &value) { _value = value; }

  std::string bind() const { 
    return(bind(name()));
  }

  static std::string bind(const std::string &name) {
    return(std::string(":" + name));
  }

  private:
    std::string _name;
    std::string _value;
};


struct DbStatus {
  DbStatus() : table(), columns(0), sql(), status(-1), rowsAffected(0), 
               lastError() { }
  DbStatus(const std::string &name) : 
           table(name), columns(0), operation("none"), sql(""), status(-1), 
           rowsAffected(0), lastError() { }
  DbStatus(const std::string &name, int ncols, const std::string &sqlOp, 
           const std::string &sqlStmt) : table(name), columns(ncols),
           operation(sqlOp),  sql(sqlStmt), status(-1),  rowsAffected(0), 
           lastError() { }

  std::string table;     //!< Name of table
  int columns;           //!< Number columns in operation
  std::string operation; //!< type of operation performed (UPDATE, INSERT)
  std::string sql;       //!< Prepared or executed SQL statement
  int status;            //!< Query status -1=unexecuted, 0=OK, 1=failed
  int rowsAffected;      //!< Number rows the statement affected
  std::string lastError; //!< Last textual DB error
};

class DbResult {
  public:
    typedef std::vector<DbStatus> ResultSet;

    DbResult() : _dbStats() { }
    virtual ~DbResult() { }

    int size() const { return (_dbStats.size()); }
    void add(const DbStatus &stats) { _dbStats.push_back(stats); }
    const DbStatus &get(int nth = 0) { return (_dbStats[nth]); }

  private:
    ResultSet _dbStats;
};


}  // namespace Isis
#endif

