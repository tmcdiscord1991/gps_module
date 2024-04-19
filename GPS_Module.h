
/*
/
/ STATRT OF MODULE INTERPRETATION HEADER FILE
/ NOTE : MUST BE ABSTRACT INDEPENDENT OF OPERATING SYSTEMS AND ONLY C++ SPECIFIC DATA TYPES
/
*/


#ifndef GPS_Module_H
#define GPS_Module_H

#include "Serial_Module.h"
#include<iostream>
#include<stdio.h>
#include <string>
#include <vector>
#include <iomanip>
#include <sstream>
#include <thread>
#include <mutex>
#include <atomic>


using namespace std;
using namespace cfg;

/*
/ DEFINING NEW VARIABLE TYPES FOR EFFECIENT MEMORY USE
*/

typedef short INT_16;
typedef int INT_32;

typedef signed char INT_8;
typedef unsigned char U_INT8;

typedef unsigned short U_INT16;
typedef unsigned int U_INT32;

class GPS_Module {

private:

	/*
	/	ONLY CHANGE THOSE ENUMS DATA BY INCREASEING THEM AND
	/	DECREASING IF YOU HAVE A DIFFERENT GPS MODULE THAT
	/	PRODUCES DIFFERENT NUMBERS OF PARAMETERS WITH EACH
	/	SETENCES HEADER. YOU DON'T HAVE HAVE TO CHANGE THE
	/	PARSING METHODS THEMSELVES THAT GETS THE SENTENCE.
	/	YOU ALSO NEED TO CHANGE THE PRINT METHODS.
	*/

	enum GPS_NMEA {
		GGA,
		GSA,
		RMC,
		VTG,
		GSV,
		NMEA_LEN
	};

	enum _GGA_DATA {
		GGA_HEADER,		    // Log header
		GGA_UTC,			// UTC of position fix
		LAT,				// Latitude
		LAT_DIR,			// Latitude Direction
		LON,				// Longitude
		LON_DIR,			// Longitude Direction
		QUALITY,			// Quality Indicators
		NUM_SATS,			// Number of Satellites
		HOR_DIL,			// Horizontal Dilution
		ALT,				// Antenna Altitude
		A_UNITS,			// Units of Antenna Altitude
		UNDULATION,			// Undulation geoid and ellipsoid
		U_UNITS,			// Units of Undulation
		AGE,				// Age of Correction Data
		GGA_DIFF,			// Data before *
		GGA_CHECK_SUM,		// Check Sum to Find Errors
		GGA_LENGTH			// Length of the Parsed Data

	};

	enum _GSA_DATA {
		GSA_HEADER,		    // Log header
		MODE_1,				// Mode number
		MODE_2,				// Mode number
		SAT_1,			    // Satellite 1
		SAT_2,				// Satellite 2
		SAT_3,			    // Satellite 3
		SAT_4,			    // Satellite 4
		SAT_5,			    // Satellite 5
		SAT_6,			    // Satellite 6
		SAT_7,				// Satellite 7
		SAT_8,			    // Satellite 8
		SAT_9,			    // Satellite 9
		SAT_10,			    // Satellite 10
		SAT_11,				// Satellite 11
		SAT_12,				// Satellite 12
		PDOP,
		HDOP,
		VDOP,
		GSA_CHECK_SUM,		// Check Sum to Find Errors
		GSA_LENGTH			// Length of the Parsed Data
	};

	enum _RMC_DATA {
		RMC_HEADER,		    // Log header
		RMC_UTC,			// UTC time status
		STATUS,				// Latitude
		RMC_LAT,			// Latitude 
		RMC_LAT_DIR,	    // Longitude Direction
		RMC_LON,			// Longitude 
		RMC_LON_DIR,	    // Longitude Direction
		SPD_GRD,			// Spped over ground
		CRS_GRD,		    // Course over ground
		DATE,				// Date
		MAG_VAR,			// Magnetic Variation
		RMC_MODE,
		RMC_STR,
		RMC_CHECK_SUM,		// Check Sum to Find Errors
		RMC_LENGTH			// Length of the Parsed Data
	};

