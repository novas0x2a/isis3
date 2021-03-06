#include "Isis.h"
#include "SpiceDbGen.h"
#include "iTime.h"

using namespace Isis;
void IsisMain(){

  UserInterface &ui = Application::GetUserInterface();
  PvlGroup dependency("Dependencies");

  //create the database writer based on the kernel type
  SpiceDbGen sdg(ui.GetString("TYPE"));
  
  //Load the SCLK. If it exists, add its location to the dependency group 
  //If there is none, set a flag so that no file is searched for
  bool needSclk = false;
  Filename sclkFile("");
  if (ui.WasEntered("SCLK")){
    iString sclkString = ui.GetAsString("SCLK");
    if (sclkString.length() != 0){
      sclkString.Trim("\\");
      sclkFile = Filename(sclkString);
      if (sclkFile.Expanded().find_first_of("?") != string::npos){
        sclkFile.HighestVersion();
	sclkString = sclkFile.OriginalPath() + "/" + sclkFile.Name();
      }
      dependency += PvlKeyword("SpacecraftClockKernel", sclkString);
      needSclk = true;
    }
  }
  
  iString lskString = ui.GetAsString("LSK");
  lskString.Trim("\\");
  Filename lskFile(lskString);
  if (lskFile.Expanded().find_first_of("?") != string::npos){
    lskFile.HighestVersion();
    lskString = lskFile.OriginalPath() + "/" + lskFile.Name();
  }
  dependency += PvlKeyword("LeapsecondKernel", lskString);
  //furnish dependencies with an SCLK
  if (needSclk){sdg.FurnishDependencies(sclkFile.Expanded(), lskFile.Expanded());}
  //Furnish dependencies without an SCLK
  else {sdg.FurnishDependencies("", lskFile.Expanded());}
  
  //Determine the type of kernel that the user wants a database for. This will
  //eventually become the name of the object in the output PVL
  string kernelType;
  if (ui.GetString("TYPE") == "CK"){kernelType = "SpacecraftPointing";}
  else if (ui.GetString("TYPE") == "SPK"){kernelType = "SpacecraftPosition";}
  PvlObject selections(kernelType);
  
  selections += PvlKeyword("RunTime", iTime::CurrentLocalTime());
  selections.AddGroup(dependency);
  
  iString location = "", filter = "";
  
  if (ui.GetString("NADIRFILTER") != "none" && 
      ui.GetString("NADIRDIR") != "none"){
    location = ui.GetString("NADIRDIR");
    location.Trim("\\");
    filter = ui.GetString("NADIRFILTER");    
    PvlObject result = sdg.Direct("Nadir",location, filter);
    PvlObject::PvlGroupIterator grp = result.BeginGroup();
    while(grp != result.EndGroup()){ selections.AddGroup(*grp);grp++;}
  }
  if (ui.GetString("PREDICTFILTER") != "none" &&
      ui.GetString("PREDICTDIR") != "none"){
    location = ui.GetString("PREDICTDIR");
    location.Trim("\\");
    filter = ui.GetString("PREDICTFILTER");
    PvlObject result = sdg.Direct("Predicted",location, filter);
    PvlObject::PvlGroupIterator grp = result.BeginGroup();
    while(grp != result.EndGroup()){selections.AddGroup(*grp);grp++;}
  }
  if (ui.GetString("RECONDIR") != "none" && 
      ui.GetString("RECONFILTER") != "none"){
    location = ui.GetString("RECONDIR");
    location.Trim("\\");
    filter = ui.GetString("RECONFILTER");    
    PvlObject result = sdg.Direct("Reconstructed",location, filter);
    PvlObject::PvlGroupIterator grp = result.BeginGroup();
    while(grp != result.EndGroup()){selections.AddGroup(*grp);grp++;}
  }

  if (ui.GetString("SMITHEDDIR") != "none" && 
      ui.GetString("SMITHEDFILTER") != "none"){
    location = ui.GetString("SMITHEDDIR");
    location.Trim("\\");
    filter = ui.GetString("SMITHEDFILTER");    
    PvlObject result = sdg.Direct("Smithed",location, filter);
    PvlObject::PvlGroupIterator grp = result.BeginGroup();
    while(grp != result.EndGroup()){selections.AddGroup(*grp);grp++;}
  }
  if (filter == ""){
    string message = 
    "You must enter a filter AND directory for at least one type of kernel";
    throw Isis::iException::Message(Isis::iException::User,message,
                                    _FILEINFO_);
  }
  
  //specify a name for the output file
  Filename to("./kernels.????.db");
  if (ui.WasEntered("TO")){to = ui.GetFilename("TO");}
  //create a new output version if the user specified any version sequence
  if (to.Expanded().find_first_of("?") != string::npos){to.NewVersion();}
  
  Pvl writer;
  writer.AddObject(selections);
  writer.Write(to.Expanded());
}
