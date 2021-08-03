#include "spidevices/gpx2/config.h"
#include <cstdint>
#include <string>

using namespace SPI::GPX2_TDC;

// default config defined by the author of this program
Config::Config()
	: PIN_ENA_RSTIDX{ 0 }
	, PIN_ENA_DISABLE{ 0 }
	, PIN_ENA_LVDS_OUT{ 0 }
	, PIN_ENA_REFCLK{ 0 }
	, PIN_ENA_STOP4{ 0 }
	, PIN_ENA_STOP3{ 0 }
	, PIN_ENA_STOP2{ 0 }
	, PIN_ENA_STOP1{ 0 } // reg_addr 0
	, HIGH_RESOLUTION{ 0 }
	, CHANNEL_COMBINE{ 0 }
	, HIT_ENA_STOP4{ 0 }
	, HIT_ENA_STOP3{ 0 }
	, HIT_ENA_STOP2{ 0 }
	, HIT_ENA_STOP1{ 0 } // reg_addr 1
	, BLOCKWISE_FIFO_READ{ 0 }
	, COMMON_FIFO_READ{ 0 }
	, LVS_DOUBLE_DATA_RATE{ 0 }
	, STOP_DATA_BITWIDTH{ 0 }
	, REF_INDEX_BITWIDTH{ 0 } // reg_addr 2
	, REFCLK_DIVISIONS_LOWER{ 0 } // reg_addr 3
	, REFCLK_DIVISIONS_MIDDLE{ 0 } // reg_addr 4
	, REFCLK_DIVISIONS_UPPER{ 0 } // reg_addr 5
	, LVDS_TEST_PATTERN{ 0 } // reg_addr 6
	, REFCLK_BY_XOSC{ 0 }
	, LVDS_DATA_VALID_ADJUST{ 0 } // reg_addr 7
	, CMOS_INPUT{ 0 } // reg_addr 16
{}

void Config::loadDefaultConfig() {
	// reg_addr 0
	PIN_ENA_DISABLE = 0;
	PIN_ENA_RSTIDX = 0;
	PIN_ENA_LVDS_OUT = 0;
	PIN_ENA_REFCLK = 0;
	PIN_ENA_STOP4 = 1;
	PIN_ENA_STOP3 = 1;
	PIN_ENA_STOP2 = 1;
	PIN_ENA_STOP1 = 1;

	// reg_addr 1
	//constexpr std::uint8_t high_resolution_mode_0 = 0;
	constexpr std::uint8_t high_resolution_mode_2 = 2;
	//constexpr std::uint8_t channel_combine_none = 0;
	//constexpr std::uint8_t channel_combine_pulsewidth = 2;
	constexpr std::uint8_t channel_combine_pulsedistance = 1;
	HIGH_RESOLUTION = high_resolution_mode_2;
	CHANNEL_COMBINE = channel_combine_pulsedistance;
	HIT_ENA_STOP4 = 1;
	HIT_ENA_STOP3 = 1;
	HIT_ENA_STOP2 = 1;
	HIT_ENA_STOP1 = 1;

	// reg_addr 2
	constexpr std::uint8_t stop_data_bitwidth_3 = 0b11;
	constexpr std::uint8_t ref_index_bitwidth_2 = 0b010;
	BLOCKWISE_FIFO_READ = 1;
	COMMON_FIFO_READ = 1;
	LVS_DOUBLE_DATA_RATE = 0;
	STOP_DATA_BITWIDTH = stop_data_bitwidth_3;
	REF_INDEX_BITWIDTH = ref_index_bitwidth_2;

	// reg_addr 3
	constexpr std::uint8_t refclk_div_200k_lower = 0x40;
	REFCLK_DIVISIONS_LOWER = refclk_div_200k_lower;	// 5MHz clk => T = 200ns = 200,000ps

	// reg_addr 4
	constexpr std::uint8_t refclk_div_200k_middle = 0x0d;
	REFCLK_DIVISIONS_MIDDLE = refclk_div_200k_middle;	// set REFCLK_DIVISIONS to 200,000 for LSB to be 1ps

	// reg_addr 5
	constexpr std::uint8_t refclk_div_200k_upper = 0x03;
	REFCLK_DIVISIONS_UPPER = refclk_div_200k_upper; // 200,000 = 0x030d40

	// reg_addr 6
	LVDS_TEST_PATTERN = 0;

	// reg_addr 7
	REFCLK_BY_XOSC = 1; // use external quartz, ignore external clock => 1
	LVDS_DATA_VALID_ADJUST = 1;

	// reg_addr 16
	CMOS_INPUT = 1;
}

