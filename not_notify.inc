/*
	=====================================================
	NOT_NOTIFY.INC - Modular Visual Notification System
	=====================================================
	Author: not
	Version: 1.1
	Date: 2026
	License: MIT
	Credits / Base: Nero_3D (td-string-width)

	DESCRIPTION:
	Modular visual notification system for SA-MP and open.mp featuring 5
	independent HUD notification channels with slot-based timers, automatic
	line wrapping, and optional auto-replace.

	INCLUDED SYSTEMS:

	1. NOTIFICATIONS (Notify) - Bottom-left (Icon + Title + Message)
	   Functions: ShowNotify, DestroyNotifyByID, DestroyNotify, IsNotifyActive,
	              GetActiveNotifyCount, HideAllNotifications, UpdateNotifyText

	2. SUPPLIES (Supplies) - Top-right compact feed
	   Functions: ShowNotifySupplies, DestroyNotifySuppliesByID, DestroySupply,
	              IsSupplyActive, GetActiveSupplyCount, HideAllSupplies, UpdateSupplyText

	3. ADS (Ads) - Bottom-center prominent announcements
	   Functions: ShowNotifyADS, DestroyNotifyADSByID, DestroyAds, IsAdsActive,
	              GetActiveAdsCount, HideAllAds, UpdateAdsText

	4. MESSAGES (Messages) - Center above chat (custom font/shadow/outline)
	   Functions: ShowMessage, DestroyMessageByID, DestroyMessageManual,
	              IsMessageActive, GetActiveMessageCount, HideAllMessages, UpdateMessageText

	5. LOW MESSAGES (Low Messages) - Bottom-center below chat
	   Functions: ShowLowMessage, DestroyLowMessageByID, DestroyLowMessageManual,
	              IsLowMessageActive, GetActiveLowMessageCount, HideAllLowMessages, UpdateLowMessageText

	BASIC CONFIGURATION:
	```pawn
	// Optional: override max simultaneous slots per channel before including
	#define MAX_NORMAL_NOTIFY        5
	#define MAX_NOTIFY_SUPPLIES      3
	#define MAX_NOTIFY_ADS           2
	#define MAX_NOTIFY_MESSAGES      4
	#define MAX_NOTIFY_LOW_MESSAGES  3
	#include <not_notify>
	```

	USAGE EXAMPLE:
	```pawn
	new id = ShowNotify(playerid, "~g~Success", "Operation completed", 5000, "ld_beat:chit");
	UpdateNotifyText(playerid, id, "~y~Updated", "New text");
	DestroyNotify(playerid, id);
	```

	COMPATIBILITY:
	- SA-MP 0.3.7 and open.mp
	- Zero external dependencies (requires only <a_samp>)
	- Standard ALS hooks on OnPlayerConnect and OnPlayerDisconnect

	=====================================================
*/

#if defined _not_notify_included
	#endinput
#endif
#define _not_notify_included

#include <a_samp>

#if defined NOT_NOTIFY_DEBUG
	#tryinclude <textdraw-streamer>
#endif

#if !defined MAX_LENGTH_MESSAGE
	#define MAX_LENGTH_MESSAGE (512)
#endif
#if !defined INVALID_NOTIFY_ID
	#define INVALID_NOTIFY_ID (-1)
#endif

// Enable all 5 channels by default with safe defaults
#if !defined USE_NORMAL_NOTIFY
	#define USE_NORMAL_NOTIFY
#endif
#if !defined USE_NOTIFY_SUPPLIES
	#define USE_NOTIFY_SUPPLIES
#endif
#if !defined USE_NOTIFY_ADS
	#define USE_NOTIFY_ADS
#endif
#if !defined USE_NOTIFY_MESSAGES
	#define USE_NOTIFY_MESSAGES
#endif
#if !defined USE_NOTIFY_LOW_MESSAGES
	#define USE_NOTIFY_LOW_MESSAGES
#endif

#if !defined MAX_NORMAL_NOTIFY
	#define MAX_NORMAL_NOTIFY (5)
#endif
#if !defined MAX_NOTIFY_SUPPLIES
	#define MAX_NOTIFY_SUPPLIES (3)
#endif
#if !defined MAX_NOTIFY_ADS
	#define MAX_NOTIFY_ADS (2)
#endif
#if !defined MAX_NOTIFY_MESSAGES
	#define MAX_NOTIFY_MESSAGES (4)
#endif
#if !defined MAX_NOTIFY_LOW_MESSAGES
	#define MAX_NOTIFY_LOW_MESSAGES (3)
#endif

#if defined USE_NORMAL_NOTIFY
	#if !defined MAX_NORMAL_NOTIFY
		#error "define the maximum number of notifications to show -> (#define MAX_NORMAL_NOTIFY [value])"
	#endif
	#define MAX_WIDTH_MESSAGE			(775)	// Ancho maximo del mensaje (en pixels, 805 es el ancho de la pantalla)
	#if !defined MAX_LENGTH_ICON
		#define MAX_LENGTH_ICON (32)
	#endif
	#if !defined MAX_LENGTH_TITLE
		#define MAX_LENGTH_TITLE (44)
	#endif

	#define MAX_TEXTDRAW_NOTIFY			(4)		// Numero maximo de textdraw
	#define FONT_TEXT_NOTIFY			(1)		// Fuente del texto
	#define PROPORTIONAL_TEXT_NOTIFY	(true)		// Proportional del texto
	#define OUTLINE_TEXT_NOTIFY			(1)		// Outline del texto
	#define SHADOW_TEXT_NOTIFY			(0)		// Shadow del texto

	static enum not_info_notify
	{
		nNotifyIcon[MAX_LENGTH_ICON],
		nNotifyTitle[MAX_LENGTH_TITLE],
		nNotifyMsg[MAX_LENGTH_MESSAGE],
		nNotifyLines,
		nNotifyOrder,          // Orden de creacion (para ordenar visualmente)
		bool:nNotifyActive,    // Si esta activa o no
		nNotifyTimerID        // ID del timer para auto-destruccion
	};
	static NOTIFY_INFO[MAX_PLAYERS][MAX_NORMAL_NOTIFY][not_info_notify], PlayerText:NOTIFY_TEXTDRAW[MAX_PLAYERS][(MAX_TEXTDRAW_NOTIFY * MAX_NORMAL_NOTIFY)];
	static g_NotifyOrderCounter[MAX_PLAYERS];  // Contador global de orden
	static g_NotifyCount[MAX_PLAYERS];         // Cantidad de notificaciones activas
#endif

#if defined USE_NOTIFY_SUPPLIES
	#if !defined MAX_NOTIFY_SUPPLIES
		#error "Define the maximum number of notifications to show -> (#define MAX_NOTIFY_SUPPLIES [value])"
	#endif

	#define MAX_TEXTDRAW_SUPPLIES			(1) // Numero de maximo de textdraw
	#define FONT_TEXT_SUPPLIES				(3) // Fuente del texto
	#define PROPORTIONAL_TEXT_SUPPLIES		(true) // Proporcional del texto
	#define OUTLINE_TEXT_SUPPLIES			(1) // Outline del texto
	#define SHADOW_TEXT_SUPPLIES			(0) // Shadow del texto

	static enum not_info_supply
	{
		nSupplyMsg[MAX_LENGTH_MESSAGE],
		nSupplyLines,
		nSupplyOrder,
		bool:nSupplyActive,
		nSupplyTimerID
	};
	static SUPPLY_INFO[MAX_PLAYERS][MAX_NOTIFY_SUPPLIES][not_info_supply], PlayerText:SUPPLY_TEXTDRAW[MAX_PLAYERS][MAX_NOTIFY_SUPPLIES];
	static g_SupplyOrderCounter[MAX_PLAYERS];
	static g_SupplyCount[MAX_PLAYERS];
#endif

#if defined USE_NOTIFY_ADS
	#if !defined MAX_NOTIFY_ADS
		#error "Define the maximum number of notifications to show -> (#define MAX_NOTIFY_ADS [value])"
	#endif

	#define MAX_TEXTDRAW_ADS			(1) // Numero de maximo de textdraw
	#define FONT_TEXT_ADS				(1) // Fuente del texto
	#define PROPORTIONAL_TEXT_ADS		(true) // Proporcional del texto
	#define OUTLINE_TEXT_ADS			(1) // Outline del texto
	#define SHADOW_TEXT_ADS				(0) // Shadow del texto

	static enum not_info_ads
	{
		nAdsMsg[MAX_LENGTH_MESSAGE],
		nAdsLines,
		nAdsOrder,
		bool:nAdsActive,
		nAdsTimerID
	};
	static ADS_INFO[MAX_PLAYERS][MAX_NOTIFY_ADS][not_info_ads], PlayerText:ADS_TEXTDRAW[MAX_PLAYERS][MAX_NOTIFY_ADS];
	static g_AdsOrderCounter[MAX_PLAYERS];
	static g_AdsCount[MAX_PLAYERS];
#endif

#if defined USE_NOTIFY_MESSAGES
	#if !defined MAX_NOTIFY_MESSAGES
		#error "Define the maximum number of messages to show -> (#define MAX_NOTIFY_MESSAGES [value])"
	#endif

	#define MAX_TEXTDRAW_MESSAGES		(1) // Numero de maximo de textdraw
	#define FONT_TEXT_MESSAGES			(1) // Fuente del texto (por defecto)
	#define PROPORTIONAL_TEXT_MESSAGES	(true) // Proporcional del texto
	#define OUTLINE_TEXT_MESSAGES		(1) // Outline del texto (por defecto)
	#define SHADOW_TEXT_MESSAGES		(0) // Shadow del texto (por defecto)

	static enum not_info_message
	{
		nMessageMsg[MAX_LENGTH_MESSAGE],
		nMessageLines,
		nMessageOrder,
		bool:nMessageActive,
		nMessageTimerID,
		nMessageFont,           // Font personalizado por mensaje
		bool:nMessageShadow,    // Shadow personalizado
		bool:nMessageOutline    // Outline personalizado
	};
	static MESSAGE_INFO[MAX_PLAYERS][MAX_NOTIFY_MESSAGES][not_info_message], PlayerText:MESSAGE_TEXTDRAW[MAX_PLAYERS][MAX_NOTIFY_MESSAGES];
	static g_MessageOrderCounter[MAX_PLAYERS];
	static g_MessageCount[MAX_PLAYERS];
#endif

#if defined USE_NOTIFY_LOW_MESSAGES
	#if !defined MAX_NOTIFY_LOW_MESSAGES
		#error "Define the maximum number of low messages to show -> (#define MAX_NOTIFY_LOW_MESSAGES [value])"
	#endif

	#define MAX_TEXTDRAW_LOW_MESSAGES	(1) // Numero de maximo de textdraw
	#define FONT_TEXT_LOW_MESSAGES		(1) // Fuente del texto (por defecto)
	#define PROPORTIONAL_TEXT_LOW_MESSAGES	(true) // Proporcional del texto
	#define OUTLINE_TEXT_LOW_MESSAGES	(0) // Outline del texto (por defecto)
	#define SHADOW_TEXT_LOW_MESSAGES	(0) // Shadow del texto (por defecto)

	static enum not_info_low_message
	{
		nLowMessageMsg[MAX_LENGTH_MESSAGE],
		nLowMessageLines,
		nLowMessageOrder,
		bool:nLowMessageActive,
		nLowMessageTimerID,
		nLowMessageFont,           // Font personalizado por mensaje
		bool:nLowMessageShadow,    // Shadow personalizado
		bool:nLowMessageOutline    // Outline personalizado
	};
	static LOW_MESSAGE_INFO[MAX_PLAYERS][MAX_NOTIFY_LOW_MESSAGES][not_info_low_message], PlayerText:LOW_MESSAGE_TEXTDRAW[MAX_PLAYERS][MAX_NOTIFY_LOW_MESSAGES];
	static g_LowMessageOrderCounter[MAX_PLAYERS];
	static g_LowMessageCount[MAX_PLAYERS];
#endif

#if !defined NOTIFY_AUTO_REPLACE
	#define NOTIFY_AUTO_REPLACE		(true)
#endif

#if !defined SUPPLIES_AUTO_REPLACE
	#define SUPPLIES_AUTO_REPLACE	(true)
#endif

#if !defined ADS_AUTO_REPLACE
	#define ADS_AUTO_REPLACE		(true)
#endif

#if !defined MESSAGES_AUTO_REPLACE
	#define MESSAGES_AUTO_REPLACE	(true)
#endif

#if !defined LOW_MESSAGES_AUTO_REPLACE
	#define LOW_MESSAGES_AUTO_REPLACE	(true)
#endif

#define 	nNotify::%0(%1,%2)				NOTIFY_INFO[%1][%2][%0]
#define 	nSupply::%0(%1,%2)				SUPPLY_INFO[%1][%2][%0]
#define 	nAds::%0(%1,%2)					ADS_INFO[%1][%2][%0]
#define 	nMessage::%0(%1,%2)				MESSAGE_INFO[%1][%2][%0]
#define 	nLowMessage::%0(%1,%2)			LOW_MESSAGE_INFO[%1][%2][%0]

#define 	ntdNotify::(%1,%0)				NOTIFY_TEXTDRAW[%1][%0]
#define 	ntdSupply::(%1,%0)				SUPPLY_TEXTDRAW[%1][%0]
#define 	ntdAds::(%1,%0)					ADS_TEXTDRAW[%1][%0]
#define 	ntdMessage::(%1,%0)				MESSAGE_TEXTDRAW[%1][%0]
#define 	ntdLowMessage::(%1,%0)			LOW_MESSAGE_TEXTDRAW[%1][%0]

#if !defined _NOT_TD_CHAR_WIDTH_DEFINED
#define _NOT_TD_CHAR_WIDTH_DEFINED
static const TDCharacterDefaultWidth[4] = {27, 20, 27, 20};

