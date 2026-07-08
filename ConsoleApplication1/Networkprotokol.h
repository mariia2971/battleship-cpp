#pragma once //to prevent the file from being copied multiple times
#include <cstdint> // for uint8_t

#pragma pack(push, 1) 
//so that the fields of the structure in memory lie close together. 
// this is mandatory because otherwise the data transmitted over the network will not be read properly.

// @brief packets exchanged between the client and the server
enum class PacketType : uint8_t { //a closed list. one byte per word

	ClientHell0 = 1,            //the client connects and sends the name
	ServerWelcome = 2,          //the server confirms the connection
	MatchFound = 3,             //the server tells you whose turn it is

	Fire = 4,                   
	FireResult = 5,            

	KeepAlive = 6,              //checking the connection
	PlayerLeft = 7              //technical defeat

};

// @brief The results of the shot
enum class HitResult : uint8_t {

	Miss = 0,
	Hit = 1,
	Destroed = 2

};

// @brief the header for any network message
enum class PacketHeader {

	PacketType type;
	uint16_t size;

};

// @brief found a couple to play with
struct MsgMatchFound {

	PacketHeader header;
	uint8_t playerNumber;
	char enemyName[32];

};

// @brief the player clicks on someone else's field
struct MsgFire {

	PacketHeader header;
	uint8_t x;
	uint8_t y;

};

// @brief The answer to the shot
struct MsgFireResult {

	PacketHeader header;
	uint8_t x; //Where were the shots fired
	uint8_t y;
	HitResult result;
	bool isGameOver;

};

//returning the standard alignment
#pragma pack(pop)