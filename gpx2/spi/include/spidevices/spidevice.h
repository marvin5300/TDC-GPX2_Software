#ifndef SPI_DEVICE_H
#define SPI_DEVICE_H

#include <string>
#include <vector>

namespace SPI {
	enum class Mode {
		spi_mode_0,
		spi_mode_1,
		spi_mode_2,
		spi_mode_3
	};

	class spiDevice {
	public:
		spiDevice();
		~spiDevice();

		auto init(std::string busAddress = "/dev/spidev0.0", std::uint32_t speed = 61035, Mode mode = Mode::spi_mode_0, uint8_t bits = 8)->bool;

		/**
		* Writes to physical device
		* @param command
		* @param data
		* @return
		*/
		auto write(const std::uint8_t command, const std::string& data)->bool;

		/**
		* Reads from physical device
		* @param command
		* @param nBytes
		* @return
		*/
		auto read(const std::uint8_t command, const std::size_t nBytes)->std::string;

		/**
		* Writes one byte to a register
		* @param regAddr
		* @param byte
		* @return
		*/
		//virtual auto writeReg(const std::uint8_t regAddr, const std::uint8_t byte)->bool = 0;

		/**
		* Reads one byte from a register
		* @param regAddr
		* @param byte
		* @return
		*/
		//virtual auto readReg(const std::uint8_t regAddr, std::uint8_t& byte)->bool = 0;

		virtual auto devicePresent()->bool;

	protected:
		static auto spi_xfer(const int handle, const uint32_t speed, const uint8_t mode, const uint8_t bits, uint8_t* tx, uint8_t* rx, std::size_t nBytes)->int;

		int fHandle{ -1 };
		unsigned long int fNrBytesWritten{ 0 };
		unsigned long int fNrBytesRead{ 0 };
		static unsigned long int fGlobalNrBytesWritten;
		static unsigned long int fGlobalNrBytesRead;
		static std::vector<spiDevice*> fGlobalDeviceList;
		static unsigned int fNrDevices;

		std::uint8_t fAddress{};
		std::uint8_t fMode{};
		std::uint8_t fNrBits{};
		std::uint32_t fSpeed{};
	};
}
#endif // SPI_DEVICE_H