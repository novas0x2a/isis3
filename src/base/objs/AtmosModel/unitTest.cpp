#include <iostream>
#include <cmath>
#include "PhotoModel.h"
#include "PhotoModelFactory.h"
#include "AtmosModel.h"
#include "AtmosModelFactory.h"
#include "Pvl.h"
#include "PvlGroup.h"
#include "Anisotropic1.h"
#include "iException.h"
#include "Preference.h"

using namespace Isis;
void doit(Pvl &lab, PhotoModel &pm);

int main () {
  Isis::Preference::Preferences(true);
  std::cout << "UNIT TEST for Isis::AtmosModel" << std::endl << std::endl; 

  Pvl lab;
  lab.AddObject(PvlObject("PhotometricModel"));
  lab.FindObject("PhotometricModel").AddGroup(PvlGroup("Algorithm"));
  lab.FindObject("PhotometricModel").FindGroup("Algorithm").AddKeyword(PvlKeyword("Name","Lambert"));
  PhotoModel *pm = PhotoModelFactory::Create(lab);
  
  std::cout << "Testing missing AtmosphericModel object ..." << std::endl;
  doit(lab,*pm);

  lab.AddObject(PvlObject("AtmosphericModel"));
  std::cout << "Testing missing Algorithm group ..." << std::endl;
  doit(lab,*pm);

  lab.FindObject("AtmosphericModel").AddGroup(PvlGroup("Algorithm"));
  std::cout << "Testing missing Name keyword ..." << std::endl;
  doit(lab,*pm);

  lab.FindObject("AtmosphericModel").FindGroup("Algorithm").AddKeyword(PvlKeyword("Name","Anisotropic1"), Pvl::Replace);
  std::cout << "Testing supported atmospheric model ..." << std::endl;
  doit(lab,*pm);

  AtmosModel *am = AtmosModelFactory::Create(lab,*pm);

  try {
    am->SetAtmosWha(0.98);
    std::cout << "Testing atmospheric model get methods ..." << std::endl;
    std::cout << "AlgorithmName = " << am->AlgorithmName() << std::endl;
    std::cout << "AtmosTau = " << am->AtmosTau() << std::endl;
    std::cout << "AtmosWha = " << am->AtmosWha() << std::endl;
    std::cout << "AtmosHga = " << am->AtmosHga() << std::endl;
    std::cout << "AtmosNulneg = " << am->AtmosNulneg() << std::endl;
    std::cout << "AtmosNinc = " << am->AtmosNinc() << std::endl;

    std::cout << std::endl;

    am->SetStandardConditions(true);
    std::cout << "Testing atmospheric model get methods in standard conditions..." << std::endl;
    std::cout << "AlgorithmName = " << am->AlgorithmName() << std::endl;
    std::cout << "AtmosTau = " << am->AtmosTau() << std::endl;
    std::cout << "AtmosWha = " << am->AtmosWha() << std::endl;
    std::cout << "AtmosHga = " << am->AtmosHga() << std::endl;
    std::cout << "AtmosNulneg = " << am->AtmosNulneg() << std::endl;
    std::cout << "AtmosNinc = " << am->AtmosNinc() << std::endl;
    am->SetStandardConditions(false);

    am->SetAtmosWha(0.95);
  } 
  catch (iException &e) {
    e.Report(false);
  }
  std::cout << std::endl;

  std::cout << "Testing boundary conditions of atmospheric model set methods ..." << std::endl;
  try {
    am->SetAtmosTau(-1.0);
  }
  catch (iException &e) {
    e.Report(false);
  }

  try {
    am->SetAtmosWha(0.0);
  }
  catch (iException &e) {
    e.Report(false);
  }

  try {
    am->SetAtmosWha(2.0);
  }
  catch (iException &e) {
    e.Report(false);
  }

  try {
    am->SetAtmosHga(-1.0);
  }
  catch (iException &e) {
    e.Report(false);
  }

  try {
    am->SetAtmosHga(1.0);
  }
  catch (iException &e) {
    e.Report(false);
  }

  std::cout << std::endl;

  std::cout << "Testing atmospheric model InrFunc2Bint method ..." 
      << std::endl;
  try {
    am->SetAtmosAtmSwitch(1);
    am->SetAtmosInc(0.0);
    am->SetAtmosPhi(0.0);
    am->SetAtmosHga(.68);
    am->SetAtmosTau(.28);
    double result = am->InrFunc2Bint(1.0e-6);
    std::cout << "Results from InrFunc2Bint = " << result <<
        std::endl << std::endl;
  }
  catch (iException &e) {
    e.Report(false);
  }
  std::cout << std::endl;

  try {
    am->SetAtmosAtmSwitch(2);
    am->SetAtmosInc(3.0);
    am->SetAtmosPhi(78.75);
    am->SetAtmosHga(.68);
    am->SetAtmosTau(.28);
    double result = am->InrFunc2Bint(.75000025);
    std::cout << "Results from InrFunc2Bint = " << result <<
        std::endl << std::endl;
  }
  catch (iException &e) {
    e.Report(false);
  }
  std::cout << std::endl;

  std::cout << "Testing atmospheric model r8trapzd method ..." 
      << std::endl;
  try {
    am->SetAtmosAtmSwitch(1);
    am->SetAtmosInc(0.0);
    am->SetAtmosPhi(0.0);
    am->SetAtmosHga(.68);
    am->SetAtmosTau(.28);
    double ss;
    am->r8trapzd(0,0.,180.,&ss,1);
    std::cout << "Results from r8trapzd = " << ss <<
        std::endl << std::endl;
  }
  catch (iException &e) {
    e.Report(false);
  }

  std::cout << "Testing atmospheric model OutrFunc2Bint method ..." 
      << std::endl;
  try {
    am->SetAtmosAtmSwitch(1);
    am->SetAtmosInc(0.0);
    am->SetAtmosPhi(0.0);
    am->SetAtmosHga(.68);
    am->SetAtmosTau(.28);
    double result = am->OutrFunc2Bint(0.0);
    std::cout << "Results from OutrFunc2Bint = " << result <<
        std::endl << std::endl;
  }
  catch (iException &e) {
    e.Report(false);
  }
  std::cout << std::endl;

  std::cout << "Testing atmospheric model r8qromb method ..." 
      << std::endl;
  try {
    am->SetAtmosAtmSwitch(1);
    am->SetAtmosInc(0.0);
    am->SetAtmosPhi(0.0);
    am->SetAtmosHga(.68);
    am->SetAtmosTau(.28);
    double ss;
    am->r8qromb(0,0.,180.,&ss);
    std::cout << "Results from r8qromb = " << ss <<
        std::endl << std::endl;
  }
  catch (iException &e) {
    e.Report(false);
  }
  std::cout << std::endl;

  std::cout << "Testing atmospheric model PhtGetAhTable method ..." 
      << std::endl;
  try {
    am->PhtGetAhTable();
    std::cout << "Results from PhtGetAhTable = " << std::endl;
    std::cout << "Ab = " << am->AtmosAb() << std::endl;
    int ninc = am->AtmosNinc();
    double *inctable;
    inctable = am->AtmosIncTable();
    for (int i=0; i<ninc; i++) {
      std::cout << "i IncTable(i) = " << i << " " << inctable[i] <<
          std::endl;
    }
    double *ahtable;
    ahtable = am->AtmosAhTable();
    for (int i=0; i<ninc; i++) {
      std::cout << "i AhTable(i) = " << i << " " << ahtable[i] <<
          std::endl;
    }
    double *ahtable2;
    ahtable2 = am->AtmosAhTable2();
    for (int i=0; i<ninc; i++) {
      // neglect rounding error. The r8spline call in AtmosModel introduces this.
      if(fabs(ahtable2[i]) < 5E-9) {
        ahtable2[i] = 0.0;
      }
      std::cout << "i AhTable2(i) = " << i << " " << ahtable2[i] <<
          std::endl;
    }
  }
  catch (iException &e) {
    e.Report(false);
  }
  std::cout << std::endl;

  std::cout << "Testing atmospheric model GetHahgTables method ..." 
      << std::endl;
  try {
    am->SetAtmosWha(.95);
    am->SetAtmosInc(0.0);
    am->SetAtmosPhi(0.0);
    am->SetAtmosHga(.68);
    am->SetAtmosTau(.28);
    am->GetHahgTables();
    std::cout << "Results from GetHahgTables = " << std::endl;
    std::cout << "Hahgsb = " << am->AtmosHahgsb() << std::endl;
    int ninc = am->AtmosNinc();
    double *inctable;
    inctable = am->AtmosIncTable();
    for (int i=0; i<ninc; i++) {
      std::cout << "i IncTable(i) = " << i << " " << inctable[i] <<
          std::endl;
    }
    double *hahgttable;
    hahgttable = am->AtmosHahgtTable();
    for (int i=0; i<ninc; i++) {
      std::cout << "i HahgtTable(i) = " << i << " " << hahgttable[i] <<
          std::endl;
    }
    double *hahgt0table;
    hahgt0table = am->AtmosHahgt0Table();
    for (int i=0; i<ninc; i++) {
      // neglect rounding error.
      if(fabs(hahgt0table[i]) < 1E-16) {
        hahgt0table[i] = 0.0;
      }
      std::cout << "i Hahgt0Table(i) = " << i << " " << hahgt0table[i] <<
          std::endl;
    }
    double *hahgttable2;
    hahgttable2 = am->AtmosHahgtTable2();
    for (int i=0; i<ninc; i++) {
      std::cout << "i HahgtTable2(i) = " << i << " " << hahgttable2[i] <<
          std::endl;
    }
    double *hahgt0table2;
    hahgt0table2 = am->AtmosHahgt0Table2();
    for (int i=0; i<ninc; i++) {
      std::cout << "i Hahgt0Table2(i) = " << i << " " << hahgt0table2[i] <<
          std::endl;
    }
  }
  catch (iException &e) {
    e.Report(false);
  }

  std::cout << std::endl;
}

void doit(Pvl &lab, PhotoModel &pm) {
  try {
    //AtmosModel *am = AtmosModelFactory::Create(lab,pm);
    AtmosModelFactory::Create(lab,pm);
  }
  catch (iException &error) {
    error.Report(false);
  }

  std::cout << std::endl;
}
