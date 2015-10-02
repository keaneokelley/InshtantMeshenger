#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <cstdlib>
#include <cstdint>

namespace libmeshenger
{
	class Net final
	{
		private:
			/* IO service for async asio operations */
			boost::asio::io_service& io_service;
			/* Socket the server will listen on*/
			udp::socket listen_socket;
			/* Endpoint for any remote connections */
			udp::endpoint remote_endpoint;
			/* UDP port number to listen on */
			u_int16_t port;
			/* Temporary/unused: data received on the socket */
			char data[1024];
		public:
			/* Construct with io_service object */
			Net(boost::asio::io_service& io_service);

			/* Starts a UDP listener on the provided port. The listener will
			 * create new node objects upon new connections and responds to the
			 * remote host. */
			void discoveryListen(u_int16_t port);
	}
}