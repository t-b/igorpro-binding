// ============================================================================
//
// = CONTEXT
//   Tango Generic Client for Igor Pro
//
// = FILENAME
//   TangoBinding.cpp
//
// = AUTHOR
//   Nicolas Leclercq
//
// ============================================================================

//=============================================================================
// DEPENDENCIES
//=============================================================================
#include "XDK_StandardHeaders.h"
#include "DevRepository.h"
#include "DataCodec.h"
#include "MonitoredObjectManager.h"
#include "TangoBinding.h"

#if !defined (__XDK_INLINE__)
  #include "TangoBinding.i"
#endif 

//=============================================================================
// STATIC MEMBERS
//=============================================================================
TangoBinding* TangoBinding::instance_ = 0;

//=============================================================================
// MACROS
//=============================================================================
//-- BEGIN _TRY MACRO ---------------------------------------
#define _TRY(_invoke, _dev, _cmd) \
  try { \
     _invoke; \
  } \
  catch (const Tango::DevFailed &e) { \
    XDK_UTILS->set_error(e); \
    std::string r = std::string(_cmd) + " failed"; \
    std::string d = "failed to execute " + std::string(_cmd); \
    d += " on device " + _dev; \
    std::string o = "TangoBinding::" + std::string(_cmd); \
    XDK_UTILS->push_error(r.c_str(), d.c_str(), o.c_str()); \
		return kError; \
	} \
  catch (...) { \
    std::string o = "TangoBinding::" + std::string(_cmd); \
    XDK_UTILS->set_error("unknown error", \
                         "unknown exception caught", \
                         o.c_str()); \
    return kError; \
  } \
//-- END _TRY MACRO ------------------------------------------

//-- BEGIN _IS_WRITABLE MACRO --------------------------------
#define _IS_WRITABLE(_X) \
  (_X.writable == Tango::READ_WITH_WRITE || _X.writable == Tango::READ_WRITE)
//-- BEGIN _IS_WRITABLE MACRO --------------------------------

//=============================================================================
// TangoBinding::init
//=============================================================================
int TangoBinding::init ()
{
  if (TangoBinding::instance_)
    return kNoError;

  if (DevRepository::init() == kError)
    return kError;

  if (DataCodec::init() == kError)
    return kError;

  if (MonitoredObjectManager::init() == kError)
    return kError;

  TangoBinding::instance_ = new TangoBinding;

  return (TangoBinding::instance_) ? kNoError : kError;
}

//=============================================================================
// TangoBinding::cleanup
//=============================================================================
void TangoBinding::cleanup ()
{
  MonitoredObjectManager::cleanup();

  DataCodec::cleanup();

  DevRepository::cleanup();

  if (TangoBinding::instance_) 
  {
    delete TangoBinding::instance_;
    TangoBinding::instance_ = 0;
  }
}

//=============================================================================
// TangoBinding::TangoBinding 
//=============================================================================
TangoBinding::TangoBinding ()
{
 // no-op ctor
}

//=============================================================================
// TangoBinding::~TangoBinding 
//=============================================================================
TangoBinding::~TangoBinding ()
{
 // no-op dtor
}

//=============================================================================
// TangoBinding::new_experiment 
//=============================================================================
int TangoBinding::new_experiment ()
{
  int res = kNoError;
 
  if (MON_OBJ_MANAGER)   
    MON_OBJ_MANAGER->remove_attribute("*", "*");

  if (DEV_REP) 
    DEV_REP->remove_device("*");

  return res;
}

//=============================================================================
// TangoBinding::load_experiment 
//=============================================================================
int TangoBinding::load_experiment ()
{
  return this->new_experiment();
}

//=============================================================================
// TangoBinding::open_device 
//=============================================================================
int TangoBinding::open_device (const std::string& _dev)
{
  DevDescriptor* ddesc = DEV_REP->device_desc(_dev, true);
  if (ddesc == 0) 
  {
    std::string r = "failed to open device " + _dev;
    XDK_UTILS->push_error(r.c_str(),
                          "could not obtain a valid device reference",
                          "TangoBinding::open_device");
    return kError;
  }

  return kNoError;
}

