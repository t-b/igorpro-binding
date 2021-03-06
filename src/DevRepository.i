// ============================================================================
//
// = CONTEXT
//   Tango Generic Client for Igor Pro
//
// = FILENAME
//   DevDescriptor.i
//
// = AUTHOR
//   Nicolas Leclercq
//
// ============================================================================

//=============================================================================
// DevDescriptor::name
//=============================================================================
XDK_INLINE const std::string& DevDescriptor::name () const
{
  return this->dev_name_;
}

//=============================================================================
// DevDescriptor::proxy
//=============================================================================
XDK_INLINE yat4tango::ThreadSafeDeviceProxy* DevDescriptor::proxy () const
{
  return this->dev_proxy_;
}

//=============================================================================
// DevDescriptor::cmd_list
//=============================================================================
XDK_INLINE const Tango::CommandInfoList& DevDescriptor::cmd_list () const
{
  return *this->dev_cmd_list_;
}

//=============================================================================
// DevDescriptor::attr_list
//=============================================================================
XDK_INLINE const Tango::AttributeInfoList& DevDescriptor::attr_list () const
{
  return *this->dev_attr_list_;
}

//=============================================================================
// DevDescriptor::data_folder
//=============================================================================
XDK_INLINE  DataFolderHandle DevDescriptor::data_folder () const
{
  return this->dev_df_;
}

//=============================================================================
// DevDescriptor::cmd_exists 
//=============================================================================
XDK_INLINE int DevDescriptor::cmd_exists (const char* _cmd_name)
{
  return this->cmd_exists(std::string(_cmd_name));
}

//=============================================================================
// DevDescriptor::attr_exists 
//=============================================================================
XDK_INLINE int DevDescriptor::attr_exists (const char* _attr_name)
{
  return this->attr_exists(std::string(_attr_name));
}

//=============================================================================
// DevRepository::instance
//=============================================================================
XDK_INLINE DevRepository* DevRepository::instance ()
{
  return DevRepository::instance_;
}

//=============================================================================
// DevRepository::device_desc
//=============================================================================
XDK_INLINE DevDescriptor* DevRepository::device_desc (const char* _device_name, bool _connect)
{
  return this->device_desc(std::string(_device_name), _connect);
}