// pack bits in the correct order and return a string.
auto Config::str() const->std::string {
	std::string data;
	uint8_t byte;

	byte = static_cast<uint8_t>(PIN_ENA_RSTIDX << 7U);
	byte |= static_cast<uint8_t>(PIN_ENA_DISABLE << 6U);
	byte |= static_cast<uint8_t>(PIN_ENA_LVDS_OUT << 5U);
	byte |= static_cast<uint8_t>(PIN_ENA_REFCLK << 4U);
	byte |= static_cast<uint8_t>(PIN_ENA_STOP4 << 3U);
	byte |= static_cast<uint8_t>(PIN_ENA_STOP3 << 2U);
	byte |= static_cast<uint8_t>(PIN_ENA_STOP2 << 1U);
	byte |= static_cast<uint8_t>(PIN_ENA_STOP1);

	data += static_cast<char>(byte);

	byte = static_cast<uint8_t>(HIGH_RESOLUTION << 6U);
	byte |= static_cast<uint8_t>(CHANNEL_COMBINE << 4U);
	byte |= static_cast<uint8_t>(HIT_ENA_STOP4 << 3U);
	byte |= static_cast<uint8_t>(HIT_ENA_STOP3 << 2U);
	byte |= static_cast<uint8_t>(HIT_ENA_STOP2 << 1U);
	byte |= static_cast<uint8_t>(HIT_ENA_STOP1);

	data += static_cast<char>(byte);

	byte = static_cast<uint8_t>(BLOCKWISE_FIFO_READ << 7U);
	byte |= static_cast<uint8_t>(COMMON_FIFO_READ << 6U);
	byte |= static_cast<uint8_t>(LVS_DOUBLE_DATA_RATE << 5U);
	byte |= static_cast<uint8_t>(STOP_DATA_BITWIDTH << 3U);
	byte |= static_cast<uint8_t>(REF_INDEX_BITWIDTH);

	data += static_cast<char>(byte);

	data += static_cast<uint8_t>(REFCLK_DIVISIONS_LOWER);
	data += static_cast<uint8_t>(REFCLK_DIVISIONS_MIDDLE);
	data += static_cast<uint8_t>(REFCLK_DIVISIONS_UPPER);

	byte = static_cast<uint8_t>(0b110U << 5U);
	byte |= static_cast<uint8_t>(LVDS_TEST_PATTERN << 4U);
	data += static_cast<char>(byte);

	byte = static_cast<uint8_t>(REFCLK_BY_XOSC << 8U);
	byte |= static_cast<uint8_t>(0b1U << 7U);
	byte |= static_cast<uint8_t>(LVDS_DATA_VALID_ADJUST << 4U);
	byte |= static_cast<uint8_t>(0b0011U);
	data += static_cast<char>(byte);

	constexpr uint8_t reg_default_08 = 0b10100001U;
	constexpr uint8_t reg_default_09 = 0b00010011U;
	constexpr uint8_t reg_default_10 = 0b00000000U;
	constexpr uint8_t reg_default_11 = 0b00001010U;
	constexpr uint8_t reg_default_12 = 0b11001100U;
	constexpr uint8_t reg_default_13 = 0b11001100U;
	constexpr uint8_t reg_default_14 = 0b11110001U;
	constexpr uint8_t reg_default_15 = 0b01111101U;

	data += static_cast<char>(reg_default_08);
	data += static_cast<char>(reg_default_09);
	data += static_cast<char>(reg_default_10);
	data += static_cast<char>(reg_default_11);
	data += static_cast<char>(reg_default_12);
	data += static_cast<char>(reg_default_13);
	data += static_cast<char>(reg_default_14);
	data += static_cast<char>(reg_default_15);

	byte = ((CMOS_INPUT) << 2U);
	data += static_cast<char>(byte);
	return data;
}

uint32_t Config::refclk_divisions(){
	uint32_t dat{};
	dat |= (static_cast<uint32_t>(REFCLK_DIVISIONS_UPPER) << 16U);
	dat |= (static_cast<uint32_t>(REFCLK_DIVISIONS_MIDDLE) << 8U);
	dat |= static_cast<uint32_t>(REFCLK_DIVISIONS_LOWER);
	return dat;
}