#include <parser.h>
#include <vector>
#include <cstdint>
#include <iostream>

using namespace std;
using namespace libmeshenger;

int
main()
{
	
	uint8_t test_data[] =
		{ 'I', 'M', 1, 0, 0, 1, 0, 32, /* Preamble */
			1, 1, 1, 1, 1, 1, 1, 1, /* ID */
			1, 1, 1, 1, 1, 1, 1, 1, /* ID */
			'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', /* Message */
			'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p'}; /* Message */

	/* Build a vector */
	std::vector<uint8_t> raw_data(test_data, test_data+40);
	
	/* Validate */
	cout << (ValidatePacket(raw_data) ? "Good" : "Bad") << endl;

	/* Try packet constructor */
	Packet p(raw_data);

	/* Print calculated values */
	cout << p.length() << endl;
	cout << (int) p.type() << endl;

	/* Construct message */
	ClearMessage m(p);

	/* Print calculated value */
	cout << m.length() << endl;

	/* Print subset vectors (id, body) */
	for(int i = 0; i < p.id().size(); i++) {
		cout << (int) p.id()[i];
	}
	cout << endl;
	
	for(int i = 0; i < 16; i++) {
		cout << (int) m.body()[i];
	}
	cout << endl;

	/* Create additional packets for operator== testing */
	ClearMessage m2(p);

	raw_data[8] = 2;
	p = Packet(raw_data);
	ClearMessage m3(p);

	cout << "M and M1 should be equal, M3 should be different" << endl;
	cout << "M1 and M2 are " << ((m == m2) ? "Equal" : "Inequal") << endl;
	cout << "M2 and M3 are " << ((m2 == m3) ? "Equal" : "Inequal") << endl;

	cout << "M1 ID: " << p.idString() << endl;
	cout << "M1 body: " << m.bodyString() << endl;

	cout << "Constructing packet" << endl;

	m = ClearMessage("This one was constructed instead of parsed");

	cout << "M1 ID: " << p.idString() << endl;
	cout << "M1 body: " << m.bodyString() << endl;

	raw_data[1] = 'm'; /* Mangle packet */
	p = Packet(raw_data); /* Throw exception */
}
