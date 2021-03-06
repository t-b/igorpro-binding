// ============================================================================
//
// = CONTEXT
//   Tango Generic Client for Igor Pro
//
// = FILENAME
//   MonitoredObject.i
//
// = AUTHOR
//   Nicolas Leclercq
//
// ============================================================================

// ============================================================================
// MonitoredAttribute::num_ms_to_polling_period_expiration
// ============================================================================
XDK_INLINE yat::uint32 MonitoredObject::num_ms_to_polling_period_expiration ()
{
  //- get current time
  yat::Timestamp now;
  _GET_TIME(now);

  //- compute diff between min_pp and elapsed msec since last import
  long dt = this->min_pp_ - static_cast<long>(this->import_timer_.elapsed_msec());

  return (dt <= 0) ? 0 : static_cast<unsigned long>(dt);
}

//=============================================================================
// MonitoredDevice::has_monitored_objects
//=============================================================================
XDK_INLINE bool MonitoredDevice::has_monitored_objects () const
{
  return ! this->mattr_map_.empty();
}


