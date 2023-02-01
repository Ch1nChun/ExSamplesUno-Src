#pragma once

#include "hwid\hwid.h"
#include "TlsClient\TlsClient.h"
#include "TlsClient\crypt_data.h"
#include <jwt\jwt.hpp>
#include <string>
#include "m_xor_str.h"
#include <Windows.h>

key m_key;
using namespace jwt::params;

void crash_log(const char* text, ...)
{
	if (!text)
		return;

	char path[MAX_PATH];
	IFH(GetEnvironmentVariable)(m_xor_str("USERPROFILE"), path, MAX_PATH);

	strcat(path, m_xor_str("\\Desktop\\legendware.log"));
	auto file = std::ofstream(path, std::ios::app);

	if (!file.is_open())
		return;

	va_list va_alist;
	char buffer[1024] = "\0";

	va_start(va_alist, text);
	_vsnprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), text, va_alist); //-V2007
	va_end(va_alist);

	file << buffer << std::endl;
	file.close();
}

__forceinline nlohmann::json decrypt_data(const std::string& data)
{
	std::error_code ec;
	auto dec_obj = jwt::decode(data, algorithms({ m_xor_str("RS256") }), ec, secret(m_key.pub_s), verify(true));

	return dec_obj.payload().create_json_obj();
}

__forceinline std::string sing_data(std::map<std::string, std::string> data)
{
	jwt::jwt_object obj =
	{
		algorithm({ m_xor_str("RS256") }),
		secret(m_key.priv),
		payload(data)
	};

	std::error_code ec;
	return obj.signature(ec);
}

__forceinline std::vector <std::string>& split(const std::string& s, char delim, std::vector<std::string>& elems)
{
	std::stringstream ss(s);
	std::string item;

	while (std::getline(ss, item, delim))
		elems.push_back(item);

	return elems;
}

__forceinline std::vector <std::string> split(const std::string& s, char delim)
{
	std::vector <std::string> elems;
	split(s, delim, elems);
	return elems;
}

__forceinline std::string get_data(const std::string& key)
{
	auto tls_client = new TlsClient(m_xor_str("api.legendware.pw"), m_xor_str("1339"));
	m_key = get_token();

	if (tls_client->connect())
	{
		auto key = tls_client->send(m_xor_str("qewrwqertdfszvdzfdg"));
		m_key.pub_s = key;
	}
	else
		return m_xor_str("1");

	auto m_hwid = new hwid();
	std::map <std::string, std::string> data;

	std::string m_token;
	std::string m_uid_str;

	auto login_data = split(key, ':');

	m_token = login_data[0];
	m_uid_str = login_data[1];

	data[m_xor_str("type")] = m_xor_str("login");
	data[m_xor_str("token")] = m_token;
	data[m_xor_str("hwid")] = m_hwid->getHWID();
	data[m_xor_str("uid")] = std::to_string(atoi(m_uid_str.c_str()));

	auto done = false;

	for (auto i = 0; i < 40; ++i)
	{
		if (!tls_client->is_send)
		{
			done = true;

			if (tls_client->connect())
			{
				auto singed_data = sing_data(data);
				auto data_out = tls_client->send(m_key.pub + ':' + singed_data);
				auto json_data = decrypt_data(data_out);

				if (json_data[std::string(m_xor_str("error"))].get <std::string>().size() > 2)
				{
					data.clear();
					return m_xor_str("9");
				}

				g_ctx.username = json_data[std::string(m_xor_str("username"))].get <std::string>();
				data.clear();

				done = false;

				for (auto i = 0; i < 40; ++i)
				{
					if (!tls_client->is_send)
					{
						done = true;

						data[m_xor_str("type")] = m_xor_str("get_signature");
						data[m_xor_str("version")] = BETA ? m_xor_str("beta") : m_xor_str("stable");

						if (tls_client->connect())
						{
							auto singed_data = sing_data(data);
							auto data_out = tls_client->send(m_key.pub + ':' + singed_data);

							data.clear();
							return data_out;
						}

						return m_xor_str("8");
					}

					std::this_thread::sleep_for(std::chrono::milliseconds(500));
				}

				if (!done)
					return m_xor_str("7");
			}

			if (!done)
				return m_xor_str("6");

			break;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}

	if (!done)
		return m_xor_str("5");

	data.clear();

	delete tls_client;
	delete m_hwid;

	return m_xor_str("4");
}