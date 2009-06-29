#include "ViewportBuffer.h"

#include "CubeViewport.h"
#include "SpecialPixel.h"

#include <QApplication>

#define round(x) ((x) > 0.0 ? (x) + 0.5 : (x) - 0.5)

namespace Qisis {
  unsigned long ViewportBuffer::p_totalBufferSize = 0;

  /**
   * ViewportBuffer constructor. Viewport and Cube can not be null. 
   * Band is not initialized 
   *  
   * @param viewport viewport that will use the buffer
   * @param cube cube to read from
   */
  ViewportBuffer::ViewportBuffer(CubeViewport *viewport, Isis::Cube *cube) {
    p_viewport = viewport;
    p_cube = cube;
    p_bufferInitialized = false;
    p_band = -1;
    p_enabled = true;
  }

  /**
   * Updates total buffer size on destruction.
   * 
   */
  ViewportBuffer::~ViewportBuffer() {
    emptyBuffer(true);
  }

  /**
   * Removes overlaps in the list of rects and calls fillBuffer on each rectangle.
   * 
   * @param rects 
   */
  void ViewportBuffer::fillBuffer(QList<QRect> rects) {
    rects = removeOverlaps(rects);
    for(int i = 0; i < rects.size(); i++) {
      fillBuffer(rects[i]);
    }
  }

  /**
   * This method will convert the rect to sample/line positions and read from the 
   * cube into the buffer. The rect is in viewport pixels. 
   * 
   * @param rect 
   */
  void ViewportBuffer::fillBuffer(QRect rect) {
    rect = rect.intersected(p_XYBoundingRect);

    if(p_band == -1) {
      throw std::exception();
    }

    // Prep to create minimal buffer(s) to read the cube
    double ssamp, esamp, sline, eline;
    p_viewport->viewportToCube(rect.left(), rect.top(), ssamp, sline);
    p_viewport->viewportToCube(rect.right(), rect.bottom(), esamp, eline);

    int brickWidth = (int) (ceil(esamp) - floor(ssamp)) + 1;

    if(brickWidth <= 0) return;

    Isis::Brick *brick = new Isis::Brick(brickWidth, 1, 1, p_cube->PixelType());

    double samp,line;
    for (int y = rect.top(); y <= rect.bottom(); y++) {
      p_viewport->viewportToCube(rect.left(), y, samp, line);
      brick->SetBasePosition((int)(samp+0.5), (int)(line+0.5), p_band);
      p_cube->Read(*brick);

      for (int x = rect.left(); x <= rect.right(); x++) {
        p_viewport->viewportToCube(x, y, samp, line);

        int index = (int)(samp + 0.5) - brick->Sample();

        if ((index < 0) || (index >= brick->size())) {
          p_buffer.at(y - p_XYBoundingRect.top()).at(x - p_XYBoundingRect.left()) = Isis::Null;
        }
        else {
          p_buffer.at(y - p_XYBoundingRect.top()).at(x - p_XYBoundingRect.left()) = (*brick)[index];
        }
      }
    }
  }

  /**
   * Retrieves a line from the buffer. Line is relative to the top of the visible 
   * area of the cube in the viewport. 
   * 
   * @param line 
   * 
   * @return const std::vector<double>& 
   */
  const std::vector<double> &ViewportBuffer::getLine(int line) {
    if(!p_bufferInitialized || !p_enabled) {
      throw new std::exception();
    }

    return p_buffer.at(line);
  }

  /**
   * Retrieves the current bounding rect in viewport pixels of the visible cube 
   * area. 
   * 
   * 
   * @return QRect 
   */
  QRect ViewportBuffer::getXYBoundingRect() {
    int startx, starty, endx, endy;
    p_viewport->cubeToViewport(0.5, 0.5, startx, starty);
    p_viewport->cubeToViewport(p_viewport->cubeSamples() + 0.5, p_viewport->cubeLines() + 0.5, endx, endy);

    if(startx < 0) {
      startx = 0;
    }

    if(starty < 0) {
      starty = 0;
    }

    if(endx > p_viewport->viewport()->width()) {
      endx = p_viewport->viewport()->width();
    }

    if(endy > p_viewport->viewport()->height()) {
      endy = p_viewport->viewport()->height();
    }

    return QRect(startx, starty, endx-startx, endy-starty);
  }

