
/*
/
/ STATRT OF GPS API : MODULE INTERPRETATION CLASS
/ NOTE : MUST BE ABSTRACT INDEPENDENT OF OPERATING SYSTEMS AND ONLY C++ SPECIFIC DATA TYPES
/
*/


/* --------------------------------------------- NOTES --------------------------------------------------
*
*	1) Must apply poll functin from unistd.h to all reading methods. Already done the read_pattern() method
*		otherwise, if you only used read() functin without poll functin first, you wil get input/output errors
*
*
*/


#include "GPS_Module.h"


/*
/ DESCRIPTION;
/ Constructor for the GPS MODULE class
/
/ RETURN:
/ None
*/


GPS_Module::GPS_Module() {}


//TODO: Fill up the method body
GPS_Module::GPS_Module(string _com_num) : _serial_module(_com_num) {
}


/*
/ DESCRIPTION;
/ It initilizes the serial port handle instance and all the
/ necessary steps to be able to read from the serial port
/
/ TODO: 1) change the return massage to meaningful output
/		to determine which funtion caused the -1
/
/		2) make it return value from _serial_module
/
/ RETURN:
/ If successful, it returns 1, otherwise, returns -1
*/

INT_8 GPS_Module::init(string COM_num) {

	// pass com number and baud rate to serial module instance
	this->_serial_module.init(COM_num);

	this->config_gps.SET_MULTITHREAD = false;
	this->config_gps.SET_UPDATE = true;

	return 1;
}

bool GPS_Module::update_GPS_attr() {

	if (this->config_gps.SET_MULTITHREAD == true &&
		this->config_gps.SET_UPDATE == false) {
		// start the update thread TODO: use pthread instead
		gps_thread = std::thread([this] {threaded_update(); });
	}
	else if (this->config_gps.SET_P_THREAD == true &&
		this->config_gps.SET_UPDATE == false) {

		/* create a unix thread which executes p_threaded_update */
		/*if (pthread_create(&_unix_thread, NULL, GPS_Module::p_threaded_update, this)) {

			std::clog << "Error creating thread\n";
			return 0;

		}*/

	}
	return 1;
}

bool GPS_Module::set_gps_config(GPS_Config const& config) {

	this->config_gps = config;
	this->update_GPS_attr();

	return 1;
}


bool GPS_Module::get_gps_config(GPS_Config& config) {

	config = this->config_gps;
	return 1;
}

bool GPS_Module::set_serial_config(Serial_Config const& config) {

	if (config.is_init == true) {
		_serial_module.set_config(config);
		return 1;
	}
	else {
		cerr << "Error: Call get_seria_config first.";
		return 0;
	}
}


bool GPS_Module::get_serial_config(Serial_Config& config) {

	_serial_module.get_config(config);
	return 1;
}

/*
/ DESCRIPTION;
/ It calls the serial port handle from the Serial_Handle class
/ to read the data lines from the module communication
/
/ RETURN:
/ NMEA sentences line by line
*/

void GPS_Module::threaded_update() {

	while (true) {


		// reads gps pattern
		string nmea_data = this->_serial_module.read_pattern('$', '\n', 4);

		// Check for errors
		if (nmea_data.compare("-1") == 0) {
			cout << "Error: Cannot read lines";
			break;
		}

		size_t pos = 0;
		string deli = "\r\n";		// string delimiter
		string gps_snt = "a";			// one gps header token
		
		// Lock mutex to prevent sharing data
		std::lock_guard<std::mutex> lock(gps_mutex);

		/*-------------Begining of Critical Section ------------*/

		while ((pos = nmea_data.find(deli)) != string::npos) {
			gps_snt = nmea_data.substr(0, pos);
			if (gps_snt.substr(4, 1).compare("G") == 0)
				_gps_data[GGA] = gps_snt;
			else if (gps_snt.substr(4, 2).compare("SA") == 0)
				_gps_data[GSA] = gps_snt;
			else if (gps_snt.substr(4, 1).compare("M") == 0)
				_gps_data[RMC] = gps_snt;
			else if (gps_snt.substr(4, 1).compare("T") == 0)
				_gps_data[VTG] = gps_snt;
			else if (gps_snt.substr(4, 2).compare("SV") == 0)
				_gps_data[GSV] = gps_snt;

			nmea_data.erase(0, pos + deli.length());
		}

		/*-------------Ending of Critical Section ------------*/

		// to save cpu power, let thread sleep for 1 second every update
		std::this_thread::sleep_for(chrono::milliseconds(1000));
	}
}


/*TODO: cannot use pthread with member methods, make it static and pass the object*/

void* GPS_Module::p_threaded_update(void* arg_void_ptr) {

	while (true) {


		// reads gps pattern
		string nmea_data = this->_serial_module.read_pattern('$', '\n', 4);

		// Check for errors
		if (nmea_data.compare("-1") == 0) {
			cout << "Error: Cannot read lines";
			break;
		}

		size_t pos = 0;
		string deli = "\r\n";		// string delimiter
		string gps_snt = "a";			// one gps header token

		while ((pos = nmea_data.find(deli)) != string::npos) {
			gps_snt = nmea_data.substr(0, pos);
			if (gps_snt.substr(4, 1).compare("G") == 0)
				_gps_data[GGA] = gps_snt;
			else if (gps_snt.substr(4, 2).compare("SA") == 0)
				_gps_data[GSA] = gps_snt;
			else if (gps_snt.substr(4, 1).compare("M") == 0)
				_gps_data[RMC] = gps_snt;
			else if (gps_snt.substr(4, 1).compare("T") == 0)
				_gps_data[VTG] = gps_snt;
			else if (gps_snt.substr(4, 2).compare("SV") == 0)
				_gps_data[GSV] = gps_snt;

			nmea_data.erase(0, pos + deli.length());
		}

	}

	return NULL;
}

void GPS_Module::update() {

	// ----------------------- SERIES BUG HERE ----------------------------
	/*
	/ It was giving me empty output and stuck in console, but when I cleared
	/ and initilized the the string array, it was resolved. However, when
	/ removed the inisialization/clearing it was still showing, it should
	/ go back like before. I ran the previous bugged source code in another
	/ computer, and it worked.
	/ NOTES: I might have cleared the buffer in the usbuart/gps modules?
	*/

	// reads gps pattern
	string nmea_data = this->_serial_module.read_pattern('$', '\n', 4);

	// Check for errors
	if (nmea_data.compare("-1") == 0) {
		cerr << "Error: Cannot read lines";
		return;
	}

	size_t pos = 0;
	string deli = "\r\n";		// string delimiter
	string gps_snt;			// one gps header token

	while ((pos = nmea_data.find(deli)) != string::npos) {

		gps_snt = nmea_data.substr(0, pos);
		if (gps_snt.substr(4, 1).compare("G") == 0)
			_gps_data[GGA] = gps_snt;
		else if (gps_snt.substr(4, 2).compare("SA") == 0)
			_gps_data[GSA] = gps_snt;
		else if (gps_snt.substr(4, 1).compare("M") == 0)
			_gps_data[RMC] = gps_snt;
		else if (gps_snt.substr(4, 1).compare("T") == 0)
			_gps_data[VTG] = gps_snt;
		else if (gps_snt.substr(4, 2).compare("SV") == 0)
			_gps_data[GSV] = gps_snt;
		else
			cerr << "Error, cannot find delimiter";  //debugging

		nmea_data.erase(0, pos + deli.length());
	}

	return;
}



