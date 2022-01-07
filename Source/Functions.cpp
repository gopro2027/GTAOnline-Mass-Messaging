#include "stdafx.h"

std::set<Ped> lastSeenPeds;

//=================
// PED FUNCTIONS
//=================

Ped ClonePed(Ped ped)
{
	if (ENTITY::DOES_ENTITY_EXIST(ped) && !ENTITY::IS_ENTITY_DEAD(ped))
	{
		return PED::CLONE_PED(ped, ENTITY::GET_ENTITY_HEADING(ped), 1, 1);
	}

	return 0;
}

Ped CreatePed(char* PedName, Vector3 SpawnCoordinates, int ped_type, bool network_handle)
{
	Ped NewPed;
	int PedHash = GAMEPLAY::GET_HASH_KEY(PedName);
	if (STREAMING::IS_MODEL_IN_CDIMAGE(PedHash))
	{
		if (STREAMING::IS_MODEL_VALID(PedHash))
		{
			STREAMING::REQUEST_MODEL(PedHash);
			while (!STREAMING::HAS_MODEL_LOADED(PedHash)) WAIT(0);

			NewPed = PED::CREATE_PED(ped_type, PedHash, SpawnCoordinates.x, SpawnCoordinates.y, SpawnCoordinates.z, 0, network_handle, 1);
			STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(PedHash);
			return NewPed;
		}
	}

	return -1;
}

//Animations
void LoadAnim(char * dict)
{
	int tick = 0;
	STREAMING::REQUEST_ANIM_DICT(dict);
	while (tick < 100 && !STREAMING::HAS_ANIM_DICT_LOADED(dict))
	{
		tick++;
		WAIT(0);
	}
}

void playAnimation(Ped ped, bool loop, char * dict, char * anim)
{
	LoadAnim(dict);
	int a = -1;
	int b = 1;

	if (!loop)
	{
		a = 1;
		b = 0;
	}

	AI::TASK_PLAY_ANIM(ped, dict, anim, 10000.0f, -1.5f, a, b, 0.445f, false, false, false);

}

//Skins
bool applyChosenSkin(DWORD model)
{
	if (STREAMING::IS_MODEL_IN_CDIMAGE(model) && STREAMING::IS_MODEL_VALID(model))
	{
		STREAMING::REQUEST_MODEL(model);
		while (!STREAMING::HAS_MODEL_LOADED(model))
		{
			WAIT(0);
		}

		Vehicle veh = NULL;
		if (PED::IS_PED_IN_ANY_VEHICLE(PLAYER::PLAYER_PED_ID(), 0))
		{
			veh = PED::GET_VEHICLE_PED_IS_USING(PLAYER::PLAYER_PED_ID());
		}

		PLAYER::SET_PLAYER_MODEL(PLAYER::PLAYER_ID(), model);
		//PED::SET_PED_RANDOM_COMPONENT_VARIATION(PLAYER::PLAYER_PED_ID(), FALSE);
		PED::SET_PED_DEFAULT_COMPONENT_VARIATION(PLAYER::PLAYER_PED_ID());
		WAIT(0);

		if (veh != NULL)
		{
			PED::SET_PED_INTO_VEHICLE(PLAYER::PLAYER_PED_ID(), veh, -1);
		}

		WAIT(100);
		STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(model);

		return true;
	}
	return false;
}

bool applyChosenSkin(std::string skinName)
{
	DWORD model = GAMEPLAY::GET_HASH_KEY((char *)skinName.c_str());
	return applyChosenSkin(model);
}

//CONTROL
void RequestControlOfid(Entity netid)
{
	int tick = 0;

	while (!NETWORK::NETWORK_HAS_CONTROL_OF_NETWORK_ID(netid) && tick <= 25)
	{
		NETWORK::NETWORK_REQUEST_CONTROL_OF_NETWORK_ID(netid);
		tick++;
	}
}

void RequestControlOfEnt(Entity entity)
{
	int tick = 0;
	while (!NETWORK::NETWORK_HAS_CONTROL_OF_ENTITY(entity) && tick <= 25)
	{
		NETWORK::NETWORK_REQUEST_CONTROL_OF_ENTITY(entity);
		tick++;
	}
	if (NETWORK::NETWORK_IS_SESSION_STARTED()) 
	{
		int netID = NETWORK::NETWORK_GET_NETWORK_ID_FROM_ENTITY(entity);
		RequestControlOfid(netID);
		NETWORK::SET_NETWORK_ID_CAN_MIGRATE(netID, 1);
	}
}

//FORCE
void ApplyForceToEntity(Entity e, float x, float y, float z)
{
	if (e != PLAYER::PLAYER_PED_ID() && NETWORK::NETWORK_HAS_CONTROL_OF_ENTITY(e) == FALSE)
	{
		RequestControlOfEnt(e);
	}

	ENTITY::APPLY_FORCE_TO_ENTITY(e, 1, x, y, z, 0, 0, 0, 0, 1, 1, 1, 0, 1);

}

//GOD MODE
void GodMode(bool toggle)
{
	static int armour_player = 0;
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	if (armour_player == 0)
	{
		armour_player = PED::GET_PED_ARMOUR(playerPed);
	}

	if (toggle)
	{
		PLAYER::SET_PLAYER_INVINCIBLE(player, true);
		ENTITY::SET_ENTITY_PROOFS(playerPed, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE);
		PED::SET_PED_CAN_RAGDOLL(playerPed, FALSE);
		PED::SET_PED_CAN_RAGDOLL_FROM_PLAYER_IMPACT(playerPed, FALSE);
		PED::ADD_ARMOUR_TO_PED(playerPed, PLAYER::GET_PLAYER_MAX_ARMOUR(player) - PED::GET_PED_ARMOUR(playerPed));
	}
	else
	{
		PLAYER::SET_PLAYER_INVINCIBLE(player, false);
		ENTITY::SET_ENTITY_PROOFS(playerPed, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE);
		PED::SET_PED_CAN_RAGDOLL(playerPed, TRUE);
		PED::SET_PED_CAN_RAGDOLL_FROM_PLAYER_IMPACT(playerPed, TRUE);
		if (armour_player != 0)
		{
			PED::SET_PED_ARMOUR(playerPed, armour_player);
			armour_player = 0;
		}
	}
}

