#include "GroupedStatistics.h"
#include "Statistics.h"

#include <QMap>
#include <QString>

namespace Isis
{

  //! construct a GroupedStatistics object
  GroupedStatistics::GroupedStatistics()
  {
    groupedStats = NULL;
    groupedStats = new QMap< QString, Statistics >();
  }
  
  
  /**
   *  copy construct a GroupedStatistics object
   *
   *  @param other The GroupedStatistics to copy
   */
  GroupedStatistics::GroupedStatistics(const GroupedStatistics & other)
  {
    groupedStats = NULL;
    groupedStats = new QMap< QString, Statistics >(*other.groupedStats);
  }
  
  
  //! destroy a GroupedStatistics object
  GroupedStatistics::~GroupedStatistics()
  {
    if (groupedStats)
    {
      delete groupedStats;
      groupedStats = NULL;
    }
  }
  
 
  /**
   *  Add a new data entry for a given type of data
   *
   *  @param statType Type of data (GoodnessOfFit for example)
   *
   *  @param newStat New statistical data to be added
   *
   */
  void GroupedStatistics::AddStatistic(const QString & statType, const
      double & newStat)
  {
    (*groupedStats)[statType].AddData(newStat);
  }
  
  
  /**
   *  Get statistics for a given type of data
   *
   *  @param statType Type of data
   *
   *  @returns Statisticts for the given type of data
   *
   *  @throws iException When the given type of data does not exist
   */
  const Statistics & GroupedStatistics::GetStatistics(const QString & statType)
      const
  {
    QMap< QString, Statistics >::const_iterator i;
    i = groupedStats->constFind(statType);
    
    if (i == groupedStats->constEnd())
    {
      std::string msg = statType.toStdString();
      msg += " passed to GetStats but does not exist within the map";
      throw Isis::iException::Message(Isis::iException::Programmer, msg,
                                      _FILEINFO_);
    }
    
    return i.value();
  }
  
  
  /**
   *  Assign a GroupedStatistics with another GroupedStatistics using =
   *
   *  @param other The GroupedStatistics to copy
   */
  GroupedStatistics & GroupedStatistics::operator=(const GroupedStatistics
      & other)
  {
    delete groupedStats;
    groupedStats = NULL;

    groupedStats = new QMap< QString, Statistics >(*other.groupedStats);
    
    return *this;
  }  
}
