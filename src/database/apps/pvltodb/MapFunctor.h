#if !defined(MapFunctor_h)
#define MapFunctor_h
/**                                                                       
 * @file                                                                  
 * $Revision: 1.6 $                                                             
 * $Date: 2008/10/27 23:39:01 $
 *                                                                        
 *   Unless noted otherwise, the portions of Isis written by the USGS are 
 *   public domain. See individual third-party library and package descriptions 
 *   for intellectual property information, user agreements, and related  
 *   information.                                                         
 *                                                                        
 *   Although Isis has been used by the USGS, no warranty, expressed or   
 *   implied, is made by the USGS as to the accuracy and functioning of such 
 *   software and related material nor shall the fact of distribution     
 *   constitute any such warranty, and no responsibility is assumed by the
 *   USGS in connection therewith.                                        
 *                                                                        
 *   For additional information, launch                                   
 *   $ISISROOT/doc//documents/Disclaimers/Disclaimers.html                
 *   in a browser or see the Privacy &amp; Disclaimers page on the Isis website,
 *   http://isis.astrogeology.usgs.gov, and the USGS privacy and disclaimers on
 *   http://www.usgs.gov/privacy.html.                                    
 */                                                                       

#include <cstdio>
#include <cctype>

#include <string>
#include <iostream>
#include <sstream>
#include <stack>
#include <functional>

#include <boost/spirit/core.hpp>
#include <boost/spirit/symbols/symbols.hpp>


#include "SpiceUsr.h"
//#include "SpiceZfc.h"
//#include "SpiceZmc.h"

#include "CollectorMap.h"
#include "iTime.h"
#include "iString.h"
#include "Filename.h"


#include "geos/io/WKTReader.h"
#include "geos/geom/GeometryFactory.h"
#include "geos/geom/Geometry.h"
#include "geos/geom/MultiPolygon.h"
#include "geos/util/GEOSException.h"

namespace Isis {

class MapFunctor {
  public:
    MapFunctor() : _name() { }
    MapFunctor(const std::string &name) : _name(name) { }
    virtual ~MapFunctor() { }

    std::string name() const { return (_name); }
    virtual std::string exec(const std::vector<std::string> &args) const = 0;
    virtual int minArgs() const = 0;
    virtual int maxArgs() const = 0;

  private:
    std::string _name;
};

class Now : public MapFunctor {
  public:
    Now() : MapFunctor("now") { }
    ~Now() { }

    std::string exec(const std::vector<std::string> &args) const {
      return (iTime::CurrentLocalTime());
    }
    int minArgs() const { return (0); } 
    int maxArgs() const { return (0); }
};


inline bool loadNaifTiming( ) {
  static bool naifLoaded = false;

  if (!naifLoaded) {
//  Load the NAIF kernels to determine timing data
    Isis::Filename leapseconds("$base/kernels/lsk/naif????.tls");
    leapseconds.HighestVersion();

//  Load the kernels
    string leapsecondsName(leapseconds.Expanded());
    furnsh_c(leapsecondsName.c_str());

//  Ensure it is loaded only once
    naifLoaded = true;
  }
  return (true);
}

class UtcToJd : public MapFunctor {
  public:
    UtcToJd() : MapFunctor("utctojd") { }
    ~UtcToJd() { }

    std::string exec(const std::vector<std::string> &args) const {
      loadNaifTiming();

      std::string utc = iTime::CurrentLocalTime();
      if (args.size() > 0) utc = args[0];

      double et;
      utc2et_c(utc.c_str(), &et);

      iString trans(et);
      return (std::string(trans));
    }

    int minArgs() const { return (0); } 
    int maxArgs() const { return (1); }
};

class JdToUtc : public MapFunctor {
  public:
    JdToUtc() : MapFunctor("jdtoutc") { }
    ~JdToUtc() { }

