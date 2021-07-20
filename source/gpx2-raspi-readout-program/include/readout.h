#ifndef READOUT_H
#define READOUT_H

#include "gpx2.h"
#include "gpio.h"
#include <future>
#include <thread>
#include <iostream>
#include <iomanip>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <chrono>

inline static void print_hex(const std::string& str) {
	for (auto byte : str) {
		std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(byte) << " ";
	}
	std::cout << std::endl;
}

class Readout {
public:
	Readout(double max_ref_diff = 100e-9, unsigned interrupt_pin = 20);
	~Readout();

	void stop();

private:
	[[nodiscard]] auto setup()->int;
	[[nodiscard]] auto read_tdc()->int;

	[[nodiscard]] auto process_loop()->int;

	void process_queue(bool ignore_max_queue = false);

	std::unique_ptr<gpio> handler{};
	std::shared_ptr<gpio::callback> callback{};
	const std::chrono::milliseconds process_loop_timeout{ std::chrono::milliseconds(100) };
	size_t max_queue_size{ 500 };
	size_t min_queue_size{ 5 };
	const std::chrono::microseconds readout_loop_timeout{ std::chrono::microseconds(1) };
	double m_max_interval{};
	unsigned m_interrupt_pin{};
	std::chrono::time_point<std::chrono::high_resolution_clock> start_time{};
	std::chrono::time_point<std::chrono::high_resolution_clock> end_time{};
	std::condition_variable queue_condition;
	std::array<std::queue<SPI::GPX2_TDC::Meas>,2> tdc_stop{};
	std::unique_ptr<SPI::GPX2_TDC::GPX2> gpx2{};
	std::atomic<uint64_t> evt_count{};
	std::atomic<bool> m_run{ true };
	std::thread gpio_thread;
	std::thread analysis_thread;
};
#endif // READOUT_H