//NEARBY PEDS
std::set<Ped> getNearbyPeds()
{
	return lastSeenPeds;
}

void update_nearby_peds(Ped playerPed, int count)
{
	const int numElements = count;
	const int arrSize = numElements * 2 + 2;

	Ped *peds = new Ped[arrSize];
	peds[0] = numElements;
	int found = PED::GET_PED_NEARBY_PEDS(playerPed, peds, -1);

	for (int i = 0; i < found; i++)
	{
		int offsettedID = i * 2 + 2;

		if (!ENTITY::DOES_ENTITY_EXIST(peds[offsettedID]))
		{
			continue;
		}

		Ped xped = peds[offsettedID];

		bool inSet = lastSeenPeds.find(xped) != lastSeenPeds.end();
		if (!inSet)
		{
			lastSeenPeds.insert(xped);
		}
	}

	std::set<Ped>::iterator it;
	for (it = lastSeenPeds.begin(); it != lastSeenPeds.end();)
	{
		if (!ENTITY::DOES_ENTITY_EXIST(*it))
		{
			lastSeenPeds.erase(it++);
		}
		else
		{
			++it;
		}
	}

	delete peds;
}

//CALM PEDS
void set_all_nearby_peds_to_calm()
{
	for each (Ped xped in lastSeenPeds)
	{
		PED::SET_BLOCKING_OF_NON_TEMPORARY_EVENTS(xped, true);
		PED::SET_PED_FLEE_ATTRIBUTES(xped, 0, 0);
		PED::SET_PED_COMBAT_ATTRIBUTES(xped, 17, 1);
	}
}

//Converts Radians to Degrees
float degToRad(float degs)
{
	return degs*3.141592653589793f / 180.f;
}

//little one-line function called '$' to convert $TRING into a hash-key:
Hash $(std::string str) {
	return GAMEPLAY::GET_HASH_KEY(&str[0u]);
}

// quick function to get - coords - of - entity:
Vector3 coordsOf(Entity entity) {
	return ENTITY::GET_ENTITY_COORDS(entity, 1);
}

//quick function to get distance between 2 points: eg - if (distanceBetween(coordsOf(player), targetCoords) < 50)
float distanceBetween(Vector3 A, Vector3 B) {
	return GAMEPLAY::GET_DISTANCE_BETWEEN_COORDS(A.x, A.y, A.z, B.x, B.y, B.z, 1);
}

//quick "get random int in range 0-x" function:
int rndInt(int start, int end) {
	return GAMEPLAY::GET_RANDOM_INT_IN_RANGE(start, end);
}

//TELEPORTATION
void teleport_to_coords(Entity e, Vector3 coords)
{
	ENTITY::SET_ENTITY_COORDS_NO_OFFSET(e, coords.x, coords.y, coords.z, 0, 0, 1);
	WAIT(0);
}

Vector3 get_blip_marker()
{
	static Vector3 zero;
	Vector3 coords;

	bool blipFound = false;
	// search for marker blip
	int blipIterator = UI::_GET_BLIP_INFO_ID_ITERATOR();
	for (Blip i = UI::GET_FIRST_BLIP_INFO_ID(blipIterator); UI::DOES_BLIP_EXIST(i) != 0; i = UI::GET_NEXT_BLIP_INFO_ID(blipIterator))
	{
		if (UI::GET_BLIP_INFO_ID_TYPE(i) == 4)
		{
			coords = UI::GET_BLIP_INFO_ID_COORD(i);
			blipFound = true;
			break;
		}
	}
	if (blipFound)
	{
		return coords;
	}

	return zero;
}

void teleport_to_marker()
{
	Vector3 coords = get_blip_marker();

	if (coords.x == 0 && coords.y == 0)
	{
		//notifyMap("No Waypoint has been set!", 0);
		return;
	}

	// get entity to teleport
	Entity e = PLAYER::PLAYER_PED_ID();
	if (PED::IS_PED_IN_ANY_VEHICLE(e, 0))
	{
		e = PED::GET_VEHICLE_PED_IS_USING(e);
	}

	// load needed map region and check height levels for ground existence
	bool groundFound = false;
	static float groundCheckHeight[] =
	{ 100.0, 150.0, 50.0, 0.0, 200.0, 250.0, 300.0, 350.0, 400.0, 450.0, 500.0, 550.0, 600.0, 650.0, 700.0, 750.0, 800.0 };
	for (int i = 0; i < sizeof(groundCheckHeight) / sizeof(float); i++)
	{
		ENTITY::SET_ENTITY_COORDS_NO_OFFSET(e, coords.x, coords.y, groundCheckHeight[i], 0, 0, 1);
		WAIT(100);
		if (GAMEPLAY::GET_GROUND_Z_FOR_3D_COORD(coords.x, coords.y, groundCheckHeight[i], &coords.z, 0))
		{
			groundFound = true;
			coords.z += 3.0;
			break;
		}
	}
	// if ground not found then set Z in air and give player a parachute
	if (!groundFound)
	{
		coords.z = 1000.0;
		WEAPON::GIVE_DELAYED_WEAPON_TO_PED(PLAYER::PLAYER_PED_ID(), 0xFBAB5776, 1, 0);
	}
	//do it
	teleport_to_coords(e, coords);
}


//In Game KEYBOARD
std::string show_keyboard(char* title_id, char* prepopulated_text)
{
	DWORD time = GetTickCount() + 400;
	while (GetTickCount() < time)
	{
		WAIT(0);
	}

	GAMEPLAY::DISPLAY_ONSCREEN_KEYBOARD(true, (title_id == NULL ? "HUD_TITLE" : title_id), "", (prepopulated_text == NULL ? "" : prepopulated_text), "", "", "", 64);

	while (GAMEPLAY::UPDATE_ONSCREEN_KEYBOARD() == 0)
	{
		WAIT(0);
	}

	std::stringstream ss;
	if (!GAMEPLAY::GET_ONSCREEN_KEYBOARD_RESULT())
	{
		return std::string("");
	}
	else
	{
		return std::string(GAMEPLAY::GET_ONSCREEN_KEYBOARD_RESULT());
	}
}