/*
/ DESCRIPTION;
/ ---------------------------------------------
/ $GPGGA : Global Positioning System Fix Data
/ ---------------------------------------------
/ Only picks the NMEA sentence starting with $GPGGA by comparing
/ the 4th letter of the sentence with the letter "G"
/
/ RETURN:
/ NMEA sentence starting with $GPGGA
*/

string GPS_Module::get_GP_GGA() {
	string gp_gga = _gps_data[GGA];

	// To avoid out-range memory exception when calling substr
	if (gp_gga.empty()) { return ""; }
	return gp_gga.substr(4, 1).compare("G") == 0 ? gp_gga : "";
}

/*
/ DESCRIPTION;
/
/ Parses the NMEA sentence into readable formating
/
/ RETURN:
/  1 if sentence has been parsed, if empty 0
*/

INT_8 GPS_Module::parse_GGA() {

	string gp_gga = get_GP_GGA();

	if (gp_gga.empty()) // to avoid out-of-bound exception
		return -1;

	int char_pos = 0;
	for (int i = 0; i < GGA_LENGTH; ++i) {
		parsed_gga_data[i].clear();
	}

	for (int parse_index = 0; parse_index < GGA_LENGTH; ++parse_index) {
		for (; char_pos < gp_gga.length(); ++char_pos) {
			if (gp_gga.at(char_pos) == ',' || gp_gga.at(char_pos) == '*') {
				if (gp_gga.at(char_pos - 1) == ',') {  //if the previous char was also ',' then append empty 
					parsed_gga_data[parse_index] += "mpt";
				}
				++char_pos; // manually increase, break doesn't allow increase in for-loop
				break;
			}

			if (gp_gga.at(char_pos) == '\r') break;
			parsed_gga_data[parse_index] += gp_gga.at(char_pos);
		}
	}

	//Calculate check sum, return either ok or bad
	parsed_gga_data[GGA_CHECK_SUM] = cal_check_sum(gp_gga, parsed_gga_data[GGA_CHECK_SUM]);

	//Parse specific data e.g. utc
	if (parsed_gga_data[GGA_UTC] != "mpt") {
		parsed_gga_data[GGA_UTC].insert(0, "H:");
		parsed_gga_data[GGA_UTC].insert(4, " M:");
		parsed_gga_data[GGA_UTC].insert(9, " S:");
	}

	//Parse Latitude 
	if (parsed_gga_data[LAT] != "mpt") {
		parsed_gga_data[LAT].insert(0, "D:");
		parsed_gga_data[LAT].insert(4, " M:");
	}

	//Parse Longitude 
	if (parsed_gga_data[LON] != "mpt") {
		parsed_gga_data[LON].insert(0, "D:");
		parsed_gga_data[LON].insert(5, " M:");
	}


	return 1; // must clear memory with delete[] later if use string*
}


/*
/  TODO:
/  Fix the get useful functions regarding time showing them together
*/

/*
/ DESCRIPTION;
/
/ Prints the parsed NMEA sentence into rows with titles
/
/ RETURN:
/  None
*/

void GPS_Module::print_par_GGA() {

	// First parse the data of GGA then print them using enums
	if (parse_GGA() == -1)
		return;

	// Printing the parsed data in rows
	cout << "Header : " << parsed_gga_data[GGA_HEADER] << endl;
	cout << "UTC : " << parsed_gga_data[GGA_UTC] << endl;
	cout << "Latitude : " << parsed_gga_data[LAT] << endl;
	cout << "Latitude Dir : " << parsed_gga_data[LAT_DIR] << endl;
	cout << "Longitude : " << parsed_gga_data[LON] << endl;
	cout << "Longitude Dir : " << parsed_gga_data[LON_DIR] << endl;
	cout << "Quality : " << parsed_gga_data[QUALITY] << endl;
	cout << "Num of Satellites : " << parsed_gga_data[NUM_SATS] << endl;
	cout << "Horizontal Dil : " << parsed_gga_data[HOR_DIL] << endl;
	cout << "Antenna Altitude : " << parsed_gga_data[ALT] << endl;
	cout << "Units of Antenna : " << parsed_gga_data[A_UNITS] << endl;
	cout << "Undulation : " << parsed_gga_data[UNDULATION] << endl;
	cout << "Units of Undul : " << parsed_gga_data[U_UNITS] << endl;
	cout << "Age : " << parsed_gga_data[AGE] << endl;
	cout << "Data Star : " << parsed_gga_data[GGA_DIFF] << endl;
	cout << "Check Sum : " << parsed_gga_data[GGA_CHECK_SUM] << endl << endl;

	return;
}


/*
/ DESCRIPTION;
/ ---------------------------------------------
/ $GPGSA : GPS DOP and active satellites
/ ---------------------------------------------
/ Only picks the NMEA sentence starting with $GPGSA by comparing
/ the 4th and 5th letters of the sentence with the letters "SA"
/
/ RETURN:
/ NMEA sentence starting with $GPGSA
*/

string GPS_Module::get_GP_GSA() {
	string gp_gsa = _gps_data[GSA];

	// To avoid out-range memory exception when calling substr
	if (gp_gsa.empty()) { return ""; }
	return gp_gsa.substr(4, 2).compare("SA") == 0 ? gp_gsa : "";
}

/*
/ DESCRIPTION;
/
/ Parses the NMEA sentence into readable formating
/
/ RETURN:
/  1 if sentence has been parsed, if empty 0
*/

INT_8 GPS_Module::parse_GSA() {

	string gp_gsa = get_GP_GSA();

	if (gp_gsa.empty()) // to avoid out-of-bound exception
		return -1;

	int char_pos = 0;
	for (int i = 0; i < GSA_LENGTH; ++i) {
		parsed_gsa_data[i].clear();
	}

	for (int parse_index = 0; parse_index < GSA_LENGTH; ++parse_index) {
		for (; char_pos < gp_gsa.length(); ++char_pos) {
			if (gp_gsa.at(char_pos) == ',' || gp_gsa.at(char_pos) == '*') {
				if (gp_gsa.at(char_pos - 1) == ',') {  //if the previous char was also ',' then append empty 
					parsed_gsa_data[parse_index] += "mpt";
				}
				++char_pos; // manually increase, break doesn't allow increase in for-loop
				break;
			}
			if (gp_gsa.at(char_pos) == '\r') break;
			parsed_gsa_data[parse_index] += gp_gsa.at(char_pos);
		}
	}


	//Calculate check sum, return either ok or bad
	parsed_gsa_data[GSA_CHECK_SUM] = cal_check_sum(gp_gsa, parsed_gsa_data[GSA_CHECK_SUM]);

	return 1; // must clear memory with delete[] later if use string*
}


