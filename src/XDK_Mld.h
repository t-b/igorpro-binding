// ============================================================================
//
// = CONTEXT
//   XDK
//
// = FILENAME
//   XDK_Mld.h
//
// = AUTHOR
//   Nicolas Leclercq
//
// ============================================================================
#ifndef _XDK_MLD_H_
#define _XDK_MLD_H_

//=============================================================================
// Class XDK_Mld       (WARNING! THIS CLASS IS NOT MULTITHREADS SAFE)
//=============================================================================
class XDK_Mld
{
public : 
  XDK_Mld ();
  //- Ctor
  virtual ~XDK_Mld();
  //- Dtor
  static yat::int32 value ();
  //- Returns the MLD counter value. 

private:
  static yat::int32 value_;
  //- The MLD counter value : should be 0 when XOP quits. 
  //- Reads & writes should be protected again race conditions (to do). 

  // = Disallow these operations (except for friends).
  //---------------------------------------------------------
  XDK_Mld (const XDK_Mld&);
  void operator= (const XDK_Mld&); 
};

//=============================================================================
// MACROS
//=============================================================================
// Just drop 'MLD' anywhere in your class definition 
// to get cheap memory leak detection for your class.
#define MLD Mld mld_

// Use 'MLD_COUNTER' to see if things are OK.
// MLD_COUNTER should return 0 when XOP quits. 
#define MLD_COUNTER Mld::value()

#if defined (__XDK_INLINE__)
  #include "XDK_Mld.i"
#endif 

#endif // _XDK_MLD_H_