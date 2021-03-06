// ============================================================================
//
// = CONTEXT
//   XDK
//
// = FILENAME
//   XDK_Utils.cpp
//
// = AUTHOR
//   Nicolas Leclercq
//
// ============================================================================

//============================================================================
// DEPENDENCIEs
//=============================================================================
#include <cstdarg>
#include "XDK_StandardHeaders.h"
#include "XDK_Utils.h"

#if !defined (__XDK_INLINE__)
  #include "XDK_Utils.i"
#endif 

//=============================================================================
// #DEFINEs                                                 
//=============================================================================
#if defined (_XDK_DEBUG_)
# define INLINE_BUF_SIZE 1024
#else
# define INLINE_BUF_SIZE  512
#endif

#define kTANGO_ROOT_DF    "root:tango"
#define kTANGO_COMMON_DF  "root:tango:common"
#define kTANGO_DEVICE_DF  "root:tango:devices"

//=============================================================================
// STATIC MEMBERS
//=============================================================================
XDK_Utils* XDK_Utils::instance_ = 0;
MDWaveDims XDK_Utils::kdefault_dims;
const char* XDK_Utils::common_df_path_ = kTANGO_COMMON_DF;

//=============================================================================
// LOCAL CONSTS
//=============================================================================
static const char *lkTANGO_ERR_CODE = "error";
//----------------------------------------------------------------------------

//=============================================================================
// XDK_Utils::init
//=============================================================================
int XDK_Utils::init (const char* xop_name)
{
  if (XDK_Utils::instance_ != 0)
    return 0;

  ::MemClear(XDK_Utils::kdefault_dims, sizeof(MDWaveDims));

  XDK_Utils::instance_ = new XDK_Utils(xop_name);

  return (XDK_Utils::instance_) ? 0 : -1;
}

//=============================================================================
// XDK_Utils::cleanup
//=============================================================================
void XDK_Utils::cleanup ()
{
  if (XDK_Utils::instance_) {
    delete XDK_Utils::instance_;
    XDK_Utils::instance_ = 0;
  }
}

//=============================================================================
// XDK_Utils::XDK_Utils                                                
//=============================================================================
XDK_Utils::XDK_Utils (const char* _xop_name)
: xop_name_(std::string(_xop_name))
{

}

//=============================================================================
// XDK_Utils::~XDK_Utils                                                
//=============================================================================
XDK_Utils::~XDK_Utils ()
{

}

//=============================================================================
// XDK_Utils::notify                                                 
//=============================================================================
void XDK_Utils::notify (const char *format, ...)
{
  static char notify_str[INLINE_BUF_SIZE];

	char *buf = notify_str;

  size_t fmtlen = ::strlen(format) + 1;

  if (fmtlen > INLINE_BUF_SIZE)  
    buf = (char*)::WMNewPtr(fmtlen);
    
  va_list argptr;
  va_start(argptr, format);
  int len = ::vsprintf(buf, format, argptr);
  va_end(argptr);

  if (len > 0)
  {
    const char * cr = "\015";
    ::CallBack1(NOTICE, buf);
    ::CallBack1(NOTICE, const_cast<char*>(cr));  
  }
	
  if (buf != notify_str)
      ::WMDisposePtr((Ptr)buf);
}

//=============================================================================
// XDK_Utils::exec_igor_cmd                                          
//=============================================================================
int XDK_Utils::exec_igor_cmd (const char *format, ...)
{
  static char cmd_str[INLINE_BUF_SIZE];

  char *buf = cmd_str;
	
  size_t fmtlen = strlen(format) + 1;

  if (fmtlen > INLINE_BUF_SIZE)
      buf = (char*)WMNewPtr(fmtlen);
    
  va_list argptr;
  va_start(argptr, format);
  int len = ::vsprintf(buf, format, argptr);
  va_end(argptr);

  XOPIORecResult result = 0;

  if (len > 0)
      result = ::CallBack1(SILENT_COMMAND, buf);

  if (buf != cmd_str)
      ::WMDisposePtr((Ptr)buf);
	  
  return static_cast<int>(result);
}

