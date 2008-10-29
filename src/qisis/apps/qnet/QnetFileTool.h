#ifndef QnetFileTool_h
#define QnetFileTool_h

/**
 * @file
 * $Revision: 1.2 $
 * $Date: 2007/05/15 22:44:12 $
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

#include <QAction>
#include <QListWidget>
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
 * @brief Qnet File operations
 *
 * @ingroup Visualization Tools
 *
 * @author 2006-02-01 Jeff Anderson
 *
 * @internal
 *  @history 2006-08-02 Tracie Sucharski - Initialize cameras for every image
 *                        in cube list.
 *
 */

  class QnetFileTool : public FileTool {
    Q_OBJECT

    signals:
      void serialNumberListUpdated();
      void controlNetworkUpdated();

    public:
      QnetFileTool (QWidget *parent);

    public slots:
      virtual void open();
      virtual void exit();
      virtual void saveAs();
      void loadOverlap(Isis::ImageOverlap *overlap);
      void loadPoint(Isis::ControlPoint *point);
      void loadImage(const QString &serialNumber);
      void setSaveNet();

    private:
      bool p_saveNet;
 };
};

#endif
