#include <iostream>
#include <algorithm>
#include <cstdint>

#include <parser.h>

using namespace std;

namespace libmeshenger
{
	static bool
	validateClearMessage(vector<uint8_t> msg);

	/* Statics */
	static bool
	validateClearMessage(vector<uint8_t> body)
	{
		if (body.size() < 32)
		{
			return false;
		}
		return true;
	}

	/* Global functions */
	bool
	ValidatePacket(vector<uint8_t> msg)
	{
		/* Validate minimum size */
		if (msg.size() < 8)
			return false;

		/* Validate magic */
		if (msg[0] != 'I' || msg[1] != 'M')
			return false;

		/* Validate version (always 1 for now) */
		if (msg[2] != 1)
			return false;

		/* Get body length */
		uint16_t bodyLength = msg[7];
		bodyLength |= (msg[6] << 8);

		/* Validate body length */
		if (msg.size() != bodyLength + 8)
			return false;

		/* Return true if valid empty packet */
		if (msg.size() == 8 && msg[5] == 0x00)
			return true;

		/* Create body vector */
		vector<uint8_t> body(msg.begin() + 8, msg.end());

		/* Validate message body based on body type */
		switch (msg[5]) {
			case 0x01:
				return validateClearMessage(body);
				break;
			default:
				return false;
		}
	}

	bool
	operator==(const Packet& lhs, const Packet& rhs)
	{
		if (lhs.length() != rhs.length())
			return false;

		if (lhs.type() != rhs.type())
			return false;

		switch (lhs.type()) {
			case 0x00: return true;
					   break;

			case 0x01: return equal(lhs.body().begin(), lhs.body().end(),
							   		rhs.body().begin());
					   break;

			default: return false;
		}
	}

	/* Class methods */
	Packet::Packet(vector<uint8_t> data)
	: raw_m(data)
	{
		if (ValidatePacket(data) == false)
			throw InvalidPacketException();

		type_m = data[5]; /* Store type */
	}

	Packet::Packet(ClearMessage &m)
	{
		/* Magic, version, res, type (ClearMessage), length (blank) */
		uint8_t base_preamble[] = { 'I', 'M', 1, 0, 0, 1, 0, 0 };
		vector<uint8_t> preamble(base_preamble, base_preamble + 8);
		
		/* Length */
		preamble[7] = (m.length() + 16) % 256;
		preamble[6] = (m.length() + 16) / 256;

		vector<uint8_t> id = m.id();
		vector<uint8_t> body = m.body();

		/* Build */
		raw_m = vector<uint8_t>();
		raw_m.insert(raw_m.end(), preamble.begin(), preamble.end());
		raw_m.insert(raw_m.end(), id.begin(), id.end());
		raw_m.insert(raw_m.end(), body.begin(), body.end());

		if (ValidatePacket(raw_m) == false)
			throw InvalidPacketException();

		type_m = raw_m[5];
	}

	uint16_t
	Packet::length() const
	{
		return body().size();
	}

	vector<uint8_t>
	Packet::body() const
	{
		/* Return (by value) a copy of the body */
		return vector<uint8_t>(raw_m.begin() + 8, raw_m.end());
	}

	vector<uint8_t>
	Packet::raw() const
	{
		return vector<uint8_t>(raw_m);
	}

	uint8_t
	Packet::type() const
	{
		return type_m;
	}

	/* Exception definitions */

	InvalidPacketException::InvalidPacketException(string const& error)
		: runtime_error(error)
	{}

	InvalidPacketException::InvalidPacketException()
		: runtime_error("Invalid packet!")
	{}

	WrongPacketTypeException::WrongPacketTypeException(string const& error)
		: runtime_error(error)
	{}

	WrongPacketTypeException::WrongPacketTypeException()
		: runtime_error("Packet type mismatch!")
	{}
}