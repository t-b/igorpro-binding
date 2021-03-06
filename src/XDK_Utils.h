// ============================================================================
//
// = CONTEXT
//   XDK
//
// = FILENAME
//   XDK_Utils.h
//
// = AUTHOR
//   Nicolas Leclercq
//
// ============================================================================
#ifndef _XDK_UTILS_H_
#define _XDK_UTILS_H_

//=============================================================================
// DEPENDENCIES
//=============================================================================
#include <math.h>

//=============================================================================
// PLATFORM SPECIFIC UTILITIES
//=============================================================================
#if defined(_XDK_WINDOWS_)
# include "XDK_Utils_Win32.h"
#elif defined(_XDK_MACOSX_)
# include "XDK_Utils_MacOS.h"
#endif

//=============================================================================
// TYPEDEFS & SYNONYMES
//=============================================================================
typedef CountInt MDWaveDims[MAX_DIMENSIONS + 1];
typedef DataFolderHandle DFHndl;

//=============================================================================
// Class XDK_Utils (SINGLETON)
//=============================================================================
class XDK_Utils
{
  friend class XDK_Xop;


public :
  
  // XDK_Utils::fetch_or_make_wave <_change_existing_wave> options
  enum {
    DO_NOTHING,
    TYPE,
    DIMS,
    ANY
  };

  static XDK_Utils* instance ();
  // Returns the unique instance of <XDK_Utils>.

  //------------------------------------------------------
  //- Misc. ultility methods.
  //------------------------------------------------------
  const char* xop_name ();

  void notify (const char *format, ...); 
  void notify (const std::string& txt); 

  int exec_igor_cmd (const char *format, ...);
  int exec_igor_cmd (const std::string& cmd); 

  DFHndl create_df (const char* full_path);
  DFHndl create_df (const std::string& full_path);

  int create_df_obj(DFHndl _dfh, const char* _obj,  const double& _r, const double& _i = 0, bool cmplx = false);
  int create_df_obj(DFHndl _dfh, const std::string& _obj, const double& _r, const double& _i = 0, bool cmplx = false);
  int create_df_obj(DFHndl _dfh, const char* _obj, yat::int32 _val);
  int create_df_obj(DFHndl _dfh, const std::string& _obj, yat::int32 _val);
  int create_df_obj(DFHndl _dfh, const char* _obj, const char* _str);
  int create_df_obj(DFHndl _dfh, const std::string& _obj, const std::string& _str);
  
  int set_df_obj (DFHndl _dfh, const char* _obj, const double& _r, const double& _i = 0, bool cmplx = false);
  int set_df_obj (DFHndl _dfh, const std::string& _obj, const double& _r, const double& _i = 0, bool cmplx = false);
  int set_df_obj (DFHndl _dfh, const char* _obj, yat::int32 _val);
  int set_df_obj (DFHndl _dfh, const std::string& _obj, yat::int32 _val);
  int set_df_obj (DFHndl _dfh, const char* _obj, const char* _str);
  int set_df_obj (DFHndl _dfh, const std::string& _obj, const std::string& _str);

  int get_df_obj (DFHndl _dfh, const char* _obj, double* val_);
  int get_df_obj (DFHndl _dfh, const std::string& _obj, double* val_);
  int get_df_obj (DFHndl _dfh, const char* _obj, yat::int32* val_);
  int get_df_obj (DFHndl _dfh, const std::string& _obj, yat::int32* val_);
  int get_df_obj (DFHndl _dfh, const char* _obj, char* str_, yat::uint32 _size);
  int get_df_obj (DFHndl _dfh, const std::string& _obj,  std::string& str_);
  
  int get_obj_df (const std::string& _fpn, DFHndl& df_, std::string& obj_name_, bool create = false);

  int fetch_or_make_wave (waveHndl *whndl_, 
                          DFHndl _dfh, 
                          const char* _wname, 
                          int _wtype,
                          int _change_wave_opt = XDK_Utils::DO_NOTHING,
                          MDWaveDims _wdims = XDK_Utils::kdefault_dims);