//=============================================================================
// TangoBinding::close_device 
//=============================================================================
int TangoBinding::close_device (const std::string& _dev)
{
  //- remove/kill all monitors for the specified device
  MON_OBJ_MANAGER->remove_attribute(_dev, "*");

  //- remove device from the devices repository
  DEV_REP->remove_device(_dev);

  return kNoError;
}

//=============================================================================
// TangoBinding::command_in_out 
//=============================================================================
int TangoBinding::command_in_out (const std::string& _dev, 
                                  const std::string& _cmd, 
                                  const std::string& _arg_in, 
                                  const std::string& _arg_out)
{
  //- Get device descriptor
  DevDescriptor* ddesc = DEV_REP->device_desc(_dev);
  if (ddesc == 0) 
  {
    std::string r = "failed to execute " + _cmd + " on " + _dev;
    XDK_UTILS->push_error(r.c_str(),
                          "could not obtain a valid device reference",
                          "TangoBinding::command_in_out");
    return kError;
  }

  //- Is <cmd> a valid command for <dev>
  int cmd_id = ddesc->cmd_exists(_cmd);
  if (cmd_id == kError) 
  {
    std::string d = _cmd + " is not a valid " + _dev + " command";
    XDK_UTILS->set_error("API_CommandNotFound",
                         d.c_str(),
                         "TangoBinding::command_in_out");
    return kError;
  }

  Tango::DeviceData dd_out;

  int arg_in_type = (ddesc->cmd_list())[cmd_id].in_type;

  if (arg_in_type != Tango::DEV_VOID) 
  {
    //- Encode argin
    Tango::DeviceData dd_in;
    if (DATA_CODEC->encode_argin(ddesc, _arg_in, cmd_id, dd_in)) 
      return kError;
    //- Exec command
    _TRY(dd_out = ddesc->proxy()->command_inout(const_cast<string&>(_cmd), dd_in), _dev, "command_inout");
  }
  else 
  {
    //- Exec command
    _TRY(dd_out = ddesc->proxy()->command_inout(const_cast<string&>(_cmd)), _dev, "command_inout");
  }

  int arg_out_type = (ddesc->cmd_list())[cmd_id].out_type;

  if (arg_out_type != Tango::DEV_VOID) 
  {
    //- Decode argout
    if (DATA_CODEC->decode_argout(ddesc, _arg_out, cmd_id, dd_out))
      return kError;
  }

  return kNoError;
}


//=============================================================================
// TangoBinding::read_attribute 
//=============================================================================
int TangoBinding::read_attribute (const std::string& _dev, 
                                  const std::string& _attr, 
                                  const std::string& _arg_out)
{
  //- Get device descriptor
  DevDescriptor * ddesc = DEV_REP->device_desc(_dev);
  if (ddesc == 0) 
  {
    std::string r = "failed to read attribute " + _attr + " on " + _dev;
    XDK_UTILS->push_error(r.c_str(),
                          "could not obtain a valid device reference",
                          "TangoBinding::read_attribute");
    return kError;
  }

  //- Is <_attr> an attribute of <_dev>
  int attr_id = ddesc->attr_exists(_attr);
  if (attr_id == kError) 
  {
    std::string d = _attr + " is not a valid " + _dev + " attribute";
    XDK_UTILS->set_error("API_AttrNotFound",
                         d.c_str(),
                         "TangoBinding::read_attribute");
    return kError;
  }

  //- Read attribute 
  Tango::DeviceAttribute value;
  _TRY(value = ddesc->proxy()->read_attribute(const_cast<string&>(_attr)), 
                                              _dev, 
                                              "read_attribute");

  //- Decode argout
  if (DATA_CODEC->decode_attr(ddesc, _arg_out, attr_id, value)) 
  {
    std::string r = "could not read attribute " + _attr + " on " + _dev;
    XDK_UTILS->push_error(r.c_str(),
                          "failed to extract value from device answer",
                          "TangoBinding::read_attribute");
    return kError;
  }

  return kNoError;
}

