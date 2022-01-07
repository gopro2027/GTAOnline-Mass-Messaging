
#pragma once
#include "stdafx.h"

// This Menu Base is made by maxiZzModz it would be cool if you use this base when you mark me in the Credits as the Base Dev 
// These Base also support the Numpad,Arrow Key and Controller Support Openkey  Numpad * (Controller : Both Shoulderbuttons


//Nano 42 Is am Extension of Virtual base that works for the update 1.42 - Doomsday. All Credits goes to Cretor of MaxxizzModz, and other developer involved in creation of Virtual Base.
//This Base is modified and brought to 1.44 by AlphaModz

//To edit the Alpha.log file into YOURNAME.log go to file "Log.cpp"

bool TestBool = false;
int selectedPlayer;

char* CharKeyboard(char* windowName = "", int maxInput = 21, char* defaultText = "") {
	GAMEPLAY::DISPLAY_ONSCREEN_KEYBOARD(0, "", "", defaultText, "", "", "", maxInput);
	while (GAMEPLAY::UPDATE_ONSCREEN_KEYBOARD() == 0) WAIT(0);
	if (!GAMEPLAY::GET_ONSCREEN_KEYBOARD_RESULT()) return "";
	return GAMEPLAY::GET_ONSCREEN_KEYBOARD_RESULT();
}
int NumberKeyboard() {
	GAMEPLAY::DISPLAY_ONSCREEN_KEYBOARD(1, "", "", "", "", "", "", 10);
	while (GAMEPLAY::UPDATE_ONSCREEN_KEYBOARD() == 0) WAIT(0);
	if (!GAMEPLAY::GET_ONSCREEN_KEYBOARD_RESULT()) return 0;
	return atof(GAMEPLAY::GET_ONSCREEN_KEYBOARD_RESULT());
}
float FloatKeyboard() {
	GAMEPLAY::DISPLAY_ONSCREEN_KEYBOARD(1, "", "", "", "", "", "", 10);
	while (GAMEPLAY::UPDATE_ONSCREEN_KEYBOARD() == 0) WAIT(0);
	if (!GAMEPLAY::GET_ONSCREEN_KEYBOARD_RESULT()) return 0;
	return atof(GAMEPLAY::GET_ONSCREEN_KEYBOARD_RESULT());
}
void notifyleft(char* msg)
{
	UI::_SET_NOTIFICATION_TEXT_ENTRY("STRING");
	UI::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME(msg);
	UI::_DRAW_NOTIFICATION(2000, 1);
}

Vector3 TPCoords;
void TPto(Vector3 Coords)
{
	int Handle = PLAYER::PLAYER_PED_ID();
	if (PED::IS_PED_IN_ANY_VEHICLE(Handle, 0))
	{
		ENTITY::SET_ENTITY_COORDS(PED::GET_VEHICLE_PED_IS_IN(Handle, false), Coords.x, Coords.y, Coords.z, 0, 0, 0, 1);
	}
	else
		ENTITY::SET_ENTITY_COORDS(Handle, Coords.x, Coords.y, Coords.z, 0, 0, 0, 1);
}
void Features()
{


	globalHandle(262145).At(86).As<int>() = 999999;
	globalHandle(262145).At(87).As<int>() = 999999;
	globalHandle(262145).At(88).As<int>() = 999999;
	globalHandle(262145).At(89).As<int>() = 999999;


	if (TestBool)
	{
		//Code to Execute
	}
	else
	{
		//Code to Execute
	}
}




char *readTextFile(char *location) {
	FILE *f = fopen(location, "rb");
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	char *string = (char *)malloc(fsize + 1);
	fread(string, fsize, 1, f);
	fclose(f);

	string[fsize] = 0;

	return string;
}

void appendInFile(char * fmt, ...)
{
	char buf[2048] = { 0 };
	va_list va_alist;

	va_start(va_alist, fmt);
	vsprintf_s(buf, fmt, va_alist);
	va_end(va_alist);

	char buff2[2048] = { 0 };
	sprintf_s(buff2, "%s", buf);

	std::ofstream outfile;
	outfile.open("C:\\Users\\Ty\\Documents\\FTPServer\\thelog.txt", std::ios_base::app);
	outfile << buff2;
}

char *appendInFile2(char * fmt, ...)
{
	char buf[2048] = { 0 };
	va_list va_alist;

	va_start(va_alist, fmt);
	vsprintf_s(buf, fmt, va_alist);
	va_end(va_alist);

	char buff2[2048] = { 0 };
	sprintf_s(buff2, "%s", buf);

	return buff2;
}

/*
void antidebugger() {
	__asm("mov eax,dword ptr fs:[18]");
	__asm("mov eax, dword ptr ds : [eax + 30]");
	__asm("mov byte ptr ds : [eax + 2], 0");
}*/