  int fetch_or_make_wave (waveHndl *whndl_, 
                          DFHndl _dfh, 
                          const std::string& _wname, 
                          int _wtype,
                          int _change_wave_opt = XDK_Utils::DO_NOTHING,
                          MDWaveDims _wdims = XDK_Utils::kdefault_dims);

  int fetch_wave (waveHndl *whndl_, DFHndl _dfh, const char* _wname, int _wtype, int _ndims = 1);
  int fetch_wave (waveHndl *whndl_, DFHndl _dfh, const std::string& _wname, int _wtype, int _ndims = 1);

  int str_to_handle (const char* _cstr, Handle& h_);
  int str_to_handle (const std::string& _stdstr, Handle& h_);

  int handle_to_str (Handle& _hdl, char* cstr_, int _len, bool _release = true);
  int handle_to_str (Handle& _hdl, std::string& stdstr_, bool _release = true);

  //------------------------------------------------------
  //- TANGO Client specific methods
  //------------------------------------------------------
  int build_tango_default_objs ();

  const char* tango_common_df_path ();

  DFHndl tango_common_df ();

  std::string device_to_df_name (const std::string& dev_name);

  void reset_error ();

  void set_error (const char *r, 
                  const char *d, 
                  const char *o,
                  Tango::ErrSeverity s = Tango::ERR);

  void set_error (const std::string& r, 
                  const std::string& d, 
                  const std::string& o,
                  Tango::ErrSeverity s = Tango::ERR); 

  void set_error (const Tango::DevFailed &e);

  void push_error (const char *r, 
                   const char *d, 
                   const char *o,
                   Tango::ErrSeverity s = Tango::ERR);

  void push_error (const std::string& r, 
                   const std::string& d, 
                   const std::string& o,
                   Tango::ErrSeverity s = Tango::ERR); 

  void push_error (const Tango::DevFailed &e);

  int export_error ();

private:
  static int init (const char* xop_name);
  //- Instanciates the singleton.
  
  static void cleanup ();
  //- Releases the singleton.

  static XDK_Utils* instance_;
  //- The unique instance of <DeviceRepository>.

  XDK_Utils(const char* xop_name);
  //- Ctor

  int set_tango_error_code (int _err_code = kError);
  //- Set TANGO error global var (in Igor env.) to <_err_code>

  std::string xop_name_;
  //- A string to store the name of the XOP.

  static const char* common_df_path_;
  //- Path to the <DataFolder> where the last TANGO error is stored.

  Tango::DevFailed error_;
  //- Last error

  static MDWaveDims kdefault_dims;
  // Default wave dimensions (for fetch_or_make_wave)

  // = Disallow these operations (except for friends).
  //---------------------------------------------------------
  XDK_Utils (const XDK_Utils&);
  virtual ~XDK_Utils();
  void operator= (const XDK_Utils&); 
};


//=============================================================================
// MACROS 
//=============================================================================
#define XDK_UTILS XDK_Utils::instance()

#if defined(_XDK_DEBUG_)

# define XOP_ERROR(X) \
    do { \
      XDK_UTILS->notify("XOP-ERROR:: xop:%s - file:%s - line:%d", \
                        XDK_UTILS->xop_name(), __FILE__,__LINE__); \
      XDK_UTILS->notify X; \
    } while (0) \

# define XOP_WARNING(X) \
    do { \
      XDK_UTILS->notify("XOP-WARNING:: xop:%s - file:%s - line:%d", \
                        XDK_UTILS->xop_name(), __FILE__,__LINE__); \
      XDK_UTILS->notify X; \
    } while (0) \

# define XOP_MSG(X) \
    do { \
      XDK_UTILS->notify X; \
    } while (0) \

#else // _XDK_DEBUG_

# define XOP_ERROR(X) \
    do {} while (0) \

# define XOP_WARNING(C) \
    do {} while (0) \

# define XOP_MSG(X) \
    do {} while (0) \

#endif  //_XDK_DEBUG_

#if defined (__XDK_INLINE__)
  #include "XDK_Utils.i"
#endif 

#endif // _XDK_UTILS_H_