static const TDCharacterWidth[4][176] = {
	{
		 0,  12,  12,  12,  12,  12,  12,  12,
		12,  12,  12,  12,  12,  12,  12,  12,
		12,  12,  12,  12,  12,  12,  12,  12,
		12,  12,  12,  12,  12,  12,  12,  12,
		12,  13,  13,  28,  28,  28,  28,   8,
		17,  17,  30,  28,  28,  12,   9,  21,
		28,  14,  28,  28,  28,  28,  28,  28,
		28,  28,  13,  13,  30,  30,  30,  30,
		10,  25,  23,  21,  24,  22,  20,  24,
		24,  17,  20,  22,  20,  30,  27,  27,
		26,  26,  24,  23,  24,  31,  23,  31,
		24,  23,  21,  28,  33,  33,  14,  28,
		10,  11,  12,   9,  11,  10,  10,  12,
		12,   7,   7,  13,   5,  18,  12,  10,
		12,  11,  10,  12,   8,  13,  13,  18,
		17,  13,  12,  30,  30,  37,  35,  37,
		25,  25,  25,  25,  33,  21,  24,  24,
		24,  24,  17,  17,  17,  17,  27,  27,
		27,  27,  31,  31,  31,  31,  11,  11,
		11,  11,  11,  20,   9,  10,  10,  10,
		10,   7,   7,   7,   7,  10,  10,  10,
		10,  13,  13,  13,  13,  27,  12,  30
	}, {
		 0,  15,  15,  15,  15,  15,  15,  15,
		15,  15,  15,  15,  15,  15,  15,  15,
		15,  15,  15,  15,  15,  15,  15,  15,
		15,  15,  15,  15,  15,  15,  15,  15,
		15,   9,  17,  27,  20,  34,  23,  12,
		12,  12,  21,  20,  12,  14,  12,  15,
		23,  15,  21,  21,  21,  21,  21,  21,
		20,  21,  12,  12,  24,  24,  24,  19,
		10,  22,  19,  19,  22,  16,  19,  24,
		22,  11,  16,  21,  15,  28,  24,  27,
		20,  25,  19,  19,  18,  23,  23,  31,
		23,  19,  21,  21,  13,  35,  11,  21,
		10,  19,  20,  14,  20,  19,  13,  20,
		19,   9,   9,  19,   9,  29,  19,  21,
		19,  19,  15,  15,  14,  18,  19,  27,
		20,  20,  17,  21,  17,  20,  15,  15,
		22,  22,  22,  22,  29,  19,  16,  16,
		16,  16,  11,  11,  11,  11,  27,  27,
		27,  27,  23,  23,  23,  23,  20,  19,
		19,  19,  19,  30,  14,  19,  19,  19,
		19,   9,   9,   9,   9,  21,  21,  21,
		21,  18,  18,  18,  18,  24,  19,  19
	}, {
		 0,  15,  23,  15,  21,  21,  21,  21,
		21,  21,  20,  21,  12,  12,  24,  24,
		24,  19,  10,  22,  19,  19,  22,  16,
		19,  24,  22,  11,  16,  21,  15,  28,
		12,  13,  13,  28,  37,  28,  30,   8,
		17,  17,  30,  28,  28,  12,   9,  21,
		27,  16,  27,  27,  27,  27,  27,  27,
		27,  27,  18,  13,  30,  30,  30,  30,
		10,  29,  26,  25,  28,  26,  25,  27,
		28,  12,  24,  25,  24,  30,  27,  29,
		26,  26,  25,  26,  25,  26,  28,  32,
		27,  26,  26,  28,  33,  33,  10,  28,
		10,  29,  26,  25,  28,  26,  25,  27,
		28,  12,  24,  25,  24,  30,  27,  29,
		26,  26,  25,  26,  25,  26,  28,  32,
		27,  26,  26,  30,  30,  37,  35,  37,
		29,  29,  29,  29,  33,  25,  26,  26,
		26,  26,  14,  14,  14,  14,  29,  29,
		29,  29,  26,  26,  26,  26,  21,  29,
		29,  29,  29,  33,  25,  26,  26,  26,
		26,  14,  14,  14,  14,  29,  29,  29,
		29,  26,  26,  26,  26,  25,  25,  30
	}, {
		 0,   9,   9,  18,  18,  18,  18,  18,
		18,  18,  18,  19,  19,  19,   0,   9,
		 9,   9,   9,  18,  18,  18,  18,  18,
		18,  18,  18,  19,  19,  19,   0,   9,
		15,  10,  17,  27,  20,  34,  23,  10,
		15,  15,  21,  20,  12,  14,   9,  15,
		20,  18,  19,  19,  21,  19,  19,  19,
		19,  19,  16,  12,  24,  24,  24,  21,
		10,  19,  19,  19,  20,  19,  16,  19,
		19,   9,  19,  20,  14,  29,  19,  19,
		19,  19,  19,  19,  21,  19,  20,  32,
		21,  19,  19,  21,  13,  35,  10,  21,
		10,  19,  19,  19,  20,  19,  16,  19,
		19,   9,  19,  20,  14,  29,  19,  19,
		19,  19,  19,  19,  21,  19,  20,  32,
		21,  19,  19,  21,  17,  20,  15,  15,
		19,  19,  19,  19,  29,  19,  19,  19,
		19,  19,   9,   9,   9,   9,  19,  19,
		19,  19,  19,  19,  19,  19,  19,  19,
		19,  19,  19,  29,  19,  19,  19,  19,
		19,   9,   9,   9,   9,  19,  19,  19,
		19,  19,  19,  19,  19,  21,  21,  19
	}
};

/**
 *    Y88b         888    888                   888                                           888      d8b          888                                      888
 *     Y88b        888    888                   888                                           888      Y8P          888                                      888
 *      Y88b       888    888                   888                                           888                   888                                      888
 *       Y88b      8888888888  .d88b.   .d88b.  888  888 .d8888b        8888b.  88888b.   .d88888      888 88888b.  888888 .d88b.  888d888 88888b.   8888b.  888
 *       d88P      888    888 d88""88b d88""88b 888 .88P 88K               "88b 888 "88b d88" 888      888 888 "88b 888   d8P  Y8b 888P"   888 "88b     "88b 888
 *      d88P       888    888 888  888 888  888 888888K  "Y8888b.      .d888888 888  888 888  888      888 888  888 888   88888888 888     888  888 .d888888 888
 *     d88P        888    888 Y88..88P Y88..88P 888 "88b      X88      888  888 888  888 Y88b 888      888 888  888 Y88b. Y8b.     888     888  888 888  888 888
 *    d88P         888    888  "Y88P"   "Y88P"  888  888  88888P'      "Y888888 888  888  "Y88888      888 888  888  "Y888 "Y8888  888     888  888 "Y888888 888
 *
 *
 *
 */

stock GetTextDrawCharacterWidth(character, font, bool:proportional = true) {
	if (!(0 <= font <= 3) || !(0 <= character < 176)) {
		return 0;
	}

	new width;

	if (!proportional || character >= sizeof(TDCharacterWidth[])) {
		width = TDCharacterDefaultWidth[font];
	} else {
		width = TDCharacterWidth[font][character];
	}

	return width;
}
#endif // _NOT_TD_CHAR_WIDTH_DEFINED

static stock GetMessageLineCount(const message[], font = FONT_TEXT_NOTIFY, bool:proportional = PROPORTIONAL_TEXT_NOTIFY, maxWidth = MAX_WIDTH_MESSAGE)
{
	new size = 0, lines = 1, i = 0, c;

	while ((c = message[i]) != '\0')
	{
		// Si es un codigo de color o un salto de linea
		if (c == '~' && message[i + 1] != '\0' && message[i + 2] == '~')
		{
			// Si es un salto de linea manual (~n~), se incrementa el contador de lineas
			if (message[i + 1] == 'n')
			{
				lines++;
				size = 0;
			}
			// En cualquier caso, se saltan los 3 caracteres del codigo
			i += 3;
			continue;
		}

		// Obtenemos el ancho del caracter (incluyendo espacios)
		new charWidth = GetTextDrawCharacterWidth(c, font, proportional);

		// Si el acumulado mas el ancho actual excede el maximo, se fuerza el salto de linea
		if (size + charWidth > maxWidth)
		{
			lines++;
			size = charWidth; // Iniciamos la nueva linea con el ancho del caracter actual
		}
		else
		{
			size += charWidth;
		}

		i++;
	}

	return lines;
}

static stock TextDrawFixedLatinoText(const text[])
{
	new string[1024];
	new len = strlen(text);
	if(len >= sizeof(string)) len = sizeof(string) - 1;

	for(new i = 0; i < len; i++)
	{
		switch(text[i] & 0xFF)
		{
			case 0xE0: string[i] = 151;
			case 0xE1: string[i] = 152;
			case 0xE2: string[i] = 153;
			case 0xE4: string[i] = 154;
			case 0xC0: string[i] = 128;
			case 0xC1: string[i] = 129;
			case 0xC2: string[i] = 130;
			case 0xC4: string[i] = 131;
			case 0xE8: string[i] = 157;
			case 0xE9: string[i] = 158;
			case 0xEA: string[i] = 159;
			case 0xEB: string[i] = 160;
			case 0xC8: string[i] = 134;
			case 0xC9: string[i] = 135;
			case 0xCA: string[i] = 136;
			case 0xCB: string[i] = 137;
			case 0xEC: string[i] = 161;
			case 0xED: string[i] = 162;
			case 0xEE: string[i] = 163;
			case 0xEF: string[i] = 164;
			case 0xCC: string[i] = 138;
			case 0xCD: string[i] = 139;
			case 0xCE: string[i] = 140;
			case 0xCF: string[i] = 141;
			case 0xF2: string[i] = 165;
			case 0xF3: string[i] = 166;
			case 0xF4: string[i] = 167;
			case 0xF6: string[i] = 168;
			case 0xD2: string[i] = 142;
			case 0xD3: string[i] = 143;
			case 0xD4: string[i] = 144;
			case 0xD6: string[i] = 145;
			case 0xF9: string[i] = 169;
			case 0xFA: string[i] = 170;
			case 0xFB: string[i] = 171;
			case 0xFC: string[i] = 172;
			case 0xD9: string[i] = 146;
			case 0xDA: string[i] = 147;
			case 0xDB: string[i] = 148;
			case 0xDC: string[i] = 149;
			case 0xF1: string[i] = 174;
			case 0xD1: string[i] = 173;
			case 0xBF: string[i] = 175;
			case 0xA1: string[i] = 64;
			default: string[i] = text[i];
		}
	}
	string[len] = EOS;
	return string;
}

public OnPlayerConnect(playerid)
{
	g_NotifyCount[playerid] = 0;
	g_NotifyOrderCounter[playerid] = 0;
	g_SupplyCount[playerid] = 0;
	g_SupplyOrderCounter[playerid] = 0;
	g_AdsCount[playerid] = 0;
	g_AdsOrderCounter[playerid] = 0;
	g_MessageCount[playerid] = 0;
	g_MessageOrderCounter[playerid] = 0;
	g_LowMessageCount[playerid] = 0;
	g_LowMessageOrderCounter[playerid] = 0;

	for(new i = 0; i < MAX_NORMAL_NOTIFY; i++)
	{
		nNotify::nNotifyActive(playerid, i) = false;
		nNotify::nNotifyOrder(playerid, i) = 0;
		nNotify::nNotifyLines(playerid, i) = 0;
		nNotify::nNotifyTimerID(playerid, i) = -1;
		nNotify::nNotifyTitle(playerid, i)[0] = EOS;
		nNotify::nNotifyMsg(playerid, i)[0] = EOS;
		nNotify::nNotifyIcon(playerid, i)[0] = EOS;

		new base_idx = i * MAX_TEXTDRAW_NOTIFY;
		for(new j = 0; j < MAX_TEXTDRAW_NOTIFY; j++)
		{
			ntdNotify::(playerid, base_idx + j) = PlayerText:INVALID_TEXT_DRAW;
		}
	}

	for(new i = 0; i < MAX_NOTIFY_SUPPLIES; i++)
	{
		nSupply::nSupplyActive(playerid, i) = false;
		nSupply::nSupplyOrder(playerid, i) = 0;
		nSupply::nSupplyLines(playerid, i) = 0;
		nSupply::nSupplyTimerID(playerid, i) = -1;
		nSupply::nSupplyMsg(playerid, i)[0] = EOS;

		ntdSupply::(playerid, i) = PlayerText:INVALID_TEXT_DRAW;
	}

	for(new i = 0; i < MAX_NOTIFY_ADS; i++)
	{
		nAds::nAdsActive(playerid, i) = false;
		nAds::nAdsOrder(playerid, i) = 0;
		nAds::nAdsLines(playerid, i) = 0;
		nAds::nAdsTimerID(playerid, i) = -1;
		nAds::nAdsMsg(playerid, i)[0] = EOS;

		ntdAds::(playerid, i) = PlayerText:INVALID_TEXT_DRAW;
	}

	for(new i = 0; i < MAX_NOTIFY_MESSAGES; i++)
	{
		nMessage::nMessageActive(playerid, i) = false;
		nMessage::nMessageOrder(playerid, i) = 0;
		nMessage::nMessageLines(playerid, i) = 0;
		nMessage::nMessageTimerID(playerid, i) = -1;
		nMessage::nMessageFont(playerid, i) = FONT_TEXT_MESSAGES;
		nMessage::nMessageShadow(playerid, i) = true;
		nMessage::nMessageOutline(playerid, i) = false;
		nMessage::nMessageMsg(playerid, i)[0] = EOS;

		ntdMessage::(playerid, i) = PlayerText:INVALID_TEXT_DRAW;
	}

	for(new i = 0; i < MAX_NOTIFY_LOW_MESSAGES; i++)
	{
		nLowMessage::nLowMessageActive(playerid, i) = false;
		nLowMessage::nLowMessageOrder(playerid, i) = 0;
		nLowMessage::nLowMessageLines(playerid, i) = 0;
		nLowMessage::nLowMessageTimerID(playerid, i) = -1;
		nLowMessage::nLowMessageFont(playerid, i) = FONT_TEXT_LOW_MESSAGES;
		nLowMessage::nLowMessageShadow(playerid, i) = false;
		nLowMessage::nLowMessageOutline(playerid, i) = false;
		nLowMessage::nLowMessageMsg(playerid, i)[0] = EOS;

		ntdLowMessage::(playerid, i) = PlayerText:INVALID_TEXT_DRAW;
	}

	#if defined NT_OnPlayerConnect
		return NT_OnPlayerConnect(playerid);
	#else
		return true;
	#endif
}
#if defined _ALS_OnPlayerConnect
	#undef OnPlayerConnect