//VEHICLE
static std::string lastvehmodel("");
bool get_vehicle_keyboard_result(uint* outModel)
{
	std::string result = show_keyboard("FMMC_KEY_TIP", &lastvehmodel[0]);
	if (!result.empty())
	{
		uint model = $(result);
		if (!STREAMING::IS_MODEL_IN_CDIMAGE(model) || !STREAMING::IS_MODEL_A_VEHICLE(model))
		{
			lastvehmodel = "";
			//notifyBottom("~HUD_COLOUR_RED~Not A Valid Model!");
			return false;
		}

		else
		{
			STREAMING::REQUEST_MODEL(model);
			DWORD now = GetTickCount();
			while (!STREAMING::HAS_MODEL_LOADED(model) && GetTickCount() < now + 5000)
			{
				WAIT(0);
			}

			if (!STREAMING::HAS_MODEL_LOADED(model))
			{
				lastvehmodel = "";
				std::ostringstream ss2;
				ss2 << "~HUD_COLOUR_RED~ Timed out requesting  " << result << " : 0x" << model;
//				notifyBottom(ss2.str());
				return false;
			}

			lastvehmodel = result;
			*outModel = model;
			return true;
		}

	}

	return false;
}

//VECTOR AND FLOAT FUNCTIONS
Vector3 rot_to_direction(Vector3*rot) {
	float radiansZ = rot->z*0.0174532924f;
	float radiansX = rot->x*0.0174532924f;
	float num = abs((float)cos((double)radiansX));
	Vector3 dir;
	dir.x = (float)((double)((float)(-(float)sin((double)radiansZ)))*(double)num);
	dir.y = (float)((double)((float)cos((double)radiansZ))*(double)num);
	dir.z = (float)sin((double)radiansX);
	return dir;
}

Vector3 add(Vector3*vectorA, Vector3*vectorB) {
	Vector3 result;
	result.x = vectorA->x;
	result.y = vectorA->y;
	result.z = vectorA->z;
	result.x += vectorB->x;
	result.y += vectorB->y;
	result.z += vectorB->z;
	return result;
}

Vector3 multiply(Vector3*vector, float x) {
	Vector3 result;
	result.x = vector->x;
	result.y = vector->y;
	result.z = vector->z;
	result.x *= x;
	result.y *= x;
	result.z *= x;
	return result;
}

float get_distance(Vector3*pointA, Vector3*pointB) {
	float a_x = pointA->x;
	float a_y = pointA->y;
	float a_z = pointA->z;
	float b_x = pointB->x;
	float b_y = pointB->y;
	float b_z = pointB->z;
	double x_ba = (double)(b_x - a_x);
	double y_ba = (double)(b_y - a_y);
	double z_ba = (double)(b_z - a_z);
	double y_2 = y_ba*y_ba;
	double x_2 = x_ba*x_ba;
	double sum_2 = y_2 + x_2;
	return(float)sqrt(sum_2 + z_ba);
}

float get_vector_length(Vector3*vector) {
	double x = (double)vector->x;
	double y = (double)vector->y;
	double z = (double)vector->z;
	return(float)sqrt(x*x + y*y + z*z);
}

//NOTIFICATIONS
/*
Colours:
~r~ = Red
~b~ = Blue
~g~ = Green
~y~ = Yellow
~p~ = Purple
~o~ = Orange
~c~ = Grey
~m~ = Dark Grey
~u~ = Black
~n~ = Skip Line
~s~ = White
~d~ = dark blue
~f~ = light blue
~l~ = black
~t~ = gray
~v~ = black

Fonts:
~italic~ = italic font
~bold~ = bold font
*/

char status_text[2048] = { 0 };
void update_status_text()
{
	if (GetTickCount() < 2500)
	{
		UI::SET_TEXT_FONT(0);
		UI::SET_TEXT_SCALE(0.55f, 0.55f);
		UI::SET_TEXT_COLOUR(255, 255, 255, 255);
		UI::SET_TEXT_WRAP(0.0f, 1.0f);
		UI::SET_TEXT_CENTRE(1);
		UI::SET_TEXT_DROPSHADOW(0, 0, 0, 0, 0);
		UI::SET_TEXT_EDGE(1, 0, 0, 0, 205);
		UI::BEGIN_TEXT_COMMAND_DISPLAY_TEXT("STRING");
		UI::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME(status_text);
		UI::END_TEXT_COMMAND_DISPLAY_TEXT(0.5, 0.5);
	}
}

void notifyBottom(char * fmt, ...)
{
	char buf[2048] = { 0 };
	va_list va_alist;

	va_start(va_alist, fmt);
	vsprintf_s(buf, fmt, va_alist);
	va_end(va_alist);

	char buff2[2048] = { 0 };
	sprintf_s(buff2, "%s", buf);

	UI::BEGIN_TEXT_COMMAND_PRINT("STRING");
	UI::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME(buff2);
	UI::END_TEXT_COMMAND_PRINT(4000, 1);
}	void notifyBottom(std::string str) { notifyBottom(&str[0]); }

void notifyMap(char * fmt, ...)
{
	char buf[2048] = { 0 };
	va_list va_alist;

	va_start(va_alist, fmt);
	vsprintf_s(buf, fmt, va_alist);
	va_end(va_alist);

	char buff2[2048] = { 0 };
	sprintf_s(buff2, "%s", buf);

	UI::SET_TEXT_OUTLINE();
	UI::_SET_NOTIFICATION_TEXT_ENTRY("STRING");
	UI::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME(buff2);
	UI::_DRAW_NOTIFICATION(FALSE, FALSE);
}	void notifyMap(std::string str) { notifyMap(&str[0]); }

void notifyCenter(char * fmt, ...)
{
	char buf[2048] = { 0 };
	va_list va_alist;

	va_start(va_alist, fmt);
	vsprintf_s(buf, fmt, va_alist);
	va_end(va_alist);

	sprintf_s(status_text, "%s", buf);
}	void notifyCenter(std::string str) { notifyCenter(&str[0]); }

//DRAWING FUNCTIONS
void draw_rect(float A_0, float A_1, float A_2, float A_3, int A_4, int A_5, int A_6, int A_7)
{
	GRAPHICS::DRAW_RECT((A_0 + (A_2 * 0.5f)), (A_1 + (A_3 * 0.5f)), A_2, A_3, A_4, A_5, A_6, A_7);
}