//=============================================================================
// XDK_Utils::create_df                                          
//=============================================================================
DataFolderHandle XDK_Utils::create_df(const std::string& _path)
{
  //******************************************************
  // <_path> MUST BE A FULL PATH  (e.g. root:df1:df2:df3)
  //******************************************************

  DataFolderHandle dfh;

  //- if DF exists then return 
  if (::GetNamedDataFolder(0, const_cast<char*>(_path.c_str()), &dfh) == 0)
    return dfh;

  //- DF does not exist, create it
  DataFolderHandle parent_dfh;
  if (::GetRootDataFolder(0, &parent_dfh)) 
    return 0;

  //- Build full path
  std::string::size_type last_pos = 0;
  std::string::size_type pos = _path.find(':');
  if (pos == std::string::npos)
    return ::NewDataFolder(parent_dfh, const_cast<char*>(_path.c_str()), &dfh) ? dfh : 0;

  int done = 0;
  std::string token;
  std::string newdf;
  do {
    last_pos = ++pos;
    pos = _path.find(':', last_pos);
    if (pos == std::string::npos) 
    {
      pos = _path.size();
      done = 1;
    }
    newdf.assign(_path, 0, pos);
    if (::GetNamedDataFolder(0, const_cast<char*>(newdf.c_str()), &dfh)) 
    { 
      token.assign(_path, last_pos, pos - last_pos);
      if (::NewDataFolder(parent_dfh, const_cast<char*>(token.c_str()), &dfh)) 
      {
        dfh = 0;
        done = 1;
      }
    }
    parent_dfh = dfh;
  } while (!done);

  return dfh;
}

//=============================================================================
// XDK_Utils::create_df_obj                                          
//=============================================================================
int XDK_Utils::create_df_obj(DataFolderHandle _dfh, 
                             const char* _obj, 
                             const double & _r, 
                             const double & _i, 
                             bool _cmplx)
{
  int result = kNoError;

  DataFolderHandle last_dfh = 0;
  ::GetCurrentDataFolder(&last_dfh);
  do 
  {
    if (result = ::SetCurrentDataFolder(_dfh))  
      break;
    if (! _cmplx) 
    {
      if (
          result = ::SetIgorFloatingVar(_obj, 
                                        const_cast<double*>(&_r), 
                                        1)
         )
         break;
    }
    else 
    {
      if (
          result = ::SetIgorComplexVar(_obj, 
                                       const_cast<double*>(&_r), 
                                       const_cast<double*>(&_i), 
                                       1)
         )
         break;
    }
  } while (0);
  ::SetCurrentDataFolder(last_dfh);
  return result ? kError : kNoError;
}

//=============================================================================
// XDK_Utils::create_df_obj                                          
//=============================================================================
int XDK_Utils::create_df_obj (DataFolderHandle _dfh, const char* _obj, yat::int32 _val)
{
  bool restore_last_df = true;
  
  DataFolderHandle last_dfh = 0;

  if (::GetCurrentDataFolder(&last_dfh))
    restore_last_df = false;

  if (::SetCurrentDataFolder(_dfh) && restore_last_df) 
  {
    ::SetCurrentDataFolder(last_dfh);
    return kError;
  }

  if(::SetIgorIntVar(_obj, _val, 1)  && restore_last_df) 
  {
    ::SetCurrentDataFolder(last_dfh);
    return kError;
  }

  if (restore_last_df)
    ::SetCurrentDataFolder(last_dfh);

  return kNoError;
}

//=============================================================================
// XDK_Utils::create_df_obj                                          
//=============================================================================
int XDK_Utils::create_df_obj(DataFolderHandle _dfh, const char* _obj, const char* _str)
{
  bool restore_last_df = true;
  
  DataFolderHandle last_dfh = 0;

  if (::GetCurrentDataFolder(&last_dfh)) 
    restore_last_df = false;

  if (::SetCurrentDataFolder(_dfh) && restore_last_df) 
  {
    ::SetCurrentDataFolder(last_dfh);
    return kError;
  }

  if(::SetIgorStringVar(_obj, _str, 1)  && restore_last_df)
  {
    ::SetCurrentDataFolder(last_dfh);
    return kError;
  }

  if (restore_last_df) 
    ::SetCurrentDataFolder(last_dfh);

  return kNoError;
}

