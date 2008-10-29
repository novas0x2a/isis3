#ifndef Workspace_h
#define Workspace_h
/**
 * @file
 * $Revision: 1.5 $
 * $Date: 2008/05/27 17:04:36 $
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


#include <vector>
#include <string>
#include <qworkspace.h>
#include "Cube.h"

namespace Qisis {
 /**
 * @brief 
 *
 * @ingroup Visualization Tools
 *
 * @author ??? Jeff Anderson
 *
 * @internal
 * @history  2007-03-21  Tracie Sucharski - Changed call from fitScale to
 *                           fitScaleMinDimension so that the minimum cube
 *                           dimension is displayed in its entirety.
 * @history  2008-05-27  Noah Hilt - Now allows cubes to be 
 *           opened with additional arguments read by the
 *           CubeAttributeInput class, specifically to open
 *           certain bands as well as open 3 bands in RGB mode.
 */

  class CubeViewport;
  class Workspace : public QWorkspace {
    Q_OBJECT

    public:
      Workspace (QWidget *parent=0);
      std::vector<CubeViewport *> *cubeViewportList() { return &p_cubeViewportList; };

    signals:
      void cubeViewportAdded (Qisis::CubeViewport *);
      void cubeViewportActivated (Qisis::CubeViewport *);

    public slots:
      void addCubeViewport(QString cube);
      CubeViewport* addCubeViewport(Isis::Cube *cube);

      void addBrowseView(QString cube);

    protected slots:
      void activateViewport(QWidget *w);

    private:
      std::vector<Qisis::CubeViewport *> p_cubeViewportList;
      void populateList();
      QWidget *p_lastWindow;

  };
};

#endif
