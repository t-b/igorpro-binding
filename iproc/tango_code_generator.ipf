#pragma TextEncoding = "UTF-8"
#pragma rtGlobals = 3
#pragma version = 1.0
#pragma IgorVersion = 6.0
#pragma hide = 0

//==============================================================================
// InterfaceGenerator.ipf
//==============================================================================
// N.Leclercq - SOLEIL
//==============================================================================

//==============================================================================
// DEPENDENCIES
//==============================================================================
#include "tango"
 
//==============================================================================
// naming scheme
//==============================================================================
constant kNS_OLD = 0
constant kNS_NEW = 1

//==============================================================================
// tango_gen_device_interface
//==============================================================================
function tango_gen_device_interface (cintf, [dev_name])
	Variable cintf
	String dev_name
	String file_path = tango_get_global_obj("gen_file", kSVAR)
	SVAR file = $file_path
	if (! strlen(file))
		file = "<enter the destination file name here>"
	endif
	String prefix_path = tango_get_global_obj("gen_prefix", kSVAR)
	SVAR prefix = $prefix_path
	if (! strlen(prefix))
		prefix = ""
	endif
	String device_path = tango_get_global_obj("gen_device", kSVAR)
	SVAR device = $device_path
	if (! ParamIsDefault(dev_name))
		device = dev_name
	endif
	String local_file = file
	String local_prefix = prefix
	String local_device = device
	Variable naming_scheme = kNS_NEW
	Prompt local_file, "File name [the name of the <ipf> file]"
	Prompt local_prefix, "Interface Prefix [all generated function name will begin with <prefix>]"
	if (cintf)
		Prompt  local_device, "Device Name [a device belonging to the family which interface will be generated]"
	else
		Prompt  local_device, "Device Name [the name of the device which interface will be generated]"
	endif
	Prompt naming_scheme, "Use new function naming scheme?  [say 'no' (i.e. zero) for backward compatibility]" 
	DoPrompt "Tango Device Interface Generator", local_file, local_prefix, local_device, naming_scheme
	file = local_file
	prefix = local_prefix
	device = local_device
	if (V_flag == 1)
		return kNO_ERROR
	endif
	if (strlen(WinList(file + ".ipf", ";", "WIN:128")))
		String err_str = "A procedure file named <" + file + ".ipf> is already open.\n"
		err_str  += "Close the existing file then retry." 
		tango_display_error_str (err_str)
		return kERROR
	endif
	tango_gen_dev_int(local_file, local_prefix, local_device, naming_scheme, cintf)
	return kNO_ERROR
end

//==============================================================================
// generated_func_name_exist
//==============================================================================
static function generated_func_name_exist (funcname)
   String funcname
	Wave/T funcnames = root:tango:funcnames
	NVAR funcnames_idx = root:tango:funcnames_idx
	Variable i = 0
	Variable n = numpnts(funcnames)
	for (; i < funcnames_idx; i+=1)
	  if (cmpstr(funcname, funcnames[i], 0) == 0)
	    return 1
	  endif
	endfor 
	return 0
end

//==============================================================================
// push_generated_func_name
//==============================================================================
static function push_generated_func_name (funcname)
   String funcname
	Wave/T funcnames = root:tango:funcnames
	NVAR funcnames_idx = root:tango:funcnames_idx
	funcnames[funcnames_idx] = funcname
	funcnames_idx += 1
end

//==============================================================================
// snake_to_camel_case (e.g. double_scalar_ro --> DoubleScalarRo)
//==============================================================================
static function/S snake_to_camel_case (sc)
   String sc
   String cc = ""
   sc2cc(sc, cc, 0)
   return cc
end

//==============================================================================
// sc2cc_next_alphanum_pos (e.g. double_scalar_ro --> DoubleScalarRo)
//==============================================================================
static function sc2cc_next_alphanum_pos (sc, i)
   String sc
   Variable i
	Variable l = strlen(sc)
	for (; i < l; i += 1)
	  if (cmpstr(sc[i], "_") && cmpstr(sc[i], "-") && cmpstr(sc[i], ".")) 
	     break
	  endif
	endfor
	return i
end

//==============================================================================
// sc2cc_next_sep_pos (e.g. double_scalar_ro --> DoubleScalarRo)
//==============================================================================
static function sc2cc_next_sep_pos (sc, i)
   String sc
   Variable i
	Variable l = strlen(sc)
	for (; i < l; i += 1)
	  if (!cmpstr(sc[i], "_") || !cmpstr(sc[i], "-") || !cmpstr(sc[i], ".")) 
	     break
	  endif
	endfor
	return i
end

//==============================================================================
// sc2cc_next_word_pos (e.g. double_scalar_ro --> DoubleScalarRo)
//==============================================================================
static function sc2cc_next_word_pos (sc, i, from, to)
   String sc
   Variable i
   Variable& from
   Variable& to
	from = sc2cc_next_alphanum_pos(sc, i)
	to = sc2cc_next_sep_pos(sc, from + 1) - 1
end

//==============================================================================
// sc2cc (e.g. double_scalar_ro --> DoubleScalarRo)
//==============================================================================
static function sc2cc (sc, cc, i)
   String &sc
	String &cc
   Variable i
	Variable l = strlen(sc)
	if (i >= l)
	   return 0
	endif
	Variable from, to
	sc2cc_next_word_pos(sc, i, from, to)
	cc += UpperStr(sc)[from] + sc[from + 1, to]
	sc2cc(sc, cc, to + 1)
end

