#include "hal/Rtc.hpp"

using namespace hal;

Rtc::Rtc(port_t port) : Periph(CORE_PERIPH_RTC, port){

}


int Rtc::set_time(const rtc_time_t & time) const {
	return ioctl(
				arg::IoRequest(I_RTC_SET),
				arg::IoConstArgument(&time)
				);
}

int Rtc::get_time(rtc_time_t & time) const {
	return ioctl(
				arg::IoRequest(I_RTC_GET),
				arg::IoArgument(&time)
				);
}