  /**
   * Retrieves the current bounding rect in sample/line coordinates of the visible 
   * cube area. 
   * 
   * 
   * @return QList<double> 
   */
  QList<double> ViewportBuffer::getSampLineBoundingRect() {
    QRect xyRect = getXYBoundingRect();
    double ssamp, esamp, sline, eline;
    p_viewport->viewportToCube(xyRect.left(), xyRect.top(), ssamp, sline);
    p_viewport->viewportToCube(xyRect.right(), xyRect.bottom(), esamp, eline);

    QList<double> boundingRect;

    boundingRect.insert(rectLeft,ssamp);
    boundingRect.insert(rectTop, sline);
    boundingRect.insert(rectRight, esamp);
    boundingRect.insert(rectBottom, eline);

    return boundingRect;
  }

  /**
   * Sets the old and new bounding rects.
   * 
   */
  void ViewportBuffer::updateBoundingRects() {
    p_oldXYBoundingRect = p_XYBoundingRect;
    p_XYBoundingRect = getXYBoundingRect();

    p_oldSampLineBoundingRect = p_sampLineBoundingRect;
    p_sampLineBoundingRect = getSampLineBoundingRect();
  }

  /**
   * Enlarges or shrinks the buffer and fills with nulls if necessary.
   * 
   * @param width 
   * @param height 
   */
  void ViewportBuffer::resizeBuffer(unsigned int width, unsigned int height) {
    p_buffer.resize(height);

    for(unsigned int i = 0; i < p_buffer.size(); i++) {
      p_buffer[i].resize(width, Isis::Null);
    }
  }

  /**
   * Shifts the DN values in the buffer by deltaX and deltaY. Does not fill from 
   * outside the buffer. 
   * 
   * @param deltaX 
   * @param deltaY 
   */
  void ViewportBuffer::shiftBuffer(int deltaX, int deltaY) {
    if(deltaY >= 0) {
      for(int i = p_buffer.size()-1; i >= deltaY; i--) {
        p_buffer[i] = p_buffer[i - deltaY];

        if(deltaX > 0) {
          for(int j = p_buffer[i].size()-1; j >= deltaX; j--) {
            p_buffer[i][j] = p_buffer[i][j - deltaX];
          }
        }
        else if(deltaX < 0) {
          for(int j = 0; j < (int)p_buffer[i].size() + deltaX; j++) {
            p_buffer[i][j] = p_buffer[i][j - deltaX];
          }
        }
      }
    }
    else if(deltaY < 0) {
      for(int i = 0; i < (int)p_buffer.size() + deltaY; i++) {
        p_buffer[i] = p_buffer[i - deltaY];

        if(deltaX > 0) {
          for(int j = p_buffer[i].size()-1; j >= deltaX; j--) {
            p_buffer[i][j] = p_buffer[i][j - deltaX];
          }
        }
        else if(deltaX < 0) {
          for(int j = 0; j < (int)p_buffer[i].size() + deltaX; j++) {
            p_buffer[i][j] = p_buffer[i][j - deltaX];
          }
        }
      }
    }
  }