#else
	#define _ALS_OnPlayerConnect
#endif

#define OnPlayerConnect NT_OnPlayerConnect
#if defined NT_OnPlayerConnect
	forward NT_OnPlayerConnect(playerid);
#endif

public OnPlayerDisconnect(playerid, reason)
{
	HideAllNotifications(playerid);
	HideAllSupplies(playerid);
	HideAllAds(playerid);
	HideAllMessages(playerid);
	HideAllLowMessages(playerid);

	#if defined NT_OnPlayerDisconnect
		return NT_OnPlayerDisconnect(playerid, reason);
	#else
		return 1;
	#endif
}
#if defined _ALS_OnPlayerDisconnect
	#undef OnPlayerDisconnect
#else
	#define _ALS_OnPlayerDisconnect
#endif
#define OnPlayerDisconnect NT_OnPlayerDisconnect
#if defined NT_OnPlayerDisconnect
	forward NT_OnPlayerDisconnect(playerid, reason);
#endif


/**
 *    Y88b         8888888888                              8888888          888                                      888
 *     Y88b        888                                       888            888                                      888
 *      Y88b       888                                       888            888                                      888
 *       Y88b      8888888 888  888 88888b.   .d8888b        888   88888b.  888888 .d88b.  888d888 88888b.   8888b.  888
 *       d88P      888     888  888 888 "88b d88P"           888   888 "88b 888   d8P  Y8b 888P"   888 "88b     "88b 888
 *      d88P       888     888  888 888  888 888             888   888  888 888   88888888 888     888  888 .d888888 888
 *     d88P        888     Y88b 888 888  888 Y88b.           888   888  888 Y88b. Y8b.     888     888  888 888  888 888
 *    d88P         888      "Y88888 888  888  "Y8888P      8888888 888  888  "Y888 "Y8888  888     888  888 "Y888888 888
 *
 *
 *
 *     .d8888b.  888                             888b    888          888    d8b  .d888
 *    d88P  Y88b 888                             8888b   888          888    Y8P d88P"
 *    Y88b.      888                             88888b  888          888        888
 *     "Y888b.   88888b.   .d88b.  888  888  888 888Y88b 888  .d88b.  888888 888 888888 888  888
 *        "Y88b. 888 "88b d88""88b 888  888  888 888 Y88b888 d88""88b 888    888 888    888  888
 *          "888 888  888 888  888 888  888  888 888  Y88888 888  888 888    888 888    888  888
 *    Y88b  d88P 888  888 Y88..88P Y88b 888 d88P 888   Y8888 Y88..88P Y88b.  888 888    Y88b 888
 *     "Y8888P"  888  888  "Y88P"   "Y8888888P"  888    Y888  "Y88P"   "Y888 888 888     "Y88888
 *                                                                                           888
 *                                                                                      Y8b d88P
 *                                                                                       "Y88P"
 */

// Busca el primer slot libre disponible para una nueva notificacion
// Si no hay slots libres y AUTO_REPLACE esta activo, reemplaza la mas antigua
// Retorna: indice del slot libre (0 a MAX_NORMAL_NOTIFY-1) o -1 si no hay disponible
static stock Internal_GetFreeNotifySlot(playerid)
{
	if(!IsPlayerConnected(playerid)) return -1;
	for(new i = 0; i < MAX_NORMAL_NOTIFY; i++)
	{
		if(!nNotify::nNotifyActive(playerid, i))
		{
			return i;
		}
	}

	#if NOTIFY_AUTO_REPLACE
	new oldestID = -1, oldestOrder = 0x7FFFFFFF;

	for(new i = 0; i < MAX_NORMAL_NOTIFY; i++)
	{
		if(nNotify::nNotifyOrder(playerid, i) < oldestOrder)
		{
			oldestOrder = nNotify::nNotifyOrder(playerid, i);
			oldestID = i;
		}
	}

	if(oldestID != -1)
	{
		if(nNotify::nNotifyTimerID(playerid, oldestID) != -1)
		{
			KillTimer(nNotify::nNotifyTimerID(playerid, oldestID));
			nNotify::nNotifyTimerID(playerid, oldestID) = -1;
		}

		nNotify::nNotifyActive(playerid, oldestID) = false;

		if(g_NotifyCount[playerid] > 0)
			g_NotifyCount[playerid]--;

		return oldestID;
	}
	#endif
	return -1;
}

// Actualiza visualmente todas las notificaciones activas del jugador
// Destruye todos los textdraws existentes y los recrea en orden
// Las notificaciones se ordenan por nNotifyOrder (mas recientes primero)
// Retorna: cantidad de notificaciones actualizadas
static stock Internal_UpdateAllNotifications(playerid)
{
	// Destruir todos los textdraws existentes
	for(new i = 0; i < MAX_NORMAL_NOTIFY; i++)
	{
		new base_idx = i * MAX_TEXTDRAW_NOTIFY;
		for(new j = 0; j < MAX_TEXTDRAW_NOTIFY; j++)
		{
			if(ntdNotify::(playerid, base_idx + j) == PlayerText:INVALID_TEXT_DRAW) continue;
			PlayerTextDrawDestroy(playerid, ntdNotify::(playerid, base_idx + j));
			ntdNotify::(playerid, base_idx + j) = PlayerText:INVALID_TEXT_DRAW;
		}
	}

	// Si no hay notificaciones activas, salir
	if(g_NotifyCount[playerid] == 0) return false;

	// Ordenar notificaciones por orden de creacion (mas recientes primero)
	new sortedNotifs[MAX_NORMAL_NOTIFY];
	new sortedCount = 0;

	for(new i = 0; i < MAX_NORMAL_NOTIFY; i++)
	{
		if(nNotify::nNotifyActive(playerid, i))
		{
			sortedNotifs[sortedCount++] = i;
		}
	}

	// Bubble sort descendente por nNotifyOrder
	for(new i = 0; i < sortedCount - 1; i++)
	{
		for(new j = i + 1; j < sortedCount; j++)
		{
			if(nNotify::nNotifyOrder(playerid, sortedNotifs[i]) < nNotify::nNotifyOrder(playerid, sortedNotifs[j]))
			{
				new temp = sortedNotifs[i];
				sortedNotifs[i] = sortedNotifs[j];
				sortedNotifs[j] = temp;
			}
		}
	}

	// Recrear notificaciones en orden (mas recientes arriba)
	new Float:PosX = 0.0;
	for(new i = 0; i < sortedCount; i++)
	{
		Internal_CreateNotifyTD(playerid, sortedNotifs[i], (sortedNotifs[i] * MAX_TEXTDRAW_NOTIFY), PosX);
		PosX += ((nNotify::nNotifyLines(playerid, sortedNotifs[i]) * 10.0) + 25.0);
	}

	return sortedCount;
}

// Crea los textdraws de una notificacion especifica
// Crea 5 textdraws: fondo, barra de titulo, icono, titulo y mensaje
// Los posiciona dinamicamente segun el numero de lineas del mensaje
// Retorna: true si se creo correctamente
static stock Internal_CreateNotifyTD(playerid, index, index_textdraw, Float:static_x)
{
	// [0] Fondo - altura dinamica: base 18.0 + lineas extra * 10.0
	// Con 1 linea = 18.0, con 2 lineas = 28.0, con 3 lineas = 38.0, etc.
	new lines = nNotify::nNotifyLines(playerid, index), Float:PosX = (lines * 10.0) + static_x;
	new Float:bgHeight = 18.0 + (float(lines) * 10.0);

	NOTIFY_TEXTDRAW[playerid][index_textdraw] = CreatePlayerTextDraw(playerid, 10.000, (300.000 - PosX), "LD_SPAC:white");
	PlayerTextDrawTextSize(playerid, NOTIFY_TEXTDRAW[playerid][index_textdraw], 150.000, bgHeight);
	PlayerTextDrawAlignment(playerid, NOTIFY_TEXTDRAW[playerid][index_textdraw], 1);
	PlayerTextDrawColor(playerid, NOTIFY_TEXTDRAW[playerid][index_textdraw], 128);
	PlayerTextDrawSetShadow(playerid, NOTIFY_TEXTDRAW[playerid][index_textdraw], 0);
	PlayerTextDrawSetOutline(playerid, NOTIFY_TEXTDRAW[playerid][index_textdraw], 0);
	PlayerTextDrawBackgroundColor(playerid, NOTIFY_TEXTDRAW[playerid][index_textdraw], 128);
	PlayerTextDrawFont(playerid, NOTIFY_TEXTDRAW[playerid][index_textdraw], 4);
	PlayerTextDrawSetProportional(playerid, NOTIFY_TEXTDRAW[playerid][index_textdraw], 1);
	PlayerTextDrawShow(playerid, NOTIFY_TEXTDRAW[playerid][index_textdraw]);

	// [1] Icono
	index_textdraw++;
	NOTIFY_TEXTDRAW[playerid][index_textdraw] = CreatePlayerTextDraw(playerid, 12.000, (302.000 - PosX), nNotify::nNotifyIcon(playerid, index));
	PlayerTextDrawTextSize(playerid, NOTIFY_TEXTDRAW[playerid][index_textdraw], 14.000, 14.000);
	PlayerTextDrawAlignment(playerid, NOTIFY_TEXTDRAW[playerid][index_textdraw], 1);
	PlayerTextDrawColor(playerid, NOTIFY_TEXTDRAW[playerid][index_textdraw], -1);
	PlayerTextDrawSetShadow(playerid, NOTIFY_TEXTDRAW[playerid][index_textdraw], 0);
	PlayerTextDrawSetOutline(playerid, NOTIFY_TEXTDRAW[playerid][index_textdraw], 0);
	PlayerTextDrawBackgroundColor(playerid, NOTIFY_TEXTDRAW[playerid][index_textdraw], 255);
	PlayerTextDrawFont(playerid, NOTIFY_TEXTDRAW[playerid][index_textdraw], 4);
	PlayerTextDrawSetProportional(playerid, NOTIFY_TEXTDRAW[playerid][index_textdraw], 1);
	PlayerTextDrawShow(playerid, NOTIFY_TEXTDRAW[playerid][index_textdraw]);

	// [2] Titulo (alignment 3 = derecha, anclado a X 156.0)
	index_textdraw++;
	NOTIFY_TEXTDRAW[playerid][index_textdraw] = CreatePlayerTextDraw(playerid, 156.000, (304.000 - PosX), nNotify::nNotifyTitle(playerid, index));
	PlayerTextDrawLetterSize(playerid, NOTIFY_TEXTDRAW[playerid][index_textdraw], 0.200, 1.200);
	PlayerTextDrawAlignment(playerid, NOTIFY_TEXTDRAW[playerid][index_textdraw], 3);
	PlayerTextDrawColor(playerid, NOTIFY_TEXTDRAW[playerid][index_textdraw], 0xFFFFFFEE);
	PlayerTextDrawSetShadow(playerid, NOTIFY_TEXTDRAW[playerid][index_textdraw], 0);
	PlayerTextDrawSetOutline(playerid, NOTIFY_TEXTDRAW[playerid][index_textdraw], 0);
	PlayerTextDrawBackgroundColor(playerid, NOTIFY_TEXTDRAW[playerid][index_textdraw], 150);
	PlayerTextDrawFont(playerid, NOTIFY_TEXTDRAW[playerid][index_textdraw], 1);
	PlayerTextDrawSetProportional(playerid, NOTIFY_TEXTDRAW[playerid][index_textdraw], 1);
	PlayerTextDrawShow(playerid, NOTIFY_TEXTDRAW[playerid][index_textdraw]);

	// [3] Mensaje - TextSize Y cubre las lineas dinamicamente
	index_textdraw++;
	NOTIFY_TEXTDRAW[playerid][index_textdraw] = CreatePlayerTextDraw(playerid, 16.000, (316.000 - PosX), nNotify::nNotifyMsg(playerid, index));
	PlayerTextDrawLetterSize(playerid, NOTIFY_TEXTDRAW[playerid][index_textdraw], 0.180, 1.000);
	PlayerTextDrawTextSize(playerid, NOTIFY_TEXTDRAW[playerid][index_textdraw], 154.000, float(lines) * 10.0);
	PlayerTextDrawAlignment(playerid, NOTIFY_TEXTDRAW[playerid][index_textdraw], 1);
	PlayerTextDrawColor(playerid, NOTIFY_TEXTDRAW[playerid][index_textdraw], 0xFFFFFFDC);
	PlayerTextDrawSetShadow(playerid, NOTIFY_TEXTDRAW[playerid][index_textdraw], 0);
	PlayerTextDrawSetOutline(playerid, NOTIFY_TEXTDRAW[playerid][index_textdraw], 0);
	PlayerTextDrawBackgroundColor(playerid, NOTIFY_TEXTDRAW[playerid][index_textdraw], 150);
	PlayerTextDrawFont(playerid, NOTIFY_TEXTDRAW[playerid][index_textdraw], 1);
	PlayerTextDrawSetProportional(playerid, NOTIFY_TEXTDRAW[playerid][index_textdraw], 1);
	PlayerTextDrawShow(playerid, NOTIFY_TEXTDRAW[playerid][index_textdraw]);
	return true;
}