//==============================================================================
// tango_gen_device_interface
//==============================================================================
static function tango_gen_dev_int (file, prefix, dev_name, naming_scheme, cintf)
	String file
	String prefix
	String dev_name
	Variable naming_scheme
	Variable cintf
	//- open destination 
	Variable ref_num
	String ipf_path
	if (tango_gen_open_proc_file(file, prefix, ref_num, ipf_path) == kERROR)
		return kERROR
	endif
	//- be sure device is running
	if (tango_ping_device(dev_name) == kERROR)
		String err_str
		err_str = "Could not generate interface for device <" + dev_name + ">\r" 
		err_str += "Check device name and be sure that it is running."
		tango_display_error_str (err_str)
		tango_gen_close_proc_file(ref_num)
		return kERROR
	endif
	//- get attribute list
	String cur_df
	tango_enter_attrs_df (dev_name, prev_df=cur_df)
	//- get a reference to the attribute list
	Wave/T/Z attr_list = alist
	if (WaveExists(attr_list) == 0)
		err_str   = "Could not generate interface for device <" + dev_name + ">\r" 
		err_str += "An error occured while trying to access the device attribute list."
		tango_display_error_str (err_str)
		tango_gen_close_proc_file(ref_num)
		return kERROR
	endif
	//- get number of attributes
	Variable num_attrs = dimsize(attr_list, 0)
	//- get command list
	tango_enter_cmds_df (dev_name)
	//- get a reference to the command list
	Wave/T/Z  cmd_list = clist
	if (WaveExists(cmd_list) == 0)
		err_str   = "Could not generate interface for device <" + dev_name + ">\r" 
		err_str += "An error occured while trying to access the device command list."
		tango_display_error_str (err_str)
		tango_gen_close_proc_file(ref_num)
		return kERROR
	endif
	//- get number of commands
	Variable num_cmds = dimsize(cmd_list, 0)
	//- start code generation
	fprintf ref_num,  "%s", "#pragma rtGlobals = 3\n"
	fprintf ref_num,  "%s", "#pragma version = 1.0\n"
	fprintf ref_num,  "%s", "#pragma IgorVersion = 6.0\n"
	fprintf ref_num,  "%s", "\n"
	fprintf ref_num,  "%s", "//==============================================================================\n"
	fprintf ref_num,  "%s", "// THIS FILE HAS BEEN GENERATED BY THE TANGO BINDING FOR IGOR PRO - DO NOT EDIT \n"
	fprintf ref_num,  "%s", "//==============================================================================\n"
	fprintf ref_num,  "%s", "//  In case you find a bug in the generated code or want to contribute to the   \n"
	fprintf ref_num,  "%s", "// improvement of the Tango API code generator for Igor Pro, please contact the \n"
	fprintf ref_num,  "%s", "//         author: nicolas[DOT]leclercq[AT]synchrotron-soleil[DOT]fr            \n"
	fprintf ref_num,  "%s", "//------------------------------------------------------------------------------\n"
	fprintf ref_num,  "// File...........%s.ipf\n", file
	fprintf ref_num,  "// Generated on...%s\n", Date() 
	fprintf ref_num,  "// Tango class....%s\n", tango_get_device_class (dev_name)
	fprintf ref_num,  "// Tango device...%s", dev_name 
	if (cintf)
		fprintf ref_num, "%s", " [used to gen. this tango class API]\n"
	else
		fprintf ref_num, "%s", " [used to gen. this specific device API]\n"
	endif
	fprintf ref_num,  "%s", "//==============================================================================\n"
	fprintf ref_num,  "%s", "\n"
	fprintf ref_num,  "%s", "//==============================================================================\n"
	fprintf ref_num,  "%s", "// DEPENDENCIES\n"
	fprintf ref_num,  "%s", "//==============================================================================\n"
	fprintf ref_num,  "%s", "#include \"tango\"\n"
	fprintf ref_num,  "%s", "\n"
	tango_enter_attrs_df (dev_name)
	//- tmp stuffs
	Make/N=512/T root:tango:funcnames
	Variable/G root:tango:funcnames_idx = 0
	//- for each attribute, generate its interface...
	Variable i
	String attr_name, cmd_name
	Variable access, format, type
	for (i  = 0; i < num_attrs; i+= 1)
		//- alist[i][0] contains the attribute name
		attr_name = attr_list[i][0]
		//- enter attribute datafolder
		tango_enter_attr_df(dev_name, attr_name)
		//- get attributre access
		access = tango_get_attr_access(dev_name, attr_name)
		//- generate set/get code for the i-th attribute
		switch (access)
			case kREAD:
				tango_gen_get_attribute(ref_num, dev_name, attr_name, naming_scheme, cintf, prefix)
				break
			case kWRITE:
			case kREAD_WRITE:
			case kREAD_WITH_WRITE: 
				tango_gen_get_attribute(ref_num, dev_name, attr_name, naming_scheme, cintf, prefix)
				tango_gen_set_attribute(ref_num, dev_name, attr_name, naming_scheme, cintf, prefix)
				break
		endswitch
	endfor
	//- continue with commands...
	tango_enter_cmds_df(dev_name)
	//- for each command, generate its interface...
	for (i  = 0; i < num_cmds; i+= 1)
		//- clist[i][0] contains the command name
		cmd_name = cmd_list[i][0]
		//- enter attribute datafolder
		tango_enter_cmd_df(dev_name, cmd_name)
		//- generate exec code for the i-th comand
		tango_gen_exec_command(ref_num, dev_name, cmd_name, naming_scheme, cintf, prefix)
	endfor
	//- close the destination file
	tango_gen_close_proc_file(ref_num)
	tango_leave_df(cur_df)
	//- open the generated proc file
	String cmd = "NewPath/O/Q ipf_tmp, \"" + ParseFilePath(1, ipf_path, ":", 1, 0) + "\"\r"
	Execute/Q(cmd)
	cmd = "OpenProc /P=ipf_tmp \"" +  ParseFilePath(0, ipf_path, ":", 1, 0) + "\"\r"
	Execute/Q(cmd)
   //- cleanup tmp stuffs
	KillWaves/Z root:tango:funcnames
	KillVariables/Z root:tango:funcnames_idx
	return kNO_ERROR
end

//==============================================================================
// tango_gen_set_attribute
//==============================================================================
static function tango_gen_set_attribute (ref_num, dev_name, attr_name, naming_scheme, cintf, pfx)
	Variable ref_num
	String dev_name
	String attr_name
	Variable naming_scheme
	Variable cintf
	String pfx
	String eol = "\n"
	String func_txt = tango_set_attr_func_txt(dev_name, attr_name, naming_scheme, cintf = cintf, pfx = pfx, eol = "\n")
	String token = ""
	Variable i = 0
	do 
		token = StringFromList(i, func_txt, eol)
		if (strlen(token) == 0)
			break
		endif
		fprintf ref_num,  "%s", token + eol
		i += 1
	while(1)
	fprintf ref_num,  "%s", eol
end

//==============================================================================
// tango_gen_get_attribute
//==============================================================================
static function tango_gen_get_attribute (ref_num, dev_name, attr_name, naming_scheme, cintf, pfx)
	Variable ref_num
	String dev_name
	String attr_name
	Variable naming_scheme
	Variable cintf
	String pfx
	String eol = "\n"
	String func_txt = tango_get_attr_func_txt(dev_name, attr_name, naming_scheme, cintf = cintf, pfx = pfx, eol = "\n")
	String token = ""
	Variable i = 0
	do 
		token = StringFromList(i, func_txt, eol)
		if (strlen(token) == 0)
			break
		endif
		fprintf ref_num, "%s", token + eol
		i += 1
	while(1)
	fprintf ref_num,  "%s", eol
end

//==============================================================================
// tango_gen_exec_command
//==============================================================================
static function tango_gen_exec_command (ref_num, dev_name, cmd_name, naming_scheme, cintf, pfx)
	Variable ref_num
	String dev_name
	String cmd_name
	Variable naming_scheme
	Variable cintf
	String pfx
	String eol = "\n"
	String func_txt = tango_cmd_func_txt(dev_name, cmd_name, naming_scheme, cintf = cintf, pfx = pfx, eol = "\n")
	String token = ""
	Variable i = 0
	do 
		token = StringFromList(i, func_txt, eol)
		if (strlen(token) == 0)
			break
		endif
		fprintf ref_num,  "%s", token + eol
		i += 1
	while(1)
	fprintf ref_num,  "%s", eol
	return kNO_ERROR
end

//==============================================================================
// tango_gen_open_procedure_file
//==============================================================================
static function tango_gen_open_proc_file (file, pfx, ref_num, full_path)
	String file
	String pfx
	Variable & ref_num
	String &  full_path
	ref_num = 0
	full_path = ""
	Variable rf
	String msg = "Specify destination file location. Existing file will be overwritten."
	//- open file
	Open/Z=2/M=msg/T=".ipf" rf as file
	//- store results from open in a safe place
	Variable err = V_flag
	if (err == kERROR)
		return kERROR
	endif
	if (err != 0)
		String err_str   = "Could not create procedure file.\n"
		err_str  += "Be sure <" + file +"> is not already open then retry.\n"
		err_str  += "Aborting code generation..." 
		tango_display_error_str (err_str)
		return kERROR
	endif
	ref_num = rf
	full_path = S_fileName
	return kNO_ERROR
end