/*
/ DESCRIPTION;
/
/ Prints the parsed NMEA sentence into rows with titles
/
/ RETURN:
/  None
*/

void GPS_Module::print_par_GSA() {

	// First parse the data of GGA then print them using enums
	if (parse_GSA() == -1)
		return;

	// Printing the parsed data in rows
	cout << "Header : " << parsed_gsa_data[GSA_HEADER] << endl;
	cout << "Mode 1 : " << parsed_gsa_data[MODE_1] << endl;
	cout << "Mode 2 : " << parsed_gsa_data[MODE_2] << endl;
	cout << "Sat_Ch.1 : " << parsed_gsa_data[SAT_1] << endl;
	cout << "Sat_Ch.2 : " << parsed_gsa_data[SAT_2] << endl;
	cout << "Sat_Ch.3 : " << parsed_gsa_data[SAT_3] << endl;
	cout << "Sat_Ch.4 : " << parsed_gsa_data[SAT_4] << endl;
	cout << "Sat_Ch.5 : " << parsed_gsa_data[SAT_5] << endl;
	cout << "Sat_Ch.6 : " << parsed_gsa_data[SAT_6] << endl;
	cout << "Sat_Ch.7 : " << parsed_gsa_data[SAT_7] << endl;
	cout << "Sat_Ch.8 : " << parsed_gsa_data[SAT_8] << endl;
	cout << "Sat_Ch.9 : " << parsed_gsa_data[SAT_9] << endl;
	cout << "Sat_Ch.10 : " << parsed_gsa_data[SAT_10] << endl;
	cout << "Sat_Ch.11 : " << parsed_gsa_data[SAT_11] << endl;
	cout << "Sat_Ch.11 : " << parsed_gsa_data[SAT_12] << endl;
	cout << "PDOP : " << parsed_gsa_data[PDOP] << endl;
	cout << "HDOP : " << parsed_gsa_data[HDOP] << endl;
	cout << "VDOP : " << parsed_gsa_data[VDOP] << endl;
	cout << "Check Sum : " << parsed_gsa_data[GSA_CHECK_SUM] << endl << endl;

	return;
}

/*
/ DESCRIPTION;
/ ---------------------------------------------
/ $GPRMC : Recommended minimum specific Loran-C data
/ ---------------------------------------------
/ Only picks the NMEA sentence starting with $GPRMC by comparing
/ the 4th letter of the sentence with the letter "M"
/
/ RETURN:
/ NMEA sentence starting with $GPRMC
*/

string GPS_Module::get_GP_RMC() {
	string gp_rmc = _gps_data[RMC];

	// To avoid out-range memory exception when calling substr
	if (gp_rmc.empty()) { return ""; }
	return gp_rmc.substr(4, 1).compare("M") == 0 ? gp_rmc : "";
}

/*
/ DESCRIPTION;
/
/ Parses the NMEA sentence into readable formating
/
/ RETURN:
/  1 if sentence has been parsed, if empty 0
*/

INT_8 GPS_Module::parse_RMC() {

	string gp_rmc = get_GP_RMC();

	if (gp_rmc.empty()) // to avoid out-of-bound exception
		return -1;

	int char_pos = 0;
	for (int i = 0; i < RMC_LENGTH; ++i) {
		parsed_rmc_data[i].clear();
	}

	for (int parse_index = 0; parse_index < RMC_LENGTH; ++parse_index) {
		for (; char_pos < gp_rmc.length(); ++char_pos) {
			if (gp_rmc.at(char_pos) == ',' || gp_rmc.at(char_pos) == '*') {
				if (gp_rmc.at(char_pos - 1) == ',') {  //if the previous char was also ',' then append empty 
					parsed_rmc_data[parse_index] += "mpt";
				}
				++char_pos; // manually increase, break doesn't allow increase in for-loop
				break;
			}

			if (gp_rmc.at(char_pos) == '\r') break;
			parsed_rmc_data[parse_index] += gp_rmc.at(char_pos);
		}
	}
	//Calculate check sum, return either ok or bad
	parsed_rmc_data[RMC_CHECK_SUM] = cal_check_sum(gp_rmc, parsed_rmc_data[RMC_CHECK_SUM]);

	//Parse specific data e.g. utc
	if (parsed_rmc_data[RMC_UTC] != "mpt") {
		parsed_rmc_data[RMC_UTC].insert(0, "H:");
		parsed_rmc_data[RMC_UTC].insert(4, " M:");
		parsed_rmc_data[RMC_UTC].insert(9, " S:");
	}

	//Parse Latitude 
	if (parsed_rmc_data[RMC_LAT] != "mpt") {
		parsed_rmc_data[RMC_LAT].insert(0, "D:");
		parsed_rmc_data[RMC_LAT].insert(4, " M:");
	}

	//Parse Longitude 
	if (parsed_rmc_data[RMC_LON] != "mpt") {
		parsed_rmc_data[RMC_LON].insert(0, "D:");
		parsed_rmc_data[RMC_LON].insert(5, " M:");
	}

	//Parse DATE
	if (parsed_rmc_data[DATE] != "mpt") {
		parsed_rmc_data[DATE].insert(0, "D:");
		parsed_rmc_data[DATE].insert(4, " M:");
		parsed_rmc_data[DATE].insert(9, " Y:");
	}

	return 1; // must clear memory with delete[] later if use string*
}

/*
/ DESCRIPTION;
/
/ Prints the parsed NMEA sentence into rows with titles
/
/ RETURN:
/  None
*/

void GPS_Module::print_par_RMC() {

	// First parse the data of GGA then print them using enums
	if (parse_RMC() == -1)
		return;

	// Printing the parsed data in rows
	cout << "Header : " << parsed_rmc_data[RMC_HEADER] << endl;
	cout << "UTC : " << parsed_rmc_data[RMC_UTC] << endl;
	cout << "Status : " << parsed_rmc_data[STATUS] << endl;
	cout << "Latitude : " << parsed_rmc_data[RMC_LAT] << endl;
	cout << "Latitiude Dir : " << parsed_rmc_data[RMC_LAT_DIR] << endl;
	cout << "Longitude : " << parsed_rmc_data[RMC_LON] << endl;
	cout << "Longitude Dir : " << parsed_rmc_data[RMC_LON_DIR] << endl;
	cout << "Groudn Speed : " << parsed_rmc_data[SPD_GRD] << endl;
	cout << "Ground Course : " << parsed_rmc_data[CRS_GRD] << endl;
	cout << "Date : " << parsed_rmc_data[DATE] << endl;
	cout << "Magnetic Var : " << parsed_rmc_data[MAG_VAR] << endl;
	cout << "Mode : " << parsed_rmc_data[RMC_MODE] << endl;
	cout << "Data Star : " << parsed_rmc_data[RMC_STR] << endl;
	cout << "Check Sum : " << parsed_rmc_data[RMC_CHECK_SUM] << endl << endl;

	return;
}

