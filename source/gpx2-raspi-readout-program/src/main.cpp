#include "spidevices/gpx2/gpx2.h"
#include "gpio.h"
#include <string>
#include <iostream>
#include <iomanip>
#include <cmath>

constexpr double pico_second = 1e-12;
static double diff(const SPI::GPX2_TDC::Meas& first, const SPI::GPX2_TDC::Meas& second) {
	if (first.status == SPI::GPX2_TDC::Meas::Invalid || second.status == SPI::GPX2_TDC::Meas::Invalid || first.refclk_freq != second.refclk_freq || first.refclk_freq == 0.) {
		return 0;// std::cout << "measurement not valid" << std::endl;
	}
	double refclk_period = 1 / first.refclk_freq;
	double stop_first = first.lsb_ps * first.stop_result;
	double stop_second = second.lsb_ps * second.stop_result;
	double result{};
	result = refclk_period * static_cast<double>(second.ref_index - first.ref_index);
	result += (stop_second - stop_first) * pico_second;
	return result;
}

void print_hex(const std::string& str) {
	for (auto byte : str) {
		std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(byte) << " ";
	}
	std::cout << std::endl;
}

auto main()->int {
	SPI::GPX2_TDC::GPX2 gpx2{};
	SPI::GPX2_TDC::Config conf{};
	conf.loadDefaultConfig();

	//conf.PIN_ENA_STOP3 = 0;
	//conf.PIN_ENA_STOP4 = 0;
	//conf.HIT_ENA_STOP3 = 0;
	//conf.HIT_ENA_STOP4 = 0;

	gpx2.init();

	int verbosity = 0;

	bool status = gpx2.write_config(conf);
	std::string config = gpx2.read_config();
	if (status && (config == conf.str())) {
		if (verbosity > 1) {
			std::cout << "config written..." << std::endl;
		}
	}
	else {
		std::cout << "tried to write:" << std::endl;
		print_hex(conf.str());
		std::cout << "read back:" << std::endl;
		print_hex(config);
		std::cout << "failed to write config!" << std::endl;
		exit(-1);
	}

	gpio handler{};
	gpio::setting pin_setting{};
	pin_setting.gpio_pins = std::vector<unsigned int>{ 20 };

	auto callback = handler.list_callback(pin_setting);

	handler.start();

	gpx2.init_reset();
	while (true) {
		auto event = callback->wait(std::chrono::milliseconds {1000000});
		if (!event) {
			if (verbosity > 1) {
				std::cout << "event was neither rising nor falling. This should not happen. Stopping." << std::endl;
			}
			break;
		}
		/*if (event.type == gpio::event::Rising){
			std::cout << "gpio " << event.pin << " rising edge" << std::endl;
		}*/
		if (event.type == gpio::event::Falling) {
			//std::cout << "gpio " << event.pin << " falling edge" << std::endl;
			std::vector<double> diffs{};
			std::vector<std::vector<SPI::GPX2_TDC::Meas> > readout;
			for (int i = 0; i < 4; i++) {
				readout.push_back(gpx2.read_results());
				if (readout.size() < 4) {
					if (verbosity > 1) {
						std::cout << "readout vector has less than 4 channels";
					}
					continue;
				}
			}
			double max_interval = 10e-9;
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 4; j++) {
					double some_readout = 0;
					some_readout = diff(readout[i][j], readout[i][(j + 1) % 4]); // combinations are possible for 1+2, 2+3, 3+4, 1+4
					if (fabs(some_readout) < max_interval) {
						diffs.push_back(some_readout);
					}
				}
			}
			for (int i = 0; i < 3; i++) {
				for (int j = 0; j < 4; j++) {
					double some_readout = 0;
					some_readout = diff(readout[i][j], readout[i + 1][(j + 1) % 4]); // combinations are possible for 1+2, 2+3, 3+4, 1+4
					if (fabs(some_readout) < max_interval) {
						diffs.push_back(some_readout);
					}
					some_readout = diff(readout[i + 1][j], readout[i][(j + 1) % 4]); // combinations are possible for 1+2, 2+3, 3+4, 1+4
					if (fabs(some_readout) < max_interval) {
						diffs.push_back(some_readout);
					}
				}
			}

			for (auto val : diffs) {
				std::cout << fabs(val) << std::endl;
			}
		}
	}

	handler.stop();
	handler.join();
}
