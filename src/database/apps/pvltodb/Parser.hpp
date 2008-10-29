#if !defined(DbMapParser_hpp)
#define DbMapParser_hpp
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
#include <iostream>
#include <stack>
#include <functional>  

#include <boost/spirit/core.hpp>
#include <boost/spirit/attribute.hpp>
#include <boost/spirit/phoenix.hpp>
#include <boost/spirit/symbols/symbols.hpp>
#include <boost/spirit/utility/chset.hpp>

#include "DbMapParameter.h"
#include "MapFunctor.h"
#include "iException.h"


// using namespace boost::spirit;
namespace boost { namespace spirit {
using namespace phoenix;

struct var_closure : closure<var_closure, Isis::DbMapVar> {
  member1 var;
};

struct varlist_closure : closure<varlist_closure, std::vector<Isis::DbMapVar> > {
  member1 vlist;
};

struct parameter_closure : closure<parameter_closure, Isis::DbMapParameter * > {
  member1 parm;
};

//  Function to create a variable
struct create_var_impl {
  template <typename T, typename Iter1, typename Iter2> 
    struct result {
      typedef Isis::DbMapVar type;
    };

  template <typename T, typename Iter1, typename Iter2> 
    Isis::DbMapVar operator() (T vtype, Iter1 &first, Iter2 &second) const { 
      return Isis::DbMapVar(vtype, std::string(first, second));
    }
};

//  Function to create a variable
struct create_func_var_impl {
  template <typename T, typename S> 
    struct result {
      typedef Isis::DbMapVar type;
    };

  template <typename T, typename S>
    Isis::DbMapVar operator() (T vtype, S s) const {
      Isis::MapFunctor * &f = s;
      return Isis::DbMapVar(vtype, f->name());
    }
};

//  Function to create a variable
struct create_parameter_impl {
  template <typename T, typename V> 
    struct result {
      typedef Isis::DbMapParameter * type;
    };

  template <typename T, typename V> 
    Isis::DbMapParameter * operator() (T vtype, V &variable) const {

      switch (vtype) {
        case Isis::D_Function:
          return (new Isis::DbMapFunction(variable));
        case Isis::D_Reference:
          return (new Isis::DbMapReference(variable));
        case Isis::D_KeySpec:
          return (new Isis::DbMapKeySpec(variable));
        case Isis::D_Constant:
          return (new Isis::DbMapConstant(variable));
        default:
            std::ostringstream mess;
            mess << "Not a valid parameter type(" << vtype << ")";
            throw Isis::iException::Message(Isis::iException::Programmer, 
                                            mess.str(), _FILEINFO_);
      }
    }
};

//  Function to create a variable
struct create_key_parameter_impl {
  template <typename T, typename V> 
    struct result {
      typedef Isis::DbMapParameter * type;
    };

  template <typename T, typename V> 
    Isis::DbMapParameter * operator() (T vtype, V &variable) const {
      return (new Isis::DbMapKeySpec(variable));
    }
};

//  Function to create a variable
struct add_parameter_element_impl {
  template <typename T, typename V> 
    struct result {
      typedef void type;
    };

  template <typename P, typename V> 
    void operator() (P plist, V &vars) const { 
      plist->add(vars);
      return;
    }
};

 //  Function to create a variable
struct push_var_impl {
  template <typename T, typename V> 
    struct result {
      typedef void type;
    };

  template <typename L, typename V> 
    void operator() (L &vlist, V &variable) const { 
      vlist.push_back(variable);
    }
};

struct parameter : public grammar<parameter, parameter_closure::context_t> {
  parameter(symbols<Isis::MapFunctor *> &functors_) : functors(functors_) {}
  template <typename ScannerT>
    struct definition {

      phoenix::function<create_var_impl>             make_var;
      phoenix::function<create_func_var_impl>        make_func_var;
      phoenix::function<create_parameter_impl>       make_parm;
      phoenix::function<create_key_parameter_impl>   make_key_parm;
      phoenix::function<push_var_impl>               add_var;
      phoenix::function<add_parameter_element_impl>  add_to_parm;

