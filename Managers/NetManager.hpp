// NetManager.hpp

#ifndef NETMANAGER_HPP
#define NETMANAGER_HPP

#if !NO_NET

#include <Poco/Net/DatagramSocket.h>
#include <Poco/Net/SocketAddress.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Net/SocketStream.h>
#include <Poco/Net/ServerSocket.h>

#include <Poco/Exception.h>

#include <list>
#include <queue>

#include "../Terrain.hpp"
#include "../Framework/Observable.hpp"
#include "../Framework/Utilities.hpp"

namespace as {
//===========================================================================
// Globals
//===========================================================================
extern char remoteIPStr[BUF_LEN];

//===========================================================================
// Types
//===========================================================================
struct UpdateMessage {
	float pos[3], yaw;
	bool fromServer;
};

struct SetMessage {
	SetMessage(int _x, int _y, int _z, DATA_TYPE _val) : x(_x), y(_y), z(_z), val(_val) {}
	SetMessage() : x(0), y(0), z(0), val(0) {}

	int x, y, z;
	DATA_TYPE val;

	bool operator==(const SetMessage &o) const {
		return x == o.x && y == o.y && z == o.z && val == o.val;
	}
};

class NetManager {
public:
	NetManager(bool server, Terrain *t, Camera *cam);
	virtual ~NetManager();
	bool sync();

	void sendPart();
	bool recvPart();

	bool isServer() const;

	void sendEntity(Entity e);
	void sendEntityDeletion(int ex, int ey, int ez);
	void sendSet(int x, int y, int z, DATA_TYPE val);

	Vec3 *getOtherPosPtr();
	float getOtherYaw();

	static void strToLocalIP(char *str);
	
	bool shouldSave();
	
private:
	enum MessageTypes {
		MT_SET = 0,
		MT_ENTITY,
		MT_DEL_ENTITY,
		MT_UPDATE,
		MT_QUIT
	};

	void sendTerrain();
	void receiveTerrain();

	void recvBlocked(void *buf, int size);
	void sendBlocked(const void *buf, int size);

	NetManager::MessageTypes recvUnblocked(void *buf);
	void sendUnblocked(NetManager::MessageTypes type, const void *buf);	

	void printLocalIP();

	bool flushQueues();
	
	enum Consts {
		CLIENT_PORT = 1101,
		SERVER_PORT = 1100,
		CLIENT_DISCOVER_PORT = 14243,
		SERVER_DISCOVER_PORT = 14242,
		TIMEOUT_SECS = 10,
		ACCEPT_TIMEOUT = 4000,
		NBUF_LEN = 64
	};
	
	bool server, otherQuit;
	Terrain *t;
	
	std::list<SetMessage> setMsgs;
	std::list<Entity> entityMsgs;
	std::list<BlockPos> delEntityMsgs;
	
	ticks_t lastSync;
	Camera *cam;
	Vec3 otherPos;
	float otherYaw;

	Poco::Net::SocketAddress localAddr, otherAddr;

	Poco::Net::StreamSocket *streamSock;
	Poco::Net::ServerSocket *servSock;
	Poco::Net::SocketStream *strm;

	Poco::Net::DatagramSocket dataSocket, discoverSocket;
	
	bool clientConnected;
	bool dontSave;
};

//===========================================================================
// Methods
//===========================================================================
inline bool NetManager::shouldSave() {
	return !dontSave;
}

inline Vec3 *NetManager::getOtherPosPtr() {
	return &otherPos;
}

inline float NetManager::getOtherYaw() {
	return otherYaw;
}

inline bool NetManager::isServer() const {
	return server;
}

inline void NetManager::sendEntity(Entity e) {
	if(server && !clientConnected) return;
	entityMsgs.push_back(e);
}

inline void NetManager::sendSet( int x, int y, int z, DATA_TYPE val ) {
	if(server && !clientConnected) return;
	setMsgs.push_back(SetMessage(x, y, z, val));
}

inline void NetManager::sendEntityDeletion( int ex, int ey, int ez ) {
	if(server && !clientConnected) return;
	delEntityMsgs.push_back(BlockPos(ex, ey, ez));
}

}

#endif // !NO_NET

#endif /* NETMANAGER_HPP_ */