/*
/ DESCRIPTION;
/ ---------------------------------------------
/ $GPVTG : Track made good and ground speed
/ ---------------------------------------------
/ Only picks the NMEA sentence starting with $GPVTG by comparing
/ the 4th letter of the sentence with the letter "T"
/
/ RETURN:
/ NMEA sentence starting with $GPVTG
*/

string GPS_Module::get_GP_VTG() {
	string gp_vtg = _gps_data[VTG];

	// To avoid out-range memory exception when calling substr
	if (gp_vtg.empty()) { return ""; }
	return gp_vtg.substr(4, 1).compare("T") == 0 ? gp_vtg : "";
}

/*
/ DESCRIPTION;
/
/ Parses the NMEA sentence into readable formating
/
/ RETURN:
/  1 if sentence has been parsed, if empty 0
*/

INT_8 GPS_Module::parse_VTG() {

	string gp_vtg = get_GP_VTG();

	if (gp_vtg.empty()) // to avoid out-of-bound exception
		return -1;

	int char_pos = 0;
	for (int i = 0; i < VTG_LENGTH; ++i) {
		parsed_vtg_data[i].clear();
	}

	for (int parse_index = 0; parse_index < VTG_LENGTH; ++parse_index) {
		for (; char_pos < gp_vtg.length(); ++char_pos) {
			if (gp_vtg.at(char_pos) == ',' || gp_vtg.at(char_pos) == '*') {
				if (gp_vtg.at(char_pos - 1) == ',') {  //if the previous char was also ',' then append empty 
					parsed_vtg_data[parse_index] += "mpt";
				}
				++char_pos; // manually increase, break doesn't allow increase in for-loop
				break;
			}

			if (gp_vtg.at(char_pos) == '\r') break;
			parsed_vtg_data[parse_index] += gp_vtg.at(char_pos);
		}
	}

	//Calculate check sum, return either ok or bad
	parsed_vtg_data[VTG_CHECK_SUM] = cal_check_sum(gp_vtg, parsed_vtg_data[VTG_CHECK_SUM]);

	return 1; // must clear memory with delete[] later if use string*
}

/*
/ DESCRIPTION;
/
/ Prints the parsed NMEA sentence into rows with titles
/
/ RETURN:
/  None
*/

void GPS_Module::print_par_VTG() {

	// First parse the data of GGA then print them using enums
	if (parse_VTG() == -1)
		return;

	// Printing the parsed data in rows
	cout << "Header : " << parsed_vtg_data[VTG_HEADER] << endl;
	cout << "Course.1 : " << parsed_vtg_data[CRS_1] << endl;
	cout << "Reference.1 : " << parsed_vtg_data[REF_1] << endl;
	cout << "Course.2 : " << parsed_vtg_data[CRS_2] << endl;
	cout << "Reference.2 : " << parsed_vtg_data[REF_2] << endl;
	cout << "Speed.1 : " << parsed_vtg_data[SPD_1] << endl;
	cout << "Units.1 : " << parsed_vtg_data[UNT_1] << endl;
	cout << "Speed.2 : " << parsed_vtg_data[SPD_2] << endl;
	cout << "Units.2 : " << parsed_vtg_data[UNT_2] << endl;
	cout << "Data Star : " << parsed_vtg_data[VTG_MODE] << endl;
	cout << "Check Sum : " << parsed_vtg_data[VTG_CHECK_SUM] << endl << endl;

	return;
}

/*
/ DESCRIPTION;
/ ---------------------------------------------
/ $GPGSV : GPS Satellites in view
/ ---------------------------------------------
/ Only picks the NMEA sentence starting with $GPGSV by comparing
/ the 4th and 5th letters of the sentence with the letters "SV"
/
/ RETURN:
/ NMEA sentence starting with $GPGSV
*/

string GPS_Module::get_GP_GSV() {
	string gp_gsv = _gps_data[GSV];

	// To avoid out-range memory exception when calling substr
	if (gp_gsv.empty()) { return ""; }
	return gp_gsv.substr(4, 2).compare("SV") == 0 ? gp_gsv : "";
}

/*
/ DESCRIPTION;
/
/ Parses the NMEA sentence into readable formating
/
/ RETURN:
/  1 if sentence has been parsed, if empty 0
*/

INT_8 GPS_Module::parse_GSV() {

	string gp_gsv = get_GP_GSV();

	if (gp_gsv.empty()) // to avoid out-of-bound exception
		return -1;

	int char_pos = 0;
	for (int i = 0; i < GSV_LENGTH; ++i) {
		parsed_gsv_data[i].clear();
	}

	for (int parse_index = 0; parse_index < GSV_LENGTH; ++parse_index) {
		for (; char_pos < gp_gsv.length(); ++char_pos) {
			if (gp_gsv.at(char_pos) == ',' || gp_gsv.at(char_pos) == '*') {
				if (gp_gsv.at(char_pos - 1) == ',') {  //if the previous char was also ',' then append empty 
					parsed_gsv_data[parse_index] += "mpt";
				}
				++char_pos; // manually increase, break doesn't allow increase in for-loop
				break;
			}

			if (gp_gsv.at(char_pos) == '\r') break;
			parsed_gsv_data[parse_index] += gp_gsv.at(char_pos);
		}
	}

	//Calculate check sum, return either ok or bad
	parsed_gsv_data[GSV_CHECK_SUM] = cal_check_sum(gp_gsv, parsed_gsv_data[GSV_CHECK_SUM]);

	return 1; // must clear memory with delete[] later if use string*
}


/*
/ DESCRIPTION;
/
/ Prints the parsed NMEA sentence into rows with titles
/
/ RETURN:
/  None
*/

void GPS_Module::print_par_GSV() {

	// First parse the data of GGA then print them using enums
	if (parse_GSV() == -1)
		return;

	// Printing the parsed data in rows
	cout << "Header : " << parsed_gsv_data[GSV_HEADER] << endl;
	cout << "Num of Messages : " << parsed_gsv_data[NUM_MSG] << endl;
	cout << "Message Number : " << parsed_gsv_data[MSG_NUM] << endl;
	cout << "SNR : " << parsed_gsv_data[SNR] << endl;
	cout << "Check Sum : " << parsed_gsv_data[GSV_CHECK_SUM] << endl << endl;

	return;
}


/*
/ DESCRIPTION;
/
/ Calculate the check sum by performing XOR on all the bytes between $ and *
/
/ RETURN:
/  Ok if checksum has integrity, Bad otherwise
*/

