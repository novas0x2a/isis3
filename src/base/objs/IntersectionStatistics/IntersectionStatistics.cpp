/**
 * @file
 * $Revision: 1.1 $
 * $Date: 2009/05/07 00:16:12 $
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

#include "IntersectionStatistics.h"
#include "iException.h"
#include "PvlGroup.h"
#include "PvlKeyword.h"
#include "Pvl.h"
#include <iomanip>

using namespace std;
namespace Isis {
      
 /**
  * Constructs an IntersectionStatistics object.  Compares and 
  * stores the two vectors, and initializes the basis and least 
  * squares functions. 
  *
  * @param statsList The list of Statistics objects corresponding 
  *                  to specific identifiers
  * @param idList The list of strings acting as names for the 
  *               Statistics objects
  *
  * @throws Isis::iException::User - The two lists must be 
  *                                  the same size
  */
  IntersectionStatistics::IntersectionStatistics (vector<Statistics> statsList, 
                                                  vector<string> idList) {
    if (statsList.size() != idList.size()) {
      string msg = "The list of statistics and the list of file ";
      msg += "identifiers must be the same size.";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    }

    p_statsList = statsList;
    p_idList = idList;   

    p_gainFunction = new BasisFunction ("BasisFunction",
                                        idList.size(),idList.size());
    p_gainLsq = new LeastSquares (*p_gainFunction);

    p_offsetFunction = new BasisFunction ("BasisFunction",
                                          idList.size(),idList.size());
    p_offsetLsq = new LeastSquares (*p_offsetFunction); 
      
    p_solved = false;
  }   

 /**
  * Attempts to add overlap data to a collection of valid 
  * overlaps, and returns the success or failure of that attempt.
  *
  * @param area1 The statistics for the overlap area of the first 
  *              file
  * @param id1 The identifying name of the first file 
  * @param area2 The statistics for the overlap area of the 
  *              second file
  * @param id2 The identifying name of the second file 
  * @param weight Relative significance of this overlap.  Default 
  *               value = 1.0
  *  
  * @return AddStatus An enumeration representing either a 
  *         successful add, or the reason for failure
  *
  * @throws Isis::iException::User - Identifying strings must 
  *             exist in the idList 
  */
  IntersectionStatistics::AddStatus IntersectionStatistics::AddIntersection (
                                const Statistics &area1, const std::string id1,
                                const Statistics &area2, const std::string id2,
                                double weight) {
    bool found = false;
    for (unsigned i=0; i<p_idList.size(); i++) {
      string curId = p_idList.at(i);
      if (curId.compare(id1) == 0) {
        found = true;
      }
    }
    if (!found) {
      string msg = "The file identifier [" + id1 + "] does not exist in the " +
                        "list of possible identifiers.";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    }

    found = false;
    for (unsigned i=0; i<p_idList.size(); i++) {
      string curId = p_idList.at(i);
      if (curId.compare(id2) == 0) {
        found = true;
      }
    }
    if (!found) {
      string msg = "The file identifier [" + id2 + "] does not exist in the " +
                        "list of possible identifiers.";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    } 

    // if there is no overlapping area, then the overlap is invalid
    if (area1.ValidPixels() == 0 || area2.ValidPixels() == 0) {
      return NoOverlap;    
    }

    // the weight must be a positive real number 
    if (weight <= 0.0) {
      return BadWeight;
    } 

    IntersectionStatistics::Intersection o;
    o.area1 = area1;
    o.area2 = area2;
    o.id1 = id1;
    o.id2 = id2;

    double avg1 = area1.Average();
    double avg2 = area2.Average();

    // averages must not be 0 to avoid messing up least squares
    if (avg1 == 0 || avg2 == 0) return NoContrast;

    intersectionList.push_back(o);    
    p_deltas.push_back(avg2 - avg1);
    p_weights.push_back(weight); 
    return Success;
  }

 /**
  * Sets the list of files to be held during the solving process.
  *
  * @param idHoldList The list of files to be held 
  *  
  * @throws Isis::iException::User - The size of the hold list 
  *             must be less than or equal to that of the list of
  *             all identifiers
  */
  void IntersectionStatistics::SetHoldList(vector<string> idHoldList) {
    if (idHoldList.size() > p_idList.size()) {
      string msg = "The list of identifiers to be held must be less than or ";
      msg += "equal to the total number of identitifers.";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    }

    p_idHoldList = idHoldList;
  }

 /**
  * Attempts to solve the least squares equation for the data 
  * set, and returns the success or failure of that attempt. 
  *
  * @param type The enumeration clarifying whether the offset, 
  *             gain, or both should be solved for here
  *  
  * @return bool Is the least squares equation now solved 
  */
  bool IntersectionStatistics::Solve(SolutionType type) {
    // Make sure the number of valid overlaps + hold images is greater than the
    // number of input images (otherwise the least squares equation will be
    // unsolvable due to having more unknowns than knowns)
    if (intersectionList.size() + p_idHoldList.size() < p_idList.size()) {
      return false;
    }

    PvlObject equ("EqualizationInformation");

    PvlGroup d("DeltaValues");
    PvlGroup w("Weights");
    for (int o=0; o<(int)intersectionList.size(); o++) {
      d += PvlKeyword(iString(o+1),p_deltas[o]);
      w += PvlKeyword(iString(o+1),p_weights[o]);
    }
    equ.AddGroup(d);
    equ.AddGroup(w);

    // Calculate offset
    if (type != Gain) {
      // Add knowns to least squares for each overlap
      for (int overlap=0; overlap<(int)intersectionList.size(); overlap++) {
        int id1 = -1;
        int id2 = -1;
        Intersection curIntersection = intersectionList[overlap];
        string overlapId = curIntersection.id1;
        for (unsigned i=0; i<p_idList.size(); i++) {
          string curId = p_idList.at(i);
          if (curId.compare(overlapId) == 0) {
            id1 = i;
          }
        }
        overlapId = curIntersection.id2;
        for (unsigned i=0; i<p_idList.size(); i++) {
          string curId = p_idList.at(i);
          if (curId.compare(overlapId) == 0) {
            id2 = i;
          }
        }

        if (id1 == -1 || id2 == -1) return false;

        vector<double> input;
        input.resize(p_idList.size());
        for (int i=0; i<(int)input.size(); i++) input[i] = 0.0;
        input[id1] = 1.0;
        input[id2] = -1.0;
        p_offsetLsq->AddKnown(input,p_deltas[overlap],p_weights[overlap]);
      }

      // Add a known to the least squares for each hold image
      for (int h=0; h<(int)p_idHoldList.size(); h++) {
        int id = -1;
        string hold = p_idHoldList[h];
        for (unsigned i=0; i<p_idList.size(); i++) {
          string curId = p_idList.at(i);
          if (curId.compare(hold) == 0) {
            id = i;
          }
        }
        
        if (id == -1) return false;

        vector<double> input;
        input.resize(p_idList.size());
        for (int i=0; i<(int)input.size(); i++) input[i] = 0.0;
        input[id] = 1.0;
        p_offsetLsq->AddKnown(input,0.0);
      }

      // Solve the least squares and get the offset coefficients to apply to the
      // images
      p_offsets.resize(p_idList.size());
      p_offsetLsq->Solve(Isis::LeastSquares::QRD);
      PvlGroup b("Base");
      for (int i=0; i<p_offsetFunction->Coefficients(); i++) {
        p_offsets[i] = p_offsetFunction->Coefficient(i);
        b += PvlKeyword(p_idList[i],p_offsets[i]);
      }
      equ.AddGroup(b);
    }

    // Calculate Gain
    if (type != Offset) {
      // Add knowns to least squares for each overlap
      PvlGroup p21("P21Values");
      for (int overlap=0; overlap<(int)intersectionList.size(); overlap++) {
        int id1 = -1;
        int id2 = -1;
        Intersection curIntersection = intersectionList[overlap];
        string overlapId = curIntersection.id1;
        for (unsigned i=0; i<p_idList.size(); i++) {
          string curId = p_idList.at(i);
          if (curId.compare(overlapId) == 0) {
            id1 = i;
          }
        }
        overlapId = curIntersection.id2;
        for (unsigned i=0; i<p_idList.size(); i++) {
          string curId = p_idList.at(i);
          if (curId.compare(overlapId) == 0) {
            id2 = i;
          }
        }

        if (id1 == -1 || id2 == -1) return false;

        vector<double> input;
        input.resize(p_idList.size());
        for (int i=0; i<(int)input.size(); i++) input[i] = 0.0;
        input[id1] = 1.0;
        input[id2] = -1.0; 

        double tanp;

        if (curIntersection.area1.StandardDeviation() == 0.0) {
          tanp = 0.0;    //Set gain to 1.0      
        }
        else {
          tanp = curIntersection.area2.StandardDeviation()
              / curIntersection.area1.StandardDeviation();
        }

        p21 += PvlKeyword(iString(overlap+1),tanp);
        if (tanp > 0.0) {
          p_gainLsq->AddKnown(input,log(tanp),p_weights[overlap]);
        }
        else {
          p_gainLsq->AddKnown(input,0.0,1e30);   //Set gain to 1.0
        }
      }
      equ.AddGroup(p21);
  
      // Add a known to the least squares for each hold image
      for (int h=0; h<(int)p_idHoldList.size(); h++) {
        int id = -1;
        string hold = p_idHoldList[h];
        for (unsigned i=0; i<p_idList.size(); i++) {
          string curId = p_idList.at(i);
          if (curId.compare(hold) == 0) {
            id = i;
          }
        }
        
        if (id == -1) return false;

        vector<double> input;
        input.resize(p_idList.size());
        for (int i=0; i<(int)input.size(); i++) input[i] = 0.0;
        input[id] = 1.0;
        p_gainLsq->AddKnown(input,0.0);
      }

      //Solve the least squares and get the gain coefficients to apply to the
      // images
      p_gains.resize(p_idList.size());
      p_gainLsq->Solve(Isis::LeastSquares::QRD);
      PvlGroup m("Multiplier");
      for (int i=0; i<p_gainFunction->Coefficients(); i++) {
        p_gains[i] = exp(p_gainFunction->Coefficient(i));
        m += PvlKeyword(p_idList[i], p_gains[i]);
      }
      equ.AddGroup(m);
    }    

    p_equ = equ;
    p_solved = true;
    return true;
  }

 /**
  * Returns the calculated gain (multiplier) for the given 
  * identifier 
  *  
  * @param id The identifying name of the file whose calculated 
  *           gain is to be returned here
  *
  * @return double The gain for the identifier 
  *  
  * @throws Isis::iException::User - Identifying string must 
  *             exist in the idList
  * @throws Isis::iException::User - Least Squares equation must 
  *             be solved before returning the gain
  */
  double IntersectionStatistics::GetGain (const std::string id) const {
    if (!p_solved) {
      string msg = "The least squares equation has not been successfully ";
      msg += "solved yet.";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    }

    int index = -1;
    for (unsigned i=0; i<p_idList.size(); i++) {
      string curId = p_idList.at(i);
      if (curId.compare(id) == 0) {
        index = i;
      }
    }
    if (index == -1) {
      string msg = "The id [" + id + "] was not found in the idList provided.";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    } 

    return p_gains[index];
  }

 /**
  * Returns the calculated offset (base) for the given 
  * identifier 
  *
  * @param id The identifying name of the file whose calculated 
  *           offset is to be returned here
  *  
  * @return double The offset for the identifier 
  *  
  * @throws Isis::iException::User - Identifying string must 
  *             exist in the idList
  * @throws Isis::iException::User - Least Squares equation must 
  *             be solved before returning the offset 
  */
  double IntersectionStatistics::GetOffset (const std::string id) const {
    if (!p_solved) {
      string msg = "The least squares equation has not been successfully ";
      msg += "solved yet.";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    }

    int index = -1;
    for (unsigned i=0; i<p_idList.size(); i++) {
      string curId = p_idList.at(i);
      if (curId.compare(id) == 0) {
        index = i;
      }
    }
    if (index == -1) {
      string msg = "The id [" + id + "] was not found in the idList provided.";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    }

    return p_offsets[index];
  }
}