//==============================================================================
// tango_gen_close_procedure_file
//==============================================================================
static function tango_gen_close_proc_file (ref_num)
	Variable ref_num
	Close ref_num
end

//==============================================================================
// tango_gen_cmd_func_to_scrap
//==============================================================================
function tango_gen_cmd_func_to_scrap (dev_name, cmd_name)
	String dev_name
	String cmd_name
	String func_txt = tango_cmd_func_txt(dev_name, cmd_name, kNS_NEW, eol = "\r")
	PutScrapText func_txt
end

//==============================================================================
// tango_gen_get_attr_func_to_scrap
//==============================================================================
function tango_get_attr_func_to_scrap (dev_name, attr_name)
	String dev_name
	String attr_name
	String func_txt = tango_get_attr_func_txt(dev_name, attr_name, kNS_NEW, eol ="\r")
	PutScrapText func_txt
end

//==============================================================================
// tango_gen_set_attr_func_to_scrap
//==============================================================================
function tango_set_attr_func_to_scrap (dev_name, attr_name)
	String dev_name
	String attr_name
	String func_txt = tango_set_attr_func_txt(dev_name, attr_name, kNS_NEW, eol ="\r")
	PutScrapText func_txt
end

//==============================================================================
// tango_get_attr_func_txt
//==============================================================================
static function/S tango_get_attr_func_txt (dev_name, attr_name, naming_scheme, [cintf, pfx, eol])
	String dev_name
	String attr_name
	Variable naming_scheme
	Variable cintf
	String pfx
	String eol
	if (ParamIsDefault(cintf))
		cintf = 0
	endif
	if (ParamIsDefault(pfx))
		pfx = ""
	endif
	if (ParamIsDefault(eol))
		eol = "\n"
	endif
	Variable format = tango_get_attr_format(dev_name, attr_name)
	Variable type = tango_get_attr_type(dev_name, attr_name)
	String tmp_attr_name = UpperStr(attr_name)[0] + attr_name[1, strlen(attr_name) - 1]
	String get_pfx
	if (naming_scheme == kNS_OLD)
	   get_pfx = "Get"
	else
	   get_pfx = "GetAttr"
	endif
	String func_name = pfx + get_pfx + tmp_attr_name
	if (naming_scheme == kNS_OLD)   
	   Variable fn_exist = generated_func_name_exist(func_name)
	   if (fn_exist)
		   func_name = pfx + "GetAttr" + tmp_attr_name
		   print("WARNING: read function for attribute " + dev_name + "/" + attr_name + " has been renamed to " + func_name)
	   endif
	   push_generated_func_name(func_name)
	endif
	if (igorversion() <= 7 && strlen(func_name) > 31)
		print "WARNING: function name <" + func_name + "> is too long - truncated to 31 characters"
		func_name = func_name[0,30]
	endif
	func_name = snake_to_camel_case(func_name)
	String func = ""
	String tmp =""
	func += "//==============================================================================" + eol
	sprintf tmp, "// %s" + eol, func_name
	func += tmp
	func += "//==============================================================================" + eol
	sprintf tmp, "//\tFunction......read then return the <%s> attribute value" + eol, attr_name
	func += tmp
	func += "//\tFeatures......may optionally return both attribute timestamp and quality" + eol
	sprintf tmp, "//\tDev.class.....%s" + eol, tango_get_device_class (dev_name)
	func += tmp
	sprintf tmp, "//\tAttr.name.....%s" + eol, attr_name
	func += tmp
	String desc = tango_get_attr_desc(dev_name, attr_name)
	Variable pos = strsearch(desc, "" + eol, 0)
	if (pos != -1)
		desc = desc[0, pos - 1]
	endif
	if (strlen(desc) > 57)
		desc = desc[0, 57] + "..."
	endif
	sprintf tmp, "//\tAttr.desc.....%s" + eol, LowerStr(desc)
	func += tmp
	sprintf tmp, "//\tAttr.Access...%s" + eol, tango_get_attr_access_str(tango_get_attr_access(dev_name, attr_name))
	func += tmp
	sprintf tmp, "//\tAttr.Format...%s" + eol, tango_get_attr_format_str(format)
	func += tmp
	sprintf tmp, "//\tAttr.Type.....%s" + eol, tango_get_attr_type_str(format, type)
	func += tmp
	sprintf tmp, "//\tExample.......the following code shows how to use this function" + eol
	func += tmp
	func += "//------------------------------------------------------------------------------" + eol 
	func += "//\tfunction myFunction ()" + eol
	if (cintf)
		func += "//\t\t//- the name of the target device" + eol
		func += "//\t\tString dev_name = \"my/tango/device\"" + eol
	endif
	switch (format)
		case kSCALAR:
			switch (type)
				case kSTRING:
					func += "//\t\t//- use a local 'string' to store the attribute value" + eol
					func += "//\t\t//- note that we cannot use a global string (SVAR) here" + eol
					func += "//\t\tString attr_value" + eol
					break 
				default:
					func += "//\t\t//- use a local 'variable' to store the attribute value" + eol
					func += "//\t\t//- note that we cannot use a global variable (NVAR) here" + eol
					func += "//\t\tVariable attr_value" + eol
					break 
			endswitch
			func += "//\t\t//- read the attribute and check error" + eol
			if (cintf)
				sprintf tmp, "//\t\tif (%s(dev_name, attr_value) == kERROR)" + eol, func_name
			else
				sprintf tmp, "//\t\tif (%s(attr_value) == kERROR)" + eol, func_name
			endif
			func += tmp
			func += "//\t\t\t//- an error occurred" + eol
			func += "//\t\t\treturn kERROR" + eol
			func += "//\t\tendif" + eol
			func += "//\t\t//- the value was successfully read " + eol
			func += "//\t\t//- <attr_value> now contains the attribute value" + eol
			break
		case kSPECTRUM:
			func += "//\t\t//- tell the tango binding where to put the 1D 'destination' wave" + eol
			func += "//\t\t//- here, we want the attr. value (i.e. the associated wave) to be" + eol
			func += "//\t\t//- placed into <root:my_df:> and named <my_spectrum>" + eol
			func += "//\t\tString dest_wave_path = \"root:my_df:my_spectrum\"" + eol
			func += "//\t\t//- read the spectrum attribute and check error" + eol
			if (cintf)
				sprintf tmp, "//\t\tif (%s(dev_name, dest_wave_path) == kERROR)" + eol, func_name
			else
				sprintf tmp, "//\t\tif (%s(dest_wave_path) == kERROR)" + eol, func_name
			endif
			func += tmp
			func += "//\t\t\t//- an error occurred" + eol
			func += "//\t\t\treturn kERROR" + eol
			func += "//\t\tendif" + eol
			func += "//\t\t//- the spectrum attribute was successfully read " + eol
			if (type == kSTRING) 
				sprintf tmp, "//\t\tWAVE/T %s_wave = $dest_wave_path" + eol, LowerStr(attr_name)
			else
				sprintf tmp, "//\t\tWAVE %s_wave = $dest_wave_path" + eol, LowerStr(attr_name)
			endif
			func += tmp
			break
		case kIMAGE:
			func += "//\t\t//- tell the tango binding where to place the 2D 'destination' wave" + eol
			func += "//\t\t//- here, we want the attr. value (i.e. the associated wave) to be" + eol
			func += "//\t\t//- placed into <root:my_df:> and named <my_image>" + eol
			func += "//\t\tString dest_wave_path = \"root:my_df:my_image\"" + eol
			func += "//\t\t//- read the image attribute and check error" + eol
			if (cintf)
				sprintf tmp, "//\t\tif (%s(dev_name, dest_wave_path) == kERROR)" + eol, func_name
			else
				sprintf tmp, "//\t\tif (%s(dest_wave_path) == kERROR)" + eol, func_name
			endif
			func += tmp
			func += "//\t\t\t//- an error occurred" + eol
			func += "//\t\t\treturn kERROR" + eol
			func += "//\t\tendif" + eol
			func += "//\t\t//- the image attribute was successfully read " + eol
			if (type == kSTRING) 
				sprintf tmp, "//\t\tWAVE/T %s_wave = $dest_wave_path" + eol, LowerStr(attr_name)
			else
				sprintf tmp, "//\t\tWAVE %s_wave = $dest_wave_path" + eol, LowerStr(attr_name)
			endif
			func += tmp
			break
	endswitch
	func += "//\t\t//..." + eol
	func += "//\t\treturn kNO_ERROR" + eol
	func += "//\tend" + eol
	func += "//==============================================================================" + eol
	switch (format)
		case kSCALAR:
			if (cintf)
				sprintf tmp, "function %s (dev, value, [tms, qlt])" + eol, func_name
				func += tmp
				func += "\tString dev" + eol
			else
				sprintf tmp, "function %s (value, [tms, qlt])" + eol, func_name
				func += tmp
			endif
			switch (type)
				case kSTRING:
					func += "\tString& value" + eol
					break 
				default:
					func += "\tVariable& value" + eol
					break
			endswitch
			func += "\tVariable& tms" + eol
			func += "\tVariable& qlt" + eol
			break
		case kSPECTRUM:
		case kIMAGE:
			if (cintf)
				sprintf tmp, "function %s (dev, dest_path, [tms, qlt])" + eol, func_name
				func += tmp
				func += "\tString dev" + eol
			else
				sprintf tmp, "function %s (dest_path, [tms, qlt])" + eol, func_name
				func += tmp
			endif
			func += "\tString& dest_path" + eol
			func += "\tVariable& tms" + eol
			func += "\tVariable& qlt" + eol
			break
	endswitch
	func += "\tStruct AttributeValue av" + eol
	if (cintf)
		sprintf tmp, "\ttango_init_attr_val(av, dev=dev, attr=\"%s\"", attr_name
	else
		sprintf tmp, "\ttango_init_attr_val(av, dev=\"%s\", attr=\"%s\"", dev_name, attr_name
	endif
	func += tmp
	switch (format)
		case kSCALAR:
			func += ")" + eol
			break
		case kSPECTRUM:
		case kIMAGE:
			func += ", path=dest_path)" + eol
			break
	endswitch
	func += "\tif (tango_read_attr(av) == kERROR)" + eol
	func += "\t\ttango_print_error()" + eol
	switch (format)
		case kSCALAR:
			switch (type)
				case kSTRING:
					func += "\t\tvalue = \"\"" + eol
					break 
				default:
					func += "\t\tvalue = NAN" + eol
					break 
					break
			endswitch
			break
		case kSPECTRUM:
		case kIMAGE:
			func += "\t\tdest_path = \"\"" + eol
			break
	endswitch
	func += "\t\tif (! ParamIsDefault(tms))" + eol
	func += "\t\t\ttms = -1" + eol
	func += "\t\tendif" + eol
	func += "\t\tif (! ParamIsDefault(qlt))" + eol
	func += "\t\t\tqlt = kAttrQualityUNKNOWN" + eol
	func += "\t\tendif" + eol
	func += "\t\treturn kERROR" + eol
	func += "\tendif" + eol
	switch (format)
		case kSCALAR:
			switch (type)
				case kSTRING:
					func += "\tvalue = av.str_val" + eol
					break 
				default:
					func += "\tvalue = av.var_val" + eol
					break 
					break
			endswitch
			break
		case kSPECTRUM:
		case kIMAGE:
			func += "\tdest_path = av.val_path" + eol
			break
	endswitch
	func += "\tif (! ParamIsDefault(tms))" + eol
	func += "\t\ttms = av.ts" + eol
	func += "\tendif" + eol
	func += "\tif (! ParamIsDefault(qlt))" + eol
	func += "\t\tqlt = av.quality" + eol
	func += "\tendif" + eol
	func += "\treturn kNO_ERROR" + eol
	func += "end" + eol
	return func