    std::string exec(const std::vector<std::string> &args) const {
      enum { UTCLEN = 41 };
      loadNaifTiming();

      iString jd = iTime::CurrentLocalTime();
      if (args.size() > 0) jd = args[0];

      double et = jd.ToDouble();
      char utcout[UTCLEN];
      et2utc_c(et, "ISOC", 3, UTCLEN, utcout);
      return (std::string(utcout));
    }

    int minArgs() const { return (0); } 
    int maxArgs() const { return (1); }
};

class GeomFromText : public MapFunctor {
  public:
    GeomFromText() : MapFunctor("GeomFromText") { }
    ~GeomFromText() { }

    std::string exec(const std::vector<std::string> &args) const {
      if (args.size() != 1) {
        ostringstream mess;
        mess << "Invalid number of arguments (" << args.size() 
             << ") - requires exactly 1";
        throw iException::Message(Isis::iException::Programmer, mess.str(), 
                                  _FILEINFO_);
      }
      return (std::string("GeomFromText('" + args[0] + "', -1)"));
    }
    int minArgs() const { return (1); } 
    int maxArgs() const { return (1); }
};

class ToFormalCase : public MapFunctor {
  public:
    ToFormalCase() : MapFunctor("toformalcase") { }
    ~ToFormalCase() { }

    std::string exec(const std::vector<std::string> &args) const {
      if (args.size() != 1) {
        ostringstream mess;
        mess << "Invalid number of arguments (" << args.size() 
             << ") - requires exactly 1";
        throw iException::Message(Isis::iException::Programmer, mess.str(), 
                                  _FILEINFO_);
      }

      std::string str = iString::DownCase(args[0]);
      bool doUp(true);
      std::string::iterator pos;
      for (pos = str.begin() ; pos != str.end() ; ++pos) {
        if (doUp) {
          *pos = std::toupper(*pos);
          doUp = false;
        }

        if (std::isspace(*pos)) {
          doUp = true;
        }
        else {
          doUp = false;
        }
      }

      return (str);
    }

    int minArgs() const { return (1); } 
    int maxArgs() const { return (1); }
};


class IsEqual : public MapFunctor {
  public:
    IsEqual() : MapFunctor("isequal") { }
    ~IsEqual() { }

    std::string exec(const std::vector<std::string> &args) const {
      if (args.size() != 2) {
        ostringstream mess;
        mess << "Invalid number of arguments (" << args.size() 
             << ") - requires exactly 2";
        throw iException::Message(Isis::iException::Programmer, mess.str(), 
                                  _FILEINFO_);
      }

      std::string status("FALSE");
      if ( iString::Equal(args[0], args[1]) ) {
        status = "TRUE";
      }

      return (status);
    }

    int minArgs() const { return (2); } 
    int maxArgs() const { return (2); }
};

class Numeric : public MapFunctor {
  public:
    Numeric() : MapFunctor("numeric") { }
    ~Numeric() { }

    std::string exec(const std::vector<std::string> &args) const {
      if ((args.size() < 1) || (args.size() > 2)) {
        ostringstream mess;
        mess << "Invalid number of arguments (" << args.size() 
             << ") - must have 1 or 2 arguments";
        throw iException::Message(Isis::iException::Programmer, mess.str(), 
                                  _FILEINFO_);
      }

      // Use the iString class to determine if it is numeric
      try {
        iString number(args[0]);
        if ( number.size() == 0 ) { 
          throw iException::Message(iException::User, 
                                    "Empty string not a number", _FILEINFO_);
        }
        else {
          number.ToDouble();
        }

        return (args[0]);
      } catch ( iException &ie ) {
        ie.Clear();
        if ( args.size() == 2 ) return (args[1]);
      }

      return (std::string("NULL"));
    }

    int minArgs() const { return (1); } 
    int maxArgs() const { return (2); }
};

class NullIf : public MapFunctor {
  public:
    NullIf() : MapFunctor("nullif") { }
    ~NullIf() { }