forward DestroyNotifyByID(playerid, notifyid);
public DestroyNotifyByID(playerid, notifyid)
{
	if(notifyid < 0 || notifyid >= MAX_NORMAL_NOTIFY) return false;
	if(!nNotify::nNotifyActive(playerid, notifyid)) return false;

	// Matar el timer si existe
	if(nNotify::nNotifyTimerID(playerid, notifyid) != -1)
	{
		KillTimer(nNotify::nNotifyTimerID(playerid, notifyid));
		nNotify::nNotifyTimerID(playerid, notifyid) = -1;
	}

	// Marcar como inactiva
	nNotify::nNotifyActive(playerid, notifyid) = false;

	// Limpiar datos
	nNotify::nNotifyTitle(playerid, notifyid)[0] = EOS;
	nNotify::nNotifyMsg(playerid, notifyid)[0] = EOS;
	nNotify::nNotifyIcon(playerid, notifyid)[0] = EOS;
	nNotify::nNotifyLines(playerid, notifyid) = 0;
	nNotify::nNotifyOrder(playerid, notifyid) = 0;

	if(g_NotifyCount[playerid] > 0)
		g_NotifyCount[playerid]--;

	// Actualizar visualizacion
	Internal_UpdateAllNotifications(playerid);

	return true;
}

forward DestroyOldestNotify(playerid);
public DestroyOldestNotify(playerid)
{
	// Buscar la notificacion mas antigua (menor nNotifyOrder)
	new oldestID = -1, oldestOrder = 0x7FFFFFFF;

	for(new i = 0; i < MAX_NORMAL_NOTIFY; i++)
	{
		if(nNotify::nNotifyActive(playerid, i))
		{
			if(nNotify::nNotifyOrder(playerid, i) < oldestOrder)
			{
				oldestOrder = nNotify::nNotifyOrder(playerid, i);
				oldestID = i;
			}
		}
	}

	if(oldestID != -1)
	{
		return DestroyNotifyByID(playerid, oldestID);
	}

	return false;
}

/**
 *    Y88b         8888888          888                     .d888
 *     Y88b          888            888                    d88P"
 *      Y88b         888            888                    888
 *       Y88b        888   88888b.  888888 .d88b.  888d888 888888 8888b.   .d8888b .d88b.
 *       d88P        888   888 "88b 888   d8P  Y8b 888P"   888       "88b d88P"   d8P  Y8b
 *      d88P         888   888  888 888   88888888 888     888   .d888888 888     88888888
 *     d88P          888   888  888 Y88b. Y8b.     888     888   888  888 Y88b.   Y8b.
 *    d88P         8888888 888  888  "Y888 "Y8888  888     888   "Y888888  "Y8888P "Y8888
 *
 *
 *
 */

/*
	ShowNotify
	Muestra una notificacion al jugador

	Parametros:
		playerid - ID del jugador
		Titulo[] - Titulo de la notificacion (soporta codigos de color)
		Mensaje[] - Mensaje de la notificacion
		Segundos - Tiempo en milisegundos antes de auto-destruirse
		Icon[] - Icono a mostrar (ej: "ld_chat:badchat")

	Retorna:
		ID de la notificacion creada (0 a MAX_NORMAL_NOTIFY-1)
		-1 si no hay slots disponibles

	Ejemplo:
		new notifyid = ShowNotify(playerid, "~g~Exito", "Operacion completada", 5000, "ld_chat:badchat");
*/
stock ShowNotify(playerid, const Titulo[], const Mensaje[], Segundos, const Icon[] = "ld_chat:badchat")
{
	// Buscar slot disponible
	new notifyid = Internal_GetFreeNotifySlot(playerid);
	if(notifyid == -1) return -1; // No hay slots disponibles

	// Guardar informacion en el slot
	strmid(nNotify::nNotifyTitle(playerid, notifyid), TextDrawFixedLatinoText(Titulo), 0, MAX_LENGTH_TITLE);
	strmid(nNotify::nNotifyMsg(playerid, notifyid), TextDrawFixedLatinoText(Mensaje), 0, MAX_LENGTH_MESSAGE);
	strmid(nNotify::nNotifyIcon(playerid, notifyid), TextDrawFixedLatinoText(Icon), 0, MAX_LENGTH_ICON);
	nNotify::nNotifyLines(playerid, notifyid) = GetMessageLineCount(TextDrawFixedLatinoText(Mensaje));
	nNotify::nNotifyActive(playerid, notifyid) = true;
	nNotify::nNotifyOrder(playerid, notifyid) = g_NotifyOrderCounter[playerid]++;

	#if defined NOT_NOTIFY_DEBUG
		new string[8];
		format(string, sizeof(string), "[#%d]", notifyid);
		strins(nNotify::nNotifyMsg(playerid, notifyid), string, 0, MAX_LENGTH_MESSAGE);
	#endif

	g_NotifyCount[playerid]++;

	// Recrear todas las notificaciones
	Internal_UpdateAllNotifications(playerid);

	// Programar auto-destruccion
	nNotify::nNotifyTimerID(playerid, notifyid) = SetTimerEx("DestroyNotifyByID", Segundos, false, "dd", playerid, notifyid);

	return notifyid;
}

/*
	IsNotifyActive
	Verifica si una notificacion esta activa
*/
stock IsNotifyActive(playerid, notifyid)
{
	if(notifyid < 0 || notifyid >= MAX_NORMAL_NOTIFY) return false;
	return nNotify::nNotifyActive(playerid, notifyid);
}

/*
	GetActiveNotifyCount
	Obtiene la cantidad de notificaciones activas
*/
stock GetActiveNotifyCount(playerid)
{
	return g_NotifyCount[playerid];
}

/*
	HideAllNotifications
	Oculta todas las notificaciones activas
*/
stock HideAllNotifications(playerid)
{
	new count = 0;
	for(new i = 0; i < MAX_NORMAL_NOTIFY; i++)
	{
		if(nNotify::nNotifyActive(playerid, i))
		{
			DestroyNotifyByID(playerid, i);
			count++;
		}
	}
	return count;
}

/*
	UpdateNotifyText
	Actualiza el texto de una notificacion activa sin reiniciar el timer

	Parametros:
		playerid - ID del jugador
		notifyid - ID de la notificacion a actualizar
		newTitle[] - Nuevo titulo a mostrar (opcional, usar "" para mantener el actual)
		newMessage[] - Nuevo mensaje a mostrar

	Retorna:
		true si se actualizo correctamente
		false si el notifyid es invalido o no esta activo

	Ejemplo:
		UpdateNotifyText(playerid, notifyid, "~y~Actualizado", "Nuevo contenido aqui");
*/
stock UpdateNotifyText(playerid, notifyid, const newTitle[] = "", const newMessage[])
{
	if(notifyid < 0 || notifyid >= MAX_NORMAL_NOTIFY) return false;
	if(!nNotify::nNotifyActive(playerid, notifyid)) return false;

	// Actualizar titulo solo si se proporciona uno nuevo
	if(strlen(newTitle) > 0)
	{
		strmid(nNotify::nNotifyTitle(playerid, notifyid), TextDrawFixedLatinoText(newTitle), 0, MAX_LENGTH_TITLE);
	}

	// Actualizar mensaje
	strmid(nNotify::nNotifyMsg(playerid, notifyid), TextDrawFixedLatinoText(newMessage), 0, MAX_LENGTH_MESSAGE);
	nNotify::nNotifyLines(playerid, notifyid) = GetMessageLineCount(TextDrawFixedLatinoText(newMessage));

	Internal_UpdateAllNotifications(playerid);

	return true;
}

/*
	DestroyNotify
	Destruye manualmente una notificacion especifica

	Parametros:
		playerid - ID del jugador
		notifyid - ID de la notificacion a destruir

	Retorna:
		true si se destruyo correctamente
		false si el notifyid es invalido o ya estaba destruida

	Ejemplo:
		if(DestroyNotify(playerid, myNotifyID))
		{
			// Notificacion destruida exitosamente
		}

	Nota:
		Esta funcion es equivalente a DestroyNotifyByID,
		pero con nombre mas amigable para uso publico.
*/
stock DestroyNotify(playerid, &notifyid)
{
	DestroyNotifyByID(playerid, notifyid);
	notifyid = INVALID_NOTIFY_ID;
	return true;
}

/**
 *    Y88b         8888888888                              8888888          888                                      888
 *     Y88b        888                                       888            888                                      888
 *      Y88b       888                                       888            888                                      888
 *       Y88b      8888888 888  888 88888b.   .d8888b        888   88888b.  888888 .d88b.  888d888 88888b.   8888b.  888
 *       d88P      888     888  888 888 "88b d88P"           888   888 "88b 888   d8P  Y8b 888P"   888 "88b     "88b 888
 *      d88P       888     888  888 888  888 888             888   888  888 888   88888888 888     888  888 .d888888 888
 *     d88P        888     Y88b 888 888  888 Y88b.           888   888  888 Y88b. Y8b.     888     888  888 888  888 888
 *    d88P         888      "Y88888 888  888  "Y8888P      8888888 888  888  "Y888 "Y8888  888     888  888 "Y888888 888
 *
 *
 *
 *     .d8888b.  888                             888b    888          888    d8b  .d888           .d8888b.                             888 d8b
 *    d88P  Y88b 888                             8888b   888          888    Y8P d88P"           d88P  Y88b                            888 Y8P
 *    Y88b.      888                             88888b  888          888        888             Y88b.                                 888
 *     "Y888b.   88888b.   .d88b.  888  888  888 888Y88b 888  .d88b.  888888 888 888888 888  888  "Y888b.   888  888 88888b.  88888b.  888 888  .d88b.  .d8888b
 *        "Y88b. 888 "88b d88""88b 888  888  888 888 Y88b888 d88""88b 888    888 888    888  888     "Y88b. 888  888 888 "88b 888 "88b 888 888 d8P  Y8b 88K
 *          "888 888  888 888  888 888  888  888 888  Y88888 888  888 888    888 888    888  888       "888 888  888 888  888 888  888 888 888 88888888 "Y8888b.
 *    Y88b  d88P 888  888 Y88..88P Y88b 888 d88P 888   Y8888 Y88..88P Y88b.  888 888    Y88b 888 Y88b  d88P Y88b 888 888 d88P 888 d88P 888 888 Y8b.          X88
 *     "Y8888P"  888  888  "Y88P"   "Y8888888P"  888    Y888  "Y88P"   "Y888 888 888     "Y88888  "Y8888P"   "Y88888 88888P"  88888P"  888 888  "Y8888   88888P'
 *                                                                                           888                     888      888
 *                                                                                      Y8b d88P                     888      888
 *                                                                                       "Y88P"                      888      888
 */

// Busca el primer slot libre disponible para un nuevo supply
// Si no hay slots libres y AUTO_REPLACE esta activo, reemplaza el mas antiguo
// Retorna: indice del slot libre (0 a MAX_NOTIFY_SUPPLIES-1) o -1 si no hay disponible
static stock Internal_GetFreeSupplySlot(playerid)
{
	if(!IsPlayerConnected(playerid)) return -1;
	for(new i = 0; i < MAX_NOTIFY_SUPPLIES; i++)
	{
		if(!nSupply::nSupplyActive(playerid, i))
		{
			return i;
		}
	}

	#if SUPPLIES_AUTO_REPLACE
	new oldestID = -1, oldestOrder = 0x7FFFFFFF;

	for(new i = 0; i < MAX_NOTIFY_SUPPLIES; i++)
	{
		if(nSupply::nSupplyOrder(playerid, i) < oldestOrder)
		{
			oldestOrder = nSupply::nSupplyOrder(playerid, i);
			oldestID = i;
		}
	}

	if(oldestID != -1)
	{
		if(nSupply::nSupplyTimerID(playerid, oldestID) != -1)
		{
			KillTimer(nSupply::nSupplyTimerID(playerid, oldestID));
			nSupply::nSupplyTimerID(playerid, oldestID) = -1;
		}

		nSupply::nSupplyActive(playerid, oldestID) = false;

		if(g_SupplyCount[playerid] > 0)
			g_SupplyCount[playerid]--;

		return oldestID;
	}
	#endif
	return -1;
}

// Actualiza visualmente todos los supplies activos del jugador
// Destruye todos los textdraws existentes y los recrea en orden
// Los supplies se ordenan por nSupplyOrder (mas recientes primero)
// Retorna: cantidad de supplies actualizados
static stock Internal_UpdateAllSupplies(playerid)
{
	// Destruir todos los textdraws
	for(new i = 0; i < MAX_NOTIFY_SUPPLIES; i++)
	{
		if(ntdSupply::(playerid, i) == PlayerText:INVALID_TEXT_DRAW) continue;
		PlayerTextDrawDestroy(playerid, ntdSupply::(playerid, i));
		ntdSupply::(playerid, i) = PlayerText:INVALID_TEXT_DRAW;
	}

	if(g_SupplyCount[playerid] == 0) return false;

	// Ordenar supplies
	new sortedSupplies[MAX_NOTIFY_SUPPLIES];
	new sortedCount = 0;

	for(new i = 0; i < MAX_NOTIFY_SUPPLIES; i++)
	{
		if(nSupply::nSupplyActive(playerid, i))
		{
			sortedSupplies[sortedCount++] = i;
		}
	}

	// Bubble sort descendente
	for(new i = 0; i < sortedCount - 1; i++)
	{
		for(new j = i + 1; j < sortedCount; j++)
		{
			if(nSupply::nSupplyOrder(playerid, sortedSupplies[i]) < nSupply::nSupplyOrder(playerid, sortedSupplies[j]))
			{
				new temp = sortedSupplies[i];
				sortedSupplies[i] = sortedSupplies[j];
				sortedSupplies[j] = temp;
			}
		}
	}

	// Recrear supplies
	new Float:PosX = 0.0;
	for(new i = 0; i < sortedCount; i++)
	{
		Internal_CreateNotifySupplyTD(playerid, sortedSupplies[i], sortedSupplies[i], PosX);
		PosX += (nSupply::nSupplyLines(playerid, sortedSupplies[i]) * 12.0);
	}

	return sortedCount;
}

