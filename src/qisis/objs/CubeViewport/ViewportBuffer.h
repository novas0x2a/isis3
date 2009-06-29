#ifndef ViewportBuffer_h
#define ViewportBuffer_h

/**
 * @file
 * $Date: 2009/05/13 19:26:08 $
 * $Revision: 1.7 $
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

#include <QRect>

#include "Cube.h"
#include "Brick.h"

namespace Qisis {
/**
 * @brief Reads and stores visible DN values
 *  
 * This class manages visible pixels in a CubeViewport. This class is responsible 
 * for reading from the Cube only what is necessary and gives fast access to 
 * visible DNs.  
 *
 * @ingroup Visualization Tools 
 *  
 * @author 2009-03-26 Steven Lambright and Noah Hilt 
 *  
 * @internal 
 *   @history 2009-04-21 Steven Lambright - Fixed problem with only half of the
 *            side pixels being loaded in
 *
 */

  class CubeViewport;

  class ViewportBuffer {
    public:
      ViewportBuffer (CubeViewport *viewport, Isis::Cube *cube);
      virtual ~ViewportBuffer ();

      const std::vector<double> &getLine(int line);

      void resizedViewport();
      //void changeScale(double newScale);
      void pan(int deltaX, int deltaY);

      void scaleChanged();

      void fillBuffer(QList<QRect> rects);
      void fillBuffer(QRect rect);

      void emptyBuffer(bool force = false);

      bool hasEntireCube();
      /**
       * Returns the bounding rectangle for the visible cube area in viewport pixels.
       * 
       * 
       * @return QRect 
       */
      QRect bufferXYRect() { return p_XYBoundingRect; }

      void setBand(int band);
      int getBand() { return p_band; }

      void enable(bool enabled);

      /**
       * Returns whether the buffer is enabled (reading data) or not.
       * 
       * 
       * @return bool 
       */
      bool enabled() { return p_enabled; }

    private:
      QRect getXYBoundingRect();
      QList<double> getSampLineBoundingRect();
      void updateBoundingRects();

      void resizeBuffer(unsigned int width, unsigned int height);
      void shiftBuffer(int deltaX, int deltaY);
      void reinitialize();

      QList<QRect> removeOverlaps(QList<QRect> rects);

      CubeViewport *p_viewport; //!< The CubeViewport which created this buffer
      Isis::Cube *p_cube; //!< The cube associated with the cube viewport

      int p_band; //!< The band to read from

      bool p_enabled; //!< True if reading from cube (active)
      std::vector< std::vector<double> > p_buffer; //!< The buffer to hold cube dn values
      bool p_bufferInitialized; //!< True if the buffer has been initialized

      QRect p_XYBoundingRect, p_oldXYBoundingRect; //!< Bounding rectangles in viewport pixels
      QList< double > p_sampLineBoundingRect, p_oldSampLineBoundingRect; //!< Bounding rectangles in sample/line coordinates (double)

      /**
       * Enumeration for accessing sample/line bounding rectangles
       */
      enum sampLineRectPosition {
        rectLeft = 0, //!< QRect.left()
        rectTop,  //!< QRect.top()
        rectRight, //!< QRect.right()
        rectBottom //!< QRect.bottom()
      };

      static unsigned long p_totalBufferSize; //!< The current total buffer size of all active buffers
      //! ~100MB max total buffer size (only applies to multiple viewports open).
      const static unsigned long p_maxBufferSize = 104857600;
  };
}

#endif
