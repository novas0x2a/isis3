/**                                                                       
 * @file                                                                  
 * $Revision: 1.2 $
 * $Date: 2007/06/06 00:29:28 $
 * 
 *   Unless noted otherwise, the portions of Isis written by the USGS are public
 *   domain. See individual third-party library and package descriptions for 
 *   intellectual property information,user agreements, and related information.
 *                                                                        
 *   Although Isis has been used by the USGS, no warranty, expressed or implied,
 *   is made by the USGS as to the accuracy and functioning of such software 
 *   and related material nor shall the fact of distribution constitute any such 
 *   warranty, and no responsibility is assumed by the USGS in connection 
 *   therewith.                                                           
 *                                                                        
 *   For additional information, launch                                   
 *   $ISISROOT/doc//documents/Disclaimers/Disclaimers.html in a browser or see 
 *   the Privacy &amp; Disclaimers page on the Isis website,              
 *   http://isis.astrogeology.usgs.gov, and the USGS privacy and disclaimers on
 *   http://www.usgs.gov/privacy.html.                                    
 */                                                                       

#include <string>
#include <vector>
#include <sstream>
#include <iostream>

using namespace std;

#include "DbPvlMap.h"
#include "DbProfile.h"
#include "DbPvlTable.h"
#include "PvlObject.h"
#include "Resolver.h"
#include "iString.h"
#include "iException.h"

namespace Isis {

DbPvlMap::DbPvlMap() : _dbmap(), _source(), _variables(), 
                       _functions(getFunctionList()), _tables() { }

DbPvlMap::DbPvlMap(const std::string &dbmap) : _dbmap(dbmap), _source(),
                                               _variables(), 
                                               _functions(getFunctionList()),
                                               _tables() {
  resolveMap(_dbmap, _tables);
}

DbPvlMap::DbPvlMap(const std::string &dbmap, const std::string& pvlsource) :
                   _dbmap(dbmap), _source(pvlsource), _variables(),
                   _functions(getFunctionList()), _tables() {
 resolveMap(_dbmap, _tables);
}

void DbPvlMap::setDbMap(const std::string &dbmap) {
  _dbmap = Pvl(dbmap);
  resolveMap(_dbmap, _tables);
  return;
}

void DbPvlMap::setPvlSource(const std::string &pvlsource) {
  _source = Pvl(pvlsource);
  return;
}

void DbPvlMap::addTableMap(PvlObject &object, bool replace) {
  DbPvlTable table(object);
  DbPvlTableIter iter = find(table.name());
  if (iter != _tables.end()) {
    if (replace) {
      *iter = table;
    }
    else {
      //  Replace not allowed
      string mess = "Specified table " + table.name() + " already exists!";
      throw iException::Message(iException::Programmer, mess.c_str(), _FILEINFO_);
    }
  }
  else {
    _tables.push_back(table);
  }
  return;
}

bool DbPvlMap::hasTable(const std::string &name) const {
  DbPvlTableConstIter iter = find(name);
  if (iter != _tables.end()) return(true);
  return (false);
}

const DbPvlTable &DbPvlMap::getTable(const std::string &name) const {
  DbPvlTableConstIter iter = find(name);
  if (iter != _tables.end()) return (*iter);

  //  Not found, gotta throw
  std::string mess = "Table " + name + " not found!";
  throw iException::Message(iException::Programmer, mess.c_str(), _FILEINFO_);
}

const DbPvlTable &DbPvlMap::getTable(int nth) const {
  if ((nth < 0) || (nth >= size())) {
    //  Not found, gotta throw
    ostringstream mess;
    mess << "Specifed table index(" << nth << ") invalid - range is 0 to " 
         << size() << " tables!";
    throw iException::Message(iException::Programmer, mess.str(), _FILEINFO_);
  }
  return (_tables[nth]);
}


void DbPvlMap::addVariable(const std::string &varname, 
                           const std::string &value) {
  _variables.add(varname, value);
  return;
}


void DbPvlMap::addVariable(const DbProfile &profile) {
  for (int i = 0 ; i < profile.size() ; i++) {
    string key = profile.key(i);
    if (iString::Equal(key, "Password")) {
      if (profile.count(key) == 1) {
        _variables.add(key, profile(key));
      }
    }
  }
  return;
}

DbResult DbPvlMap::Insert() {
  DbResult status;
  DbPvlTableIter iter = _tables.begin();
  Resolver parser(_source, _variables, _functions);
  for ( ; iter != _tables.end() ; ++iter) {
    status.add(iter->Insert(parser)); 
  }
  return (status);
}

DbResult DbPvlMap::Insert(Database db) {
  DbResult status;
  DbPvlTableIter iter = _tables.begin();
  Resolver parser(_source, _variables, _functions);
  for ( ; iter != _tables.end() ; ++iter) {
    status.add(iter->Insert(parser, db)); 
  }
  return (status);
}

DbResult DbPvlMap::Insert(const std::string &table) {
  DbResult status;
  DbPvlTableIter iter = find(table);
  if (iter == _tables.end()) {
    string mess = "Table " + table + " not found - update aborted!";
    throw iException::Message(iException::Programmer, mess.c_str(), _FILEINFO_);
  }
  Resolver parser(_source, _variables, _functions);
  status.add(iter->Insert(parser)); 
  return (status);
}

DbResult DbPvlMap::Insert(const std::string &table, Database db) {
  DbResult status;
  DbPvlTableIter iter = find(table);
  if (iter == _tables.end()) {
    std::string mess = "Table " + table + " not found - update aborted!";
    throw iException::Message(iException::Programmer, mess.c_str(), _FILEINFO_);
  }
  Resolver parser(_source, _variables, _functions);
  status.add(iter->Insert(parser, db)); 
  return (status); 
}

void  DbPvlMap::resolveMap(Pvl &maproot, DbPvlMap::DbPvlTableList &tables) {
  PvlObject map = maproot.FindObject("DbPvlMap");
//  cout << "Resolving map with " << map.Objects() << " objects!\n";
  for (int i = 0 ; i < map.Objects() ; i++) {
    PvlObject &object = map.Object(i);
    if (object.IsNamed("DbTable")) {
//      cout << "Processing new table\n";
      tables.push_back(DbPvlTable(object));
    }
  }
}

DbPvlMap::DbPvlTableConstIter DbPvlMap::find(const std::string &name) const {
  DbPvlTableConstIter iter;
  for (iter = _tables.begin() ; iter != _tables.end() ; ++iter) {
    if (iString::Equal(name, iter->name())) break;
  }
  return (iter);
}

DbPvlMap::DbPvlTableIter DbPvlMap::find(const std::string &name) {
  DbPvlTableIter iter;
  for (iter = _tables.begin() ; iter != _tables.end() ; ++iter) {
    if (iString::Equal(name, iter->name())) break;
  }
  return (iter);
}


} // namespace Isis
