#if !defined(DbMapParameter_hpp)
#define DbMapParameter_hpp
/**                                                                       
 * @file                                                                  
 * $Revision: 1.1 $                                                             
 * $Date: 2007/03/30 17:19:48 $
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

#include <string>
#include <vector>
#include <iostream>

#include "DbPvlMapUtil.h"
#include "iString.h"
#include "iException.h"

namespace Isis {

typedef enum {
  D_Undefined = 0,
  D_Constant,
  D_Integer,
  D_Real,
  D_String,
  D_DQString,
  D_SQString,
  D_Variable,
  D_Literal,
  D_Reference,
  D_Keyword,
  D_Index,
  D_KeySpec,
  D_Function,
  D_FuncName,
  D_FuncArg,
} DbMapType;


class DbMapVar {
  public:
    DbMapVar() : _type(D_Undefined), _value() { }
    DbMapVar(const DbMapType vtype, const std::string &val) :  _type(vtype),
                                                               _value(val) {}
    virtual ~DbMapVar() { }

    DbMapType type() const { return (_type); }
    std::string value() const { return (_value); }

  private:
    DbMapType   _type;
    std::string _value;
};

class DbMapParameter {
  public:
    typedef std::vector<DbMapVar> DbMapVarList;
    typedef std::vector<DbMapParameter *> DbMapParameterList;

    DbMapParameter() : _type(D_Undefined), _plist(), _vlist() { }
    DbMapParameter(DbMapType ptype) : _type(ptype), _plist(), _vlist() { }
    DbMapParameter(DbMapType ptype, DbMapVarList &vars) : _type(ptype), 
                                                          _plist(), _vlist() { 
       _vlist.insert(_vlist.end(), vars.begin(), vars.end());
    } 

    virtual ~DbMapParameter() { destroy(); }

    DbMapType type() const { return (_type); }

    virtual void add(const DbMapVar &var) {  _vlist.push_back(var); }
    virtual void add(const std::vector<DbMapVar> &vars) {
       _vlist.insert(_vlist.end(), vars.begin(), vars.end());
    }
    virtual void add(DbMapParameter *parm) { _plist.push_back(parm); }

    virtual bool Evaluate(DbResource &resource, std::string &value) const = 0;

  protected:
    DbMapType   _type;
    std::vector<DbMapParameter *> _plist;
    std::vector<DbMapVar>         _vlist;

  private:
    void destroy() {
      for (unsigned int i = 0 ; i < _plist.size() ; i++) {
        delete _plist[i];
      }
      _plist.clear();
      _vlist.clear();
    }
};

class DbMapReference : public DbMapParameter {
public:
  DbMapReference(const DbMapVar &refval) : DbMapParameter(D_Reference), 
                                           _refvar(refval) { }
  virtual void add(const DbMapVar &var) {
    throw iException::Message(iException::Programmer, 
                              "Cannot add additional variables to references!",
                              _FILEINFO_);
  }

  virtual void add(const std::vector<DbMapVar> &vars) {
    throw iException::Message(iException::Programmer, 
                              "Cannot add additional variables to references!",
                              _FILEINFO_);
  }
  virtual void add(DbMapParameter *parm) {
    throw iException::Message(iException::Programmer, 
                              "Cannot add additional parameters to references!",
                              _FILEINFO_);
  }

  virtual bool Evaluate(DbResource &resource, std::string &value) const {
    if (resource.exists(_refvar.value())) {
      value = resource.get(_refvar.value());
      return (true);
    }
    value = "";
    return (false);
  }

  private:  
    DbMapVar _refvar;
};

class DbMapConstant : public DbMapParameter {
public:
  DbMapConstant(const DbMapVar &var) : DbMapParameter(D_Constant), 
                                           _constant(var) { }
  virtual void add(const DbMapVar &var) {
    throw iException::Message(iException::Programmer, 
                              "Cannot add additional variables to constants!",
                              _FILEINFO_);
  }

  virtual void add(const std::vector<DbMapVar> &vars) {
    throw iException::Message(iException::Programmer, 
                              "Cannot add additional variables to constants!",
                              _FILEINFO_);
  }
  virtual void add(DbMapParameter *parm) {
    throw iException::Message(iException::Programmer, 
                              "Cannot add additional parameters to constants!",
                              _FILEINFO_);
  }


  virtual bool Evaluate(DbResource &resource, std::string &value) const {
    value = _constant.value();
    return (true);
  }

  private:  
    DbMapVar _constant;
};


class DbMapKeySpec : public DbMapParameter {
  public:
    DbMapKeySpec(const DbMapVar &key) : DbMapParameter(D_KeySpec), 
                                        _keys(), _lastError() {
      addKey(key);
    }
    DbMapKeySpec(const DbMapVarList &keys) : DbMapParameter(D_KeySpec), 
                                             _keys(), _lastError() {
      addKeyList(keys);
    }

    virtual ~DbMapKeySpec() { }


    virtual void add(const DbMapVar &var) {
      if (var.type() != D_Keyword) {
        throw iException::Message(iException::Programmer, 
                                  "Variable type must be a keyword!",
                                  _FILEINFO_);
      }
      addKey(var);
    }

    virtual void add(const DbMapVarList &vars) {
      addKeyList(vars);
    }

    virtual void add(DbMapParameter *parm) {
      throw iException::Message(iException::Programmer, 
                                "Cannot add parameters to keyword elements!",
                                _FILEINFO_);
    }

    virtual bool Evaluate(DbResource &resource, std::string &value) const {
      return (findObject(resource.getSource(), resource.getVariables(),
                         _keys.begin(), _keys.end(), value));
    }

  private:
    struct KeyElement {
      KeyElement() : key(), index() { }
      KeyElement(const DbMapVar &var) : key(var), index() { }
      KeyElement(const DbMapVar &var, const DbMapVar &_index) : key(var), 
                                                                index(_index) { }
      DbMapVar key;
      DbMapVar index;
    };

    typedef std::vector<KeyElement> KeyList;
    typedef KeyList::const_iterator KeyListConstIter;
    typedef KeyList::iterator KeyListIter;
    KeyList _keys;

    mutable std::string _lastError;      //!< Record last error

    void addKey(const DbMapVar &key) {
      _keys.push_back(KeyElement(key));
      return;
    }

    void addKeyList(const DbMapVarList &keys) {
      if (keys.size() == 1) {
        addKey(keys[0]);
      }
      else if (keys.size() == 2) {
        _keys.push_back(KeyElement(keys[0], keys[1]));
      }
      else {
        ostringstream mess;
        mess << "Invalid number of arguments (" << keys.size() 
             << ") - requires 1 or 2 variables only" << std::ends;
        throw iException::Message(Isis::iException::Programmer, mess.str(), 
                                  _FILEINFO_);
      }
      return;
    }

    bool getIndex(const DbMapVar &var, const Variables &vref, int &index) const;
    bool findObject(PvlObject &source, const Variables &vars, 
                    KeyListConstIter current, KeyListConstIter end,
                    std::string &value) const;
    bool findGroup(PvlObject &source, const Variables &vars, 
                    KeyListConstIter current, KeyListConstIter end,
                    std::string &value) const;
    bool findKeyword(PvlContainer &source, const Variables &vars, 
                       KeyListConstIter current, KeyListConstIter end,
                       std::string &value) const;
    bool findNothing(PvlContainer &source, const Variables &vars, 
                    KeyListConstIter current, KeyListConstIter end,
                    std::string &value) const;

};

class DbMapFunction : public DbMapParameter {
  public:
    DbMapFunction(const DbMapVar &name) : DbMapParameter(D_Function), 
                                          _name(name) { }

    virtual void add(const DbMapVar &var) {
      throw iException::Message(iException::Programmer, 
                                "Cannot add raw variable to functions!",
                                _FILEINFO_);

    }

    virtual void add(const DbMapVarList &vlist) {
      throw iException::Message(iException::Programmer, 
                                "Cannot add variable lists to functions!",
                                _FILEINFO_);
    }

    virtual void add(DbMapParameter *parm) {
      _plist.push_back(parm);
    }

    virtual bool Evaluate(DbResource &resource, std::string &value) const {
      MapFunctor *func = resource.getFunction(_name.value());
      if (!func) {
        ostringstream mess;
        mess << "Function \"" << _name.value() << "\" does not exist!" 
             << std::ends;
        throw iException::Message(Isis::iException::Programmer, mess.str(), 
                                  _FILEINFO_);
      }

      std::vector<std::string> args;
      for (unsigned int i = 0 ; i < _plist.size() ; i++) {
        std::string funcValue;
        if (!_plist[i]->Evaluate(resource,funcValue)) return (false);
        args.push_back(funcValue);
      }

      value = func->exec(args);
      return (true);
    }

  private:
    DbMapVar _name;
    std::vector<DbMapParameter *> _plist;
};


} // namspace Isis


#endif