//=============================================================================
// XDK_Utils::set_df_obj                                          
//=============================================================================
int XDK_Utils::set_df_obj(DataFolderHandle _dfh, 
                          const char* _obj,
                          const double& _r, 
                          const double& _i,
                          bool _cmplx)
{
  int obj_type;
  DataObjectValue obj_val;
  
  if (::GetDataFolderObject(_dfh, _obj, &obj_type, &obj_val))
    return this->create_df_obj(_dfh, _obj, _r, _i, _cmplx);
      
  obj_val.nv.numType = _cmplx ? NT_FP64 : NT_FP64 | NT_CMPLX;
  obj_val.nv.spare = 0;	
  obj_val.nv.realValue = _r;
  obj_val.nv.imagValue = _i;

  if (::SetDataFolderObject(_dfh, _obj, VAR_OBJECT, &obj_val))
    return kError;

  return kNoError;
}

//=============================================================================
// XDK_Utils::set_df_obj                                          
//=============================================================================
int XDK_Utils::set_df_obj (DataFolderHandle _dfh, const char* _obj, yat::int32 _val)
{
  int obj_type;
  DataObjectValue obj_val;
  
  if (::GetDataFolderObject(_dfh, _obj, &obj_type, &obj_val))
    return this->create_df_obj(_dfh, _obj, _val);
  
  obj_val.nv.numType = NT_I32;
  obj_val.nv.spare = 0;	
  obj_val.nv.realValue = _val;
  obj_val.nv.imagValue =  0;

  if (::SetDataFolderObject(_dfh, _obj, VAR_OBJECT, &obj_val))
    return kError;

  return kNoError;
}

//=============================================================================
// XDK_Utils::set_df_obj                                          
//=============================================================================
int XDK_Utils::set_df_obj (DataFolderHandle _dfh, const char* _obj, const char* _str)
{
  int obj_type;
  DataObjectValue obj_val;
  
  if (::GetDataFolderObject(_dfh, _obj, &obj_type, &obj_val))
    return this->create_df_obj(_dfh, _obj, _str);
  
  if (this->str_to_handle(_str, obj_val.strH) != kNoError)
    return kError;
  
  int err = ::SetDataFolderObject(_dfh, _obj, STR_OBJECT, &obj_val);

  WMDisposeHandle(obj_val.strH);
    
  return (err != 0) ? kError : kNoError;
}

//=============================================================================
// XDK_Utils::get_df_obj                                          
//=============================================================================
int XDK_Utils::get_df_obj (DataFolderHandle _dfh, const char* _obj, double* val_)
{
  int obj_type;
  DataObjectValue obj_val;
  char* obj_name = const_cast<char*>(_obj);

  if (::GetDataFolderObject(_dfh, obj_name, &obj_type, &obj_val))
    return kError;
 
  if (obj_type != VAR_OBJECT)
    return kError;

  *val_ = obj_val.nv.realValue;

  return kNoError;
}

//=============================================================================
// XDK_Utils::get_df_obj                                          
//=============================================================================
int XDK_Utils::get_df_obj (DataFolderHandle _dfh, const char* _obj, yat::int32* val_)
{
  int obj_type;
  DataObjectValue obj_val;
  char* obj_name = const_cast<char*>(_obj);

  if (::GetDataFolderObject(_dfh, obj_name, &obj_type, &obj_val))
    return kError;
 
  if (obj_type != VAR_OBJECT)
    return kError;

  *val_ = (yat::int32)obj_val.nv.realValue;

  return kNoError;
}

//=============================================================================
// XDK_Utils::get_df_obj                                          
//=============================================================================
int XDK_Utils::get_df_obj (DataFolderHandle _dfh, const char* _obj,  char* str_, yat::uint32 _size)
{
  int obj_type;
  DataObjectValue obj_val;
  char* obj_name = const_cast<char*>(_obj);

  if (::GetDataFolderObject(_dfh, obj_name, &obj_type, &obj_val))
    return kError;
 
  if (obj_type != STR_OBJECT)
    return kError;

  BCInt strh_size = WMGetHandleSize(obj_val.strH);
  if (strh_size > static_cast<yat::int32>(_size) - 1)
    return kError;

  ::memset(str_, 0, _size);
  ::memcpy(str_, *(obj_val.strH), strh_size);

  return kNoError;
}