//=============================================================================
// TangoBinding::read_attributes 
//=============================================================================
int TangoBinding::read_attributes (const std::string& _dev, 
                                   const std::string& _input)
{
  //-- check _input
  if (_input.size() == 0) 
  {
    XDK_UTILS->set_error("invalid argument specified",
                         "empty string passed as function argument",
                         "TangoBinding::read_attributes");
    return kError; 
  }

  //-- get dev descriptor
  DevDescriptor* ddesc = DEV_REP->device_desc(_dev);
  if (ddesc == 0) 
  {
    std::string r = "failed to read attributes on " + _dev;
    XDK_UTILS->push_error(r.c_str(),
                          "could not obtain a valid device reference",
                          "TangoBinding::read_attribute");
    return kError;
  }

  //- get current data folder
  DFHndl dfh; 
  if (::GetCurrentDataFolder(&dfh)) 
  {
    XDK_UTILS->set_error("XOP internal error",
                         "GetCurrentDataFolder failed",
                         "TangoBinding::read_attributes");
    return kError;
  }

  //- first try to fetch a 2D text wave named <_input> in <dfh>
  waveHndl tw;
  if (XDK_UTILS->fetch_wave(&tw, dfh, _input, TEXT_WAVE_TYPE, 2) == 0) 
  {
    return this->read_attributes_i(ddesc,tw);
  }

  //- no text wave named <_input> in the current df
  //- does the user provide the actual argin or a string containing argin?
  //- first try to get a global string var named <_input> in <dfh>
  std::string input;
  if (XDK_UTILS->get_df_obj (dfh, _input, input)) 
  {
    //- there is no string var named <_argin.c_str> in <dfh>
    //- the user provides the actual string value
    input = _input;
  }

  return this->read_attributes_i(ddesc, input);
}

//=============================================================================
// TangoBinding::read_attributes_i 
//=============================================================================
int TangoBinding::read_attributes_i (DevDescriptor* _ddesc, waveHndl _tw)
{
  //-- get wave dims
  int tw_ndims = 0;
  MDWaveDims tw_dims;
  if (::MDGetWaveDimensions(_tw, &tw_ndims, tw_dims)) 
  {
    XDK_UTILS->set_error("XOP internal error",
                         "MDGetWaveDimensions failed",
                         "TangoBinding::read_attributes");
    return kError;
  }
  size_t i;
  //-- attr names
  std::vector<std::string> attr_names;
  attr_names.resize(tw_dims[0]);
  //-- attr ids
  std::vector<int> attr_ids;
  attr_ids.resize(tw_dims[0]);
  //-- argout names
  std::vector<std::string> argout_names;
  argout_names.resize(tw_dims[0]);
  //-- convet wave content to CORBA strings and store them into <attr_names>   
  MDWaveDims dim_indx;
  ::MemClear(dim_indx, sizeof(MDWaveDims));
  Handle txt_hndl = ::WMNewHandle(0);
  if (txt_hndl == 0) 
  {
    XDK_UTILS->set_error("API_MemoryAllocation",
                         "WMNewHandle failed",
                         "TangoBinding::read_attributes");
    return kError; 
  }
  char *tmp = 0;
  for (i = 0; i < (size_t)tw_dims[0]; i++) 
  {
    dim_indx[0] = i;
    //-- get attr name
    dim_indx[1] = 0;
    if (::MDGetTextWavePointValue(_tw, dim_indx, txt_hndl)) 
    {
      WMDisposeHandle(txt_hndl);
      XDK_UTILS->set_error("XOP internal error",
                           "MDGetTextWavePointValue failed",
                           "TangoBinding::read_attributes");
      return kError;
    }
    tmp = CORBA::string_alloc(static_cast<CORBA::ULong>(::WMGetHandleSize(txt_hndl) + 1));
    if (tmp == 0) 
    {
      WMDisposeHandle(txt_hndl);
      XDK_UTILS->set_error("API_MemoryAllocation",
                           "CORBA::string_alloc failed",
                           "TangoBinding::read_attributes");
      return kError; 
    }
    ::MemClear(tmp, ::WMGetHandleSize(txt_hndl) + 1);
    ::memcpy(tmp, *txt_hndl, ::WMGetHandleSize(txt_hndl));
    attr_names[i] = std::string(tmp);
    attr_ids[i] = _ddesc->attr_exists(tmp);
    CORBA::string_free(tmp);
    if (attr_ids[i] == kError) 
    {
      WMDisposeHandle(txt_hndl);
      std::string d = attr_names[i] + " is not a valid " + _ddesc->name() + " attribute";
      XDK_UTILS->set_error("API_AttrNotFound",
                           d.c_str(),
                           "TangoBinding::read_attributes");
      return kError;
    }
    //-- get argout name
    dim_indx[1] = 1;
    if (::MDGetTextWavePointValue(_tw, dim_indx, txt_hndl)) 
    {
      WMDisposeHandle(txt_hndl);
      XDK_UTILS->set_error("XOP internal error",
                           "MDGetTextWavePointValue failed",
                           "TangoBinding::read_attributes");
      return kError;
    }
    if (XDK_UTILS->handle_to_str(txt_hndl, argout_names[i], false)) 
    {
      WMDisposeHandle(txt_hndl);
      XDK_UTILS->set_error("XOP internal error",
                           "handle_to_str failed",
                           "TangoBinding::read_attributes");
      return kError;  
    }
  } 
  //- release memory
  WMDisposeHandle(txt_hndl); 

  std::vector<Tango::DeviceAttribute> * attr_values;

  _TRY(attr_values = _ddesc->proxy()->read_attributes(attr_names), _ddesc->name(), "read_attributes");

  for (i = 0; i < attr_values->size(); i++) 
  {
    if (DATA_CODEC->decode_attr(_ddesc, argout_names[i], attr_ids[i], (*attr_values)[i])) 
    {
      delete attr_values; 
      std::string r = "could not read attributes on " + _ddesc->name();
      XDK_UTILS->push_error(r.c_str(),
                            "failed to extract values from device answer",
                            "TangoBinding::read_attributes");
      return kError;
    }
  }

  delete attr_values; 

  return kNoError;
}

