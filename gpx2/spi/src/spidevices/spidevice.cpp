#include "spidevices/spidevice.h"
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <memory>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

using namespace SPI;

unsigned int spiDevice::fNrDevices{ 0 };
unsigned long int spiDevice::fGlobalNrBytesWritten{ 0 };
unsigned long int spiDevice::fGlobalNrBytesRead{ 0 };
std::vector<spiDevice*> spiDevice::fGlobalDeviceList;

spiDevice::spiDevice() {
}

spiDevice::~spiDevice() {
	if (fHandle > 0) {
		fNrDevices--;
	}
	close(fHandle);
	std::vector<spiDevice*>::iterator it;
	it = std::find(fGlobalDeviceList.begin(), fGlobalDeviceList.end(), this);
	if (it != fGlobalDeviceList.end()) {
		fGlobalDeviceList.erase(it);
	}
}

bool spiDevice::init(std::string busAddress) {
	fHandle = open(busAddress.c_str(), O_RDWR);
	fMode = SPI_MODE_2;
	if (fHandle < 0) {
		std::cerr << "Could not open spi device" << std::endl;
		return false;
	}

	fNrDevices++;
	fGlobalDeviceList.push_back(this);

	/*
	 * spi mode
	*/
	int ret{};
	ret = ioctl(fHandle, SPI_IOC_WR_MODE, &fMode);
	if (ret == -1) {
		std::cerr << "can't set spi mode" << std::endl;
		return false;
	}
	ret = ioctl(fHandle, SPI_IOC_RD_MODE, &fMode);
	if (ret == -1) {
		std::cerr << "can't get spi mode" << std::endl;
	}


	/*
	 * bits per word
	*/
	const std::uint8_t bits = 8;
	ret = ioctl(fHandle, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1) {
		std::cerr << "can't set bits per word" << std::endl;
	}

	ret = ioctl(fHandle, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1) {
		std::cerr << "can't get bits per word" << std::endl;
		return false;
	}

	/*
	 * max speed hz
	 */
	ret = ioctl(fHandle, SPI_IOC_WR_MAX_SPEED_HZ, &fSpeed);
	if (ret == -1) {
		std::cerr << "can't set max speed hz" << std::endl;
	}

	ret = ioctl(fHandle, SPI_IOC_RD_MAX_SPEED_HZ, &fSpeed);
	if (ret == -1) {
		std::cerr << "can't get max speed hz" << std::endl;
	}

	return true;
}

bool spiDevice::devicePresent() {
	return false;
}

bool spiDevice::write(const std::uint8_t command, const std::string& data) {
	if (fHandle==-1) {
		std::cerr << "tried to write to spi without initialising." << std::endl;
		return false;
	}
	const std::size_t n = data.size() + 1;

	auto txBuf{ std::make_unique<std::uint8_t[]>(n) };
	auto rxBuf{ std::make_unique<std::uint8_t[]>(n) };
	txBuf[0] = command;
	for (std::size_t i = 1; i < n; i++) {
		txBuf[i] = data[i - 1];
	}

	auto status = spi_xfer(fHandle, fSpeed, fNrBits, txBuf.get(), rxBuf.get(), n);
	std::cout << "spi_xfer returned " << status << std::endl;
	return status;
}

std::string spiDevice::read(const std::uint8_t command, const std::size_t nBytes) {
	if (fHandle == -1) {
		std::cerr << "tried to read from spi without initialising." << std::endl;
		return "";
	}
	const std::size_t n = nBytes + 1;

	auto txBuf{ std::make_unique<std::uint8_t[]>(n) };
	auto rxBuf{ std::make_unique<std::uint8_t[]>(n) };
	txBuf[0] = command;

	auto status = spi_xfer(fHandle, fSpeed, fNrBits, txBuf.get(), rxBuf.get(), n);
	std::cout << "spi_xfer returned " << status << std::endl;
	std::string data;
	for (std::size_t i = 1; i < n; i++) {
		data += rxBuf[i];
	}
	return data;
}


int spiDevice::spi_xfer(const int handle, const uint32_t speed, const uint8_t bits, uint8_t* tx, uint8_t* rx, uint32_t nBytes)
{
	int ret{};
	uint16_t delay{};
	spi_ioc_transfer tr = {
		/*
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.len = nBytes,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
		*/
		(unsigned long)tx, // tx_buf
		(unsigned long)rx, // rx_buf
		nBytes, // len
		speed, // speed_hz
		delay, // delay_usecs
		bits, // bits_per_word
		0, // cs_change
		8, // rx_nbits
		8, // tx_nbits
		SPI_MODE_2 // mode
		//0 // cs

		/** 
		* Note:
		* Depending on spidev version there are different parameters in struct
		*/
	};

	ret = ioctl(handle, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1) {
		std::cerr << "can't send spi message" << std::endl;
	}
	return ret;
}