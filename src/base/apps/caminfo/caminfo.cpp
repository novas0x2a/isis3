#include "Isis.h"

#include <cstdio>
#include <cmath>
#include <string>


#include "UserInterface.h"
#include "CubeAttribute.h"
#include "Filename.h"
#include "iException.h"
#include "Histogram.h"
#include "Pvl.h"
#include "Process.h"
#include "iTime.h"
#include "iString.h"
#include "ImagePolygon.h"
#include "SerialNumber.h"
#include "OriginalLabel.h"
#include "SpecialPixel.h"

using namespace std; 
using namespace Isis;

static inline PvlKeyword ValidateKey(const std::string keyname, 
                                     const double &value, 
                                     const std::string &unit = "") { 
  if (IsSpecial(value)) {
    return (PvlKeyword(keyname,"NULL"));
  }
  else {
    return (PvlKeyword(keyname,value,unit));
  }
}

static inline PvlKeyword ValidateKey(const std::string keyname, PvlKeyword &key,
                              const std::string &unit = "") { 
  if (key.IsNull()) {
    return (PvlKeyword(keyname,"NULL"));
  }
  else {
    return (ValidateKey(keyname,(double) key,unit));
  }
}

inline double DegToRad(const double ang) { return (ang * rpd_c()); }
inline double RadToDeg(const double ang) { return (ang * dpr_c()); }