// Crea el textdraw de un supply especifico
// Posiciona el mensaje en la esquina superior derecha
// Ajusta la posicion vertical segun la cantidad de lineas
// Retorna: true si se creo correctamente
static stock Internal_CreateNotifySupplyTD(playerid, index, index_textdraw, Float:pos_x)
{
	SUPPLY_TEXTDRAW[playerid][index_textdraw] = CreatePlayerTextDraw(playerid, 630.000, (100.000 + pos_x), nSupply::nSupplyMsg(playerid, index));
	#if FONT_TEXT_SUPPLIES == 3
	PlayerTextDrawLetterSize(playerid, SUPPLY_TEXTDRAW[playerid][index_textdraw], 0.250, 1.300);
	#else
	PlayerTextDrawLetterSize(playerid, SUPPLY_TEXTDRAW[playerid][index_textdraw], 0.180, 1.050);
	#endif
	PlayerTextDrawAlignment(playerid, SUPPLY_TEXTDRAW[playerid][index_textdraw], 3);
	PlayerTextDrawColor(playerid, SUPPLY_TEXTDRAW[playerid][index_textdraw], -1);
	PlayerTextDrawSetShadow(playerid, SUPPLY_TEXTDRAW[playerid][index_textdraw], SHADOW_TEXT_SUPPLIES);
	PlayerTextDrawSetOutline(playerid, SUPPLY_TEXTDRAW[playerid][index_textdraw], OUTLINE_TEXT_SUPPLIES);
	PlayerTextDrawBackgroundColor(playerid, SUPPLY_TEXTDRAW[playerid][index_textdraw], 128);
	PlayerTextDrawFont(playerid, SUPPLY_TEXTDRAW[playerid][index_textdraw], FONT_TEXT_SUPPLIES);
	PlayerTextDrawSetProportional(playerid, SUPPLY_TEXTDRAW[playerid][index_textdraw], PROPORTIONAL_TEXT_SUPPLIES);
	PlayerTextDrawShow(playerid, SUPPLY_TEXTDRAW[playerid][index_textdraw]);
	return true;
}

forward DestroyNotifySuppliesByID(playerid, supplyid);
public DestroyNotifySuppliesByID(playerid, supplyid)
{
	if(supplyid < 0 || supplyid >= MAX_NOTIFY_SUPPLIES) return false;
	if(!nSupply::nSupplyActive(playerid, supplyid)) return false;

	if(nSupply::nSupplyTimerID(playerid, supplyid) != -1)
	{
		KillTimer(nSupply::nSupplyTimerID(playerid, supplyid));
		nSupply::nSupplyTimerID(playerid, supplyid) = -1;
	}

	nSupply::nSupplyActive(playerid, supplyid) = false;
	nSupply::nSupplyMsg(playerid, supplyid)[0] = EOS;
	nSupply::nSupplyLines(playerid, supplyid) = 0;
	nSupply::nSupplyOrder(playerid, supplyid) = 0;

	if(g_SupplyCount[playerid] > 0)
		g_SupplyCount[playerid]--;

	Internal_UpdateAllSupplies(playerid);

	return true;
}

forward DestroyNotifyOldestSupplies(playerid);
public DestroyNotifyOldestSupplies(playerid)
{
	new oldestID = -1, oldestOrder = 0x7FFFFFFF;

	for(new i = 0; i < MAX_NOTIFY_SUPPLIES; i++)
	{
		if(nSupply::nSupplyActive(playerid, i))
		{
			if(nSupply::nSupplyOrder(playerid, i) < oldestOrder)
			{
				oldestOrder = nSupply::nSupplyOrder(playerid, i);
				oldestID = i;
			}
		}
	}

	if(oldestID != -1)
	{
		return DestroyNotifySuppliesByID(playerid, oldestID);
	}

	return false;
}

/**
 *    Y88b         8888888          888                     .d888
 *     Y88b          888            888                    d88P"
 *      Y88b         888            888                    888
 *       Y88b        888   88888b.  888888 .d88b.  888d888 888888 8888b.   .d8888b .d88b.
 *       d88P        888   888 "88b 888   d8P  Y8b 888P"   888       "88b d88P"   d8P  Y8b
 *      d88P         888   888  888 888   88888888 888     888   .d888888 888     88888888
 *     d88P          888   888  888 Y88b. Y8b.     888     888   888  888 Y88b.   Y8b.
 *    d88P         8888888 888  888  "Y888 "Y8888  888     888   "Y888888  "Y8888P "Y8888
 *
 *
 *
 */

/*
	ShowNotifySupplies
	Muestra un mensaje de suministro/supply al jugador (esquina superior derecha)

	Parametros:
		playerid - ID del jugador
		Mensaje[] - Mensaje a mostrar (soporta codigos de color)
		Segundos - Tiempo en milisegundos antes de auto-destruirse
		sound - ID del sonido a reproducir (0 = sin sonido)

	Retorna:
		ID del supply creado (0 a MAX_NOTIFY_SUPPLIES-1)
		-1 si no hay slots disponibles

	Ejemplo:
		new supplyid = ShowNotifySupplies(playerid, "~g~+10 Materiales", 5000, 1083);
*/
stock ShowNotifySupplies(playerid, const Mensaje[], Segundos, sound = 0)
{
	new supplyid = Internal_GetFreeSupplySlot(playerid);
	if(supplyid == -1) return -1;

	strmid(nSupply::nSupplyMsg(playerid, supplyid), TextDrawFixedLatinoText(Mensaje), 0, MAX_LENGTH_MESSAGE);
	nSupply::nSupplyLines(playerid, supplyid) = GetMessageLineCount(TextDrawFixedLatinoText(Mensaje));
	nSupply::nSupplyActive(playerid, supplyid) = true;
	nSupply::nSupplyOrder(playerid, supplyid) = g_SupplyOrderCounter[playerid]++;

	#if defined NOT_NOTIFY_DEBUG
		new string[8];
		format(string, sizeof(string), "[#%d]", supplyid);
		strins(nSupply::nSupplyMsg(playerid, supplyid), string, 0, MAX_LENGTH_MESSAGE);
	#endif

	g_SupplyCount[playerid]++;

	Internal_UpdateAllSupplies(playerid);

	nSupply::nSupplyTimerID(playerid, supplyid) = SetTimerEx("DestroyNotifySuppliesByID", Segundos, false, "dd", playerid, supplyid);

	if(sound != 0) PlayerPlaySound(playerid, sound, 0.0, 0.0, 0.0);

	return supplyid;
}

/*
	IsSupplyActive
	Verifica si un supply esta activo

	Parametros:
		playerid - ID del jugador
		supplyid - ID del supply a verificar

	Retorna:
		true si esta activo
		false si no esta activo o el ID es invalido
*/
stock IsSupplyActive(playerid, supplyid)
{
	if(supplyid < 0 || supplyid >= MAX_NOTIFY_SUPPLIES) return false;
	return nSupply::nSupplyActive(playerid, supplyid);
}

/*
	GetActiveSupplyCount
	Obtiene la cantidad de supplies activos

	Parametros:
		playerid - ID del jugador

	Retorna:
		Cantidad de supplies activos
*/
stock GetActiveSupplyCount(playerid)
{
	return g_SupplyCount[playerid];
}

/*
	HideAllSupplies
	Oculta todos los supplies activos

	Parametros:
		playerid - ID del jugador

	Retorna:
		Cantidad de supplies que fueron ocultados
*/
stock HideAllSupplies(playerid)
{
	new count = 0;
	for(new i = 0; i < MAX_NOTIFY_SUPPLIES; i++)
	{
		if(nSupply::nSupplyActive(playerid, i))
		{
			DestroyNotifySuppliesByID(playerid, i);
			count++;
		}
	}
	return count;
}

/*
	UpdateSupplyText
	Actualiza el texto de un supply activo sin reiniciar el timer

	Parametros:
		playerid - ID del jugador
		supplyid - ID del supply a actualizar
		newText[] - Nuevo texto a mostrar

	Retorna:
		true si se actualizo correctamente
		false si el supplyid es invalido o no esta activo
*/
stock UpdateSupplyText(playerid, supplyid, const newText[])
{
	if(supplyid < 0 || supplyid >= MAX_NOTIFY_SUPPLIES) return false;
	if(!nSupply::nSupplyActive(playerid, supplyid)) return false;

	strmid(nSupply::nSupplyMsg(playerid, supplyid), TextDrawFixedLatinoText(newText), 0, MAX_LENGTH_MESSAGE);
	nSupply::nSupplyLines(playerid, supplyid) = GetMessageLineCount(TextDrawFixedLatinoText(newText));

	Internal_UpdateAllSupplies(playerid);

	return true;
}

/*
	DestroySupply
	Destruye manualmente un supply especifico

	Parametros:
		playerid - ID del jugador
		supplyid - ID del supply a destruir

	Retorna:
		true si se destruyo correctamente
		false si el supplyid es invalido o ya estaba destruido

	Ejemplo:
		if(DestroySupply(playerid, mySupplyID))
		{
			// Supply destruido exitosamente
		}
*/
stock DestroySupply(playerid, &supplyid)
{
	DestroyNotifySuppliesByID(playerid, supplyid);
	supplyid = INVALID_NOTIFY_ID;
	return true;
}

/**
 *    Y88b         8888888888                              8888888          888                                      888
 *     Y88b        888                                       888            888                                      888
 *      Y88b       888                                       888            888                                      888
 *       Y88b      8888888 888  888 88888b.   .d8888b        888   88888b.  888888 .d88b.  888d888 88888b.   8888b.  888
 *       d88P      888     888  888 888 "88b d88P"           888   888 "88b 888   d8P  Y8b 888P"   888 "88b     "88b 888
 *      d88P       888     888  888 888  888 888             888   888  888 888   88888888 888     888  888 .d888888 888
 *     d88P        888     Y88b 888 888  888 Y88b.           888   888  888 Y88b. Y8b.     888     888  888 888  888 888
 *    d88P         888      "Y88888 888  888  "Y8888P      8888888 888  888  "Y888 "Y8888  888     888  888 "Y888888 888
 *
 *
 *
 *     .d8888b.  888                             888b    888          888    d8b  .d888                 d8888 8888888b.   .d8888b.
 *    d88P  Y88b 888                             8888b   888          888    Y8P d88P"                 d88888 888  "Y88b d88P  Y88b
 *    Y88b.      888                             88888b  888          888        888                  d88P888 888    888 Y88b.
 *     "Y888b.   88888b.   .d88b.  888  888  888 888Y88b 888  .d88b.  888888 888 888888 888  888     d88P 888 888    888  "Y888b.
 *        "Y88b. 888 "88b d88""88b 888  888  888 888 Y88b888 d88""88b 888    888 888    888  888    d88P  888 888    888     "Y88b.
 *          "888 888  888 888  888 888  888  888 888  Y88888 888  888 888    888 888    888  888   d88P   888 888    888       "888
 *    Y88b  d88P 888  888 Y88..88P Y88b 888 d88P 888   Y8888 Y88..88P Y88b.  888 888    Y88b 888  d8888888888 888  .d88P Y88b  d88P
 *     "Y8888P"  888  888  "Y88P"   "Y8888888P"  888    Y888  "Y88P"   "Y888 888 888     "Y88888 d88P     888 8888888P"   "Y8888P"
 *                                                                                           888
 *                                                                                      Y8b d88P
 *                                                                                       "Y88P"
 */

// Busca el primer slot libre disponible para un nuevo anuncio
// Si no hay slots libres y AUTO_REPLACE esta activo, reemplaza el mas antiguo
// Retorna: indice del slot libre (0 a MAX_NOTIFY_ADS-1) o -1 si no hay disponible
static stock Internal_GetFreeAdsSlot(playerid)
{
	if(!IsPlayerConnected(playerid)) return -1;
	for(new i = 0; i < MAX_NOTIFY_ADS; i++)
	{
		if(!nAds::nAdsActive(playerid, i))
		{
			return i;
		}
	}

	#if ADS_AUTO_REPLACE
	new oldestID = -1, oldestOrder = 0x7FFFFFFF;

	for(new i = 0; i < MAX_NOTIFY_ADS; i++)
	{
		if(nAds::nAdsOrder(playerid, i) < oldestOrder)
		{
			oldestOrder = nAds::nAdsOrder(playerid, i);
			oldestID = i;
		}
	}

	if(oldestID != -1)
	{
		if(nAds::nAdsTimerID(playerid, oldestID) != -1)
		{
			KillTimer(nAds::nAdsTimerID(playerid, oldestID));
			nAds::nAdsTimerID(playerid, oldestID) = -1;
		}

		nAds::nAdsActive(playerid, oldestID) = false;

		if(g_AdsCount[playerid] > 0)
			g_AdsCount[playerid]--;

		return oldestID;
	}
	#endif
	return -1;
}

