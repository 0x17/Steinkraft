// NetManager.cpp



#include "../Framework/Utilities.hpp"

char remoteIPStr[BUF_LEN];

#if !NO_NET

#include <sys/types.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <iostream>

#if !WIN32 && !ANDROID
#include <ifaddrs.h>
#endif

#include <Poco/Net/NetworkInterface.h>
#include <Poco/Thread.h>

#include "NetManager.hpp"

using Poco::Net::DatagramSocket;
using Poco::Net::SocketAddress;
using Poco::Net::StreamSocket;
using Poco::Net::SocketStream;
using Poco::Net::ServerSocket;
using Poco::Net::NetworkInterface;
using Poco::Net::IPAddress;

using Poco::Exception;

namespace as {

//==================================================================================
// IP address detection methods
//==================================================================================
void strToLocalIP(char *str);

bool wsInitialized = false;
#define INIT_WSOCK()	WSAData wsaData; \
							if(!wsInitialized) { \
								if(WSAStartup(MAKEWORD(2,2), &wsaData) != 0) \
									error("WinSock setup failed horribly!"); \
								else wsInitialized = true; \
							}
#define QUIT_WSOCK()	if(wsInitialized) { WSACleanup(); wsInitialized = false; }

void strToLocalIP(char *str) {
#if !ANDROID && !WIN32
	struct ifaddrs *ifAddrStruct = NULL;
	struct ifaddrs *ifa = NULL;
	void *tmpAddrPtr = NULL;

	getifaddrs(&ifAddrStruct);

	for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
		if (ifa ->ifa_addr->sa_family == AF_INET) { // check it is IP4
			// is a valid IP4 Address
			tmpAddrPtr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
			char addressBuffer[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
			if (!strcmp(ifa->ifa_name, "lo")
					|| !strcmp(ifa->ifa_name, "lo0")) continue;
			//printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer);
			strcpy(str, addressBuffer);
			break;
		}
	}
	if (ifAddrStruct != NULL) freeifaddrs(ifAddrStruct);
#else
#if ANDROID
	int sfd, i;
	struct ifreq ifr;
	struct sockaddr_in *sin = (struct sockaddr_in *) & ifr.ifr_addr;

	memset(&ifr, 0, sizeof ifr);

	if (0 > (sfd = socket(AF_INET, SOCK_STREAM, 0))) {
		perror("socket()");
		exit(1);
	}

	strcpy(ifr.ifr_name, "eth0");
	sin->sin_family = AF_INET;

	if (0 == ioctl(sfd, SIOCGIFADDR, &ifr)) {
		//printf("%s: %s\n", ifr.ifr_name, inet_ntoa(sin->sin_addr));
		strcpy(str, inet_ntoa(sin->sin_addr));
	}
#else // WIN32
	char ac[80];

	INIT_WSOCK();

	if (gethostname(ac, 80) == SOCKET_ERROR) {
		std::printf("Code: %d\n", WSAGetLastError());
		error("Can't get hostname!");
	}

	struct hostent *phe = gethostbyname(ac);
	if (!phe) {
		std::printf("Code: %d\n", WSAGetLastError());
		error("Host lookup failed!");
	}

	for (int i = 0; phe->h_addr_list[i] != 0; ++i) {
		struct in_addr addr;
		memcpy(&addr, phe->h_addr_list[i], sizeof(struct in_addr));
		strcpy(str, inet_ntoa(addr));
		return;
	}
	
	QUIT_WSOCK()
#endif
#endif
}

#if ANDROID
void localAddrToStr(char *str) {
	int sfd, i;
	struct ifreq ifr;
	struct sockaddr_in *sin = (struct sockaddr_in *) & ifr.ifr_addr;

	memset(&ifr, 0, sizeof ifr);

	if (0 > (sfd = socket(AF_INET, SOCK_STREAM, 0))) {
		perror("socket()");
		exit(1);
	}

	strcpy(ifr.ifr_name, "eth0");
	sin->sin_family = AF_INET;

	if (0 == ioctl(sfd, SIOCGIFADDR, &ifr)) {
		//printf("%s: %s\n", ifr.ifr_name, inet_ntoa(sin->sin_addr));
		strcpy(str, inet_ntoa(sin->sin_addr));
	}
}
#endif

#if !IPHONE && !ANDROID
void localAddrToStr(char *dest);

void localAddrToStr(char *dest) {
	strToLocalIP(dest);
}
#endif
	
const ticks_t SYNC_DELAY = (int)(1000.0f / 15.0f);

#define NETFAIL(msg, e)	printf("Exception: %s\n%s\n", msg, e.message().c_str()); \
						fflush(stdout); \
						otherQuit = true;

//==================================================================================
// NetManager
//==================================================================================
NetManager::NetManager(bool _server, Terrain *_t, Camera *_cam)
:	server(_server),
	otherQuit(false),
	t(_t),
	lastSync(0),
	cam(_cam),
	localAddr("localhost", _server ? SERVER_PORT : CLIENT_PORT),
	streamSock(NULL),
	servSock(NULL),
	strm(NULL),
	clientConnected(false),
	dontSave(false)
{
	printLocalIP();
	
	char localIpStr[256];
	strToLocalIP(localIpStr);
	
	dataSocket = DatagramSocket(SocketAddress(localIpStr, _server ? SERVER_PORT : CLIENT_PORT));
	discoverSocket = DatagramSocket(SocketAddress(IPAddress(), _server ? SERVER_DISCOVER_PORT : CLIENT_DISCOVER_PORT));

	if(!server) { // client
		try {
			SocketAddress to("255.255.255.255", SERVER_DISCOVER_PORT);

			char buf[BUF_LEN];
			strToLocalIP(buf);
			
			discoverSocket.setBroadcast(true);
			discoverSocket.sendTo(buf, BUF_LEN, to);
			discoverSocket.setBroadcast(false);
			
			discoverSocket.close();
			discoverSocket = DatagramSocket(SocketAddress(localIpStr, CLIENT_DISCOVER_PORT));

			ticks_t startDiscover = getTicks();
			const ticks_t discoverTimeout = 2000;

			bool serverDiscovered = false;

			while(getTicks() - startDiscover < discoverTimeout) {
				if(discoverSocket.available()) {
					discoverSocket.receiveBytes(buf, BUF_LEN);
					otherAddr = SocketAddress(IPAddress(std::string(buf)), SERVER_PORT);
					serverDiscovered = true;
					
					break;
				}
			}
			
			discoverSocket.close();

			if(!serverDiscovered) {
				dontSave = true;
				otherQuit = true;
				return;
			}
		} catch(Exception &e) {
			NETFAIL("Server discovery failed!", e);
			return;
		}

		try {
			streamSock = new StreamSocket(otherAddr);
			Poco::Timespan timeoutSpan(TIMEOUT_SECS, 0);
			streamSock->setSendTimeout(timeoutSpan);
			streamSock->setReceiveTimeout(timeoutSpan);
			strm = new SocketStream(*streamSock);

			receiveTerrain();

			SAFE_DELETE(strm);
			SAFE_DELETE(streamSock);
		} catch(Exception &e) {
			NETFAIL("Receive terrain failed!", e);
		}
	}
}

void NetManager::printLocalIP() {
	char destStr[1024];
	localAddrToStr(destStr);
}

void NetManager::strToLocalIP(char *str) {
	localAddrToStr(str);
}

void NetManager::sendTerrain() {
	DATA_TYPE *data = t->getDataPtr();

	// compress before!
	sendBlocked(data, Terrain::MAX_X*Terrain::MAX_Y*Terrain::MAX_Z);

	std::list<Entity> *entities = t->getEntitiesPtr();

	int numEntities = (int)entities->size();
	sendBlocked(&numEntities, sizeof(int));

	std::list<Entity>::const_iterator it;
	for (it = entities->begin(); it != entities->end(); ++it) {
		sendBlocked(&(*it), sizeof(Entity));
	}
}

void NetManager::receiveTerrain() {
	DATA_TYPE *data = t->getDataPtr();

	// decompress here!
	recvBlocked(data, Terrain::MAX_X*Terrain::MAX_Y*Terrain::MAX_Z);

	int numEntities = 0;
	recvBlocked(&numEntities, sizeof(int));

	Entity entity;

	for (int i = 0; i < numEntities; i++) {
		recvBlocked(&entity, sizeof(Entity));
		t->addEntity(entity);
	}
}

NetManager::~NetManager() {
	if (!otherQuit && (clientConnected || !server)) {
		uchar buf[NBUF_LEN];
		sendUnblocked(MT_QUIT, buf);
	}
	
	Poco::Thread::sleep(1000);
	
	SAFE_DELETE(strm);
	SAFE_DELETE(streamSock);
	SAFE_DELETE(servSock);
}

void NetManager::sendBlocked( const void *buf, int size ) {
	try {
		strm->write((const char *)buf, size);
		strm->flush();
	} catch(Exception &e) {
		NETFAIL("Failed to send blocked!", e);
	}
}

void NetManager::recvBlocked( void *buf, int size ) {
	try {
		strm->read((char *)buf, size);
	} catch(Exception &e) {
		NETFAIL("Failed to receive blocked!", e);
	}
}

void NetManager::sendPart() {
	SetMessage msg;
	UpdateMessage upMsg;
	Entity entMsg;
	BlockPos entDelMsg;
	uchar buf[NBUF_LEN];

	Vec3 *camPos = cam->getPosPtr();
	float cx = camPos->x;
	float cy = camPos->y;
	float cz = camPos->z;
	upMsg.pos[0] = cx;
	upMsg.pos[1] = cy;
	upMsg.pos[2] = cz;
	upMsg.yaw = cam->getYaw();
	upMsg.fromServer = server;

	memcpy(buf, &upMsg, sizeof(UpdateMessage));
	sendUnblocked(MT_UPDATE, buf);

	if(!setMsgs.empty()) {
		msg = setMsgs.front();
		memcpy(buf, &msg, sizeof(SetMessage));
		sendUnblocked(MT_SET, buf);
		setMsgs.remove(msg);
	} else if(!entityMsgs.empty()) {
		entMsg = entityMsgs.front();
		memcpy(buf, &entMsg, sizeof(Entity));
		sendUnblocked(MT_ENTITY, buf);
		entityMsgs.remove(entMsg);
	} else if(!delEntityMsgs.empty()) {
		entDelMsg = delEntityMsgs.front();
		memcpy(buf, &entDelMsg, sizeof(BlockPos));
		sendUnblocked(MT_DEL_ENTITY, buf);
		delEntityMsgs.remove(entDelMsg);
	}
}

#define XOR(a,b) (a ^ b)

bool NetManager::recvPart() {
	SetMessage msg;
	UpdateMessage upMsg;
	Entity entMsg;
	BlockPos entDelMsg;

	uchar buf[NBUF_LEN];
	
	while(dataSocket.available()) {
		MessageTypes type = recvUnblocked(buf);

		switch(type) {
		case MT_SET:
			memcpy(&msg, buf, sizeof(SetMessage));
			t->set(msg.x, msg.y, msg.z, msg.val);
			break;
		case MT_ENTITY:
			memcpy(&entMsg, buf, sizeof(Entity));
			t->addEntity(entMsg);
			break;
		case MT_DEL_ENTITY:
			memcpy(&entDelMsg, buf, sizeof(BlockPos));
			t->removeEntityAt(entDelMsg.x, entDelMsg.y, entDelMsg.z);
			break;
		case MT_UPDATE:
			memcpy(&upMsg, buf, sizeof(UpdateMessage));
			//if(!XOR(upMsg.fromServer, server)) std::printf("From myself...\n");
			otherPos.x = upMsg.pos[0];
			otherPos.y = upMsg.pos[1];
			otherPos.z = upMsg.pos[2];
			otherYaw = upMsg.yaw;
			break;
		case MT_QUIT:
			otherQuit = true;
			return true;
			break;
		}
	}

	return false;
}

class AcceptRunner : public Poco::Runnable {
public:
	Poco::Net::ServerSocket *servSock;
	Poco::Net::StreamSocket *ssock;
	