void IsisMain ()
{
  Process p;
  // Grab the file to import
  UserInterface &ui = Application::GetUserInterface();
  string from = ui.GetAsString("FROM");
  Filename in = ui.GetFilename("FROM");
  bool doCamstat = ui.GetBoolean("CAMSTATS");

  Pvl pout;
  // if true then run spiceinit, xml default is FALSE
  //spiceinit will use system kernels
  if (ui.GetBoolean("SPICE")) {
    string parameters = "FROM=" + in.Expanded();
    Isis::iApp->Exec("spiceinit",parameters);
  }

  Cube *icube = p.SetInputCube("FROM");

  // get some common things like #line, #samples, bands.
  PvlObject params("Parameters");
  PvlObject common("Common");
  common += PvlKeyword("IsisId",SerialNumber::Compose(*icube));
  common += PvlKeyword("From",icube->Filename());
  common += PvlKeyword("Lines",icube->Lines());
  common += PvlKeyword("Samples",icube->Samples());
  common += PvlKeyword("Bands",icube->Bands());


  // Run camstats on the entire image (all bands)
  // another camstats will be run for each band and output
  // for each band.
  Pvl camPvl;    //  This becomes useful if there is only one band, which is
                 //  frequent!  Used below if single band image.
  if (doCamstat) {
    int linc = ui.GetInteger("LINC");
    int sinc = ui.GetInteger("SINC");
    Filename tempCamPvl;
    tempCamPvl.Temporary(in.Basename()+"_", "pvl");
    string pvlOut = tempCamPvl.Expanded();
    PvlGroup pcband("CubeCamStats");
    //set up camstats run and execute
    string parameters = "FROM=" + from + 
                        " TO=" + pvlOut +
                        " LINC=" + iString(linc) +
                        " SINC=" + iString(sinc);

    Isis::iApp->Exec("camstats",parameters);
    //out put to common object of the PVL
    camPvl.Read(pvlOut);
    remove(pvlOut.c_str());

    PvlGroup cg = camPvl.FindGroup("Latitude",Pvl::Traverse);
    pcband += ValidateKey("MinimumLatitude",cg["latitudeminimum"]);
    pcband += ValidateKey("MaximumLatitude",cg["latitudemaximum"]);
    cg = camPvl.FindGroup("Longitude",Pvl::Traverse);
    pcband += ValidateKey("MinimumLongitude",cg["longitudeminimum"]);
    pcband += ValidateKey("MaximumLongitude",cg["longitudemaximum"]);
    cg = camPvl.FindGroup("Resolution",Pvl::Traverse);
    pcband += ValidateKey("MinimumResolution",cg["resolutionminimum"]);
    pcband += ValidateKey("MaximumResolution",cg["resolutionmaximum"]);
    cg = camPvl.FindGroup("PhaseAngle",Pvl::Traverse);
    pcband += ValidateKey("MinimumPhase",cg["phaseminimum"]);
    pcband += ValidateKey("MaximumPhase",cg["phasemaximum"]);
    cg = camPvl.FindGroup("EmissionAngle",Pvl::Traverse);
    pcband += ValidateKey("MinimumEmission",cg["emissionminimum"]);
    pcband += ValidateKey("MaximumEmission",cg["emissionmaximum"]);
    cg = camPvl.FindGroup("IncidenceAngle",Pvl::Traverse);
    pcband += ValidateKey("MinimumIncidence",cg["incidenceminimum"]);
    pcband += ValidateKey("MaximumIncidence",cg["incidencemaximum"]);
    cg = camPvl.FindGroup("LocalSolarTime",Pvl::Traverse);
    pcband += ValidateKey("LocalTimeMinimum",cg["localsolartimeMinimum"]);
    pcband += ValidateKey("LocalTimeMaximum",cg["localsolartimeMaximum"]);        

    common.AddGroup(pcband);
  }
  params.AddObject(common);

  //  Add the input ISIS label if requested
  if ( ui.GetBoolean("ISISLABEL") ) {
    Pvl label = *(icube->Label());
    label.SetName("IsisLabel");
    params.AddObject(label);
  }

  // write out the orginal label blob
  if (ui.GetBoolean("ORIGINALLABEL")) {
    OriginalLabel orig;
    icube->Read(orig);
    Pvl p = orig.ReturnLabels();
    p.SetName("OriginalLabel");
    params.AddObject(p);
  }

  Camera *cam = icube->Camera();
  int eband  = cam->Bands();

  // for geometry, stats, camstats, or polygon get the info for each band
  if (ui.GetBoolean("GEOMETRY") || ui.GetBoolean("STATISTICS") || 
      ui.GetBoolean("CAMSTATS") || ui.GetBoolean("POLYGON")) {
    PvlObject bSet("BandSet");
    for (int band =1; band <= eband; band++) {
      int realBand = icube->PhysicalBand(band);
      double cLine = icube->Lines();
      double cSamp = icube->Samples();
      double centerLine = cLine / 2;
      double centerSamp = cSamp / 2;
      if (cam->SetImage(centerSamp,centerLine)) {
        cam->SetBand(band);
        PvlGroup pband("Band");
        pband += PvlKeyword("BandNumber",realBand); 
        if (ui.GetBoolean("GEOMETRY")) {
          double centerlatitude = cam->UniversalLatitude();
          double centerlongitude= cam->UniversalLongitude();

          double sampRes = cam->SampleResolution();
          double lineRes = cam->LineResolution();

          double solarlongitude = cam->SolarLongitude();
          double northazimuth = cam->NorthAzimuth();
          double offnader = cam->OffNadirAngle();
          double subsolarazimuth = cam->SunAzimuth();
          double subspacecraftazimuth = cam->SpacecraftAzimuth();
          double localsolartime = cam->LocalSolarTime();
          double targetcenterdistance = cam->TargetCenterDistance();
          double slantdistance = cam->SlantDistance();

          double subsolarlatitude, subsolarlongitude;
          cam->SubSolarPoint(subsolarlatitude,subsolarlongitude);

          double subspacecraftlatitude, subspacecraftlongitude;
          cam->SubSpacecraftPoint(subspacecraftlatitude,subspacecraftlongitude);

          iTime t1(cam->CacheStartTime());
          string starttime = t1.UTC();

          iTime t2(cam->CacheEndTime());
          string endtime = t2.UTC();

          //  solve for the parallax and shadow stuff
          double phase = cam->PhaseAngle();
          double emi   = cam->EmissionAngle();
          double inc   = cam->IncidenceAngle();


           //  Need some radian values
          double parallaxx(Null), parallaxy(Null);
          if ( !IsSpecial(emi) && !IsSpecial(subspacecraftazimuth) ) {
            double emi_r = DegToRad(emi);
            parallaxx = RadToDeg(-tan(emi_r)*cos(DegToRad(subspacecraftazimuth)));
            parallaxy = RadToDeg(tan(emi_r)*sin(DegToRad(subspacecraftazimuth)));
           }

          double shadowx(Null), shadowy(Null);
          if ( !IsSpecial(inc) && !IsSpecial(subsolarazimuth) ) {
            double inc_r = DegToRad(inc);
            shadowx = RadToDeg(-tan(inc_r)*cos(DegToRad(subsolarazimuth)));
            shadowy = RadToDeg(tan(inc_r)*sin(DegToRad(subsolarazimuth)));
          }

          //  OK...now get corner pixel geometry.  NOTE this resets image
          //  pixel location from center!!!
          double upperLeftLongitude(Null), upperLeftLatitude(Null);
          if ( cam->SetImage(1.0, 1.0) ) {
            upperLeftLongitude = cam->UniversalLongitude();
            upperLeftLatitude =  cam->UniversalLatitude();
          }

          double lowerLeftLongitude(Null), lowerLeftLatitude(Null);
          if ( cam->SetImage(1.0, cLine) ) {
            lowerLeftLongitude = cam->UniversalLongitude();
            lowerLeftLatitude =  cam->UniversalLatitude();
          }

          double lowerRightLongitude(Null), lowerRightLatitude(Null);
          if ( cam->SetImage(cSamp, cLine) ) {
            lowerRightLongitude = cam->UniversalLongitude();
            lowerRightLatitude =  cam->UniversalLatitude();
          }

          double upperRightLongitude(Null), upperRightLatitude(Null);
          if ( cam->SetImage(cSamp, 1.0) ) {
            upperRightLongitude = cam->UniversalLongitude();
            upperRightLatitude =  cam->UniversalLatitude();
          }

          //geometry keywords for band output
          pband += PvlKeyword("StartTime",starttime);
          pband += PvlKeyword("EndTime",endtime);

          pband += ValidateKey("CenterLatitude",centerlatitude);
          pband += ValidateKey("CenterLongitude",centerlongitude);

          pband += ValidateKey("UpperLeftLongitude",upperLeftLongitude);
          pband += ValidateKey("UpperLeftLatitude",upperLeftLatitude);
          pband += ValidateKey("LowerLeftLongitude",lowerLeftLongitude);
          pband += ValidateKey("LowerLeftLatitude",lowerLeftLatitude);
          pband += ValidateKey("LowerRightLongitude",lowerRightLongitude);
          pband += ValidateKey("LowerRightLatitude",lowerRightLatitude);
          pband += ValidateKey("UpperRightLongitude",upperRightLongitude);
          pband += ValidateKey("UpperRightLatitude",upperRightLatitude);

          pband += ValidateKey("PhaseAngle",phase);
          pband += ValidateKey("EmissionAngle",emi);
          pband += ValidateKey("IncidenceAngle",inc);

          pband += ValidateKey("NorthAzimuth",northazimuth);
          pband += ValidateKey("OffNadir",offnader);
          pband += ValidateKey("SolarLongitude",solarlongitude);
          pband += ValidateKey("LocalTime",localsolartime);
          pband += ValidateKey("TargetCenterDistance",targetcenterdistance);
          pband += ValidateKey("SlantDistance",slantdistance);

          double aveRes = (sampRes + lineRes) / 2.0;
          pband += ValidateKey("SampleResolution",sampRes);
          pband += ValidateKey("LineResolution",lineRes);
          pband += ValidateKey("PixelResolution",aveRes);

          pband += ValidateKey("SubSolarAzimuth",subsolarazimuth);
          pband += ValidateKey("SubSolarLatitude",subsolarlatitude);
          pband += ValidateKey("SubSolarLongitude",subsolarlongitude);

          pband += ValidateKey("SubSpacecraftAzimuth",subspacecraftazimuth);
          pband += ValidateKey("SubSpacecraftLatitude",subspacecraftlatitude);
          pband += ValidateKey("SubSpacecraftLongitude",subspacecraftlongitude);

          pband += ValidateKey("ParallaxX",parallaxx);
          pband += ValidateKey("ParallaxY",parallaxy);

          pband += ValidateKey("ShadowX",shadowx);
          pband += ValidateKey("ShadowY",shadowy);

          //  Determine if image crosses Longitude domain
          Pvl camMap;
          cam->BasicMapping(camMap);
          if ( cam->IntersectsLongitudeDomain(camMap) ) {
            pband += PvlKeyword("HasLongitudeBoundary","TRUE");
          }
          else {
            pband += PvlKeyword("HasLongitudeBoundary","FALSE");
          }

          //  Add test for North pole in image
          if (cam->SetUniversalGround(90.0, 0.0)) {
            pband += PvlKeyword("HasNorthPole", "TRUE");
          }
          else {
            pband += PvlKeyword("HasNorthPole", "FALSE");
          }

          //  Add test for South pole in image
          if (cam->SetUniversalGround(-90.0, 0.0)) {
            pband += PvlKeyword("HasSouthPole", "TRUE");
          }
          else {
            pband += PvlKeyword("HasSouthPole", "FALSE");
          }
        }

        bool stat = ui.GetBoolean("STATISTICS");
        if (stat) {
          //use hisogram to gather DN stats.
          Histogram *hist = icube->Histogram(band);
          double nullpercent = (hist->NullPixels()/(cLine * cSamp))*100;      
          double hispercent = (hist->HisPixels()/(cLine * cSamp))*100;
          double hrspercent = (hist->HrsPixels()/(cLine * cSamp))*100;
          double lispercent = (hist->LisPixels()/(cLine * cSamp))*100;
          double lrspercent = (hist->LrsPixels()/(cLine * cSamp))*100;
          //statitics keyword output for band
          pband += ValidateKey("MeanValue",hist->Average());
          pband += ValidateKey("MedianValue",hist->Median());
          pband += ValidateKey("StandardDeviation",hist->StandardDeviation());
          pband += ValidateKey("MinimumValue",hist->Minimum());
          pband += ValidateKey("MaximumValue",hist->Maximum());
          pband += PvlKeyword("PercentHIS",hispercent);      
          pband += PvlKeyword("PercentHRS",hrspercent);      
          pband += PvlKeyword("PercentLIS",lispercent);      
          pband += PvlKeyword("PercentLRS",lrspercent);      
          pband += PvlKeyword("PercentNull",nullpercent);

        }

        // run camstats for the bands
        if (doCamstat) {
          // Its already been run once for the entire file.  If there is only
          // on band in the file, we have saved the output so we don't have to
          // run it again.
          if (icube->Bands() != 1 ) {
            int linc = ui.GetInteger("LINC");
            int sinc = ui.GetInteger("SINC");
            Filename tempCamPvl;
            tempCamPvl.Temporary(in.Basename()+"_", "pvl");
            string pvlOut = tempCamPvl.Expanded();
            string parameters = "FROM=" + icube->Filename() +"+"+ iString(realBand) +
                                " TO=" + pvlOut +
                                " LINC=" + iString(linc) +
                                " SINC=" + iString(sinc);

            Isis::iApp->Exec("camstats",parameters);
            //camstats keywords for band output
            camPvl.Read(pvlOut);
            remove(pvlOut.c_str());
          }

          PvlGroup g = camPvl.FindGroup("Latitude",Pvl::Traverse);
          pband += ValidateKey("MinimumLatitude",g["latitudeminimum"]);
          pband += ValidateKey("MaximumLatitude",g["latitudemaximum"]);
          g = camPvl.FindGroup("Longitude",Pvl::Traverse);
          pband += ValidateKey("MinimumLongitude",g["longitudeminimum"]);
          pband += ValidateKey("MaximumLongitude",g["longitudemaximum"]);
          g=camPvl.FindGroup("Resolution",Pvl::Traverse);
          pband += ValidateKey("MinimumResolution",g["resolutionminimum"]);
          pband += ValidateKey("MaximumResolution",g["resolutionmaximum"]);
          g = camPvl.FindGroup("PhaseAngle",Pvl::Traverse);
          pband += ValidateKey("MinimumPhase",g["phaseminimum"]);
          pband += ValidateKey("MaximumPhase",g["phasemaximum"]);
          g = camPvl.FindGroup("EmissionAngle",Pvl::Traverse);
          pband += ValidateKey("MinimumEmission",g["emissionminimum"]);
          pband += ValidateKey("MaximumEmission",g["emissionmaximum"]);
          g = camPvl.FindGroup("IncidenceAngle",Pvl::Traverse);
          pband += ValidateKey("MinimumIncidence",g["incidenceminimum"]);
          pband += ValidateKey("MaximumIncidence",g["incidencemaximum"]);
          g = camPvl.FindGroup("LocalSolarTime",Pvl::Traverse);
          pband += ValidateKey("LocalTimeMinimum",g["localsolartimeMinimum"]);
          pband += ValidateKey("LocalTimeMaximum",g["localsolartimeMaximum"]);        
        }

        // if polygon is true then use imagePloygon to get polygon information
        if (ui.GetBoolean("POLYGON")) {
          try {
            PvlGroup pvlPoly ("Polygon");
            ImagePolygon poly(icube->Filename());
            poly.Create(*icube,0,1,1,0,0,band);
            geos::geom::MultiPolygon *multiP = poly.Polys();
            pband += PvlKeyword("GisFootprint", multiP->toString()); 
          } 
          catch (iException &ie  ) {
            string msg = "Failed to generate polygon";
            ie.Message(iException::Programmer,msg,_FILEINFO_);
            throw;
          }
        }
        bSet.AddGroup(pband);
      }
      else {
        if (ui.GetBoolean("VCAMERA")) {
          string msg = "Image center does not project in camera model";
          throw iException::Message(iException::Camera,msg,_FILEINFO_);
        }
      }
    }
    params.AddObject(bSet);
  }
  string outFile = ui.GetFilename("TO");
  pout.AddObject(params);
  pout.Write(outFile);
}


