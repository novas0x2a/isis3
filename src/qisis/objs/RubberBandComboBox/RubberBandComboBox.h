#ifndef RubberBandComboBox_h
#define RubberBandComboBox_h

#include <QComboBox>
#include "RubberBandTool.h"

namespace Qisis {
  /**
  * @brief Combo box for choosing a rubber band type
  *
  * @ingroup Visualization Tools
  *
  * @author 2007-09-11 Steven Lambright
  */

  class RubberBandComboBox : public QComboBox {
    Q_OBJECT

    public:
      RubberBandComboBox(unsigned int bandingOptions, unsigned int defaultOption, bool showIndicatorColors = false);

      /**
       * Enum to store rubber band shapes.
       */
      enum RubberBandOptions {
        Angle            = 1, //!< Angle
        Circle           = 2, //!< Circle
        Ellipse          = 4, //!< Ellipse
        Line             = 8, //!< Line
        Rectangle        = 16,//!< Rectangle
        RotatedRectangle = 32,//!< RotatedRectangle
        Polygon          = 64 //!< Polygon
      };

      //! Returns the icon directory.
      QString toolIconDir() const { return RubberBandTool::getInstance()->toolIconDir(); }
      void reset();

    protected slots:
      void selectionChanged(int index);

    private:
      //! This is used to figure out which option should be defaulted
      unsigned int getDefault(unsigned int defaultOption, unsigned int bandingOptions); 

      //! Enables the angle shape
      void showAngle()             { RubberBandTool::enable(RubberBandTool::Angle, p_showIndicatorColors);            };
      //! Enables the circle shape
      void showCircle()            { RubberBandTool::enable(RubberBandTool::Circle, p_showIndicatorColors);           };
      //! Enables the ellipse shape
      void showEllipse()           { RubberBandTool::enable(RubberBandTool::Ellipse, p_showIndicatorColors);          };
      //! Enables the line
      void showLine()              { RubberBandTool::enable(RubberBandTool::Line, p_showIndicatorColors);             };
      //! Enables the rectangle shape
      void showRectangle()         { RubberBandTool::enable(RubberBandTool::Rectangle, p_showIndicatorColors);        };
      //! Enables the rotated rectangle shape
      void showRotatedRectangle()  { RubberBandTool::enable(RubberBandTool::RotatedRectangle, p_showIndicatorColors); };
      //! Enables the polygon shape
      void showPolygon()           { RubberBandTool::enable(RubberBandTool::Polygon, p_showIndicatorColors);          };

      QStringList p_bandingOptionStrings; //!< List of rubberband options
      bool p_showIndicatorColors; //!< Show colors?
  };
};

#endif
