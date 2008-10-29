#include "Isis.h"

#include <iostream> 
#include <vector>
#include <string>

#include <sstream>
#include <fstream>
#include <iomanip> 

#include <algorithm>
#include <memory>

using namespace std;

#include "UserInterface.h"
#include "SessionLog.h"
#include "iString.h"
#include "iTime.h"
#include "Filename.h"
#include "DbAccess.h"
#include "Database.h"
#include "DbPvlMap.h"
#include "DbPvlTable.h"

using namespace Isis;

void IsisMain() {


  const std::string version("$Revision: 1.6 $");

  UserInterface &ui = Application::GetUserInterface();
  string pvlSource = ui.GetFilename("FROM");

  string dbMap("");
  if (ui.WasEntered("DBMAP")) dbMap = ui.GetFilename("DBMAP");

  string dbConf("");
  if (ui.WasEntered("DBCONFIG")) dbConf = ui.GetFilename("DBCONFIG");

  string dbProf("");
  if (ui.WasEntered("PROFILE")) dbProf = ui.GetString("PROFILE");

  string dbTable("");
  if (ui.WasEntered("TABLES")) dbTable = ui.GetString("TABLES");

  string dbMissing("EXCLUDE");
  dbMissing = ui.GetString("MISSING");

  string dbMode("COMMIT");
  dbMode = ui.GetString("MODE");
  bool commit = iString::Equal(dbMode, "COMMIT");

  string dbVerbose("NO");
  dbVerbose = ui.GetString("VERBOSE");
  bool verbose = iString::Equal(dbVerbose, "YES");

  string dbShowSql("NO");
  dbShowSql = ui.GetString("SHOWSQL");
  bool showsql = iString::Equal(dbShowSql, "YES");

  // If we are not commiting or want'n to see the SQL, turn verbose on
  if (!commit) verbose = true;
  if (showsql) verbose = true;

  try {

//  First thing to do is add the access profile if provided by user
    if (!dbConf.empty()) {
      if (!Database::addAccessConfig(dbConf)) {
        std::string mess = "Unable to open/add DBCONF file " + dbConf;
        throw iException::Message(iException::User, mess, _FILEINFO_);
      }
    }

// Get the default profile.
    DbProfile profile = Database::getProfile(dbProf);

// Instantiate the map object
    DbPvlMap pvlmap;

//  Add program and profile variables
    pvlmap.addVariable("PROGRAM", "pvltodb");
    pvlmap.addVariable("VERSION", version);
    pvlmap.addVariable("RUNDATE", iTime::CurrentLocalTime());
    pvlmap.addVariable("MISSING", dbMissing);
    pvlmap.addVariable("VERBOSE", dbVerbose);
    pvlmap.addVariable("SHOWSQL", dbShowSql);
    pvlmap.addVariable(profile);

//  Add PVL input source file (FROM)
    pvlmap.setPvlSource(pvlSource);
    pvlmap.addVariable("FROM", pvlSource);

// Determine the map file and add it
    if (dbMap.empty()) {
      try {
        dbMap = profile("DBMAP");
      } catch (iException &ie) {
        ostringstream mess;
        mess << "DBMAP file not provided by user and not found in DBCONF ("
             << dbConf << ") file profile (" << profile.Name() << ") either."
             << endl;
        ie.Message(iException::User, mess.str(), _FILEINFO_);
        throw;
      }
    }

// Add the map file
    pvlmap.setDbMap(dbMap);
    pvlmap.addVariable("DBMAP", dbMap);

//  Insert map into database
    DbResult result;
    if (commit) {
      // This implementation is necessary for persistant connects when using
      // more than one table.  Pointers are required so the database connection
      // can be removed for more than one run (in GUI mode) as Qt will complain.
      Database *db = new Database(profile,Database::Connect);
      string dbName = db->Name();
      db->makePersistant();
      if (!dbTable.empty()) {
        result = pvlmap.Insert(dbTable, *db);
      } 
      else {
        result = pvlmap.Insert(*db);
      }
      delete db;
      Database::remove(dbName);
    }
    else {
      if (!dbTable.empty()) {
        result = pvlmap.Insert(dbTable);
      } 
      else {
        result = pvlmap.Insert();
      }
    }

// Report processing if requested
    int nerrors(0);
    for (int i = 0 ; i < result.size() ; i++) {
      const DbStatus &status = result.get(i);
      PvlGroup pResult("Result" + iString(i));
      pResult += PvlKeyword("Table", status.table);
      pResult += PvlKeyword("Columns", status.columns);
      pResult += PvlKeyword("Operation", status.operation);
      if (showsql) pResult += PvlKeyword("Query", status.sql);
      pResult += PvlKeyword("RowsAffected", status.rowsAffected);
      pResult += PvlKeyword("Status", status.status);
      pResult += PvlKeyword("Error", ((status.lastError.empty()) ? "None" : 
                                      status.lastError));
      if (verbose) {
        Application::Log(pResult);
      }
      else {
        SessionLog::TheLog().AddResults(pResult);
        SessionLog::TheLog().Write();
      }

      if (status.status > 0) {
        nerrors++;
        iException::Message(iException::Programmer, status.lastError, 
                            _FILEINFO_);
      }
    }

    if (nerrors > 0) {
      throw iException::Message(iException::Programmer,
                                "Database operations has errors",
                                 _FILEINFO_);
    }

  }
  catch (iException &ie) {
    ie.Message(iException::Programmer, "Error encountered in pvltodb processing",
               _FILEINFO_);
    throw;
  }
  catch (...) {
    // This is really unexpected
    cerr << "Unhandled exception" << endl;
    throw iException::Message(iException::Programmer, "Unhandled exception",
                               _FILEINFO_);
  }
  return;
}