// Actualiza visualmente todos los anuncios activos del jugador
// Destruye todos los textdraws existentes y los recrea en orden
// Los anuncios se ordenan por nAdsOrder (mas recientes primero)
// Retorna: cantidad de anuncios actualizados
static stock Internal_UpdateAllAds(playerid)
{
	for(new i = 0; i < MAX_NOTIFY_ADS; i++)
	{
		if(ntdAds::(playerid, i) == PlayerText:INVALID_TEXT_DRAW) continue;
		PlayerTextDrawDestroy(playerid, ntdAds::(playerid, i));
		ntdAds::(playerid, i) = PlayerText:INVALID_TEXT_DRAW;
	}

	if(g_AdsCount[playerid] == 0) return false;

	new sortedAds[MAX_NOTIFY_ADS];
	new sortedCount = 0;

	for(new i = 0; i < MAX_NOTIFY_ADS; i++)
	{
		if(nAds::nAdsActive(playerid, i))
		{
			sortedAds[sortedCount++] = i;
		}
	}

	for(new i = 0; i < sortedCount - 1; i++)
	{
		for(new j = i + 1; j < sortedCount; j++)
		{
			if(nAds::nAdsOrder(playerid, sortedAds[i]) < nAds::nAdsOrder(playerid, sortedAds[j]))
			{
				new temp = sortedAds[i];
				sortedAds[i] = sortedAds[j];
				sortedAds[j] = temp;
			}
		}
	}

	new Float:PosX = 0.0;
	for(new i = 0; i < sortedCount; i++)
	{
		Internal_CreateNotifyAdsTD(playerid, sortedAds[i], sortedAds[i], PosX);
		PosX += (nAds::nAdsLines(playerid, sortedAds[i]) * 12.0);
	}

	return sortedCount;
}

// Crea el textdraw de un anuncio especifico
// Posiciona el mensaje en la zona central-inferior
// Ajusta la posicion vertical segun la cantidad de lineas
// Retorna: true si se creo correctamente
static stock Internal_CreateNotifyAdsTD(playerid, index, index_textdraw, Float:pos_x)
{
	ADS_TEXTDRAW[playerid][index_textdraw] = CreatePlayerTextDraw(playerid, 140.000, (410.000 - pos_x), nAds::nAdsMsg(playerid, index));
	PlayerTextDrawLetterSize(playerid, ADS_TEXTDRAW[playerid][index_textdraw], 0.180, 1.050);
	PlayerTextDrawTextSize(playerid, ADS_TEXTDRAW[playerid][index_textdraw], 320.000, 0.000);
	PlayerTextDrawAlignment(playerid, ADS_TEXTDRAW[playerid][index_textdraw], 1);
	PlayerTextDrawColor(playerid, ADS_TEXTDRAW[playerid][index_textdraw], -1);
	PlayerTextDrawSetShadow(playerid, ADS_TEXTDRAW[playerid][index_textdraw], SHADOW_TEXT_ADS);
	PlayerTextDrawSetOutline(playerid, ADS_TEXTDRAW[playerid][index_textdraw], OUTLINE_TEXT_ADS);
	PlayerTextDrawBackgroundColor(playerid, ADS_TEXTDRAW[playerid][index_textdraw], 128);
	PlayerTextDrawFont(playerid, ADS_TEXTDRAW[playerid][index_textdraw], FONT_TEXT_ADS);
	PlayerTextDrawSetProportional(playerid, ADS_TEXTDRAW[playerid][index_textdraw], PROPORTIONAL_TEXT_ADS);
	PlayerTextDrawShow(playerid, ADS_TEXTDRAW[playerid][index_textdraw]);
	return true;
}

forward DestroyNotifyADSByID(playerid, adsid);
public DestroyNotifyADSByID(playerid, adsid)
{
	if(adsid < 0 || adsid >= MAX_NOTIFY_ADS) return false;
	if(!nAds::nAdsActive(playerid, adsid)) return false;

	if(nAds::nAdsTimerID(playerid, adsid) != -1)
	{
		KillTimer(nAds::nAdsTimerID(playerid, adsid));
		nAds::nAdsTimerID(playerid, adsid) = -1;
	}

	nAds::nAdsActive(playerid, adsid) = false;
	nAds::nAdsMsg(playerid, adsid)[0] = EOS;
	nAds::nAdsLines(playerid, adsid) = 0;
	nAds::nAdsOrder(playerid, adsid) = 0;

	if(g_AdsCount[playerid] > 0)
		g_AdsCount[playerid]--;

	Internal_UpdateAllAds(playerid);

	return true;
}

forward DestroyNotifyOldestADS(playerid);
public DestroyNotifyOldestADS(playerid)
{
	new oldestID = -1, oldestOrder = 0x7FFFFFFF;

	for(new i = 0; i < MAX_NOTIFY_ADS; i++)
	{
		if(nAds::nAdsActive(playerid, i))
		{
			if(nAds::nAdsOrder(playerid, i) < oldestOrder)
			{
				oldestOrder = nAds::nAdsOrder(playerid, i);
				oldestID = i;
			}
		}
	}

	if(oldestID != -1)
	{
		return DestroyNotifyADSByID(playerid, oldestID);
	}

	return false;
}

/**
 *    Y88b         8888888          888                     .d888
 *     Y88b          888            888                    d88P"
 *      Y88b         888            888                    888
 *       Y88b        888   88888b.  888888 .d88b.  888d888 888888 8888b.   .d8888b .d88b.
 *       d88P        888   888 "88b 888   d8P  Y8b 888P"   888       "88b d88P"   d8P  Y8b
 *      d88P         888   888  888 888   88888888 888     888   .d888888 888     88888888
 *     d88P          888   888  888 Y88b. Y8b.     888     888   888  888 Y88b.   Y8b.
 *    d88P         8888888 888  888  "Y888 "Y8888  888     888   "Y888888  "Y8888P "Y8888
 *
 *
 *
 */

/*
	ShowNotifyADS
	Muestra un anuncio/ad al jugador (zona central-inferior)

	Parametros:
		playerid - ID del jugador
		Mensaje[] - Mensaje a mostrar (soporta codigos de color)
		Segundos - Tiempo en milisegundos antes de auto-destruirse
		sound - ID del sonido a reproducir (0 = sin sonido)

	Retorna:
		ID del anuncio creado (0 a MAX_NOTIFY_ADS-1)
		-1 si no hay slots disponibles

	Ejemplo:
		new adsid = ShowNotifyADS(playerid, "~y~/discord ~w~para mas informacion", 10000, 1085);
*/
stock ShowNotifyADS(playerid, const Mensaje[], Segundos, sound = 0)
{
	new adsid = Internal_GetFreeAdsSlot(playerid);
	if(adsid == -1) return -1;

	strmid(nAds::nAdsMsg(playerid, adsid), TextDrawFixedLatinoText(Mensaje), 0, MAX_LENGTH_MESSAGE);
	nAds::nAdsLines(playerid, adsid) = GetMessageLineCount(TextDrawFixedLatinoText(Mensaje));
	nAds::nAdsActive(playerid, adsid) = true;
	nAds::nAdsOrder(playerid, adsid) = g_AdsOrderCounter[playerid]++;

	#if defined NOT_NOTIFY_DEBUG
		new string[8];
		format(string, sizeof(string), "[#%d]", adsid);
		strins(nAds::nAdsMsg(playerid, adsid), string, 0, MAX_LENGTH_MESSAGE);
	#endif

	g_AdsCount[playerid]++;

	Internal_UpdateAllAds(playerid);

	nAds::nAdsTimerID(playerid, adsid) = SetTimerEx("DestroyNotifyADSByID", Segundos, false, "dd", playerid, adsid);

	if(sound != 0) PlayerPlaySound(playerid, sound, 0.0, 0.0, 0.0);

	return adsid;
}

/*
	IsAdsActive
	Verifica si un anuncio esta activo

	Parametros:
		playerid - ID del jugador
		adsid - ID del anuncio a verificar

	Retorna:
		true si esta activo
		false si no esta activo o el ID es invalido
*/
stock IsAdsActive(playerid, adsid)
{
	if(adsid < 0 || adsid >= MAX_NOTIFY_ADS) return false;
	return nAds::nAdsActive(playerid, adsid);
}

/*
	GetActiveAdsCount
	Obtiene la cantidad de anuncios activos

	Parametros:
		playerid - ID del jugador

	Retorna:
		Cantidad de anuncios activos
*/
stock GetActiveAdsCount(playerid)
{
	return g_AdsCount[playerid];
}

/*
	HideAllAds
	Oculta todos los anuncios activos

	Parametros:
		playerid - ID del jugador

	Retorna:
		Cantidad de anuncios que fueron ocultados
*/
stock HideAllAds(playerid)
{
	new count = 0;
	for(new i = 0; i < MAX_NOTIFY_ADS; i++)
	{
		if(nAds::nAdsActive(playerid, i))
		{
			DestroyNotifyADSByID(playerid, i);
			count++;
		}
	}
	return count;
}

/*
	UpdateAdsText
	Actualiza el texto de un anuncio activo sin reiniciar el timer

	Parametros:
		playerid - ID del jugador
		adsid - ID del anuncio a actualizar
		newText[] - Nuevo texto a mostrar

	Retorna:
		true si se actualizo correctamente
		false si el adsid es invalido o no esta activo
*/
stock UpdateAdsText(playerid, adsid, const newText[])
{
	if(adsid < 0 || adsid >= MAX_NOTIFY_ADS) return false;
	if(!nAds::nAdsActive(playerid, adsid)) return false;

	strmid(nAds::nAdsMsg(playerid, adsid), TextDrawFixedLatinoText(newText), 0, MAX_LENGTH_MESSAGE);
	nAds::nAdsLines(playerid, adsid) = GetMessageLineCount(TextDrawFixedLatinoText(newText));

	Internal_UpdateAllAds(playerid);

	return true;
}

/*
	DestroyAds
	Destruye manualmente un anuncio especifico

	Parametros:
		playerid - ID del jugador
		adsid - ID del anuncio a destruir

	Retorna:
		true si se destruyo correctamente
		false si el adsid es invalido o ya estaba destruido

	Ejemplo:
		if(DestroyAds(playerid, myAdsID))
		{
			// Anuncio destruido exitosamente
		}
*/
stock DestroyAds(playerid, &adsid)
{
	DestroyNotifyADSByID(playerid, adsid);
	adsid = INVALID_NOTIFY_ID;
	return true;
}

/**
 *    Y88b         8888888888                              8888888          888                                      888
 *     Y88b        888                                       888            888                                      888
 *      Y88b       888                                       888            888                                      888
 *       Y88b      8888888 888  888 88888b.   .d8888b        888   88888b.  888888 .d88b.  888d888 88888b.   8888b.  888
 *       d88P      888     888  888 888 "88b d88P"           888   888 "88b 888   d8P  Y8b 888P"   888 "88b     "88b 888
 *      d88P       888     888  888 888  888 888             888   888  888 888   88888888 888     888  888 .d888888 888
 *     d88P        888     Y88b 888 888  888 Y88b.           888   888  888 Y88b. Y8b.     888     888  888 888  888 888
 *    d88P         888      "Y88888 888  888  "Y8888P      8888888 888  888  "Y888 "Y8888  888     888  888 "Y888888 888
 *
 *
 *
 *     .d8888b.  888                             888b     d888
 *    d88P  Y88b 888                             8888b   d8888
 *    Y88b.      888                             88888b.d88888
 *     "Y888b.   88888b.   .d88b.  888  888  888 888Y88888P888  .d88b.  .d8888b  .d8888b   8888b.   .d88b.   .d88b.
 *        "Y88b. 888 "88b d88""88b 888  888  888 888 Y888P 888 d8P  Y8b 88K      88K          "88b d88P"88b d8P  Y8b
 *          "888 888  888 888  888 888  888  888 888  Y8P  888 88888888 "Y8888b. "Y8888b. .d888888 888  888 88888888
 *    Y88b  d88P 888  888 Y88..88P Y88b 888 d88P 888   "   888 Y8b.          X88      X88 888  888 Y88b 888 Y8b.
 *     "Y8888P"  888  888  "Y88P"   "Y8888888P"  888       888  "Y8888   88888P'  88888P' "Y888888  "Y88888  "Y8888
 *                                                                                                      888
 *                                                                                                 Y8b d88P
 *                                                                                                  "Y88P"
 */

// Busca el primer slot libre disponible para un nuevo mensaje
// Si no hay slots libres y AUTO_REPLACE esta activo, reemplaza el mas antiguo
// Retorna: indice del slot libre (0 a MAX_NOTIFY_MESSAGES-1) o -1 si no hay disponible
static stock Internal_GetFreeMessageSlot(playerid)
{
	if(!IsPlayerConnected(playerid)) return -1;
	for(new i = 0; i < MAX_NOTIFY_MESSAGES; i++)
	{
		if(!nMessage::nMessageActive(playerid, i))
		{
			return i;
		}
	}

	#if MESSAGES_AUTO_REPLACE
	new oldestID = -1, oldestOrder = 0x7FFFFFFF;

	for(new i = 0; i < MAX_NOTIFY_MESSAGES; i++)
	{
		if(nMessage::nMessageOrder(playerid, i) < oldestOrder)
		{
			oldestOrder = nMessage::nMessageOrder(playerid, i);
			oldestID = i;
		}
	}

	if(oldestID != -1)
	{
		if(nMessage::nMessageTimerID(playerid, oldestID) != -1)
		{
			KillTimer(nMessage::nMessageTimerID(playerid, oldestID));
			nMessage::nMessageTimerID(playerid, oldestID) = -1;
		}

		nMessage::nMessageActive(playerid, oldestID) = false;

		if(g_MessageCount[playerid] > 0)
			g_MessageCount[playerid]--;

		return oldestID;
	}
	#endif
	return -1;
}