end

//==============================================================================
// tango_set_attr_func_txt
//==============================================================================
static function/S tango_set_attr_func_txt(dev_name, attr_name, naming_scheme, [cintf, pfx, eol])
	String dev_name
	String attr_name
	Variable naming_scheme
	Variable cintf
	String pfx
	String eol
	if (ParamIsDefault(cintf))
		cintf = 0
	endif
	if (ParamIsDefault(pfx))
		pfx = ""
	endif
	if (ParamIsDefault(eol))
		eol = "\n"
	endif
	Variable format = tango_get_attr_format(dev_name, attr_name)
	Variable type = tango_get_attr_type(dev_name, attr_name)
	String tmp_attr_name = UpperStr(attr_name)[0] + attr_name[1, strlen(attr_name) - 1]
	String set_pfx = "SetAttr"
	if (naming_scheme == kNS_OLD)
	   set_pfx = "Set"
	endif
	String func_name = pfx + set_pfx + tmp_attr_name
	if (naming_scheme == kNS_OLD)   
	   Variable fn_exist = generated_func_name_exist(func_name)
	   if (fn_exist)
		   func_name = pfx + "SetAttr" + tmp_attr_name
		   print("WARNING: write function for attribute " + dev_name + "/" + attr_name + " has been renamed to " + func_name)
	   endif
	   push_generated_func_name(func_name)
	endif
	if (igorversion() <= 7 && strlen(func_name) > 31)
		print "WARNING: function name <" + func_name + "> is too long - truncated to 31 characters"
		func_name = func_name[0,30]
	endif
	func_name = snake_to_camel_case(func_name)
	String func = ""
	String tmp
	func += "//==============================================================================" + eol
	sprintf tmp, "// %s" + eol, func_name
	func += tmp
	func += "//==============================================================================" + eol
	sprintf tmp, "//\tFunction......write the specified value on the <%s> attribute" + eol, attr_name
	func += tmp
	sprintf tmp, "//\tDev.class.....%s" + eol, tango_get_device_class (dev_name)
	func += tmp
	sprintf tmp, "//\tAttr.name.....%s" + eol, attr_name
	func += tmp
	String desc = tango_get_attr_desc(dev_name, attr_name)
	Variable pos = strsearch(desc, "" + eol, 0)
	if (pos != -1)
		desc = desc[0, pos - 1]
	endif
	if (strlen(desc) > 57)
		desc = desc[0, 57] + "..."
	endif
	sprintf tmp, "//\tAttr.desc.....%s" + eol, LowerStr(desc)
	func += tmp
	sprintf tmp, "//\tAttr.Access...%s" + eol, tango_get_attr_access_str(tango_get_attr_access(dev_name, attr_name))
	func += tmp
	sprintf tmp, "//\tAttr.Format...%s" + eol, tango_get_attr_format_str(format)
	func += tmp
	sprintf tmp, "//\tAttr.Type.....%s" + eol, tango_get_attr_type_str(format, type)
	func += tmp
	func += "//\tExample.......the following code shows how to use this function" + eol
	func += "//------------------------------------------------------------------------------" + eol 
	func += "//\tfunction myFunction ()" + eol
	if (cintf)
		func += "//\t\t//- the name of the target device" + eol
		func += "//\t\tString dev_name = \"my/tango/device\"" + eol
	endif
	switch (format)
		case kSCALAR:
			func += "//\t\t//- specify the attribute value (i.e. the value to be applied)" + eol
			switch (type)
				case kSTRING:
					func += "//\t\t//- this value can be stored into a local or a global variable" + eol
					sprintf tmp, "//\t\tString attr_value = root:my_df:my_%s_value" + eol, LowerStr(attr_name)
					func += tmp
					func += "//\t\t//- change the value of our global string" + eol
					func += "//\t\tattr_value = \"my attribute text\"" + eol
					break 
				default:
					func += "//\t\t//- this value can be stored into a local or a global variable" + eol
					sprintf tmp, "//\t\tNVAR attr_value = root:my_df:my_%s_value" + eol, LowerStr(attr_name)
					func += tmp
					func += "//\t\t//- change the value of our global variable" + eol
					switch (type)
						case kBOOL:
							func += "//\t\tattr_value = 1" + eol
							break
						case kCHAR:
							func += "//\t\tattr_value = 256" + eol
							break
						case kUCHAR:
							func += "//\t\tattr_value = -127" + eol
							break
						case kSHORT:
							func += "//\t\tattr_value = 4096" + eol
							break
						case kUSHORT:
							func += "//\t\tattr_value = -4096" + eol
							break
						case kLONG:
							func += "//\t\tattr_value = -100000" + eol
							break
						case kULONG:
							func += "//\t\tattr_value = 100000" + eol
							break
						case kFLOAT:
						case kDOUBLE:
							func += "//\t\tattr_value = 123456.789" + eol
							break
					endswitch
					break 
			endswitch
			func += "//\t\t//- apply the value and check error" + eol
			if (cintf)
				sprintf tmp, "//\t\tif (%s(dev_name, attr_value) == kERROR)" + eol, func_name
			else
				sprintf tmp, "//\t\tif (%s(attr_value) == kERROR)" + eol, func_name
			endif
			func += tmp
			break
		case kSPECTRUM:
			func += "//\t\t//- make the source wave (must match the attribute data type)" + eol
			func += "//\t\tMake/O"
			switch (type)
				case kSTRING:
					func += "/T/N=128 my_spectrum = \"none\"" + eol
					break
				case kBOOL:
				case kCHAR:
					func += "/B/N=128 my_spectrum = enoise(127)" + eol
					break
				case kUCHAR:
					func += "/U/B/N=128 my_spectrum = abs(enoise(127))" + eol
					break
				case kSHORT:
					func += "/W/N=128 my_spectrum = enoise(4096)" + eol
					break
				case kUSHORT:
					func += "/U/W/N=128 my_spectrum = abs(enoise(4096))" + eol
					break
				case kLONG:
					func += "/I/N=128 my_spectrum = enoise(4096)" + eol
					break
				case kULONG:
					func += "/U/I/N=128 my_spectrum = abs(enoise(4096))" + eol
					break
				case kFLOAT:
					func += "/N=128 my_spectrum = enoise(1.0)" + eol
					break
				case kDOUBLE:
					func += "/D/N=128 my_spectrum = enoise(1.0)" + eol
					break
			endswitch
			func += "//\t\t//- get full path to the source wave (i.e. wave location)" + eol
			func += "//\t\tString src_wave_path = GetWavesDataFolder(my_spectrum, 2)" + eol
			func += "//\t\t//- apply the value and check error" + eol
			if (cintf)
				sprintf tmp, "//\t\tif (%s(dev_name, src_wave_path) == kERROR)" + eol, func_name
			else
				sprintf tmp, "//\t\tif (%s(src_wave_path) == kERROR)" + eol, func_name
			endif
			func += tmp
			break
		case kIMAGE:
			func += "//\t\t//- make the source wave (must match the attribute data type)" + eol
			func += "//\t\tMake/O"
			switch (type)
				case kSTRING:
					func += "/T/N=(128,128) my_image = \"none\"" + eol
					break
				case kBOOL:
				case kCHAR:
					func += "/B/N=(128,128) my_image = enoise(127)" + eol
					break
				case kUCHAR:
					func += "/U/B/N=(128,128) my_image = abs(enoise(127))" + eol
					break
				case kSHORT:
					func += "/W/N=(128,128) my_image = enoise(4096)" + eol
					break
				case kUSHORT:
					func += "/U/W/N=(128,128) my_image = abs(enoise(4096))" + eol
					break
				case kLONG:
					func += "/I/N=(128,128) my_image = enoise(4096)" + eol
					break
				case kULONG:
					func += "/U/I/N=(128,128) my_image = abs(enoise(4096))" + eol
					break
				case kFLOAT:
					func += "/N=(128,128) my_image = enoise(1.0)" + eol
					break
				case kDOUBLE:
					func += "/D/N=(128,128) my_image = enoise(1.0)" + eol
					break
			endswitch
			func += "//\t\t//- get full path to the source wave (i.e. wave location)" + eol
			func += "//\t\tString src_wave_path = GetWavesDataFolder(my_image, 2)" + eol
			func += "//\t\t//- apply the value and check error" + eol
			if (cintf)
				sprintf tmp, "//\t\tif (%s(dev_name, src_wave_path) == kERROR)" + eol, func_name
			else
				sprintf tmp, "//\t\tif (%s(src_wave_path) == kERROR)" + eol, func_name
			endif
			func += tmp
			break
	endswitch
	func += "//\t\t\t//- an error occurred" + eol
	func += "//\t\t\treturn kERROR" + eol
	func += "//\t\tendif" + eol
	func += "//\t\t//- the value was successfully applied" + eol
	func += "//\t\treturn kNO_ERROR" + eol
	func += "//\tend" + eol
	func += "//==============================================================================" + eol
	switch (format)
		case kSCALAR:
			if (cintf)
				sprintf tmp, "function %s (dev, value)" + eol, func_name
				func += tmp
				func += "\tString dev" + eol
			else
				sprintf tmp, "function %s (value)" + eol, func_name
				func += tmp
			endif
			switch (type)
				case kSTRING:
					func += "\tString value" + eol
					break 
				default:
					func += "\tVariable value" + eol
					break 
			endswitch
			break
		case kSPECTRUM:
		case kIMAGE:
			if (cintf)
				sprintf tmp, "function %s (dev, src_path)" + eol, func_name
				func += tmp
				func += "\tString dev" + eol
			else
				sprintf tmp, "function %s (src_path)" + eol, func_name
				func += tmp
			endif
			func += "\tString src_path" + eol
			break
	endswitch
	func += "\tStruct AttributeValue av" + eol
	if (cintf)
		sprintf tmp, "\ttango_init_attr_val(av, dev=dev, attr=\"%s\"", attr_name
	else
		sprintf tmp, "\ttango_init_attr_val(av, dev=\"%s\", attr=\"%s\"", dev_name, attr_name
	endif
	func += tmp
	switch (format)
		case kSCALAR:
			switch (type)
				case kSTRING:
					func += ", sval=value)" + eol
					break 
				default:
					func += ", nval=value)" + eol
					break 
			endswitch
			break
		case kSPECTRUM:
		case kIMAGE:
			func += ", path=src_path)" + eol
			break
	endswitch
	func += "\tif (tango_write_attr(av) == kERROR)" + eol
	func += "\t\ttango_print_error()" + eol
	func += "\t\treturn kERROR" + eol
	func += "\tendif" + eol
	func += "\treturn kNO_ERROR" + eol
	func += "end" + eol
	return func
