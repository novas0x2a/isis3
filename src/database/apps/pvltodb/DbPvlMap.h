#if !defined(DbPvlMap_h)
#define DbPvlMap_h
/**                                                                       
 * @file                                                                  
 * $Revision: 1.1 $
 * $Date: 2007/03/30 17:19:48 $
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
#include <iostream>
#include <stack>
#include <functional>  

#include "Pvl.h"
#include "CollectorMap.h"
#include "Database.h"
#include "DbPvlMapUtil.h"
#include "MapFunctor.h"
#include "DbPvlTable.h"

namespace Isis {

class DbProfile;
class iException;

class DbPvlMap {
  public:
    DbPvlMap();
    DbPvlMap(const std::string &dbmap);
    DbPvlMap(const std::string &dbmap, const std::string &pvlsource);
    virtual ~DbPvlMap() {
      clearFunctions(_functions);
    }

    void setDbMap(const std::string &dbmap);
    void setPvlSource(const std::string &pvlsource);
    void addTableMap(PvlObject &table, bool replace = false);

    int size() const { return (_tables.size()); }
    bool hasTable(const std::string &name) const;
    const DbPvlTable &getTable(const std::string &name) const;
    const DbPvlTable &getTable(int nth = 0) const;

    void addVariable(const std::string &name, const std::string&value);
    void addVariable(const DbProfile &profile);
    const Variables &getVariables() const { return (_variables); }

    DbResult Insert();
    DbResult Insert(Database db);
    DbResult Insert(const std::string &table);
    DbResult Insert(const std::string &table, Database db);

  private:
    //  Do not allow copying of this class
    DbPvlMap(const DbPvlMap &NoNo);
    DbPvlMap &operator=(const DbPvlMap &NoNo);

    Pvl _dbmap;
    Pvl _source;
    Variables _variables;
    FunctionList _functions;

    typedef std::vector<DbPvlTable> DbPvlTableList;
    typedef DbPvlTableList::iterator       DbPvlTableIter;
    typedef DbPvlTableList::const_iterator DbPvlTableConstIter;
    DbPvlTableList _tables;

    void resolveMap(Pvl &maproot, DbPvlTableList &tables);
    DbPvlTableIter find(const std::string &name);
    DbPvlTableConstIter find(const std::string &name) const;

};

}  // namespace Isis
#endif