//=============================================================================
// TangoBinding::read_attributes_i
//=============================================================================
int TangoBinding::read_attributes_i (DevDescriptor* _ddesc, const std::string& _input)
{
  //-- we have a maximum of <_ddesc->attr_list().size()> attrs <this> device
  int max = _ddesc->attr_list().size();
  if (max == 0) 
  {
    std::string  d = "device " + _ddesc->name() + " has no attributes";
    XDK_UTILS->set_error("API_AttrNotFound",
                         d.c_str(),
                         "TangoBinding::read_attributes");
    return kError;
  }
  //-- get the num of attr we have to read
  int nattr = -1;
  std::string::size_type pos_av = -1;
  do 
  {
    pos_av = _input.find(kNameSep, pos_av + 1);
    nattr++;
  } while (pos_av != std::string::npos);
  //-- input should contain (at least) one <attr>>val> pair.
  if (nattr == 0) 
  {
    XDK_UTILS->set_error("invalid argument specified",
                         "syntax error in <attr:val> list",
                         "TangoBinding::read_attributes");
    return kError;
  }
  //-- attr names
  std::vector<std::string> attr_names;
  attr_names.resize(nattr);
  //-- attr ids
  std::vector<int> attr_ids;
  attr_ids.resize(nattr);
  //-- argout names
  std::vector<std::string> argout_names;
  argout_names.resize(nattr);
  pos_av = -1;
  std::string tmp;
  std::string::size_type pos_avp = -1;
  for (size_t i = 0; i < (size_t)nattr; i++) 
  {
    pos_av = _input.find(kNameSep, pos_avp + 1);
    if (pos_av == std::string::npos)
    {
      std::string d = "Error while trying to process the attribute list '" + _input + "'";
      XDK_UTILS->set_error("API_InvalidArgument",
                           d.c_str(),
                           "TangoBinding::read_attributes");
      return kError;
    }
    tmp.assign(_input, pos_avp + 1, pos_av - 1 - pos_avp);
    attr_names[i] = CORBA::string_dup(tmp.c_str());
    attr_ids[i] = _ddesc->attr_exists(tmp);
    if (attr_ids[i] == kError) 
    {
      std::string d = std::string(attr_names[i]) + " is not a valid " + _ddesc->name() + " attribute";
      XDK_UTILS->set_error("API_AttrNotFound",
                           d.c_str(),
                           "TangoBinding::read_attributes");
      return kError;
    }
    pos_av++;
    pos_avp = _input.find(';', pos_av);
    if (pos_avp == std::string::npos) 
      pos_avp = _input.size();
    tmp.assign(_input, pos_av, pos_avp - pos_av);
    argout_names[i] = tmp;
  }  

  std::vector<Tango::DeviceAttribute> * attr_values;

  _TRY(attr_values = _ddesc->proxy()->read_attributes(attr_names), _ddesc->name(), "read_attributes");

  for (size_t i = 0; i < attr_values->size(); i++) 
  {
    if (DATA_CODEC->decode_attr(_ddesc, argout_names[i], attr_ids[i], (*attr_values)[i])) 
    {
      std::string r = "could not read attributes on " + _ddesc->name();
      XDK_UTILS->push_error(r.c_str(),
                            "failed to extract values from device answer",
                            "TangoBinding::read_attributes");
      return kError;
    }
  }

  delete attr_values;

  return kNoError;
}

