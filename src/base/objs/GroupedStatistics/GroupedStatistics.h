#ifndef GroupedStatistics_h
#define GroupedStatistics_h

/**
 * @file
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

template< class A, class B > class QMap;
class QString;

namespace Isis
{

  // Isis forward declarations
  class Statistics;
  
  /**
   * @brief Control Measure statistics
   *
   * This class is used to store statistics on a group of control measures
   *
   * @ingroup ControlNetwork
   *
   * @author
   *
   * @see ControlGraph ControlMeasure
   *
   * @internal
   *                              
   */
  class GroupedStatistics 
  {
    public:
      GroupedStatistics();
      GroupedStatistics(const GroupedStatistics & other);
      ~GroupedStatistics();
      
      void AddStatistic(const QString & statType, const double & newStat);
      const Statistics & GetStatistics(const QString & statType) const;

      GroupedStatistics & operator=(const GroupedStatistics & other);

    private:
      //! Map from statistic type to Statistics object
      QMap< QString, Statistics > * groupedStats;
  };
};

#endif

