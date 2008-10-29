#ifndef QnetNavTool_h
#define QnetNavTool_h

/**
 * @file
 * $Date: 2008/09/03 19:28:59 $ $Revision: 1.3 $
 *
 *  Unless noted otherwise, the portions of Isis written by the USGS are public domain. See
 *  individual third-party library and package descriptions for intellectual property information,
 *  user agreements, and related information.
 *
 *  Although Isis has been used by the USGS, no warranty, expressed or implied, is made by the
 *  USGS as to the accuracy and functioning of such software and related material nor shall the
 *  fact of distribution constitute any such warranty, and no responsibility is assumed by the
 *  USGS in connection therewith.
 *
 *  For additional information, launch $ISISROOT/doc//documents/Disclaimers/Disclaimers.html
 *  in a browser or see the Privacy &amp; Disclaimers page on the Isis website,
 *  http://isis.astrogeology.usgs.gov, and the USGS privacy and disclaimers on
 *  http://www.usgs.gov/privacy.html.
 */


#include <QAction>
#include <QListWidget>
#include <QPushButton>
#include <QComboBox>
#include <QDialog>
#include <QStackedWidget>
#include <QLabel>
#include <QStringList>
#include "Tool.h"
#include "FileTool.h"
#include "SerialNumberList.h"
#include "ImageOverlap.h"
#include "ControlNet.h"
#include "Projection.h"

namespace Qisis {
  /**
   * @brief Qnet Navigation Tool
   *
   * @ingroup Visualization Tools
   *
   * @author Elizabeth Ribelin - 2006-11-07
   *
   * @internal
   * @history 2007-06-05 Tracie Sucharski - Added enumerators for filter indices
   */
  class QnetNavTool : public Tool {
    Q_OBJECT

    public:
      QnetNavTool (QWidget *parent);

      enum FilterIndex {
        Points,
        Cubes
      };
      enum PointFilterIndex {
        Errors,
        Id,
        NumberImages,
        Type,
        LatLonRange,
        Distance,
        MeasureType
      };
      enum CubeFilterIndex {
        Name,
        NumberPoints,
        PointDistance
      };


    public slots:
      void updateList();

    private slots:
      void load();
      void tie();
      void filter();
      void selectionChanged();
      void updateSelected();
      void filterList();
      void resetFilter();
      void enableTie();

    signals:
      void loadPoint (Isis::ControlPoint *);
      void loadOverlap (Isis::ImageOverlap *);
      void loadImage (const QString &);
      void selectionUpdate();
      void modifyPoint(Isis::ControlPoint *);

    private:
      void createNavigationDialog(QWidget *parent);
      void createFilters();


      QDialog *p_navDialog;
      QPushButton *p_filter;
      QPushButton *p_tie;
      QStackedWidget *p_filterStack;
      QStringList p_list;
      QComboBox *p_listCombo;
      QListWidget *p_listBox;
  };
};

#endif