  /**
   * Call this when the viewport is resized (not zoomed). 
   * 
   */
  void ViewportBuffer::resizedViewport() {
    updateBoundingRects();

    if(!p_bufferInitialized || !p_enabled) { 
      return;
    }

    QList<QRect> fillAreas;

    //We need to know how much data was gained/lost on each side of the cube
    double deltaLeftSamples =  p_sampLineBoundingRect[rectLeft] - p_oldSampLineBoundingRect[rectLeft];
    //The input to round should be close to an integer
    int deltaLeftPixels = (int)round(deltaLeftSamples * p_viewport->scale());

    double deltaRightSamples = p_sampLineBoundingRect[rectRight] - p_oldSampLineBoundingRect[rectRight];
    int deltaRightPixels = (int)round(deltaRightSamples * p_viewport->scale());

    double deltaTopSamples = p_sampLineBoundingRect[rectTop] - p_oldSampLineBoundingRect[rectTop];
    int deltaTopPixels = (int)round(deltaTopSamples * p_viewport->scale());

    double deltaBottomSamples = p_sampLineBoundingRect[rectBottom] - p_oldSampLineBoundingRect[rectBottom];
    int deltaBottomPixels = (int)round(deltaBottomSamples * p_viewport->scale());

    //deltaW is the change in width in the visible area of the cube
    int deltaW = - deltaLeftPixels + deltaRightPixels;

    //deltaH is the change in height in the visible area of the cube
    int deltaH = - deltaTopPixels + deltaBottomPixels;

    //If the new visible width has changed (resized in the horizontal direction)
    if(p_XYBoundingRect.width() != p_oldXYBoundingRect.width()) {
      //Resized larger in the horizontal direction
      if(deltaW > 0) {
        //Using old height because we might lose data if new height is smaller
        resizeBuffer(p_XYBoundingRect.width(), p_oldXYBoundingRect.height());
        shiftBuffer(-deltaLeftPixels, 0);
  
        fillAreas.push_back(
            // left side that needs filled
            QRect(
              p_XYBoundingRect.topLeft(), 
              QPoint(p_XYBoundingRect.left() - deltaLeftPixels, p_XYBoundingRect.bottom())
              )
          );
  
        fillAreas.push_back(
            // right side that needs filled
            QRect(
                QPoint(p_XYBoundingRect.right() - deltaRightPixels, p_XYBoundingRect.top()),
                p_XYBoundingRect.bottomRight()
              )
          );
      }
      //Resized smaller in the horizontal direction
      else if(deltaW < 0) {
        shiftBuffer(-deltaLeftPixels, 0);
        resizeBuffer(p_XYBoundingRect.width(), p_oldXYBoundingRect.height());
      }
    }

    //If the new visible height has changed (resized in the vertical direction)
    if(p_XYBoundingRect.height() != p_oldXYBoundingRect.height()) {
      //Resized larger in the vertical direction
      if(deltaH > 0) {
        resizeBuffer(p_XYBoundingRect.width(), p_XYBoundingRect.height());
        shiftBuffer(0, -deltaTopPixels);
  
        QRect topSideToFill(p_XYBoundingRect.topLeft(), QPoint(p_XYBoundingRect.right(), p_XYBoundingRect.top() - deltaTopPixels));
        QRect bottomSideToFill(QPoint(p_XYBoundingRect.left(), p_XYBoundingRect.bottom() - deltaBottomPixels), p_XYBoundingRect.bottomRight());
  
        fillAreas.push_back(topSideToFill);
        fillAreas.push_back(bottomSideToFill);
      }
      //Resized smaller in the vertical direction
      else if(deltaH < 0) {
        shiftBuffer(0, -deltaTopPixels);
        resizeBuffer(p_XYBoundingRect.width(), p_XYBoundingRect.height());
      }
    }

    fillBuffer(fillAreas);
  }


