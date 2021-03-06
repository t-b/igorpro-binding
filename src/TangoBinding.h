// ============================================================================
//
// = CONTEXT
//   Tango Generic Client for Igor Pro
//
// = FILENAME
//   TangoBinding.h
//
// = AUTHOR
//   Nicolas Leclercq
//
// ============================================================================

#ifndef _TANGO_BINDING_H_
#define _TANGO_BINDING_H_

//=============================================================================
// #DEFINEs
//=============================================================================
#define IDEV_PROXY (TangoBinding::instance())

//=============================================================================
// FORWARD DEC
//=============================================================================
class DevDescriptor;
class MonitoredAttribute;

//=============================================================================
// CLASS : TangoBinding (SINGLETON)
//=============================================================================
class TangoBinding 
{
  friend class TangoClientXop;

public:
  static TangoBinding* instance ();
  //- Returns the unique instance of <TangoBinding>.
    
  int open_device (const std::string& dev);
  // Just for test purpose - should be removed in a near future.

  int close_device (const std::string& dev);
  // Just for test purpose - should be removed in a near future.

  int command_in_out (const std::string& dev, 
                      const std::string& cmd, 
                      const std::string& arg_in, 
                      const std::string& arg_out);
  //- DevProxy::command_inout binding.

  int read_attribute (const std::string& dev, 
                      const std::string& attr, 
                      const std::string& arg_out);
  //- DevProxy::read_attribute binding.

  int read_attributes (const std::string& dev, 
                       const std::string& arg_in);
  //- DevProxy::read_attributes binding.

  int write_attribute (const std::string& dev, 
                       const std::string& attr, 
                       const std::string& arg_in);
  //- DevProxy::write_attribute binding.

  int write_attributes (const std::string& dev, 
                        const std::string& arg_in);
  //- DevProxy::write_attributes binding.

  int get_info (const std::string& dev, std::string& info);
  //- DevProxy::get_info binding.

  int status (const std::string& dev, std::string& status);
  //- DevProxy::status binding.

  int ping (const std::string& dev);
  //- DevProxy::ping binding.

  int set_timeout (const std::string& dev, int timeout);
  //- DevProxy::set_timeout binding.

  int get_timeout (const std::string& dev);
  //- DevProxy::get_timeout binding.

  int black_box (const std::string& _dev, 
                 const std::string& argout,
                 yat::int32 _n);
  //- DevProxy::black_box binding.

  virtual int new_experiment ();
  //- NEW msg handler

  virtual int load_experiment ();
  //- LOAD msg handler

private:
  static int init ();
  //- Instanciates the singleton.
  
  static void cleanup ();
  //- Releases the singleton.

  int read_attributes_i (DevDescriptor* ddesc, 
                         waveHndl tw);
  // read_attributes internal implementation

  int read_attributes_i (DevDescriptor* ddesc, 
                         const std::string& input);
  // read_attributes internal implementation

  int write_attributes_i (DevDescriptor* ddesc, 
                          waveHndl tw);
  // write_attributes internal implementation

  int write_attributes_i (DevDescriptor* ddesc, 
                          const std::string& input);
  // write_attributes internal implementation

  static TangoBinding* instance_;
  //- The unique instance of <TangoBinding>.

  // = Disallow these operations (except for TangoClientXop).
  //---------------------------------------------------------
  TangoBinding();
  TangoBinding (const TangoBinding&);
  virtual ~TangoBinding();
  void operator= (const TangoBinding&); 
};

#if defined (__XDK_INLINE__)
  #include "TangoBinding.i"
#endif 

#endif // _TANGO_BINDING_H_

   
   