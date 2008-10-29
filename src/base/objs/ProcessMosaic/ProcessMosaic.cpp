/**
 * @file
 * $Revision: 1.3 $
 * $Date: 2008/10/03 21:46:43 $
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
#include "Preference.h"

#include "Portal.h"
#include "ProcessMosaic.h"
#include "SpecialPixel.h"
#include "iException.h"
#include "Application.h"
#include "Pvl.h"

using namespace std;
namespace Isis {
 /** 
  * This method invokes the process by mosaic operation over a single input cube 
  * and single output cube. Unlike other Isis process objects, no application .
  * function will be called. The processing is handled entirely within the 
  * mosaic object. The input cube must be pixel aligned with the output cube 
  * before mosaiking. If the input cube does not overlay any of the output cube,  
  * no processing takes place and no errors are thrown.
  * 
  * @param os The sample position of input cube starting sample relative to 
  *           the output cube. The cordinate is in output cube space and may
  *           be any integer value negative or positive.
  * 
  * @param ol The line position of input cube starting line relative to the 
  *           output cube. The cordinate is in output cube space and may be 
  *           any integer value negative or positive.
  * 
  * @param ob The band position of input cube starting band relative to the
  *           output cube. The cordinate is in output cube space and must be 
  *           a legal band number within the output cube.
  * 
  * @param priority This parameter determines which cube takes priority 
  *                 when both the input and output cubes contain non null 
  *                 data. There ara two possible values (input and mosaic).
  *                 When this parameter is set to input all non null pixels
  *                 will be transfered to the mosaic. Null pixels will not 
  *                 be transfered. When this parameter is set to mosaic 
  *                 input pixel values will only be transfered to the mosaic  
  *                 when the mosaic contains a null value.
  * 
  * @throws Isis::iException::Message
  */
  void ProcessMosaic::StartProcess (const int &os, const int &ol,
                                    const int &ob, const MosaicPriority &priority) {
  
    int outSample = os;
    int outLine = ol;
    int outBand =ob;
  
    // Error checks ... there must be one input and one output
    if ((OutputCubes.size() != 1) || (InputCubes.size() != 1)){
      string m = "You must specify exactly one input and one output cube";
      throw Isis::iException::Message(Isis::iException::Programmer,m, _FILEINFO_);
    }

    // Check to make sure the bandbins match if necessary
    if (p_bandbinMatch) {
      Pvl *inLab = InputCubes[0]->Label();
      Pvl *outLab = OutputCubes[0]->Label();

      if (inLab->FindObject("IsisCube").HasGroup("BandBin")) {
        PvlGroup &inBin = inLab->FindGroup("BandBin",Pvl::Traverse);
        // Check to make sure the output cube has a bandbin group & make sure it 
        // matches the input cube bandbin group
        if (outLab->FindObject("IsisCube").HasGroup("BandBin")) {
          PvlGroup &outBin = outLab->FindGroup("BandBin",Pvl::Traverse);
          if (inBin.Keywords() != outBin.Keywords()) {
            string msg = "Pvl Group [BandBin] does not match between the input and output cubes";
            throw Isis::iException::Message(iException::User,msg,_FILEINFO_);
          }
          for (int i=0; i<outBin.Keywords(); i++) {
            PvlKeyword &outKey = outBin[i];
            if (inBin.HasKeyword(outKey.Name())) {
              PvlKeyword &inKey = inBin[outKey.Name()];
              for (int v=0; v<outKey.Size(); v++) {
                if (outKey[v] != inKey[v]) {
                  string msg = "Pvl Group [BandBin] Keyword[" + outBin[i].Name() + 
                    "] does not match between the input and output cubes";
                  throw Isis::iException::Message(iException::User,msg,_FILEINFO_);
                }
              }
            }
            else {
              string msg = "Pvl Group [BandBin] Keyword[" + outBin[i].Name() + 
                "] does not match between the input and output cubes";
              throw Isis::iException::Message(iException::User,msg,_FILEINFO_);
            }
          }
        }
        // Otherwise copy the input cube bandbin to the output file
        else {
          outLab->FindObject("IsisCube").AddGroup(inBin);
        }
      }
    }

    if ((outBand < 1) || ((outBand + p_inb - 1) > OutputCubes[0]->Bands())) {
      string m = "All bands from the input cube must fit within the output mosaic";
      throw Isis::iException::Message(Isis::iException::Programmer,m, _FILEINFO_);
    }
  
    // Set up the input sub cube (This must be a legal sub-area of the input cube)
    int ins = p_ins;
    int inl = p_inl;
    int inb = p_inb;

    if (ins == 0) ins = (int)InputCubes[0]->Samples();
    if (inl == 0) inl = (int)InputCubes[0]->Lines();
    if (inb == 0) inb = (int)InputCubes[0]->Bands();
  
    // Adjust the input sub-area if it overlaps any edge of the output cube
  
    int iss = p_iss;
    int isl = p_isl;

    // Left edge
    if (outSample < 1) {
      iss = iss - outSample + 1;
      ins = ins + outSample - 1;
      outSample = 1;
    }
    // Top edge
    if (outLine < 1) {
      isl = isl - outLine + 1;
      inl = inl + outLine - 1;
      outLine = 1;
    }
    // Right edge
    if ((outSample + ins - 1) > OutputCubes[0]->Samples()) {
      ins = OutputCubes[0]->Samples() - outSample + 1;
    }
    // Bottom edge
    if ((outLine + inl - 1) > OutputCubes[0]->Lines()) {
      inl = OutputCubes[0]->Lines() - outLine + 1;
    }
  
    // Tests for completly off the mosaic
    if ((ins < 1) || (inl < 1)){
      string m = "The input cube does not overlap the mosaic";
      throw Isis::iException::Message(Isis::iException::User,m, _FILEINFO_);
    }
  
    // Create portal buffers for the input and output files
    Isis::Portal iportal (ins, 1, InputCubes[0]->PixelType());
    Isis::Portal oportal (ins, 1, OutputCubes[0]->PixelType());
  
    // Start the progress meter
    p_progress->SetMaximumSteps (inl*inb);
    p_progress->CheckStatus();
  
    for (int ib=p_isb, ob=outBand; ib<p_isb+inb; ib++, ob++) {
      for (int il=isl, ol=outLine; il<isl+inl; il++, ol++) {
        // Set the position of the portals in the input and output cubes
        iportal.SetPosition (iss, il, ib);
        InputCubes[0]->Read(iportal);
  
        oportal.SetPosition (outSample, ol, ob);
        OutputCubes[0]->Read(oportal);
  
        // Move the input data to the output
        for (int pixel=0; pixel<oportal.size(); pixel++) {
          if (priority == input) {
            if (!Isis::IsNullPixel(iportal[pixel])) oportal[pixel] = iportal[pixel];
          }
          else if (priority == mosaic) {
            if (Isis::IsNullPixel(oportal[pixel])) oportal[pixel] = iportal[pixel];
          }
        } // End sample loop
  
        OutputCubes[0]->Write(oportal);
        p_progress->CheckStatus();
      } // End line loop
    } // End band loop
  } // End StartProcess
  
 /** 
  * Opens an input cube specified by the user. This method is overloaded and  
  * adds the requirement that only one input cube can be specified.
  * 
  * @return Cube*
  *
  * @param parameter User parameter to obtain file to open. Typically, the value 
  *                  is "FROM". For example, the user can specify on the command
  *                  line FROM=myfile.cub and this method will attempt to open  
  *                  the cube "myfile.cub" if the parameter was set to "FROM".
  * 
  * @param ss The starting sample within the input cube. This allowd the 
  *           application to choose a sub-area from the input cube to be place
  *           into the mosaic. Defaults to 1
  * 
  * @param sl The starting line within the input cube. This allowd the 
  *           application to choose a sub-area from the input cube to be place 
  *           into the mosaic. Defaults to 1
  * 
  * @param sb The starting band within the input cube. This allowd the  
  *           application to choose a sub-area from the input cube to be place 
  *           into the mosaic. Defaults to 1
  * 
  * @param ns The number of samples from the input cube. This allowd the 
  *           application to choose a sub-area from the input cube to be place 
  *           into the mosaic. Defaults to number of samples in the cube
  * 
  * @param nl The number of lines from the input cube. This allowd the  
  *           application to choose a sub-area from the input cube to be place 
  *           into the mosaic. Defaults to number of lines in the cube
  * 
  * @param nb The number of bands from the input cube. This allowd the  
  *           application to choose a sub-area from the input cube to be place 
  *           into the mosaic. Defaults to number of bands in the cube
  * 
  * @throws Isis::iException::Message
  */
  Isis::Cube* ProcessMosaic::SetInputCube (const std::string &parameter,
                                               const int ss, const int sl, const int sb,
                                               const int ns, const int nl, const int nb) {
   
    // Make sure only one input is active at a time
    if (InputCubes.size() > 0) {
      string m = "You must specify exactly one input cube";
      throw Isis::iException::Message(Isis::iException::Programmer,m, _FILEINFO_);
    }
  
    p_iss = ss;
    p_isl = sl;
    p_isb = sb;
    p_ins = ns;
    p_inl = nl;
    p_inb = nb;
  
    return Isis::Process::SetInputCube (parameter);
  
  }
  
 /** 
  * Opens an input cube specified by the user. This method is overloaded and  
  * adds the requirement that only one input cube can be specified.
  * 
  * @return Cube*
  *
  * @param fname  
  * 
  * @param att 
  * 
  * @param ss The starting sample within the input cube. This allowd the 
  *           application to choose a sub-area from the input cube to be place
  *           into the mosaic. Defaults to 1
  * 
  * @param sl The starting line within the input cube. This allowd the 
  *           application to choose a sub-area from the input cube to be place 
  *           into the mosaic. Defaults to 1
  * 
  * @param sb The starting band within the input cube. This allowd the  
  *           application to choose a sub-area from the input cube to be place 
  *           into the mosaic. Defaults to 1
  * 
  * @param ns The number of samples from the input cube. This allowd the 
  *           application to choose a sub-area from the input cube to be place 
  *           into the mosaic. Defaults to number of samples in the cube
  * 
  * @param nl The number of lines from the input cube. This allowd the  
  *           application to choose a sub-area from the input cube to be place 
  *           into the mosaic. Defaults to number of lines in the cube
  * 
  * @param nb The number of bands from the input cube. This allowd the  
  *           application to choose a sub-area from the input cube to be place 
  *           into the mosaic. Defaults to number of bands in the cube
  * 
  * @throws Isis::iException::Message
  */
  Isis::Cube* ProcessMosaic::SetInputCube (const std::string &fname,
                                            Isis::CubeAttributeInput &att,
                                            const int ss, const int sl, const int sb,
                                            const int ns, const int nl, const int nb) {
  
    // Make sure only one input is active at a time
    if (InputCubes.size() > 0) {
      string m = "You must specify exactly one input cube";
      throw Isis::iException::Message(Isis::iException::Programmer,m, _FILEINFO_);
    }
  
    p_iss = ss;
    p_isl = sl;
    p_isb = sb;
    p_ins = ns;
    p_inl = nl;
    p_inb = nb;
  
    return Isis::Process::SetInputCube (fname, att);
  }
  
 /** 
  * Opens an output cube specified by the user. This method is overloaded and 
  * adds the requirement that only one output cube can be specified. The output 
  * cube must exist before calling SetOutputCube.
  * 
  * @return Cube*
  *
  * @param parameter User parameter to obtain file to open. Typically, the value 
  *                  is "TO". For example, the user can specify on the command  
  *                  line TO=mosaic.cub and this method will attempt to open the  
  *                  cube "mosaic.cub" if the parameter was set to "TO".
  * 
  * @throws Isis::iException::Message
  */
  Isis::Cube* ProcessMosaic::SetOutputCube (const std::string &parameter) {
  
    // Make sure there is only one output cube
    if (OutputCubes.size() > 0) {
      string m = "You must specify exactly one output cube";
      throw Isis::iException::Message(Isis::iException::Programmer,m, _FILEINFO_);
    }
  
    // Attempt to open a cube ... get the filename from the user parameter
    // (e.g., "TO") and the cube size from an input cube
    Isis::Cube *cube = new Isis::Cube;
    try {
      string fname = Application::GetUserInterface().GetFilename (parameter);
      cube->Open (fname,"rw");
    }
    catch (Isis::iException &e) {
      delete cube;
      throw;
    }
  
    // Everything is fine so save the cube on the stack
    OutputCubes.push_back (cube);
    return cube;
  }
} // end namespace isis
