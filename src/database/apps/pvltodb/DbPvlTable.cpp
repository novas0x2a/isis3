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
#include "DbPvlMapInsert.h"
#include "PvlObject.h"
#include "iString.h"
#include "iException.h"

namespace Isis {

DbPvlTable::DbPvlTable() : _name(), _keys(), _update() { }
DbPvlTable::DbPvlTable(const std::string &name) : _name(name), _keys(), 
                                                  _update() { }
DbPvlTable::DbPvlTable(PvlObject &table) : _name(), _keys(), _update() {
    init(table); 
}


DbStatus DbPvlTable::Insert(Resolver &parser) {
//  cout << "Inserting " << _name << " in to database w/out a db\n";
  return (resolve(parser));
}

DbStatus DbPvlTable::Insert(Resolver &parser, Database db) {
  return (resolve(parser, db));
}

void DbPvlTable::init(PvlObject &table) {

// Retrieve and set table name
  _name = table["Name"][0];
//  cout << "Got table named " << _name << endl;

//  Get table/map configuration
  if (table.HasGroup("Configuration")) {
    PvlGroup &config = table.FindGroup("Configuration");

    // Check for update information
    if (config.HasKeyword("PrimaryKey")) {
      _update.primaryKey = config["PrimaryKey"][0];
      _update.whereClause = config["Where"][0];
    }
  }

//  Now verify mapping group and initialize
  if (!table.HasGroup(_name)) {
    //  Not there!  
    string mess = "Required group \"" + _name + "\" to map Pvl keywords to "
                  "table columns not present!";
    throw iException::Message(iException::Programmer, mess.c_str(), _FILEINFO_);
  }

//  cout << "Processing group " << _name << endl;
  PvlGroup &tgroup = table.FindGroup(_name);
  for (int k = 0 ; k < tgroup.Keywords(); k++) {
//    cout << "Adding keyword: " << tgroup[k].Name() << endl;
    _keys.push_back(tgroup[k]);
  }
   return;
}

void DbPvlTable::processKeys(Resolver &parser, const PvlMapKeys &keys,  
                             ColumnSet &columns, PvlMapKeys &missing) const {
  try {
    for (PvlMapKeysConstIter iter = keys.begin() ; iter != keys.end() ; 
          ++iter) {
      if (iter->Size() != 1) {
        ostringstream mess;
        mess << "Map keyword \"" << iter->Name() << "\" has invalid number "
             << "of values (" << iter->Size() << ") - expected just one!";
        throw iException::Message(iException::User, mess.str(), _FILEINFO_);
      }

      if (parser.resolve((*iter)[0])) {
        DbColumn col(iter->Name(), parser.value());
        columns.push_back(col);
        parser.add(col.name(), col.value());
      }
      else {
        missing.push_back(*iter);
      }
    }
  } 
  catch (iException &ie) {
    string mess = "Error encountered generating keyword map for table " + 
                  name();
    ie.Message(iException::Programmer,mess.c_str(), _FILEINFO_);
    throw;
  } catch (...) {
    string mess = "Undetermined error while generating keyword map for table " + 
                  name();
    throw iException::Message(iException::Programmer,mess.c_str(), _FILEINFO_);
  }
  return;
}

void DbPvlTable::build(Resolver &parser, DbPvlMapInsert &inserter) const {
  ColumnSet columns;
  PvlMapKeys current(_keys), missing;

  //  Loop the indicated times to determine if any are resolved in a subsequent
  //  pass.  (This could be a variable)
  unsigned int count = 0; 
  for (int i = 0 ; i < 3 ; i++) {
    missing.clear();
    processKeys(parser,current,columns,missing);
    if (missing.size() == 0) break;
    if (count == columns.size()) break;
    current = missing;
  }

//  If any are missing, determine how to handle it
  if (missing.size() > 0) {
    std::string missDisp = parser.get("MISSING");
    if (iString::Equal(missDisp, "ERROR")) {
      ostringstream mess;
      mess << "These keywords and value cannot be resolved or are missing:\n";
      for (unsigned int k = 0 ; k < missing.size(); k++) {
        mess << missing[k].Name() << " = " << missing[k][0] << endl;
      }
      mess << "--> Operation aborted!";
      throw iException::Message(iException::User, mess.str(), _FILEINFO_);
    }
    else if (iString::Equal(missDisp, "NULL")) {
      if (parser.exists("NULL")) inserter.setNull(parser.get("NULL"));
      for (unsigned int k = 0 ; k < missing.size() ; k++) {
        columns.push_back(DbColumn(missing[k].Name()));
      }
    }
  }

  //  All done...add what we got
  inserter.add(columns);
}

std::string DbPvlTable::buildWhere(Resolver &parser, const UpdateInfo &update) 
                                   const {
  if (!update.isValid()) return ("true");
  return (iString::Replace(update.whereClause, 
                              DbColumn::bind(update.primaryKey),
                 DbPvlMapInsert::sqlQuote(parser.get(update.primaryKey))));

}


DbStatus DbPvlTable::resolve(Resolver &parser) const {
  DbPvlMapInsert inserter(name());
  build(parser, inserter);
  if (_update.isValid()) {
    return(inserter.Update(buildWhere(parser, _update)));
  }
  else {
    return (inserter.Insert());
  }
}

DbStatus DbPvlTable::resolve(Resolver &parser, Database db) const {
  DbPvlMapInsert inserter(name());
  build(parser, inserter);
  if (_update.isValid()) {
    return(inserter.Update(db, buildWhere(parser, _update)));
  }
  else {
    return (inserter.Insert(db));
  }
}

} // namespace Isis
