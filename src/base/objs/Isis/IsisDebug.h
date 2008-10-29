#ifndef IsisDebug_h
#define IsisDebug_h

#ifdef __GTHREADS
#error *****IsisDebug.h MUST be included before any system header files!*****
#endif

#ifdef CWDEBUG
  #include <libcwd/sys.h>
  #include <libcwd/debug.h>
  #include <execinfo.h>
  #include <dlfcn.h>

  #include <QMutex>
  class MyMutex : public QMutex {
    public:
      bool trylock() {
        return tryLock();
      }
  };

#endif
#endif