  /**
   * Call this when the viewport is panned. DeltaX and deltaY are relative to the 
   * direction the buffer needs to shift. 
   * 
   * @param deltaX 
   * @param deltaY 
   */
  void ViewportBuffer::pan(int deltaX, int deltaY) {
    updateBoundingRects();

    if(!p_bufferInitialized || !p_enabled) { 
      return;
    }

    if(p_sampLineBoundingRect == p_oldSampLineBoundingRect) {
      //The sample line bounding rect contains the bounds of the
      //screen pixels to the cube sample/line bounds. If the cube
      //bounds do not change, then we do not need to do anything to 
      //the buffer.
      return;
    }

    double deltaLeftSamples =  p_sampLineBoundingRect[rectLeft] - p_oldSampLineBoundingRect[rectLeft];
    int deltaLeftPixels = (int)round(deltaLeftSamples * p_viewport->scale());

    double deltaTopSamples = p_sampLineBoundingRect[rectTop] - p_oldSampLineBoundingRect[rectTop];
    int deltaTopPixels = (int)round(deltaTopSamples * p_viewport->scale());

    QList<QRect> redrawAreas;

    //Left side of the visible area changed (start sample is different)
    if(p_sampLineBoundingRect[rectLeft] != p_oldSampLineBoundingRect[rectLeft]) {
      //Shifting data to the right
      if(deltaX > 0) {
        //The buffer is getting bigger
        resizeBuffer(p_XYBoundingRect.width(), p_oldXYBoundingRect.height());
        shiftBuffer(-deltaLeftPixels, 0);

        QRect rect(p_XYBoundingRect.topLeft(), QPoint(p_XYBoundingRect.left() + deltaX, p_XYBoundingRect.bottom()));
        redrawAreas.push_back(rect);
      }
      //Shifting data to the left
      else if(deltaX < 0) {
        //The buffer is getting smaller - no new data
        shiftBuffer(-deltaLeftPixels, 0);
        resizeBuffer(p_XYBoundingRect.width(), p_oldXYBoundingRect.height());

        // if new samples on the screen at all, mark it for reading
        if(p_sampLineBoundingRect[rectRight] != p_oldSampLineBoundingRect[rectRight]) {
          redrawAreas.push_back(
              QRect(QPoint(p_XYBoundingRect.right() + deltaX, p_XYBoundingRect.top()), p_XYBoundingRect.bottomRight())
              );
        }
      }
    }
    //Left side of the visible area is the same (start sample has not changed, but end sample may be different)
    else {
      resizeBuffer(p_XYBoundingRect.width(), p_oldXYBoundingRect.height());

      if(deltaX < 0) {
        redrawAreas.push_back(
            QRect(QPoint(p_XYBoundingRect.right() + deltaX, p_XYBoundingRect.top()), p_XYBoundingRect.bottomRight())
            );
      }
    }

    //Top side of the visible area changed (start line is different)
    if(p_sampLineBoundingRect[rectTop] != p_oldSampLineBoundingRect[rectTop]) {
      //Shifting data down
      if(deltaY > 0) {
        //The buffer is getting bigger
        resizeBuffer(p_XYBoundingRect.width(), p_XYBoundingRect.height());
        shiftBuffer(0, -deltaTopPixels);

        QRect rect(p_XYBoundingRect.topLeft(), QPoint(p_XYBoundingRect.right(), p_XYBoundingRect.top() + deltaY));
        redrawAreas.push_back(rect);
      }
      //Shifting data up
      else if(deltaY < 0) {
        //The buffer is getting smaller - no new data
        shiftBuffer(0, -deltaTopPixels);
        resizeBuffer(p_XYBoundingRect.width(), p_XYBoundingRect.height());

        // if new lines on the screen at all, mark it for reading
        if(p_sampLineBoundingRect[rectBottom] != p_oldSampLineBoundingRect[rectBottom]) {
          redrawAreas.push_back(
              QRect(QPoint(p_XYBoundingRect.left(), p_XYBoundingRect.bottom() + deltaY), p_XYBoundingRect.bottomRight())
            );
        }
      }
    }
    //Top side of the visible area is the same (start line has not changed, but end line may be different)
    else {
      resizeBuffer(p_XYBoundingRect.width(), p_XYBoundingRect.height());

      if(deltaY < 0) {
        redrawAreas.push_back(
            QRect(
              QPoint(p_XYBoundingRect.left(), p_XYBoundingRect.bottom() + deltaY), 
              p_XYBoundingRect.bottomRight()
            ));
      }
    }

    fillBuffer(redrawAreas);
  }