void draw_menu_line(std::string caption, float lineWidth, float lineHeight, float lineTop, float lineLeft, float textLeft, bool active, bool title, bool rescaleText)
{
	// default values
	int text_col[4] = { 255, 255, 255, 255 },
		rect_col[4] = { 0, 0, 0, 80 };
	float text_scale = 0.30f;
	int font = 0;
	bool outline = true;
	bool dropshadow = true;

	// correcting values for active line
	if (active)
	{
		
		outline = false;
		dropshadow = true;
		text_col[0] = 0;
		text_col[1] = 0;
		text_col[2] = 0;
		text_col[3] = 255;

		rect_col[0] = 255;
		rect_col[1] = 255;
		rect_col[2] = 255;
		rect_col[3] = 80;

		if (rescaleText)text_scale = 0.30f;
	}

	if (title)
	{
		outline = true;
		dropshadow = true;
		text_col[0] = 255;
		text_col[1] =255;
		text_col[2] = 255;
		text_col[3] = 255;

		rect_col[0] = 93;
		rect_col[1] = 182;
		rect_col[2] = 229;
		rect_col[2] = 255;

		if (rescaleText)text_scale = 0.36f;
		font = 0;
	}

	int screen_w, screen_h;
	GRAPHICS::GET_SCREEN_RESOLUTION(&screen_w, &screen_h);

	textLeft += lineLeft;

	float lineWidthScaled = lineWidth / (float)screen_w; // line width
	float lineTopScaled = lineTop / (float)screen_h; // line top offset
	float textLeftScaled = textLeft / (float)screen_w; // text left offset
	float lineHeightScaled = lineHeight / (float)screen_h; // line height

	float lineLeftScaled = lineLeft / (float)screen_w;

	// this is how it's done in original scripts

	// text upper part
	UI::SET_TEXT_FONT(font);
	UI::SET_TEXT_SCALE(0.0, text_scale);
	UI::SET_TEXT_COLOUR(text_col[0], text_col[1], text_col[2], text_col[3]);
	UI::SET_TEXT_CENTRE(0);
	if (outline)
	{
		UI::SET_TEXT_EDGE(1, 255, 215, 0, 255); //UI::SET_TEXT_OUTLINE();
	}

	if (dropshadow)
	{
		UI::SET_TEXT_DROPSHADOW(5, 0, 78, 255, 255);
	}
	UI::SET_TEXT_EDGE(0, 0, 0, 0, 0);
	UI::BEGIN_TEXT_COMMAND_DISPLAY_TEXT("STRING");
	UI::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME((LPSTR)caption.c_str());
	UI::END_TEXT_COMMAND_DISPLAY_TEXT(textLeftScaled, (((lineTopScaled + 0.00278f) + lineHeightScaled) - 0.005f));

	// text lower part
	UI::SET_TEXT_FONT(font);
	UI::SET_TEXT_SCALE(0.0, text_scale);
	UI::SET_TEXT_COLOUR(text_col[0], text_col[1], text_col[2], text_col[3]);
	UI::SET_TEXT_CENTRE(0);
	if (outline)
	{
		UI::SET_TEXT_EDGE(1, 255, 215, 0, 255); //UI::SET_TEXT_OUTLINE();
	}

	if (dropshadow)
	{
		UI::SET_TEXT_DROPSHADOW(5, 0, 78, 255, 255);
	}
	UI::SET_TEXT_EDGE(0, 0, 0, 0, 0);
	UI::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME((LPSTR)caption.c_str());
	int num25 = UI::_GET_TEXT_SCREEN_LINE_COUNT(textLeftScaled, (((lineTopScaled + 0.00278f) + lineHeightScaled) - 0.005f));

	// rect
	draw_rect(lineLeftScaled, lineTopScaled + (0.00278f),
		lineWidthScaled, ((((float)(num25)* UI::_GET_TEXT_SCALE_HEIGHT(text_scale, 0)) + (lineHeightScaled * 1.0f)) + 0.005f),
		rect_col[0], rect_col[1], rect_col[2], rect_col[3]);
}



























































#define _148

//namespace hax {

uint64_t getBaseAddress() {
	//return (uint64_t)GetModuleHandleA("GTA.exe");
	return (uint64_t)GetModuleHandle(NULL);
}

namespace addresses {
#ifdef _143
#define address_base									(getBaseAddress()-(0x7FF791521000 - 0x1000))
#endif

#ifdef _144
#define address_base									(getBaseAddress()-(0x7FF6E5031000 - 0x1000))
#endif

#ifdef _146
#define address_base									(getBaseAddress()-(0x7FF6A17A1000 - 0x1000))
#endif

#ifdef _148
#define address_base									(getBaseAddress()-(0x7FF6EFA61000 - 0x1000))
#endif

	/*#define address_processMetric							(0x7FF7924F1EC8 + address_base)
	#define address_NETWORK_INCREMENT_STAT_EVENT_local		(0x7FF7926309C0 + address_base)
	#define address_Return3									(0x7FF79164700C + address_base)*/
	uint64_t
		NETWORK_INCREMENT_STAT_EVENT_local,
		getCNetGamePlayer,
		NETWORK_PLAYER_GET_NAME,
		sendSocialClubEventJson,
		sendTheSCEvent,
		processMetric,
		REPORT_CASH_SPAWN_EVENT_local,
		REPORT_MYSELF_EVENT_local,
		REPORT_CASH_SPAWN_EVENT,
		REPORT_MYSELF_EVENT,
		NETWORK_INCREMENT_STAT_EVENT,
		amIOnline,
		getCNetGamePlayer2,
		NETWORK_IS_SIGNED_ONLINE,
		amISignedIn,
		amISignedInChar,
		getSingleplayerPlayerData,
		localPlayerData,
		getCVehicleModelInfo,
		localPlayerDataStructForSending,
		createCashSpawnMetric
		;

};

#define getAddressOfFunction(sig) \
	(uint64_t)(void*)pattern(sig).count(1).get(0).get<void>(0);

#define getAddressOfFunctionWithIndex(sig, index) \
	(uint64_t)(void*)pattern(sig).count(1+index).get(index).get<void>(0);





bool didSetup = false;
bool isReadyForHax() {
	return didSetup;
}
void *getCNetGamePlayer(int player) {
	void *playerAddr = ((void*(*)(int, int))(addresses::getCNetGamePlayer))(player, 1);
	return playerAddr;
}