    std::string exec(const std::vector<std::string> &args) const {
      if ((args.size() < 2) || (args.size() > 3)) {
        ostringstream mess;
        mess << "Invalid number of arguments (" << args.size() 
             << ") - must have 2 or 3 arguments";
        throw iException::Message(Isis::iException::Programmer, mess.str(), 
                                  _FILEINFO_);
      }

      if (iString::Equal(args[0],args[1])) {
        return (((args.size() == 3) ? args[2] : std::string("NULL")));
      }

      return (args[0]);
    }

    int minArgs() const { return (2); } 
    int maxArgs() const { return (3); }
};


class ToMultiPolygon : public MapFunctor {
  public:
    ToMultiPolygon() : MapFunctor("tomultipolygon") { }
    ~ToMultiPolygon() { }

    std::string exec(const std::vector<std::string> &args) const {
      if (args.size() != 1) {
        ostringstream mess;
        mess << "Invalid number of arguments (" << args.size() 
             << ") - must have 1 argument";
        throw iException::Message(Isis::iException::Programmer, mess.str(), 
                                  _FILEINFO_);
      }

      // Attempt to convert incoming string to GOES representation and check
      //  its type
      std::string wktpoly(args[0]);
      geos::geom::Geometry *poly(0);
      try {
        geos::io::WKTReader parser;
        poly = parser.read(wktpoly);
        if (poly->getGeometryTypeId() != geos::geom::GEOS_MULTIPOLYGON ) {
           vector<geos::geom::Geometry *> polys;
           polys.push_back(poly);
           const geos::geom::GeometryFactory *gfactory = geos::geom::GeometryFactory::getDefaultInstance();
           geos::geom::MultiPolygon *geom = gfactory->createMultiPolygon(polys);
           wktpoly = geom->toString();
           delete geom;
        }
      } catch ( geos::util::GEOSException &ge ) {
        //  Ensure return the default input string
        wktpoly = args[0];
      }

      delete poly;
      return (wktpoly);
    }

    int minArgs() const { return (1); } 
    int maxArgs() const { return (1); }
};



///  Define a function container for other classes to use
typedef CollectorMap<std::string,MapFunctor *,NoCaseStringCompare> FunctionList;

/**
 * @brief Template function to add all defined functions
 * 
 * This templated function can be used by any object that defines a method that
 * has the following signature:
 * @code
 *   void add(MapFunctor *func);
 * @endcode
 * 
 * Any such class that use this function takes ownership of all functions
 * added using this method.
 * 
 * @param Receiver  A class that has an add method compatible with the described
 *                  signature.
 * @param user An object of type Receiver that contains the add method
 * 
 * @return int  The number of functions added
 */
struct FunctionFactory {
  FunctionFactory() { }
  ~FunctionFactory() { }

  FunctionList getFunctions() const {
    FunctionList funcs;
    add(funcs,new Now());
    add(funcs,new UtcToJd());
    add(funcs,new JdToUtc());
    add(funcs, new GeomFromText());
    add(funcs, new ToFormalCase());
    add(funcs, new IsEqual()); 
    add(funcs, new Numeric()); 
    add(funcs, new NullIf()); 
    add(funcs, new ToMultiPolygon()); 
    return (funcs);
  }
  void add(FunctionList &funcs, MapFunctor *func) const {
    funcs.add(func->name(), func);
  }
};

/**
 * @brief Function to create a list of all available functions
 * 
 * @return FunctionList New list that must be managed by caller
 */
inline FunctionList getFunctionList() {
  FunctionFactory factory;
  return (factory.getFunctions());
}

inline void clearFunctions(FunctionList &funcs) {
  while (funcs.size() > 0) {
    MapFunctor *func = funcs.getNth(0);
    funcs.remove(func->name());
    delete func;
  }
  return;
}

}  // namespace Isis

#endif