//=============================================================================
// TangoBinding::write_attribute 
//=============================================================================
int TangoBinding::write_attribute (const std::string& _dev, 
                                   const std::string& _attr, 
                                   const std::string& _arg_in)
{
  //- Get device descriptor
  DevDescriptor* ddesc = DEV_REP->device_desc(_dev);
  if (ddesc == 0) 
  {
    std::string r = "failed to write attribute " + _attr + " on " + _dev;
    XDK_UTILS->push_error(r.c_str(),
                          "could not obtain a valid device reference",
                          "TangoBinding::write_attribute");
    return kError;
  }
  //- Is <cmd> a valid command for <dev>
  int attr_id = ddesc->attr_exists(_attr);
  if (attr_id == kError) 
  {
    std::string d = _attr + " is not a valid " + _dev + " attribute";
    XDK_UTILS->set_error("API_AttrNotFound",
                         d.c_str(),
                         "TangoBinding::write_attribute");
    return kError;
  }

  if (ddesc->is_attr_writable(attr_id) == false) 
  {
    std::string d = "attribute " + _attr + " of " + _dev + " is not writable";
    XDK_UTILS->set_error("API_AttrNotWritable",
                         d.c_str(),
                         "TangoBinding::write_attribute");
    return kError;
  }

  //- Encode argin
  Tango::DeviceAttribute value;

  if (DATA_CODEC->encode_attr(ddesc, _arg_in, attr_id, value)) 
  {
    std::string r = "could not write attribute " + _attr + " on device " + _dev;
    XDK_UTILS->push_error(r.c_str(),
                          "failed to extract data from argin",
                          "TangoBinding::write_attribute");
    return kError;
  }

  //- Write attribute 
  _TRY(ddesc->proxy()->write_attribute(value), _dev, "write_attribute");

  return kNoError;
}


//=============================================================================
// TangoBinding::write_attributes 
//=============================================================================
int TangoBinding::write_attributes (const std::string& _dev, 
                                      const std::string& _input)
{
  //-- check _input
  if (_input.size() == 0) 
  {
    XDK_UTILS->set_error("invalid argument specified",
                         "empty string passed as function argument",
                         "TangoBinding::write_attributes");
    return kError; 
  }
  //-- get dev descriptor
  DevDescriptor* ddesc = DEV_REP->device_desc(_dev);
  if (ddesc == 0) 
  {
    std::string d = "could not obtain a valid reference for device " + _dev; 
    XDK_UTILS->push_error("XOP internal error",
                          d.c_str(),
                          "TangoBinding::write_attributes");
    return kError;
  }
  //- get current data folder
  DFHndl dfh; 
  if (::GetCurrentDataFolder(&dfh)) 
  {
    XDK_UTILS->set_error("XOP internal error",
                         "GetCurrentDataFolder failed",
                         "TangoBinding::write_attributes");
    return kError;
  }
  //- first try to fetch a 2D text wave named <_input> in <dfh>
  waveHndl tw;
  if (XDK_UTILS->fetch_wave(&tw, dfh, _input, TEXT_WAVE_TYPE, 2) == 0) 
  {
    return this->write_attributes_i(ddesc, tw);
  }
  //- no text wave named <_input> in the current df
  //- does the user provide the actual argin or a string containing argin?
  //- first try to get a global string var named <_input> in <dfh>
  std::string input;
  if (XDK_UTILS->get_df_obj (dfh, _input, input)) 
  {
    //- there is no string var named <_argin.c_str> in <dfh>
    //- the user provides the actual string value
    input = _input;
  }
  return this->write_attributes_i(ddesc, input);
}

