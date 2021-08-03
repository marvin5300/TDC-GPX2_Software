#include "spidevices/gpx2/gpx2.h"
#include "spidevices/gpx2/config.h"
#include <iostream>
#include <iomanip>
#include <cmath>

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
	std::string conf_str = std::string({ static_cast<char>(data) });
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
	std::string readout = read(spiopc_read_results | 0x08U, 24);
	/*for (unsigned i = 0; i < 24; i++) {
		std::cout << std::setw(2) << std::dec << i << ": " << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned>(readout[i]) << std::endl;
	}*/
	/*uint32_t test_ref_index = 0U;
	uint32_t test_stop_result = 0U; 
	test_ref_index = (static_cast<uint32_t>(readout[0]) << 16U);
	test_ref_index |= (static_cast<uint32_t>(readout[1]) << 8U);
	test_ref_index |= static_cast<uint32_t>(readout[2]);
	test_stop_result = (static_cast<uint32_t>(readout[3]) << 16U);
	test_stop_result |= (static_cast<uint32_t>(readout[4]) << 8U);
	test_stop_result |= static_cast<uint32_t>(readout[5]);
	std::cout << "first 6 byte: " <<std::dec<< test_ref_index << "  " << test_stop_result << std::endl;;*/

	std::vector<Meas> measurements{};
	if (readout.empty()) {
		return measurements;
	}
	double lsb_ps = 1e12 / (config.refclk_divisions() * config.refclk_freq);
	for (std::size_t i = 0; i < 4; i++) {
		Meas meas;
		meas.status = Meas::Valid;
		meas.lsb_ps = lsb_ps;
		meas.refclk_freq = config.refclk_freq;

		meas.ref_index = 0U;
		meas.stop_result = 0U;

		meas.ref_index = (static_cast<uint32_t>(readout[i * 6U]) << 16U);
		meas.ref_index |= (static_cast<uint32_t>(readout[i * 6U + 1U]) << 8U);
		meas.ref_index |= static_cast<uint32_t>(readout[i * 6U + 2U]);

		meas.stop_result = (static_cast<uint32_t>(readout[i * 6U + 3U]) << 16U);
		meas.stop_result |= (static_cast<uint32_t>(readout[i * 6U + 4U]) << 8U);
		meas.stop_result |= static_cast<uint32_t>(readout[i * 6U + 5U]);

		if (meas.ref_index == 0xffffffU && meas.stop_result == 0xffffffU) {
			meas.status = Meas::Invalid;
		}
		//std::cout << std::setw(2) << std::setfill('0') << std::dec << "meas_stop_" << i << ": ref_index=" << meas.ref_index << " stop_result=" << meas.stop_result << std::endl; 
		//<< " refclk_freq=" <<meas.refclk_freq << " lsb_ps=" << meas.lsb_ps << std::endl;
		measurements.push_back(meas);
	}
	//std::cout << std::endl;
	return measurements;
}

auto GPX2::get_filtered_intervals(double max_interval) -> std::vector<double>{
	//std::cout << "gpio " << event.pin << " falling edge" << std::endl;
	std::vector<double> diffs{};
	std::vector<std::vector<SPI::GPX2_TDC::Meas> > readout;
	for (int i = 0; i < 4; i++) {
		readout.push_back(read_results());
		if (readout.size() < 4) {
			std::cerr << "readout vector has less than 4 channels";
			continue;
		}
	}
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			double some_readout = 0;
			SPI::GPX2_TDC::Meas val0;
			SPI::GPX2_TDC::Meas val1;
			val0 = readout[i][j];
			val1 = readout[i][(j + 1) % 4];
			if (val0.status == SPI::GPX2_TDC::Meas::Invalid || val1.status == SPI::GPX2_TDC::Meas::Invalid) {
				continue;
			}

			some_readout = diff(val0, val1); // combinations are possible for 1+2, 2+3, 3+4, 1

			//std::cout << "first loop: i=" << i << " j="<< j << "  " << some_readout << std::endl;

			if (j % 2 == 1) {
				some_readout = -some_readout;
			}
			if (fabs(some_readout) < max_interval) {
				diffs.push_back(some_readout);
			}
		}
	}
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 4; j++) {
			double some_readout = 0;
			SPI::GPX2_TDC::Meas val0;
			SPI::GPX2_TDC::Meas val1;
			val0 = readout[i][j];
			val1 = readout[i + 1][(j + 1) % 4];
			if (val0.status == SPI::GPX2_TDC::Meas::Invalid || val1.status == SPI::GPX2_TDC::Meas::Invalid) {
				continue;
			}

			some_readout = diff(val0, val1); // combinations are possible for 1+2, 2+3, 3+4, 1+4
			//std::cout << "second loop: i=" << i << "j=" << j << "  " << some_readout << std::endl;

			if (j % 2 == 1) {
				some_readout = -some_readout;
			}
			if (fabs(some_readout) < max_interval) {
				diffs.push_back(some_readout);
			}

			val0 = readout[i+1][j];
			val1 = readout[i][(j + 1) % 4];
			some_readout = diff(readout[i + 1][j], readout[i][(j + 1) % 4]); // combinations are possible for 1+2, 2+3, 3+4, 1+4

			//std::cout << "second loop: i=" << i << "j=" << j << "  " << some_readout << std::endl;

			if (j % 2 == 1) {
				some_readout = -some_readout;
			}

			if (fabs(some_readout) < max_interval) {
				diffs.push_back(some_readout);
			}
		}
	}
	//std::cout << std::endl;
	return diffs;
}


Meas::operator bool()
{
	return status != Invalid;
}
bool Meas::operator < (const Meas& other) const
{
	if (ref_index < other.ref_index) {
		return true;
	}
	if (ref_index == other.ref_index) {
		return stop_result < other.stop_result;
	}
	return false;
}
bool Meas::operator > (const Meas& other) const
{
	if (ref_index > other.ref_index) {
		return true;
	}
	if (ref_index == other.ref_index) {
		return stop_result > other.stop_result;
	}
	return false;
}
auto Meas::operator == (const Meas& other) const -> bool
{
	return (ref_index == other.ref_index && stop_result == other.stop_result);
}
auto Meas::operator != (const Meas& other) const -> bool
{
	return !(*this == other);
}
auto Meas::operator >= (const Meas& other) const -> bool
{
	return (*this == other) || (*this > other);
}
auto Meas::operator <= (const Meas& other) const  -> bool
{
	return (*this == other) || (*this < other);
}