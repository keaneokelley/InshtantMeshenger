#ifndef __STATE_H
#define __STATE_H

#include <vector>
#include <cstdint>
#include <parser.h>

namespace libmeshenger
{
	class PacketEngine
	{
		private:
		   	std::vector<std::vector<uint8_t>> seenMessages;
			std::vector<void (*)(Packet&)> callbacks;
			bool compareIds(uint8_t *, uint8_t *);
			uint32_t packets_processed;
		public:
			PacketEngine();
			void ProcessPacket(Packet&);
			void AddCallback(void (*)(Packet&));
			bool IsPacketNew(Packet &);
			uint32_t PacketsProcessed();
	};
}
#endif