string GPS_Module::cal_check_sum(string gp_xxx, string gp_cksum) {

	// Total check sum 
	char tot_cksum = 0;

	//Start at the second letter to ignore the $ character
	// gp_xxx is any gps header e.g. gp_gga

	for (int char_count = 1; char_count < gp_xxx.length(); ++char_count) {

		//Break if reached *
		if (gp_xxx.at(char_count) == '*')
			break;

		//XORing every character 
		tot_cksum = char(tot_cksum ^ gp_xxx.at(char_count));

	}

	//Convert string to int first and then hex
	unsigned int temp_cksum;
	std::stringstream _stream;
	_stream << hex << gp_cksum;
	_stream >> temp_cksum;

	//Now conver it to hex
	std::stringstream __stream;
	__stream << hex << temp_cksum;

	//Create a strinstream to convert the xor to hexadecimal
	std::stringstream stream;
	stream << hex << int(tot_cksum);

	if (stream.str().compare(__stream.str()) == 0)
		return "Ok";
	else
		return "Bad";
}


/*
/ DESCRIPTION;
/
/ It parses the relevant sentence and calls the parsed data of utc
/
/ RETURN:
/  UTC TIME
*/

string GPS_Module::get_utc() {
	// First parse the data of GGA then print them using enums
	if (parse_GGA() == -1)
		return {};

	return parsed_gga_data[GGA_UTC];
}

/*
/ DESCRIPTION;
/
/ It parses the relevant sentence and calls the parsed data of speed
/
/ RETURN:
/  SPEED
*/

string GPS_Module::get_speed() {
	// First parse the data of GGA then print them using enums
	if (parse_VTG() == -1)
		return {};

	return parsed_vtg_data[SPD_1];
}

/*
/ DESCRIPTION;
/
/ It parses the relevant sentence and calls the parsed data of latitude
/
/ RETURN:
/ LATITUDE
*/

string GPS_Module::get_latitude() {
	// First parse the data of GGA then print them using enums
	if (parse_GGA() == -1)
		return {};

	return parsed_gga_data[LAT];
}

/*
/ DESCRIPTION;
/
/ It parses the relevant sentence and calls the parsed data of LONGITUDE
/
/ RETURN:
/ LONGITUDE
*/

string GPS_Module::get_longitude() {
	// First parse the data of GGA then print them using enums
	if (parse_GGA() == -1)
		return {};

	return parsed_gga_data[LON];
}

/*
/ DESCRIPTION;
/
/ It parses the relevant sentence and calls the parsed data of DATE
/
/ RETURN:
/ DATE
*/

string GPS_Module::get_date() {
	// First parse the data of GGA then print them using enums
	if (parse_RMC() == -1)
		return {};

	return parsed_rmc_data[DATE];
}

/*
/ DESCRIPTION;
/
/ It parses the relevant sentence and calls the parsed data of SATALLITES NUMBER
/
/ RETURN:
/ NUMBER OF SATELLITES
*/

string GPS_Module::get_num_sat() {
	// First parse the data of GGA then print them using enums
	if (parse_GGA() == -1)
		return {};

	return parsed_gga_data[NUM_SATS];
}


/*
/ DESCRIPTION;
/
/ It parses the relevant sentence and calls the parsed data of GPGGA header
/
/ RETURN:
/ GPGGA header
*/

string GPS_Module::get_gga_header() {
	// First parse the data of GGA then print them using enums
	if (parse_GGA() == -1)
		return {};

	return parsed_gga_data[GGA_HEADER];
}


/*
/ DESCRIPTION;
/
/ It parses the relevant sentence and calls the parsed data of GPGGA
/ latitude direction
/
/ RETURN:
/ GPGGA latitude direction
*/

string GPS_Module::get_lat_dir() {
	// First parse the data of GGA then print them using enums
	if (parse_GGA() == -1)
		return {};

	return parsed_gga_data[LAT_DIR];
}

/*
/ DESCRIPTION;
/
/ It parses the relevant sentence and calls the parsed data of GPGGA
/ longitude direction
/
/ RETURN:
/ GPGGA longitude direction
*/

string GPS_Module::get_lon_dir() {
	// First parse the data of GGA then print them using enums
	if (parse_GGA() == -1)
		return {};

	return parsed_gga_data[LON_DIR];
}

/*
/ DESCRIPTION;
/
/ It parses the relevant sentence and calls the parsed data of GPGGA quality
/
/ RETURN:
/ GPGGA quality
*/

string GPS_Module::get_quality() {
	// First parse the data of GGA then print them using enums
	if (parse_GGA() == -1)
		return {};

	return parsed_gga_data[QUALITY];
}

/*
/ DESCRIPTION;
/
/ It parses the relevant sentence and calls the parsed data of GPGGA
/ horizontal dilution
/
/ RETURN:
/ GPGGA horizontal dilution
*/

string GPS_Module::get_hor_dil() {
	// First parse the data of GGA then print them using enums
	if (parse_GGA() == -1)
		return {};

	return parsed_gga_data[HOR_DIL];
}


/*
/ DESCRIPTION;
/
/ It parses the relevant sentence and calls the parsed data of GPGGA altitude
/
/ RETURN:
/ GPGGA altitude
*/

string GPS_Module::get_alt() {
	// First parse the data of GGA then print them using enums
	if (parse_GGA() == -1)
		return {};

	return parsed_gga_data[ALT];
}

/*
/ DESCRIPTION;
/
/ It parses the relevant sentence and calls the parsed data of GPGGA
/ antenna units
/
/ RETURN:
/ GPGGA antenna units
*/

string GPS_Module::get_antenna_units() {
	// First parse the data of GGA then print them using enums
	if (parse_GGA() == -1)
		return {};

	return parsed_gga_data[A_UNITS];
}


/*
/ DESCRIPTION;
/
/ It parses the relevant sentence and calls the parsed data of GPGGA
/ Undulation geoid and ellipsoid
/
/ RETURN:
/ GPGGA Undulation geoid and ellipsoid
*/

string GPS_Module::get_undulation() {
	// First parse the data of GGA then print them using enums
	if (parse_GGA() == -1)
		return {};

	return parsed_gga_data[UNDULATION];
}

/*
/ DESCRIPTION;
/
/ It parses the relevant sentence and calls the parsed data of GPGGA
/ Undulation units
/
/ RETURN:
/ GPGGA Undulation units
*/

string GPS_Module::get_undul_units() {
	// First parse the data of GGA then print them using enums
	if (parse_GGA() == -1)
		return {};

	return parsed_gga_data[U_UNITS];
}


/*
/ DESCRIPTION;
/
/ It parses the relevant sentence and calls the parsed data of GPGGA age
/
/ RETURN:
/ GPGGA age
*/