  /**
   * If p_totalBufferSize > large value then buffer will be cleared, otherwise
   * ignored, to try to limit memory usage.
   * 
   * cubeViewport will call this method on viewports buffers that are not related
   * to the active viewport. 
   *  
   * @param force If true, memory will be freed regardless of current total buffer 
   *              size (b/w -> rgb for example).
   */
  void ViewportBuffer::emptyBuffer(bool force) {
    if(force) {
      p_buffer.clear();
      p_bufferInitialized = false;
      return;
    }

    if(p_totalBufferSize > p_maxBufferSize) {
      //Write to tmp file
    }
  }

  /**
   * Method to see if the entire cube is in the buffer.
   * 
   * 
   * @return bool 
   */
  bool ViewportBuffer::hasEntireCube() {
    double sampTolerance = 0.05 * p_cube->Samples();
    double lineTolerance = 0.05 * p_cube->Lines();

    return p_sampLineBoundingRect[rectLeft] <= (1 + sampTolerance) &&
           p_sampLineBoundingRect[rectTop] <= (1 + lineTolerance) &&
           p_sampLineBoundingRect[rectRight] >= (p_cube->Samples() - sampTolerance) &&
           p_sampLineBoundingRect[rectBottom] >= (p_cube->Lines() - lineTolerance);
  }

  /**
   * Call this when zoomed, re-reads visible area.
   * 
   */
  void ViewportBuffer::scaleChanged() { 
    if(!p_enabled) return;

    updateBoundingRects(); 
    reinitialize();
  }

  /**
   * This turns on or off reading from the cube. If enabled is true the buffer 
   * will be re-read. 
   * 
   * @param enabled 
   */
  void ViewportBuffer::enable(bool enabled) {
    p_enabled = enabled;

    if(p_enabled) {
      updateBoundingRects(); 
      reinitialize();
    }
  }

  /**
   * Sets the band to read from, the buffer will be re-read if the band changes.
   * 
   * @param band 
   */
  void ViewportBuffer::setBand(int band) { 
    if(p_band == band) return;
    p_band = band;

    updateBoundingRects();

    if(!p_enabled) return;

    reinitialize();
  }

  /**
   * This resizes and fills entire buffer.
   * 
   */
  void ViewportBuffer::reinitialize() {
    resizeBuffer(p_XYBoundingRect.width(), p_XYBoundingRect.height());
    fillBuffer(p_XYBoundingRect);
    p_bufferInitialized = true;
  }

  /**
   * This removes overlapping areas in the list of rects and removes empty 
   * rectangles. 
   * 
   * @param rects 
   * 
   * @return QList<QRect> 
   */
  QList <QRect> ViewportBuffer::removeOverlaps(QList<QRect> rects) {
    for(int i = 0; i < rects.size(); i++) {
      for(int j = 0; j < rects.size(); j++) {
        if(i == j) continue;

        QRect &rect1 = rects[i];
        QRect &rect2 = rects[j];

        if(!rect1.intersects( rect2 )) continue;
  
        // left side is overlapping, rect1 is the horizontal rect
        if(rect1.left() < rect2.right() && rect1.left() >= rect2.left()) {
          // double check not losing data...
          if(rect1.intersected( rect2 ).height() == rect1.height()) {
            rect1.setLeft(rect2.right()+1);
          }
        }
    
        // right side is overlapping, rect1 is the horizontal rect
        if(rect1.right() > rect2.left() && rect1.right() <= rect2.right()) {
          // double check not losing data...
          if(rect1.intersected( rect2 ).height() == rect1.height()) {
            rect1.setRight(rect2.left()-1);
          }
        }
      }
    }

    // Delete empty rectangles
    for(int i = rects.size()-1; i >= 0; i--) {
      if(rects[i].left() == rects[i].right() || rects[i].top() == rects[i].bottom()) {
        rects.removeAt(i);
      }
    }

    return rects;
  }
}



