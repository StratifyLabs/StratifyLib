/*! \file */ // Copyright 2011-2020 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md for rights.

#ifndef SAPI_HAL_SPI_HPP_
#define SAPI_HAL_SPI_HPP_

#include <sos/dev/spi.h>
#include "PinAssignment.hpp"
#include "Periph.hpp"


namespace hal {

class SpiFlags {
public:
	enum flags {
		flag_is_format_spi/*! See \ref SPI_FLAG_IS_FORMAT_SPI */ = SPI_FLAG_IS_FORMAT_SPI,
		flag_is_format_ti /*! See \ref SPI_FLAG_IS_FORMAT_TI */ = SPI_FLAG_IS_FORMAT_TI,
		flag_is_format_microwire /*! See \ref SPI_FLAG_IS_FORMAT_MICROWIRE */ = SPI_FLAG_IS_FORMAT_MICROWIRE,
		flag_is_mode0 /*! See \ref SPI_FLAG_IS_MODE0 */ = SPI_FLAG_IS_MODE0,
		flag_is_mode1 /*! See \ref SPI_FLAG_IS_MODE1 */ = SPI_FLAG_IS_MODE1,
		flag_is_mode2 /*! See \ref SPI_FLAG_IS_MODE2 */ = SPI_FLAG_IS_MODE2,
		flag_is_mode3 /*! See \ref SPI_FLAG_IS_MODE3 */ = SPI_FLAG_IS_MODE3,
		flag_set_master /*! See \ref SPI_FLAG_SET_MASTER */ = SPI_FLAG_SET_MASTER,
		flag_set_slave /*! See \ref SPI_FLAG_SET_SLAVE */ = SPI_FLAG_SET_SLAVE,
		flag_is_full_duplex /*! See \ref SPI_FLAG_IS_FULL_DUPLEX */ = SPI_FLAG_IS_FULL_DUPLEX,
		flag_set_full_duplex /*! See \ref SPI_FLAG_IS_FULL_DUPLEX */ = SPI_FLAG_SET_FULL_DUPLEX,
		flag_is_half_duplex /*! See \ref SPI_FLAG_IS_HALF_DUPLEX */ = SPI_FLAG_IS_HALF_DUPLEX,
		flag_set_half_duplex /*! See \ref SPI_FLAG_IS_FULL_DUPLEX */ = SPI_FLAG_SET_HALF_DUPLEX,
	};

};

API_OR_FLAGS_OPERATOR(SpiFlags)


/*! \brief SPI Pin Assignment
 * \details This class allows simple manipulation of the spi_pin_assignment_t.
 *
 */
class SpiPinAssignment : public PinAssignment<spi_pin_assignment_t>{};


class SpiAttributes : public PinAssignmentPeriphAttributes<spi_attr_t, spi_pin_assignment_t>, public SpiFlags {
public:

	SpiAttributes(){
		set_flags(
					flag_set_master |
					flag_is_format_spi |
					flag_is_mode0 |
					flag_is_half_duplex
					);
		set_frequency(1000000);
		set_width(8);
	}

	SpiAttributes & set_miso(const mcu_pin_t & pin){ m_attr.pin_assignment.miso = pin; return *this; }
	SpiAttributes & set_mosi(const mcu_pin_t & pin){ m_attr.pin_assignment.mosi = pin; return *this; }
	SpiAttributes & set_sck(const mcu_pin_t & pin){ m_attr.pin_assignment.sck = pin; return *this; }
	SpiAttributes & set_cs(const mcu_pin_t & pin){ m_attr.pin_assignment.cs = pin; return *this; }
	SpiAttributes & set_width(u8 value){ m_attr.width = value; return *this; }
	SpiAttributes & set_frequency(u32 value){ PeriphAttributes::set_frequency(value); return *this; }
	SpiAttributes & set_flags(enum flags value){ PeriphAttributes::set_flags(value); return *this; }

	mcu_pin_t miso() const { return m_attr.pin_assignment.miso; }
	mcu_pin_t mosi() const { return m_attr.pin_assignment.mosi; }
	mcu_pin_t sck() const { return m_attr.pin_assignment.sck; }
	mcu_pin_t cs() const { return m_attr.pin_assignment.cs; }
	u8 width() const { return m_attr.width; }

private:


};

typedef SpiAttributes SpiAttr;

class SpiInfo {
public:
	SpiInfo(){ memset(&m_info, 0, sizeof(spi_info_t)); }
	SpiInfo(const spi_info_t & info){ m_info = info; }

	bool is_valid() const { return m_info.o_flags != 0; }
	u32 o_flags() const { return m_info.o_flags; }
	u32 o_events() const { return m_info.o_events; }


private:
	spi_info_t m_info;
};


/*! \brief SPI Class
 * \details This class gives access to a SPI port.
 *
 * \code
 * #include <sapi/hal.hpp>
 *
 * int main(int argc, char * argv[]){
 *   char buffer[16];
 *   Spi spi(0);           //access to SPI port 0
 *   spi.init();           //init SPI with default settings from BSP
 *   spi.read(buffer, 16); //read 16 bytes from the SPI
 *   spi.close();          //close the SPI (power it off)
 *   return 0;
 * }
 * \endcode
 */
class Spi : public Periph<spi_info_t, spi_attr_t, SpiAttributes, 's'>, public SpiFlags {
public:

	using SourceBuffer = var::Reference::SourceBuffer;
	using DestinationBuffer = var::Reference::DestinationBuffer;

	/*! \details Constructs a SPI object using \a port. */
	Spi(port_t port);

	/*! \details swap a byte on the SPI bus */
	int swap(int byte) const;

	int set_attr(u32 o_flags, u32 freq, u32 width = 8, const spi_pin_assignment_t * pin_assignment = 0) const {
		spi_attr_t attr;
		attr.o_flags = o_flags;
		attr.freq = freq;
		if( pin_assignment != 0 ){
			memcpy(&attr.pin_assignment, pin_assignment, sizeof(spi_pin_assignment_t));
		} else {
			memset(&attr.pin_assignment, 0xff, sizeof(spi_pin_assignment_t));
		}
		attr.width = width;
		return set_attributes(attr);
	}

	int initialize(u32 o_flags, u32 freq, u32 width = 8, const spi_pin_assignment_t * pin_assignment = 0){
		if( open() < 0 ){
			return -1;
		}
		return set_attr(o_flags, freq, width, pin_assignment);
	}
	int init(u32 o_flags, u32 freq, u32 width = 8, const spi_pin_assignment_t * pin_assignment = 0){ return initialize(o_flags, freq, width, pin_assignment); }

	using Periph::initialize;
	using Periph::get_info;
	SpiInfo get_info() const;


#if !defined __link
	int transfer(
			SourceBuffer write_data,
			DestinationBuffer read_data,
			Size nbytes
			);
#endif

private:

};

}

#endif /* SAPI_HAL_SPI_HPP_ */