string GPS_Module::get_age() {
	// First parse the data of GGA then print them using enums
	if (parse_GGA() == -1)
		return {};

	return parsed_gga_data[AGE];
}

/*
/ DESCRIPTION;
/
/ It parses the relevant sentence and calls the parsed data of GPGGA
/ data before check sum (to be looked)
/
/ RETURN:
/ data before check sum
*/

string GPS_Module::get_gga_diff() {
	// First parse the data of GGA then print them using enums
	if (parse_GGA() == -1)
		return {};

	return parsed_gga_data[GGA_DIFF];
}

/*
/ DESCRIPTION;
/
/ It parses the relevant sentence and calls the parsed data of GPGGA
/ check sum
/
/ RETURN:
/ check sum
*/

string GPS_Module::get_gga_check_sum() {
	// First parse the data of GGA then print them using enums
	if (parse_GGA() == -1)
		return {};

	return parsed_gga_data[GGA_CHECK_SUM];
}

/*
/ DESCRIPTION;
/
/ It parses the relevant sentence and calls the parsed data of GPGGA
/ length of it's parsed elements
/
/ RETURN:
/ Number of parsed elements
*/

string GPS_Module::get_gga_len() {
	// First parse the data of GGA then print them using enums
	if (parse_GGA() == -1)
		return {};

	return parsed_gga_data[GGA_LENGTH];
}


//--------------------------------------------------------------------


//			      START OF GPGSA GETTERS METHODS


//--------------------------------------------------------------------


string GPS_Module::get_gsa_header() {
	// First parse the data of GSA then print them using enums
	if (parse_GSA() == -1)
		return {};

	return parsed_gsa_data[GSA_HEADER];
}

string GPS_Module::get_gsa_mode_1() {
	// First parse the data of GSA then print them using enums
	if (parse_GSA() == -1)
		return {};

	return parsed_gsa_data[MODE_1];
}
string GPS_Module::get_gsa_mode_2() {
	// First parse the data of GSA then print them using enums
	if (parse_GSA() == -1)
		return {};

	return parsed_gsa_data[MODE_2];
}

string GPS_Module::get_sat_1() {
	// First parse the data of GSA then print them using enums
	if (parse_GSA() == -1)
		return {};

	return parsed_gsa_data[SAT_1];
}


string GPS_Module::get_sat_2() {
	// First parse the data of GSA then print them using enums
	if (parse_GSA() == -1)
		return {};

	return parsed_gsa_data[SAT_2];
}

string GPS_Module::get_sat_3() {
	// First parse the data of GSA then print them using enums
	if (parse_GSA() == -1)
		return {};

	return parsed_gsa_data[SAT_3];
}

string GPS_Module::get_sat_4() {
	// First parse the data of GSA then print them using enums
	if (parse_GSA() == -1)
		return {};

	return parsed_gsa_data[SAT_4];
}

string GPS_Module::get_sat_5() {
	// First parse the data of GSA then print them using enums
	if (parse_GSA() == -1)
		return {};

	return parsed_gsa_data[SAT_5];
}

string GPS_Module::get_sat_6() {
	// First parse the data of GSA then print them using enums
	if (parse_GSA() == -1)
		return {};

	return parsed_gsa_data[SAT_6];
}

string GPS_Module::get_sat_7() {
	// First parse the data of GSA then print them using enums
	if (parse_GSA() == -1)
		return {};

	return parsed_gsa_data[SAT_7];
}

string GPS_Module::get_sat_8() {
	// First parse the data of GSA then print them using enums
	if (parse_GSA() == -1)
		return {};

	return parsed_gsa_data[SAT_8];
}

string GPS_Module::get_sat_9() {
	// First parse the data of GSA then print them using enums
	if (parse_GSA() == -1)
		return {};

	return parsed_gsa_data[SAT_9];
}

string GPS_Module::get_sat_10() {
	// First parse the data of GSA then print them using enums
	if (parse_GSA() == -1)
		return {};

	return parsed_gsa_data[SAT_10];
}

string GPS_Module::get_sat_11() {
	// First parse the data of GSA then print them using enums
	if (parse_GSA() == -1)
		return {};

	return parsed_gsa_data[SAT_11];
}

string GPS_Module::get_sat_12() {
	// First parse the data of GSA then print them using enums
	if (parse_GSA() == -1)
		return {};

	return parsed_gsa_data[SAT_12];
}


string GPS_Module::get_pdop() {
	// First parse the data of GSA then print them using enums
	if (parse_GSA() == -1)
		return {};

	return parsed_gsa_data[PDOP];
}

string GPS_Module::get_hdop() {
	// First parse the data of GSA then print them using enums
	if (parse_GSA() == -1)
		return {};

	return parsed_gsa_data[HDOP];
}

string GPS_Module::get_VDOP() {
	// First parse the data of GSA then print them using enums
	if (parse_GSA() == -1)
		return {};

	return parsed_gsa_data[VDOP];
}

string GPS_Module::get_gsa_check_sum() {
	// First parse the data of GSA then print them using enums
	if (parse_GSA() == -1)
		return {};

	return parsed_gsa_data[GSA_CHECK_SUM];
}

string GPS_Module::get_gsa_len() {
	// First parse the data of GSA then print them using enums
	if (parse_GSA() == -1)
		return {};

	return parsed_gsa_data[GSA_LENGTH];
}



//--------------------------------------------------------------------


//			      START OF GPRMC GETTERS METHODS


//--------------------------------------------------------------------

string GPS_Module::get_rmc_header() {
	// First parse the data of GSA then print them using enums
	if (parse_RMC() == -1)
		return {};

	return parsed_rmc_data[RMC_LENGTH];
}

string GPS_Module::get_rmc_utc() {
	// First parse the data of GSA then print them using enums
	if (parse_RMC() == -1)
		return {};

	return parsed_rmc_data[RMC_UTC];
}

string GPS_Module::get_status() {
	// First parse the data of GSA then print them using enums
	if (parse_RMC() == -1)
		return {};

	return parsed_rmc_data[STATUS];
}

string GPS_Module::get_rmc_lat() {
	// First parse the data of GSA then print them using enums
	if (parse_RMC() == -1)
		return {};

	return parsed_rmc_data[RMC_LAT];
}


string GPS_Module::get_rmc_lat_dir() {
	// First parse the data of GSA then print them using enums
	if (parse_RMC() == -1)
		return {};

	return parsed_rmc_data[RMC_LAT_DIR];
}

string GPS_Module::get_rmc_lon() {
	// First parse the data of GSA then print them using enums
	if (parse_RMC() == -1)
		return {};

	return parsed_rmc_data[RMC_LON];
}

string GPS_Module::get_rmc_lon_dir() {
	// First parse the data of GSA then print them using enums
	if (parse_RMC() == -1)
		return {};

	return parsed_rmc_data[RMC_LON_DIR];
}

