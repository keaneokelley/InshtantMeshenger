/* Tests for crypto */

#include <crypto.h>
#include <cryptopp/osrng.h>
#include <iostream>
#include <string>

using namespace std;
using namespace libmeshenger;
using namespace CryptoPP;

int main(int argc, char ** argv)
{
	AutoSeededRandomPool rng;

	RSA::PublicKey pubkey;
	RSA::PrivateKey privkey;

	if (argc != 3) {
		cout << "Usage: test <privkeyfile> <pubkeyfile>" << endl;
		return -1;
	}

	privkey = CryptoEngine::privkeyFromFile(argv[1]);
	pubkey = CryptoEngine::pubkeyFromFile(argv[2]);

	cout << "Pubkey base64: " << endl << CryptoEngine::pubkeyToBase64(pubkey);
	cout << "Pubkey fingerprint: " << CryptoEngine::fingerprint(pubkey) << endl;

	cout << "Passing pubkey through base64 encode then decode" << endl;
	string pub_base64 = CryptoEngine::pubkeyToBase64(pubkey);
	cout << "Pubkey fingerprint: " << CryptoEngine::fingerprint(
			CryptoEngine::pubkeyFromBase64(pub_base64)) << endl;

	string original = "This is a test message that will be encrypted";

	CryptoEngine engine;
	engine.setPrivateKey(privkey);

	cout << "Pubkey fingerprint from engine: " << CryptoEngine::fingerprint(
			engine.getPubkey()) << endl;

	EncryptedMessage em_orig(original);

	cout << "Original: " << original << endl;
	cout << "EM decrypted body: " << string((const char *) em_orig.decryptedBody().data()) << endl;

	cout << "EM is " << (em_orig.decrypted() ? "decrypted" : "NOT decrypted") << endl;
	cout << "EM is " << (em_orig.encrypted() ? "encrypted" : "NOT encrypted") << endl;

	engine.encryptMessage(em_orig, pubkey);
	cout << "Encrypted message!" << endl;

	cout << "EM is " << (em_orig.decrypted() ? "decrypted" : "NOT decrypted") << endl;
	cout << "EM is " << (em_orig.encrypted() ? "encrypted" : "NOT encrypted") << endl;
	cout << "EM ciphertext length: " << em_orig.encryptedBody().size() << endl;

	Packet p(em_orig);

	cout << "Packet ID string: " << p.idString() << endl;
	cout << "Packet body length: " << p.body().size() << endl;

	EncryptedMessage em_new(p);

	cout << "Parsed packet into new EM" << endl;

	cout << "EM is " << (em_new.decrypted() ? "decrypted" : "NOT decrypted") << endl;
	cout << "EM is " << (em_new.encrypted() ? "encrypted" : "NOT encrypted") << endl;
	cout << "EM ciphertext length: " << em_new.encryptedBody().size() << endl;

	cout << "Decrypting message" << endl;
	cout << (engine.tryDecrypt(em_new) ? "SUCCESS" : "FAIL") << endl;

	cout << "EM body: " << (const char *) em_new.decryptedBody().data() << endl;

	return 0;
}
