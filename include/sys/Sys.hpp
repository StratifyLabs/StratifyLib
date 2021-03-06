/*! \file */ // Copyright 2011-2020 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md for rights.

#ifndef SAPI_SYS_SYS_HPP_
#define SAPI_SYS_SYS_HPP_

#if !defined __link
#include <sos/sos.h>
#endif

#include <sos/dev/sys.h>
#include <sos/link.h>
#include "../fs/File.hpp"
#include "../var/ConstString.hpp"
#include "Appfs.hpp"

namespace sys {

class Sys;


/*! \brief Serial Number class
 * \details The SerialNumber class holds a value for an
 * MCU serial number.
 *
 * Stratify OS supports reading the serial number directly
 * from the chip. This class makes doing so as simply as possible.
 *
 */
class SerialNumber {
public:

	/*! \details Constructs an empty serial number. */
	SerialNumber();

	/*! \details Constructs a serial number for an array of u32 values. */
	explicit SerialNumber(const u32 serial_number[4]){ memcpy(m_serial_number.sn, serial_number, sizeof(u32)*4); }

	/*! \details Constructs a serial number from an mcu_sn_t. */
	explicit SerialNumber(const mcu_sn_t serial_number) : m_serial_number(serial_number){}

	/*! \details Constructs this serial number from \a str. */
	explicit SerialNumber(const var::String & str);

	/*! \details Returns true if a valid serial number is held. */
	bool is_valid() const {
		return at(0) + at(1) + at(2) + at(3) != 0;
	}

	/*! \details Returns a serial number object from a string type. */
	static SerialNumber from_string(const var::String & str);

	/*! \details Returns the u32 section of the serial number specified by *idx*. */
	u32 at(u32 idx) const {
		if( idx >= 4 ){
			idx = 3;
		}
		return m_serial_number.sn[idx];
	}



	/*! \details Compares this strig to \a serial_number. */
	bool operator == (const SerialNumber & serial_number);

	/*! \details Converts the serial number to a string. */
	var::String to_string() const;

private:
	mcu_sn_t m_serial_number;
};

/*! \brief System Information Class
 * \details This class holds the system information.
 *
 *
 * \code
 * #include <sapi/sys.hpp>
 * SysInfo info = SysInfo::get(); //grab system information
 * Printer p;
 * p << info; //print system information using the printer
 * \endcode
 *
 */
class SysInfo {
	friend class Sys;
public:

	/*! \details Constructs an empty SysInfo object. */
	SysInfo(){ clear(); }

	/*! \details Constructs an object from *info*. */
	explicit SysInfo(const sys_info_t & info ) : m_info(info) {}

	operator const sys_info_t & () const { return m_info; }

	/*! \details Returns true if the object is valid. */
	bool is_valid() const { return cpu_frequency() != 0; }

	//use Sys::get_info()
	static SysInfo get();

	/*! \details Returns the OS package project ID. */
	var::String id() const { return m_info.id; }
	/*! \details Returns the team ID of the system. */
	var::String team_id() const { return m_info.team_id; }
	/*! \details Returns the name of the system. */
	var::String name() const { return m_info.name; }
	/*! \details Returns the system version. */
	var::String system_version() const { return m_info.sys_version; }
	/*! \details Returns the board support package version (same as system_version()). */
	var::String bsp_version() const { return m_info.sys_version; }
	/*! \details Returns the Stratify OS version version. */
	var::String sos_version() const { return m_info.kernel_version; }
	/*! \details Returns the kernel version (same as sos_version()). */
	var::String kernel_version() const { return m_info.kernel_version; }

	/*! \details Returns the CPU architecture.
	 *
	 * Applications that are installed on the system must
	 * have a compatible architecture.
	 *
	 *
	 */
	var::String cpu_architecture() const { return m_info.arch; }

	/*! \details Returns the CPU core clock frequency in Hertz. */
	u32 cpu_frequency() const { return m_info.cpu_freq; }

	/*! \details Returns the signature for the system call table.
	 *
	 * Applications that are installed must have a compatible signature.
	 *
	 */
	u32 application_signature() const { return m_info.signature; }

	/*! \details Returns the GIT Hash of the OS package as built. */
	var::String bsp_git_hash() const { return m_info.bsp_git_hash; }
	/*! \details Returns the Stratify OS library used to link the OS package. */
	var::String sos_git_hash() const { return m_info.sos_git_hash; }
	/*! \details Returns the Stratify OS MCU library used to link the OS package. */
	var::String mcu_git_hash() const { return m_info.mcu_git_hash; }

	u32 o_flags() const { return m_info.o_flags; }

	var::String arch() const { return m_info.arch; }
	var::String stdin_name() const { return m_info.stdin_name; }
	var::String stdout_name() const { return m_info.stdout_name; }
	var::String trace_name() const { return m_info.trace_name; }
	u32 hardware_id() const { return m_info.hardware_id; }


	SerialNumber serial_number() const { return SerialNumber(m_info.serial); }