#define PACKED
#pragma pack(push,1)
struct playerDataStruct {//size of 0xF0
	uint8_t bytes[0x55];
	uint8_t name[50];
};
#pragma pack(pop)
#undef PACKED

#pragma warning(disable : 4996)






#ifdef _143
#define playerStructData ((void*)*(uint64_t*)(address_base+0x7FF793A636C8))
#define playerStructCount (*(short*)(address_base+0x7FF793A636D0))
#endif

#ifdef _144
#define playerStructData ((void*)*(uint64_t*)(address_base+0x7FF6E75A5ED0))
#define playerStructCount (*(short*)(address_base+0x7FF6E75A5ED8))   //search for sig "40 53 48 83 EC 20 0F B7 05 ? ? ? ? 33 DB 66 85 C0 74 1B 4C 8B C1 48 8B 0D ? ? ? ? 0F B7 D0 E8 ? ? ? ? 66 89 1D ? ? ? ? B0 01 48 83 C4 20 5B C3 " and it's in this function
#endif

#ifdef _146
#define playerStructData ((void*)*(uint64_t*)(address_base+0x7FF6A3D86860))
#define playerStructCount (*(short*)(address_base+0x7FF6A3D86868))
#define playerFindAmount (*(int*)(address_base+0x7FF6A27ABF3A))
#endif

#ifdef _148
#define temp_addr_psd 0x7FF6F205EC88   //search for sig "40 53 48 83 EC 20 0F B7 05 ? ? ? ? 33 DB 66 85 C0 74 1B 4C 8B C1 48 8B 0D ? ? ? ? 0F B7 D0 E8 ? ? ? ? 66 89 1D ? ? ? ? B0 01 48 83 C4 20 5B C3 " and it's in this function
#define playerStructData ((void*)*(uint64_t*)(address_base+temp_addr_psd-8))
#define playerStructCount (*(short*)(address_base+temp_addr_psd))
//#define playerFindAmount (*(int*)(address_base+0x7FF6A27ABF3A))
#endif


void setPlayerAmount(int amt) {
	//playerFindAmount = amt;
}
int getPlayerAmount() {
	//return playerFindAmount;
	return -1;
}



char *doublePointerEmail = "gta5email";//CEventNetworkEmailReceivedEvent
char *doublePointerEmailMarketing = "gta5marketing";//CEventNetworkMarketingEmailReceivedEvent
bool useMarketing = false;
void sendEmail(int *playerData, char *jsonData) {
	char **valueToPass = useMarketing ? &doublePointerEmailMarketing : &doublePointerEmail;

	SOCIALCLUB::_0xDA024BDBD600F44A(playerData);//adds player to it

	((void(*)(DWORD, void *, DWORD, char *, char **, DWORD64, DWORD64))(addresses::sendTheSCEvent))(0, playerStructData, playerStructCount, jsonData, valueToPass,useMarketing?0x00007FF700000001:1,0x278D00);
	playerStructCount = 0;
}


void sendSCEventNoTime(int *playerData/*char *playerName*/, char *jsonData) {
	int playerCount = 1;

	//notifyMap("a2");
	SOCIALCLUB::_0xDA024BDBD600F44A(playerData);


	//notifyMap("a3");
	((void(*)(DWORD, void *, DWORD, char *, DWORD))(addresses::sendSocialClubEventJson))(0, playerStructData, playerStructCount, jsonData, 0);
	playerStructCount = 0;
	//notifyMap("Sent SC");

}

uint64_t getMemberIDOfPlayer(int *networkStruct) {
	char *memberID = NETWORK::NETWORK_MEMBER_ID_FROM_GAMER_HANDLE(networkStruct);
	uint64_t value;
	std::istringstream iss(memberID);
	iss >> value;
	return value;
}

struct sendingPlayerIDStruct {
	uint64_t memberid;//remember, little endian format is retarded :)
	uint64_t three;//no idea what it is used for but it is
};

void getStructOfPlayerNetworkData(int *networkStruct, sendingPlayerIDStruct *saveTo, int messageIndex = 3) {
	//on doing mine, the return should be a4 6f 1d 07 00 00 00 00 03 00 00 00 00 00 00 00 aka pG8dBwAAAAADAAAAAAAAAA==
	saveTo->memberid = getMemberIDOfPlayer(networkStruct);
	saveTo->three = messageIndex;//I tried using 0,1,2,and 4 but none of those made anything show up
}

std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len);
void sendTextToPlayerAnonymous(char *message, int *networkStruct, int messageIndex) {
	//I can't send a mass message since each string has to be different in player data :(
	//setup stuff
	const char *fmt = "\"ros.publish\":{\"channel\":\"self\",\"msg\":{\"gm.evt\":{\"e\":\"TextMessage\",\"d\":{\"ps\":\"%s\",\"gh\":\"%s\"}}}}";
	char buf[500];
	char ghBuf[100];
	//get the formatted sender info for the gh param
	sendingPlayerIDStruct senderInfo;
	getStructOfPlayerNetworkData(networkStruct, &senderInfo, messageIndex);
	//senderInfo.memberid = 67252214;//username "rockstar"
	std::string r = base64_encode((const unsigned char *)&senderInfo, 16);
	strcpy(ghBuf, r.c_str());
	//notifyMap("New buffer: %s",ghBuf);//output test
	//format json string
	snprintf(buf, sizeof(buf), fmt, message, ghBuf);
	sendSCEventNoTime(networkStruct, buf);
}

bool useRockstarName = false;
char *sValue = "test";
int iValue = 0;
void sendEmailToPlayerAnonymous(char *subject, char *message, int *networkStruct, int messageIndex) {
	//I can't send a mass message since each string has to be different in player data :(
	//setup stuff
	//const char *fmt = "{\"email\":{\"gh\":\"%s\",\"sb\":\"%s\",\"cn\":\"%s\",\"i\":\"%i\",\"s\":\"%s\"}}";//sender,subject,message,i,s
	const char *fmt = "{\"email\":{\"gh\":\"%s\",\"sb\":\"%s\",\"cn\":\"%s\"}}";//sender,subject,message
	char buf[500];
	char ghBuf[100];
	//get the formatted sender info for the gh param
	sendingPlayerIDStruct senderInfo;
	getStructOfPlayerNetworkData(networkStruct, &senderInfo, messageIndex);
	if (useRockstarName) {
		senderInfo.memberid = 67252214;//username "rockstar"
		senderInfo.memberid = 108160633;//Liam4329
	}
	std::string r = base64_encode((const unsigned char *)&senderInfo, 16);
	strcpy(ghBuf, r.c_str());
	//notifyMap("New buffer: %s",ghBuf);//output test
	//format json string
	snprintf(buf, sizeof(buf), fmt, ghBuf, subject, message/*, iValue, sValue*/);
	sendEmail(networkStruct, buf);
}






