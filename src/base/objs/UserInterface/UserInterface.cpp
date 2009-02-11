/**                                                                       
 * @file                                                                  
 * $Revision: 1.12 $                                                             
 * $Date: 2009/01/07 17:20:48 $                                                                 
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

#include <sstream>
#include <vector>
#include "UserInterface.h"
#include "iException.h"
#include "Filename.h"
#include "iString.h"
#include "System.h"
#include "Message.h"
#include "Gui.h"
#include "Preference.h"
#include "TextFile.h"

using namespace std;
namespace Isis {

  /** 
   * Constructs an UserInterface object.
   * 
   * @param xmlfile Name of the Isis application xml file to open.
   * 
   * @param argc Number of arguments on the command line
   * 
   * @param argv[] Array of arguments
   */
  UserInterface::UserInterface (const std::string &xmlfile, int argc, char*argv[]) :
  IsisAml::IsisAml (xmlfile) {
    p_interactive = false;
    p_info = false;
    p_infoFileName = "";
    p_gui = NULL;
    p_errList = "";
    p_saveFile = "";
    p_abortOnError = true;

    // Make sure the user has a .Isis and .Isis/history directory
    Isis::Filename setup("$HOME/.Isis");
    if (!setup.Exists()) {
      setup.MakeDirectory();
    }

    setup = "$HOME/.Isis/history";
    if (!setup.Exists()) {
      setup.MakeDirectory();
    }

    // Parse the user input
    LoadCommandLine(argc,argv);

    // See if we need to create the gui
    if (p_interactive) {
      p_gui = Gui::Create (*this,argc,argv);
    }
  }

  //! Destroys the UserInterface object
  UserInterface::~UserInterface () { 
    if(p_gui) {
      delete p_gui;
      p_gui = NULL;
    }
  }

  /** 
   * This is used to load the command line into p_cmdline and the Aml object 
   * using information contained in argc and argv.
   * 
   * @param argc Number of arguments on the command line
   * 
   * @param argv[] Array of arguments
   * 
   * @throws Isis::iException::User - Invalid command line
   * @throws Isis::iException::System - -GUI and -PID are incompatible arguments
   * @throws Isis::iException::System - -BATCHLIST & -GUI are incompatible 
   *                                    arguments
   */
  void UserInterface::LoadCommandLine (int argc, char *argv[]) {
    // The program will be interactive if it has no arguments or
    // if it has the name unitTest
    p_progName = argv[0];
    Isis::iString progName (argv[0]);
    Isis::Filename file(progName);
    if ((argc == 1) && (file.Name() != "unitTest")) {
      p_interactive = true;
    }

    // Convert the command line arguments into a stream
    stringstream ss;
    for (int i=1; i<argc; i++) {
      ss << argv[i] << " ";
    }

    // Tokenize the stream
    try {
      p_cmdline.Load (ss);
    } catch (Isis::iException &e) {
      throw Isis::iException::Message(Isis::iException::User,"Invalid command line",_FILEINFO_);
    }

    // Check for special tokens (those beginning with a dash)
    p_parentId = 0;
    string log = "";
    bool verbose = false;
    string loadHistory = "";
    vector<Isis::PvlToken> t = p_cmdline.GetTokenList ();

    vector<string> opts;
    opts.push_back("-GUI");
    opts.push_back("-NOGUI");
    opts.push_back("-BATCHLIST");
    opts.push_back("-LAST");
    opts.push_back("-RESTORE");
    opts.push_back("-WEBHELP");
    opts.push_back("-HELP");
    opts.push_back("-ERRLIST");
    opts.push_back("-ONERROR");
    opts.push_back("-SAVE");
    opts.push_back("-INFO");
    opts.push_back("-PREFERENCE");
    opts.push_back("-LOG");
    opts.push_back("-VERBOSE");
    opts.push_back("-PID");

    for (int i=0; i<(int)t.size(); i++) {
      string tok = t[i].GetKeyUpper();
      if (tok[0] != '-') continue;
      else {
        int match = 0;
        int index = -1;
        for (int j=0; j<(int)opts.size(); j++) {
          int compare = opts[j].find(tok);
          if (compare == 0) {
            match++; 
            index = j;
          }
        }
        // The input parameter does not match any of the reserve parameters!!
        if (index < 0) {
          string msg = "Invalid Reserve Parameter Option [" + tok + "]. Choices are: ";
          // Make sure not to show -PID as an option (its the last char in the vector)
          for (int t=0; t<(int)opts.size()-2; t++) {
            msg += opts[t] + ",";
          }
          msg += opts[opts.size()-2];
          throw iException::Message(iException::User,msg,_FILEINFO_);
        }
        // The input parameter is ambiguous
        else if (match > 1) {
          string msg = "Ambiguous Reserve Parameter [" + tok + "]. Please clarify.";
          throw iException::Message(iException::User,msg,_FILEINFO_);
        }
        // Assign the closest parameter name to the token
        else tok = opts[index];
      }

      if (tok == "-GUI") {
        p_interactive = true;
      } else if (tok == "-NOGUI") {
        p_interactive = false;
      } else if (tok == "-BATCHLIST") {
        LoadBatchList(t[i].GetValue());
      } else if (tok == "-LAST") {
        loadHistory = "last";
      } else if (tok == "-RESTORE") {
        loadHistory = t[i].GetValue();
      } else if (tok == "-WEBHELP") {
        // Make netscape a user preference
        Isis::PvlGroup &pref = Isis::Preference::Preferences().FindGroup("UserInterface");
        string command = pref["GuiHelpBrowser"];
        command += " $ISISROOT/doc/Application/presentation/Tabbed/";
        command += file.Name() + "/" + file.Name() + ".html";
        Isis::System(command);
        exit(0);
      } else if (tok == "-INFO") {
        p_info = true;
        // check for filename and set value
        if (t[i].ValueSize() >0) {
          p_infoFileName = t[i].GetValue();
        }else{
          p_infoFileName = "";
        }
        
      } else if (tok == "-HELP") {
        if (t[i].ValueSize() == 0) {
          Pvl params;
          params.SetTerminator("");
          for (int k=0; k<NumGroups(); k++) {
            for (int j=0; j<NumParams(k); j++) {
              if (ParamListSize(k,j) == 0) {
                params += PvlKeyword(ParamName(k,j), ParamDefault(k,j)); 
              } else {
                PvlKeyword key(ParamName(k,j));
                string def = ParamDefault(k,j);
                for (int l=0; l<ParamListSize(k,j); l++) {
                  if (ParamListValue(k,j,l) == def) key.AddValue("*" + def);
                  else key.AddValue(ParamListValue(k,j,l));
                }
                params += key;
              }
            }
          }
          cout << params;
        } else {
          Pvl param;
          param.SetTerminator("");
          string key = t[i].GetValueUpper();
          for (int k=0; k<NumGroups(); k++) {
            for (int j=0; j<NumParams(k); j++) {
              if (ParamName(k,j) == key) {
                param += PvlKeyword("ParameterName",key);
                param += PvlKeyword("Brief",ParamBrief(k,j));
                param += PvlKeyword("Type",ParamType(k,j));               
                if (PixelType(k,j) != "") {
                  param += PvlKeyword("PixelType",PixelType(k,j));
                }
                if (ParamInternalDefault(k,j) != "") {
                  param += PvlKeyword("InternalDefault",
                                      ParamInternalDefault(k,j));
                } else param += PvlKeyword("Default",ParamDefault(k,j));
                if (ParamMinimum(k,j) != "") {
                  if (ParamMinimumInclusive(k,j) =="YES") {
                    param += PvlKeyword("GreaterThanOrEqual",ParamMinimum(k,j));
                  } else {
                    param += PvlKeyword("GreaterThan",ParamMinimum(k,j));
                  }
                }
                if (ParamMaximum(k,j) != "") {
                  if (ParamMaximumInclusive(k,j) =="YES") {
                    param += PvlKeyword("LessThanOrEqual",ParamMaximum(k,j));
                  } else {
                    param += PvlKeyword("LessThan",ParamMaximum(k,j));
                  }
                }
                if (ParamLessThanSize(k,j) > 0) {
                  PvlKeyword key("LessThan");
                  for (int l=0; l<ParamLessThanSize(k,j); l++) {
                    key.AddValue(ParamLessThan(k,j,l));
                  }
                  param += key;
                }
                if (ParamLessThanOrEqualSize(k,j) > 0) {
                  PvlKeyword key("LessThanOrEqual");
                  for (int l=0; l<ParamLessThanOrEqualSize(k,j); l++) {
                    key.AddValue(ParamLessThanOrEqual(k,j,l));
                  }
                  param += key;
                }
                if (ParamNotEqualSize(k,j) > 0) {
                  PvlKeyword key("NotEqual");
                  for (int l=0; l<ParamNotEqualSize(k,j); l++) {
                    key.AddValue(ParamNotEqual(k,j,l));
                  }
                  param += key;
                }
                if (ParamGreaterThanSize(k,j) > 0) {
                  PvlKeyword key("GreaterThan");
                  for (int l=0; l<ParamGreaterThanSize(k,j); l++) {
                    key.AddValue(ParamGreaterThan(k,j,l));
                  }
                  param += key;
                }
                if (ParamGreaterThanOrEqualSize(k,j) > 0) {
                  PvlKeyword key("GreaterThanOrEqual");
                  for (int l=0; l<ParamGreaterThanOrEqualSize(k,j); l++) {
                    key.AddValue(ParamGreaterThanOrEqual(k,j,l));
                  }
                  param += key;
                }
                if (ParamIncludeSize(k,j) >0) {
                  PvlKeyword key("Inclusions");
                  for (int l=0; l<ParamIncludeSize(k,j); l++) {
                    key.AddValue(ParamInclude(k,j,l));
                  }
                  param += key;
                }
                if (ParamExcludeSize(k,j) >0) {
                  PvlKeyword key("Exclusions");
                  for (int l=0; l<ParamExcludeSize(k,j); l++) {
                    key.AddValue(ParamExclude(k,j,l));
                  }
                  param += key;
                }
                if (ParamOdd(k,j) != "") {
                  param +=PvlKeyword("Odd",ParamOdd(k,j));
                }
                if (ParamListSize(k,j) != 0) {
                  for (int l=0; l<ParamListSize(k,j); l++) {
                    PvlGroup grp(ParamListValue(k,j,l));
                    grp += PvlKeyword("Brief",ParamListBrief(k,j,l));
                    if (ParamListIncludeSize(k,j,l) != 0) {
                      PvlKeyword include("Inclusions");
                      for (int m=0; m<ParamListIncludeSize(k,j,l); m++) {
                        include.AddValue(ParamListInclude(k,j,l,m));
                      }
                      grp += include;
                    }
                    if (ParamListExcludeSize(k,j,l) != 0) {
                      PvlKeyword exclude("Exclusions");
                      for (int m=0; m<ParamListExcludeSize(k,j,l); m++) {
                        exclude.AddValue(ParamListExclude(k,j,l,m));
                      }
                      grp += exclude;
                    }
                    param.AddGroup(grp);
                  }
                }
                cout << param;
              }
            }
          }
        }
        exit(0);
      } else if (tok == "-PID") {
        p_parentId = Isis::iString(t[i].GetValue()).ToInteger();
      } else if (tok == "-ERRLIST") {
        p_errList = t[i].GetValue();
        if (Filename(p_errList).Exists()) {
          remove(p_errList.c_str());
        }
      } else if (tok == "-ONERROR") {
        if (t[i].GetValueUpper() == "CONTINUE") p_abortOnError = false;
        else if (t[i].GetValueUpper() == "ABORT") continue;
        else {
          string msg = "[" + t[i].GetValueUpper() + 
                       "] is an invalid value for -ONERROR, options are ABORT or CONTINUE";
          throw Isis::iException::Message(Isis::iException::User,msg,_FILEINFO_);
        }
      } else if (tok == "-SAVE") {
        if (t[i].ValueSize() == 0) p_saveFile = ProgramName() + ".par";
        else p_saveFile = t[i].GetValue();
      } else if (tok == "-PREFERENCE") {
        Preference &p = Preference::Preferences();
        p.Load(t[i].GetValue());
      } else if (tok == "-LOG") {
        if (t[i].ValueSize() == 0) log = "print.prt";
        else log = t[i].GetValue();
      } else if (tok == "-VERBOSE") {
        verbose = true;
      }
    }


    // Can't have a parent id and the gui
    if (p_parentId > 0 && p_interactive) {
      string msg = "-GUI and -PID are incompatible arguments";
      throw Isis::iException::Message(Isis::iException::System,msg,_FILEINFO_);
    }

    // Can't use the batchlist with the gui, save, last or restore option
    if (BatchListSize() != 0 && (p_interactive || loadHistory != "" 
                                 || p_saveFile != "")) {
      string msg = "-BATCHLIST cannot be used with -GUI, -SAVE, -RESTORE, ";
      msg += "or -LAST";
      throw Isis::iException::Message(Isis::iException::System,msg,_FILEINFO_);
    }

    // Must use batchlist if using errorlist or onerror=continue
    if ((BatchListSize() == 0) && (!p_abortOnError || p_errList != "")) {
      string msg = "-ERRLIST and -ONERROR=continue cannot be used without ";
      msg += " the -BATCHLIST option";
      throw Isis::iException::Message(Isis::iException::System,msg,_FILEINFO_);
    }

    // Now load the log, so it isn't overriden by the preference file if
    // it was also set
    Preference &p = Preference::Preferences();
    if (log != "") {
      PvlGroup &grp = p.FindGroup("SessionLog", Isis::Pvl::Traverse);
      grp["Filename"].SetValue(log);
      grp["FileOutput"].SetValue("On");
    }
    if (verbose) {
      PvlGroup &grp = p.FindGroup("SessionLog", Isis::Pvl::Traverse);
      grp["TerminalOutput"].SetValue("On");
    }

    if (loadHistory != "") {
      if (loadHistory == "last") {
        PvlGroup &grp = p.FindGroup("UserInterface", Isis::Pvl::Traverse);
        loadHistory = grp["HistoryPath"][0] + "/" + file.Name() + ".par"; 
      }
      LoadHistory(loadHistory);
    }

    // Load the normal tokens into the appdata class and ignore any
    // special tokens
    if (BatchListSize() == 0) {
      for (int i=0; i<(int)t.size(); i++) {
        try {
          string tok = t[i].GetKey();
          if (tok[0] == '-') continue;
          Clear(t[i].GetKey());
          PutAsString(t[i].GetKey(),t[i].ValueVector());
        } catch (Isis::iException &e) {
          throw Isis::iException::Message(Isis::iException::User,"Invalid command line",_FILEINFO_);
        }
      }
    }

    return;
  }

  /**
   * Loads the user entered batchlist file into a private variable for later use
   * 
   * @param file The batchlist file to load
   * 
   * @throws Isis::iException::User - The batchlist does not contain any data
   */
  void UserInterface::LoadBatchList(const std::string file) {
    // Read in the batch list
    TextFile temp;
    try {
      temp.Open(file);
    } catch (iException &e) {
      string msg = "The batchlist file [" + file + "] could not be opened";
      throw iException::Message(iException::User,msg,_FILEINFO_);   
    }
    p_batchList.resize(temp.LineCount());
    for (int i=0; i<temp.LineCount(); i++) {
      p_batchList[i].resize(10);
      iString t;
      temp.GetLine(t);

      // Convert tabs to spaces but leave tabs inside quotes alone
      t.Replace("\t"," ",true);

      t.Compress();
      t.Trim(" ");
      // Allow " ," " , " or ", " as a valid single seperator
      t.Replace( " ," , "," ,true);
      t.Replace( ", " , "," ,true);
      // Convert all spaces to "," the use "," as delimiter
      t.Replace (" ", "," , true);
      int j=0;
      iString token = t.Token(",");
      int iteration =0;
      while (token != "") {
        iteration++;
        // removes quotes from tokens. NOTE: also removes escaped quotes.
        token = token.Remove("\"'");
        p_batchList[i][j] = token;
        token = t.Token(",");
        j++;
      }
      p_batchList[i].resize(j);
      // Every row in the batchlist must have the same number of columns
      if (i==0) continue;
      if (p_batchList[i-1].size() != p_batchList[i].size()) {
        string msg = "The number of columns must be constant in batchlist";
        throw iException::Message(iException::User,msg,_FILEINFO_);   
      }
    }
    // The batchlist cannot be empty
    if (p_batchList.size() < 1) {
      string msg = "The list file [" + file + "] does not contain any data";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    }
  }

  /**
   * Loads the previous history for the program
   * 
   * @param file Filename to get the history entry from
   * 
   * @throws Isis::iException::User - The file does not contain any parameters 
   *                                  to restore
   * @throws Isis::iException::User - The file does not contain a valid parameter
   *                                  history file
   * @throws Isis::iException::User - Parameter history file does not exist
   */
  void UserInterface::LoadHistory(const std::string file) {
    Isis::Filename hist(file);
    if (hist.Exists()) {
      try {
        Isis::Pvl lab(hist.Expanded());

        int g = lab.Groups() - 1;
        if (g >= 0 && lab.Group(g).IsNamed("UserParameters")) {
          Isis::PvlGroup &up = lab.Group(g);
          for (int k=0; k<up.Keywords(); k++) {
            string keyword = up[k].Name();
            string value = up[k][0];
            PutAsString(keyword,value);
          }
          return;
        }

        for (int o=lab.Objects()-1; o>=0; o--) {
          if (lab.Object(o).IsNamed(ProgramName())) {
            Isis::PvlObject &obj = lab.Object(o);
            for (int g=obj.Groups()-1; g>=0; g--) {
              Isis::PvlGroup &up = obj.Group(g);
              if (up.IsNamed("UserParameters")) {
                for (int k=0; k<up.Keywords(); k++) {
                  string keyword = up[k].Name();
                  string value = up[k][0];
                  PutAsString(keyword,value);
                }
              }
              return;
            }
          }
        }

        string msg = "[" + hist.Expanded() + 
                     "] does not contain any parameters to restore";
        throw Isis::iException::Message(Isis::iException::User,msg,_FILEINFO_);
      } catch (...) {
        string msg = "A corrupt parameter history file [" + file + 
                     "] has been detected. Please fix or remove this file";
        throw Isis::iException::Message(Isis::iException::User,msg,_FILEINFO_);
      }
    } else {
      string msg = "Parameter history file [" + file + 
                   "] does not exist";
      throw Isis::iException::Message(Isis::iException::User,msg,_FILEINFO_);
    }
  }

  /**
   * Saves the user parameter information in the history of the program for later
   * use
   */
  void UserInterface::SaveHistory() {

    // If history recording is off, return
    Preference &p = Preference::Preferences();
    PvlGroup &grp = p.FindGroup("UserInterface", Isis::Pvl::Traverse);
    if (grp["HistoryRecording"][0] == "Off") return;

    // Get the current history file
    Isis::Filename histFile(grp["HistoryPath"][0] + "/" +ProgramName()+".par");

    // If a save file is specified, override the default file path
    if (p_saveFile != "") histFile = p_saveFile;

    // Get the current command line
    Isis::Pvl cmdLine;
    CommandLine(cmdLine);

    Isis::Pvl hist;

    // If the history file's Pvl is corrupted, then
    // leave hist empty such that the history gets
    // overwriten with the new entry. 
    try {
      if (histFile.Exists()) {
        hist.Read(histFile.Expanded());
      }
    } catch (iException e) { 
      e.Clear();
    }

    // Add it
    hist.AddGroup(cmdLine.FindGroup("UserParameters"));

    // See if we have exceeded history length
    while (hist.Groups() > (int)grp["HistoryLength"][0]) {
      hist.DeleteGroup("UserParameters");
    }

    // Write it
    hist.Write(histFile.Expanded());

  }

  /**
   * Clears the gui parameters and sets the batch list information at line i as
   * the new parameters 
   * 
   * @param i The line number to retrieve parameter information from
   */
  void UserInterface::SetBatchList(int i) {
    //Clear all parameters currently in the gui 
    for (int k=0; k<NumGroups(); k++) {
      for (int j=0; j<NumParams(k); j++) {
        Clear(ParamName(k,j));
      }
    }

    //Load the new parameters into the gui
    vector<Isis::PvlToken> t = p_cmdline.GetTokenList();
    cout << p_progName << " ";
    for (int k=0; k<(int)t.size(); k++) {
      try {
        string tok = t[k].GetKey();
        if (tok[0] == '-') continue;
        iString value = t[k].GetValue();
        value.Trim(" ");
        string cmd;
        // Replace all $number occurences with the value in that column of the
        // batchlist
        string token = value.Token("$");
        while (value != "") {
          cmd += token;
          int j = iString(value.substr(0,1)).ToInteger() - 1;
          cmd += p_batchList[i][j];
          value.replace(0,1,"");
          token = value.Token("$");
        }
        if (token != "") cmd += token;
        PutAsString(t[k].GetKey(),cmd);
        cout << t[k].GetKey() << "=" << cmd << " ";
      } catch (Isis::iException &e) {
        throw Isis::iException::Message(Isis::iException::User,"Invalid command line",_FILEINFO_);
      }
    }
    cout << endl;

    // Verify the command line
    VerifyAll();
  }

  /**
   * This method adds the line specified in the BatchList that the error occured
   * on.  The BatchList line is added exactly as it is seen, so the BatchList 
   * command can be run on the errorlist file created.
   * 
   * @param i The line of the batchlist to write to the error file
   */
  void UserInterface::SetErrorList(int i) {
    if (p_errList != "") {
      std::ofstream os;
      string fileName(Filename(p_errList).Expanded());
      os.open(fileName.c_str(),std::ios::app);
      if (!os.good()) {
        string msg = "Unable to create error list [" + p_errList + 
                     "] Disk may be full or directory permissions not writeable";
        throw Isis::iException::Message(Isis::iException::User,msg,_FILEINFO_);
      }
      for (int j=0; j<(int)p_batchList[i].size(); j++) {
        os << p_batchList[i][j] << " ";
      }
      os << endl;
      os.close();
    }
  }

  /**
   * This method returns the flag state of info. This returns if
   * its in debugging mode(the -info tag was specified).
   */
  bool UserInterface::GetInfoFlag() {
    return p_info;
  }

  /**
   * This method returns the filename where the debugging info is
   * stored when the "-info" tag is used.
   */
  std::string UserInterface::GetInfoFileName() {
    return p_infoFileName;
  }
} // end namespace isis
