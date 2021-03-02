#include "gpx2.h"
#include <string>
#include <iostream>
#include <iomanip>

void print_hex(std::string str) {
	for (auto byte : str) {
		std::cout << std::hex << std::setw(2) << std::setfill('0') <<  static_cast<unsigned int>(byte) << " ";
	}
	std::cout << std::endl;
}

int main() {
	GPX2_TDC::GPX2 gpx2;
	GPX2_TDC::Config conf;

	/*
	std::cout << "trying to write config..." << std::endl;
	print_hex(conf.str());
	bool status = gpx2.write_config(conf);
	if (status) {
		std::cout << "config written..." << std::endl;
	}
	else {
		std::cout << "failed to write config!" << std::endl;
		exit(-1);
	}

	std::string config = gpx2.read_config();
	*/
	uint8_t confRegID = 0;
	std::string conf_str = conf.str();
	std::cout << "trying to write config reg " << confRegID << std::endl;
	print_hex(std::string({static_cast<char>(conf_str[confRegID])}));
	bool status = gpx2.write_config(confRegID, conf_str[confRegID]);
	if (status) {
		std::cout << "config written..." << std::endl;
	}
	else {
		std::cout << "failed to write config!" << std::endl;
		exit(-1);
	}
	uint8_t config = gpx2.read_config(confRegID);
	print_hex(std::string({static_cast<char>(config)}));
}
