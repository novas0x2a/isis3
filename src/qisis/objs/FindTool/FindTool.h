#ifndef FindTool_h
#define FindTool_h
/**
 * @file
 * $Revision: 1.6 $
 * $Date: 2008/09/03 16:27:29 $
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
#include <QLineEdit>
#include <QCheckBox>
#include <QDialog>
#include "Tool.h"

namespace Qisis {
  /**
   * @brief Tool to locate a point on a cube that is projected and/or has a camera 
   *        model
   *
   * This tool is part of the Qisis namespace and allows the user to locate a 
   * point on a cube that has been projected and/or has a camera model. It also 
   * allows the user to link viewports and sync scales. 
   * 
   * @ingroup Visualization Tools
   * 
   * @author  ?? Unknown
   * 
   * @internal

   *  @history 2008-06-25 Noah Hilt - Switched positions of the sample/line line
   *           edits and labels.
   *  
   *  @history 2008-08 Stacy Alley - Added red dot which is draw
   *           at the specified lat/lon or line/sample.  Also, the
   *           red dot is draw in the corresponding spot of a
   *           linked image, if there is overlap in the two
   *           images.
   */
  class GroundTab : public QWidget {
      Q_OBJECT

      public:
        GroundTab(QWidget *parent = 0);
      
        QLineEdit *p_lonLineEdit; //!< Input for longitude
        QLineEdit *p_latLineEdit; //!< Input for latitude
  };

  class ImageTab : public QWidget {
      Q_OBJECT

      public:
        ImageTab(QWidget *parent = 0);
      
        QLineEdit *p_sampLineEdit; //!< Input for sample
        QLineEdit *p_lineLineEdit; //!< Input for line
  };

  class FindTool : public Qisis::Tool {
    Q_OBJECT

    public:
      FindTool (QWidget *parent);
      ~FindTool();
      void addTo(QMenu *menu);
      void paintViewport(CubeViewport *vp,QPainter *painter);

      

    protected:
      QAction *toolPadAction(ToolPad *toolpad);
      /**
       * This method returns the menu name associated with this tool.
       * 
       * 
       * @return QString 
       */
      QString menuName() const { return "&Options"; };
      QWidget *createToolBarWidget(QStackedWidget *parent);
      void updateTool();
      void createDialog(QWidget *parent);

      protected slots:
        void mouseButtonRelease(QPoint p, Qt::MouseButton s);
        void mouseButtonPress(QPoint p, Qt::MouseButton s);
        void mouseMove(QPoint p);

    private slots:
      void okAction();
      void getUserImagePoint();
      void getUserGroundPoint();
      void findPoint (QPoint p);
      void linkValid();
      void clearPoint();

    private:
      QDialog *p_dialog;
      QAction *p_findPoint;
      QToolButton *p_findPtButton;
      QToolButton *p_linkButton;
      QCheckBox *p_syncScale;
      QLineEdit *p_status;
      QTabWidget *p_tabWidget;
      GroundTab *p_groundTab;
      ImageTab *p_imageTab;
      bool p_released;
      bool p_pressed;
      QPoint p_point;
      QPen p_pen;

  };

  

};

#endif
