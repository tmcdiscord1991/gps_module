#ifndef Serial_Config_H
#define Serial_Config_H

#include <iostream>
#include <string>

/*
/ NOTE : You add here any other module's configuration class and
/	make inheretence to the Serial_Config base class first.
*/

// --------- Serial Configuration ------------ //


/*
/	TODO: Fix errors related to setting values outside 
/		  the allowed range in the serial moduels, by
/		  permitting developer to set them through
		  error messages
		  For example, calling in this order:
				cfg::Serial_Config config;
				serial.get_config(config);
				cout << config.baud_rate << endl;
				config.baud_rate = 152000;
				serial.set_config(config);
				cout << config.baud_rate << endl;
/
*/


// ------------ BASE CLASS - SERIAL_CONFIG ------------//
namespace cfg {

	class Serial_Config {
	public:

		//TODO: Alternatively, you can call contructor here to initialize member fields
		bool is_init;
		// baud-rate, com name and bits per bytes memory allocation 
		speed_t baud_rate;
		int8_t bits_per_byte;

		// parity bit, stop field and flow control enumerations 
		enum parity_bit { EVEN, ODD, NONE_BIT, MARK, SPACE } parity_bit;
		enum stop_field { ONE, ONE_HALF, TWO } stop_field;
		enum flow_control { NONE, SOFTWARE, HARDWARE } flow_control;

		// TODO: change reading methods to fit non_canonical mode
		enum read_mode { CANONICAL, NON_CANONICAL } read_mode;

	};


	// GPS module inherets the data members of Serial configuration
	class GPS_Config : public Serial_Config{
	public:
		bool SET_MULTITHREAD;
		bool SET_P_THREAD;
		bool SET_UPDATE;
	};

}

#endif