//=============================================================================
// XDK_Utils::get_df_obj                                          
//=============================================================================
int XDK_Utils::get_df_obj (DataFolderHandle _dfh, const std::string& _obj, std::string& stdstr_)
{
  int obj_type;
  DataObjectValue obj_val;
  char* obj_name = const_cast<char*>(_obj.c_str());

  if (::GetDataFolderObject(_dfh, obj_name, &obj_type, &obj_val))
    return kError;
 
  if (obj_type != STR_OBJECT)
    return kError;

  BCInt strh_size = ::WMGetHandleSize(obj_val.strH);

  stdstr_.assign(*(obj_val.strH), strh_size);

  return kNoError;
}

//=============================================================================
// XDK_Utils::get_obj_df
//=============================================================================
int XDK_Utils::get_obj_df (const std::string& fpn, 
                           DFHndl& df_, 
                           std::string& obj_name_,
                           bool create_df)
{
  std::string _fpn = fpn;

  std::string::size_type search_pos = std::string::npos;

  std::string::size_type sep_pos = _fpn.find(kNameSep);
  if ( sep_pos != std::string::npos )
    search_pos = sep_pos - 1;

  df_ = 0;
  std::string::size_type pos = _fpn.rfind(':', search_pos);
  if (pos == std::string::npos) 
  {
    obj_name_ = _fpn; 
  }
  else 
  {
    std::string df_path;
    df_path.assign(_fpn.begin(), _fpn.begin() + pos);
    obj_name_.assign(_fpn.begin() + pos + 1, _fpn.end());
    int res = ::GetNamedDataFolder(0, const_cast<char*>(df_path.c_str()), &df_);
    if (res)  
    {
      if (create_df && df_path.size()) 
        df_ = this->create_df(df_path);
      else 
        ::GetCurrentDataFolder(&df_);
    }
  }

  if (df_ == 0) 
    ::GetCurrentDataFolder(&df_);

  return (df_ != 0) ? kNoError : kError;
}

//=============================================================================
// XDK_Utils::handle_to_str
//=============================================================================
int XDK_Utils::handle_to_str (Handle& _hdl, char* cstr_, int _len, bool _release)
{
  if (_hdl == 0)
    return USING_NULL_STRVAR;

  if (::WMGetHandleSize(_hdl) >= _len) 
    return kError;

  ::memset(cstr_, 0, _len);

  ::strncpy(cstr_, *_hdl, ::WMGetHandleSize(_hdl));
  
  if (_release)
    ::WMDisposeHandle(_hdl);

  return kNoError;
}

//=============================================================================
// XDK_Utils::handle_to_str
//=============================================================================
int XDK_Utils::handle_to_str (Handle& _hdl, std::string& stdstr_, bool _release)
{
  if (_hdl == 0)
    return USING_NULL_STRVAR;

  stdstr_.assign(*_hdl, ::WMGetHandleSize(_hdl));

  if (_release)
    ::WMDisposeHandle(_hdl);

  return kNoError;
}

//=============================================================================
// XDK_Utils::str_to_handle
//=============================================================================
int XDK_Utils::str_to_handle (const char* _cstr, Handle& h_)
{
  if (! _cstr)
    return USING_NULL_STRVAR;

  h_ = ::WMNewHandle(::strlen(_cstr));
  if (! h_)
    return NOMEM;

  ::memset(*h_, 0, ::strlen(_cstr));
  ::memcpy(*h_, _cstr, ::strlen(_cstr));

  return kNoError;
}

//=============================================================================
// XDK_Utils::str_to_handle
//=============================================================================
int XDK_Utils::str_to_handle (const std::string& _stdstr, Handle& h_)
{
  h_ = ::WMNewHandle(_stdstr.size());
  if (! h_)
    return NOMEM;

  ::memset(*h_, 0, _stdstr.size());
  ::memcpy(*h_, _stdstr.c_str(), _stdstr.size());

  return kNoError;
}