string GPS_Module::get_spd_grd() {
	// First parse the data of GSA then print them using enums
	if (parse_RMC() == -1)
		return {};

	return parsed_rmc_data[SPD_GRD];
}


string GPS_Module::get_crs_grd() {
	// First parse the data of GSA then print them using enums
	if (parse_RMC() == -1)
		return {};

	return parsed_rmc_data[CRS_GRD];
}

string GPS_Module::get_mag_var() {
	// First parse the data of GSA then print them using enums
	if (parse_RMC() == -1)
		return {};

	return parsed_rmc_data[MAG_VAR];
}

string GPS_Module::get_rmc_mode() {
	// First parse the data of GSA then print them using enums
	if (parse_RMC() == -1)
		return {};

	return parsed_rmc_data[RMC_MODE];
}

string GPS_Module::get_rmc_str() {
	// First parse the data of GSA then print them using enums
	if (parse_RMC() == -1)
		return {};

	return parsed_rmc_data[RMC_STR];
}

string GPS_Module::get_rmc_check_sum() {
	// First parse the data of GSA then print them using enums
	if (parse_RMC() == -1)
		return {};

	return parsed_rmc_data[RMC_CHECK_SUM];
}

string GPS_Module::get_rmc_len() {
	// First parse the data of GSA then print them using enums
	if (parse_RMC() == -1)
		return {};

	return parsed_rmc_data[RMC_LENGTH];
}


//--------------------------------------------------------------------


//			      START OF GPVTG GETTERS METHODS


//--------------------------------------------------------------------

string GPS_Module::get_vtg_header() {
	// First parse the data of GSA then print them using enums
	if (parse_VTG() == -1)
		return {};

	return parsed_vtg_data[VTG_HEADER];
}

string GPS_Module::get_crs_1() {
	// First parse the data of GSA then print them using enums
	if (parse_VTG() == -1)
		return {};

	return parsed_vtg_data[CRS_1];
}
string GPS_Module::get_ref_1() {
	// First parse the data of GSA then print them using enums
	if (parse_VTG() == -1)
		return {};

	return parsed_vtg_data[REF_1];
}
string GPS_Module::get_crs_2() {
	// First parse the data of GSA then print them using enums
	if (parse_VTG() == -1)
		return {};

	return parsed_vtg_data[CRS_2];
}


string GPS_Module::get_ref_2() {
	// First parse the data of GSA then print them using enums
	if (parse_VTG() == -1)
		return {};

	return parsed_vtg_data[REF_2];
}

string GPS_Module::get_spd_1() {
	// First parse the data of GSA then print them using enums
	if (parse_VTG() == -1)
		return {};

	return parsed_vtg_data[SPD_1];
}
string GPS_Module::get_units_1() {
	// First parse the data of GSA then print them using enums
	if (parse_VTG() == -1)
		return {};

	return parsed_vtg_data[UNT_1];
}
string GPS_Module::get_spd_2() {
	// First parse the data of GSA then print them using enums
	if (parse_VTG() == -1)
		return {};

	return parsed_vtg_data[SPD_2];
}


string GPS_Module::get_units_2() {
	// First parse the data of GSA then print them using enums
	if (parse_VTG() == -1)
		return {};

	return parsed_vtg_data[UNT_2];
}

string GPS_Module::get_vtg_mode() {
	// First parse the data of GSA then print them using enums
	if (parse_VTG() == -1)
		return {};

	return parsed_vtg_data[VTG_MODE];
}
string GPS_Module::get_vtg_check_sum() {
	// First parse the data of GSA then print them using enums
	if (parse_VTG() == -1)
		return {};

	return parsed_vtg_data[VTG_CHECK_SUM];
}
string GPS_Module::get_vtg_len() {
	// First parse the data of GSA then print them using enums
	if (parse_VTG() == -1)
		return {};

	return parsed_vtg_data[VTG_LENGTH];
}


//--------------------------------------------------------------------


//			      START OF GPGSV GETTERS METHODS


//--------------------------------------------------------------------

string GPS_Module::get_gsv_header() {
	// First parse the data of GSA then print them using enums
	if (parse_GSV() == -1)
		return {};

	return parsed_gsv_data[GSV_HEADER];
}

string GPS_Module::get_num_msg() {
	// First parse the data of GSA then print them using enums
	if (parse_GSV() == -1)
		return {};

	return parsed_gsv_data[NUM_MSG];
}

string GPS_Module::get_msg_num() {
	// First parse the data of GSA then print them using enums
	if (parse_GSV() == -1)
		return {};

	return parsed_gsv_data[MSG_NUM];
}
string GPS_Module::get_snr() {
	// First parse the data of GSA then print them using enums
	if (parse_GSV() == -1)
		return {};

	return parsed_gsv_data[SNR];
}
string GPS_Module::get_gsv_check_sum() {
	// First parse the data of GSA then print them using enums
	if (parse_GSV() == -1)
		return {};

	return parsed_gsv_data[GSV_CHECK_SUM];
}
string GPS_Module::get_gsv_len() {
	// First parse the data of GSA then print them using enums
	if (parse_GSV() == -1)
		return {};

	return parsed_gsv_data[GSV_LENGTH];
}

INT_16 GPS_Module::get_NMEA_header_len() {

	return NMEA_header_len;
}

/*
/ DESCRIPTION;
/
/ Prints all the parsed NMEA sentence of all headers in an ordered table
/
/ RETURN:
/ NONE
*/

