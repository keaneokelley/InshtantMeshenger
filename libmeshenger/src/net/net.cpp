#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <cstdlib>
#include <cstdint>
#include <vector>
#include <string>

#include <parser.h>
#include <net.h>
#include <parser.h>

#define NET_DEBUG true

using boost::asio::ip::udp;
using namespace std;

const int32_t MAX_LENGTH = 1024;
const uint8_t RESP[] = "meshenger-discovery-reply";
const uint8_t MSG[] = "meshenger-discovery-probe";

namespace libmeshenger
{
	void
	netVerbosePrint(string s, int color)
	{
		if (NET_DEBUG)
			cout << "\033[1;31m[libmeshenger-net]\033[0m-> " <<
				"\033[1;" << color << "m" << s << "\033[0m" << endl;
	}

	/* Net class methods */

	/* Default constructor. Initializes io_service */
	Net::Net(uint16_t udp_port,
			uint16_t tcp_port)
		: udp_port(udp_port),
		io_service(),
		tcp_port(tcp_port),
		/* Initialize UDP listen socket on all interfaces */
		listen_socket(io_service, udp::endpoint(udp::v4(), udp_port))
	{
	}

	void
	Net::run() {
		io_service.run();
	}

	void
	Net::discoveryListen()
	{
		/* Handle any incoming connections asynchronously */
		netVerbosePrint("Starting discovery listener", 33);
		listen_socket.async_receive_from(
			boost::asio::buffer(data, MAX_LENGTH), remote_endpoint,
			/* Bind connection to acceptDiscoveryConn method */
			boost::bind(&Net::acceptDiscoveryConn, this,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
	}

	void
	Net::acceptDiscoveryConn(const boost::system::error_code& error, size_t recv_len)
	{
		/* Bind handler for new connections. */

		/* Check if we received a discovery packet and if it is from a new peer */
		if (error) netVerbosePrint("ERROR", 33);
		if (recv_len > 0) {
			remote_endpoint.port(udp_port);
			udp::socket socket(io_service, udp::endpoint(udp::v4(), 0));
			socket.set_option(udp::socket::reuse_address(true));
		   	if (!strcmp((char*) data, (char*) MSG)) {
				netVerbosePrint("Received discovery probe from peer " +
						remote_endpoint.address().to_string());
				socket.async_send_to(
					boost::asio::buffer(RESP, strlen((char*) RESP) + 1), remote_endpoint,
					boost::bind(&Net::handleDiscoveryReply, this,
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred));
				if (!peerExistsByAddress(remote_endpoint.address())) {
					peers.insert(peers.end(), Peer(remote_endpoint.address()));
					netVerbosePrint("Found new peer at " +
							remote_endpoint.address().to_string(), 32);
				return;
				}
			} else if (!strcmp((char*) data, (char*) RESP)) {
				netVerbosePrint("Received discovery reply from peer " +
						remote_endpoint.address().to_string());
				socket.async_send_to(
					boost::asio::buffer("", 0), remote_endpoint,
					boost::bind(&Net::handleDiscoveryReply, this,
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred));
				if (!peerExistsByAddress(remote_endpoint.address())) {
					peers.insert(peers.end(), Peer(remote_endpoint.address()));
					netVerbosePrint("Found new peer at " +
							remote_endpoint.address().to_string(), 32);
				return;
				}
			} else
				netVerbosePrint("Received invalid probe", 30);
		}
		listen_socket.async_receive_from(
			boost::asio::buffer(data, MAX_LENGTH), remote_endpoint,
			boost::bind(&Net::acceptDiscoveryConn, this,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
	}

	/* Not the best way to do this */
	bool
	Net::peerExistsByAddress(boost::asio::ip::address ip_addr)
	{
		for(auto &peer : peers)
			if (peer.ip_addr == ip_addr) return true;
		return false;
	}

	void
	Net::handleDiscoveryReply(const boost::system::error_code& error, size_t send_len)
	{
		/* Simply a bind handler */
		if (error) netVerbosePrint("ERROR", 31);
		netVerbosePrint("Sending discovery reply to " + remote_endpoint.address().to_string(), 33);
		listen_socket.async_receive_from(
			boost::asio::buffer(data, MAX_LENGTH), remote_endpoint,
			boost::bind(&Net::acceptDiscoveryConn, this,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
	}

	void
	Net::discoverPeers()
	{
		/* Discover peers on the LAN using UDP broadcast */

		netVerbosePrint("Sending discovery probe", 33);

		/* Create the socket that will send UDP broadcast */
		udp::socket socket(io_service, udp::endpoint(udp::v4(), 0));

		/* Set socket options so that it can use the host's address and send
		* broadcast */
		socket.set_option(udp::socket::reuse_address(true));
		socket.set_option(udp::socket::broadcast(true));

		/* Create endpoint for the connections */
		udp::endpoint endpoint(boost::asio::ip::address_v4::broadcast(), udp_port);

		/* Send discovery packet */
		socket.send_to(boost::asio::buffer(MSG, strlen((char*) MSG) + 1), endpoint);

		/*
		size_t recv_length = socket.receive_from(boost::asio::buffer(data, MAX_LENGTH), endpoint);
		if (!strcmp((char*) data, (char*) RESP)) {
			netVerbosePrint("Received discovery reply from " + endpoint.address().to_string());
			boost::asio::ip::address addr = endpoint.address();
			if (!peerExistsByAddress(addr)) {
				peers.insert(peers.end(), Peer(addr));
				netVerbosePrint("Found new peer at " +
						addr.to_string(), 32);
			}
		}
		*/
	}

	Packet
	Net::getPacket()
	{
		Packet p = packets.back();
		packets.pop_back();
		return p;
	}

	void
	Net::addPeer(Peer p)
	{
		peers.push_back(p);
	}

	void
	Net::addPeer(std::string s)
	{
		peers.push_back(Peer(s));
	}

	std::vector<Peer>
	Net::getPeers()
	{
		return std::vector<Peer>(peers.begin(), peers.end());
	}

	/* Sends a Packet to all previously discovered peers using TCP */
	void
	Net::sendToAllPeers(Packet p)
	{
		// Should probably have addr reuse in here
		/* Cycle through the peers vector and prepare to send */
		for(int i = 0; i < peers.size(); i++) {
			/* Peer IP address */
			boost::asio::ip::address addr = peers[i].ip_addr;

			/* Endpoint for the peer */
			boost::asio::ip::tcp::endpoint endpoint(addr, tcp_port);

			/* Socket used to create the connection */
			boost::asio::ip::tcp::socket sock(io_service);

			/* Try to connect and report any connection errors */
			try {
				sock.connect(endpoint);
				/* Send the data */
				sock.send(boost::asio::buffer(p.raw().data(), p.raw().size()));
				peers[i].strikes = 0;
			} catch(std::exception &e) {
				/* Handle connection errors */
				netVerbosePrint(e.what(), 33);
				netVerbosePrint("Peer " + addr.to_string() +
						" is problematic. Strike.", 33);
				peers[i].strikes++;

				/* Remove peer if it fails to be reached 3 times */
				if (peers[i].strikes >= 3) {
					netVerbosePrint("Three strikes. Removing.");
					peers.erase(peers.begin() + i);
				}
			}
		}
	}

	/* Accept TCP connections on tcp_port and attempt to create a valid packet
	 * if possible. Adds packet to the packets vector and returns the length
	 * of the packet. */
	uint16_t
	Net::receivePacket()
	{
	    try {
			/* Attempt to bind to tcp_port */
			// Should probably have addr reuse in here
			boost::asio::ip::tcp::acceptor acceptor(io_service,
					boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),
					tcp_port));

			boost::asio::ip::tcp::socket socket(io_service);

			/* Begin accepting connections */
			acceptor.accept(socket);

			boost::asio::streambuf sb;
			boost::system::error_code ec;
			uint8_t b[MAX_LENGTH];

			/* Read from socket into buffer */
			size_t bytes = boost::asio::read(socket, boost::asio::buffer(b, MAX_LENGTH), ec);
			vector<uint8_t> v(b, b + bytes);
			/* If the packet is valid, construct and add to packet vector */
			if (ValidatePacket(v)) {
				netVerbosePrint("Packet received ", 36);
				packets.push_back(Packet(v));
			}
			/* Close connection */
			socket.close();
			if (ec) {
				netVerbosePrint("Connection closed.", 35);
			}
			/* Report any exceptions */
		} catch (std::exception& e) {
			std::cerr << "Exception: " << e.what() << std::endl;
		}
		return packets.size();
	}

	/* Peer class methods */
	Peer::Peer(boost::asio::ip::address ip_addr)
		: ip_addr(ip_addr),
		strikes(0)
	{

	}

	Peer::Peer(std::string s)
		: strikes(0)
	{
		ip_addr = boost::asio::ip::address::from_string(s);
	}
}
