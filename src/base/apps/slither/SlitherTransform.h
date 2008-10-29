#ifndef SlitherTransform_h
#define SlitherTransform_h

#include <vector>
#include <fstream>
#include <iostream>
#include "Transform.h"
#include "PvlGroup.h"
#include "ControlNet.h"
#include "DataInterp.h"
#include "ControlByRow.h"

namespace Isis {

class SlitherTransform : public Transform {
    public:
      typedef DataInterp::InterpType InterpType;
      SlitherTransform(Cube &cube, ControlNet &cnet, 
                       InterpType lInterp = DataInterp::Cubic,
                       InterpType sInterp = DataInterp::Cubic);

      /**
       * @brief Destructor
       *
       * All cleanup is automagically handled!
       */
      virtual ~SlitherTransform() { }

      /**
       * @brief Number of points used in computation of transform
       * 
       * This method returns the number of row points used in the computation of
       * the spline interpolation fits.
       * 
       * @return int Number of row point data sets used in sample/line
       *             interopolations
       */
      inline int size() const { return (_rows.size()); }

      /**
       * @brief Sets forward transform direction
       *
       * This is the normal expected operation.  The forward transform direction
       * implies the transform is applied to the search image, not the pattern,
       * or reference, image used to create the control network.
       * 
       * The search image is typically the FROM parameter in co-registration
       * applications.
       */
      inline void setForward() { _iDir = 1.0; }

      /**
       * @brief Sets reverse transform direction
       *
       * The reverse transform direction implies the transform is applied to the
       * pattern, or reference, image, not the search image used to create the
       * control network.
       * 
       * The reference image is the MATCH parameter in co-registration
       * applications.
       */
      inline void setReverse() { _iDir = -1.0; }

      /**
       * @brief Total points in control net file
       * 
       * This method reports to the caller the total number of points in the
       * control net file.  This includes ignored points as well that are
       * automatically excluded.  Points that do not have exactly 2 measures are
       * excluded as well.
       * 
       * @return int Number of points used
       */
      inline int totalPoints() const { return(_pntsTotal); }

      /**
       * @brief Number of points used from the input control net file
       * 
       * This method reports to the caller the number of points used when read
       * from the control point file.  This count does not include ingored
       * points or bad points.
       * 
       * @return int Number of points used
       */
      inline int numberPointsUsed() const { return(_pntsUsed); }

      /**
       * @brief Number of points tossed on input to this class
       * 
       * This method reports to the caller the number of points excluded when
       * read from the control point file that exceeded a certain cnet line.
       * 
       * @return int Number of points excluded
       */
      inline int numberBadPoints() const { return(_pntsTossed); }

      /**
       * @brief Number of bad rows detected in control net
       * 
       * This method reports to the caller the number of bad rows detected after
       * merging chip column registrations for each row.  This will typically
       * indicate the goodness of fit excluded all the points in that row by
       * constraints imposed by the merging function.
       * 
       * @return int Number of bad rows detected
       * @see ControlByRow
       */
      inline int numberBadRows() const { return(_badRows.size()); }

      // Implementations for parent's pure virtual members
      bool Xform (double &inSample, double &inLine,
                  const double outSample, const double outLine);

      /**
       * @brief Determine the number of samples in the output image
       * 
       * This method returns the number of samples that will result in the image
       * created from this transform.  Default behavior is to use the same
       * number of samples as the input image.
       * 
       * @return int Number of samples in output image
       */
      int OutputSamples () const { return _outputSamples; };

      /**
       * @brief Determine the number of lines in the output image
       * 
       * This method returns the number of lines that will result in the image
       * created from this transform.  Default behavior is to use the same
       * number of lines as the input image.
       * 
       * @return int Number of lines in output image
       */
      int OutputLines () const { return _outputLines; };

