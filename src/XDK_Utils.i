// ============================================================================
//
// = CONTEXT
//   
//
// = FILENAME
//   XDK_Utils.i
//
// = AUTHOR
//   Nicolas Leclercq
//
// ============================================================================

//=============================================================================
// XDK_Utils::instance
//=============================================================================
XDK_INLINE XDK_Utils* XDK_Utils::instance ()
{
  return XDK_Utils::instance_;
}

//=============================================================================
// XDK_Utils::xop_name
//=============================================================================
XDK_INLINE const char* XDK_Utils::xop_name ()
{
  return this->xop_name_.c_str();
}

//=============================================================================
// XDK_Utils::notify
//=============================================================================
XDK_INLINE void XDK_Utils::notify (const std::string& txt)
{
  this->notify(txt.c_str());
}

//=============================================================================
// XDK_Utils::exec_igor_cmd 
//=============================================================================
XDK_INLINE int XDK_Utils::exec_igor_cmd (const std::string& cmd)
{
  return this->exec_igor_cmd(cmd.c_str());
}

//=============================================================================
// XDK_Utils::create_df                                          
//=============================================================================
XDK_INLINE DataFolderHandle XDK_Utils::create_df(const char* _path)
{
  return this->create_df(std::string(_path));
}

//=============================================================================
// XDK_Utils::create_df_obj                                          
//=============================================================================
XDK_INLINE int XDK_Utils::create_df_obj(DataFolderHandle _dfh, 
                                        const std::string& _obj, 
                                        const double& _r, 
                                        const double& _i, 
                                        bool cmplx)
{
  return this->create_df_obj(_dfh, _obj.c_str(), _r, _i, cmplx);
}

//=============================================================================
// XDK_Utils::create_df_obj                                          
//=============================================================================
XDK_INLINE int XDK_Utils::create_df_obj(DataFolderHandle _dfh, 
                                        const std::string& _obj, 
                                        yat::int32 _val)
{
  return this->create_df_obj(_dfh, _obj.c_str(), _val);
}

//=============================================================================
// XDK_Utils::create_df_obj                                          
//=============================================================================
XDK_INLINE int XDK_Utils::create_df_obj(DataFolderHandle _dfh, 
                                        const std::string& _obj, 
                                        const std::string& _str)
{
  return this->create_df_obj(_dfh, _obj.c_str(), _str.c_str());
}

//=============================================================================
// XDK_Utils::set_df_obj                                          
//=============================================================================
XDK_INLINE int XDK_Utils::set_df_obj(DataFolderHandle _dfh, 
                                     const std::string& _obj, 
                                     const double& _r, 
                                     const double& _i, 
                                     bool cmplx)
{
  return this->set_df_obj(_dfh, _obj.c_str(), _r, _i, cmplx);
}

//=============================================================================
// XDK_Utils::set_df_obj                                          
//=============================================================================
XDK_INLINE int XDK_Utils::set_df_obj(DataFolderHandle _dfh, 
                                     const std::string& _obj, 
                                     yat::int32 _val)
{
  return this->set_df_obj(_dfh, _obj.c_str(), _val);
}

//=============================================================================
// XDK_Utils::set_df_obj                                          
//=============================================================================
XDK_INLINE int XDK_Utils::set_df_obj(DataFolderHandle _dfh, 
                                     const std::string& _obj, 
                                     const std::string& _str)
{
  return this->set_df_obj(_dfh, _obj.c_str(), _str.c_str());
}

//=============================================================================
// XDK_Utils::get_df_obj                                          
//=============================================================================
XDK_INLINE int XDK_Utils::get_df_obj(DataFolderHandle _dfh, 
                                     const std::string& _obj, 
                                     double* val_)
{
  return this->get_df_obj(_dfh, _obj.c_str(), val_);
}

//=============================================================================
// XDK_Utils::get_df_obj                                          
//=============================================================================
XDK_INLINE int XDK_Utils::get_df_obj(DataFolderHandle _dfh, 
                                     const std::string& _obj, 
                                     yat::int32* val_)
{
  return this->get_df_obj(_dfh, _obj.c_str(), val_);
}

//=============================================================================
// XDK_Utils::fetch_or_make_wave                                          
//=============================================================================
XDK_INLINE int XDK_Utils::fetch_or_make_wave (waveHndl *whndl_, 
                                              DataFolderHandle _dfh, 
                                              const std::string& _wname, 
                                              int _wtype,
                                              int _change_wave,
                                              MDWaveDims _wdims)
{
  return this->fetch_or_make_wave(whndl_, _dfh, _wname.c_str(), _wtype, _change_wave, _wdims);
}

//=============================================================================
// XDK_Utils::fetch_wave                                          
//=============================================================================
XDK_INLINE int XDK_Utils::fetch_wave (waveHndl *whndl_, 
                                      DataFolderHandle _dfh, 
                                      const std::string& _wname, 
                                      int _wtype, 
                                      int _ndims)
{
  return this->fetch_wave(whndl_, _dfh, _wname.c_str(), _wtype, _ndims);
}


//=============================================================================
// XDK_Utils::tango_common_dfname                                         
//=============================================================================
XDK_INLINE const 
char* XDK_Utils::tango_common_df_path ()
{
  return XDK_Utils::common_df_path_;
}

//=============================================================================
// XDK_Utils::reset_error                                         
//=============================================================================
XDK_INLINE void XDK_Utils::reset_error ()
{
  this->set_tango_error_code(0);
  this->error_.errors.length(0);
}

//=============================================================================
// XDK_Utils::set_error                                         
//=============================================================================
XDK_INLINE void XDK_Utils::set_error (const std::string& _r, 
                                      const std::string& _d, 
                                      const std::string& _o,
                                      Tango::ErrSeverity _s)
{
  this->set_error(_r.c_str(), _d.c_str(), _o.c_str(), _s);
}

//=============================================================================
// XDK_Utils::push_error                                         
//=============================================================================
XDK_INLINE void XDK_Utils::push_error (const std::string& _r, 
                                       const std::string& _d, 
                                       const std::string& _o,
                                       Tango::ErrSeverity _s)
{
  this->push_error(_r.c_str(), _d.c_str(), _o.c_str(), _s);
}