static const std::string base64_chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";


static inline bool is_base64(unsigned char c) {
	return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len) {
	std::string ret;
	int i = 0;
	int j = 0;
	unsigned char char_array_3[3];
	unsigned char char_array_4[4];

	while (in_len--) {
		char_array_3[i++] = *(bytes_to_encode++);
		if (i == 3) {
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for (i = 0; (i <4); i++)
				ret += base64_chars[char_array_4[i]];
			i = 0;
		}
	}

	if (i)
	{
		for (j = i; j < 3; j++)
			char_array_3[j] = '\0';

		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;

		for (j = 0; (j < i + 1); j++)
			ret += base64_chars[char_array_4[j]];

		while ((i++ < 3))
			ret += '=';

	}

	return ret;

}





#include "Memory.h"
using namespace Memory;

/*
typedef void(__cdecl* fpsendSocialClubEventJson)(DWORD, void *, DWORD, char *, DWORD);
fpsendSocialClubEventJson OG_sendSocialClubEventJson = nullptr;
void __cdecl HK_sendSocialClubEventJson(DWORD unk1, void *playerStruct, DWORD playerCount, char *jsonData, DWORD unk2)
{
	notifyMap("%s", jsonData);
	//notifyMap("%s", jsonData + 0x20);//just try to print more off
	
	appendInFile(jsonData);
	OG_sendSocialClubEventJson(unk1, playerStruct, playerCount, jsonData, unk2);
}*/

typedef void(__cdecl* fpsendTheSCEvent)(void *a, void *b, void *c, char *jsonData, void *e, void *f, void *g);
fpsendTheSCEvent OG_sendTheSCEvent = nullptr;
void __cdecl HK_sendTheSCEvent(void *a, void *b, void *c, char *jsonData, char **e, void *f, void *g)
{
	appendInFile("%p %p %p %s %s %p %p\n", a, b, c, jsonData, *e, f, g);

	notifyMap("%s", jsonData);//this was only part of it

	OG_sendTheSCEvent(a, b, c, jsonData, e, f, g);
}



typedef void(__cdecl* fpgetEmailJsonData)(void *a, void *b, void *c, void *d, void *e, void *f, void *g);
fpgetEmailJsonData OG_getEmailJsonData = nullptr;
void __cdecl HK_getEmailJsonData(unsigned char *a, void *b, void *c, void *d, void *e, void *f, void *g)
{

	uint64_t addr = (uint64_t)b;
	char *str = (char*)(*(uint64_t*)addr);//thhis should work



	//appendInFile("%p %p %p %s %s %p %p\n", a, b, c, d, e, f, g);
	notifyMap("Received something email!\n%s",str);
	OG_getEmailJsonData(a, b, c, d, e, f, g);

	std::ofstream outfile;
	outfile.open("C:\\Users\\User\\Documents\\FTPServer\\thelog.txt", std::ios_base::app);

	outfile << appendInFile2("Data: %s  ---- ",str);

	for (int i = 0; i < 0x3F0; i++) {
		outfile << appendInFile2("%02X", a[i]);
		if ((i + 1) % 4 == 0)
			outfile << appendInFile2(" ");
	}
	outfile << appendInFile2("\n");

}


