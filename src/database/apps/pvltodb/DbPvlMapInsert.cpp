/**                                                                       
 * @file                                                                  
 * $Revision: 1.5 $
 * $Date: 2007/11/13 20:54:42 $
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

#include "DbPvlMapInsert.h"
#include "SqlQuery.h"
#include "iString.h"
#include "iException.h"

namespace Isis {

DbPvlMapInsert::DbPvlMapInsert() : _name(), _null("NULL"), 
                                    _columns() { }

DbPvlMapInsert::DbPvlMapInsert(const std::string &tblname) : _name(tblname),
                                                             _null("NULL"),
                                                             _columns() { }
DbPvlMapInsert::DbPvlMapInsert(const std::string &tblname,
                               const DbPvlMapInsert::DbColumnList &clist) : 
                               _name(tblname),_null("NULL"),_columns(clist) { }


void DbPvlMapInsert::add(const DbColumn &column, bool replace) {
  DbColumnIter iter = find(column.name());
  if (iter != _columns.end()) {
    if (!replace) {
      std::string mess = "Column \"" + column.name() + "\" already exists!";
      throw iException::Message(iException::Programmer, mess.c_str(), _FILEINFO_);
    }
    *iter = column;
  }
  else {
    _columns.push_back(column);
  }
  return;
}

void DbPvlMapInsert::add(const DbColumnList &columns, bool replace) {
  for (DbColumnConstIter iter = columns.begin() ; iter != columns.end() ; 
       ++iter) {
    add(*iter,replace);
  }
  return;
}

bool DbPvlMapInsert::exist(const std::string &name) const {
  DbColumnConstIter iter = find(name);
  if (iter != _columns.end()) return (true);
  else return (false);
}

const DbColumn &DbPvlMapInsert::get(const std::string &name) const {
  DbColumnConstIter iter = find(name);
  if (iter == _columns.end()) {
    string mess = "Column \"" + name + "\" does not exist!";
    throw iException::Message(iException::Programmer, mess.c_str(), _FILEINFO_);
  }
  return (*iter);
}

const DbColumn &DbPvlMapInsert::get(int nth) const {
  if ((nth < 0) || (nth >= size())) {
    ostringstream mess;
    mess << "Specifed column index(" << nth << ") invalid - range is 0 to " 
         << size() << " columns!";
    throw iException::Message(iException::Programmer, mess.str(), _FILEINFO_);
  }
  return (_columns[nth]);
}

DbStatus DbPvlMapInsert::Insert() {
  return(prepareInsert());
}

DbStatus DbPvlMapInsert::Insert(Database db) {
  return (executeSql(prepareInsert(), db));
}

DbStatus DbPvlMapInsert::Update(const std::string &where) {
  return (prepareUpdate(where));
}

DbStatus DbPvlMapInsert::Update(Database db, const std::string &where) {
#if 0
  DbStatus stat = checkUpdate(where,db);
  if (stat.rowsAffected > 0) return (executeSql(prepareUpdate(where), db));
  else return (Insert(db));
#else
  DbStatus stat = executeSql(prepareUpdate(where), db);
  if (stat.status != 0) return (stat);        // Sql failed
  if (stat.rowsAffected != 0) return (stat);  //  Update was successful
  return (Insert(db));  // If no rows were updated (doesn't exist yet)
#endif
}

DbStatus DbPvlMapInsert::checkUpdate(const std::string &where, 
                                     Database &db) const {
  ostringstream sql;
  sql << "SELECT * FROM " << name();
  string whereClause = (where.empty()) ? "true" : where;
  sql << " WHERE " << whereClause;
  return (executeSql(DbStatus(name(), 0, "SELECT", sql.str()),db, false));
}

DbPvlMapInsert::DbColumnConstIter DbPvlMapInsert::find(const std::string &name) const {
  DbColumnConstIter iter;
  for (iter = _columns.begin() ; iter != _columns.end() ; ++iter) {
    if (iString::Equal(name, iter->name())) break;
  }
  return (iter);
}

DbPvlMapInsert::DbColumnIter DbPvlMapInsert::find(const std::string &name) {
  DbColumnIter iter;
  for (iter = _columns.begin() ; iter != _columns.end() ; ++iter) {
    if (iString::Equal(name, iter->name())) break;
  }
  return (iter);
}

std::string DbPvlMapInsert::sqlQuote(const std::string &value) {

  // If its NULL or inserting a geometry text value just return it unmodified
  if (iString::Equal(value, "NULL")) return (value);
  if (iString::DownCase(value).find("geomfromtext") != string::npos) 
       return (value);

  //  Must change single quotes to two quotes
  string dbvalue(value);
  for (string::iterator quote = dbvalue.begin() ; quote != dbvalue.end() ; 
        ++quote) {
    if (*quote == '\'') {
      quote = dbvalue.insert(quote+1,'\'');
    }
  }

#if 0
  // Probably not fool/idiot-proof
  if (value.find_first_of(" \t\v\f\n,=<>:$%")  != std::string::npos) 
    return (string("'" + value + "'"));
  return (value);  // Not needed?
#endif
  return (string("'" + dbvalue + "'"));
}


DbStatus DbPvlMapInsert::prepareInsert() const {
  ostringstream columns, values, sql;
  string separator;

  columns << "(";
  values  << "(";

  DbColumnConstIter iter = _columns.begin();
  for (separator = "" ; iter != _columns.end() ; ++iter, separator = ", ") {
    columns << separator << iter->name();
    std::string value = (iter->isMissing()) ? null() : iter->value();
    values  << separator << sqlQuote(value);
  }
  columns << ")";
  values  << ")";

  sql << "INSERT into " << name() << " " << columns.str() << " VALUES " 
      << values.str();

  return (DbStatus(name(), _columns.size(), "INSERT", sql.str()));
}

DbStatus DbPvlMapInsert::prepareUpdate(const std::string &where) const {
  ostringstream sql;
  string separator;

  sql << "UPDATE " << name() << " SET "; 

  DbColumnConstIter iter = _columns.begin();
  for (separator = "" ; iter != _columns.end() ; ++iter, separator = ", ") {
    sql << separator << iter->name() << " = ";
    std::string value = (iter->isMissing()) ? null() : iter->value();
    sql << sqlQuote(value);
  }

  string whereClause = (where.empty()) ? "true" : where;
  sql << " WHERE " << whereClause;

  return (DbStatus(name(), _columns.size(), "UPDATE", sql.str()));
}

DbStatus DbPvlMapInsert::executeSql(const DbStatus &statement, Database &db,
                                    bool useTransactions) const {

  //  Set up a transaction for error recovery
  if (useTransactions) db.transaction();

  //  Set up the inserter query
  SqlQuery inserter(db);
  inserter.setThrowOnFailure();
  DbStatus status(statement);

  // Execute
  try {
    inserter.exec(statement.sql);
    if (useTransactions) db.commit();   // Commit the update
    status.status = 0;
    status.rowsAffected = max(inserter.numRowsAffected(), inserter.nRows());
  } 
  catch (iException &ie) {
    //  Failed, rollback and report
    if (useTransactions) db.rollback();
    status.status = 1;
    status.lastError = ie.Errors();
  }

  return (status);
}

} // namespace Isis