bool firstload = true;
void main() {

	setup();
	if (firstload == true)
	{

		UI::_SET_NOTIFICATION_TEXT_ENTRY("STRING");
		UI::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME("Press '*' To Open");
		UI::_SET_NOTIFICATION_MESSAGE_CLAN_TAG_2("CHAR_SOCIAL_CLUB", "CHAR_SOCIAL_CLUB", true, 8, "~b~Ayy lmao", "How To Open", 1, "", 9, 0);
		UI::_DRAW_NOTIFICATION(FALSE, FALSE);
		firstload = false;
	}

	while (true) {
		findGamers();
		Menu::Checks::Controlls();
		Features();//anti-timeout in here
		//notifyMap("~f~Alpha Base loaded...");
		switch (Menu::Settings::currentMenu) {

		case mainmenu:
		{
			Menu::Title("Message Sender"/*"Alpha Base 1.44"*/);
			//Menu::MenuOption("Sub Menu", sub);
			//Menu::MenuOption("Player List", plist);
			//Menu::MenuOption("Settings", settings); //To add more submenus go to file "GUI.h" and add them in the list at the top
			//Menu::Option("");
			
			Menu::Toggle("Dont Send The Messages?", dontSendOnlyFind);
			Menu::Toggle("Use Rockstar Name?", useRockstarName);
			Menu::Toggle("Use Marketing?", useMarketing);
			
			if (Menu::Option("Start")) {
				findGamersInit();
			}
			if (Menu::Option("Stop")) {
				findGamersStop();
			}
			if (Menu::Option("Test send to self")) {
				int playerData[14];
				NETWORK::NETWORK_HANDLE_FROM_PLAYER(PLAYER::PLAYER_ID(), playerData, 13);
				//GAMEPLAY::DISPLAY_ONSCREEN_KEYBOARD(true, "Enter Message", "", "", "", "", "", 64);
				//while (GAMEPLAY::UPDATE_ONSCREEN_KEYBOARD() == 0) WAIT(0);
				//char *message = (char*)_strdup(GAMEPLAY::GET_ONSCREEN_KEYBOARD_RESULT());

				char *message = messageSendText;
				//notifyMap("Message: %s", message);
				sendTextToPlayerAnonymous(message, playerData);
			}
			if (Menu::Option("Test send to self (email)")) {
				int playerData[14];
				NETWORK::NETWORK_HANDLE_FROM_PLAYER(PLAYER::PLAYER_ID(), playerData, 13);

				char *message = messageSendText;
				char *subject = "SSSS";
				sendEmailToPlayerAnonymous(subject, message, playerData);
				notifyMap("Message: %s", message);
			}
			if (Menu::Option("Load message from message.txt")) {
				messageSendText = readTextFile("C:\\Users\\Ty\\Documents\\FTPServer\\message.txt");
				notifyMap("Message loaded!");
			}
			if (Menu::Int("iValue", iValue, -200, 200))
			{
				if (IsKeyPressed(VK_NUMPAD5) || CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, ControlFrontendAccept)) {
					iValue = NumberKeyboard();
				}
			}
			if (Menu::Option("sValue")) {
				GAMEPLAY::DISPLAY_ONSCREEN_KEYBOARD(true, "Enter Message", "", sValue, "", "", "", 20);
				while (GAMEPLAY::UPDATE_ONSCREEN_KEYBOARD() == 0) WAIT(0);
				sValue = (char*)_strdup(GAMEPLAY::GET_ONSCREEN_KEYBOARD_RESULT());
				notifyMap("New sValue: %s",sValue);
			}
			if (Menu::Option("Send email to self from text file")) {
				char *buf = readTextFile("C:\\Users\\Ty\\Documents\\FTPServer\\json_string_email.txt");
				notifyMap(buf);
				int playerData[14];
				NETWORK::NETWORK_HANDLE_FROM_PLAYER(PLAYER::PLAYER_ID(), playerData, 13);
				sendEmail(playerData, buf);
			}
			/*if (Menu::Int("Var 1", var1, -200, 200))
			{
				if (IsKeyPressed(VK_NUMPAD5) || CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, ControlFrontendAccept)) {
					var1 = NumberKeyboard();
				}
			}
			if (Menu::Float("Var 2", var2, -200, 200))
			{
				if (IsKeyPressed(VK_NUMPAD5) || CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, ControlFrontendAccept)) {
					var2 = FloatKeyboard();
				}
			}
			if (Menu::Float("Var 3", var3, -200, 200))
			{
				if (IsKeyPressed(VK_NUMPAD5) || CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, ControlFrontendAccept)) {
					var3 = FloatKeyboard();
				}
			}
			if (Menu::Float("Var 4", var4, -200, 200))
			{
				if (IsKeyPressed(VK_NUMPAD5) || CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, ControlFrontendAccept)) {
					var4 = FloatKeyboard();
				}
			}*/
			static int counterO = 0;
			counterO = getPlayerAmount();
			if (Menu::Int("Player find count", counterO, -10000, 10000))
			{
				//playerFindAmount
				//0x7FF6A27ABF3A
				if (IsKeyPressed(VK_NUMPAD5) || CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, ControlFrontendAccept)) {
					counterO = NumberKeyboard();
					setPlayerAmount(counterO);
				}
				setPlayerAmount(counterO);
			}
		}
		break;
#pragma region Self Menu
		case plist:
		{
			Menu::Title("Player List");
			for (int i = 0; i < 32; ++i) {
				if (ENTITY::DOES_ENTITY_EXIST(PLAYER::GET_PLAYER_PED_SCRIPT_INDEX(i))) {
					Menu::MenuOption(PLAYER::GET_PLAYER_NAME(i), pmenu) ? selectedPlayer = i : NULL;
				}
			}
		}
			break;
		case pmenu:
		{
			Menu::Title(PLAYER::GET_PLAYER_NAME(selectedPlayer));
			Menu::Break("It Works!");
		}
			break;
		case sub :
		{
			Menu::Title("Sub Menu");
			Menu::Toggle("Test Bool", TestBool);
			//To use functions do _>
			if (Menu::Option("Toggle Function"))
			{
				//Code to execute
			}
			//SubMenu
		}
		break;
#pragma endregion
#pragma region Settings Menu
		case settings:
		{
			Menu::Title("Settings");
			Menu::MenuOption("Colors ~b~>", settingstheme);
			if (Menu::Int("Scroll Delay", Menu::Settings::keyPressDelay2, 1, 200))
			{
				if (IsKeyPressed(VK_NUMPAD5) || CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, ControlFrontendAccept)) {
					Menu::Settings::keyPressDelay2 = NumberKeyboard();
				}
			}
			if (Menu::Int("Int Delay", Menu::Settings::keyPressDelay3, 1, 200))
			{
				if (IsKeyPressed(VK_NUMPAD5) || CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, ControlFrontendAccept)) {
					Menu::Settings::keyPressDelay3 = NumberKeyboard();
				}
			}
			Menu::Option("~HUD_COLOUR_GOLD~Social Club Account:");
			Menu::Option(PLAYER::GET_PLAYER_NAME(PLAYER::PLAYER_ID()));
			Menu::MenuOption("~r~KILL GAME", exitgta);
		}
		break;
		case Credits:
		{
			Menu::Title("Credits");
			Menu::Option("Base : ~r~maxiZzModz");
			Menu::Option("Modified : ~f~CptShad");
			Menu::Option("Modified : ~f~AlphaModz");
			Menu::Option("Thanks to:");
			Menu::Option("~b~LHDModz");
			Menu::Option("~p~Mike Rohsoft");
			Menu::Option("Taran VG");
			Menu::Option("~r~Name");
			Menu::Option("V. 0.0");
		}
		break;
		case exitgta:
		{
			Menu::Title("Exit GTA V");
			if (Menu::Option("Yes")) exit(0);
		}
		break;
		case settingstheme:
		{
			Menu::Title("Colors");
			Menu::MenuOption("Theme Loader ~b~>", themeloader);
			Menu::MenuOption("Title Background ~b~>", settingstitlerect);
			Menu::MenuOption("Selection Box ~b~>", settingsscroller);
			Menu::MenuOption("Option Text ~b~>", settingsoptiontext);
			if (Menu::Option("MenuX plus")) {
				if (Menu::Settings::menuX < 0.81f) Menu::Settings::menuX += 0.01f;
			}
			if (Menu::Option("MenuX minus")) {
				if (Menu::Settings::menuX > 0.17f) Menu::Settings::menuX -= 0.01f;
			}
		}
		break;
		case themeloader:
		{
			Menu::Title("Theme Colors");
			if (Menu::Option("Red Theme")) {
				Menu::Settings::titleRect = { 100, 0, 0, 255 };
				Menu::Settings::scroller = { 100, 0, 0, 255 };
			}
			if (Menu::Option("Blue Theme")) {
				Menu::Settings::titleRect = { 0, 0, 200, 255 };
				Menu::Settings::scroller = { 0, 0, 200, 255 };
			}
			if (Menu::Option("Green Theme")) {
				Menu::Settings::titleRect = { 0, 180, 0, 255 };
				Menu::Settings::scroller = { 0, 0, 180, 255 };
			}
			if (Menu::Option("Load Default Theme")) {
				Menu::Settings::titleText = { 255, 255, 255, 255, 7 };
				Menu::Settings::titleRect = { 100, 0, 0, 255 };
				Menu::Settings::scroller = { 100, 0, 0, 255 };
				Menu::Settings::optionText = { 255, 255, 255, 255, 0 };
				Menu::Settings::optionRect = { 0, 0, 0, 110 };
			}
		}
		break;
		case settingstitlerect:
		{
			Menu::Title("Title Rect");
			if (Menu::Int("Red", Menu::Settings::titleRect.r, 0, 255))
			{
				if (IsKeyPressed(VK_NUMPAD5) || CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, ControlFrontendAccept)) {
					Menu::Settings::titleRect.r = NumberKeyboard();
				}
			}
			if (Menu::Int("Green", Menu::Settings::titleRect.g, 0, 255))
			{
				if (IsKeyPressed(VK_NUMPAD5) || CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, ControlFrontendAccept)) {
					Menu::Settings::titleRect.g = NumberKeyboard();
				}
			}
			if (Menu::Int("Blue", Menu::Settings::titleRect.b, 0, 255))
			{
				if (IsKeyPressed(VK_NUMPAD5) || CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, ControlFrontendAccept)) {
					Menu::Settings::titleRect.b = NumberKeyboard();
				}
			}
			if (Menu::Int("Opacity", Menu::Settings::titleRect.a, 0, 255))
			{
				if (IsKeyPressed(VK_NUMPAD5) || CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, ControlFrontendAccept)) {
					Menu::Settings::titleRect.a = NumberKeyboard();
				}
			}
		}
		break;
		case settingsoptiontext:
		{
			Menu::Title("Option Text");
			if (Menu::Int("Red", Menu::Settings::optionText.r, 0, 255))
			{
				if (IsKeyPressed(VK_NUMPAD5) || CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, ControlFrontendAccept)) {
					Menu::Settings::optionText.r = NumberKeyboard();
				}
			}
			if (Menu::Int("Green", Menu::Settings::optionText.g, 0, 255))
			{
				if (IsKeyPressed(VK_NUMPAD5) || CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, ControlFrontendAccept)) {
					Menu::Settings::optionText.g = NumberKeyboard();
				}
			}
			if (Menu::Int("Blue", Menu::Settings::optionText.b, 0, 255))
			{
				if (IsKeyPressed(VK_NUMPAD5) || CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, ControlFrontendAccept)) {
					Menu::Settings::optionText.b = NumberKeyboard();
				}
			}
			if (Menu::Int("Opacity", Menu::Settings::optionText.a, 0, 255))
			{
				if (IsKeyPressed(VK_NUMPAD5) || CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, ControlFrontendAccept)) {
					Menu::Settings::optionText.a = NumberKeyboard();
				}
			}
			Menu::MenuOption("Font ~b~>", font);
		}
		break;
		case font:
		{
			Menu::Title("Font");
			if (Menu::Option("Chalet London")) { Menu::Settings::optionText.f = 0; }
			if (Menu::Option("House Script")) { Menu::Settings::optionText.f = 1; }
			if (Menu::Option("Monospace")) { Menu::Settings::optionText.f = 2; }
			if (Menu::Option("Wing Dings")) { Menu::Settings::optionText.f = 3; }
			if (Menu::Option("Chalet Comprime Cologne")) { Menu::Settings::optionText.f = 4; }
			if (Menu::Option("Pricedown")) { Menu::Settings::optionText.f = 7; }
		}
		break;
		case settingsscroller:
		{
			Menu::Title("Scroller");
			if (Menu::Int("Red", Menu::Settings::scroller.r, 0, 255))
			{
				if (IsKeyPressed(VK_NUMPAD5) || CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, ControlFrontendAccept)) {
					Menu::Settings::scroller.r = NumberKeyboard();
				}
			}
			if (Menu::Int("Green", Menu::Settings::scroller.g, 0, 255))
			{
				if (IsKeyPressed(VK_NUMPAD5) || CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, ControlFrontendAccept)) {
					Menu::Settings::scroller.g = NumberKeyboard();
				}
			}
			if (Menu::Int("Blue", Menu::Settings::scroller.b, 0, 255))
			{
				if (IsKeyPressed(VK_NUMPAD5) || CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, ControlFrontendAccept)) {
					Menu::Settings::scroller.b = NumberKeyboard();
				}
			}
			if (Menu::Int("Opacity", Menu::Settings::scroller.a, 0, 255))
			{
				if (IsKeyPressed(VK_NUMPAD5) || CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, ControlFrontendAccept)) {
					Menu::Settings::scroller.a = NumberKeyboard();
				}
			}
		}
		break;
#pragma endregion
		}
		Menu::End();
		WAIT(0);
	}
}

void ScriptMain() {
	srand(GetTickCount());

	main();
}