//=============================================================================
// TangoBinding::write_attributes_i 
//=============================================================================
int TangoBinding::write_attributes_i (DevDescriptor* _ddesc, waveHndl _tw)
{
  //-- get wave dims
  int tw_ndims = 0;
  MDWaveDims tw_dims;
  if (::MDGetWaveDimensions(_tw, &tw_ndims, tw_dims)) 
  {
    XDK_UTILS->set_error("XOP internal error",
                         "MDGetWaveDimensions failed",
                         "TangoBinding::write_attributes");
    return kError;
  }
  //-- attr names
  std::vector<std::string> attr_names;
  attr_names.resize(tw_dims[0]);
  //-- attr ids
  std::vector<int> attr_ids;
  attr_ids.resize(tw_dims[0]);
  //-- argin names
  std::vector<std::string> argin_names;
  argin_names.resize(tw_dims[0]);
  //-- convet wave content to CORBA strings and store them into <attr_names>   
  MDWaveDims dim_indx;
  ::MemClear(dim_indx, sizeof(MDWaveDims));
  Handle txt_hndl = ::WMNewHandle(0);
  if (txt_hndl == 0) 
  {
    XDK_UTILS->set_error("API_MemoryAllocation",
                         "WMNewHandle failed",
                         "TangoBinding::write_attributes");
    return kError;
  }
  for (size_t i = 0; i < (size_t)tw_dims[0]; i++) 
  {
    dim_indx[0] = i;
    //-- get attr name
    dim_indx[1] = 0;
    if (::MDGetTextWavePointValue(_tw, dim_indx, txt_hndl)) 
    {
      WMDisposeHandle(txt_hndl);
      XDK_UTILS->set_error("XOP internal error",
                           "MDGetTextWavePointValue failed",
                           "TangoBinding::write_attributes");
      return kError; 
    }
    if (XDK_UTILS->handle_to_str(txt_hndl, attr_names[i], false)) 
    {
      WMDisposeHandle(txt_hndl);
      XDK_UTILS->set_error("XOP internal error",
                           "handle_to_str failed",
                           "TangoBinding::write_attributes");
      return kError; 
    }
    attr_ids[i] = _ddesc->attr_exists(attr_names[i]);
    if (attr_ids[i] == kError) 
    {
      WMDisposeHandle(txt_hndl);
      std::string d = std::string(attr_names[i]) + " is not a valid " + _ddesc->name() + " attribute";
      XDK_UTILS->set_error("API_AttrNotFound",
                           d.c_str(),
                           "TangoBinding::write_attributes");
      return kError;
    }
    if (_ddesc->is_attr_writable(attr_ids[i]) == false) 
    {
      WMDisposeHandle(txt_hndl);
      std::string d = "attribute " + std::string(attr_names[i]) + " of " + _ddesc->name() + " is not writable";
      XDK_UTILS->set_error("API_AttrNotWritable",
                           d.c_str(),
                           "TangoBinding::write_attributes");
      return kError;
    }
    //-- get argout name
    dim_indx[1] = 1;
    if (::MDGetTextWavePointValue(_tw, dim_indx, txt_hndl)) 
    {
      WMDisposeHandle(txt_hndl);
      XDK_UTILS->set_error("XOP internal error",
                           "MDGetTextWavePointValue failed",
                           "TangoBinding::write_attributes");
      return kError; 
    }
    if (XDK_UTILS->handle_to_str(txt_hndl, argin_names[i], false)) 
    {
      WMDisposeHandle(txt_hndl);
      XDK_UTILS->set_error("XOP internal error",
                           "handle_to_str failed",
                           "TangoBinding::write_attributes");
      return kError; 
    }
  } 
  WMDisposeHandle(txt_hndl);

  std::vector<Tango::DeviceAttribute> attr_values;
  attr_values.resize(tw_dims[0]);
  for (size_t i = 0; i < (size_t)tw_dims[0]; i++) 
  {
    if (DATA_CODEC->encode_attr(_ddesc, argin_names[i], attr_ids[i], attr_values[i])) 
    {
      std::string r = "could not write attribute " + std::string(attr_names[i]) + " on " + _ddesc->name();
      XDK_UTILS->push_error(r.c_str(),
                            "failed to extract data from argin",
                            "TangoBinding::write_attributes");
      return kError;
    }
  }

  _TRY(_ddesc->proxy()->write_attributes(attr_values),  _ddesc->name(), "write_attributes");

  return kNoError;
}

