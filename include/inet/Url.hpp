/*! \file */ // Copyright 2011-2020 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md for rights.

#ifndef SAPI_INET_URL_HPP_
#define SAPI_INET_URL_HPP_

#include "../api/InetObject.hpp"
#include "../var/String.hpp"

namespace inet {

class Url : public api::InfoObject {
public:
	enum protocol {
		protocol_https,
		protocol_http
	};

	Url(const var::String & url = "");

	int set(const var::String & url);
	var::String to_string() const;

	u16 port() const { return m_port; }
	u8 protocol() const { return m_protocol; }
	const var::String & domain_name() const { return m_domain_name; }
	const var::String & path() const { return m_path; }

	static var::String encode(const var::String & input);
	static var::String decode(const var::String & input);

private:
	/*! \cond */
	var::String m_domain_name;
	var::String m_path;
	u8 m_protocol;
	u16 m_port;
	/*! \endcond */


};

}

#endif // SAPI_INET_URL_HPP_
