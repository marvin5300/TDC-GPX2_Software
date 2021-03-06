#include "spidevices/gpx2/gpx2.h"
#include "spidevices/gpx2/config.h"
#include <iostream>
#include <iomanip>

using namespace SPI::GPX2_TDC;

void GPX2::init(std::string busAddress, std::uint32_t speed, Mode mode, std::uint8_t bits) {
	spiDevice::init(busAddress, speed, mode, bits);
	power_on_reset();
}

void GPX2::power_on_reset() {
	write(spiopc_power, "");
}

void GPX2::init_reset() {
	write(spiopc_init, "");
}

auto GPX2::write_config()->bool {
	return write_config(config);
}

auto GPX2::write_config(const Config& data)->bool {
	config = data;
	return write_config(data.str());
}

auto GPX2::write_config(const std::string& data)->bool {
	if (data.size() != 17) {
		std::cerr << "write all of the config only possible if 17 bytes of data provided\n";
		return false;
	}
	return write(spiopc_write_config, data);
}

auto GPX2::write_config(const std::uint8_t reg_addr, const std::uint8_t data)->bool {
	if (reg_addr > 16) {
		std::cerr << "write config is only possible on register addr. 0...16\n";
		return false;
	}
	std::string conf_str = std::string({static_cast<char>(data)});
	return write(spiopc_write_config | reg_addr, conf_str);
}

auto GPX2::read_config()->std::string {
	return read(spiopc_read_config, 17);
}

auto GPX2::read_config(const std::uint8_t reg_addr)->std::uint8_t {
	if (reg_addr > 16) {
		std::cerr << "read config is only possible on register addr. 0...16\n";
		return 0;
	}
	std::string data = read(spiopc_read_config | reg_addr, 1);
	if (data.empty()) {
		return 0;
	}
	return data[0];
}

auto GPX2::read_results()->std::vector<Meas> {
	std::string readout = read(spiopc_read_results | 0x08, 26);
	std::vector<Meas> measurements{};
		if (readout.empty()){
		return measurements;
	}
	double lsb_ps = 1e12/(config.refclk_divisions()*config.refclk_freq);
	for (std::size_t i = 0; i < 4; i++){
		Meas meas;
		meas.status = Meas::Valid;
		meas.lsb_ps = lsb_ps;
		meas.refclk_freq = config.refclk_freq;

		meas.ref_index = 0U;
		meas.stop_result = 0U;

		meas.ref_index |= (static_cast<uint32_t>(readout[i*6U]) << 16U);
		meas.ref_index |= (static_cast<uint32_t>(readout[i*6U+1U]) << 8U);
		meas.ref_index |= static_cast<uint32_t>(readout[i*6U+2U]);

		meas.stop_result |= (static_cast<uint32_t>(readout[i*6U+3U]) << 16U);
		meas.stop_result |= (static_cast<uint32_t>(readout[i*6U+4U]) << 8U);
		meas.stop_result |= static_cast<uint32_t>(readout[i*6U+5U]);

		measurements.push_back(meas);
	}
}