	enum _VTG_DATA {
		VTG_HEADER,		    // Log header
		CRS_1,
		REF_1,				// Reference 1
		CRS_2,
		REF_2,	            // Reference 2
		SPD_1,			    // Speed 1
		UNT_1,	            // Units 1
		SPD_2,			    // Speed 2
		UNT_2,		        // Units 2
		VTG_MODE,
		VTG_CHECK_SUM,		// Check Sum to Find Errors
		VTG_LENGTH			// Length of the Parsed Data
	};

	enum _GSV_DATA {
		GSV_HEADER,		    // Log header
		NUM_MSG,			// Number of messages
		MSG_NUM,			// Messages number
		SNR,
		GSV_CHECK_SUM,		// Check Sum to Find Errors
		GSV_LENGTH			// Length of the Parsed Data
	};

	Serial_Module _serial_module;

	thread gps_thread;
	pthread_t _unix_thread;

	mutex gps_mutex;

	string _gps_data[NMEA_LEN];
	INT_16 NMEA_header_len = NMEA_LEN;

	string parsed_gga_data[GGA_LENGTH];
	string parsed_gsa_data[GSA_LENGTH];
	string parsed_rmc_data[RMC_LENGTH];
	string parsed_vtg_data[VTG_LENGTH];
	string parsed_gsv_data[GSV_LENGTH];

	// Private method to calculate checksum
	string cal_check_sum(string gp_xxx, string gp_cksum);

	void threaded_update();
	void *p_threaded_update(void* arg_void_ptr);
	bool update_GPS_attr();

public:

	GPS_Config config_gps;

	GPS_Module();
	GPS_Module(string _com_num);

	INT_8 init(string COM_num);

	bool set_gps_config(GPS_Config const& config);
	bool get_gps_config(GPS_Config& config);

	bool set_serial_config(Serial_Config const& config);
	bool get_serial_config(Serial_Config& config);

	void update();
	void close();

	string get_GP_GGA();
	string get_GP_GSA();
	string get_GP_RMC();
	string get_GP_VTG();
	string get_GP_GSV();

	INT_8 parse_GGA();
	void print_par_GGA();
	INT_8 parse_GSA();
	void print_par_GSA();
	INT_8 parse_RMC();
	void print_par_RMC();
	INT_8 parse_VTG();
	void print_par_VTG();
	INT_8 parse_GSV();
	void print_par_GSV();

	// GP_GGA Attributes
	string get_utc();
	string get_longitude();
	string get_latitude();
	string get_date();
	string get_num_sat();
	string get_gga_header();
	string get_lat_dir();
	string get_lon_dir();
	string get_quality();
	string get_hor_dil();
	string get_alt();
	string get_antenna_units();
	string get_undulation();
	string get_undul_units();
	string get_age();
	string get_gga_diff();
	string get_speed();
	string get_gga_check_sum();
	string get_gga_len();

	// GP_GSA Attributes
	string get_gsa_header();
	string get_gsa_mode_1();
	string get_gsa_mode_2();
	string get_sat_1();
	string get_sat_2();
	string get_sat_3();
	string get_sat_4();
	string get_sat_5();
	string get_sat_6();
	string get_sat_7();
	string get_sat_8();
	string get_sat_9();
	string get_sat_10();
	string get_sat_11();
	string get_sat_12();
	string get_pdop();
	string get_hdop();
	string get_VDOP();
	string get_gsa_check_sum();
	string get_gsa_len();

	// GP_RMC Attributes
	string get_rmc_header();
	string get_rmc_utc();
	string get_status();
	string get_rmc_lat();
	string get_rmc_lat_dir();
	string get_rmc_lon();
	string get_rmc_lon_dir();
	string get_spd_grd();
	string get_crs_grd();
	string get_mag_var();
	string get_rmc_mode();
	string get_rmc_str();
	string get_rmc_check_sum();
	string get_rmc_len();

	// GP_VTG Attributes
	string get_vtg_header();
	string get_crs_1();
	string get_ref_1();
	string get_crs_2();
	string get_ref_2();
	string get_spd_1();
	string get_units_1();
	string get_spd_2();
	string get_units_2();
	string get_vtg_mode();
	string get_vtg_check_sum();
	string get_vtg_len();

	// GP_GSV attributes
	string get_gsv_header();
	string get_num_msg();
	string get_msg_num();
	string get_snr();
	string get_gsv_check_sum();
	string get_gsv_len();
	INT_16 get_NMEA_header_len();
	void print_all_par_NMEA();

	~GPS_Module();

};



#endif