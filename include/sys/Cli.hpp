/*! \file */ // Copyright 2011-2020 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md for rights.

#ifndef SAPI_SYS_CLI_HPP_
#define SAPI_SYS_CLI_HPP_

#include "../api/SysObject.hpp"
#include "../hal/Uart.hpp"
#include "../hal/I2C.hpp"
#include "../hal/Pwm.hpp"
#include "../hal/Adc.hpp"
#include "../hal/Dac.hpp"
#include "../var/Vector.hpp"
#include "../var/String.hpp"
#include "../var/Token.hpp"
#include "../var/ConstString.hpp"

namespace sys {


/*! \brief Command Line Interface class
 * \details This class contains methods to help analyze input from the
 * command line.
 */
class Cli : public api::WorkObject {
public:


	using Description = arg::Argument<const var::String &, struct CliDescriptionTag>;

	enum format {
		format_text,
		format_json,
		format_total
	};

	/*! \details Constructs a new object to parse and handle command line interface arguments.
	 *
	 * @param argc The number of arguments
	 * @param argv A pointer to the arguments
	 *
	 * The Cli is construted as shown in the following example.
	 * \code
	 * int main(int argc, char * argv[]){
	 * 	Cli cli(argc, argv);
	 * }
	 * \endcode
	 *
	 */
	Cli(int argc,
		 char * argv[],
		 const char * app_git_hash = nullptr
			);

	/*! \details Handles the --version and -v options to show the version.
	 *
	 * \sa print_version()
	 *
	 */
	void handle_version() const;

	/*! \details Sets the publisher of the program.
	 *
	 * @param publisher A pointer to the publisher's name
	 *
	 */
	void set_publisher(
			const var::String & publisher
			){
		m_publisher = publisher;
	}


	/*! \details Sets whether the arguments are case sensitive. */
	void set_case_sensitive(bool value = true){
		m_is_case_sensitive = value;
	}

	/*!
	  * \details Returns true if parsing is case sensitive.
	  *
	  * By default, parsing is case senstive unless set_case_sensitive(false)
	  * is called.
	  *
	  */
	bool is_case_senstive() const { return m_is_case_sensitive; }

	enum {
		PRINT_DEBUG,
		PRINT_INFO,
		PRINT_WARNING,
		PRINT_ERROR,
		PRINT_FATAL,
		PRINT_TOTAL
	};

	/*! \details Accesses the program version. */
	const var::String & version() const { return m_version; }

	/*! \details Accesses the program publisher. */
	const var::String publisher() const { return m_publisher; }

	/*! \details Accesses the program name. */
	const var::String name() const { return m_name; }

	/*! \details Accesses the path to the program (including the name). */
	const var::String path() const { return m_path; }

	/*! \details Accesses the path to the program (including the name). */
	const var::String app_git_hash() const { return m_app_git_hash; }


	var::String to_string() const;


	/*! \details Returns the argument offset by value as a var::String. */
	var::String at(u16 value) const;

	/*! \details Returns the argument offset by value as a mcu_pin_t value. */
	mcu_pin_t pin_at(u16 value) const;

	/*! \details Return the argument offset by value as an int value */
	int value_at(u16 value) const;

	/*! \details Checks to see if the option exists as a command line argument.
	 *
	 * @param value A pointer to the option string to search for
	 * @return True if value is any of the options available
	 *
	 * For example,
	 *
	 * > program -v -i text.txt
	 *
	 * `is_option("-v")` will return true.
	 *
	 */
	bool is_option(const var::String & value) const;


	/*! \details Checks to see if the option exists and returns its value
	 *  as a String.
	 *
	 * @param name The option name
	 *
	 * This method will work with the following notations
	 *
	 * ```
	 * -<name>
	 * -<name> <value>
	 * -<name>=<value>
	 * --<name>
	 * --<name> <value>
	 * --<name>=<value>
	 * ```
	 *
	 * If `-<name>` or `--<name>` is given, the return value is set to "true".
	 *
	 */
	var::String get_option(
			const var::String & name,
			Description help = Description(var::String())
			) const;