//=============================================================================
// TangoBinding::write_attributes_i 
//=============================================================================
int TangoBinding::write_attributes_i (DevDescriptor* _ddesc, 
                                      const std::string& _input)
{
  //-- we have a maximum of <_ddesc->attr_list().size()> attr <this> device
  int max = _ddesc->attr_list().size();
  if (max == 0) 
  {
    std::string  d = _ddesc->name() + " has no attributes";
    XDK_UTILS->set_error("API_AttrNotFound",
                         d.c_str(),
                         "TangoBinding::write_attributes");
    return kError; 
  }
  //-- get the num of attr we have to read
  int nattr = -1;
  std::string::size_type pos_av = -1;
  do 
  {
    pos_av = _input.find(kNameSep, pos_av + 1);
    nattr++;
  } while (pos_av != std::string::npos);
  //-- input should contain (at least) one <attr>>val> pair.
  if (nattr == 0) 
  {
    XDK_UTILS->set_error("invalid argument specified",
                         "syntax error in <attr:val> list",
                         "TangoBinding::write_attributes");
    return kError;
  }
  //-- attr names
  std::vector<std::string> attr_names;
  attr_names.resize(nattr);
  //-- attr ids
  std::vector<int> attr_ids;
  attr_ids.resize(nattr);
  //-- argout names
  std::vector<std::string> argin_names;
  argin_names.resize(nattr);
  pos_av = -1;
  std::string tmp;
  std::string::size_type pos_avp = -1;
  for (size_t i = 0; i < (size_t)nattr; i++) 
  {
    pos_av = _input.find(kNameSep, pos_avp + 1);
    tmp.assign(_input, pos_avp + 1, pos_av - 1 - pos_avp);
    attr_names[i] = CORBA::string_dup(tmp.c_str());
    attr_ids[i] = _ddesc->attr_exists(tmp);
    if (attr_ids[i] == kError) 
    {
      std::string d = std::string(attr_names[i]) + " is not a valid " + _ddesc->name() + " attribute";
      XDK_UTILS->set_error("API_AttrNotFound",
                           d.c_str(),
                           "TangoBinding::write_attributes");
      return kError;
    }
    if (_ddesc->is_attr_writable(attr_ids[i]) == false) 
    {
      std::string d = "attribute " + std::string(attr_names[i]) + " of " + _ddesc->name() + " is not writable";
      XDK_UTILS->set_error("API_AttrNotWritable",
                           d.c_str(),
                           "TangoBinding::write_attributes");
      return kError;
    }
    pos_av++;
    pos_avp = _input.find(';', pos_av);
    if (pos_avp == std::string::npos) 
    {
      pos_avp = _input.size();
    }
    tmp.assign(_input, pos_av, pos_avp - pos_av);
    argin_names[i] = tmp;
  }
  
  std::vector<Tango::DeviceAttribute> attr_values;
  attr_values.resize(nattr);
  for (size_t i = 0; i < (size_t)nattr; i++) 
  {
    if (DATA_CODEC->encode_attr(_ddesc, argin_names[i], attr_ids[i], attr_values[i])) 
    {
      std::string r = "could not write attribute " + std::string(attr_names[i]) + " on " + _ddesc->name();
      XDK_UTILS->push_error(r.c_str(),
                            "failed to extract data from argin",
                            "TangoBinding::write_attributes");
      return kError;
    }
  }

  _TRY(_ddesc->proxy()->write_attributes(attr_values), _ddesc->name(), "write_attributes");

  return kNoError;
}

//=============================================================================
// TangoBinding::status
//=============================================================================
int TangoBinding::status (const std::string& _dev, std::string& status_)
{
  DevDescriptor* ddesc = DEV_REP->device_desc(_dev);
  if (ddesc == 0) 
  {
    std::string r = "failed to get " + _dev + " status";
    XDK_UTILS->push_error(r.c_str(),
                          "could not obtain a valid device reference",
                          "TangoBinding::status");
    return kError;
  }

  _TRY(status_ = ddesc->proxy()->status(), _dev, "status");

  return kNoError;
}