//=============================================================================
// XDK_Utils::fetch_or_make_wave 
//=============================================================================
int XDK_Utils::fetch_or_make_wave (waveHndl *whndl_, 
                                   DataFolderHandle _dfh,
                                   const char* _wname,
                                   int _wtype,
                                   int _change_wave_opt,
                                   MDWaveDims _wdims)
{
  *whndl_ = ::FetchWaveFromDataFolder(_dfh, _wname);
  if (*whndl_ == 0) 
  {
    if (::MDMakeWave(whndl_, _wname, _dfh, _wdims, _wtype, 1))
		  return kError;
    return kNoError;
  }
  else if (_change_wave_opt == XDK_Utils::DO_NOTHING)
  {
    return kNoError;
  }
  
  int ndims;
  MDWaveDims dims;
  if (::MDGetWaveDimensions(*whndl_, &ndims, dims)) 
    return kError;

  int not_same_type = ::WaveType(*whndl_) != _wtype;
  int not_same_dims = ::memcmp(_wdims, dims, sizeof(MDWaveDims));

  int do_it = 0;
  if (_change_wave_opt == XDK_Utils::ANY && (not_same_type || not_same_dims)) 
    do_it = 1;
  else if (_change_wave_opt == XDK_Utils::TYPE && not_same_type) 
    do_it = 1;
  else if (_change_wave_opt == XDK_Utils::DIMS && not_same_dims) 
    do_it = 1;

  if (do_it && ::MDChangeWave(*whndl_, _wtype, _wdims)) 
    return kError;

  return kNoError;
}

//=============================================================================
// XDK_Utils::fetch_wave
//=============================================================================
int XDK_Utils::fetch_wave (waveHndl *whndl_, 
                           DataFolderHandle _dfh, 
                           const char* _wname,
                           int _wtype, 
                           int _num_dims)
{
  *whndl_ = ::FetchWaveFromDataFolder(_dfh, _wname);
  if (*whndl_ == 0) 
    return kError;

  if (::WaveType(*whndl_) != _wtype)
    return kError;

  int ndims;
  if (::MDGetWaveDimensions(*whndl_, &ndims, 0) || (ndims != _num_dims))
    return kError;
	
  return kNoError;
}

//=============================================================================
// XDK_Utils::tango_common_df                                        
//=============================================================================
DataFolderHandle XDK_Utils::tango_common_df ()
{
  //- get DF handle
  DataFolderHandle dfh = 0;
  if (::GetNamedDataFolder(0, const_cast<char*>(this->tango_common_df_path()), &dfh)) 
  {
    //- user may have killed the DF
    dfh = this->create_df(this->tango_common_df_path());
  }
  return dfh;
}

//=============================================================================
// XDK_Utils::set_tango_error_code                                         
//=============================================================================
int XDK_Utils::set_tango_error_code (int _err_code)
{
  //- get target DF handle
  DataFolderHandle dfh = this->tango_common_df();
  if (dfh == 0) 
    return kError;

  //- update value
  if (this->set_df_obj(dfh, lkTANGO_ERR_CODE, (yat::int32)_err_code))
    return kError;

  return kNoError;
}

//=============================================================================
// XDK_Utils::device_to_df_name
//=============================================================================
std::string XDK_Utils::device_to_df_name (const std::string& _dev_name)
{
  std::string df_name(kTANGO_DEVICE_DF);
  df_name += ':' + _dev_name;
  std::string::size_type pos = 0;
  do 
  {
    pos = df_name.find('/', pos);
    if (pos != std::string::npos) 
    {
      df_name.replace(pos, 1, ":", 1);
    } else 
    {
      break;
    }
    pos++;
  } while (1);
  return df_name;
}

//=============================================================================
// XDK_Utils::set_error                                       
//=============================================================================
void XDK_Utils::set_error (const Tango::DevFailed &e)
{
  this->reset_error();
  yat::int32 nb_err = e.errors.length();
  this->error_.errors.length(nb_err);
  for (int i = 0; i < nb_err; i++) 
  {
    this->error_.errors[i].severity = e.errors[i].severity;
    this->error_.errors[i].reason = CORBA::string_dup(e.errors[i].reason);
    this->error_.errors[i].desc = CORBA::string_dup(e.errors[i].desc);
    this->error_.errors[i].origin = CORBA::string_dup(e.errors[i].origin);
  }
}