	/*! \details Gets the argument of an option as a var::String.
	 *
	 * @param option The option to match
	 * @return A String containing the option argument
	 *
	 * For example, take the given command line
	 *
	 * > program -i filename.txt
	 *
	 * `get_option_argument("-i")` will return a String containing "filename.txt"
	 *
	 *
	 */
	var::String get_option_argument(const char * option) const;

	/*! \details Gets the argument of an option as a var::String.
	 *
	 * @param option The option to match
	 * @return The value of the argument or 0 if the option wasn't found
	 *
	 * For example, take the given command line
	 *
	 * > program -i 10
	 *
	 * `get_option_value("-i")` will return 10.
	 *
	 *
	 */
	int get_option_value(const char * option) const;

	/*! \details Gets the argument of an option as a var::String.
	 *
	 * @param option The option to match
	 * @return The value of the argument or 0 if the option wasn't found
	 *
	 * For example, take the given command line
	 *
	 * > program -i 0x40
	 *
	 * `get_option_hex_value("-i")` will return 0x40 (64 decimal).
	 *
	 *
	 */
	int get_option_hex_value(const char * option) const;


	/*! \details Gets the argument of an option as a var::String.
	 *
	 * @param option The option to match
	 * @return The value of the argument or 0 if the option wasn't found
	  *SOS_GIT_HASH="${SOS_GIT_HASH}"
	 * For example, take the given command line
	 *
	 * > program -i 2.1
	 *
	 * `get_option_pio("-i")` will return a mcu_pin_t structure with port = 2 and pin = 1.
	 *
	 *
	 */
	mcu_pin_t get_option_pin(const char * option) const;

	/*! \details Returns the number of arguments. */
	u32 count() const { return m_argc; }
	u32 size() const { return m_argc; }

	//handling hardware inputs
	/*! \details Handles arguments for setting UART attributes.
	 *
	 * @param attr A reference to the destination attributes
	 * @return true if UART attributes were parsed
	 *
	 * The arguments are
	 * - "-uart [port]" (required)
	 * - "-freq [bitrate]" (optional, default is 115200)
	 * - "-width [byte width]" (optional, default is 8)
	 * - "-stop1" (optional, default is 1 stop bit)
	 * - "-stop2" (optional)
	 * - "-tx [X.Y]" (optional port.pin, uses system default otherwise)
	 * - "-rx [X.Y]" (optional port.pin, uses system default otherwise)
	 * - "-rts [X.Y]" (optional port.pin, uses system default otherwise)
	 * - "-cts [X.Y]" (optional port.pin, uses system default otherwise)
	 *
	 */
	bool handle_uart(hal::UartAttributes & attr) const;

	/*! \details Handles arguments for setting I2C attributes.
	 *
	 * @param attr A reference to the destination attributes
	 * @return true if I2C attributes were parsed
	 *
	 * The arguments are
	 * - "-i2c [port]" (required)
	 * - "-freq [bitrate]" (optional, default is 100000)
	 * - "-slave" (optional, default is 0)
	 * - "-scl [X.Y]" (optional port.pin, uses system default otherwise)
	 * - "-sda [X.Y]" (optional port.pin, uses system default otherwise)
	 * - "-pu" (optional flag, use internal pullup resistors)
	 *
	 */
	bool handle_i2c(hal::I2CAttr & attr) const;
	var::String get_version_details() const;

	void show_options() const;


private:

	bool is_option_equivalent_to_argument(const var::String & option, const var::String & argument) const;
	bool is_option_equivalent_to_argument_with_equality(const var::String & option, const var::String & argument, var::String & value) const;

	bool compare_with_prefix(const var::String & option, const var::String & argument) const;

	u16 m_argc;
	char ** m_argv;
	var::String m_version;
	var::String m_publisher;
	var::String m_name;
	var::String m_path;
	bool m_is_case_sensitive;
	const char * m_app_git_hash;
	mutable var::Vector<var::String> m_help_list;


};

Printer & operator << (Printer& printer, const Cli & a);


} /* namespace sys */

#endif /* SAPI_SYS_CLI_HPP_ */
