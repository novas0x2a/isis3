#ifndef AtmosModel_h
#define AtmosModel_h
/**                                                                       
 * @file                                                                  
 * $Revision: 1.8 $                                                             
 * $Date: 2008/07/09 19:57:32 $                                                                 
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
#include <vector>
#include "Pvl.h"
#include "PhotoModel.h"

namespace Isis {

/**
 * @brief Isotropic atmos scattering model 
 *
 * @author 1998-12-21 Randy Kirk
 *
 * @internal
 *  @history 2007-07-31 Steven Lambright - Fixed unit test
 *  @history 2007-08-15 Steven Lambright - Refactored
 *  @history 2008-03-07 Janet Barrett - Added code to set standard
 *                      conditions. Also added bharef, hgaref, tauref,
 *                      and wharef variables and supporting methods.
 *  @history 2008-06-18 Christopher Austin - Fixed much documentation
 *  @history 2008-07-09 Steven Lambright - Fixed unit test
 */
  class AtmosModel {
    public:
      AtmosModel (Pvl &pvl, PhotoModel &pmodel);
      virtual ~AtmosModel() {};

      //! Calculate atmospheric scattering effect
      void CalcAtmEffect(double pha, double inc, double ema,
        double *pstd, double *trans, double *trans0, double *sbar);

      //! Used to calculate atmosphere at standard conditions
      virtual void SetStandardConditions(bool standard);

      //! Set parameters needed for atmospheric correction
      void SetAtmosBha(const double bha);
      void SetAtmosBharef(const double bharef);
      void SetAtmosTau (const double tau);
      void SetAtmosTauref(const double tauref);
      void SetAtmosWha (const double wha);
      void SetAtmosWharef(const double wharef);
      void SetAtmosHga (const double hga);
      void SetAtmosHgaref(const double hgaref);
      void SetAtmosAtmSwitch (const int atmswitch);
      void SetAtmosInc (const double inc);
      void SetAtmosPhi (const double phi);
      void SetAtmosNulneg(const std::string nulneg);

      //! Obtain hemispheric and bihemispheric albedo by
      //! integrating the photometric function
      void PhtGetAhTable();

      //! Perform integration for Hapke Henyey-Greenstein
      //! atmosphere correction
      void GetHahgTables();

      //! Function to be integrated
      double OutrFunc2Bint(double phi);

      //! Function to be integrated
      double InrFunc2Bint(double mu);

      //! Return atmospheric algorithm name
      std::string AlgorithmName () const { return p_atmosAlgorithmName; };

      //! Return atmospheric Bha value
      double AtmosBha () const { return p_atmosBha; };

      //! Return atmospheric Tau value
      double AtmosTau () const { return p_atmosTau; };

      //! Return atmospheric Wha value
      double AtmosWha () const { return p_atmosWha; };

      //! Return atmospheric Hga value
      double AtmosHga () const { return p_atmosHga; };

      //! Return atmospheric Bharef value
      double AtmosBharef () const { return p_atmosBharef; };

      //! Return atmospheric Hgaref value
      double AtmosHgaref () const { return p_atmosHgaref; };

      //! Return atmospheric Tauref value
      double AtmosTauref () const { return p_atmosTauref; };

      //! Return atmospheric Wharef value
      double AtmosWharef () const { return p_atmosWharef; };

      //! Return atmospheric Nulneg value
      bool AtmosNulneg () const { return p_atmosNulneg; };

      //! Return atmospheric Ab value
      double AtmosAb () const { return p_atmosAb; };

      //! Return atmospheric Ninc value
      int AtmosNinc () const { return p_atmosNinc; };

      //! Return atmospheric IncTable value
      double *AtmosIncTable () { return p_atmosIncTable; };

      //! Return atmospheric AhTable value
      double *AtmosAhTable () { return p_atmosAhTable; };

      //! Return atmospheric AhTable2 value
      double *AtmosAhTable2 () { return p_atmosAhTable2; };

      //! Return atmospheric Hahgsb value
      double AtmosHahgsb () const { return p_atmosHahgsb; };

      //! Return atmospheric HahgtTable value
      double *AtmosHahgtTable () { return p_atmosHahgtTable; };

      //! Return atmospheric Hahgt0Table value
      double *AtmosHahgt0Table () { return p_atmosHahgt0Table; };

      //! Return atmospheric HahgtTable2 value
      double *AtmosHahgtTable2 () { return p_atmosHahgtTable2; };

      //! Return atmospheric Hahgt0Table2 value
      double *AtmosHahgt0Table2 () { return p_atmosHahgt0Table2; };

      void r8qromb(int sub, double a, double b, double *ss);
      void r8trapzd(int sub, double a, double b, double *s, int n);
          
    protected:
      virtual void AtmosModelAlgorithm (double phase, 
            double incidence, double emission) = 0;
      
      void SetAlgorithmName(std::string name) { p_atmosAlgorithmName = name; }
      void SetAtmosNulneg(bool nulneg) { p_atmosNulneg = nulneg; }
      void SetOldTau(double tau) { p_atmosTauold = tau; }
      void SetOldWha(double wha) { p_atmosWhaold = wha; }

      PhotoModel *GetPhotoModel() const { return p_atmosPM; }
      bool StandardConditions() const { return p_standardConditions; }
      bool TauOrWhaChanged() const;
      double Eulgam() const { return p_atmosEulgam; }

      int p_atmosAtmSwitch;
      int p_atmosNinc;

      double p_atmosBha;
      double p_atmosBharef;
      double p_atmosBhasave;
      double p_atmosHgaref;
      double p_atmosHgasave;
      double p_atmosTauref;
      double p_atmosTausave;
      double p_atmosWharef;
      double p_atmosWhasave;

      double p_pstd;
      double p_trans;
      double p_trans0;
      double p_sbar;
      double p_atmosHga;
      double p_atmosTau;
      double p_atmosWha;
      double p_atmosAb;
      double p_atmosIncTable[91];
      double p_atmosAhTable[91];
      double p_atmosAhTable2[91];
      double p_atmosHahgsb;
      double p_atmosHahgtTable[91];
      double p_atmosHahgtTable2[91];
      double p_atmosHahgt0Table[91];
      double p_atmosHahgt0Table2[91];
      double p_atmosInc;
      double p_atmosPhi;
      double p_atmosMunot;
      double p_atmosSini;
      double p_atmosCosphi;
      double p_atmosEulgam;

    private:
      bool p_standardConditions;
    
      std::string p_atmosAlgorithmName;
 
      PhotoModel *p_atmosPM;

      bool p_atmosNulneg;

      double p_atmosTauold;
      double p_atmosWhaold;
  };
};

#endif