// Actualiza visualmente todos los mensajes activos del jugador
// Destruye todos los textdraws existentes y los recrea en orden
// Los mensajes se ordenan por nMessageOrder (mas recientes primero, arriba)
// Retorna: cantidad de mensajes actualizados
static stock Internal_UpdateAllMessages(playerid)
{
	// Destruir todos los textdraws
	for(new i = 0; i < MAX_NOTIFY_MESSAGES; i++)
	{
		if(ntdMessage::(playerid, i) == PlayerText:INVALID_TEXT_DRAW) continue;
		PlayerTextDrawDestroy(playerid, ntdMessage::(playerid, i));
		ntdMessage::(playerid, i) = PlayerText:INVALID_TEXT_DRAW;
	}

	if(g_MessageCount[playerid] == 0) return false;

	// Ordenar mensajes
	new sortedMessages[MAX_NOTIFY_MESSAGES];
	new sortedCount = 0;

	for(new i = 0; i < MAX_NOTIFY_MESSAGES; i++)
	{
		if(nMessage::nMessageActive(playerid, i))
		{
			sortedMessages[sortedCount++] = i;
		}
	}

	// Bubble sort descendente (mas recientes primero, arriba)
	for(new i = 0; i < sortedCount - 1; i++)
	{
		for(new j = i + 1; j < sortedCount; j++)
		{
			if(nMessage::nMessageOrder(playerid, sortedMessages[i]) < nMessage::nMessageOrder(playerid, sortedMessages[j]))
			{
				new temp = sortedMessages[i];
				sortedMessages[i] = sortedMessages[j];
				sortedMessages[j] = temp;
			}
		}
	}

	// Recrear mensajes (mas recientes arriba)
	new Float:PosY = 0.0;
	for(new i = 0; i < sortedCount; i++)
	{
		Internal_CreateMessageTD(playerid, sortedMessages[i], sortedMessages[i], PosY);
		PosY += (nMessage::nMessageLines(playerid, sortedMessages[i]) * 12.0); // Espaciado vertical
	}

	return sortedCount;
}

// Crea el textdraw de un mensaje especifico
// Posiciona el mensaje centrado sobre el chat
// Aplica configuracion personalizada (fuente, shadow, outline)
// Ajusta la posicion vertical segun la cantidad de lineas
// Retorna: true si se creo correctamente
static stock Internal_CreateMessageTD(playerid, index, index_textdraw, Float:pos_y)
{
	MESSAGE_TEXTDRAW[playerid][index_textdraw] = CreatePlayerTextDraw(playerid, 320.000, (400.000 - pos_y), nMessage::nMessageMsg(playerid, index));
	PlayerTextDrawLetterSize(playerid, MESSAGE_TEXTDRAW[playerid][index_textdraw], 0.180, 1.050);
	PlayerTextDrawTextSize(playerid, MESSAGE_TEXTDRAW[playerid][index_textdraw], 0.000, 300.000);
	PlayerTextDrawAlignment(playerid, MESSAGE_TEXTDRAW[playerid][index_textdraw], 2);
	PlayerTextDrawColor(playerid, MESSAGE_TEXTDRAW[playerid][index_textdraw], -1);

	// Aplicar configuracion personalizada o por defecto
	PlayerTextDrawSetShadow(playerid, MESSAGE_TEXTDRAW[playerid][index_textdraw], nMessage::nMessageShadow(playerid, index) ? 1 : 0);
	PlayerTextDrawSetOutline(playerid, MESSAGE_TEXTDRAW[playerid][index_textdraw], nMessage::nMessageOutline(playerid, index) ? 1 : 0);
	PlayerTextDrawFont(playerid, MESSAGE_TEXTDRAW[playerid][index_textdraw], nMessage::nMessageFont(playerid, index));

	PlayerTextDrawBackgroundColor(playerid, MESSAGE_TEXTDRAW[playerid][index_textdraw], 150);
	PlayerTextDrawSetProportional(playerid, MESSAGE_TEXTDRAW[playerid][index_textdraw], PROPORTIONAL_TEXT_MESSAGES);
	PlayerTextDrawShow(playerid, MESSAGE_TEXTDRAW[playerid][index_textdraw]);
	return true;
}

forward DestroyMessageByID(playerid, messageid);
public DestroyMessageByID(playerid, messageid)
{
	if(messageid < 0 || messageid >= MAX_NOTIFY_MESSAGES) return false;
	if(!nMessage::nMessageActive(playerid, messageid)) return false;

	if(nMessage::nMessageTimerID(playerid, messageid) != -1)
	{
		KillTimer(nMessage::nMessageTimerID(playerid, messageid));
		nMessage::nMessageTimerID(playerid, messageid) = -1;
	}

	nMessage::nMessageActive(playerid, messageid) = false;
	nMessage::nMessageMsg(playerid, messageid)[0] = EOS;
	nMessage::nMessageLines(playerid, messageid) = 0;
	nMessage::nMessageOrder(playerid, messageid) = 0;
	nMessage::nMessageFont(playerid, messageid) = FONT_TEXT_MESSAGES;
	nMessage::nMessageShadow(playerid, messageid) = true;
	nMessage::nMessageOutline(playerid, messageid) = false;

	if(g_MessageCount[playerid] > 0)
		g_MessageCount[playerid]--;

	Internal_UpdateAllMessages(playerid);

	return true;
}

forward DestroyOldestMessage(playerid);
public DestroyOldestMessage(playerid)
{
	new oldestID = -1, oldestOrder = 0x7FFFFFFF;

	for(new i = 0; i < MAX_NOTIFY_MESSAGES; i++)
	{
		if(nMessage::nMessageActive(playerid, i))
		{
			if(nMessage::nMessageOrder(playerid, i) < oldestOrder)
			{
				oldestOrder = nMessage::nMessageOrder(playerid, i);
				oldestID = i;
			}
		}
	}

	if(oldestID != -1)
	{
		return DestroyMessageByID(playerid, oldestID);
	}

	return false;
}

/**
 *    Y88b         8888888          888                     .d888
 *     Y88b          888            888                    d88P"
 *      Y88b         888            888                    888
 *       Y88b        888   88888b.  888888 .d88b.  888d888 888888 8888b.   .d8888b .d88b.
 *       d88P        888   888 "88b 888   d8P  Y8b 888P"   888       "88b d88P"   d8P  Y8b
 *      d88P         888   888  888 888   88888888 888     888   .d888888 888     88888888
 *     d88P          888   888  888 Y88b. Y8b.     888     888   888  888 Y88b.   Y8b.
 *    d88P         8888888 888  888  "Y888 "Y8888  888     888   "Y888888  "Y8888P "Y8888
 *
 *
 *
 */

/*
	ShowMessage
	Muestra un mensaje TextDraw al jugador

	Parametros:
		playerid - ID del jugador
		Mensaje[] - Mensaje a mostrar
		Segundos - Tiempo en milisegundos antes de auto-destruirse
		sound - Sonido a reproducir (0 = sin sonido)
		font - Fuente del texto (0-3, por defecto 1)
		shadow - Aplicar sombra al texto (por defecto true)
		outline - Aplicar contorno al texto (por defecto false)

	Retorna:
		ID del mensaje creado (0 a MAX_NOTIFY_MESSAGES-1)
		-1 si no hay slots disponibles

	Ejemplo:
		new msgid = ShowMessage(playerid, "~g~Bienvenido", 5000, 1083, 1, true, false);
*/
stock ShowMessage(playerid, const Mensaje[], Segundos = 3000, sound = 0, font = FONT_TEXT_MESSAGES, bool:shadow = bool:SHADOW_TEXT_MESSAGES, bool:outline = bool:OUTLINE_TEXT_MESSAGES)
{
	new msglen = strlen(Mensaje);
	if(msglen >= MAX_LENGTH_MESSAGE) return -1;

	new messageid = Internal_GetFreeMessageSlot(playerid);
	if(messageid == -1) return -1;

	strmid(nMessage::nMessageMsg(playerid, messageid), TextDrawFixedLatinoText(Mensaje), 0, MAX_LENGTH_MESSAGE);
	nMessage::nMessageLines(playerid, messageid) = GetMessageLineCount(TextDrawFixedLatinoText(Mensaje));
	nMessage::nMessageActive(playerid, messageid) = true;
	nMessage::nMessageOrder(playerid, messageid) = g_MessageOrderCounter[playerid]++;
	nMessage::nMessageFont(playerid, messageid) = font;
	nMessage::nMessageShadow(playerid, messageid) = shadow;
	nMessage::nMessageOutline(playerid, messageid) = outline;

	#if defined NOT_NOTIFY_DEBUG
		new string[8];
		format(string, sizeof(string), "[#%d]", messageid);
		strins(nMessage::nMessageMsg(playerid, messageid), string, 0, MAX_LENGTH_MESSAGE);
	#endif

	g_MessageCount[playerid]++;

	Internal_UpdateAllMessages(playerid);

	nMessage::nMessageTimerID(playerid, messageid) = SetTimerEx("DestroyMessageByID", Segundos, false, "dd", playerid, messageid);

	if(sound != 0) PlayerPlaySound(playerid, sound, 0.0, 0.0, 0.0);

	return messageid;
}

/*
	IsMessageActive
	Verifica si un mensaje esta activo
*/
stock IsMessageActive(playerid, messageid)
{
	if(messageid < 0 || messageid >= MAX_NOTIFY_MESSAGES) return false;
	return nMessage::nMessageActive(playerid, messageid);
}

/*
	GetActiveMessageCount
	Obtiene la cantidad de mensajes activos
*/
stock GetActiveMessageCount(playerid)
{
	return g_MessageCount[playerid];
}

/*
	HideAllMessages
	Oculta todos los mensajes activos
*/
stock HideAllMessages(playerid)
{
	new count = 0;
	for(new i = 0; i < MAX_NOTIFY_MESSAGES; i++)
	{
		if(nMessage::nMessageActive(playerid, i))
		{
			DestroyMessageByID(playerid, i);
			count++;
		}
	}
	return count;
}

/*
	UpdateMessageText
	Actualiza el texto de un mensaje activo sin reiniciar el timer
*/
stock UpdateMessageText(playerid, messageid, const newText[])
{
	if(messageid < 0 || messageid >= MAX_NOTIFY_MESSAGES) return false;
	if(!nMessage::nMessageActive(playerid, messageid)) return false;

	strmid(nMessage::nMessageMsg(playerid, messageid), TextDrawFixedLatinoText(newText), 0, MAX_LENGTH_MESSAGE);
	nMessage::nMessageLines(playerid, messageid) = GetMessageLineCount(TextDrawFixedLatinoText(newText));

	Internal_UpdateAllMessages(playerid);

	return true;
}

/*
	DestroyMessageManual
	Destruye manualmente un mensaje especifico

	Parametros:
		playerid - ID del jugador
		messageid - ID del mensaje a destruir

	Retorna:
		true si se destruyo correctamente
		false si el messageid es invalido o ya estaba destruido

	Ejemplo:
		if(DestroyMessageManual(playerid, myMessageID))
		{
			// Mensaje destruido exitosamente
		}

	Nota:
		Se llama "Manual" para evitar conflicto con la funcion
		DestroyMessage que destruye el mas antiguo.
*/
stock DestroyMessageManual(playerid, &messageid)
{
	DestroyMessageByID(playerid, messageid);
	messageid = INVALID_NOTIFY_ID;
	return true;
}

/**
 *    Y88b         8888888888                              8888888          888                                      888
 *     Y88b        888                                       888            888                                      888
 *      Y88b       888                                       888            888                                      888
 *       Y88b      8888888 888  888 88888b.   .d8888b        888   88888b.  888888 .d88b.  888d888 88888b.   8888b.  888
 *       d88P      888     888  888 888 "88b d88P"           888   888 "88b 888   d8P  Y8b 888P"   888 "88b     "88b 888
 *      d88P       888     888  888 888  888 888             888   888  888 888   88888888 888     888  888 .d888888 888
 *     d88P        888     Y88b 888 888  888 Y88b.           888   888  888 Y88b. Y8b.     888     888  888 888  888 888
 *    d88P         888      "Y88888 888  888  "Y8888P      8888888 888  888  "Y888 "Y8888  888     888  888 "Y888888 888
 *
 *
 *
 *     .d8888b.  888                             888                            888b     d888
 *    d88P  Y88b 888                             888                            8888b   d8888
 *    Y88b.      888                             888                            88888b.d88888
 *     "Y888b.   88888b.   .d88b.  888  888  888 888      .d88b.  888  888  888 888Y88888P888  .d88b.  .d8888b  .d8888b   8888b.   .d88b.   .d88b.
 *        "Y88b. 888 "88b d88""88b 888  888  888 888     d88""88b 888  888  888 888 Y888P 888 d8P  Y8b 88K      88K          "88b d88P"88b d8P  Y8b
 *          "888 888  888 888  888 888  888  888 888     888  888 888  888  888 888  Y8P  888 88888888 "Y8888b. "Y8888b. .d888888 888  888 88888888
 *    Y88b  d88P 888  888 Y88..88P Y88b 888 d88P 888     Y88..88P Y88b 888 d88P 888   "   888 Y8b.          X88      X88 888  888 Y88b 888 Y8b.
 *     "Y8888P"  888  888  "Y88P"   "Y8888888P"  88888888 "Y88P"   "Y8888888P"  888       888  "Y8888   88888P'  88888P' "Y888888  "Y88888  "Y8888
 *                                                                                                                                     888
 *                                                                                                                                Y8b d88P
 *                                                                                                                                 "Y88P"
 */