void findAddresses() {
	//basically, read base + these values + 0x1000 = the address in mem
	addresses::sendSocialClubEventJson = (uint64_t)pattern("48 8B C4 48 89 58 08 48 89 68 10 48 89 70 18 4C 89 70 20 41 57 48 83 EC 40 44 8B F9 48 8D 48 E8 49 8B F1 41 8B E8 4C 8B F2 32 DB E8 ? ? ? ? 84 C0 74 36 8B 44 24 70 48 8B 4C 24 ? 44 8B CD 89 44 24 28 4D 8B C6 41 8B D7 48 89 74 24 ? E8 ? ? ? ? 3A C3 74 12 48 8B 4C 24 ? E8 ? ? ? ? 3A C3 74 04 B3 01 EB 0A 48 8B 4C 24 ? E8 ? ? ? ? 48 8B 6C 24 ? 48 8B 74 24 ? 4C 8B 74 24 ? 8A C3 48 8B 5C 24 ? 48 83 C4 40 41 5F C3 ").count(1).get(0).get<void>(0);//address_base + 0x7FF6E63D8310;
																																																																																																																													   //addresses::localPlayerDataStructForSending = address_base + 0x7FF6E782A5D0; //https://i.imgur.com/v4dgxjo.png found from finding "/members/%s/%s/%s" and it's a few above it             used in text messaging and metrics reporting
	addresses::getCNetGamePlayer = getAddressOfFunction("48 83 EC 28 33 C0 38 05 ? ? ? ? 74 0A 83 F9 1F 77 05 E8 ? ? ? ? 48 83 C4 28 C3 ");//address_base + 0x7FF6E5A3FD40;
	
	addresses::sendTheSCEvent = getAddressOfFunction("48 8B C4 48 89 58 08 48 89 68 10 48 89 70 18 48 89 78 20 41 54 41 56 41 57 48 83 EC 50 33 DB 49 8B E9 45 8B F0 4C 8B FA 8B F1 85 C9 0F 85 ? ? ? ? 8A 05 ? ? ? ? 4C 8D 25 ? ? ? ? F6 D8 48 1B C9 49 23 CC E8 ? ? ? ? 48 8B F8 48 85 C0 74 7C 48 05 ? ? ? ? 45 8B CE 4D 8B C7 48 89 44 24 ? 8B 84 24 ? ? ? ? 8B D6 89 44 24 38 8B 84 24 ? ? ? ? 48 8B CF 89 44 24 30 48 8B 84 24 ? ? ? ? 48 89 44 24 ? 48 89 6C 24 ? E8 ? ? ? ? 40 8A 35 ? ? ? ? 84 C0 74 1D 40 8A C6 4C 8B C7 F6 D8 48 1B C9 33 D2 49 23 CC E8 ? ? ? ? 84 C0 74 04 B3 01 EB 11 40 F6 DE 48 8B D7 48 1B C9 49 23 CC E8 ? ? ? ? 4C 8D 5C 24 ? 8A C3 49 8B 5B 20 49 8B 6B 28 49 8B 73 30 49 8B 7B 38 49 8B E3 41 5F 41 5E 41 5C C3 ");//this one is actually 2 down, because the x64 function spliits it up more than the pc version, so xref it twice to get real address I use now getAddressOfFunction("48 8B C4 48 89 58 08 48 89 68 10 48 89 70 18 48 89 78 20 41 56 48 81 EC ? ? ? ? 41 8B E9 4D 8B F0 48 63 DA 48 8B F9 85 D2 0F 85 ? ? ? ? 8B D3 E8 ? ? ? ? 84 C0 0F 84 ? ? ? ? 4C 8B C3 48 8D 05 ? ? ? ? 4D 69 C0 ? ? ? ? 4C 03 C0 41 83 B8 ? ? ? ? ? 0F 86 ? ? ? ? 83 4C 24 ? ? 83 64 24 ? ? 48 8D 15 ? ? ? ? 41 B1 01 48 8B CF E8 ? ? ? ? 84 C0 0F 84 ? ? ? ? 48 8D B7 ? ? ? ? 48 8D 15 ? ? ? ? 48 8B CE E8 ? ? ? ? 84 C0 0F 84 ? ? ? ? 33 DB 85 ED 7E 35 48 63 CB 48 8D 54 24 ? 41 B8 ? ? ? ? 48 C1 E1 04 49 03 CE E8 ? ? ? ? 48 8D 54 24 ? 48 8B CE E8 ? ? ? ? 84 C0 0F 84 ? ? ? ? FF C3 3B DD 7C CB 4C 8B 84 24 ? ? ? ? 83 4C 24 ? ? 83 64 24 ? ? 48 8D 15 ? ? ? ? 41 B1 01 48 8B CF E8 ? ? ? ? 84 C0 74 5F 48 8D 15 ? ? ? ? 48 8B CE E8 ? ? ? ? 84 C0 74 4C 8B AC 24 ? ? ? ? 33 DB 85 ED 7E 21 48 8B BC 24 ? ? ? ? 48 8B 17 48 8B CE E8 ? ? ? ? 84 C0 74 28 FF C3 48 83 C7 08 3B DD 7C E7 44 8B 84 24 ? ? ? ? 48 8D 15 ? ? ? ? 48 8B CE E8 ? ? ? ? 84 C0 0F 95 C0 EB 02 32 C0 4C 8D 9C 24 ? ? ? ? 49 8B 5B 10 49 8B 6B 18 49 8B 73 20 49 8B 7B 28 49 8B E3 41 5E C3 ");

	//MH_STATUS status = MH_CreateHook((void*)addresses::sendSocialClubEventJson, HK_sendSocialClubEventJson, (void**)&OG_sendSocialClubEventJson);
	//MH_EnableHook((void*)addresses::sendSocialClubEventJson);
	MH_STATUS status = MH_CreateHook((void*)addresses::sendTheSCEvent, HK_sendTheSCEvent, (void**)&OG_sendTheSCEvent);
	MH_EnableHook((void*)addresses::sendTheSCEvent);



	//DWORD64 getEmailJsonData = address_base + 0x7FF6A280B1DC;
	//MH_STATUS status2 = MH_CreateHook((void*)getEmailJsonData, HK_getEmailJsonData, (void**)&OG_getEmailJsonData);
	//MH_EnableHook((void*)getEmailJsonData);


	//*(uint64_t*)(address_base + 0x7FF6A34F2150) = (uint64_t)(doublePointerEmailMarketing);//that should work...

}





void setup() {
	findAddresses();
	didSetup = true;
}

//};





/*
Color syntax:
~r~ = Red
~b~ = Blue
~g~ = Green
~y~ = Yellow
~p~ = Purple
~o~ = Orange
~c~ = Grey
~m~ = Darker Grey
~u~ = Black
~n~ = New Line
~s~ = Default White
~w~ = White
~h~ = Bold Text
~nrt~ = ???

Special characters:
¦ = Rockstar Verified Icon (U+00A6:Broken Bar - Alt+0166)
÷ = Rockstar Icon (U+00F7:Division Sign - Alt+0247)
∑ = Rockstar Icon 2 (U+2211:N-Ary Summation)
*/




bool dontSendOnlyFind = false;

int var1 = 1;
float var2 = 0;
float var3 = -1.0f;
float var4 = -1.0f;

char *messageSendText = "<font size='40'>~y~NOTICE\n~s~Wendy's Spicy Nuggets are back!";


int findStatus = 4;
int totalFoundGamers = 0;

time_t startSearchTime = 0;