	void clear(){
		memset(&m_info, 0, sizeof(m_info));
	}



private:
	sys_info_t m_info;


};

class LaunchOptions {
	API_ACCESS_COMPOUND(LaunchOptions,var::String,path);
	API_ACCESS_COMPOUND(LaunchOptions,var::String,arguments);
	API_ACCESS_COMPOUND(LaunchOptions,var::String,environment);
	API_ACCESS_FUNDAMENTAL(LaunchOptions,enum Appfs::flags,application_flags,Appfs::flag_is_default);
	API_ACCESS_FUNDAMENTAL(LaunchOptions,int,ram_size,0);
};

/*! \brief Sys Class
 * \details This class allows access to system attributes and functions.
 */
class Sys : public fs::File {
public:

	using Arguments = arg::Argument<const var::String &, struct SysArgumentsTag>;
	using Environment = arg::Argument<const var::String &, struct SysEnvironmentTag>;
	using DestinationPath = arg::Argument<var::String &, struct SysEnvironmentTag>;
	using KernelRequest = arg::Argument<int, struct SysKernelRequest>;
	using KernelArgument = arg::Argument<void*, struct SysKernelArgument>;

	Sys(
			SAPI_LINK_DRIVER_NULLPTR
			);



	/*! \details Launches a new application.
	 *
	 * @param path The path to the application
	 * @param args The arguments to pass to the applications
	 * @param options For example:  LAUNCH_OPTIONS_FLASH | LAUNCH_OPTIONS_STARTUP
	 * @param ram_size The amount of RAM that will be used by the app
	 * @return The process ID of the new app if successful
	 *
	 * This method must be called locally in an app. It can't be executed over the link protocol.
	 */
	static int launch(
			const var::String & path,
			Arguments args,
			enum Appfs::flags options = Appfs::flag_is_default, //run in RAM, discard on exit
			int ram_size = 0
			);

	/*! \details Launches a new application.
	 *
	 * @param path The path to the application
	 * @param exec_dest A pointer to a buffer where the execution path will be written (null if not needed)
	 * @param args The arguments to pass to the applications
	 * @param options For example:  LAUNCH_OPTIONS_FLASH | LAUNCH_OPTIONS_STARTUP
	 * @param ram_size The amount of RAM that will be used by the app (0 assumes default value)
	 * @param update_progress A callback to show the progress if the app needs to be installed (copied to flash/RAM)
	 * @param envp Not used (leave as default empty string)
	 * @return The process ID of the new app if successful
	 *
	 */
	static int launch(
			const var::String & path,
			Arguments args,
			DestinationPath exec_destination,
			enum Appfs::flags options, //run in RAM, discard on exit
			int ram_size,
			const sys::ProgressCallback * progress_callback,
			Environment envp = Environment("")
			);

	static var::String launch(
			const LaunchOptions & options,
			const sys::ProgressCallback * progress_callback
			);

	/*!
	 * \details Installs an application from the data filesystem
	 * to the application filesystem.
	 *
	 * \param path The path to the source application to install
	 * \param options The installation flag options
	 * \param ram_size The number of bytes to allow for the application's data section
	 *
	 * \return The full path of the install location.
	 */
	static var::String install(
			const var::String & path,
			enum Appfs::flags options = Appfs::flag_is_default, //run in RAM, discard on exit
			int ram_size = 0
			);

	/*!
	 * \details Installs an application from the data filesystem
	 * to the application filesystem.
	 *
	 * \param path The path to the source file.
	 * \param options The installation options.
	 * \param ram_size The number of bytes to allow for the application's data section.
	 * \param progress_callback A pointer to a callback to indicate the installation progress.
	 * \return
	 */
	static var::String install(
			const var::String & path,
			enum Appfs::flags options, //run in RAM, discard on exit
			int ram_size,
			const sys::ProgressCallback * progress_callback
			);



	/*! \details Frees the RAM associated with the app without deleting the code from flash
	 * (should not be called when the app is currently running).
	 *
	 * @param path The path to the app (use \a exec_dest from launch())
	 * @param driver Used with link protocol only
	 * @return Zero on success
	 *
	 * This method can causes problems if not used correctly. The RAM associated with
	 * the app will be free and available for other applications. Any applications
	 * that are using the RAM must quit before the RAM can be reclaimed using reclaim_ram().
	 *
	 * \sa reclaim_ram()
	 */
	static int free_ram(
			const var::String & path
		#if defined __link
			SAPI_LINK_DRIVER_NULLPTR_LAST
		#endif
			);

	/*! \details Reclaims RAM that was freed using free_ram().
	 *
	 * @param path The path to the app
	 * @param driver Used with link protocol only
	 * @return Zero on success
	 *
	 * \sa free_ram()
	 */
	static int reclaim_ram(
			const var::String & path
			SAPI_LINK_DRIVER_NULLPTR_LAST
			);

#if !defined __link

	/*! \details Gets the version (system/board version).
	 *
	 * @param version The destination string for the version
	 * @return Zero on success
	 */
	static var::String get_version();
	/*! \cond */
	static int get_version(var::String & version);
	/*! \endcond */

