#ifndef QnetFileTool_h
#define QnetFileTool_h

/**
 * @file
 * $Revision: 1.7 $
 * $Date: 2008/12/19 22:08:34 $
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
#include <QComboBox>
#include <QDialog>
#include <QLabel>
#include <QListWidget>
#include <QStackedWidget>
#include <QStringList>
#include "ControlNet.h"
#include "FileTool.h"
#include "Projection.h"
#include "SerialNumberList.h"
#include "Tool.h"

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
 *  @history 2008-11-24 Jeannie Walldren - Replace references to
 *           PointEdit class with ControlPointEdit
 *  @history 2008-11-26 Jeannie Walldren - Added cNetName
 *            parameter to controlNetworkUpdated() so that 
 *            QnetTool can read the name of the control net file.
 * @history  2008-11-26 Tracie Sucharski - Remove all polygon/overlap 
 *                         polygon/overlap references, this functionality will
 *                         be in qmos.
 *  @history 2008-12-10 Jeannie Walldren - Reworded "What's
 *            this?" description for saveAs action. Changed
 *            "Save As" action text to match QnetTool's "Save
 *            As" action
 *
 */

  class QnetFileTool : public FileTool {
    Q_OBJECT

    signals:
      void serialNumberListUpdated();
      void controlNetworkUpdated(QString cNetName);

    public:
      QnetFileTool (QWidget *parent);
      QString cNetFilename;

    public slots:
      virtual void open();
      virtual void exit();
      virtual void saveAs();
      void loadPoint(Isis::ControlPoint *point);
      void loadImage(const QString &serialNumber);
      void setSaveNet();

    private:
      bool p_saveNet;
 };
};

#endif
