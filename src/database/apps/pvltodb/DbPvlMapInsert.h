#if !defined(DbPvlMapInsert_h)
#define DbPvlMapInsert_h
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

#include "Database.h"
#include "DbPvlMapUtil.h"
//#include "DbPvlTable.h"

namespace Isis {

class iException;

class DbPvlMapInsert {
  public:
    typedef std::vector<DbColumn>        DbColumnList;
    DbPvlMapInsert();
    DbPvlMapInsert(const std::string &tblname);
    DbPvlMapInsert(const std::string &tblname, const DbColumnList &clist);
    virtual ~DbPvlMapInsert() {}

    int size() const { return (_columns.size()); }
    std::string name() const { return (_name); }
    void setTableName(const std::string &tblname) { _name = tblname; }

    void setNull(const std::string &nullv = "NULL") { _null = nullv; }
    std::string null() const { return (_null); }

    void add(const DbColumn &column, bool replace = false);
    void add(const DbColumnList &columns, bool replace = false);
    bool exist(const std::string &name) const;
    const DbColumn &get(const std::string &name) const;
    const DbColumn &get(int nth = 0) const;
    void clear() { _columns.clear(); }

    DbStatus Insert();
    DbStatus Insert(Database db);

    DbStatus Update(const std::string &where = "");
    DbStatus Update(Database db, const std::string &where = "");

//  DbStatus Select(Database db, const std::string &where = "");
//  DbStatus Next();

    static std::string sqlQuote(const std::string &value);

  private:
    std::string _name;
    std::string _null;

    typedef DbColumnList::iterator       DbColumnIter;
    typedef DbColumnList::const_iterator DbColumnConstIter;
    DbColumnList _columns;

    DbColumnIter find(const std::string &name);
    DbColumnConstIter find(const std::string &name) const;

    DbStatus prepareInsert() const;
    DbStatus prepareUpdate(const std::string &where) const;
    DbStatus checkUpdate(const std::string &where, Database &db) const;
    DbStatus executeSql(const DbStatus &statement, Database &db,
                        bool useTransactions = true) const;

};

}  // namespace Isis
#endif

