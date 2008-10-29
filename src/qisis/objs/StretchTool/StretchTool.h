#ifndef StretchTool_h
#define StretchTool_h

#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>

#include "Tool.h"
#include "RubberBandTool.h"

/**
 * @file
 * $Revision: 1.5 $
 * $Date: 2008/06/25 23:49:46 $
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

namespace Qisis {
  /**
   * @brief Stretch image edit tool
   *
   * This tool is part of the Qisis namespace and allows interactive editing
   * of displayed images.
   * 
   * @ingroup Visualization Tools
   * 
   * @author 
   * 
   * @internal
   *  @history 2008-05-23 Noah Hilt - Added RubberBandTool
   *           functionality.
   *  @history 2008-05-25 Noah Hilt - Now checks if the min/max fields are empty
   *           before setting the stretch and acts accordingly.
   * 
   *
   */
  class StretchTool : public Qisis::Tool {
    Q_OBJECT

    public:
      StretchTool (QWidget *parent);
      void addTo(QMenu *menu);

      /**
       * Enum to store the bands.
       */
      enum StretchBand {
        Red, //!< Red Band
        Green, //!< Green Band
        Blue, //!< Blue Band
        All //!< All Bands
      };

    public slots:
      void stretchGlobal();
      void stretchGlobalAllBands();
      void stretchGlobalAllViewports();
      void stretchRegional();

    protected:
      //! Returns the menu name.
      QString menuName() const { return "&View"; };
      QAction *toolPadAction(ToolPad *pad);
      QWidget *createToolBarWidget (QStackedWidget *parent);
      void updateTool();

    protected slots:
      void mouseButtonRelease(QPoint p, Qt::MouseButton s);
      virtual void enableRubberBandTool();
      void rubberBandComplete();

    private:
      QToolButton *p_copyButton; //!< Copy Button
      QToolButton *p_globalButton; //!< Global Button
      QToolButton *p_stretchRegionalButton; //!< Regional Stretch Button

      QAction *p_stretchGlobal; //!< Global stretch action
      QAction *p_stretchRegional; //!< Regional stretch action
      QAction *p_stretchManual; //!< Manual stretch action
      QAction *p_copyBands; //!< Copy band stretch action

      QComboBox *p_stretchBandComboBox; //!< Stretch combo box

      QLineEdit *p_stretchMinEdit; //!< Min. line edit
      QLineEdit *p_stretchMaxEdit;//!< Max. line edit 

      StretchBand p_stretchBand; //!< Current stretch band

    private slots:
      void changeStretch();
      void selectBandMinMax(int index);
      void setStretchBands();
      void setStretchAllViewports();

  };
};

#endif