void GPS_Module::print_all_par_NMEA() {
	parse_GGA();
	parse_GSA();
	parse_RMC();
	parse_VTG();
	parse_GSV();

	cout << left << setw(12) << "Header : " << left << setw(25) << parsed_gga_data[GGA_HEADER] << left << setw(12) << "Header : " << left << setw(25) << parsed_gsa_data[GSA_HEADER] << left << setw(12) << "Header : " << left << setw(25) << parsed_rmc_data[RMC_HEADER] << left << setw(12) << "Header : " << left << setw(25) << parsed_vtg_data[VTG_HEADER] << left << setw(12) << "Header : " << left << setw(25) << parsed_gsv_data[GSV_HEADER] << endl;
	cout << left << setw(12) << "UTC : " << left << setw(25) << parsed_gga_data[GGA_UTC] << left << setw(12) << "Mode 1 : " << left << setw(25) << parsed_gsa_data[MODE_1] << left << setw(12) << "UTC : " << left << setw(25) << parsed_rmc_data[RMC_UTC] << left << setw(12) << "Course.1 : " << left << setw(25) << parsed_vtg_data[CRS_1] << left << setw(12) << "Num Msg : " << left << setw(25) << parsed_gsv_data[NUM_MSG] << endl;
	cout << left << setw(12) << "Latitude : " << left << setw(25) << parsed_gga_data[LAT] << left << setw(12) << "Mode 2 : " << left << setw(25) << parsed_gsa_data[MODE_2] << left << setw(12) << "Status : " << left << setw(25) << parsed_rmc_data[STATUS] << left << setw(12) << "Ref.1 : " << left << setw(25) << parsed_vtg_data[REF_1] << left << setw(12) << "Msg Num : " << left << setw(25) << parsed_gsv_data[MSG_NUM] << endl;
	cout << left << setw(12) << "Lat Dir : " << left << setw(25) << parsed_gga_data[LAT_DIR] << left << setw(12) << "Sat_Ch.1 : " << left << setw(25) << parsed_gsa_data[SAT_1] << left << setw(12) << "Latitude : " << left << setw(25) << parsed_rmc_data[RMC_LAT] << left << setw(12) << "Course.2 : " << left << setw(25) << parsed_vtg_data[CRS_2] << left << setw(12) << "SNR : " << left << setw(25) << parsed_gsv_data[SNR] << endl;
	cout << left << setw(12) << "Longitude : " << left << setw(25) << parsed_gga_data[LON] << left << setw(12) << "Sat_Ch.2 : " << left << setw(25) << parsed_gsa_data[SAT_2] << left << setw(12) << "Lat Dir : " << left << setw(25) << parsed_rmc_data[RMC_LAT_DIR] << left << setw(12) << "Ref.2 : " << left << setw(25) << parsed_vtg_data[REF_2] << left << setw(12) << "Check Sum : " << left << setw(25) << parsed_gsv_data[GSV_CHECK_SUM] << endl;
	cout << left << setw(12) << "Long Dir : " << left << setw(25) << parsed_gga_data[LON_DIR] << left << setw(12) << "Sat_Ch.3 : " << left << setw(25) << parsed_gsa_data[SAT_3] << left << setw(12) << "Longitude : " << left << setw(25) << parsed_rmc_data[RMC_LON] << left << setw(12) << "Speed.1 : " << left << setw(25) << parsed_vtg_data[SPD_1] << endl;
	cout << left << setw(12) << "Quality : " << left << setw(25) << parsed_gga_data[QUALITY] << left << setw(12) << "Sat_Ch.4 : " << left << setw(25) << parsed_gsa_data[SAT_4] << left << setw(12) << "Lon Dir : " << left << setw(25) << parsed_rmc_data[RMC_LON_DIR] << left << setw(12) << "Units.1 : " << left << setw(25) << parsed_vtg_data[UNT_1] << endl;
	cout << left << setw(12) << "Num Sats : " << left << setw(25) << parsed_gga_data[NUM_SATS] << left << setw(12) << "Sat_Ch.5 : " << left << setw(25) << parsed_gsa_data[SAT_5] << left << setw(12) << "Grd Speed : " << left << setw(25) << parsed_rmc_data[SPD_GRD] << left << setw(12) << "Speed.2 : " << left << setw(25) << parsed_vtg_data[SPD_2] << endl;
	cout << left << setw(12) << "Hor Dil : " << left << setw(25) << parsed_gga_data[HOR_DIL] << left << setw(12) << "Sat_Ch.6 : " << left << setw(25) << parsed_gsa_data[SAT_6] << left << setw(12) << "Grd Crs : " << left << setw(25) << parsed_rmc_data[CRS_GRD] << left << setw(12) << "Units.2 : " << left << setw(25) << parsed_vtg_data[UNT_2] << endl;
	cout << left << setw(12) << "Ant Alt : " << left << setw(25) << parsed_gga_data[ALT] << left << setw(12) << "Sat_Ch.7 : " << left << setw(25) << parsed_gsa_data[SAT_7] << left << setw(12) << "Date : " << left << setw(25) << parsed_rmc_data[DATE] << left << setw(12) << "Data Star : " << left << setw(25) << parsed_vtg_data[VTG_MODE] << endl;
	cout << left << setw(12) << "Uni Ant : " << left << setw(25) << parsed_gga_data[A_UNITS] << left << setw(12) << "Sat_Ch.8 : " << left << setw(25) << parsed_gsa_data[SAT_8] << left << setw(12) << "Mag Var : " << left << setw(25) << parsed_rmc_data[MAG_VAR] << left << setw(12) << "Check Sum : " << left << setw(25) << parsed_vtg_data[VTG_CHECK_SUM] << endl;
	cout << left << setw(12) << "Undul : " << left << setw(25) << parsed_gga_data[UNDULATION] << left << setw(12) << "Sat_Ch.9 : " << left << setw(25) << parsed_gsa_data[SAT_9] << left << setw(12) << "Mode : " << left << setw(25) << parsed_rmc_data[RMC_MODE] << endl;
	cout << left << setw(12) << "Uni Und : " << left << setw(25) << parsed_gga_data[U_UNITS] << left << setw(12) << "Sat_Ch.10 : " << left << setw(25) << parsed_gsa_data[SAT_10] << left << setw(12) << "Data Star : " << left << setw(25) << parsed_rmc_data[RMC_STR] << endl;
	cout << left << setw(12) << "Age : " << left << setw(25) << parsed_gga_data[AGE] << left << setw(12) << "Sat_Ch.11 : " << left << setw(25) << parsed_gsa_data[SAT_11] << left << setw(12) << "Check Sum : " << left << setw(25) << parsed_rmc_data[RMC_CHECK_SUM] << endl;
	cout << left << setw(12) << "Data Star : " << left << setw(25) << parsed_gga_data[GGA_DIFF] << left << setw(12) << "Sat_Ch.11 : " << left << setw(25) << parsed_gsa_data[SAT_12] << endl;
	cout << left << setw(12) << "Check Sum : " << left << setw(25) << parsed_gga_data[GGA_CHECK_SUM] << left << setw(12) << "PDOP : " << left << setw(25) << parsed_gsa_data[PDOP] << endl;
	cout << left << setw(37) << "" << "HDOP : " << left << setw(25) << parsed_gsa_data[HDOP] << endl;
	cout << left << setw(37) << "" << "VDOP : " << left << setw(25) << parsed_gsa_data[VDOP] << endl;
	cout << left << setw(37) << "" << "Check Sum : " << left << setw(25) << parsed_gsa_data[GSA_CHECK_SUM] << endl;

	cout << "------------------------------------------------------------------------------------------------------------------------------------------------------------------------" << endl;
}


void GPS_Module::close() {
	this->_serial_module.close();
}

/*
/ DESCRIPTION;
/ Destructor for the class, when done, it destroyes the serial Handle
/
/ RETURN:
/ None
*/

GPS_Module::~GPS_Module() {

	// TODD: You can use destructor alternatively
	//this->_serial_module.close();

	//TODO: close the thread appropriately
	if (this->config_gps.SET_MULTITHREAD == true)
		gps_thread.join();

	/* wait for the second thread to finish */
	else if (this->config_gps.SET_P_THREAD == true) {
		if (pthread_join(_unix_thread, NULL)) {

			std::clog << "Error joining thread\n";

		}
	}

}

