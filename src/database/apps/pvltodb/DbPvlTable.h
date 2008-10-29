#if !defined(DbPvlTable_h)
#define DbPvlTable_h
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
#include "DbPvlMapUtil.h"
#include "DbPvlMapInsert.h"
#include "CollectorMap.h"
#include "Resolver.h"

namespace Isis {

class DbPvlTable {
  public:
    DbPvlTable();
    DbPvlTable(const std::string &name);
    DbPvlTable(PvlObject &table);
    virtual ~DbPvlTable() { }

    bool isValid() const { return (_name.empty()); }
    std::string name() const { return (_name); }
    int size() const { return (_keys.size()); }

    DbStatus Insert(Resolver &parser);
    DbStatus Insert(Resolver &parser, Database db);

  private:
    typedef DbPvlMapInsert::DbColumnList ColumnSet;

    /** Structure containing update information */
    struct UpdateInfo {
      UpdateInfo() : primaryKey(), whereClause(){}
      bool isValid() const { return (!primaryKey.empty()); }
      std::string primaryKey;
      std::string whereClause;
    };

    typedef std::vector<PvlKeyword>    PvlMapKeys;
    typedef PvlMapKeys::const_iterator PvlMapKeysConstIter;
    typedef PvlMapKeys::iterator       PvlMapKeysIter;

    std::string _name;
    PvlMapKeys  _keys;
    UpdateInfo  _update;

    void init(PvlObject &table);
    void processKeys(Resolver &parser, const PvlMapKeys &keys, 
                     ColumnSet &columns, PvlMapKeys &missing) const;

    void build(Resolver &parser, DbPvlMapInsert &inserter) const;
    std::string buildWhere(Resolver &parser, const UpdateInfo &update) const;
    DbStatus resolve(Resolver &parser) const;
    DbStatus resolve(Resolver &parser, Database db) const;
};

}

#endif