end

//==============================================================================
// tango_cmd_func_txt
//==============================================================================
static function/S tango_cmd_func_txt(dev_name, cmd_name, naming_scheme, [cintf, pfx, eol])
	String dev_name
	String cmd_name
	Variable naming_scheme
	Variable cintf
	String pfx
	String eol
	if (ParamIsDefault(cintf))
		cintf = 0
	endif
	if (ParamIsDefault(pfx))
		pfx = ""
	endif
	if (ParamIsDefault(eol))
		eol = "\n"
	endif
	String tmp_cmd_name = UpperStr(cmd_name)[0] + cmd_name[1, strlen(cmd_name) - 1]
	String exec_pfx = "Exec"
	if (naming_scheme == kNS_OLD)
	   exec_pfx = ""
	endif
	String func_name = pfx + exec_pfx + cmd_name
	if (naming_scheme == kNS_OLD)   
	   Variable fn_exist = generated_func_name_exist(func_name)
	   if (fn_exist)
		   func_name = pfx + "Exec" + cmd_name
		   print("WARNING: execute function for command " + dev_name + "/" + cmd_name + " has been renamed to " + func_name)
	   endif
	   push_generated_func_name(func_name)
	endif
	if (igorversion() <= 7 && strlen(func_name) > 31)
		print "WARNING: function name <" + func_name + "> is too long - truncated to 31 characters"
		func_name = func_name[0,30]
	endif
	func_name = snake_to_camel_case(func_name)
	Variable argin_type = tango_get_cmd_argin_type (dev_name, cmd_name)
	Variable argout_type = tango_get_cmd_argout_type (dev_name, cmd_name)
	String func = ""
	String tmp
	func += "//==============================================================================" + eol
	sprintf tmp, "// %s" + eol, func_name
	func += tmp
	func += "//==============================================================================" + eol
	sprintf tmp, "//\tFunction.......execute the <%s> command" + eol, cmd_name
	func += tmp
	sprintf tmp, "//\tDev.class......%s" + eol, tango_get_device_class (dev_name)
	func += tmp
	sprintf tmp, "//\tCmd.name.......%s" + eol, cmd_name
	func += tmp
	sprintf tmp, "//\tArg-in type....%s" + eol, tango_get_cmd_argio_type_str(argin_type)
	func += tmp
	String argin_desc = tango_get_cmd_argin_desc(dev_name, cmd_name)
	Variable pos = strsearch(argin_desc, "" + eol, 0)
	if (pos != -1)
		argin_desc = argin_desc[0, pos - 1]
	endif
	if (strlen(argin_desc) > 57)
		argin_desc = argin_desc[0, 57] + "..."
	endif
	sprintf tmp, "//\tArg-in desc....%s" + eol, LowerStr(argin_desc)
	func += tmp
	sprintf tmp, "//\tArg-out type...%s" + eol, tango_get_cmd_argio_type_str(argout_type)
	func += tmp
	String argout_desc = tango_get_cmd_argout_desc(dev_name, cmd_name)
	pos = strsearch(argout_desc, "" + eol, 0)
	if (pos != -1)
		argout_desc = argout_desc[0, pos - 1]
	endif
	if (strlen(argout_desc) > 57)
		argout_desc = argout_desc[0, 57] + "..."
	endif
	sprintf tmp, "//\tArg-out desc...%s" + eol, LowerStr(argout_desc)
	func += tmp
	func += "//\tExample........the following code shows how to use this function" + eol
	func += "//------------------------------------------------------------------------------" + eol 
	func += "//\tfunction myFunction ()" + eol
	if (cintf)
		func += "//\t\tString dev_name = \"my/tango/device\"" + eol
	endif
	switch (argin_type)
		case kDEVVOID:
			break
		case kDEVSTATE:
		case kDEVBOOLEAN:
		case kDEVSHORT:
		case kDEVLONG:
		case kDEVFLOAT:
		case kDEVDOUBLE:
		case kDEVUSHORT:
		case kDEVULONG:
		case kDEVUCHAR:
			func += "//\t\t//- specify the argin value (i.e. the value to be passed to the cmd)" + eol
			func += "//\t\t//- it can a local or a global variable (global in this example)" + eol
			func += "//\t\tNVAR argin_val = root:my_df:my_val" + eol
			break
		case kDEVSTRING:
			func += "//\t\t//- specify the argin value (i.e. the value to be passed to the cmd)" + eol
			func += "//\t\t//- it can a local or a global string (global in this example)" + eol
			func += "//\t\tSVAR argin_str = root:my_df:my_str" + eol
			break
		case kDEVVARCHARARRAY:
		case kDEVVARSHORTARRAY:
		case kDEVVARLONGARRAY:
		case kDEVVARFLOATARRAY:
		case kDEVVARDOUBLEARRAY:
		case kDEVVARUSHORTARRAY:
		case kDEVVARULONGARRAY:
		case kDEVVARBOOLEANARRAY:
		case kDEVVARSTRINGARRAY:
			func += "//\t\t//- specify the full path to the argin wave (i.e. wave to be passed to the cmd)" + eol
			func += "//\t\tString argin_path = \"root:my_df:my_wave\"" + eol
			break
		case kDEVVARLONGSTRINGARRAY:
		case kDEVVARDOUBLESTRINGARRAY:
			func += "//\t\t//- specify the full path to the argin 1D numeric wave (i.e. wave to be passed to the cmd)" + eol
			func += "//\t\tString num_argin_path = \"root:my_df:my_num_wave\"" + eol
			func += "//\t\t//- specify the full path to the argin 1D text wave (i.e. wave to be passed to the cmd)" + eol
			func += "//\t\tString txt_argin_path = \"root:my_df:my_txt_wave\"" + eol
			break
	endswitch
	switch (argout_type)
		case kDEVVOID:
			break
		case kDEVSTATE:
		case kDEVBOOLEAN:
		case kDEVSHORT:
		case kDEVLONG:
		case kDEVFLOAT:
		case kDEVDOUBLE:
		case kDEVUSHORT:
		case kDEVULONG:
		case kDEVUCHAR:
			func += "//\t\t//- use a local 'variable' to store the command result" + eol
			func += "//\t\tVariable argout_val" + eol
			break
		case kDEVSTRING:
			func += "//\t\t//- use a local 'string' to store the command result" + eol
			func += "//\t\tString argout_str" + eol
			break
		case kDEVVARCHARARRAY:
		case kDEVVARSHORTARRAY:
		case kDEVVARLONGARRAY:
		case kDEVVARFLOATARRAY:
		case kDEVVARDOUBLEARRAY:
		case kDEVVARUSHORTARRAY:
		case kDEVVARULONGARRAY:
		case kDEVVARBOOLEANARRAY:
		case kDEVVARSTRINGARRAY:
			func += "//\t\t//- tell the tango binding where to place the command result" + eol
			func += "//\t\t//- here, we want the cmd. result (i.e. the associated wave) to be" + eol
			func += "//\t\t//- placed into <root:my_df:> and named <my_result>" + eol
			func += "//\t\tString argout_path = \"root:my_df:my_result\"" + eol
			break
		case kDEVVARLONGSTRINGARRAY:
		case kDEVVARDOUBLESTRINGARRAY:
			func += "//\t\t//- tell the tango binding where to place the numeric part of the command result" + eol
			func += "//\t\t//- here, we want the num. part of the cmd. result (i.e. the associated wave) to be" + eol
			func += "//\t\t//- placed into <root:my_df:> and named <my_num_part_result>" + eol
			func += "//\t\tString num_argout_path = \"root:my_df:my_num_part_result\"" + eol
			func += "//\t\t//- tell the tango binding where to place the text part of the command result" + eol
			func += "//\t\t//- here, we want the str. part of the cmd. result (i.e. the associated wave) to be" + eol
			func += "//\t\t//- placed into <root:my_df:> and named <my_str_part_result>" + eol
			func += "//\t\tString txt_argout_path = \"root:my_df:my_str_part_result\"" + eol
			break
	endswitch
	func += "//\t\t//- execute the command and check error" + eol
	if (cintf)
		sprintf tmp, "//\t\tif (%s(dev_name", func_name
		func += tmp
		if (argin_type != kDEVVOID || argout_type != kDEVVOID)
			func += ", "
		endif
	else
		sprintf tmp, "//\t\tif (%s(", func_name
		func += tmp
	endif
	switch (argin_type)
		case kDEVVOID:
			break
		case kDEVSTATE:
		case kDEVBOOLEAN:
		case kDEVSHORT:
		case kDEVLONG:
		case kDEVFLOAT:
		case kDEVDOUBLE:
		case kDEVUSHORT:
		case kDEVULONG:
		case kDEVUCHAR:
			func += "argin_val"
			break
		case kDEVSTRING:
			func += "argin_str"
			break
		case kDEVVARCHARARRAY:
		case kDEVVARSHORTARRAY:
		case kDEVVARLONGARRAY:
		case kDEVVARFLOATARRAY:
		case kDEVVARDOUBLEARRAY:
		case kDEVVARUSHORTARRAY:
		case kDEVVARULONGARRAY:
		case kDEVVARBOOLEANARRAY:
		case kDEVVARSTRINGARRAY:
			func += "argin_path"
			break
		case kDEVVARLONGSTRINGARRAY:
		case kDEVVARDOUBLESTRINGARRAY:
			func += "num_argin_path, txt_argin_path"
			break
	endswitch
	if (argin_type != kDEVVOID && argout_type != kDEVVOID)
		func += ", "
	endif
	switch (argout_type)
		case kDEVVOID:
			break
		case kDEVSTATE:
		case kDEVBOOLEAN:
		case kDEVSHORT:
		case kDEVLONG:
		case kDEVFLOAT:
		case kDEVDOUBLE:
		case kDEVUSHORT:
		case kDEVULONG:
		case kDEVUCHAR:
			func += "argout_val"
			break
		case kDEVSTRING:
			func += "argout_str"
			break
		case kDEVVARCHARARRAY:
		case kDEVVARSHORTARRAY:
		case kDEVVARLONGARRAY:
		case kDEVVARFLOATARRAY:
		case kDEVVARDOUBLEARRAY:
		case kDEVVARUSHORTARRAY:
		case kDEVVARULONGARRAY:
		case kDEVVARBOOLEANARRAY:
		case kDEVVARSTRINGARRAY:
			func += "argout_path"
			break
		case kDEVVARLONGSTRINGARRAY:
		case kDEVVARDOUBLESTRINGARRAY:
			func += "num_argout_path, txt_argout_path"
			break
	endswitch
	func += ") == kERROR)" + eol
	func += "//\t\t\t//- an error occurred" + eol
	func += "//\t\t\treturn kERROR" + eol
	func += "//\t\tendif" + eol
	func += "//\t\t//- the command was successfully executed" + eol
	switch (argout_type)
		case kDEVVOID:
			break
		case kDEVSTATE:
		case kDEVBOOLEAN:
		case kDEVSHORT:
		case kDEVLONG:
		case kDEVFLOAT:
		case kDEVDOUBLE:
		case kDEVUSHORT:
		case kDEVULONG:
		case kDEVUCHAR:
			func += "//\t\t//- <argout_val> now contains the command result" + eol
			break
		case kDEVSTRING:
			func += "//\t\t//- <argout_str> now contains the command result" + eol
			break
		case kDEVVARCHARARRAY:
		case kDEVVARSHORTARRAY:
		case kDEVVARLONGARRAY:
		case kDEVVARFLOATARRAY:
		case kDEVVARDOUBLEARRAY:
		case kDEVVARUSHORTARRAY:
		case kDEVVARULONGARRAY:
		case kDEVVARBOOLEANARRAY:
			func += "//\t\tWAVE cmd_result = $argout_path" + eol
			break
		case kDEVVARSTRINGARRAY:
			func += "//\t\tWAVE/T cmd_result = $argout_path" + eol
			break
		case kDEVVARLONGSTRINGARRAY:
		case kDEVVARDOUBLESTRINGARRAY:
			func += "//\t\tWAVE num_cmd_result = $num_argout_path" + eol
			func += "//\t\tWAVE/T txt_cmd_result = $txt_argout_path" + eol
			break
			break
	endswitch
	func += "//\t\t//..." + eol
	func += "//\t\treturn kNO_ERROR" + eol
	func += "//\tend" + eol
	func += "//==============================================================================" + eol
	if (cintf)
		sprintf tmp, "function %s (dev_name", func_name
	else
		sprintf tmp, "function %s (", func_name
	endif	
	func += tmp
	if (cintf && ((argin_type != kDEVVOID) || (argout_type != kDEVVOID)))
		func += ", "
	endif
	switch (argin_type)
		case kDEVVOID:
			break
		case kDEVSTATE:
		case kDEVBOOLEAN:
		case kDEVSHORT:
		case kDEVLONG:
		case kDEVFLOAT:
		case kDEVDOUBLE:
		case kDEVUSHORT:
		case kDEVULONG:
		case kDEVUCHAR:
			func += "in_val"
			break
		case kDEVSTRING:
			func += "in_str"
			break
		case kDEVVARCHARARRAY:
		case kDEVVARSHORTARRAY:
		case kDEVVARLONGARRAY:
		case kDEVVARFLOATARRAY:
		case kDEVVARDOUBLEARRAY:
		case kDEVVARUSHORTARRAY:
		case kDEVVARULONGARRAY:
		case kDEVVARBOOLEANARRAY:
		case kDEVVARSTRINGARRAY:
			func += "in_wave_path"
			break
		case kDEVVARLONGSTRINGARRAY:
		case kDEVVARDOUBLESTRINGARRAY:
			func += "in_num_wave_path, in_str_wave_path"
			break
	endswitch
	if ((argin_type != kDEVVOID) && (argout_type != kDEVVOID))
		func += ", "
	endif
	switch (argout_type)
		case kDEVVOID:
			break
		case kDEVSTATE:
		case kDEVBOOLEAN:
		case kDEVSHORT:
		case kDEVLONG:
		case kDEVFLOAT:
		case kDEVDOUBLE:
		case kDEVUSHORT:
		case kDEVULONG:
		case kDEVUCHAR:
			func += "out_val"
			break
		case kDEVSTRING:
			func += "out_str"
			break
		case kDEVVARCHARARRAY:
		case kDEVVARSHORTARRAY:
		case kDEVVARLONGARRAY:
		case kDEVVARFLOATARRAY:
		case kDEVVARDOUBLEARRAY:
		case kDEVVARUSHORTARRAY:
		case kDEVVARULONGARRAY:
		case kDEVVARBOOLEANARRAY:
		case kDEVVARSTRINGARRAY:
			func += "out_wave_path"
			break
		case kDEVVARLONGSTRINGARRAY:
		case kDEVVARDOUBLESTRINGARRAY:
			func += "out_num_wave_path, out_str_wave_path"
			break
	endswitch
	func += ")" + eol
	if (cintf)
		func += "\tString dev_name" + eol
	endif	
	switch (argin_type)
		case kDEVVOID:
			break
		case kDEVSTATE:
		case kDEVBOOLEAN:
		case kDEVSHORT:
		case kDEVLONG:
		case kDEVFLOAT:
		case kDEVDOUBLE:
		case kDEVUSHORT:
		case kDEVULONG:
		case kDEVUCHAR:
			func += "\tVariable in_val" + eol
			break
		case kDEVSTRING:
			func += "\tString in_str" + eol
			break
		case kDEVVARCHARARRAY:
		case kDEVVARSHORTARRAY:
		case kDEVVARLONGARRAY:
		case kDEVVARFLOATARRAY:
		case kDEVVARDOUBLEARRAY:
		case kDEVVARUSHORTARRAY:
		case kDEVVARULONGARRAY:
		case kDEVVARBOOLEANARRAY:
		case kDEVVARSTRINGARRAY:
			func += "\tString in_wave_path" + eol
			break
		case kDEVVARLONGSTRINGARRAY:
		case kDEVVARDOUBLESTRINGARRAY:
			func += "\tString in_num_wave_path" + eol
			func += "\tString in_str_wave_path" + eol
			break
	endswitch
	switch (argout_type)
		case kDEVVOID:
			break
		case kDEVSTATE:
		case kDEVBOOLEAN:
		case kDEVSHORT:
		case kDEVLONG:
		case kDEVFLOAT:
		case kDEVDOUBLE:
		case kDEVUSHORT:
		case kDEVULONG:
		case kDEVUCHAR:
			func += "\tVariable& out_val" + eol
			break
		case kDEVSTRING:
			func += "\tString& out_str" + eol
			break
		case kDEVVARCHARARRAY:
		case kDEVVARSHORTARRAY:
		case kDEVVARLONGARRAY:
		case kDEVVARFLOATARRAY:
		case kDEVVARDOUBLEARRAY:
		case kDEVVARUSHORTARRAY:
		case kDEVVARULONGARRAY:
		case kDEVVARBOOLEANARRAY:
		case kDEVVARSTRINGARRAY:
			func += "\tString& out_wave_path" + eol
			break
		case kDEVVARLONGSTRINGARRAY:
		case kDEVVARDOUBLESTRINGARRAY:
			func += "\tString& out_num_wave_path" + eol
			func += "\tString& out_str_wave_path" + eol
			break
	endswitch
	if (argin_type != kDEVVOID)
		func += "\tStruct CmdArgIO cai" + eol
		func += "\ttango_init_cmd_argio (cai)" + eol
	endif
	switch (argin_type)
		case kDEVVOID:
			break
		case kDEVSTATE:
		case kDEVBOOLEAN:
		case kDEVSHORT:
		case kDEVLONG:
		case kDEVFLOAT:
		case kDEVDOUBLE:
		case kDEVUSHORT:
		case kDEVULONG:
		case kDEVUCHAR:
			func += "\tcai.var_val = in_val" + eol
			break
		case kDEVSTRING:
			func += "\tcai.str_val = in_str" + eol
			break
		case kDEVVARCHARARRAY:
		case kDEVVARSHORTARRAY:
		case kDEVVARLONGARRAY:
		case kDEVVARFLOATARRAY:
		case kDEVVARDOUBLEARRAY:
		case kDEVVARUSHORTARRAY:
		case kDEVVARULONGARRAY:
		case kDEVVARBOOLEANARRAY:
			func += "\tcai.num_wave_path = in_wave_path" + eol
			break
		case kDEVVARSTRINGARRAY:
			func += "\tcai.str_wave_path = in_wave_path" + eol
			break
		case kDEVVARLONGSTRINGARRAY:
		case kDEVVARDOUBLESTRINGARRAY:
			func += "\tcai.num_wave_path = in_num_wave_path" + eol
			func += "\tcai.str_wave_path = in_str_wave_path" + eol
			break
	endswitch  
	if (argout_type != kDEVVOID)  
		func += "\tStruct CmdArgIO cao" + eol
		func += "\ttango_init_cmd_argio (cao)" + eol
	endif
	switch (argout_type)
		case kDEVVARCHARARRAY:
		case kDEVVARSHORTARRAY:
		case kDEVVARLONGARRAY:
		case kDEVVARFLOATARRAY:
		case kDEVVARDOUBLEARRAY:
		case kDEVVARUSHORTARRAY:
		case kDEVVARULONGARRAY:
		case kDEVVARBOOLEANARRAY:
			func += "\tcao.num_wave_path = out_wave_path" + eol
			break
		case kDEVVARSTRINGARRAY:
			func += "\tcao.str_wave_path = out_wave_path" + eol
			break
		case kDEVVARLONGSTRINGARRAY:
		case kDEVVARDOUBLESTRINGARRAY:
			func += "\tcao.num_wave_path = out_num_wave_path" + eol
			func += "\tcao.str_wave_path = out_str_wave_path" + eol
			break
	endswitch
	func += "\tif (tango_cmd_inout("
	if (cintf)
		func += "dev_name, "
	else
		sprintf tmp, "\"%s\", ", dev_name
		func += tmp
	endif	
	sprintf tmp, "\"%s\"", cmd_name
	func += tmp
	if (argin_type != kDEVVOID)  
		func += ", arg_in = cai"
	endif
	if (argout_type != kDEVVOID)  
		func += ", arg_out = cao"
	endif
	func += ") == kERROR)" + eol
	func += "\t\ttango_print_error()" + eol
	func += "\t\treturn kERROR" + eol
	func += "\tendif" + eol
	switch (argout_type)
		case kDEVVOID:
			break
		case kDEVSTATE:
		case kDEVBOOLEAN:
		case kDEVSHORT:
		case kDEVLONG:
		case kDEVFLOAT:
		case kDEVDOUBLE:
		case kDEVUSHORT:
		case kDEVULONG:
		case kDEVUCHAR:
			func += "\tout_val = cao.var_val" + eol
			break
		case kDEVSTRING:
			func += "\tout_str = cao.str_val" + eol
			break
		case kDEVVARCHARARRAY:
		case kDEVVARSHORTARRAY:
		case kDEVVARLONGARRAY:
		case kDEVVARFLOATARRAY:
		case kDEVVARDOUBLEARRAY:
		case kDEVVARUSHORTARRAY:
		case kDEVVARULONGARRAY:
		case kDEVVARBOOLEANARRAY:
			func += "\tout_wave_path = cao.num_wave_path" + eol
			break
		case kDEVVARSTRINGARRAY:
			func += "\tout_wave_path = cao.str_wave_path" + eol
			break
		case kDEVVARLONGSTRINGARRAY:
		case kDEVVARDOUBLESTRINGARRAY:
			func += "\tout_num_wave_path = cao.num_wave_path" + eol
			func += "\tout_str_wave_path = cao.str_wave_path" + eol
			break
	endswitch
	func += "\treturn kNO_ERROR" + eol
	func += "end" + eol
	return func
end