// Busca el primer slot libre disponible para un nuevo low message
// Si no hay slots libres y AUTO_REPLACE esta activo, reemplaza el mas antiguo
// Retorna: indice del slot libre (0 a MAX_NOTIFY_LOW_MESSAGES-1) o -1 si no hay disponible
static stock Internal_GetFreeLowMessageSlot(playerid)
{
	if(!IsPlayerConnected(playerid)) return -1;
	for(new i = 0; i < MAX_NOTIFY_LOW_MESSAGES; i++)
	{
		if(!nLowMessage::nLowMessageActive(playerid, i))
		{
			return i;
		}
	}

	#if LOW_MESSAGES_AUTO_REPLACE
	new oldestID = -1, oldestOrder = 0x7FFFFFFF;

	for(new i = 0; i < MAX_NOTIFY_LOW_MESSAGES; i++)
	{
		if(nLowMessage::nLowMessageOrder(playerid, i) < oldestOrder)
		{
			oldestOrder = nLowMessage::nLowMessageOrder(playerid, i);
			oldestID = i;
		}
	}

	if(oldestID != -1)
	{
		if(nLowMessage::nLowMessageTimerID(playerid, oldestID) != -1)
		{
			KillTimer(nLowMessage::nLowMessageTimerID(playerid, oldestID));
			nLowMessage::nLowMessageTimerID(playerid, oldestID) = -1;
		}

		nLowMessage::nLowMessageActive(playerid, oldestID) = false;

		if(g_LowMessageCount[playerid] > 0)
			g_LowMessageCount[playerid]--;

		return oldestID;
	}
	#endif
	return -1;
}

// Actualiza visualmente todos los low messages activos del jugador
// Destruye todos los textdraws existentes y los recrea en orden
// Los low messages se ordenan por nLowMessageOrder (mas recientes primero, arriba)
// Retorna: cantidad de low messages actualizados
static stock Internal_UpdateAllLowMessages(playerid)
{
	// Destruir todos los textdraws
	for(new i = 0; i < MAX_NOTIFY_LOW_MESSAGES; i++)
	{
		if(ntdLowMessage::(playerid, i) == PlayerText:INVALID_TEXT_DRAW) continue;
		PlayerTextDrawDestroy(playerid, ntdLowMessage::(playerid, i));
		ntdLowMessage::(playerid, i) = PlayerText:INVALID_TEXT_DRAW;
	}

	if(g_LowMessageCount[playerid] == 0) return false;

	// Ordenar low messages
	new sortedLowMessages[MAX_NOTIFY_LOW_MESSAGES];
	new sortedCount = 0;

	for(new i = 0; i < MAX_NOTIFY_LOW_MESSAGES; i++)
	{
		if(nLowMessage::nLowMessageActive(playerid, i))
		{
			sortedLowMessages[sortedCount++] = i;
		}
	}

	// Bubble sort descendente (mas recientes primero, arriba)
	for(new i = 0; i < sortedCount - 1; i++)
	{
		for(new j = i + 1; j < sortedCount; j++)
		{
			if(nLowMessage::nLowMessageOrder(playerid, sortedLowMessages[i]) < nLowMessage::nLowMessageOrder(playerid, sortedLowMessages[j]))
			{
				new temp = sortedLowMessages[i];
				sortedLowMessages[i] = sortedLowMessages[j];
				sortedLowMessages[j] = temp;
			}
		}
	}

	// Recrear low messages (mas recientes arriba)
	new Float:PosY = 0.0;
	for(new i = 0; i < sortedCount; i++)
	{
		Internal_CreateLowMessageTD(playerid, sortedLowMessages[i], sortedLowMessages[i], PosY);
		PosY += (nLowMessage::nLowMessageLines(playerid, sortedLowMessages[i]) * 12.0); // Espaciado vertical
	}

	return sortedCount;
}

// Crea el textdraw de un low message especifico
// Posiciona el mensaje centrado en la parte inferior de la pantalla
// Aplica configuracion personalizada (fuente, shadow, outline)
// Ajusta la posicion vertical segun la cantidad de lineas
// Retorna: true si se creo correctamente
static stock Internal_CreateLowMessageTD(playerid, index, index_textdraw, Float:pos_y)
{
	LOW_MESSAGE_TEXTDRAW[playerid][index_textdraw] = CreatePlayerTextDraw(playerid, 320.000, (435.000 - pos_y), nLowMessage::nLowMessageMsg(playerid, index));
	PlayerTextDrawLetterSize(playerid, LOW_MESSAGE_TEXTDRAW[playerid][index_textdraw], 0.180, 1.050);
	PlayerTextDrawAlignment(playerid, LOW_MESSAGE_TEXTDRAW[playerid][index_textdraw], 2);
	PlayerTextDrawColor(playerid, LOW_MESSAGE_TEXTDRAW[playerid][index_textdraw], -156);

	// Aplicar configuracion personalizada o por defecto
	PlayerTextDrawSetShadow(playerid, LOW_MESSAGE_TEXTDRAW[playerid][index_textdraw], nLowMessage::nLowMessageShadow(playerid, index) ? 1 : 0);
	PlayerTextDrawSetOutline(playerid, LOW_MESSAGE_TEXTDRAW[playerid][index_textdraw], nLowMessage::nLowMessageOutline(playerid, index) ? 1 : 0);
	PlayerTextDrawFont(playerid, LOW_MESSAGE_TEXTDRAW[playerid][index_textdraw], nLowMessage::nLowMessageFont(playerid, index));

	PlayerTextDrawBackgroundColor(playerid, LOW_MESSAGE_TEXTDRAW[playerid][index_textdraw], -128);
	PlayerTextDrawSetProportional(playerid, LOW_MESSAGE_TEXTDRAW[playerid][index_textdraw], PROPORTIONAL_TEXT_LOW_MESSAGES);
	PlayerTextDrawShow(playerid, LOW_MESSAGE_TEXTDRAW[playerid][index_textdraw]);
	return true;
}

forward DestroyLowMessageByID(playerid, lowmessageid);
public DestroyLowMessageByID(playerid, lowmessageid)
{
	if(lowmessageid < 0 || lowmessageid >= MAX_NOTIFY_LOW_MESSAGES) return false;
	if(!nLowMessage::nLowMessageActive(playerid, lowmessageid)) return false;

	if(nLowMessage::nLowMessageTimerID(playerid, lowmessageid) != -1)
	{
		KillTimer(nLowMessage::nLowMessageTimerID(playerid, lowmessageid));
		nLowMessage::nLowMessageTimerID(playerid, lowmessageid) = -1;
	}

	nLowMessage::nLowMessageActive(playerid, lowmessageid) = false;
	nLowMessage::nLowMessageMsg(playerid, lowmessageid)[0] = EOS;
	nLowMessage::nLowMessageLines(playerid, lowmessageid) = 0;
	nLowMessage::nLowMessageOrder(playerid, lowmessageid) = 0;
	nLowMessage::nLowMessageFont(playerid, lowmessageid) = FONT_TEXT_LOW_MESSAGES;
	nLowMessage::nLowMessageShadow(playerid, lowmessageid) = false;
	nLowMessage::nLowMessageOutline(playerid, lowmessageid) = false;

	if(g_LowMessageCount[playerid] > 0)
		g_LowMessageCount[playerid]--;

	Internal_UpdateAllLowMessages(playerid);

	return true;
}

forward DestroyOldestLowMessage(playerid);
public DestroyOldestLowMessage(playerid)
{
	new oldestID = -1, oldestOrder = 0x7FFFFFFF;

	for(new i = 0; i < MAX_NOTIFY_LOW_MESSAGES; i++)
	{
		if(nLowMessage::nLowMessageActive(playerid, i))
		{
			if(nLowMessage::nLowMessageOrder(playerid, i) < oldestOrder)
			{
				oldestOrder = nLowMessage::nLowMessageOrder(playerid, i);
				oldestID = i;
			}
		}
	}

	if(oldestID != -1)
	{
		return DestroyLowMessageByID(playerid, oldestID);
	}

	return false;
}

/**
 *    Y88b         8888888          888                     .d888
 *     Y88b          888            888                    d88P"
 *      Y88b         888            888                    888
 *       Y88b        888   88888b.  888888 .d88b.  888d888 888888 8888b.   .d8888b .d88b.
 *       d88P        888   888 "88b 888   d8P  Y8b 888P"   888       "88b d88P"   d8P  Y8b
 *      d88P         888   888  888 888   88888888 888     888   .d888888 888     88888888
 *     d88P          888   888  888 Y88b. Y8b.     888     888   888  888 Y88b.   Y8b.
 *    d88P         8888888 888  888  "Y888 "Y8888  888     888   "Y888888  "Y8888P "Y8888
 *
 *
 *
 */

/*
	ShowLowMessage
	Muestra un mensaje TextDraw en la parte inferior de la pantalla

	Parametros:
		playerid - ID del jugador
		Mensaje[] - Mensaje a mostrar
		Segundos - Tiempo en milisegundos antes de auto-destruirse
		sound - Sonido a reproducir (0 = sin sonido)
		font - Fuente del texto (0-3, por defecto 1)
		shadow - Aplicar sombra al texto (por defecto false)
		outline - Aplicar contorno al texto (por defecto false)

	Retorna:
		ID del low message creado (0 a MAX_NOTIFY_LOW_MESSAGES-1)
		-1 si no hay slots disponibles

	Ejemplo:
		new msgid = ShowLowMessage(playerid, "~y~Suscribete al canal", 5000, 1083, 1, false, false);
*/
stock ShowLowMessage(playerid, const Mensaje[], Segundos = 3000, sound = 0, font = FONT_TEXT_LOW_MESSAGES, bool:shadow = bool:SHADOW_TEXT_LOW_MESSAGES, bool:outline = bool:OUTLINE_TEXT_LOW_MESSAGES)
{
	new msglen = strlen(Mensaje);
	if(msglen >= MAX_LENGTH_MESSAGE) return -1;

	new lowmessageid = Internal_GetFreeLowMessageSlot(playerid);
	if(lowmessageid == -1) return -1;

	strmid(nLowMessage::nLowMessageMsg(playerid, lowmessageid), TextDrawFixedLatinoText(Mensaje), 0, MAX_LENGTH_MESSAGE);
	nLowMessage::nLowMessageLines(playerid, lowmessageid) = GetMessageLineCount(TextDrawFixedLatinoText(Mensaje));
	nLowMessage::nLowMessageActive(playerid, lowmessageid) = true;
	nLowMessage::nLowMessageOrder(playerid, lowmessageid) = g_LowMessageOrderCounter[playerid]++;
	nLowMessage::nLowMessageFont(playerid, lowmessageid) = font;
	nLowMessage::nLowMessageShadow(playerid, lowmessageid) = shadow;
	nLowMessage::nLowMessageOutline(playerid, lowmessageid) = outline;

	#if defined NOT_NOTIFY_DEBUG
		new string[8];
		format(string, sizeof(string), "[#%d]", lowmessageid);
		strins(nLowMessage::nLowMessageMsg(playerid, lowmessageid), string, 0, MAX_LENGTH_MESSAGE);
	#endif

	g_LowMessageCount[playerid]++;

	Internal_UpdateAllLowMessages(playerid);

	nLowMessage::nLowMessageTimerID(playerid, lowmessageid) = SetTimerEx("DestroyLowMessageByID", Segundos, false, "dd", playerid, lowmessageid);

	if(sound != 0) PlayerPlaySound(playerid, sound, 0.0, 0.0, 0.0);

	return lowmessageid;
}

/*
	IsLowMessageActive
	Verifica si un low message esta activo

	Parametros:
		playerid - ID del jugador
		lowmessageid - ID del low message a verificar

	Retorna:
		true si esta activo
		false si no esta activo o el ID es invalido
*/
stock IsLowMessageActive(playerid, lowmessageid)
{
	if(lowmessageid < 0 || lowmessageid >= MAX_NOTIFY_LOW_MESSAGES) return false;
	return nLowMessage::nLowMessageActive(playerid, lowmessageid);
}

/*
	GetActiveLowMessageCount
	Obtiene la cantidad de low messages activos

	Parametros:
		playerid - ID del jugador

	Retorna:
		Cantidad de low messages activos
*/
stock GetActiveLowMessageCount(playerid)
{
	return g_LowMessageCount[playerid];
}

/*
	HideAllLowMessages
	Oculta todos los low messages activos

	Parametros:
		playerid - ID del jugador

	Retorna:
		Cantidad de low messages que fueron ocultados
*/
stock HideAllLowMessages(playerid)
{
	new count = 0;
	for(new i = 0; i < MAX_NOTIFY_LOW_MESSAGES; i++)
	{
		if(nLowMessage::nLowMessageActive(playerid, i))
		{
			DestroyLowMessageByID(playerid, i);
			count++;
		}
	}
	return count;
}

/*
	UpdateLowMessageText
	Actualiza el texto de un low message activo sin reiniciar el timer

	Parametros:
		playerid - ID del jugador
		lowmessageid - ID del low message a actualizar
		newText[] - Nuevo texto a mostrar

	Retorna:
		true si se actualizo correctamente
		false si el lowmessageid es invalido o no esta activo
*/
stock UpdateLowMessageText(playerid, lowmessageid, const newText[])
{
	if(lowmessageid < 0 || lowmessageid >= MAX_NOTIFY_LOW_MESSAGES) return false;
	if(!nLowMessage::nLowMessageActive(playerid, lowmessageid)) return false;

	strmid(nLowMessage::nLowMessageMsg(playerid, lowmessageid), TextDrawFixedLatinoText(newText), 0, MAX_LENGTH_MESSAGE);
	nLowMessage::nLowMessageLines(playerid, lowmessageid) = GetMessageLineCount(TextDrawFixedLatinoText(newText));

	Internal_UpdateAllLowMessages(playerid);

	return true;
}

/*
	DestroyLowMessageManual
	Destruye manualmente un low message especifico

	Parametros:
		playerid - ID del jugador
		lowmessageid - ID del low message a destruir

	Retorna:
		true si se destruyo correctamente
		false si el lowmessageid es invalido o ya estaba destruido

	Ejemplo:
		if(DestroyLowMessageManual(playerid, myLowMessageID))
		{
			// Low message destruido exitosamente
		}

	Nota:
		Se llama "Manual" para evitar conflicto con la funcion
		DestroyLowMessage que destruye el mas antiguo.
*/
stock DestroyLowMessageManual(playerid, &lowmessageid)
{
	DestroyLowMessageByID(playerid, lowmessageid);
	lowmessageid = INVALID_NOTIFY_ID;
	return true;
}

// # END :D