void findGamers() {
	static int currentFoundAmt = 0;
	static int currentCount = 0;



	switch (findStatus) {
	case 0:
		//if (NETWORK::NETWORK_IS_PLAYER_CONNECTED(PLAYER::PLAYER_ID())) {//maybe 7FF792F02678 is the value for I_AM_ONLINE?
		//NETWORK::NETWORK_FIND_MATCHED_GAMERS(1, 0, -1.0, -1.0);
		NETWORK::NETWORK_FIND_MATCHED_GAMERS(var1, var2, var3, var4);
		findStatus = 1;
		currentFoundAmt = 0;
		currentCount = 0;
		//}
		break;
	case 1:
		if (!NETWORK::NETWORK_IS_FINDING_GAMERS()) {
			//print2("Currently Advertising!",5000);
			if (NETWORK::_0xF9B83B77929D8863()) {
				int num = NETWORK::NETWORK_GET_NUM_FOUND_GAMERS();
				currentFoundAmt = num;
				currentCount = 0;
				totalFoundGamers += num;
				//notifyMap("Found %i gamers\n", num);
				findStatus = 2;
				if (dontSendOnlyFind == true) {
					findStatus = 3;
				}
			}
			else {
				//printf("Error with the gamer finding\n");
				findStatus = 3;
			}
		}
		else {
			//printf("Finding\n");
		}
		break;
	case 2:
		//loop through results





		while (currentCount < currentFoundAmt) {
			int gamer[13 * 5];
			if (NETWORK::NETWORK_GET_FOUND_GAMER(gamer, currentCount)) {
				if (NETWORK::NETWORK_IS_HANDLE_VALID(gamer, 13)) {

					//NETWORK::NETWORK_GAMERTAG_FROM_HANDLE_START(gamer);//depending on how you get the handle, you might have to do this.
					//while (NETWORK::NETWORK_GAMERTAG_FROM_HANDLE_PENDING())WAIT(1);
					//if (!NETWORK::NETWORK_GAMERTAG_FROM_HANDLE_SUCCEEDED())
					//	notifyMap("Getting gamertag failed!");
					//char *name = NETWORK::NETWORK_GET_GAMERTAG_FROM_HANDLE(gamer);
					char *memberID = NETWORK::NETWORK_MEMBER_ID_FROM_GAMER_HANDLE(gamer);
					//notifyMap("A1: %s A2: %i", memberID, NETWORK::NETWORK_IS_HANDLE_VALID(gamer, 13));//this used to be uncommented!
					//<font size='25'>~y~NOTICE\n<font size='15'>~s~gta got ligma lol
					//<font size='40'>~y~NOTICE\n~s~Subscribe To Pewdiepie on Youtube
					//<font size='40'>~y~NOTICE\n~s~Wendy's Spicy Nuggets are back!
					sendTextToPlayerAnonymous(messageSendText, gamer);//this should work just fine

				}
			}
			currentCount++;
		}// else {
		findStatus = 3;
		//}




		break;
	case 3:
		NETWORK::NETWORK_CLEAR_FOUND_GAMERS();
		findStatus = 0;
		time_t dif = time(0) - startSearchTime;
		notifyMap("Found: %i\nTotal: %i (in %i time)\n", currentFoundAmt, totalFoundGamers,(int)dif);
		break;

	}
}

void findGamersInit() {
	findStatus = 0;
	startSearchTime = time(0);
	notifyMap("Message to be sent:\n%s", messageSendText);
}
void findGamersStop() {
	findStatus = 4;
	//printf("Total: %i\n",totalFoundGamers);
}




void create_vehicle(char *vehicleID, bool spawnmaxed, bool spawnincar) {
	Hash model = GAMEPLAY::GET_HASH_KEY(vehicleID);
	if (STREAMING::IS_MODEL_VALID(model))
	{

		//this iis the part that loads it that is diifferent on PS3 because ps3 is a hook and pc is a thread so we can have the while loop on pc
		STREAMING::REQUEST_MODEL(model);
		while (!STREAMING::HAS_MODEL_LOADED(model)) WAIT(0);

		//calculate some values to make the car spawn at a nice position and angle
		Vector3 ourCoords = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), false);
		float forward = 5.f;
		float heading = ENTITY::GET_ENTITY_HEADING(PLAYER::PLAYER_PED_ID());
		float xVector = forward * sin(degToRad(heading)) * -1.f;
		float yVector = forward * cos(degToRad(heading));

		//spawn it
		Vehicle veh = VEHICLE::CREATE_VEHICLE(model, ourCoords.x + xVector, ourCoords.y + yVector, ourCoords.z, heading, true, true);

		//just some final values, not sure how useful they are
		RequestControlOfEnt(veh);
		VEHICLE::SET_VEHICLE_ENGINE_ON(veh, true, true, true);
		VEHICLE::SET_VEHICLE_ON_GROUND_PROPERLY(veh);
		DECORATOR::DECOR_SET_INT(veh, "MPBitset", 0);
		auto networkId = NETWORK::VEH_TO_NET(veh);
		ENTITY::_SET_ENTITY_SOMETHING(veh, true);
		if (NETWORK::NETWORK_GET_ENTITY_IS_NETWORKED(veh))
			NETWORK::SET_NETWORK_ID_EXISTS_ON_ALL_MACHINES(networkId, true);
		if (spawnincar)
		{
			PED::SET_PED_INTO_VEHICLE(PLAYER::PLAYER_PED_ID(), veh, -1);
		}
		if (spawnmaxed)
		{
			VEHICLE::SET_VEHICLE_MOD_KIT(veh, 0);
			for (int i = 0; i < 50; i++)
			{
				VEHICLE::SET_VEHICLE_MOD(veh, i, VEHICLE::GET_NUM_VEHICLE_MODS(veh, i) - 1, false);
			}
			VEHICLE::SET_VEHICLE_NUMBER_PLATE_TEXT(veh, "ayy");
		}
		WAIT(150);
		STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(model);
	}
}



void SET_UP_DRAW(float width, float height, uint r, uint g, uint b, uint a) {
	UI::SET_TEXT_FONT(7);
	//UI::SET_TEXT_BACKGROUND(0);
	UI::SET_TEXT_DROPSHADOW(0, 0, 0, 0, 255);
	UI::SET_TEXT_EDGE(1, 0, 0, 0, 255);
	UI::SET_TEXT_COLOUR(r, g, b, a);
	UI::SET_TEXT_SCALE(width, height);
	UI::SET_TEXT_PROPORTIONAL(1);
	UI::SET_TEXT_JUSTIFICATION(0);
	UI::SET_TEXT_CENTRE(0);
	UI::SET_TEXT_RIGHT_JUSTIFY(0);
}

void SET_UP_DRAW_1(float width, float height, uint r, uint g, uint b, uint a) {
	UI::SET_TEXT_FONT(5);
	//UI::SET_TEXT_BACKGROUND(0);
	UI::SET_TEXT_DROPSHADOW(0, 0, 0, 0, 255);
	UI::SET_TEXT_EDGE(1, 0, 0, 0, 255);
	UI::SET_TEXT_COLOUR(r, g, b, a);
	UI::SET_TEXT_SCALE(width, height);
	UI::SET_TEXT_PROPORTIONAL(1);
	UI::SET_TEXT_JUSTIFICATION(0);
	UI::SET_TEXT_CENTRE(0);
	UI::SET_TEXT_RIGHT_JUSTIFY(0);
}

void close_show(void) {
	SET_UP_DRAW(0.3100, 0.4550, 255, 0, 0, 255);
	UI::DISPLAY_TEXT_WITH_LITERAL_STRING(0.3100, 0.0340, "STRING", "CLOSE - ~PAD_RB~ + ~PAD_X~");
}