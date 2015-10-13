/* The class should have a zero argument constructor
 * and use some sort of efficient data structure
 * to store a list of messages that have been seen
 *
 * It should only keep a certain number of messages,
 * then start kicking out old ones to make room
 * for new ones. This should be doable with some
 * added public methods.
 *
 */

#include <state.h>

using std::vector;

namespace libmeshenger
{
	bool
	PacketEngine::compareIds(uint8_t * a, uint8_t * b)
	{
		for(int i = 0; i < 16; i++) {
			if (a[i] != b[i])
				return false;
		}
		return true;
	}

	bool
	PacketEngine::IsPacketNew(Packet &p)
	{
		if (p.type() != Packet::CLEARMESSAGE_TYPE)
			return false;

		ClearMessage m(p);

		for(int i = 0; i < seenMessages.size(); i++) {
			if (compareIds(seenMessages[i], m.id().data()))
				return false;
		}

		return true;
	}

	PacketEngine::PacketEngine()
	{
		seenMessages = vector<uint8_t[16]>();
		callbacks = vector<void (*)(ClearMessage&)>();
	}

	void
	PacketEngine::AddCallback(void (*cb)(ClearMessage&))
	{
		callbacks.push_back(cb);
	}

	void
	PacketEngine::ProcessPacket(Packet &p)
	{
		if (IsPacketNew(p)) {
			ClearMessage m(p);
			for(int i = 0; i < callbacks.size(); i++) {
				callbacks[i](m);
			}
		}
	}
}
