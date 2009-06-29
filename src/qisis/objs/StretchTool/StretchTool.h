#ifndef StretchTool_h
#define StretchTool_h

#include <QPushButton>
#include <QLineEdit>
#include <QSlider>
#include <QDoubleValidator>
#include <QComboBox>
#include <QDialog>
#include <QTableWidget>
#include <QRadioButton>
#include <QGroupBox>
#include <QStackedWidget>

#include "Tool.h"
#include "RubberBandTool.h"
#include "HistogramWidget.h"
#include "BinaryStretch.h"
#include "LinearStretch.h"
#include "ManualStretch.h"
#include "SawtoothStretch.h"

/**
 * @file
 * $Revision: 1.7 $
 * $Date: 2009/05/14 18:45:23 $
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
   *
   */
  class StretchTool : public Qisis::Tool {
    Q_OBJECT

    public:
      StretchTool (QWidget *parent);
      void addTo(QMenu *menu);
      void addStretch(QStretch *stretch);

      /**
       * Enum to store the bands.
       */
      enum StretchBand {
        Red, //!< Red Band
        Green, //!< Green Band
        Blue, //!< Blue Band
        //All //!< All Bands
      };

      /**
       * Struct to store information about each band.
       */
      struct CubeInfo {
        double min; //!< The minimum of the band
        double max; //!< The maximum of the band
        Isis::Histogram *hist; //!< The current histogram of the band
        QStretch *stretch; //!< The advanced stretch of the band
        Isis::Stretch currentStretch; //!< The current stretch of the band
        Isis::Stretch globalStretch; //!< The global stretch of the band
      };

      double getHistogramDN(double percent) const;
      double getHistogramPercent(double dn) const;

      /**
       * This method returns a pointer to the advanced dialog.
       * 
       * 
       * @return QDialog* 
       */
      QDialog *getDialog() const {return p_dialog;};

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
      void stretchIndexChanged(int index);
      void setStretch(QStretch *stretch);
      void updateStretch();
      void removeViewport(QObject *vp);

      void setRedStretch(bool toggled);
      void setGreenStretch(bool toggled);
      void setBlueStretch(bool toggled);

    private slots:
      void showAdvancedDialog();
      void flash();
      void changeStretch();
      void selectBandMinMax(int index);
      void setStretchBands();
      void setStretchAllViewports();

      void grayRangeChanged(const double min, const double max);
      void redRangeChanged(const double min, const double max);
      void greenRangeChanged(const double min, const double max);
      void blueRangeChanged(const double min, const double max);

      void savePairsAs();
      void savePairs();

    private:
      void stretchChanged();
      void stretchBuffer(QRect rect);
      void stretchEntireCube(CubeViewport *cvp);

      Isis::Statistics calculateStatisticsFromCube(Isis::Cube *cube, int band);
      Isis::Statistics calculateStatisticsFromBuffer(ViewportBuffer *buffer, QRect rect);
      Isis::Histogram calculateHistogramFromCube(Isis::Cube *cube, int band, double min, double max);
      Isis::Histogram calculateHistogramFromBuffer(ViewportBuffer *buffer, QRect rect, double min, double max);

      QDialog *p_dialog; //!< The advanced dialog
      QStretch *p_currentStretch; //!< The current advanced stretch

      QStackedWidget *p_histogramWidget; //!< A stacked widget to hold gray and RGB histogram widgets
      HistogramWidget *p_grayHistogram; //!< The gray histogram
      HistogramWidget *p_redHistogram; //!< The red histogram
      HistogramWidget *p_greenHistogram; //!< The green histogram
      HistogramWidget *p_blueHistogram; //!< The blue histogram

      QComboBox *p_stretchBox; //!< The combo box to select advanced stretches
      QStackedWidget *p_stackedWidget; //!< A stacked widget to show advanced stretch parameters

      QTableWidget *p_pairsTable; //!< The table that displays the current stretch's pairs

      QGroupBox *p_rgbBox; //!< A group box to hold RGB radio buttons
      QRadioButton *p_redButton; //!< Radio button to select red
      QRadioButton *p_greenButton; //!< Radio button to select green
      QRadioButton *p_blueButton; //!< Radio button to select blue

      QPushButton *p_saveButton; //!< Button to save pairs
      QPushButton *p_flashButton; //!< Button to flash between current and global stretch

      QList<QStretch *> p_stretchList; //!< The list of available advanced stretches

      QMap<CubeViewport *, QList <StretchTool::CubeInfo *> > p_viewportMap; //!< The viewport map to hold info for each viewport

      QFile p_currentFile; //!< The current file to save pairs to

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
  };
};

#endif