        definition(parameter const& self) {

          //  Top-most defining element
          mapspec = mapelement[self.parm = arg1];

          // Four major signatures
          mapelement = functor[mapelement.parm = arg1]
                     | keyspec[mapelement.parm = arg1]
                     | reference[mapelement.parm = arg1]
                     | constant[mapelement.parm = arg1];

          //  Define the function signature
          functor = 
               ch_p('$') >> 
                funcname[functor.parm = make_parm(Isis::D_Function,arg1)] >> 
                ch_p('(') >> !arglist(functor.parm) >> ch_p(')');

          funcname =  as_lower_d[
                          self.functors[funcname.var = make_func_var(Isis::D_FuncName,arg1)]
                      ];

          arglist = funcarg[add_to_parm(arglist.parm,arg1)] >> 
                    *(ch_p(',') >> funcarg[add_to_parm(arglist.parm,arg1)]);

          funcarg = mapelement[funcarg.parm = arg1];
                     

          //  Define keyspec signature
          keyspec = 
             keyelement[keyspec.parm = make_key_parm(Isis::D_KeySpec,arg1)] >>
             *(keyelement[add_to_parm(keyspec.parm,arg1)]);

          keyelement = ch_p('/') >> 
                        keyword[add_var(keyelement.vlist,arg1)] >> 
                        !index[add_var(keyelement.vlist,arg1)];

          keyword = (alpha_p >> *(alpha_p | ch_p('_') | ch_p(':') | digit_p))
                     [keyword.var = make_var(Isis::D_Keyword, arg1, arg2)];

          index = 
            ch_p('[') >> 
              *blank_p >> 
                (u_integer[index.var = arg1] | variable[index.var = arg1]) >>
              *blank_p >> 
            ch_p(']');

          //  Integer values
          integer = u_integer[integer.var = arg1] 
                  | s_integer[integer.var = arg1];

          u_integer = 
            (+digit_p)[u_integer.var = make_var(Isis::D_Integer,arg1,arg2)];
          s_integer = 
            (!sign >> +digit_p)[s_integer.var = make_var(Isis::D_Integer,arg1,arg2)];

          //  Define a real value...meticulously
          real = 
            (!sign >> fraction >> !exponent  | +digit_p >> exponent)
                [real.var = make_var(Isis::D_Real, arg1, arg2)];

          fraction = *digit_p >> '.' >> +digit_p | +digit_p >> '.';

          exponent = (ch_p('e') | ch_p('E')) >> !sign >> +digit_p;

          sign = (ch_p ('+') | ch_p('-'));

          // Define the variable reference signature
          reference = 
            variable[reference.parm = make_parm(Isis::D_Reference, arg1)];

          variable = str_p("${") >> varname[variable.var = arg1] >> ch_p('}');

          varname = (alpha_p >> *( alpha_p | '_' | digit_p))
                    [varname.var = make_var(Isis::D_Variable,arg1,arg2)];

          // Define the literal/constant signature
          constant = literal[constant.parm = make_parm(Isis::D_Constant, arg1)]; 
          
          literal = numerics[literal.var = arg1]
                  | strlit[literal.var = arg1]
                  | varname[literal.var = arg1];

          numerics = longest_d [ 
                        integer[numerics.var = arg1]  | 
                        real[numerics.var = arg1]
                     ];

          // Miscellaneous constants;
          strlit = single_quoted_string[strlit.var = arg1]
                 | double_quoted_string[strlit.var = arg1]
                 | catchall[strlit.var = arg1];

          single_quoted_string = 
                  ch_p('\'') >> 
                    (*(anychar_p - ch_p('\'')))
                 [single_quoted_string.var = make_var(Isis::D_SQString,arg1,arg2)] >> 
                  ch_p('\'');

          double_quoted_string = 
                  ch_p('\"') >> 
                     (*(anychar_p - ch_p('\"')))
                [double_quoted_string.var = make_var(Isis::D_DQString,arg1,arg2)]  >> 
                  ch_p('\"');


          chset<> allowed("+-.");
          chset<> not_allowed(",()");

          catchall = ((allowed | alpha_p | digit_p) >> 
                      *(anychar_p - not_allowed))
                     [catchall.var = make_var(Isis::D_Constant, arg1,arg2)];

        }


        typedef rule<ScannerT, var_closure::context_t>       rule_v;
        typedef rule<ScannerT, varlist_closure::context_t>   rule_l;
        typedef rule<ScannerT, parameter_closure::context_t> rule_p;

        rule<ScannerT> mapspec;

        rule_p mapelement, functor, arglist, funcarg, 
               keyspec, reference, constant;

        rule_l keyelement;

        rule_v funcname, keyword, index, u_integer, s_integer, integer, 
               real, numerics, literal, variable, varname, strlit,
               single_quoted_string, double_quoted_string, catchall;

        rule<ScannerT> fraction, exponent, sign;

        
        // Parse entry and return
        rule<ScannerT> const& start() const { return mapspec; }
    };

    private:
      symbols<Isis::MapFunctor *> &functors;

  };
}} // namespace boost::spirit
#endif


