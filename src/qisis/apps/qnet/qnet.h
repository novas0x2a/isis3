#ifndef Qnet_h
#define Qnet_h

#include "SerialNumberList.h"
#include "FindImageOverlaps.h"
#include "ControlNet.h"

#ifdef IN_QNET
#define EXTERN
#else
#define EXTERN extern
#endif

namespace Qisis {
  namespace Qnet {
    EXTERN Isis::SerialNumberList *g_serialNumberList;
    EXTERN Isis::FindImageOverlaps *g_imageOverlap;
    EXTERN Isis::ControlNet *g_controlNetwork;
    EXTERN QList<int> g_filteredImages;
    EXTERN QList<int> g_filteredOverlaps;
    EXTERN QList<int> g_filteredPoints;
    EXTERN QList<int> g_activeImages;
    EXTERN QList<int> g_activeOverlaps;
    EXTERN QList<int> g_activePoints;
  }
}

#endif

