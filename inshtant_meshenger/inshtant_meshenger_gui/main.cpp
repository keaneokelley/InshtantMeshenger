#include "mainwindow.h"
#include "addpeer.h"
#include "pgp.h"
#include <QApplication>
#include <QSettings>
#include <vector>

#include <parser.h>
#include <state.h>
#include <net.h>
#include <tracker.h>
#include <crypto.h>

using namespace libmeshenger;
using namespace std;
MainWindow *win;

static Net net(5555, 5556);
static QSettings settings("meshenger", "Inshtant Meshenger");

void ForwardPacketToPeers(Packet& p)
{
    /* Encapsulate message in packet */
    net.sendToAllPeers(p);
}

void displayMessageHandler(Packet& p)
{
    win->displayMessage(p);
}

Tracker tracker("http://meshtrack.pqz.us", net.get_ifaddr("meshtrack.pqz.us").to_string());
static void ReportHop(Packet& p)
{
	cout << "\033[132m<Reporting hops\033[0m" << endl;
	for(auto &peer : net.getPeers()) {
		tracker.reportHop(p.idString(), to_string(p.depth()), peer.ip_addr.to_string());
	}
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    PacketEngine engine;
	CryptoEngine cryptoEngine;
    MainWindow w(0, net, engine, cryptoEngine, tracker, settings);
    win = &w;

    engine.AddCallback(ForwardPacketToPeers);
    engine.AddCallback(displayMessageHandler);
    engine.AddCallback(ReportHop);

	tracker.reportNode();

    /* Start listening asynchronously */
    net.discoveryListen();
    net.discoverPeers();

	/* Set your private and public key. */
    string privkey = settings.value("crypto/privkey", "").toString().toStdString();
    if (argc > 1 && string(argv[1]).length() > 0) {
        cryptoEngine.setPrivateKeyFromFile(argv[1]);
        settings.setValue("crypto/privkey", cryptoEngine.privkeyToBase64(cryptoEngine.getPrivkey()).c_str());
    } else if (privkey.length() > 0)
        cryptoEngine.setPrivateKey(privkey);

    w.loadBuddies(cryptoEngine);

	/* Add buddies.
	/* Also adds peers. That should be GUIfied as well */
    for(int i = 2; i < argc; i++) {
        /* It is actually a peer. Parsing args is hard */
        if (string(argv[i], argv[i] + 2) == string("-P")) {
            cout << "Adding peer " << argv[i] + 2 << endl;
            net.addPeer(argv[i] + 2);
        } else {
            string buddy_name = argv[i];
            string filename = buddy_name + ".pub";
            CryptoPP::RSA::PublicKey pubkey = CryptoEngine::pubkeyFromFile(filename);
            cryptoEngine.addBuddy(Buddy(pubkey, buddy_name));
            cout << "Added buddy: " << buddy_name << ". Pubkey: " << endl;
            cout << CryptoEngine::pubkeyToBase64(pubkey) << endl;
        }
    }
    w.saveBuddies(cryptoEngine.buddies());

    net.startListen();
    net.run();

    w.show();
    w.populateBuddyList(cryptoEngine.buddies());
    return a.exec();
}