	/*! \details Gets the version (kernel version).
	 *
	 * @param version The destination string for the version
	 * @return Zero on success
	 */
	static var::String get_kernel_version();

	/*! \details Puts the kernel in powerdown mode.
	 *
	 * @param timeout_msec The number of milliseconds before the
	 * device will power on (reset).  If this isn't supported,
	 * the device will power off until reset by an external signal
	 */
	static void powerdown(const chrono::MicroTime& duration = chrono::Milliseconds(0)
			);

	/*! \details Puts the kernel in hibernate mode.
	 *
	 * @param timeout_msec The number of milliseconds before the
	 * device will wake up from hibernation.  If this isn't supported,
	 * the device will stay in hibernation until woken up externally
	 */
	static int hibernate(
			const chrono::MicroTime& duration = chrono::Milliseconds(0)
			);

	/*! \details Executes a kernel request.
	 *
	 * @param req The request number
	 * @param arg Argument pointer
	 * @return The result of the execution of the request. (-1 if request is not available)
	 *
	 * The kernel request must
	 * be defined and implemented by the board support package.
	 */
	static int request(
			KernelRequest req,
			KernelArgument argument = KernelArgument(nullptr)
			);

	/*! \details Request a kernel install library's API.
	 *
	 * @param request The API request value (ie SAPI_API_REQUEST_ARM_DSP)
	 * @return A pointer to the api_t.
	 *
	 * This method will request a library API from the kernel. If
	 * the library is install in the kernel. This method will return
	 * a pointer to the api. Otherwise, zero will be retured. It is
	 * possible for the application to link directly to the library
	 * if the library is not provided by the kernel.
	 *
	 */
	template<typename T> static const T * request_api(int request){
		return (const T*)::kernel_request_api(request);
	}

	/*! \details Forces a reset of the device. */
	static void reset();

	/*! \details Loads the board configuration provided as
	 * part of the board support package.
	 *
	 * @param config A reference to the destination object
	 * @return Zero on success
	 *
	 * The object must be opened before calling this method.
	 *
	 * \sa open()
	 */
	int get_board_config(sos_board_config_t & config);


#endif

	/*! \details Loads the current system info.
	 *
	 *
	 */
	static SysInfo get_info(
			SAPI_LINK_DRIVER_NULLPTR
			);

	static bool is_authenticated(
			SAPI_LINK_DRIVER_NULLPTR
			);

	static sys_secret_key_t get_secret_key(
			SAPI_LINK_DRIVER_NULLPTR
			);



	static SerialNumber get_serial_number();

	/*! \details Opens /dev/sys.
	 *
		* @return Less than zero for an error
	 *
	 */
	int open(){
		return fs::File::open(
					"/dev/sys",
					fs::OpenFlags::read_write()
					);
	}

	/*! \details Loads the current system info.
	 *
	 * @param attr A reference to where the data should be stored
	 * @return Zero on success
	 *
	 * The object must be opened before calling this method.
	 *
	 * \sa open()
	 *
	 */
	int get_info(sys_info_t & attr);

	/*! \cond */
	using File::open;
	int get_23_info(sys_23_info_t & attr);
	int get_26_info(sys_26_info_t & attr);
	/*! \endcond */

	/*! \details Loads the cloud kernel ID.
	 *
	 * @param id A reference to the destination data
	 * @return Less than zero if the operation failed
	 *
	 * The object must be opened before calling this method.
	 *
	 */
	int get_id(sys_id_t & id);

#if !defined __link
	/*! \details Redirects the standard output to the file specified.
	 *
	 * @param fd The file descriptor where the standard output should be directed.
	 *
	 * The file desriptor should be open and ready for writing. For example,
	 * to redirect the standard output to the UART:
	 *
	 * \code
	 * #include <sapi/sys.hpp>
	 * #include <sapi/hal.hpp>
	 *
	 * Uart uart(0);
	 * uart.init(); //initializes uart using default settings (if available)
	 * Sys::redirect_stdout( uart.fileno() );
	 * printf("This will be written to UART0\n");
	 * \endcode
	 *
	 *
	 */
	static void redirect_stdout(int fd){
		_impure_ptr->_stdout->_file = fd;
	}

	/*! \details Redirects the standard input from the specified file descriptor.
	 *
	 * @param fd The open and readable file descriptor to use for standard input
	 *
	 * See Sys::redirect_stdout() for an example.
	 *
	 */
	static void redirect_stdin(int fd){
		_impure_ptr->_stdin->_file = fd;
	}

	/*! \details Redirects the standard error from the specified file descriptor.
	 *
	 * @param fd The open and writable file descriptor to use for standard input
	 *
	 * See Sys::redirect_stdout() for an example.
	 *
	 */
	static void redirect_stderr(int fd){
		_impure_ptr->_stderr->_file = fd;
	}
#endif


private:

};

class Printer;
Printer & operator << (Printer& printer, const sys::SysInfo & a);
class TraceEvent;
Printer & operator << (Printer& printer, const TraceEvent & a);
}

#endif /* SAPI_SYS_SYS_HPP_ */