	AcceptRunner(Poco::Net::ServerSocket *_servSock)
		: servSock(_servSock), ssock(NULL) {}

	virtual void run() {
		ssock = new StreamSocket(servSock->acceptConnection());
	}
};

bool NetManager::sync() {
	if(otherQuit) return true;

	if(server && !clientConnected) {
		if(discoverSocket.available()) {
			try {
				char buf[BUF_LEN];
				
				discoverSocket.receiveBytes(buf, BUF_LEN);
				
				otherAddr = SocketAddress(IPAddress(std::string(buf)), CLIENT_PORT);
				SocketAddress to(IPAddress(std::string(buf)), CLIENT_DISCOVER_PORT);
				
				strToLocalIP(buf);
				
				discoverSocket.sendTo(buf, BUF_LEN, to);

				servSock = new ServerSocket(SERVER_PORT);
				
				Poco::Thread acceptThread;
				AcceptRunner runner(servSock);
				acceptThread.start(runner);
				if(!acceptThread.tryJoin(ACCEPT_TIMEOUT)) {
					otherQuit = true;
					return true;
				}
		
				streamSock = runner.ssock;
				Poco::Timespan timeoutSpan(TIMEOUT_SECS, 0);
				streamSock->setSendTimeout(timeoutSpan);
				streamSock->setReceiveTimeout(timeoutSpan);
				strm = new SocketStream(*streamSock);

				discoverSocket.close();

				sendTerrain();

				SAFE_DELETE(strm);
				SAFE_DELETE(streamSock);
				SAFE_DELETE(servSock);
				
				clientConnected = true;
			} catch(Exception &e) {
				NETFAIL("Server terrain send failed!", e);
				return true;
			}
		} else
			return false;
	}

	if (getTicks() - lastSync < SYNC_DELAY)
		return false;

	if(!server || clientConnected) {
		if(recvPart()) return true;
		sendPart();
	}

	lastSync = getTicks();

	return false;
}

void NetManager::sendUnblocked( MessageTypes type, const void *buf )
{
	static uchar bigBuf[NBUF_LEN+1];
	bigBuf[0] = type;
	memcpy(&bigBuf[1], buf, NBUF_LEN);
	try {
		dataSocket.sendTo(bigBuf, NBUF_LEN+1, otherAddr);
	} catch(Poco::Exception &e) {
		NETFAIL("unblocked send fail: ", e);
	}
}

NetManager::MessageTypes NetManager::recvUnblocked( void *buf )
{
	static uchar bigBuf[NBUF_LEN+1];
	try {
		dataSocket.receiveBytes(bigBuf, NBUF_LEN+1);
	} catch(Poco::Exception &e) {
		NETFAIL("unblocked recv fail: ", e);
	}
	MessageTypes type = (MessageTypes)bigBuf[0];
	memcpy(buf, &bigBuf[1], NBUF_LEN);
	return type;
}

}

#endif // !NO_NET

