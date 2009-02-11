#ifndef ProcessExportPds_h
#define ProcessExportPds_h
/*
 *   Unless noted otherwise, the portions of Isis written by the
 *   USGS are public domain. See individual third-party library
 *   and package descriptions for intellectual property
 *   information,user agreements, and related information.
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

#include "ProcessExport.h"

namespace Isis {
  class PvlFormatPds;

  /**
   * @brief Process class for exporting cubes to PDS standards
   * 
   * This class extends the ProcessExport class to allow the user
   * to export cubes to PDS format.
   * 
   * @history 2006-09-05 Stuart Sides - Original version
   * @history 2006-12-14 Stuart Sides - Modified keword units to be PDS complient
   * @history 2008-05-20 Steven Lambright - Fixed documentation 
   * @history 2008-08-07 Christopher Austin - Added fixed label export capability 
   * @history 2008-10-02 Christopher Austin - Fixed LabelSize() and OutputLabel() 
   *          in accordace to the pds end of line sequence requirement
   * @history 2008-12-17 Steven Lambright - Added calculations for OFFSET and 
   *          SCALEFACTOR keywords
   */
  class ProcessExportPds : public Isis::ProcessExport {
    public:
      ProcessExportPds();
      ~ProcessExportPds();
      enum PdsFileType {Image, Qube, SpectralQube};
      enum PdsExportType {Stream, Fixed};

      // Work with PDS labels for all data set types
      void StandardAllMapping(Pvl &mainPvl);

      // Work with PDS labels for data sets of type IMAGE
      void StreamImageRoot(Pvl &mainPvl);
      void FixedImageRoot(Pvl &mainPvl);
      void StandardImageImage(Pvl &mainPvl);

      void SetExportType( PdsExportType type ) { p_exportType = type; }

      Pvl& StandardPdsLabel(PdsFileType type);
      void OutputLabel(std::ofstream &os);

    protected:
      int LineBytes();
      int LabelSize();
      void CreateImageLabel();
      void CreateQubeLabel();
      void CreateSpectralQubeLabel();

      std::string ProjectionName(Pvl &inputLabel);

    private:
      PvlFormatPds *p_formatter;
      Pvl *p_label;
      PdsExportType p_exportType;
  };
}

#endif
