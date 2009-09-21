#include "Chip.h"
#include "Pvl.h"
#include "InterestOperator.h"
#include "Plugin.h"
#include "iException.h"
#include "Filename.h"
#include "Statistics.h"
#include "PolygonTools.h"

namespace Isis {

  /**
   * Create InterestOperator object.  Because this is a pure virtual class you can
   * not create an InterestOperator class directly.  Instead, see the InterestOperatorFactory 
   * class.
   * 
   * @param pvl  A pvl object containing a valid InterestOperator specification
   * 
   * @see automaticRegistration.doc
   */
  InterestOperator::InterestOperator (Pvl &pvl) {
    p_operator = pvl.FindGroup("Operator", Pvl::Traverse);

    p_operatorName = "";
    p_interestAmount = 0.0;
    p_worstInterest = 0.0;
    p_lines = 1;
    p_samples = 1;
    p_deltaSamp = 0;
    p_deltaLine = 0;
    p_clipPolygon = NULL;

    Parse(pvl);
  }


  //! Destroy InterestOperator object
  InterestOperator::~InterestOperator() {
    if (p_clipPolygon != NULL) delete p_clipPolygon;
  }


  /** 
 * Create an InterestOperator object using a PVL specification.
 * An example of the PVL required for this is:
 * 
 * @code
 * Object = InterestOperator
 *   Group = Operator
 *     Name      = StandardDeviation
 *     Samples   = 21
 *     Lines     = 21
 *     DeltaLine = 50
 *     DeltaSamp = 25
 *   EndGroup
 * EndObject
 * @endcode
 * 
 * There are many other options that can be set via the pvl and are
 * described in other documentation (see below).
 * 
 * @param pvl The pvl object containing the specification
 **/
  void InterestOperator::Parse(Pvl &pvl) {
    try {
      // Get info from the operator group
      PvlGroup &op = pvl.FindGroup("Operator",Pvl::Traverse);
      p_operatorName =  (std::string) op["Name"];

      p_samples = op["Samples"];
      p_lines = op["Lines"];
      p_deltaLine = op["DeltaLine"];
      p_deltaSamp = op["DeltaSamp"];

      if (op.HasKeyword("MinimumDN")) p_minimumDN = op["MinimumDN"];
      else p_minimumDN = Isis::ValidMinimum;
      if (op.HasKeyword("MaximumDN")) p_maximumDN = op["MaximumDN"];
      else p_maximumDN = Isis::ValidMaximum;

      if (p_maximumDN < p_minimumDN) {
        std::string msg = "MinimumDN must be less than MaximumDN";
        throw iException::Message(iException::User,msg,_FILEINFO_);
      }

      p_minimumInterest = op["MinimumInterest"];

    } catch (iException &e) {
      std::string msg = "Improper format for InterestOperator PVL ["+pvl.Filename()+"]";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    }

    return;
  }


  /**
   * Walk the pattern chip through the search chip to find the best interest
   *  
   * @param cube [in] The Isis::Cube to look for an interesting area in 
   * @param sample [in] The sample postion in the cube where the chip is located
   * @param line [in] The line postion in the cube where the chip is located 
   *  
   * @return  Returns the status of the operation.  The following conditions can 
   *          occur true=Success, false=Failed
   */
  bool InterestOperator::Operate(Cube &cube, int sample, int line) {
    int pad = Padding();
    Chip chip(2*p_deltaSamp + p_samples + pad, 2*p_deltaLine + p_lines + pad);
    chip.TackCube(sample, line);
    if (p_clipPolygon != NULL) chip.SetClipPolygon(*p_clipPolygon);
    chip.Load(cube);
    // Walk the search chip and find the best interest
    int bestSamp = 0;
    int bestLine = 0;
    double smallestDist = DBL_MAX;
    double bestInterest = Isis::Null;
    for (int lin=p_lines/2 + 1; lin<=2*p_deltaLine+p_lines/2 + 1; lin++) {
      for (int samp=p_samples/2 + 1; samp<=2*p_deltaSamp+p_samples/2 + 1; samp++) {
        Chip subChip = chip.Extract(p_samples+pad, p_lines+pad, samp, lin);
        double interest = Interest(subChip);
        if (interest != Isis::Null) {
          if ((bestInterest == Isis::Null) || CompareInterests(interest,bestInterest)) {
            double dist = std::sqrt(std::pow(sample-samp,2.0)+std::pow(line-lin,2.0));
            if (interest == bestInterest && dist>smallestDist) {
              continue;
            } else {
              bestInterest = interest;
              bestSamp = samp;
              bestLine = lin;
              smallestDist = dist;
            }
          }
        }
      }
    }

    // Check to see if we went through the interest chip and never got a interest at
    // any location.
    if (bestInterest == Isis::Null || bestInterest<p_minimumInterest) return false;

    p_interestAmount = bestInterest;
    chip.SetChipPosition(bestSamp, bestLine);
    p_cubeSample = chip.CubeSample();
    p_cubeLine   = chip.CubeLine();
    return true;
  }


  /**
   * This virtual method must return if the 1st fit is equal to or better 
   * than the second fit.
   * 
   * @param int1  1st interestAmount 
   * @param int2  2nd interestAmount
   */
  bool InterestOperator::CompareInterests(double int1, double int2) {
    return(int1 >= int2);
  }


  // add this object's group to the pvl
  void InterestOperator::AddGroup(Isis::PvlObject &obj) {
    Isis::PvlGroup group;
    obj.AddGroup(group);
  }


  /**
   * Sets the clipping polygon for the chip. The coordinates must be in
   * (sample,line) order.
   *  
   * @param clipPolygon  The polygons used to clip the chip
   */
  void InterestOperator::SetClipPolygon (const geos::geom::MultiPolygon &clipPolygon) {
    if (p_clipPolygon != NULL) delete p_clipPolygon;
    p_clipPolygon = PolygonTools::CopyMultiPolygon(clipPolygon);
  }

  /**
   * Sets an offset to pass in larger chips if operator requires it
   * This is used to offset the subchip size passed into Interest
   * 
   * @return int   Amount to add to both x & y total sizes
   */
  int InterestOperator::Padding (){
    return 0;
  }

  /**
   * This function returns the keywords that this object was 
   * created from. 
   * 
   * @return PvlGroup The keywords this object used in 
   *         initialization
   */ 
  PvlGroup InterestOperator::Operator() {
    return p_operator;
  }
}