//=============================================================================
// TangoBinding::ping
//=============================================================================
int TangoBinding::ping (const std::string& _dev)
{
  DevDescriptor* ddesc = DEV_REP->device_desc(_dev);
  if (ddesc == 0) 
  {
    std::string r = "failed to ping " + _dev;
    XDK_UTILS->push_error(r.c_str(),
                          "could not obtain a valid device reference",
                          "TangoBinding::ping");
    return kError;
  }

  _TRY(return ddesc->proxy()->ping(), _dev, "ping");

  return kNoError;
}

//=============================================================================
// TangoBinding::set_timeout
//=============================================================================
int TangoBinding::set_timeout (const std::string& _dev, int _timeout)
{
  DevDescriptor* ddesc = DEV_REP->device_desc(_dev);
  if (ddesc == 0) 
  {
    std::string r = "failed to set " + _dev + " time out";
    XDK_UTILS->push_error(r.c_str(),
                          "could not obtain a valid device reference",
                          "TangoBinding::set_timeout");
    return kError;
  }

  _TRY(ddesc->proxy()->set_timeout_millis(_timeout), _dev, "set_timeout");

  return kNoError;
}

//=============================================================================
// TangoBinding::get_timeout
//=============================================================================
int TangoBinding::get_timeout (const std::string& _dev)
{
  DevDescriptor* ddesc = DEV_REP->device_desc(_dev);
  if (ddesc == 0) 
  {
    std::string r = "failed to get " + _dev + " time out";
    XDK_UTILS->push_error(r.c_str(),
                          "could not obtain a valid device reference",
                          "TangoBinding::get_timeout");
    return kError;
  }

  _TRY(return ddesc->proxy()->get_timeout_millis(), _dev, "get_timeout");

  return kNoError;
}

//=============================================================================
// TangoBinding::black_box
//=============================================================================
int TangoBinding::black_box (const std::string& _dev, 
                             const std::string& _argout, 
                             yat::int32 _n)
{
  DevDescriptor* ddesc = DEV_REP->device_desc(_dev);
  if (ddesc == 0) 
  {
    std::string r = "failed to get " + _dev + " black box";
    XDK_UTILS->push_error(r.c_str(),
                          "could not obtain a valid device reference",
                          "TangoBinding::black_box");
    return kError;
  }

  //- create device:attributes datafolder
  std::string tmp_df_name = XDK_UTILS->device_to_df_name(_dev) + ":tmp";
  DFHndl tmp_dfh = XDK_UTILS->create_df(tmp_df_name);
  if (tmp_dfh == 0) 
  {
    std::string d = "could not create tmp datafolder for device " + _dev;
    XDK_UTILS->set_error("XOP internal error",
                         d.c_str(),
                         "TangoBinding::black_box");
    return kError;
  }

  std::vector<std::string> * bb;
  _TRY(bb = ddesc->proxy()->black_box(_n), _dev, "black_box");

  waveHndl w;
  int result = (int)bb->size();

  MDWaveDims dims;
  ::MemClear(dims, sizeof(MDWaveDims));
	dims[0] = (yat::int32)bb->size();

  if (XDK_UTILS->fetch_or_make_wave(&w, tmp_dfh, _argout.c_str(), TEXT_WAVE_TYPE, XDK_Utils::ANY, dims)) 
  {
    std::string d = "failed to make or change wave " + _argout;
    d += " (object name conflict)";
    XDK_UTILS->set_error("invalid argout specified",
                         d.c_str(),
                         "TangoBinding::black_box");
    return kError; 
  }

  Handle h_tmp = 0;
	for (size_t i = 0; i < bb->size(); i++)
	{
		dims[0] = i;
    do 
    {
      if (XDK_UTILS->str_to_handle((*bb)[i], h_tmp)) 
      {
        result = kError; break;
      }
      ::MDSetTextWavePointValue(w, dims, h_tmp);
      ::WMDisposeHandle(h_tmp);
    } while (0);
	}

  if (result == kError) 
  {
    XDK_UTILS->set_error("API_MemoryAllocation",
                         "str_to_handle failed",
                         "TangoBinding::black_box");
  }

  return result;
}
