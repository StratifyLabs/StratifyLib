/*! \file */ // Copyright 2011-2020 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md for rights.

#include <errno.h>
#include "crypto/Aes.hpp"
#include "crypto/Random.hpp"
#include "sys/Printer.hpp"

using namespace crypto;

Aes::~Aes(){
	m_initialization_vector.fill(0);
	finalize();
}

int Aes::initialize(){
	if( aes_api().is_valid() == false ){
		return set_error_number_if_error(api::error_code_crypto_missing_api);
	}
	finalize();
	int result = 0;
	if( aes_api()->init(&m_context) < 0 ){
		result = api::error_code_crypto_operation_failed;
	}
	return set_error_number_if_error(result);
}

int Aes::finalize(){
	if( m_context != nullptr ){
		aes_api()->deinit(&m_context);
	}
	return 0;
}

Aes & Aes::set_initialization_vector(
		const var::Reference & value
		){

	if( value.count<u8>() != m_initialization_vector.count() ){
		errno = EINVAL;
		set_error_number_if_error(api::error_code_crypto_bad_iv_size);
		return *this;
	}

	for(u32 i=0; i < m_initialization_vector.count(); i++){
		m_initialization_vector.at(i) = value.to_const_u8()[i];
	}

	return *this;
}

Aes & Aes::set_key(
		const var::Reference & key
		){
	set_error_number_if_error(
				aes_api()->set_key(
					m_context,
					key.to_const_u8(),
					key.size() * 8,
					8
					)
				);
	return *this;
}


int Aes::encrypt_ecb(
		SourcePlainData source_data,
		DestinationCipherData destination_data
		){
	if( source_data.argument().size() >
			destination_data.argument().size() ){
		return set_error_number_if_error(api::error_code_crypto_size_mismatch);
	}

	if( source_data.argument().size() % 16 != 0 ){
		return set_error_number_if_error(api::error_code_crypto_bad_block_size);
	}

	for(
			u32 i=0;
			i < source_data.argument().size();
			i+=16
			){
		if( aes_api()->encrypt_ecb(
					m_context,
					source_data.argument().to_const_u8() + i,
					destination_data.argument().to_u8() + i
					) < 0 ){
			return -1;
		}
	}
	return source_data.argument().size();

}

int Aes::decrypt_ecb(
		SourceCipherData source_data,
		DestinationPlainData destination_data
		){

	if( source_data.argument().size() >
			destination_data.argument().size() ){
		return set_error_number_if_error(api::error_code_crypto_size_mismatch);
	}

	if( source_data.argument().size() % 16 != 0 ){
		return set_error_number_if_error(
					api::error_code_crypto_bad_block_size
					);
	}

	for(u32 i=0; i < source_data.argument().size(); i+=16){
		if( aes_api()->decrypt_ecb(
					m_context,
					source_data.argument().to_const_u8() + i,
					destination_data.argument().to_u8() + i
					) < 0 ){
			return set_error_number_if_error(
						api::error_code_crypto_operation_failed
						);
		}
	}

	return set_error_number_if_error(
				source_data.argument().size()
				);
}

int Aes::encrypt_cbc(
		SourcePlainData source_data,
		DestinationCipherData destination_data
		){

	if( source_data.argument().size() >
			destination_data.argument().size() ){
		return set_error_number_if_error(api::error_code_crypto_size_mismatch);

	}

	if( source_data.argument().size() % 16 != 0 ){
		return set_error_number_if_error(api::error_code_crypto_bad_block_size);
	}

	int result;
	if( (result = aes_api()->encrypt_cbc(
				 m_context,
				 source_data.argument().size(),
				 m_initialization_vector.data(), //init vector
				 source_data.argument().to_const_u8(),
				 destination_data.argument().to_u8()
				 ) < 0)
			){
		result = api::error_code_crypto_operation_failed;
	}

	return set_error_number_if_error(result);
}

int Aes::decrypt_cbc(
		SourceCipherData source_data,
		DestinationPlainData destination_data
		){

	if( source_data.argument().size() >
			destination_data.argument().size() ){
		return set_error_number_if_error(api::error_code_crypto_size_mismatch);
	}

	if( source_data.argument().size() % 16 != 0 ){
		return set_error_number_if_error(api::error_code_crypto_bad_block_size);
	}

	int result;
	if( (result = aes_api()->decrypt_cbc(
				 m_context,
				 source_data.argument().size(),
				 m_initialization_vector.data(), //init vector
				 source_data.argument().to_const_u8(),
				 destination_data.argument().to_u8()
				 )) < 0 ){
		result = api::error_code_crypto_operation_failed;
	}

	return set_error_number_if_error(result);
}

int Aes::encrypt_ctr(
		SourcePlainData source_data,
		DestinationCipherData destination_data
		){
	return set_error_number_if_error(
				api::error_code_crypto_unsupported_operation
				);
}

int Aes::decrypt_ctr(
		SourceCipherData source_data,
		DestinationPlainData destination_data
		){
	return set_error_number_if_error(
				api::error_code_crypto_unsupported_operation
				);
}


Aes::CbcCipherData Aes::get_cbc_cipher_data(const var::Blob & key, const var::Blob & source){
	Aes::CbcCipherData result;
	Aes aes;
	aes.initialize();
	aes.set_key(key);
	aes.set_initialization_vector( Random::get_data(16) );

	result.set_initialization_vector( aes.initialization_vector() );
	result.data().resize( ((source.size() + 15) / 16) * 16 );

	size_t first_blob_size = (source.size() / 16) * 16;
	size_t second_blob_size = result.data().size() - first_blob_size;

	if( first_blob_size ){
		//alinged to 16
		var::Blob plain_data = var::Blob(
					var::Blob::ReadOnlyBuffer(source.to_const_u8()),
					var::Blob::Size(first_blob_size));

		var::Blob cipher_data = var::Blob(
					static_cast<void*>(result.data().to_u8()),
					var::Blob::Size(first_blob_size));

		aes.encrypt_cbc(SourcePlainData(plain_data),
										DestinationCipherData(cipher_data)
										);
	}

	if( second_blob_size ){
		var::Blob plain_data = var::Blob(
					var::Blob::ReadOnlyBuffer(source.to_const_u8() + first_blob_size),
					var::Blob::Size(second_blob_size));

		var::Blob cipher_data = var::Blob(
					static_cast<void*>(result.data().to_u8() + first_blob_size),
					var::Reference::Size(second_blob_size));

		aes.encrypt_cbc(SourcePlainData(plain_data),
										DestinationCipherData(cipher_data)
										);
	}


	return result;
}

var::Data Aes::get_plain_data(const var::Blob & key, const Aes::CbcCipherData & source){
	var::Data result;

	Aes aes;
	aes.initialize();

	aes.set_key(key);
	aes.set_initialization_vector( source.initialization_vector() );
	result.resize( source.data().size() );

	aes.decrypt_cbc(SourceCipherData(source.data()),
									DestinationPlainData(result)
									);

	return result;
}