      /**
       * @brief Add an additional offset to the line output translation
       * 
       * This method provides the users of this class to shift the image an
       * additional number of lines than are determined by the control net
       * registration information provides.
       * 
       * This is mostly useful when the registration had an initial starting
       * offset as opposed to the assumption of generally spatially registering
       * images.
       * 
       * Negative values shifts the image down in the output image.  Positive
       * values shift the image up.
       *
       * This method can be called at any time during the processing to change
       * the relative shift.  It does not affect the size of the output image,
       * only the placement of the samples.
       * 
       * @param lineOffset Additional offset to shift lines in output image
       */
      void addLineOffset(double lineOffset) {
        _lineOffset = lineOffset;
        return;
      }

      /**
       * @brief Add an additional offset to the sample output transform
       * 
       * This method provides the users of this class to shift the image an
       * additional number of samples than are determined by the control net
       * registration information provides.
       * 
       * This is mostly useful when the registration had an initial starting
       * offset as opposed to the assumption of generally spatially registering
       * images.
       * 
       * Negative values shifts the image right in the output image.  Positive
       * values shift the image left.
       * 
       * This method can be called at any time during the processing to change
       * the relative shift.  It does not affect the size of the output image,
       * only the placement of the samples.
       * 
       * @param sampOffset Additional offset to shift samples in output image
       */
      void addSampleOffset(double sampOffset) {
        _sampOffset = sampOffset;
        return;
      }

      Statistics LineStats() const;
      Statistics SampleStats() const;

      std::ostream &dumpState(std::ostream &out);
  
    private:
      typedef ControlByRow::RowPoint RowPoint;
      typedef std::vector<RowPoint> RowList;
      RowList     _rows;                  //!<  Collected row points
      RowList     _badRows;               //!<  Collects bad row points
      int         _pntsTotal;             //!<  Total number points in control
      int         _pntsUsed;              //!<  Total number points not ignored
      int         _pntsTossed;            //!<  Total number points tossed
      double      _iDir;                  //!<  Interpolation direction
                                          
      DataInterp  _lineSpline;            //!< Line spline interpolation
      DataInterp  _sampSpline;            //!< Sample spline interpolation

      int _outputLines;                   //!< Number output lines
      int _outputSamples;                 //!< Number output samples

      double _lineOffset;                 //!< Additional spatial line offset
      double _sampOffset;                 //!< Additional spatial sample offset

      /**
       * @brief Compute the relative shift of the given axis
       * 
       * This method computes the relative shift at the given location,
       * typically the line, additionally incorporating the user selected
       * direction, forward or reverse.
       * 
       * To acheive the actual location, the input element (sample, line)
       * location and the user specified offset must be additionally applied.
       * 
       * @param x  Coordinate of the value to get interpolation for
       * @param interp Interpolation computed for the axis
       * 
       * @return double Relative shift at the specified coordinate
       * @see getLineXform
       * @see getSampXform
       */
      inline double getOffset(const double &x, const DataInterp &interp) const {
         return (_iDir * interp.Evaluate(x));
      }

      /**
       * @brief Compute the line transform given output line
       * 
       * This method computes the absolute input line from the image given the
       * requested output line.  It uses interpolation functions precomputed
       * from the input control net file provided by a coregistration
       * application, typically.
       * 
       * @param line Output line coordinate to compute input line for
       * 
       * @return double Input line coordinate at the output line coordinate
       */
      inline double getLineXform(const double line) const {
        return (line - getOffset(line, _lineSpline) + _lineOffset);
      }

      /**
       * @brief Compute the sample transform given output line, sample
       * 
       * This method computes the absolute input line from the image given the
       * requested output line.  It uses interpolation functions precomputed
       * from the input control net file provided by a coregistration
       * application, typically.
       * 
       * @param line Output line coordinate to compute input sample for
       * @param samp Output sample coordinate to compute input sample for
       * 
       * @return double Input sample coordinate at the output line, sample
       *         coordinate
       */
      inline double getSampXform(const double line, const double samp) const {
        return (samp - getOffset(line, _sampSpline) + _sampOffset);
      }

  
  };

}
#endif