//=============================================================================
// XDK_Utils::set_error                                       
//=============================================================================
void XDK_Utils::set_error (const char *_r, 
                           const char *_d, 
                           const char *_o,
                           Tango::ErrSeverity _s) 
{
  this->reset_error();
  this->error_.errors.length(1);
  this->error_.errors[0].severity = _s;
  this->error_.errors[0].reason = CORBA::string_dup(_r);
  this->error_.errors[0].desc = CORBA::string_dup(_d);
  this->error_.errors[0].origin = CORBA::string_dup(_o);
}

//=============================================================================
// XDK_Utils::push_error                                       
//=============================================================================
void XDK_Utils::push_error (const char *_r, 
                            const char *_d, 
                            const char *_o,
                            Tango::ErrSeverity _s) 
{
  this->set_tango_error_code();
  yat::int32 nb_err = this->error_.errors.length();
  this->error_.errors.length(nb_err + 1);
  this->error_.errors[nb_err].severity = _s;
  this->error_.errors[nb_err].reason = CORBA::string_dup(_r);
  this->error_.errors[nb_err].desc = CORBA::string_dup(_d);
  this->error_.errors[nb_err].origin = CORBA::string_dup(_o);
}

//=============================================================================
// XDK_Utils::push_error                                       
//=============================================================================
void XDK_Utils::push_error (const Tango::DevFailed &e)
{
  this->set_tango_error_code();
  yat::int32 add_nb_err = e.errors.length();
  yat::int32 cur_nb_err = this->error_.errors.length();
  yat::int32 total_err = cur_nb_err + add_nb_err;
  this->error_.errors.length(total_err);
  int i, j;
  for (i = cur_nb_err, j = 0; i < total_err; i++, j++) 
  {
    this->error_.errors[i].severity = e.errors[j].severity;
    this->error_.errors[i].reason = CORBA::string_dup(e.errors[j].reason);
    this->error_.errors[i].desc = CORBA::string_dup(e.errors[j].desc);
    this->error_.errors[i].origin = CORBA::string_dup(e.errors[j].origin);
  }
}

//=============================================================================
// XDK_Utils::export_error                                    
//=============================================================================
int XDK_Utils::export_error ()
{
  DataFolderHandle dfh = this->tango_common_df();
  if (dfh == 0)
    return kError;

  waveHndl err_w; 
  MDWaveDims dims;
  ::MemClear(&dims, sizeof(MDWaveDims));
  yat::int32 nb_err = this->error_.errors.length();
  dims[0] = nb_err;
  dims[1] = 4;
  if (this->fetch_wave(&err_w, dfh, "error_stack", TEXT_WAVE_TYPE, 1) == kNoError)
    ::KillWave(err_w);

  if (this->fetch_or_make_wave(&err_w, dfh, "error_stack", TEXT_WAVE_TYPE, XDK_Utils::ANY, dims))
    return kError;

  if (nb_err == 0)
    return kNoError;

  Handle temp;
  for (int i = nb_err - 1, j = 0; i >= 0; i--, j++) 
  {
    dims[0] = j;
    dims[1] = 0;
    switch (this->error_.errors[i].severity) 
    {
      case Tango::WARN:
        if (XDK_UTILS->str_to_handle("WARNING", temp))
          return kError;
        break;
      case Tango::ERR:
        if (XDK_UTILS->str_to_handle("ERROR", temp))
          return kError;
        break;
      case Tango::PANIC:
        if (XDK_UTILS->str_to_handle("PANIC", temp))
          return kError;
        break;
    }
    ::MDSetTextWavePointValue(err_w, dims, temp);
    ::WMDisposeHandle(temp);
   
    dims[1] = 1;
    if (XDK_UTILS->str_to_handle(this->error_.errors[i].reason, temp))
      return kError;
    ::MDSetTextWavePointValue(err_w, dims, temp);
    ::WMDisposeHandle(temp);
    
    dims[1] = 2;
    if (XDK_UTILS->str_to_handle(this->error_.errors[i].desc, temp)) 
      return kError;
    ::MDSetTextWavePointValue(err_w, dims, temp);
    ::WMDisposeHandle(temp);
    
    dims[1] = 3;
    if (XDK_UTILS->str_to_handle(this->error_.errors[i].origin, temp))
      return kError;

    ::MDSetTextWavePointValue(err_w, dims, temp);
    ::WMDisposeHandle(temp);
  }
  return kNoError;
}
