//  This File Came From MMOBugs.com
//  Please Do Not Distribute Without Authors or MMOBugs Consent.

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//
// Projet: MQ2NetBots.cpp
// Author: s0rCieR
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//
//
// Deadchicken added .Duration on or about September 2007 and tried not to
// mangle s0rCieR's code to much.  Thanks to Kroak for helping debug.
//  Updated for 10/9 release of MQ2
// CombatState member added  Thanks mijuki.
// .Stacks member added
//
// Up to 1.23: Updated by several people (www.macroquest2.com) VIP forums.
// 1.24 - 8/11/09: mmobugs.com/devestator, added code to Buff, ShortBuff, PetBuff to allow index by name as well as number
// 1.25: Small correction to Buff code
// 1.26: Added INI value to allow 'extended' info spam
// v2.1 woobs
//    Upped most of the maximum values to handle more buffs.
//    Add Detrimental information to merge in MQ2Debuffs functions
// v2.2 Added String Safety - eqmule
// v3.0 woobs
//    Updated .Stacks
//    Added   .StacksPet
// v3.01+ htw
//    Lot of fixes/additions, TLO handling and additions

#include <mq/Plugin.h>

#include "../Blech/Blech.h"

#include <string>
#include <map>
#include <iostream>
#include <sstream>

using namespace std;


double compile_date();
#define        PLUGIN_DATE           compile_date()
#define        PLUGIN_VERS           (double)3.93
#define	       PLUGIN_TITLE          "MQ2NetBots"

#define        PETS_MAX              65
#define        NOTE_MAX              500
#define        DETR_MAX              30

#define        NETTICK               50
#define        REFRESH               60000
#define        UPDATES               6000
#define        UIUPDATE              100

#define        DEBUGGING             0

PreSetup("MQ2NetBots");
PLUGIN_VERSION(PLUGIN_VERS);

double compile_date()
{
	string datestr = __DATE__;
	string timestr = __TIME__;

	istringstream iss_date(datestr);
	string str_month;
	int day;
	int year;
	iss_date >> str_month >> day >> year;

	int month;
	if (str_month == "Jan") month = 1;
	else if (str_month == "Feb") month = 2;
	else if (str_month == "Mar") month = 3;
	else if (str_month == "Apr") month = 4;
	else if (str_month == "May") month = 5;
	else if (str_month == "Jun") month = 6;
	else if (str_month == "Jul") month = 7;
	else if (str_month == "Aug") month = 8;
	else if (str_month == "Sep") month = 9;
	else if (str_month == "Oct") month = 10;
	else if (str_month == "Nov") month = 11;
	else if (str_month == "Dec") month = 12;
	else return ((double)0.0);
	return ((double)year + ((double)month / (double)100.0) + ((double)day / (double)10000.0));
}

struct PredIgnoreCase
{
	bool operator() (const string& str1, const string& str2) const
	{
		string str1NoCase(str1), str2NoCase(str2);
		transform(str1.begin(), str1.end(), str1NoCase.begin(), ::tolower);
		transform(str2.begin(), str2.end(), str2NoCase.begin(), ::tolower);
		return (str1NoCase < str2NoCase);
	};
};

enum
{
	STATE_DEAD = 0x00000001, STATE_FEIGN = 0x00000002, STATE_DUCK = 0x00000004, STATE_BIND = 0x00000008,
	STATE_STAND = 0x00000010, STATE_SIT = 0x00000020, STATE_MOUNT = 0x00000040, STATE_INVIS = 0x00000080,
	STATE_LEV = 0x00000100, STATE_ATTACK = 0x00000200, STATE_MOVING = 0x00000400, STATE_STUN = 0x00000800,
	STATE_RAID = 0x00001000, STATE_GROUP = 0x00002000, STATE_LFG = 0x00004000, STATE_AFK = 0x00008000,
	STATE_ITU = 0x00010000, STATE_RANGED = 0x00020000, STATE_WANTAGGRO = 0x00040000, STATE_HAVEAGGRO = 0x00080000,
	STATE_HOVER = 0x00100000
};

enum
{
	BUFFS, CASTD, ENDUS, EXPER, LEADR, LEVEL, LIFES, MANAS, PBUFF,
	PETIL, SPGEM, SONGS, STATE, TARGT, ZONES, DURAS, LOCAT, HEADN,
	AAPTS, OOCST, NOTE, DETR, MSTATE, MNAME, NAVACT, NAVPAU, NVERS,
	BOTACT, CAMPSTATUS, CAMPX, CAMPY, CAMPRADIUS, CAMPDISTANCE, EQBC,
	FREEINV, GROUPLEADER, ESIZE
};

enum
{
	RESERVED, DETRIMENTALS, COUNTERS, BLINDED, CASTINGLEVEL, CHARMED, CURSED, DISEASED, ENDUDRAIN,
	FEARED, HEALING, INVULNERABLE, LIFEDRAIN, MANADRAIN, MESMERIZED, POISONED, RESISTANCE, ROOTED,
	SILENCED, SLOWED, SNARED, SPELLCOST, SPELLDAMAGE, SPELLSLOWED, TRIGGR, CORRUPTED, NOCURE, DSIZE
};

enum
{
	MACRO_NONE, MACRO_RUNNING, MACRO_PAUSED
};

class BotInfo
{
public:
	char              Name[0x40];          // Client NAME
	char              Leader[0x40];        // Leader Name
	DWORD             State;               // State
	long              ZoneID;              // Zone ID
	long              InstID;              // Instance ID
	long              SpawnID;             // Spawn ID
	long              ClassID;             // Class ID
	long              Level;               // Level
	long              CastID;              // Casting Spell ID
	long              LifeCur;             // HP Current
	long              LifeMax;             // HP Maximum
	long              EnduCur;             // ENDU Current
	long              EnduMax;             // ENDU Maximum
	long              ManaCur;             // MANA Current
	long              ManaMax;             // MANA Maximum
	long              PetID;               // PET ID
	long              PetHP;               // PET HP Percentage
	long              TargetID;            // Target ID
	long              TargetHP;            // Target HP Percentage
	long              Gem[NUM_SPELL_GEMS]; // Spell Memorized
	long              Pets[PETS_MAX];      // Spell Pets
	long              Song[NUM_SHORT_BUFFS]; // Spell Song
	long              Buff[NUM_LONG_BUFFS]; // Spell Buff
	long              Duration[NUM_LONG_BUFFS]; // Buff duration
	long              FreeBuff;            // FreeBuffSlot;
	char			  GroupLeader[64];	   // Group Leader Name
#ifdef EMU
	double            glXP;                // glXP
#endif
	DWORD             aaXP;                // aaXP
	__int64           XP;                  // XP
	DWORD             Updated;             // Update
	char			  Location[0x40];	   // Y,X,Z
	char			  Heading[0x40];       // Heading
	long              TotalAA;             // totalAA
	long              UsedAA;              // usedAA
	long              UnusedAA;            // unusedAA
	DWORD             CombatState;         // CombatState
	char              Note[NOTE_MAX];      // User Mesg
	int               Detrimental[DSIZE];
	int				  MacroState;
	char			  MacroName[MAX_PATH];
	bool			  NavActive;
	bool			  NavPaused;
	double			  Version;
	bool			  bBotActive;
	bool              MakeCampStatus;
	double            MakeCampX;
	double            MakeCampY;
	double            MakeCampRadius;
	double			  MakeCampDistance;
	float			  X;					// Break loc down
	float			  Y;					//
	float			  Z;					//
	ULONGLONG         EQBC_Packets;
	ULONGLONG         EQBC_HeartBeat;
	int				  FreeInventory;
};

#define TIME unsigned long long

long                NetInit = 0;           // Plugin Initialized?
long                NetStat = 0;           // Plugin is On?
long                NetGrab = 0;           // Grab Information?
long                NetSend = 0;           // Send Information?
long                NetLast = 0;           // Last Send Time Mark
char                NetNote[NOTE_MAX];   // Network Note
TIME				NetShow = 0;			// Flag for showing UI window

map<string, BotInfo, PredIgnoreCase> NetMap;              // BotInfo Mapped List
Blech               Packet('#');         // BotInfo Event Triggers
BotInfo* CurBot = 0;            // BotInfo Current

long                sTimers[ESIZE];      // Save Timers
char                sBuffer[ESIZE][MAX_STRING]; // Save Buffer
char                wBuffer[ESIZE][MAX_STRING]; // Work Buffer
bool                wChange[ESIZE];      // Work Change
bool                wUpdate[ESIZE];      // Work Update
int                 dValues[DSIZE];
bool				bExtended = false;   // Allow extended info?
bool				bUseSimpleSearch = false;  // Allow any substring of Buff/ShortBuff/PetBuff to return result

bool				bSendBuffs = false;
bool				bSendPetBuffs = false;

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//

void WriteWindowINI(CSidlScreenWnd* pWindow);
void ReadWindowINI(CSidlScreenWnd* pWindow);
void DestroyMyWindow();
void CreateMyWindow();
void HideMyWindow();
void ShowMyWindow();
void WindowUpdate();

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//
char zOutput[MAX_STRING] = { 0 };

long Evaluate(char* zFormat, ...)
{
	va_list vaList; va_start(vaList, zFormat);
	vsprintf_s(zOutput, zFormat, vaList); if (!zOutput[0]) return 1;
	//DebugSpewAlways("E[%s]",zOutput);
	ParseMacroData(zOutput, MAX_STRING);
	//DebugSpewAlways("R[%s]",zOutput);
	return atoi(zOutput);
}


bool EQBCConnected()
{
	using fEqbcIsConnected = uint16_t(*)();

	if (fEqbcIsConnected checkf = (fEqbcIsConnected)GetPluginProc("MQ2EQBC", "isConnected"))
	{
		return checkf();
	}
	return false;
}

void EQBCBroadCast(PCHAR Buffer)
{
	using fEqbcNetBotSendMsg = void(*)(const char*);
	if (strlen(Buffer) > 9)
	{
		if (fEqbcNetBotSendMsg requestf = (fEqbcNetBotSendMsg)GetPluginProc("MQ2EQBC", "NetBotSendMsg"))
		{
#if DEBUGGING > 1
			DebugSpewAlways("%s->BroadCasting(%s)", PLUGIN_TITLE, Buffer);
#endif
			requestf(Buffer);
		}
	}
}

BotInfo* BotFind(const char* Name)
{
	auto f = NetMap.find(Name);
	return (NetMap.end() == f) ? NULL : &(*f).second;
}

BotInfo* BotLoad(PCHAR Name)
{
	auto f = NetMap.find(Name);
	if (NetMap.end() == f)
	{
		BotInfo RecInfo;
		ZeroMemory(&RecInfo.Name, sizeof(BotInfo));
		strcpy_s(RecInfo.Name, Name);
		NetMap.emplace(RecInfo.Name, RecInfo);
		f = NetMap.find(Name);
	}
	return &f->second;
}

void BotQuit(PCHAR Name)
{
	auto f = NetMap.find(Name);
	if (NetMap.end() != f)
		NetMap.erase(f);
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//

bool inGroup(const char* szName)
{
	char szTemp[MAX_STRING] = { 0 };
	if (pLocalPC && pLocalPlayer)
	{
		if (!_stricmp(szName, pLocalPC->Name))
		{
			return true;
		}
		if (pLocalPC->pGroupInfo)
		{
			for (int G = 1; G < MAX_GROUP_SIZE; G++)
			{
				sprintf_s(szTemp, "${Group.Member[%d].Name}", G);
				ParseMacroData(szTemp, MAX_STRING);
				if (!_stricmp(szTemp, szName))
				{
					return true;
				}
			}
		}
	}
	return false;
}

bool inZoned(unsigned long zID, unsigned long iID)
{
	int ZoneID = 0;
	if (pLocalPC)
		ZoneID = pLocalPC->zoneId;
	if (ZoneID > MAX_ZONES)
		ZoneID &= 0x7FFF;
	if (ZoneID <= 0 || ZoneID >= MAX_ZONES)
		return 0;
	return (pLocalPC && ZoneID == zID && pLocalPC->instance == iID);
}

// ***************************************************************************
// Function:    LargerEffectTest
// Description: Return boolean true if the spell effect is to be ignored
//              for stacking purposes
// ***************************************************************************
bool NBLargerEffectTest(PSPELL aSpell, PSPELL bSpell, int i)
{
	int aAttrib = GetSpellNumEffects(aSpell) > i ? GetSpellAttrib(aSpell, i) : SPA_NOSPELL;
	int bAttrib = GetSpellNumEffects(bSpell) > i ? GetSpellAttrib(bSpell, i) : SPA_NOSPELL;
	if (aAttrib == bAttrib
		&& (aAttrib == SPA_HP            // HP +/-: heals/regen/dd
			|| aAttrib == SPA_AC           // Ac Mod
			|| aAttrib == SPA_STONESKIN	// Add Effect: Absorb Damage
			|| aAttrib == 69	// Max HP Mod
			|| aAttrib == 79	// HP Mod
			|| aAttrib == 114	// Aggro Multiplier
			|| aAttrib == 127))	// Spell Haste
		return abs(GetSpellBase(aSpell, i)) >= abs(GetSpellBase(bSpell, i));
	return false;
}

// ***************************************************************************
// Function:    TriggeringEffectSpell
// Description: Return boolean true if the spell effect is to be ignored
//              for stacking purposes
// ***************************************************************************
BOOL NBTriggeringEffectSpell(PSPELL aSpell, int i)
{
	LONG aAttrib = GetSpellNumEffects(aSpell) > i ? GetSpellAttrib(aSpell, i) : SPA_NOSPELL;
	return (aAttrib == 85		// Add Proc
		|| aAttrib == 374 	// Trigger Spell
		|| aAttrib == 419 	// Add Proc
		|| aAttrib == 442 	// Trigger Effect
		|| aAttrib == 443 	// Trigger Effect
		|| aAttrib == 453 	// Trigger Effect
		|| aAttrib == 454 	// Trigger Effect
		|| aAttrib == 475 	// Trigger Spell Non-Item
		|| aAttrib == 481); 	// Trigger Spell
}

// ***************************************************************************
// Function:    DurationWindowTest
// Description: Return boolean true if the spell effect is to be ignored
//              for stacking purposes
// ***************************************************************************
BOOL NBDurationWindowTest(PSPELL aSpell, PSPELL bSpell, int i)
{
	LONG aAttrib = GetSpellNumEffects(aSpell) > i ? GetSpellAttrib(aSpell, i) : SPA_NOSPELL;
	LONG bAttrib = GetSpellNumEffects(bSpell) > i ? GetSpellAttrib(bSpell, i) : SPA_NOSPELL;
	if ((aSpell->SpellType != SpellType_Beneficial && aSpell->SpellType != SpellType_BeneficialGroupOnly)
		|| (bSpell->SpellType != SpellType_Beneficial && bSpell->SpellType != SpellType_BeneficialGroupOnly)
		|| (aSpell->DurationWindow == bSpell->DurationWindow))
		return false;
	return (!(aAttrib == bAttrib &&
		(aAttrib == SPA_ATTACK_POWER            // Attack Mod
			|| aAttrib == SPA_MELEE_GUARD)));   // Mitigate Melee Damage
}

// ***************************************************************************
// Function:    SpellEffectTest
// Description: Return boolean true if the spell effect is to be ignored
//              for stacking purposes
// ***************************************************************************
BOOL NBSpellEffectTest(PSPELL aSpell, PSPELL bSpell, int i, BOOL bIgnoreTriggeringEffects, BOOL bIgnoreCrossDuration)
{
	LONG aAttrib = GetSpellNumEffects(aSpell) > i ? GetSpellAttrib(aSpell, i) : 254;
	LONG bAttrib = GetSpellNumEffects(bSpell) > i ? GetSpellAttrib(bSpell, i) : 254;
	return ((aAttrib == 57 || bAttrib == 57)		// Levitate
		|| (aAttrib == 134 || bAttrib == 134)		// Limit: Max Level
		|| (aAttrib == 135 || bAttrib == 135)		// Limit: Resist
		|| (aAttrib == 136 || bAttrib == 136)		// Limit: Target
		|| (aAttrib == 137 || bAttrib == 137)		// Limit: Effect
		|| (aAttrib == 138 || bAttrib == 138)		// Limit: SpellType
		|| (aAttrib == 139 || bAttrib == 139)		// Limit: Spell
		|| (aAttrib == 140 || bAttrib == 140)		// Limit: Min Duraction
		|| (aAttrib == 141 || bAttrib == 141)		// Limit: Instant
		|| (aAttrib == 142 || bAttrib == 142)		// Limit: Min Level
		|| (aAttrib == 143 || bAttrib == 143)		// Limit: Min Cast Time
		|| (aAttrib == 144 || bAttrib == 144)		// Limit: Max Cast Time
		|| (aAttrib == 254 || bAttrib == 254)		// Placeholder
		|| (aAttrib == 311 || bAttrib == 311)		// Limit: Combat Skills not Allowed
		|| (aAttrib == 339 || bAttrib == 339)		// Trigger DoT on cast
		|| (aAttrib == 340 || bAttrib == 340)		// Trigger DD on cast
		|| (aAttrib == 348 || bAttrib == 348)		// Limit: Min Mana
		|| (aAttrib == 385 || bAttrib == 385)		// Limit: SpellGroup
		|| (aAttrib == 391 || bAttrib == 391)		// Limit: Max Mana
		|| (aAttrib == 403 || bAttrib == 403)		// Limit: SpellClass
		|| (aAttrib == 404 || bAttrib == 404)		// Limit: SpellSubclass
		|| (aAttrib == 411 || bAttrib == 411)		// Limit: PlayerClass
		|| (aAttrib == 412 || bAttrib == 412)		// Limit: Race
		|| (aAttrib == 414 || bAttrib == 414)		// Limit: CastingSkill
		|| (aAttrib == 415 || bAttrib == 415)		// Limit: Item Class
		|| (aAttrib == 420 || bAttrib == 420)		// Limit: Use
		|| (aAttrib == 421 || bAttrib == 421)		// Limit: Use Amt
		|| (aAttrib == 422 || bAttrib == 422)		// Limit: Use Min
		|| (aAttrib == 423 || bAttrib == 423)		// Limit: Use Type
		|| (aAttrib == 428 || bAttrib == 428)		// Limit: Skill
		|| (aAttrib == 442 || bAttrib == 442)		// Trigger Effect
		|| (aAttrib == 443 || bAttrib == 443)		// Trigger Effect
		|| (aAttrib == 453 || bAttrib == 453)		// Trigger Effect
		|| (aAttrib == 454 || bAttrib == 454)		// Trigger Effect
		|| (aAttrib == 460 || bAttrib == 460)		// Limit: Include Non-Focusable
		|| (aAttrib == 475 || bAttrib == 475)		// Trigger Spell Non-Item
		|| (aAttrib == 479 || bAttrib == 479)		// Limit: Value
		|| (aAttrib == 480 || bAttrib == 480)		// Limit: Value
		|| (aAttrib == 481 || bAttrib == 481)		// Trigger Spell
		|| (aAttrib == 485 || bAttrib == 485)		// Limit: Caster Class
		|| (aAttrib == 486 || bAttrib == 486)		// Limit: Caster
		|| (NBLargerEffectTest(aSpell, bSpell, i))	// Ignore if the new effect is greater than the old effect
		|| (bIgnoreTriggeringEffects && (NBTriggeringEffectSpell(aSpell, i) || NBTriggeringEffectSpell(bSpell, i)))		// Ignore triggering effects validation
		|| (bIgnoreCrossDuration && NBDurationWindowTest(aSpell, bSpell, i)));	// Ignore if the effects cross Long/Short Buff windows (with exceptions)
}

// ***************************************************************************
// Function:    BuffStackTest
// Description: Return boolean true if the two spells will stack
// ***************************************************************************
BOOL NBBuffStackTest(PSPELL aSpell, PSPELL bSpell, BOOL bIgnoreTriggeringEffects, BOOL bIgnoreCrossDuration)
{
	if (aSpell->ID == bSpell->ID)
		return true;

	// We need to loop over the largest of the two, this may seem silly but one could have stacking command blocks
	// which we will always need to check.
	LONG effects = max(GetSpellNumEffects(aSpell), GetSpellNumEffects(bSpell));
	for (int i = 0; i < effects; i++)
	{
		//Compare 1st Buff to 2nd. If Attrib[i]==254 its a place holder. If it is 10 it
		//can be 1 of 3 things: PH(Base=0), CHA(Base>0), Lure(Base=-6). If it is Lure or
		//Placeholder, exclude it so slots don't match up. Now Check to see if the slots
		//have equal attribute values. If the do, they don't stack.
		LONG aAttrib = 254, bAttrib = 254; // Default to placeholder ...
		int64_t aBase = 0, bBase = 0, aBase2 = 0, bBase2 = 0;
		if (GetSpellNumEffects(aSpell) > i)
		{
			aAttrib = GetSpellAttrib(aSpell, i);
			aBase = GetSpellBase(aSpell, i);
			aBase2 = GetSpellBase2(aSpell, i);
		}
		if (GetSpellNumEffects(bSpell) > i)
		{
			bAttrib = GetSpellAttrib(bSpell, i);
			bBase = GetSpellBase(bSpell, i);
			bBase2 = GetSpellBase2(bSpell, i);
		}
		//		if (TriggeringEffectSpell(aSpell, i) || TriggeringEffectSpell(bSpell, i)) 
		//		{
		//			if (!BuffStackTest(GetSpellByID(TriggeringEffectSpell(aSpell, i) ? aBase2 : aSpell->ID), GetSpellByID(TriggeringEffectSpell(bSpell, i) ? bBase2 : bSpell->ID), bIgnoreTriggeringEffects))
		//				return false;
		//		}
		if (bAttrib == aAttrib && !NBSpellEffectTest(aSpell, bSpell, i, bIgnoreTriggeringEffects, bIgnoreCrossDuration))
		{
			if (aAttrib == 55 && bAttrib == 55)
			{
				//WriteChatf("Increase Absorb Damage by %d over %d", aBase, bBase);
				return (aBase >= bBase);
			}
			else if (!((bAttrib == 10 && (bBase == -6 || bBase == 0)) ||
				(aAttrib == 10 && (aBase == -6 || aBase == 0)) ||
				(bAttrib == 79 && bBase > 0 && bSpell->TargetType == 6) ||
				(aAttrib == 79 && aBase > 0 && aSpell->TargetType == 6) ||
				(bAttrib == 0 && bBase < 0) ||
				(aAttrib == 0 && aBase < 0) ||
				(bAttrib == 148 || bAttrib == 149) ||
				(aAttrib == 148 || aAttrib == 149)))
			{
				return false;
			}
		}
		//Check to see if second buffs blocks first buff:
		//148: Stacking: Block new spell if slot %d is effect
		//149: Stacking: Overwrite existing spell if slot %d is effect
		if (bAttrib == 148 || bAttrib == 149)
		{
			// in this branch we know bSpell has enough slots
			int tmpSlot = (bAttrib == 148 ? (int)bBase2 - 1 : GetSpellCalc(bSpell, i) - 200 - 1);
			int tmpAttrib = (int)bBase;
			if (GetSpellNumEffects(aSpell) > tmpSlot)  // verify aSpell has that slot
			{
				if (GetSpellMax(bSpell, i) > 0)
				{
					int64_t tmpVal = abs(GetSpellMax(bSpell, i));
					if (GetSpellAttrib(aSpell, tmpSlot) == tmpAttrib && GetSpellBase(aSpell, tmpSlot) < tmpVal)
					{
						return false;
					}
				}
				else if (GetSpellAttrib(aSpell, tmpSlot) == tmpAttrib)
				{
					return false;
				}
			}
		}
		//Now Check to see if the first buff blocks second buff. This is necessary
		//because only some spells carry the Block Slot. Ex. Brells and Spiritual
		//Vigor don't stack Brells has 1 slot total, for HP. Vigor has 4 slots, 2
		//of which block Brells.
		if (aAttrib == 148 || aAttrib == 149)
		{
			// in this branch we know aSpell has enough slots
			int tmpSlot = (aAttrib == 148 ? (int)aBase2 - 1 : GetSpellCalc(aSpell, i) - 200 - 1);
			int tmpAttrib = (int)aBase;
			if (GetSpellNumEffects(bSpell) > tmpSlot)  // verify bSpell has that slot
			{
				if (GetSpellMax(aSpell, i) > 0)
				{
					int64_t tmpVal = abs(GetSpellMax(aSpell, i));
					if (GetSpellAttrib(bSpell, tmpSlot) == tmpAttrib && GetSpellBase(bSpell, tmpSlot) < tmpVal)
					{
						return false;
					}
				}
				else if (GetSpellAttrib(bSpell, tmpSlot) == tmpAttrib)
				{
					return false;
				}
			}
		}
	}
	return true;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//

void InfoSong(const char* Line)
{
	char Buf[MAX_STRING];
	for (long Idx = 0; Idx < NUM_SHORT_BUFFS; Idx++)
	{
		GetArg(Buf, Line, Idx + 1, FALSE, FALSE, FALSE, ':');
		CurBot->Song[Idx] = atol(Buf);
	}
}

void InfoPets(const char* Line)
{
	char Buf[MAX_STRING];
	for (long Idx = 0; Idx < PETS_MAX; Idx++)
	{
		GetArg(Buf, Line, Idx + 1, FALSE, FALSE, FALSE, ':');
		CurBot->Pets[Idx] = atol(Buf);
	}
}

void InfoGems(const char* Line)
{
	char Buf[MAX_STRING];
	if (!bExtended)
		return;
	for (long Idx = 0; Idx < NUM_SPELL_GEMS; Idx++)
	{
		GetArg(Buf, Line, Idx + 1, FALSE, FALSE, FALSE, ':');
		CurBot->Gem[Idx] = atol(Buf);
	}
}

void InfoBuff(const char* Line)
{
	char Buf[MAX_STRING];
	for (long Idx = 0; Idx < NUM_LONG_BUFFS; Idx++)
	{
		GetArg(Buf, Line, Idx + 1, FALSE, FALSE, FALSE, ':');
		CurBot->Buff[Idx] = atol(Buf);
	}
}

void InfoDura(const char* Line)
{
	char Buf[MAX_STRING];
	if (!bExtended)
		return;
	for (long Idx = 0; Idx < NUM_LONG_BUFFS; Idx++)
	{
		//  WriteChatf("We got=%s", Line);
		GetArg(Buf, Line, Idx + 1, FALSE, FALSE, FALSE, ':');
		CurBot->Duration[Idx] = atol(Buf);
		//   WriteChatf("Set Duration to %s", Buf);
		//   WriteChatf("CurBot->Duration is %d", CurBot->Duration[Idx]);
	}
}

void InfoDetr(const char* Line)
{
	char Buf[MAX_STRING];
	for (long Idx = 0; Idx < DSIZE; Idx++)
	{
		GetArg(Buf, Line, Idx + 1, FALSE, FALSE, FALSE, ':');
		CurBot->Detrimental[Idx] = atoi(Buf);
	}
}

void InfoLoc(const char* Line)
{
	char Buf[MAX_STRING];
	strcpy_s(CurBot->Location, Line);
	GetArg(Buf, Line, 1, FALSE, FALSE, FALSE, ':');
	CurBot->Y = (float)atof(Buf);
	GetArg(Buf, Line, 2, FALSE, FALSE, FALSE, ':');
	CurBot->X = (float)atof(Buf);
	GetArg(Buf, Line, 3, FALSE, FALSE, FALSE, ':');
	CurBot->Z = (float)atof(Buf);
}

int SlotCalculate(PSPELL spell, int slot)
{
	char Buf[MAX_STRING] = { 0 };
	SlotValueCalculate(Buf, spell, slot, 1);
	return atoi(Buf);
}

void __stdcall ParseInfo(unsigned int ID, void* pData, PBLECHVALUE pValues)
{
	if (CurBot)
		while (pValues)
		{
			//     WriteChatf("Parsing=%s", pValues->Name);
			switch (atoi(pValues->Name.c_str()))
			{
			case  1: CurBot->ZoneID = atol(pValues->Value.c_str()) & 0x7fff;        break;
			case  2: CurBot->InstID = atol(pValues->Value.c_str());        break;
			case  3: CurBot->SpawnID = atol(pValues->Value.c_str());        break;
			case  4: CurBot->Level = atol(pValues->Value.c_str());        break;
			case  5: CurBot->ClassID = atol(pValues->Value.c_str());        break;
			case  6: CurBot->LifeCur = atol(pValues->Value.c_str());        break;
			case  7: CurBot->LifeMax = atol(pValues->Value.c_str());        break;
			case  8: if (!bExtended) CurBot->EnduCur = 0; else CurBot->EnduCur = atol(pValues->Value.c_str());        break;
			case  9: if (!bExtended) CurBot->EnduMax = 0; else CurBot->EnduMax = atol(pValues->Value.c_str());        break;
			case 10: CurBot->ManaCur = atol(pValues->Value.c_str());        break;
			case 11: CurBot->ManaMax = atol(pValues->Value.c_str());        break;
			case 12: CurBot->PetID = atol(pValues->Value.c_str());        break;
			case 13: CurBot->PetHP = atol(pValues->Value.c_str());        break;
			case 14: CurBot->TargetID = atol(pValues->Value.c_str());        break;
			case 15: CurBot->TargetHP = atol(pValues->Value.c_str());        break;
			case 16: CurBot->CastID = atol(pValues->Value.c_str());        break;
			case 17: CurBot->State = (DWORD)atol(pValues->Value.c_str());  break;
			case 18: CurBot->XP = (__int64)atol(pValues->Value.c_str()); break;
			case 19: CurBot->aaXP = (DWORD)atol(pValues->Value.c_str()); break;
#ifdef EMU
			case 20: CurBot->glXP = atof(pValues->Value.c_str());        break;
#endif
			case 21: CurBot->FreeBuff = atol(pValues->Value.c_str());        break;
			case 22: strcpy_s(CurBot->Leader, pValues->Value.c_str());         break;
			case 30: InfoGems(pValues->Value.c_str());                      break;
			case 31: InfoBuff(pValues->Value.c_str());                      break;
			case 32: InfoSong(pValues->Value.c_str());                      break;
			case 33: InfoPets(pValues->Value.c_str());                      break;
			case 34: InfoDura(pValues->Value.c_str());                     break;
			case 35: CurBot->TotalAA = atol(pValues->Value.c_str());        break;
			case 36: CurBot->UsedAA = atol(pValues->Value.c_str());        break;
			case 37: CurBot->UnusedAA = atol(pValues->Value.c_str());        break;
			case 38: CurBot->CombatState = atol(pValues->Value.c_str());      break;
			case 39: strcpy_s(CurBot->Note, pValues->Value.c_str());	      break;
			case 40: InfoDetr(pValues->Value.c_str());                      break;
			case 89: InfoLoc(pValues->Value.c_str());	  break;
			case 90: strcpy_s(CurBot->Heading, pValues->Value.c_str());		  break;
			case 91: CurBot->MacroState = atoi(pValues->Value.c_str());    break;
			case 92: strcpy_s(CurBot->MacroName, pValues->Value.c_str());  break;
			case 93: CurBot->NavActive = !_stricmp(pValues->Value.c_str(), "TRUE") ? true : false;  break;
			case 94: CurBot->NavPaused = !_stricmp(pValues->Value.c_str(), "TRUE") ? true : false;  break;
			case 95: CurBot->Version = atof(pValues->Value.c_str()); break;
			case 96: CurBot->bBotActive = !_stricmp(pValues->Value.c_str(), "TRUE") ? true : false; break;
			case 97: CurBot->MakeCampStatus = !_stricmp(pValues->Value.c_str(), "ON") ? true : false; break;
			case 98: CurBot->MakeCampX = atof(pValues->Value.c_str());  break;
			case 99: CurBot->MakeCampY = atof(pValues->Value.c_str());  break;
			case 100: CurBot->MakeCampRadius = atof(pValues->Value.c_str());  break;
			case 101: CurBot->MakeCampDistance = atof(pValues->Value.c_str());  break;
			case 102:
			{
				CurBot->EQBC_Packets = 0;
				CurBot->EQBC_HeartBeat = 0;
				char szTemp[MAX_STRING] = { 0 };
				char* token1 = NULL, * next_token1 = NULL;
				bool iSet = false;
				strcpy_s(szTemp, pValues->Value.c_str());
				token1 = strtok_s(szTemp, ",", &next_token1);
				while (token1 != NULL)
				{
					if (token1 != NULL)
					{
						if (strlen(token1))
						{
							ULONGLONG ulValue = strtoull(token1, NULL, 10);
							if (!iSet)
								CurBot->EQBC_Packets = ulValue;
							else
								CurBot->EQBC_HeartBeat = ulValue;
							iSet = true;
						}
						token1 = strtok_s(NULL, ",", &next_token1);
					}
				}
				break;
			}
			case 103:
				CurBot->FreeInventory = atoi(pValues->Value.c_str()); break;
			case 104:
				strcpy_s(CurBot->GroupLeader, pValues->Value.c_str()); break;
			}
			pValues = pValues->pNext;
		}
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//

template <unsigned int SizeT>
char* MakeDURAS(char(&Buffer)[SizeT])
{
	long SpellID, Duration;
	char tmp[MAX_STRING] = { 0 };
	Buffer[0] = 0;
	if (!bExtended)
		return Buffer;
	for (int b = 0; b < NUM_LONG_BUFFS; b++)
	{
		if ((SpellID = GetPcProfile()->GetEffect(b).SpellID) > 0)
		{
			Duration = GetSpellBuffTimer(SpellID);
			sprintf_s(tmp, "%d:", Duration);
			strcat_s(Buffer, tmp);
		}
	}
	return Buffer;
}

template <unsigned int SizeT>
char* MakeBUFFS(char(&Buffer)[SizeT])
{
	long SpellID;
	char tmp[MAX_STRING] = { 0 };
	Buffer[0] = '\0';
	for (int b = 0; b < NUM_LONG_BUFFS; b++)
	{
		if ((SpellID = GetPcProfile()->GetEffect(b).SpellID) > 0)
		{
			sprintf_s(tmp, "%d:", SpellID);
			strcat_s(Buffer, tmp);
		}
	}
	if (strlen(Buffer))
	{
		sprintf_s(tmp, "|F=${Me.FreeBuffSlots}");
		ParseMacroData(tmp, sizeof(tmp));
		strcat_s(Buffer, tmp);
	}
	return Buffer;
}

template <unsigned int SizeT>
char* MakeCASTD(char(&Buffer)[SizeT])
{
	long Casting = pLocalPlayer->CastingData.SpellID;
	if (Casting > 0)
		_itoa_s(Casting, Buffer, 10);
	else
		Buffer[0] = 0;
	return Buffer;
}

template <unsigned int SizeT>
char* MakeHEADN(char(&Buffer)[SizeT])
{
	if (strlen(Buffer))
	{
		sprintf_s(Buffer, "%4.2f", pLocalPlayer->Heading);
	}
	return Buffer;
}

template <unsigned int SizeT>
char* MakeLOCAT(char(&Buffer)[SizeT])
{
	if (strlen(Buffer))
	{
		sprintf_s(Buffer, "%4.2f:%4.2f:%4.2f:%d", pLocalPlayer->Y, pLocalPlayer->X, pLocalPlayer->Z, pLocalPlayer->Animation);
	}
	return Buffer;
}

template <unsigned int SizeT>
char* MakeNVERS(char(&Buffer)[SizeT])
{
	if (strlen(Buffer))
	{
		sprintf_s(Buffer, "%.2f", PLUGIN_VERS);
	}
	return Buffer;
}

template <unsigned int SizeT>
char* MakeENDUS(char(&Buffer)[SizeT])
{
	if (long EnduMax = GetMaxEndurance()) sprintf_s(Buffer, "%d/%d", GetPcProfile()->Endurance, EnduMax);
	else strcpy_s(Buffer, "/");
	return Buffer;
}
#ifndef EMU
template <unsigned int SizeT>
char* MakeEXPER(char(&Buffer)[SizeT])
{
	sprintf_s(Buffer, "%I64d:%d", pLocalPC->Exp, pLocalPC->AAExp);
	return Buffer;
}
#else
template <unsigned int SizeT>
char* MakeEXPER(char(&Buffer)[SizeT])
{
	sprintf_s(Buffer, "%I64d:%d:%02.3f", pLocalPC->Exp, pLocalPC->AAExp, pLocalPC->GroupLeadershipExp);
	return Buffer;
}
#endif

template <unsigned int SizeT>
char* MakeLEADR(char(&Buffer)[SizeT])
{
	if (pLocalPC && pLocalPC->pGroupInfo && pLocalPC->pGroupInfo->GetGroupLeader())
		strcpy_s(Buffer, pLocalPC->pGroupInfo->GetGroupLeader()->GetName());
	else
		Buffer[0] = 0;
	return Buffer;
}

template <unsigned int SizeT>
char* MakeNOTE(char(&Buffer)[SizeT])
{
	strcpy_s(Buffer, NetNote);
	return Buffer;
}

template <unsigned int SizeT>
char* MakeMNAME(char(&Buffer)[SizeT])
{
	strcpy_s(Buffer, gszMacroName);
	return Buffer;
}

template <unsigned int SizeT>
char* MakeMSTATE(char(&Buffer)[SizeT])
{
	WORD Status = MACRO_NONE;
	if (gszMacroName[0])
	{
		Status = MACRO_RUNNING;
		MQMacroBlockPtr pBlock = GetCurrentMacroBlock();
		if (pBlock && pBlock->Paused)
			Status = MACRO_PAUSED;
	}
	_itoa_s(Status, Buffer, 10);
	return Buffer;
}

template <unsigned int _Size>
char* MakeFREEINV(CHAR(&Buffer)[_Size])
{
	sprintf_s(Buffer, "%d", GetFreeInventory(0));
	return Buffer;
}

template <unsigned int SizeT>
char* MakeNAVACT(char(&Buffer)[SizeT])
{
	strcpy_s(Buffer, "0");
	if (GetPlugin("MQ2Nav"))
	{
		strcpy_s(Buffer, "${Navigation.Active}");
		ParseMacroData(Buffer, SizeT);
	}
	return Buffer;
}

template <unsigned int SizeT>
char* MakeNAVPAU(char(&Buffer)[SizeT])
{
	strcpy_s(Buffer, "0");
	if (GetPlugin("MQ2Nav"))
	{
		strcpy_s(Buffer, "${Navigation.Paused}");
		ParseMacroData(Buffer, SizeT);
	}
	return Buffer;
}

template <unsigned int SizeT>
char* MakeBOTACTIVE(char(&Buffer)[SizeT])
{
	strcpy_s(Buffer, "0");
	if (GetPlugin("mq2bot"))
	{
		strcpy_s(Buffer, "${Bot.Active}");
		ParseMacroData(Buffer, SizeT);
	}
	return Buffer;
}

template <unsigned int SizeT>
char* MakeEQBC(char(&Buffer)[SizeT])
{
	char szTemp[MAX_STRING] = { 0 };
	strcpy_s(Buffer, "0,0");
	if (GetPlugin("mq2eqbc"))
	{
		strcpy_s(szTemp, "${EQBC.Packets}");
		ParseMacroData(szTemp, MAX_STRING);
		strcpy_s(Buffer, SizeT, szTemp);
		strcpy_s(szTemp, "${EQBC.HeartBeat}");
		ParseMacroData(szTemp, MAX_STRING);
		strcat_s(Buffer, SizeT, ",");
		strcat_s(Buffer, SizeT, szTemp);
	}
	return Buffer;
}

template <unsigned int SizeT>
char* MakeMAKECAMPSTATUS(char(&Buffer)[SizeT])
{
	strcpy_s(Buffer, "0");
	if (GetPlugin("mq2moveutils"))
	{
		strcpy_s(Buffer, "${MakeCamp.Status}");
		ParseMacroData(Buffer, SizeT);
	}
	return Buffer;
}

template <unsigned int SizeT>
char* MakeMAKECAMPX(char(&Buffer)[SizeT])
{
	strcpy_s(Buffer, "0.00");
	if (GetPlugin("mq2moveutils"))
	{
		strcpy_s(Buffer, "${MakeCamp.AnchorX}");
		ParseMacroData(Buffer, SizeT);
	}
	return Buffer;
}

template <unsigned int SizeT>
char* MakeMAKECAMPY(char(&Buffer)[SizeT])
{
	strcpy_s(Buffer, "0.00");
	if (GetPlugin("mq2moveutils"))
	{
		strcpy_s(Buffer, "${MakeCamp.AnchorY}");
		ParseMacroData(Buffer, SizeT);
	}
	return Buffer;
}

template <unsigned int SizeT>
char* MakeMAKECAMPRADIUS(char(&Buffer)[SizeT])
{
	strcpy_s(Buffer, "0.00");
	if (GetPlugin("mq2moveutils"))
	{
		strcpy_s(Buffer, "${MakeCamp.CampRadius}");
		ParseMacroData(Buffer, SizeT);
	}
	return Buffer;
}

template <unsigned int _Size>
char* MakeMAKECAMPDISTANCE(CHAR(&Buffer)[_Size])
{
	strcpy_s(Buffer, "0.00");
	if (GetPlugin("mq2moveutils"))
	{
		strcpy_s(Buffer, "${MakeCamp.CampDist}");
		ParseMacroData(Buffer, _Size);
	}
	return Buffer;
}

template <unsigned int SizeT>
char* MakeLEVEL(char(&Buffer)[SizeT])
{
	sprintf_s(Buffer, "%d:%d", GetPcProfile()->Level, GetPcProfile()->Class);
	return Buffer;
}

template <unsigned int _Size>
char* MakeGROUPLEADER(CHAR(&Buffer)[_Size])
{
	CHAR glName[MAX_STRING] = { 0 };
	PCHARINFO pChar = GetCharInfo();
	if (!pChar)
	{
		strcpy_s(Buffer, "");
	}
	else if (!pChar->pGroupInfo)
	{
		strcpy_s(Buffer, "");
	}
	else if (!pChar->pGroupInfo->pLeader)
	{
		strcpy_s(Buffer, "");
	}
	else
	{
		strcpy_s(glName, pChar->pGroupInfo->pLeader->Name.c_str());
		sprintf_s(Buffer, "%s", glName);
	}
	return Buffer;
}
template <unsigned int SizeT
>char* MakeLIFES(char(&Buffer)[SizeT])
{
	sprintf_s(Buffer, "%d/%d", GetCurHPS(), GetMaxHPS());
	return Buffer;
}

template <unsigned int SizeT>
char* MakeMANAS(char(&Buffer)[SizeT])
{
	if (long ManaMax = GetMaxMana()) sprintf_s(Buffer, "%d/%d", GetPcProfile()->Mana, ManaMax);
	else strcpy_s(Buffer, "/");
	return Buffer;
}

template <unsigned int SizeT>
char* MakePBUFF(char(&Buffer)[SizeT])
{
	long SpellID; char tmp[MAX_STRING]; Buffer[0] = 0;
	PSPAWNINFO Pet = GetSpawnByID(pLocalPlayer->PetID);
	for (int b = 0; b < PETS_MAX; b++)
		if ((SpellID = (Pet && pPetInfoWnd) ? pPetInfoWnd->Buff[b] : 0) > 0)
		{
			sprintf_s(tmp, "%d:", SpellID);
			strcat_s(Buffer, tmp);
		}
	//  WriteChatf("MakePBUFF: [%s]", Buffer);
	return Buffer;
}

template <unsigned int SizeT>
char* MakePETIL(char(&Buffer)[SizeT])
{
	PSPAWNINFO Pet = GetSpawnByID(pLocalPlayer->PetID);
#if defined (ROF2EMU)
	if (pPetInfoWnd && Pet) sprintf_s(Buffer, "%d:%d", Pet->SpawnID, (Pet->HPCurrent * 100 / Pet->HPMax));
#else
	if (pPetInfoWnd && Pet) sprintf_s(Buffer, "%d:%I64d", Pet->SpawnID, (Pet->HPCurrent * 100 / Pet->HPMax));
#endif
	else strcpy_s(Buffer, ":");
	return Buffer;
}

template <unsigned int SizeT>
char* MakeSPGEM(char(&Buffer)[SizeT])
{
	long SpellID; char tmp[MAX_STRING]; Buffer[0] = 0;
	if (!bExtended)
		return Buffer;
	for (int g = 0; g < NUM_SPELL_GEMS; g++)
		if ((SpellID = (pCastSpellWnd && GetMaxMana() > 0) ? GetPcProfile()->MemorizedSpells[g] : 0) > 0)
		{
			sprintf_s(tmp, "%d:", SpellID);
			strcat_s(Buffer, tmp);
		}
	return Buffer;
}

template <unsigned int SizeT>
char* MakeSONGS(char(&Buffer)[SizeT])
{
	long SpellID = 0; char tmp[MAX_STRING] = { 0 }; Buffer[0] = '\0';
	for (int b = 0; b < NUM_SHORT_BUFFS; b++)
		if ((SpellID = GetPcProfile()->GetTempEffect(b).SpellID) > 0)
		{
			sprintf_s(tmp, "%d:", SpellID);
			strcat_s(Buffer, tmp);
		}
	return Buffer;
}

// FIX: Note STATE_INVIS returns has different behavior. Lets you tell if you are invis or itu or both
template <unsigned int SizeT>
char* MakeSTATE(char(&Buffer)[SizeT])
{
	DWORD Status = 0;

	int ITU = Evaluate("${If[${Me.SPA[28]}||${Me.SPA[315]},1,0]}");
	int AM = FindMQ2DataType("Melee") ? Evaluate("${If[${Melee.AggroMode},1,0]}") : 0;

	if ((pLocalPlayer->PlayerState & 0x20) && (pLocalPlayer->StandState != STANDSTATE_DEAD))
	{
		Status |= STATE_STUN;
	}
	else if (pLocalPlayer->RespawnTimer)
	{
		Status |= STATE_HOVER;
	}
	else if (pLocalPlayer->Mount)
	{
		Status |= STATE_MOUNT;
	}
	else
	{
		switch (pLocalPlayer->StandState)
		{
		case STANDSTATE_STAND:
			Status |= STATE_STAND;
			break;
		case STANDSTATE_SIT:
			Status |= STATE_SIT;
			break;
		case STANDSTATE_DUCK:
			Status |= STATE_DUCK;
			break;
		case STANDSTATE_BIND:
			Status |= STATE_BIND;
			break;
		case STANDSTATE_FEIGN:
			Status |= STATE_FEIGN;
			break;
		case STANDSTATE_DEAD:
			Status |= STATE_DEAD;
			break;
		default:
			break;
		}
	}

	if (pEverQuestInfo->bAutoAttack)                  Status |= STATE_ATTACK;
	if (pRaid && pRaid->RaidMemberCount)              Status |= STATE_RAID;
	if (pLocalPC->pGroupInfo)                         Status |= STATE_GROUP;
	if (FindSpeed(pLocalPlayer))                      Status |= STATE_MOVING;
	if (pLocalPlayer->AFK)                            Status |= STATE_AFK;
	if (pLocalPlayer->HideMode & 0x01)                Status |= STATE_INVIS;
	if (pLocalPlayer->mPlayerPhysicsClient.Levitate)  Status |= STATE_LEV;
	if (pLocalPlayer->LFG)                            Status |= STATE_LFG;
	if (ITU)                                          Status |= STATE_ITU;
	if (pEverQuestInfo->bAutoRangeAttack)             Status |= STATE_RANGED;
	if (AM)                                           Status |= STATE_WANTAGGRO;

	if (pTarget && pLocalPlayer->TargetOfTarget == pLocalPlayer->GetId())
	{
		if (GetSpawnType(pTarget) == NPC)             Status |= STATE_HAVEAGGRO;
	}

	_itoa_s(Status, Buffer, 10);
	return Buffer;
}

template <unsigned int SizeT>
char* MakeOOCST(char(&Buffer)[SizeT])
{
	_itoa_s(pPlayerWnd->CombatState, Buffer, 10);
	return Buffer;
}

template <unsigned int SizeT>
char* MakeAAPTS(char(&Buffer)[SizeT])
{
	sprintf_s(Buffer, "%d:%d:%d", GetPcProfile()->AAPoints + GetPcProfile()->AAPointsSpent, GetPcProfile()->AAPointsSpent, GetPcProfile()->AAPoints);
	return Buffer;
}

template <unsigned int SizeT>
char* MakeTARGT(char(&Buffer)[SizeT])
{
#if defined (ROF2EMU)
	if (pTarget) sprintf_s(Buffer, "%d:%d", pTarget->SpawnID, (pTarget->HPCurrent * 100 / pTarget->HPMax));
#else
	if (pTarget) sprintf_s(Buffer, "%d:%I64d", pTarget->SpawnID, (pTarget->HPCurrent * 100 / pTarget->HPMax));
#endif
	else strcpy_s(Buffer, ":");
	return Buffer;
}

template <unsigned int SizeT>
char* MakeZONES(char(&Buffer)[SizeT])
{
	sprintf_s(Buffer, "%d:%d>%d", pLocalPC->zoneId, pLocalPC->instance, pLocalPlayer->SpawnID);
	return Buffer;
}

template <unsigned int SizeT>
char* MakeDETR(char(&Buffer)[SizeT])
{
	char tmp[MAX_STRING]; Buffer[0] = 0;
	ZeroMemory(&dValues, sizeof(dValues));
	for (int b = 0; b < NUM_LONG_BUFFS; b++)
	{
		if (PSPELL spell = GetSpellByID(GetPcProfile()->GetEffect(b).SpellID))
		{
			if (!spell->SpellType)
			{
				bool d = false;
				bool r = false;
				for (int s = 0; s < GetSpellNumEffects(spell); s++)
					switch (GetSpellAttrib(spell, s))
					{
					case SPA_HP:
						if (GetSpellBase(spell, s) < 0)
						{
							dValues[LIFEDRAIN] += SlotCalculate(spell, s);
							d = true;
						}
						break;
					case SPA_MOVEMENT_RATE:
						if (GetSpellBase(spell, s) < 0)
						{
							dValues[SNARED]++;
							d = true;
						}
						break;
					case SPA_HASTE:
						if (((GetSpellMax(spell, s)) ? GetSpellMax(spell, s) : GetSpellBase(spell, s)) - 100 < 0)
						{
							dValues[SLOWED]++;
							d = true;
						}
						break;
					case SPA_MANA:
						if (GetSpellBase(spell, s) < 0)
						{
							dValues[MANADRAIN] += SlotCalculate(spell, s); d = true;
						}
						break;
					case SPA_BLINDNESS:
						dValues[BLINDED]++;
						d = true;
						break;
					case SPA_CHARM:
						dValues[CHARMED]++;
						d = true;
						break;
					case SPA_FEAR:
						dValues[FEARED]++;
						d = true;
						break;
					case SPA_ENTHRALL:
						dValues[MESMERIZED]++;
						d = true;
						break;
					case SPA_DISEASE:
						dValues[DISEASED] += (int)GetSpellBase(spell, s);
						d = true;
						break;
					case SPA_POISON:
						dValues[POISONED] += (int)GetSpellBase(spell, s);
						d = true;
						break;
					case SPA_INVULNERABILITY:
						dValues[INVULNERABLE]++;
						d = true;
						break;
					case SPA_RESIST_FIRE:
						if (GetSpellBase(spell, s) < 0)
						{
							r = true;
							d = true;
						}
						break;
					case SPA_RESIST_COLD:
						if (GetSpellBase(spell, s) < 0)
						{
							r = true;
							d = true;
						}
						break;
					case SPA_RESIST_POISON:
						if (GetSpellBase(spell, s) < 0)
						{
							r = true;
							d = true;
						}
						break;
					case SPA_RESIST_DISEASE:
						if (GetSpellBase(spell, s) < 0)
						{
							r = true;
							d = true;
						}
						break;
					case SPA_RESIST_MAGIC:
						if (GetSpellBase(spell, s) < 0)
						{
							r = true;
							d = true;
						}
						break;
					case SPA_SILENCE:
						dValues[SILENCED]++;
						d = true;
						break;
					case SPA_ROOT:
						dValues[ROOTED]++;
						d = true;
						break;
					case SPA_FIZZLE_SKILL:
						if (GetSpellBase(spell, s) < 0)
						{
							dValues[CASTINGLEVEL]++;
							d = true;
						}
						break;
					case SPA_CURSE:
						dValues[CURSED] += (int)GetSpellBase(spell, s);
						d = true;
						break;
					case SPA_HEALMOD:
						if (GetSpellBase(spell, s) < 0)
						{
							dValues[HEALING]++;
							d = true;
						}
						break;
					case SPA_FOCUS_DAMAGE_MOD:
						if (GetSpellBase(spell, s) < 0)
						{
							dValues[SPELLDAMAGE]++;
							d = true;
						}
						break;
					case SPA_FOCUS_CAST_TIME_MOD:
						if (GetSpellBase(spell, s) < 0)
						{
							dValues[SPELLSLOWED]++;
							d = true;
						}
						break;
					case SPA_FOCUS_MANACOST_MOD:
						if (GetSpellBase(spell, s) < 0)
						{
							dValues[SPELLCOST]++;
							d = true;
						}
						break;
					case SPA_ENDURANCE:
						if (GetSpellBase(spell, s) < 0)
						{
							dValues[ENDUDRAIN] += SlotCalculate(spell, s);
							d = true;
						}
						break;
					case SPA_DOOM_EFFECT:
						dValues[TRIGGR]++;
						d = true;
						break;
					case SPA_CORRUPTION:
						dValues[CORRUPTED] += (int)GetSpellBase(spell, s);
						d = true;
						break;
					}
				if (d)
				{
					dValues[DETRIMENTALS]++;
					switch (spell->ID)
					{
					case 45473:  /* Putrid Infection is curable, but missing WearOff message */
						break;
					default:
						/*CastByMe,CastByOther,CastOnYou,CastOnAnother,WearOff*/
						const char* Wearoff = GetSpellString(spell->ID, 4);
						if ((spell->NoDispell && !Wearoff) || spell->TargetType == 6)
							dValues[NOCURE]++;
						break;
					}
				}
				if (r)
					dValues[RESISTANCE]++;
			}
		}
	}
	dValues[COUNTERS] = dValues[CURSED] + dValues[DISEASED] + dValues[POISONED] + dValues[CORRUPTED];
	for (int a = 0; a < DSIZE; a++)
	{
		sprintf_s(tmp, "%d:", dValues[a]);
		strcat_s(Buffer, tmp);
	}
	return Buffer;
}

void BroadCast()
{
	char Buffer[MAX_STRING];
	long nChange = false;
	long nUpdate = false;
	ZeroMemory(wBuffer, sizeof(wBuffer));
	ZeroMemory(wChange, sizeof(wChange));
	ZeroMemory(wUpdate, sizeof(wUpdate));
	sprintf_s(wBuffer[BUFFS], "B=%s|", MakeBUFFS(Buffer));
	sprintf_s(wBuffer[CASTD], "C=%s|", MakeCASTD(Buffer));
	if (!bExtended)
		sprintf_s(wBuffer[ENDUS], "E=0|");
	else
		sprintf_s(wBuffer[ENDUS], "E=%s|", MakeENDUS(Buffer));
	sprintf_s(wBuffer[EXPER], "X=%s|", MakeEXPER(Buffer));
	sprintf_s(wBuffer[LEADR], "N=%s|", MakeLEADR(Buffer));
	sprintf_s(wBuffer[LEVEL], "L=%s|", MakeLEVEL(Buffer));
	sprintf_s(wBuffer[LIFES], "H=%s|", MakeLIFES(Buffer));
	sprintf_s(wBuffer[MANAS], "M=%s|", MakeMANAS(Buffer));
	sprintf_s(wBuffer[PBUFF], "W=%s|", MakePBUFF(Buffer));
	sprintf_s(wBuffer[PETIL], "P=%s|", MakePETIL(Buffer));
	if (!bExtended)
		sprintf_s(wBuffer[SPGEM], "G=0|");
	else
		sprintf_s(wBuffer[SPGEM], "G=%s|", MakeSPGEM(Buffer));
	sprintf_s(wBuffer[SONGS], "S=%s|", MakeSONGS(Buffer));
	sprintf_s(wBuffer[STATE], "Y=%s|", MakeSTATE(Buffer));
	sprintf_s(wBuffer[TARGT], "T=%s|", MakeTARGT(Buffer));
	sprintf_s(wBuffer[ZONES], "Z=%s|", MakeZONES(Buffer));
	if (!bExtended)
		sprintf_s(wBuffer[DURAS], "D=0|");
	else
		sprintf_s(wBuffer[DURAS], "D=%s|", MakeDURAS(Buffer));
	sprintf_s(wBuffer[AAPTS], "A=%s|", MakeAAPTS(Buffer));
	sprintf_s(wBuffer[OOCST], "O=%s|", MakeOOCST(Buffer));
	sprintf_s(wBuffer[NOTE], "U=%s|", MakeNOTE(Buffer));
	sprintf_s(wBuffer[DETR], "R=%s|", MakeDETR(Buffer));
	sprintf_s(wBuffer[LOCAT], "@=%s|", MakeLOCAT(Buffer));
	sprintf_s(wBuffer[HEADN], "$=%s|", MakeHEADN(Buffer));
	sprintf_s(wBuffer[MSTATE], "&=%s|", MakeMSTATE(Buffer));
	sprintf_s(wBuffer[MNAME], "^=%s|", MakeMNAME(Buffer));
	sprintf_s(wBuffer[NAVACT], "<=%s|", MakeNAVACT(Buffer));
	sprintf_s(wBuffer[NAVPAU], ">=%s|", MakeNAVPAU(Buffer));
	sprintf_s(wBuffer[NVERS], "V=%s|", MakeNVERS(Buffer));
	sprintf_s(wBuffer[BOTACT], "+=%s|", MakeBOTACTIVE(Buffer));
	sprintf_s(wBuffer[CAMPSTATUS], "`=%s|", MakeMAKECAMPSTATUS(Buffer));
	sprintf_s(wBuffer[CAMPX], "~=%s|", MakeMAKECAMPX(Buffer));
	sprintf_s(wBuffer[CAMPY], "_=%s|", MakeMAKECAMPY(Buffer));
	sprintf_s(wBuffer[CAMPRADIUS], ";=%s|", MakeMAKECAMPRADIUS(Buffer));
	sprintf_s(wBuffer[CAMPDISTANCE], ":=%s|", MakeMAKECAMPDISTANCE(Buffer));
	sprintf_s(wBuffer[EQBC], "J=%s|", MakeEQBC(Buffer));
	sprintf_s(wBuffer[FREEINV], "I=%s|", MakeFREEINV(Buffer));
	sprintf_s(wBuffer[GROUPLEADER], "-=%s|", MakeGROUPLEADER(Buffer));
	//  WriteChatf("D=%s|", Buffer);
	for (int i = 0; i < ESIZE; i++)
		if ((clock() > sTimers[i] && clock() > sTimers[i] + UPDATES) || 0 != strcmp(wBuffer[i], sBuffer[i]))
		{
			wChange[i] = true;
			nChange++;
		}
		else if (clock() < sTimers[i] && clock() + UPDATES > sTimers[i])
		{
			wUpdate[i] = true;
			nUpdate++;
		}
	if (nChange)
	{
		strcpy_s(Buffer, "[NB]|");
		for (int i = 0; i < ESIZE; i++)
			if (wChange[i] || wUpdate[i] && (strlen(Buffer) + strlen(wBuffer[i])) < MAX_STRING - 5)
			{
				strcat_s(Buffer, wBuffer[i]);
				sTimers[i] = (long)clock() + REFRESH;
			}
		strcat_s(Buffer, "[NB]");
		//   WriteChatf("Broadcast %s", Buffer);
		EQBCBroadCast(Buffer);
		memcpy(sBuffer, wBuffer, sizeof(wBuffer));
		if (CurBot = BotLoad(pLocalPC->Name))
		{
			Packet.Feed(Buffer);
			CurBot->Updated = clock();
			CurBot = 0;
		}
	}
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//

class MQ2NetBotsType* pNetBotsType = 0;
class MQ2NetBotsType : public MQ2Type
{
private:
	map<string, BotInfo, PredIgnoreCase>::iterator l;
	BotInfo* BotRec = 0;
	char Temps[MAX_STRING];
	char Works[MAX_STRING];
	long Cpt;
public:
	enum Information
	{
		Enable = 1,
		Listen = 2,
		Output = 3,
		Counts = 5,
		Client = 6,
		Name = 7,
		Zone = 8,
		Instance = 9,
		ID = 10,
		Class = 11,
		Level = 12,
		PctExp = 13,
		PctAAExp = 14,
#ifdef EMU
		PctGroupLeaderExp = 15,
#endif
		CurrentHPs = 16,
		MaxHPs = 17,
		PctHPs = 18,
		CurrentEndurance = 19,
		MaxEndurance = 20,
		PctEndurance = 21,
		CurrentMana = 22,
		MaxMana = 23,
		PctMana = 24,
		PetID = 25,
		PetHP = 26,
		TargetID = 27,
		TargetHP = 28,
		Casting = 29,
		State = 30,
		Attacking = 31,
		AFK = 32,
		Binding = 33,
		Ducking = 34,
		Invis = 35,
		Feigning = 36,
		Grouped = 37,
		Levitating = 38,
		LFG = 39,
		Mounted = 40,
		Moving = 41,
		Raid = 42,
		Sitting = 43,
		Standing = 44,
		Stunned = 45,
		Gem = 46,
		Buff = 47,
		ShortBuff = 48,
		PetBuff = 49,
		FreeBuffSlots = 50,
		InZone = 51,
		InGroup = 52,
		Leader = 53,
		Updated = 54,
		Duration = 55,
		TotalAA = 56,
		UsedAA = 57,
		UnusedAA = 58,
		CombatState = 59,
		Stacks = 60,
		Note = 61,
		Detrimentals = 62,
		Counters = 63,
		Cursed = 64,
		Diseased = 65,
		Poisoned = 66,
		Corrupted = 67,
		EnduDrain = 68,
		LifeDrain = 69,
		ManaDrain = 70,
		Blinded = 71,
		CastingLevel = 72,
		Charmed = 73,
		Feared = 74,
		Healing = 75,
		Invulnerable = 76,
		Mesmerized = 77,
		Rooted = 78,
		Silenced = 79,
		Slowed = 80,
		Snared = 81,
		SpellCost = 82,
		SpellSlowed = 83,
		SpellDamage = 84,
		Trigger = 85,
		Resistance = 86,
		Detrimental = 87,
		NoCure = 88,
		Location = 89,
		Heading = 90,
		StacksPet = 91,
		MacroState = 92,
		MacroName = 93,
		NavigationActive = 94,
		NavigationPaused = 95,
		Version = 96,
		BotActive = 97,
		MakeCampStatus = 98,
		MakeCampX = 99,
		MakeCampY = 100,
		MakeCampRadius = 101,
		MakeCampDistance = 102,
		InvisToUndead = 103,
		AutoFire = 104,
		WantAggro = 105,
		HaveAggro = 106,
		X = 107,
		Y = 108,
		Z = 109,
		Distance = 110,
		Dead = 111,
		Hover = 112,
		Packets = 113,
		HeartBeat = 114,
		FreeInventory = 115,
		GroupLeader = 116,
	};

	MQ2NetBotsType() :MQ2Type("NetBots")
	{
		TypeMember(Enable);
		TypeMember(Listen);
		TypeMember(Output);
		TypeMember(Counts);
		TypeMember(Client);
		TypeMember(Name);
		TypeMember(Zone);
		TypeMember(Instance);
		TypeMember(ID);
		TypeMember(Class);
		TypeMember(Level);
		TypeMember(PctExp);
		TypeMember(PctAAExp);
#ifdef EMU
		TypeMember(PctGroupLeaderExp);
#endif
		TypeMember(CurrentHPs);
		TypeMember(MaxHPs);
		TypeMember(PctHPs);
		TypeMember(CurrentEndurance);
		TypeMember(MaxEndurance);
		TypeMember(PctEndurance);
		TypeMember(CurrentMana);
		TypeMember(MaxMana);
		TypeMember(PctMana);
		TypeMember(PetID);
		TypeMember(PetHP);
		TypeMember(TargetID);
		TypeMember(TargetHP);
		TypeMember(Casting);
		TypeMember(State);
		TypeMember(Attacking);
		TypeMember(AFK);
		TypeMember(Binding);
		TypeMember(Ducking);
		TypeMember(Invis);
		TypeMember(Feigning);
		TypeMember(Grouped);
		TypeMember(Levitating);
		TypeMember(LFG);
		TypeMember(Mounted);
		TypeMember(Moving);
		TypeMember(Raid);
		TypeMember(Sitting);
		TypeMember(Standing);
		TypeMember(Stunned);
		TypeMember(Gem);
		TypeMember(Buff);
		TypeMember(ShortBuff);
		TypeMember(PetBuff);
		TypeMember(FreeBuffSlots);
		TypeMember(InZone);
		TypeMember(InGroup);
		TypeMember(Leader);
		TypeMember(Updated);
		TypeMember(Duration);
		TypeMember(TotalAA);
		TypeMember(UsedAA);
		TypeMember(UnusedAA);
		TypeMember(CombatState);
		TypeMember(Stacks);
		TypeMember(Note);
		TypeMember(Detrimentals);
		TypeMember(Counters);
		TypeMember(Cursed);
		TypeMember(Diseased);
		TypeMember(Poisoned);
		TypeMember(Corrupted);
		TypeMember(EnduDrain);
		TypeMember(LifeDrain);
		TypeMember(ManaDrain);
		TypeMember(Blinded);
		TypeMember(CastingLevel);
		TypeMember(Charmed);
		TypeMember(Feared);
		TypeMember(Healing);
		TypeMember(Invulnerable);
		TypeMember(Mesmerized);
		TypeMember(Rooted);
		TypeMember(Silenced);
		TypeMember(Slowed);
		TypeMember(Snared);
		TypeMember(SpellCost);
		TypeMember(SpellSlowed);
		TypeMember(SpellDamage);
		TypeMember(Trigger);
		TypeMember(Resistance);
		TypeMember(Detrimental);
		TypeMember(NoCure);
		TypeMember(StacksPet);
		TypeMember(Location);
		TypeMember(Heading);
		TypeMember(MacroState);
		TypeMember(MacroName);
		TypeMember(NavigationActive);
		TypeMember(NavigationPaused);
		TypeMember(Version);
		TypeMember(BotActive);
		TypeMember(MakeCampStatus);
		TypeMember(MakeCampX);
		TypeMember(MakeCampY);
		TypeMember(MakeCampRadius);
		TypeMember(MakeCampDistance);
		TypeMember(InvisToUndead);
		TypeMember(AutoFire);
		TypeMember(WantAggro);
		TypeMember(HaveAggro);
		TypeMember(X);
		TypeMember(Y);
		TypeMember(Z);
		TypeMember(Distance);
		TypeMember(Dead);
		TypeMember(Hover);
		TypeMember(Packets);
		TypeMember(HeartBeat);
		TypeMember(FreeInventory);
		TypeMember(GroupLeader);
	}

	void Search(const char* Index)
	{
		if (!Index || Index && Index[0] == '\0')
		{
			BotRec = 0;
		}
		else if (!_stricmp(Index, "NULL"))
		{
			BotRec = 0;
		}
		else if (!BotRec || (BotRec && Index && _stricmp(BotRec->Name, Index)))
		{
			BotRec = BotFind(Index);
		}
	}

	virtual bool GetMember(MQVarPtr VarPtr, const char* Member, char* Index, MQTypeVar& Dest) override
	{
		using namespace mq::datatypes;

		if (MQTypeMember* pMember = MQ2NetBotsType::FindMember(Member))
		{
			switch ((Information)pMember->ID)
			{
			case Enable:
				Dest.Type = pBoolType;
				Dest.DWord = NetStat;
				return true;
			case Listen:
				Dest.Type = pBoolType;
				Dest.DWord = NetGrab;
				return true;
			case Output:
				Dest.Type = pBoolType;
				Dest.DWord = NetSend;
				return true;
			case Counts:
				Cpt = 0;
				if (NetStat && NetGrab)
					for (l = NetMap.begin(); l != NetMap.end(); l++)
					{
						BotRec = &(*l).second;
						if (BotRec->SpawnID == 0) continue;
						Cpt++;
					}
				Dest.Type = pIntType;
				Dest.Int = Cpt;
				return true;
			case Client:
				Cpt = 0; Temps[0] = 0;
				if (NetStat && NetGrab)
					for (l = NetMap.begin(); l != NetMap.end(); l++)
					{
						BotRec = &(*l).second;
						if (BotRec->SpawnID == 0)
							continue;
						if (Cpt++)
							strcat_s(Temps, " ");
						strcat_s(Temps, BotRec->Name);
					}
				if (IsNumber(Index))
				{
					int n = atoi(Index);
					if (n<0 || n>Cpt)
						break;
					strcpy_s(Temps, GetArg(Works, Temps, n));
				}
				Dest.Type = pStringType;
				Dest.Ptr = Temps;
				return true;
			}
			if (BotRec)
			{
				switch ((Information)pMember->ID)
				{
				case Name:
					Dest.Type = pStringType;
					Dest.Ptr = Temps;
					strcpy_s(Temps, BotRec->Name);
					return true;
				case Zone:
					Dest.Type = pIntType;
					Dest.DWord = BotRec->ZoneID;
					return true;
				case Instance:
					Dest.Type = pIntType;
					Dest.DWord = BotRec->InstID;
					return true;
				case ID:
					Dest.Type = pIntType;
					Dest.DWord = BotRec->SpawnID;
					return true;
				case Class:
					Dest.Type = pClassType;
					Dest.DWord = BotRec->ClassID;
					return true;
				case Level:
					Dest.Type = pIntType;
					Dest.DWord = BotRec->Level;
					return true;
				case PctExp:
					Dest.Type = pFloatType;
#if defined (ROF2EMU)
					Dest.Float = (float)BotRec->XP * 100.0f / 330.0f;
#else
					Dest.Float = (float)BotRec->XP / 1000.0f;
#endif
					return true;
				case PctAAExp:
					Dest.Type = pFloatType;
#if defined (ROF2EMU)
					Dest.Float = (float)BotRec->aaXP * 100.0f / 330.0f;
#else
					Dest.Float = (float)BotRec->aaXP / 1000.0f;
#endif
					return true;
#ifdef EMU
				case PctGroupLeaderExp:
					Dest.Type = pFloatType;
					Dest.Float = (float)(BotRec->glXP / 10.0f);
					return true;
#endif
				case CurrentHPs:
					Dest.Type = pIntType;
					Dest.Int = BotRec->LifeCur;
					return true;
				case MaxHPs:
					Dest.Type = pIntType;
					Dest.Int = BotRec->LifeMax;
					return true;
				case PctHPs:
					Dest.Type = pIntType;
					Dest.Int = (BotRec->LifeMax < 1 || BotRec->LifeCur < 1) ? 0 : BotRec->LifeCur * 100 / BotRec->LifeMax;
					return true;
				case CurrentEndurance:
					Dest.Type = pIntType;
					if (!bExtended)
						Dest.Int = 0;
					else
						Dest.Int = BotRec->EnduCur;
					return true;
				case MaxEndurance:
					Dest.Type = pIntType;
					if (!bExtended)
						Dest.Int = 0;
					else
						Dest.Int = BotRec->EnduMax;
					return true;
				case PctEndurance:
					Dest.Type = pIntType;
					if (!bExtended)
						Dest.Int = 0;
					else
						Dest.Int = (BotRec->EnduMax < 1 || BotRec->EnduCur < 1) ? 0 : BotRec->EnduCur * 100 / BotRec->EnduMax;
					return true;
				case CurrentMana:
					Dest.Type = pIntType;
					Dest.Int = BotRec->ManaCur;
					return true;
				case MaxMana:
					Dest.Type = pIntType;
					Dest.Int = BotRec->ManaMax;
					return true;
				case PctMana:
					Dest.Type = pIntType;
					Dest.Int = (BotRec->ManaMax < 1 || BotRec->ManaCur < 1) ? 0 : BotRec->ManaCur * 100 / BotRec->ManaMax;
					return true;
				case PetID:
					Dest.Type = pIntType;
					Dest.DWord = BotRec->PetID;
					return true;
				case PetHP:
					Dest.Type = pIntType;
					Dest.Int = BotRec->PetHP;
					return true;
				case TargetID:
					Dest.Type = pIntType;
					Dest.DWord = BotRec->TargetID;
					return true;
				case TargetHP:
					Dest.Type = pIntType;
					Dest.Int = BotRec->TargetHP;
					return true;
				case Casting:
					if (BotRec->CastID)
					{
						Dest.Type = pSpellType;
						Dest.Ptr = GetSpellByID(BotRec->CastID);
						return true;
					}
					break;
				case State:
					Dest.Type = pStringType;
					Dest.Ptr = Temps;
					if (BotRec->State & STATE_DEAD)
						strcpy_s(Temps, "DEAD");
					else if (BotRec->State & STATE_HOVER)
						strcpy_s(Temps, "HOVER");
					else if (BotRec->State & STATE_STUN)
						strcpy_s(Temps, "STUN");
					else if (BotRec->State & STATE_MOUNT)
						strcpy_s(Temps, "MOUNT");
					else if (BotRec->State & STATE_STAND)
						strcpy_s(Temps, "STAND");
					else if (BotRec->State & STATE_SIT)
						strcpy_s(Temps, "SIT");
					else if (BotRec->State & STATE_DUCK)
						strcpy_s(Temps, "DUCK");
					else if (BotRec->State & STATE_BIND)
						strcpy_s(Temps, "BIND");
					else if (BotRec->State & STATE_FEIGN)
						strcpy_s(Temps, "FEIGN");
					else
						strcpy_s(Temps, "UNKNOWN");
					return true;
				case Attacking:
					Dest.Type = pBoolType;
					Dest.DWord = BotRec->State & STATE_ATTACK;
					return true;
				case AFK:
					Dest.Type = pBoolType;
					Dest.DWord = BotRec->State & STATE_AFK;
					return true;
				case Binding:
					Dest.Type = pBoolType;
					Dest.DWord = BotRec->State & STATE_BIND;
					return true;
				case Ducking:
					Dest.Type = pBoolType;
					Dest.DWord = BotRec->State & STATE_DUCK;
					return true;
				case Feigning:
					Dest.Type = pBoolType;
					Dest.DWord = BotRec->State & STATE_FEIGN;
					return true;
				case Grouped:
					Dest.Type = pBoolType;
					Dest.DWord = BotRec->State & STATE_GROUP;
					return true;
				case Invis:
					Dest.Type = pBoolType;
					Dest.DWord = BotRec->State & STATE_INVIS;
					return true;
				case Levitating:
					Dest.Type = pBoolType;
					Dest.DWord = BotRec->State & STATE_LEV;
					return true;
				case LFG:
					Dest.Type = pBoolType;
					Dest.DWord = BotRec->State & STATE_LFG;
					return true;
				case Mounted:
					Dest.Type = pBoolType;
					Dest.DWord = BotRec->State & STATE_MOUNT;
					return true;
				case Moving:
					Dest.Type = pBoolType;
					Dest.DWord = BotRec->State & STATE_MOVING;
					return true;
				case Raid:
					Dest.Type = pBoolType;
					Dest.DWord = BotRec->State & STATE_RAID;
					return true;
				case Sitting:
					Dest.Type = pBoolType;
					Dest.DWord = BotRec->State & STATE_SIT;
					return true;
				case Standing:
					Dest.Type = pBoolType;
					Dest.DWord = BotRec->State & STATE_STAND;
					return true;
				case Stunned:
					Dest.Type = pBoolType;
					Dest.DWord = BotRec->State & STATE_STUN;
					return true;
				case Dead:
					Dest.Type = pBoolType;
					Dest.DWord = BotRec->State & STATE_DEAD;
					return true;
				case Hover:
					Dest.Type = pBoolType;
					Dest.DWord = BotRec->State & STATE_HOVER;
					return true;
				case FreeBuffSlots:
					Dest.Type = pIntType;
					Dest.Int = BotRec->FreeBuff;
					return true;
				case InZone:
					Dest.Type = pBoolType;
					Dest.DWord = (inZoned(BotRec->ZoneID, BotRec->InstID));
					return true;
				case InGroup:
					Dest.Type = pBoolType;
					Dest.DWord = inGroup(BotRec->Name);
					return true;
				case Leader:
					Dest.Type = pStringType;
					Dest.Ptr = Temps;
					strcpy_s(Temps, BotRec->Leader);
					return true;
				case Note:
					Dest.Type = pStringType;
					Dest.Ptr = Temps;
					strcpy_s(Temps, BotRec->Note);
					return true;
				case Location:
					Dest.Type = pStringType;
					Dest.Ptr = Temps;
					strcpy_s(Temps, BotRec->Location);
					return true;
				case Heading:
					Dest.Type = pStringType;
					Dest.Ptr = Temps;
					strcpy_s(Temps, BotRec->Heading);
					return true;
				case Updated:
					Dest.Type = pIntType;
					Dest.Int = clock() - BotRec->Updated;
					return true;
				case Gem:
					if (bExtended)
					{
						if (!Index[0])
						{
							Temps[0] = 0;
							for (Cpt = 0; Cpt < NUM_SPELL_GEMS; Cpt++)
							{
								sprintf_s(Works, "%d ", BotRec->Gem[Cpt]);
								strcat_s(Temps, Works);
							}
							Dest.Ptr = Temps;
							Dest.Type = pStringType;
							return true;
						}
						Cpt = atoi(Index);
						if (Cpt<NUM_SPELL_GEMS && Cpt>-1)
							if (Dest.Ptr = GetSpellByID(BotRec->Gem[Cpt]))
							{
								Dest.Type = pSpellType;
								return true;
							}
					}
					return false;
				case Buff:
					if (!Index[0])
					{
						Temps[0] = 0;
						for (Cpt = 0; Cpt < NUM_LONG_BUFFS && BotRec->Buff[Cpt]; Cpt++)
						{
							sprintf_s(Works, "%d ", BotRec->Buff[Cpt]);
							strcat_s(Temps, Works);
						}
						Dest.Ptr = Temps;
						Dest.Type = pStringType;
						return true;
					}
					if (IsNumber(Index))
					{
						Cpt = atoi(Index);
						if (Cpt<NUM_LONG_BUFFS && Cpt>-1)
						{
							if (Dest.Ptr = GetSpellByID(BotRec->Buff[Cpt]))
							{
								Dest.Type = pSpellType;
								return true;
							}
						}
					}
					else
					{
						for (Cpt = 0; Cpt < NUM_LONG_BUFFS; Cpt++)
						{
							if (BotRec->Buff[Cpt])
							{
								PSPELL pSpell = GetSpellByID(BotRec->Buff[Cpt]);
								if (pSpell)
								{
									if (bUseSimpleSearch)
									{
										if (ci_find_substr(pSpell->Name, Index) != -1)
										{
											Dest.Ptr = pSpell;
											Dest.Type = pSpellType;
											return true;
										}
									}
									else
									{
										if (!_strnicmp(Index, pSpell->Name, strlen(Index)))
										{
											Dest.Ptr = pSpell;
											Dest.Type = pSpellType;
											return true;
										}
									}
								}
							}
						}
					}
					return false;
				case Duration:
					if (bExtended)
					{
						if (!Index[0])
						{
							Temps[0] = 0;
							for (Cpt = 0; Cpt < NUM_LONG_BUFFS && BotRec->Duration[Cpt]; Cpt++)
							{
								sprintf_s(Works, "%d ", BotRec->Duration[Cpt]);
								strcat_s(Temps, Works);
							}
							Dest.Ptr = Temps;
							Dest.Type = pStringType;
							return true;
						}
						else
						{
							if (IsNumber(Index))
							{
								Cpt = atoi(Index);
								if (Cpt<NUM_LONG_BUFFS && Cpt>-1)
								{
									//   WriteChatf("Duration: %d", BotRec->Duration[Cpt]);
									if (Dest.UInt64 = BotRec->Duration[Cpt])
									{
										Dest.Type = pTimeStampType;
										return true;
									}
								}
							}
							else
							{
								for (Cpt = 0; Cpt < NUM_LONG_BUFFS; Cpt++)
								{
									if (BotRec->Buff[Cpt])
									{
										PSPELL pSpell = GetSpellByID(BotRec->Buff[Cpt]);
										if (pSpell)
										{
											if (bUseSimpleSearch)
											{
												if (ci_find_substr(pSpell->Name, Index) != -1)
												{
													Dest.UInt64 = BotRec->Duration[Cpt];
													Dest.Type = pTimeStampType;
													return true;
												}
											}
											else
											{
												if (!_strnicmp(Index, pSpell->Name, strlen(Index)))
												{
													Dest.UInt64 = BotRec->Duration[Cpt];
													Dest.Type = pTimeStampType;
													return true;
												}
											}
										}
									}
								}
							}
						}
					}
					return false;
				case ShortBuff:
					if (!Index[0])
					{
						Temps[0] = 0;
						for (Cpt = 0; Cpt < NUM_SHORT_BUFFS && BotRec->Song[Cpt]; Cpt++)
						{
							sprintf_s(Works, "%d ", BotRec->Song[Cpt]);
							strcat_s(Temps, Works);
						}
						Dest.Ptr = Temps;
						Dest.Type = pStringType;
						return true;
					}
					else
					{
						if (IsNumber(Index))
						{
							Cpt = atoi(Index);
							if (Cpt<NUM_SHORT_BUFFS && Cpt>-1)
							{
								if (Dest.Ptr = GetSpellByID(BotRec->Song[Cpt]))
								{
									Dest.Type = pSpellType;
									return true;
								}
							}
						}
						else
						{
							for (Cpt = 0; Cpt < NUM_SHORT_BUFFS; Cpt++)
							{
								if (BotRec->Song[Cpt])
								{
									PSPELL pSpell = GetSpellByID(BotRec->Song[Cpt]);
									if (pSpell)
									{
										if (bUseSimpleSearch)
										{
											if (ci_find_substr(pSpell->Name, Index) != -1)
											{
												Dest.Ptr = pSpell;
												Dest.Type = pSpellType;
												return true;
											}
										}
										else
										{
											if (!_strnicmp(Index, pSpell->Name, strlen(Index)))
											{
												Dest.Ptr = pSpell;
												Dest.Type = pSpellType;
												return true;
											}
										}
									}
								}
							}
						}
					}
					return false;
				case PetBuff:
					if (!Index[0])
					{
						Temps[0] = 0;
						for (Cpt = 0; Cpt < PETS_MAX && BotRec->Pets[Cpt]; Cpt++)
						{
							sprintf_s(Works, "%d ", BotRec->Pets[Cpt]);
							strcat_s(Temps, Works);
						}
						Dest.Ptr = Temps;
						Dest.Type = pStringType;
						return true;
					}
					else
					{
						if (IsNumber(Index))
						{
							Cpt = atoi(Index);
							if (Cpt<PETS_MAX && Cpt>-1)
							{
								if (Dest.Ptr = GetSpellByID(BotRec->Pets[Cpt]))
								{
									Dest.Type = pSpellType;
									return true;
								}
							}
						}
						else
						{
							for (Cpt = 0; Cpt < PETS_MAX; Cpt++)
							{
								if (BotRec->Pets[Cpt])
								{
									PSPELL pSpell = GetSpellByID(BotRec->Pets[Cpt]);
									if (pSpell)
									{
										if (bUseSimpleSearch)
										{
											if (ci_find_substr(pSpell->Name, Index) != -1)
											{
												Dest.Ptr = pSpell;
												Dest.Type = pSpellType;
												return true;
											}
										}
										else
										{
											if (!_strnicmp(Index, pSpell->Name, strlen(Index)))
											{
												Dest.Ptr = pSpell;
												Dest.Type = pSpellType;
												return true;
											}
										}
									}
								}
							}
						}
					}
					return false;
				case TotalAA:
					Dest.Type = pIntType;
					Dest.DWord = BotRec->TotalAA;
					return true;
				case UsedAA:
					Dest.Type = pIntType;
					Dest.DWord = BotRec->UsedAA;
					return true;
				case UnusedAA:
					Dest.Type = pIntType;
					Dest.DWord = BotRec->UnusedAA;
					return true;
				case CombatState:
					Dest.Type = pIntType;
					Dest.DWord = BotRec->CombatState;
					return true;
				case Stacks:
				{
					Dest.Type = pBoolType;
					Dest.DWord = false;
					if (!Index[0])
						return true;
					PSPELL tmpSpell = NULL;
					if (IsNumber(Index))
						tmpSpell = GetSpellByID(atoi(Index));
					else
						tmpSpell = GetSpellByName(Index);
					if (!tmpSpell)
						return true;
					Dest.DWord = true;
					// Check Buffs
					for (Cpt = 0; Cpt < NUM_LONG_BUFFS; Cpt++)
					{
						if (BotRec->Buff[Cpt])
						{
							if (PSPELL buffSpell = GetSpellByID(BotRec->Buff[Cpt]))
							{
								if (!NBBuffStackTest(tmpSpell, buffSpell, TRUE, TRUE) || (buffSpell == tmpSpell))
								{
									Dest.DWord = false;
									return true;
								}
							}
						}
					}
					// Check Songs
					for (Cpt = 0; Cpt < NUM_SHORT_BUFFS; Cpt++)
					{
						if (BotRec->Song[Cpt])
						{
							if (PSPELL buffSpell = GetSpellByID(BotRec->Song[Cpt]))
							{
								if (!IsBardSong(buffSpell) && !((IsSPAEffect(tmpSpell, SPA_CHANGE_FORM) && !tmpSpell->DurationWindow)))
								{
									if (!NBBuffStackTest(tmpSpell, buffSpell, TRUE, TRUE) || (buffSpell == tmpSpell))
									{
										Dest.DWord = false;
										return true;
									}
								}
							}
						}
					}
					return true;
				}
				case StacksPet:
				{
					Dest.Type = pBoolType;
					Dest.DWord = false;
					if (!Index[0])
						return true;
					PSPELL tmpSpell = NULL;
					if (IsNumber(Index))
						tmpSpell = GetSpellByID(atoi(Index));
					else
						tmpSpell = GetSpellByName(Index);
					if (!tmpSpell)
						return true;
					Dest.DWord = true;
					// Check Pet Buffs
					for (Cpt = 0; Cpt < PETS_MAX; Cpt++)
					{
						if (BotRec->Pets[Cpt])
						{
							if (PSPELL buffSpell = GetSpellByID(BotRec->Pets[Cpt]))
							{
								if (!NBBuffStackTest(tmpSpell, buffSpell, TRUE, FALSE) || (buffSpell == tmpSpell))
								{
									Dest.DWord = false;
									return true;
								}
							}
						}
					}
					return true;
				}
				case Detrimentals:
					Dest.Type = pIntType;
					Dest.Int = BotRec->Detrimental[DETRIMENTALS];
					return true;
				case Counters:
					Dest.Type = pIntType;
					Dest.Int = BotRec->Detrimental[COUNTERS];
					return true;
				case Cursed:
					Dest.Type = pIntType;
					Dest.Int = BotRec->Detrimental[CURSED];
					return true;
				case Diseased:
					Dest.Type = pIntType;
					Dest.Int = BotRec->Detrimental[DISEASED];
					return true;
				case Poisoned:
					Dest.Type = pIntType;
					Dest.Int = BotRec->Detrimental[POISONED];
					return true;
				case Corrupted:
					Dest.Type = pIntType;
					Dest.Int = BotRec->Detrimental[CORRUPTED];
					return true;
				case EnduDrain:
					Dest.Type = pIntType;
					Dest.Int = BotRec->Detrimental[ENDUDRAIN];
					return true;
				case LifeDrain:
					Dest.Type = pIntType;
					Dest.Int = BotRec->Detrimental[LIFEDRAIN];
					return true;
				case ManaDrain:
					Dest.Type = pIntType;
					Dest.Int = BotRec->Detrimental[MANADRAIN];
					return true;
				case Blinded:
					Dest.Type = pIntType;
					Dest.Int = BotRec->Detrimental[BLINDED];
					return true;
				case CastingLevel:
					Dest.Type = pIntType;
					Dest.Int = BotRec->Detrimental[CASTINGLEVEL];
					return true;
				case Charmed:
					Dest.Type = pIntType;
					Dest.Int = BotRec->Detrimental[CHARMED];
					return true;
				case Feared:
					Dest.Type = pIntType;
					Dest.Int = BotRec->Detrimental[FEARED];
					return true;
				case Healing:
					Dest.Type = pIntType;
					Dest.Int = BotRec->Detrimental[HEALING];
					return true;
				case Invulnerable:
					Dest.Type = pIntType;
					Dest.Int = BotRec->Detrimental[INVULNERABLE];
					return true;
				case Mesmerized:
					Dest.Type = pIntType;
					Dest.Int = BotRec->Detrimental[MESMERIZED];
					return true;
				case Rooted:
					Dest.Type = pIntType;
					Dest.Int = BotRec->Detrimental[ROOTED];
					return true;
				case Silenced:
					Dest.Type = pIntType;
					Dest.Int = BotRec->Detrimental[SILENCED];
					return true;
				case Slowed:
					Dest.Type = pIntType;
					Dest.Int = BotRec->Detrimental[SLOWED];
					return true;
				case Snared:
					Dest.Type = pIntType;
					Dest.Int = BotRec->Detrimental[SNARED];
					return true;
				case SpellCost:
					Dest.Type = pIntType;
					Dest.Int = BotRec->Detrimental[SPELLCOST];
					return true;
				case SpellSlowed:
					Dest.Type = pIntType;
					Dest.Int = BotRec->Detrimental[SPELLSLOWED];
					return true;
				case SpellDamage:
					Dest.Type = pIntType;
					Dest.Int = BotRec->Detrimental[SPELLDAMAGE];
					return true;
				case Trigger:
					Dest.Type = pIntType;
					Dest.Int = BotRec->Detrimental[TRIGGR];
					return true;
				case Resistance:
					Dest.Type = pIntType;
					Dest.Int = BotRec->Detrimental[RESISTANCE];
					return true;
				case Detrimental:
					Temps[0] = 0;
					if (BotRec->Detrimental[CURSED])
						strcat_s(Temps, "Cursed ");
					if (BotRec->Detrimental[DISEASED])
						strcat_s(Temps, "Diseased ");
					if (BotRec->Detrimental[POISONED])
						strcat_s(Temps, "Poisoned ");
					if (BotRec->Detrimental[ENDUDRAIN])
						strcat_s(Temps, "EnduDrain ");
					if (BotRec->Detrimental[LIFEDRAIN])
						strcat_s(Temps, "LifeDrain ");
					if (BotRec->Detrimental[MANADRAIN])
						strcat_s(Temps, "ManaDrain ");
					if (BotRec->Detrimental[BLINDED])
						strcat_s(Temps, "Blinded ");
					if (BotRec->Detrimental[CASTINGLEVEL])
						strcat_s(Temps, "CastingLevel ");
					if (BotRec->Detrimental[CHARMED])
						strcat_s(Temps, "Charmed ");
					if (BotRec->Detrimental[FEARED])
						strcat_s(Temps, "Feared ");
					if (BotRec->Detrimental[HEALING])
						strcat_s(Temps, "Healing ");
					if (BotRec->Detrimental[INVULNERABLE])
						strcat_s(Temps, "Invulnerable ");
					if (BotRec->Detrimental[MESMERIZED])
						strcat_s(Temps, "Mesmerized ");
					if (BotRec->Detrimental[ROOTED])
						strcat_s(Temps, "Rooted ");
					if (BotRec->Detrimental[SILENCED])
						strcat_s(Temps, "Silenced ");
					if (BotRec->Detrimental[SLOWED])
						strcat_s(Temps, "Slowed ");
					if (BotRec->Detrimental[SNARED])
						strcat_s(Temps, "Snared ");
					if (BotRec->Detrimental[SPELLCOST])
						strcat_s(Temps, "SpellCost ");
					if (BotRec->Detrimental[SPELLDAMAGE])
						strcat_s(Temps, "SpellDamage ");
					if (BotRec->Detrimental[SPELLSLOWED])
						strcat_s(Temps, "SpellSlowed ");
					if (BotRec->Detrimental[TRIGGR])
						strcat_s(Temps, "Trigger ");
					if (BotRec->Detrimental[CORRUPTED])
						strcat_s(Temps, "Corrupted ");
					if (BotRec->Detrimental[RESISTANCE])
						strcat_s(Temps, "Resistance ");
					if (size_t len = strlen(Temps))
						Temps[--len] = 0;
					Dest.Type = pStringType;
					Dest.Ptr = Temps;
					return true;
				case NoCure:
					Dest.Type = pIntType;
					Dest.Int = BotRec->Detrimental[NOCURE];
					return true;
				case MacroState:
					Dest.Type = pIntType;
					Dest.Int = BotRec->MacroState;
					return true;
				case MacroName:
					strcpy_s(Temps, BotRec->MacroName);
					Dest.Type = pStringType;
					Dest.Ptr = Temps;
					return true;
				case NavigationActive:
					Dest.Type = pBoolType;
					Dest.DWord = BotRec->NavActive;
					return true;
				case NavigationPaused:
					Dest.Type = pBoolType;
					Dest.DWord = BotRec->NavPaused;
					return true;
				case Version:
					Dest.Type = pDoubleType;
					Dest.Double = BotRec->Version;
					return true;
				case BotActive:
					Dest.Type = pBoolType;
					Dest.DWord = BotRec->bBotActive;
					return true;
				case MakeCampStatus:
					Dest.Type = pBoolType;
					Dest.DWord = BotRec->MakeCampStatus;
					return true;
				case MakeCampX:
					Dest.Type = pDoubleType;
					Dest.Double = BotRec->MakeCampX;
					return true;
				case MakeCampY:
					Dest.Type = pDoubleType;
					Dest.Double = BotRec->MakeCampY;
					return true;
				case MakeCampRadius:
					Dest.Type = pDoubleType;
					Dest.Double = BotRec->MakeCampRadius;
					return true;
				case MakeCampDistance:
					Dest.Type = pDoubleType;
					Dest.Double = BotRec->MakeCampDistance;
					return true;
				case InvisToUndead:
					Dest.Type = pBoolType;
					Dest.DWord = BotRec->State & STATE_ITU;
					return true;
				case AutoFire:
					Dest.Type = pBoolType;
					Dest.DWord = BotRec->State & STATE_RANGED;
					return true;
				case WantAggro:
					Dest.Type = pBoolType;
					Dest.DWord = BotRec->State & STATE_WANTAGGRO;
					return true;
				case HaveAggro:
					Dest.Type = pBoolType;
					Dest.DWord = BotRec->State & STATE_HAVEAGGRO;
					return true;
				case X:
					Dest.Type = pFloatType;
					Dest.Float = BotRec->X;
					return true;
				case Y:
					Dest.Type = pFloatType;
					Dest.Float = BotRec->Y;
					return true;
				case Z:
					Dest.Type = pFloatType;
					Dest.Float = BotRec->Z;
					return true;
				case Distance:
				{
					float DX = pLocalPlayer->X - BotRec->X;
					float DY = pLocalPlayer->Y - BotRec->Y;
					float DZ = pLocalPlayer->Z - BotRec->Z;
					Dest.Type = pFloatType;
					Dest.Float = sqrt(DX * DX + DY * DY + DZ * DZ);
					return true;
				}
				case Packets:
					Dest.Type = pInt64Type;
					Dest.UInt64 = BotRec->EQBC_Packets;
					return true;
				case HeartBeat:
					Dest.Type = pInt64Type;
					Dest.UInt64 = BotRec->EQBC_HeartBeat;
					return true;
				case FreeInventory:
					Dest.Type = pIntType;
					Dest.Int = BotRec->FreeInventory;
					return true;
				case GroupLeader:
					strcpy_s(Temps, BotRec->GroupLeader);
					Dest.Type = pStringType;
					Dest.Ptr = Temps;
					return true;
				}
			}
		}
		return false;
	}

	bool ToString(MQVarPtr VarPtr, char* Destination) override
	{
		strcpy_s(Destination, MAX_STRING, "TRUE");
		return true;
	}
};

bool dataNetBots(const char* Index, MQTypeVar& Dest)
{
	Dest.DWord = 1;
	Dest.Type = pNetBotsType;
	pNetBotsType->Search(Index);
	return true;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//

void Command(PSPAWNINFO pChar, PCHAR Cmd)
{
	char Tmp[MAX_STRING]; BYTE Parm = 1;
	char Var[MAX_STRING];
	char Set[MAX_STRING];
	do
	{
		GetArg(Tmp, Cmd, Parm++);
		GetArg(Var, Tmp, 1, FALSE, FALSE, FALSE, '=');
		GetArg(Set, Tmp, 2, FALSE, FALSE, FALSE, '=');
		if (_strnicmp(Var, "show", 4) == 0)
		{
			WritePrivateProfileString(PLUGIN_NAME, "Show", "1", INIFileName);
			NetShow = 1;
			ShowMyWindow();
			return;
		}
		if (_strnicmp(Var, "hide", 4) == 0)
		{
			WritePrivateProfileString(PLUGIN_NAME, "Show", "0", INIFileName);
			NetShow = 0;
			HideMyWindow();
			return;
		}

		if (!_strnicmp(Tmp, "on", 2) || !_strnicmp(Tmp, "off", 3))
		{
			NetStat = _strnicmp(Tmp, "off", 3);
			WritePrivateProfileString(PLUGIN_TITLE, "Stat", NetStat ? "1" : "0", INIFileName);
		}
		else if (!_strnicmp(Var, "stat", 4) || !_strnicmp(Var, "plugin", 6))
		{
			NetStat = _strnicmp(Set, "off", 3);
			WritePrivateProfileString(PLUGIN_TITLE, "Stat", NetStat ? "1" : "0", INIFileName);

		}
		else if (!_strnicmp(Var, "grab", 4))
		{
			NetGrab = _strnicmp(Set, "off", 3);
			WritePrivateProfileString(PLUGIN_TITLE, "Grab", NetGrab ? "1" : "0", INIFileName);
		}
		else if (!_strnicmp(Var, "send", 4))
		{
			NetSend = _strnicmp(Set, "off", 3);
			WritePrivateProfileString(PLUGIN_TITLE, "Send", NetSend ? "1" : "0", INIFileName);
		}
		else if (!_strnicmp(Var, "ext", 3))
		{
			bExtended = _strnicmp(Set, "off", 3) ? true : false;
			WritePrivateProfileString(PLUGIN_TITLE, "Extended", bExtended ? "1" : "0", INIFileName);
		}
		else if (!_strnicmp(Var, "simple", 6))
		{
			bUseSimpleSearch = _strnicmp(Set, "off", 3) ? true : false;
			WritePrivateProfileString(PLUGIN_TITLE, "UseSimpleSearch", bUseSimpleSearch ? "1" : "0", INIFileName);
		}
	} while (strlen(Tmp));
	WriteChatf("%s:: (%s) Grab (%s) Send (%s) Ext (%s) Simple (%s)    Version=\at%.2f", PLUGIN_TITLE, NetStat ? "\agon\ax" : "\aroff\ax", NetGrab ? "\agon\ax" : "\aroff\ax", NetSend ? "\agon\ax" : "\aroff\ax", bExtended ? "\agon\ax" : "\aroff\ax", bUseSimpleSearch ? "\agon\ax" : "\aroff\ax", PLUGIN_VERS);
}

void CommandNote(PSPAWNINFO pChar, PCHAR Cmd)
{
	strcpy_s(NetNote, Cmd);
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//

PLUGIN_API void OnBeginZone()
{
#if    DEBUGGING>0
	DebugSpewAlways("%s->OnBeginZone()", PLUGIN_TITLE);
#endif DEBUGGING
	ZeroMemory(sTimers, sizeof(sTimers));
	if (NetStat && NetSend && EQBCConnected())
		EQBCBroadCast("[NB]|Z=:>|[NB]");
}

PLUGIN_API void OnNetBotEVENT(PCHAR Msg)
{
#if    DEBUGGING>0
	DebugSpewAlways("%s->OnNetBotEVENT(%s)", PLUGIN_TITLE, Msg);
#endif DEBUGGING
	if (!strncmp(Msg, "NBQUIT=", 7))
		BotQuit(&Msg[7]);
	else if (!strncmp(Msg, "NBJOIN=", 7))
		ZeroMemory(sTimers, sizeof(sTimers));
	else if (!strncmp(Msg, "NBEXIT", 6))
		NetMap.clear();
}

PLUGIN_API void OnNetBotMSG(PCHAR Name, PCHAR Msg)
{
	if (NetStat && NetGrab && !strncmp(Msg, "[NB]|", 5) &&
		pLocalPC && pLocalPlayer && _stricmp(pLocalPC->Name, Name))
	{
#if    DEBUGGING>1
		DebugSpewAlways("%s->OnNetBotMSG(From:[%s] Msg[%s])", PLUGIN_TITLE, Name, Msg);
#endif DEBUGGING
		char szCmd[MAX_STRING] = { 0 };
		strcpy_s(szCmd, Msg);
		if (CurBot = BotLoad(Name))
		{
			Packet.Feed(szCmd);
			CurBot->Updated = clock();
			CurBot = 0;
		}
	}
}

PLUGIN_API void OnPulse()
{
	if (NetStat && NetSend && gbInZone && (long)clock() > NetLast)
	{
		NetLast = (long)clock() + NETTICK;
		if (EQBCConnected() && pLocalPC && pLocalPlayer && GetPcProfile())
			BroadCast();
	}
	if (NetShow && NetStat && NetSend && MQGetTickCount64() > NetShow)
		WindowUpdate();
}

PLUGIN_API void SetGameState(DWORD GameState)
{
#if    DEBUGGING>0
	DebugSpewAlways("%s->SetGameState(%d)", PLUGIN_TITLE, GameState);
#endif DEBUGGING
	if (GameState == GAMESTATE_INGAME)
	{
		if (!NetInit)
		{
#if    DEBUGGING>0
			DebugSpewAlways("%s->SetGameState(%d)->Loading", PLUGIN_TITLE, GameState);
#endif DEBUGGING
			sprintf_s(INIFileName, "%s\\%s_%s.ini", gPathConfig, GetServerShortName(), pLocalPC->Name);
			NetStat = GetPrivateProfileInt(PLUGIN_TITLE, "Stat", 0, INIFileName);
			NetGrab = GetPrivateProfileInt(PLUGIN_TITLE, "Grab", 0, INIFileName);
			NetSend = GetPrivateProfileInt(PLUGIN_TITLE, "Send", 0, INIFileName);
			bExtended = 0 != GetPrivateProfileInt(PLUGIN_TITLE, "Extended", 0, INIFileName);
			bUseSimpleSearch = 0 != GetPrivateProfileInt(PLUGIN_TITLE, "UseSimpleSearch", 0, INIFileName);
			NetInit = true;
			NetShow = GetPrivateProfileInt(PLUGIN_NAME, "Show", 0, INIFileName);
			if (NetShow)
				ShowMyWindow();
		}
	}
	else if (GameState != GAMESTATE_LOGGINGIN)
	{
		if (NetInit)
		{
#if    DEBUGGING>0
			DebugSpewAlways("%s->SetGameState(%d)->Flushing", PLUGIN_TITLE, GameState);
#endif DEBUGGING
			NetStat = false;
			NetGrab = false;
			NetSend = false;
			NetInit = false;
			NetShow = false;
		}
	}
}

void BCZ(PSPAWNINFO pChar, PCHAR Cmd)
{
	char szCmd[MAX_STRING] = { 0 };
	if (!Cmd || !Cmd[0])
		return;
	if (!pLocalPC)
		return;
	if (!pLocalPlayer)
		return;
	if (!EQBCConnected())
		return;
	if (NetStat && NetGrab)
	{
		for (auto l = NetMap.begin(); l != NetMap.end(); l++)
		{
			BotInfo* BotRec = &(*l).second;
			if (BotRec->SpawnID == 0)
				continue;
			if (BotRec->SpawnID == pLocalPlayer->SpawnID)
				continue;
			unsigned long botZID, myZID;
			botZID = BotRec->ZoneID;
			if (botZID > 0x7FFF)
				botZID &= 0x7FFF;
			myZID = pLocalPC->zoneId;
			if (myZID > 0x7FFF)
				myZID &= 0x7FFF;
			if (myZID != botZID)
				continue;
			if (pLocalPC->instance != BotRec->InstID)
				continue;
			sprintf_s(szCmd, "/bct %s %s", BotRec->Name, Cmd);
			HideDoCommand(pLocalPlayer, szCmd, false);
		}
	}
}

void BCZA(PSPAWNINFO pChar, PCHAR Cmd)
{
	char szCmd[MAX_STRING] = { 0 };
	if (!Cmd || !Cmd[0])
		return;
	if (!pLocalPC)
		return;
	if (!pLocalPlayer)
		return;
	if (!EQBCConnected())
		return;
	if (NetStat && NetGrab)
	{
		for (auto l = NetMap.begin(); l != NetMap.end(); l++)
		{
			BotInfo* BotRec = &(*l).second;
			if (BotRec->SpawnID == 0)
				continue;
			unsigned long botZID, myZID;
			botZID = BotRec->ZoneID;
			if (botZID > 0x7FFF)
				botZID &= 0x7FFF;
			myZID = pLocalPC->zoneId;
			if (myZID > 0x7FFF)
				myZID &= 0x7FFF;
			if (myZID != botZID)
				continue;
			if (pLocalPC->instance != BotRec->InstID)
				continue;
			sprintf_s(szCmd, "/bct %s %s", BotRec->Name, Cmd);
			HideDoCommand(pLocalPlayer, szCmd, false);
		}
	}
}

void BCGZ(PSPAWNINFO pChar, PCHAR Cmd)
{
	char szCmd[MAX_STRING] = { 0 };
	if (!Cmd || !Cmd[0])
		return;
	if (!pLocalPC)
		return;
	if (!pLocalPlayer)
		return;
	if (!EQBCConnected())
		return;
	if (NetStat && NetGrab)
	{
		for (auto l = NetMap.begin(); l != NetMap.end(); l++)
		{
			BotInfo* BotRec = &(*l).second;
			if (BotRec->SpawnID == 0)
				continue;
			if (BotRec->SpawnID == pLocalPlayer->SpawnID)
				continue;
			unsigned long botZID, myZID;
			botZID = BotRec->ZoneID;
			if (botZID > 0x7FFF)
				botZID &= 0x7FFF;
			myZID = pLocalPC->zoneId;
			if (myZID > 0x7FFF)
				myZID &= 0x7FFF;
			if (myZID != botZID)
				continue;
			if (pLocalPC->instance != BotRec->InstID)
				continue;
			if (!inGroup(BotRec->Name))
				continue;
			sprintf_s(szCmd, "/bct %s %s", BotRec->Name, Cmd);
			HideDoCommand(pLocalPlayer, szCmd, false);
		}
	}
}

void BCGZA(PSPAWNINFO pChar, PCHAR Cmd)
{
	char szCmd[MAX_STRING] = { 0 };
	if (!Cmd || !Cmd[0])
		return;
	if (!pLocalPC)
		return;
	if (!pLocalPlayer)
		return;
	if (!EQBCConnected())
		return;
	if (NetStat && NetGrab)
	{
		for (auto l = NetMap.begin(); l != NetMap.end(); l++)
		{
			BotInfo* BotRec = &(*l).second;
			if (BotRec->SpawnID == 0)
				continue;
			unsigned long botZID, myZID;
			botZID = BotRec->ZoneID;
			if (botZID > 0x7FFF)
				botZID &= 0x7FFF;
			myZID = pLocalPC->zoneId;
			if (myZID > 0x7FFF)
				myZID &= 0x7FFF;
			if (myZID != botZID)
				continue;
			if (pLocalPC->instance != BotRec->InstID)
				continue;
			if (!inGroup(BotRec->Name))
				continue;
			sprintf_s(szCmd, "/bct %s %s", BotRec->Name, Cmd);
			HideDoCommand(pLocalPlayer, szCmd, false);
		}
	}
}

PLUGIN_API void InitializePlugin()
{
	Packet.Reset();
	NetMap.clear();
	Packet.AddEvent("#*#[NB]#*#|Z=#1#:#2#>#3#|#*#[NB]", ParseInfo, (void*)3);
	Packet.AddEvent("#*#[NB]#*#|L=#4#:#5#|#*#[NB]", ParseInfo, (void*)5);
	Packet.AddEvent("#*#[NB]#*#|H=#6#/#7#|#*#[NB]", ParseInfo, (void*)7);
	Packet.AddEvent("#*#[NB]#*#|E=#8#/#9#|#*#[NB]", ParseInfo, (void*)9);
	Packet.AddEvent("#*#[NB]#*#|M=#10#/#11#|#*#[NB]", ParseInfo, (void*)11);
	Packet.AddEvent("#*#[NB]#*#|P=#12#:#13#|#*#[NB]", ParseInfo, (void*)13);
	Packet.AddEvent("#*#[NB]#*#|T=#14#:#15#|#*#[NB]", ParseInfo, (void*)15);
	Packet.AddEvent("#*#[NB]#*#|C=#16#|#*#[NB]", ParseInfo, (void*)16);
	Packet.AddEvent("#*#[NB]#*#|Y=#17#|#*#[NB]", ParseInfo, (void*)17);
#ifdef EMU
	Packet.AddEvent("#*#[NB]#*#|X=#18#:#19#:#20#|#*#[NB]", ParseInfo, (void*)20);
#else
	Packet.AddEvent("#*#[NB]#*#|X=#18#:#19#|#*#[NB]", ParseInfo, (void*)19);
#endif
	Packet.AddEvent("#*#[NB]#*#|F=#21#:|#*#[NB]", ParseInfo, (void*)21);
	Packet.AddEvent("#*#[NB]#*#|N=#22#|#*#[NB]", ParseInfo, (void*)22);
	Packet.AddEvent("#*#[NB]#*#|G=#30#|#*#[NB]", ParseInfo, (void*)30);
	Packet.AddEvent("#*#[NB]#*#|B=#31#|#*#[NB]", ParseInfo, (void*)31);
	Packet.AddEvent("#*#[NB]#*#|S=#32#|#*#[NB]", ParseInfo, (void*)32);
	Packet.AddEvent("#*#[NB]#*#|W=#33#|#*#[NB]", ParseInfo, (void*)33);
	Packet.AddEvent("#*#[NB]#*#|D=#34#|#*#[NB]", ParseInfo, (void*)34);
	Packet.AddEvent("#*#[NB]#*#|A=#35#:#36#:#37#|#*#[NB]", ParseInfo, (void*)37);
	Packet.AddEvent("#*#[NB]#*#|O=#38#|#*#[NB]", ParseInfo, (void*)38);
	Packet.AddEvent("#*#[NB]#*#|U=#39#|#*#[NB]", ParseInfo, (void*)39);
	Packet.AddEvent("#*#[NB]#*#|R=#40#|#*#[NB]", ParseInfo, (void*)40);
	Packet.AddEvent("#*#[NB]#*#|@=#89#|#*#[NB]", ParseInfo, (void*)89);
	Packet.AddEvent("#*#[NB]#*#|$=#90#|#*#[NB]", ParseInfo, (void*)90);
	Packet.AddEvent("#*#[NB]#*#|&=#91#|#*#[NB]", ParseInfo, (void*)91);
	Packet.AddEvent("#*#[NB]#*#|^=#92#|#*#[NB]", ParseInfo, (void*)92);
	Packet.AddEvent("#*#[NB]#*#|<=#93#|#*#[NB]", ParseInfo, (void*)93);
	Packet.AddEvent("#*#[NB]#*#|>=#94#|#*#[NB]", ParseInfo, (void*)94);
	Packet.AddEvent("#*#[NB]#*#|V=#95#|#*#[NB]", ParseInfo, (void*)95);
	Packet.AddEvent("#*#[NB]#*#|+=#96#|#*#[NB]", ParseInfo, (void*)96);
	Packet.AddEvent("#*#[NB]#*#|`=#97#|#*#[NB]", ParseInfo, (void*)97);
	Packet.AddEvent("#*#[NB]#*#|~=#98#|#*#[NB]", ParseInfo, (void*)98);
	Packet.AddEvent("#*#[NB]#*#|_=#99#|#*#[NB]", ParseInfo, (void*)99);
	Packet.AddEvent("#*#[NB]#*#|;=#100#|#*#[NB]", ParseInfo, (void*)100);
	Packet.AddEvent("#*#[NB]#*#|:=#101#|#*#[NB]", ParseInfo, (void*)101);
	Packet.AddEvent("#*#[NB]#*#|J=#102#|#*#[NB]", ParseInfo, (void*)102);
	Packet.AddEvent("#*#[NB]#*#|I=#103#|#*#[NB]", ParseInfo, (void*)103);
	Packet.AddEvent("#*#[NB]#*#|-=#104#|#*#[NB]", ParseInfo, (void*)104);
	ZeroMemory(sTimers, sizeof(sTimers));
	ZeroMemory(sBuffer, sizeof(sBuffer));
	ZeroMemory(wBuffer, sizeof(wBuffer));
	ZeroMemory(wChange, sizeof(wChange));
	ZeroMemory(wUpdate, sizeof(wUpdate));
	pNetBotsType = new MQ2NetBotsType;
	NetNote[0] = '\0';
	if (FindMQ2Data("NetBots"))
		RemoveMQ2Data("NetBots");
	AddMQ2Data("NetBots", dataNetBots);
	AddCommand("/netbots", Command);
	AddCommand("/bcz", BCZ);
	AddCommand("/bcza", BCZA);
	AddCommand("/bcgz", BCGZ);
	AddCommand("/bcgza", BCGZA);
	AddCommand("/netnote", CommandNote);
	AddXMLFile("MQUI_NetBotsWnd.xml");
	if (gGameState == GAMESTATE_INGAME)
	{
		sprintf_s(INIFileName, "%s\\%s_%s.ini", gPathConfig, GetServerShortName(), pLocalPC->Name);
		NetStat = GetPrivateProfileInt(PLUGIN_TITLE, "Stat", 0, INIFileName);
		NetGrab = GetPrivateProfileInt(PLUGIN_TITLE, "Grab", 0, INIFileName);
		NetSend = GetPrivateProfileInt(PLUGIN_TITLE, "Send", 0, INIFileName);
		bExtended = 0 != GetPrivateProfileInt(PLUGIN_TITLE, "Extended", 0, INIFileName);
		bUseSimpleSearch = 0 != GetPrivateProfileInt(PLUGIN_TITLE, "UseSimpleSearch", 0, INIFileName);
		NetInit = true;
		NetShow = GetPrivateProfileInt(PLUGIN_NAME, "Show", 0, INIFileName);
		if (NetShow)
			ShowMyWindow();
		ZeroMemory(sTimers, sizeof(sTimers));
		if (NetStat && NetSend && EQBCConnected())
			EQBCBroadCast("[NB]|Z=:>|[NB]");
	}
}

PLUGIN_API void ShutdownPlugin()
{
	RemoveCommand("/netbots");
	RemoveCommand("/netnote");
	RemoveCommand("/bcz");
	RemoveCommand("/bcza");
	RemoveCommand("/bcgz");
	RemoveCommand("/bcgza");
	Packet.Reset();
	RemoveMQ2Data("NetBots");
	delete pNetBotsType;
	DestroyMyWindow();
	RemoveXMLFile("MQUI_NetBotsWnd.xml");
}

///////////////////////////////////////////////////////////////////////////////
////
//
//      Window Class - Uses custom XLM Window - MQUI_NetBots.xml
//
////
///////////////////////////////////////////////////////////////////////////////

class CMyWnd : public CCustomWnd
{
public:
	CListWnd* List = nullptr;
	int       ErrorLoading = 0;

	CMyWnd() : CCustomWnd("NetBotsWnd")
	{
		List = (CListWnd*)GetChildItem("NBW_List");

		if (!List)
		{
			ErrorLoading = 1;
			WriteChatf("MQ2NetBots::Could not find NBW_List. Make sure your MQUI_NetBotsWnd.xml is up to date.");
			return;
		}
	}

	void ShowWin()
	{
		Show(1, 1);
		SetVisible(1);
	}
	void HideWin()
	{
		Show(0, 0);
		SetVisible(0);
	}

	virtual int WndNotification(CXWnd* pWnd, unsigned int Message, void* unknown) override
	{
		if (pWnd == 0)
		{
			if (Message == XWM_CLOSE)
			{
				CreateMyWindow();
				ShowWin();
				return 0;
			}
		}
		if (ErrorLoading)
		{
			WriteChatf("MQ2DB::Error could not find all the buttons in the UI XML. Get new UI");
		}
		return CSidlScreenWnd::WndNotification(pWnd, Message, unknown);
	};
};

CMyWnd* MyWnd = nullptr;

void CreateMyWindow()
{
	WriteChatf("CreateMyWindow:Start");
	if (MyWnd) return;
	if (pSidlMgr->FindScreenPieceTemplate("NetBotsWnd"))
	{
		WriteChatf("CreateMyWindow:Found NetBotsWnd");
		MyWnd = new CMyWnd;
		if (MyWnd && MyWnd->ErrorLoading)
		{
			delete MyWnd;
			MyWnd = 0;
		}
		if (MyWnd)
		{
			ReadWindowINI(MyWnd);
			WriteWindowINI(MyWnd);
			for (int i = 0; i < 64; i++)
				MyWnd->List->AddString("", 0xFFFFFFFF, 0, 0);
		}
	}
	WriteChatf("CreateMyWindow:Finished");
}

void DestroyMyWindow()
{
	if (MyWnd)
	{
		WriteWindowINI(MyWnd);
		MyWnd->List->DeleteAll();
		delete MyWnd;
		MyWnd = 0;
	}
}

// Need to save the Character specific file so when the window is destroyed we know where to save changes.
char szFileName[MAX_STRING];
char szSection[MAX_STRING];

void ReadWindowINI(CSidlScreenWnd* pWindow)
{
	char Buffer[MAX_STRING] = { 0 };
	sprintf_s(szFileName, "%s\\%s.ini", gPathConfig, PLUGIN_NAME);
	strcpy_s(szSection, "Default");
	if (pLocalPC) sprintf_s(szSection, "%s.%s", GetServerShortName(), pLocalPC->Name);
	pWindow->SetEscapable(0);
	pWindow->SetLocation(
		{
			(LONG)GetPrivateProfileInt(szSection, "ChatLeft", 164, szFileName),
			(LONG)GetPrivateProfileInt(szSection, "ChatTop", 357, szFileName),
			(LONG)GetPrivateProfileInt(szSection, "ChatRight", 575, szFileName),
			(LONG)GetPrivateProfileInt(szSection, "ChatBottom", 620, szFileName)
		}
	);

	pWindow->SetLocked((GetPrivateProfileInt(szSection, "Locked", 0, szFileName) ? true : false));
	pWindow->SetFades((GetPrivateProfileInt(szSection, "Fades", 1, szFileName) ? true : false));
	pWindow->SetFadeDelay(GetPrivateProfileInt(szSection, "Delay", 2000, szFileName));
	pWindow->SetFadeDuration(GetPrivateProfileInt(szSection, "Duration", 500, szFileName));
	pWindow->SetAlpha(GetPrivateProfileInt(szSection, "Alpha", 255, szFileName));
	pWindow->SetFadeToAlpha(GetPrivateProfileInt(szSection, "FadeToAlpha", 255, szFileName));
	pWindow->SetBGType(GetPrivateProfileInt(szSection, "BGType", 1, szFileName));
	ARGBCOLOR col = { 0 };
	col.A = GetPrivateProfileInt(szSection, "BGTint.alpha", 255, szFileName);
	col.R = GetPrivateProfileInt(szSection, "BGTint.red", 0, szFileName);
	col.G = GetPrivateProfileInt(szSection, "BGTint.green", 0, szFileName);
	col.B = GetPrivateProfileInt(szSection, "BGTint.blue", 0, szFileName);
	pWindow->SetBGColor(col.ARGB);
}

template <unsigned int SizeT>LPSTR SafeItoa(int _Value, char(&_Buffer)[SizeT], int _Radix)
{
	errno_t err = _itoa_s(_Value, _Buffer, _Radix);
	if (!err)
	{
		return _Buffer;
	}
	return "";
}


void WriteWindowINI(CSidlScreenWnd* pWindow)
{
	char szTemp[MAX_STRING] = { 0 };

	if (pWindow->IsMinimized())
	{
		WritePrivateProfileString(szSection, "ChatTop", SafeItoa(pWindow->GetOldLocation().top, szTemp, 10), szFileName);
		WritePrivateProfileString(szSection, "ChatBottom", SafeItoa(pWindow->GetOldLocation().bottom, szTemp, 10), szFileName);
		WritePrivateProfileString(szSection, "ChatLeft", SafeItoa(pWindow->GetOldLocation().left, szTemp, 10), szFileName);
		WritePrivateProfileString(szSection, "ChatRight", SafeItoa(pWindow->GetOldLocation().right, szTemp, 10), szFileName);
	}
	else
	{
		WritePrivateProfileString(szSection, "ChatTop", SafeItoa(pWindow->GetLocation().top, szTemp, 10), szFileName);
		WritePrivateProfileString(szSection, "ChatBottom", SafeItoa(pWindow->GetLocation().bottom, szTemp, 10), szFileName);
		WritePrivateProfileString(szSection, "ChatLeft", SafeItoa(pWindow->GetLocation().left, szTemp, 10), szFileName);
		WritePrivateProfileString(szSection, "ChatRight", SafeItoa(pWindow->GetLocation().right, szTemp, 10), szFileName);
	}
	WritePrivateProfileString(szSection, "Locked", SafeItoa(pWindow->IsLocked(), szTemp, 10), szFileName);
	WritePrivateProfileString(szSection, "Fades", SafeItoa(pWindow->GetFades(), szTemp, 10), szFileName);
	WritePrivateProfileString(szSection, "Delay", SafeItoa(pWindow->GetFadeDelay(), szTemp, 10), szFileName);
	WritePrivateProfileString(szSection, "Duration", SafeItoa(pWindow->GetFadeDuration(), szTemp, 10), szFileName);
	WritePrivateProfileString(szSection, "Alpha", SafeItoa(pWindow->GetAlpha(), szTemp, 10), szFileName);
	WritePrivateProfileString(szSection, "FadeToAlpha", SafeItoa(pWindow->GetFadeToAlpha(), szTemp, 10), szFileName);
	WritePrivateProfileString(szSection, "BGType", SafeItoa(pWindow->GetBGType(), szTemp, 10), szFileName);
	ARGBCOLOR col = { 0 };
	col.ARGB = pWindow->GetBGColor();
	WritePrivateProfileString(szSection, "BGTint.alpha", SafeItoa(col.A, szTemp, 10), szFileName);
	WritePrivateProfileString(szSection, "BGTint.red", SafeItoa(col.R, szTemp, 10), szFileName);
	WritePrivateProfileString(szSection, "BGTint.green", SafeItoa(col.G, szTemp, 10), szFileName);
	WritePrivateProfileString(szSection, "BGTint.blue", SafeItoa(col.B, szTemp, 10), szFileName);
}

void WndListPrintf(CListWnd* pWnd, int R, int C, long Color, char* zFormat, ...)
{
	va_list vaList;
	va_start(vaList, zFormat);
	char szTemp[MAX_STRING];
	vsprintf_s(szTemp, MAX_STRING, zFormat, vaList);
	if (pWnd)
	{
		pWnd->SetItemText(R, C, szTemp);
		pWnd->SetItemColor(R, C, Color);
	}

}

void WndListSetPerc(CListWnd* pWnd, int R, int C, long Cur, long Max)
{
	float v;
	if (Max < 1)
	{
		WndListPrintf(pWnd, R, C, 0xFFFFFFFF, "");
		return;
	}
	v = (float)100.0 * Cur / Max;
	if (v > 70)
		WndListPrintf(pWnd, R, C, 0xFF20FF20, "%3.0f", v);
	else if (v > 40)
		WndListPrintf(pWnd, R, C, 0xFF88B888, "%3.0f", v);
	else
		WndListPrintf(pWnd, R, C, 0xFFFFA000, "%3.0f", v);
}

DWORD GetNameColor(BotInfo* BotRec)
{
	if (!BotRec)
		return 0;
	if (!inZoned(BotRec->ZoneID, BotRec->InstID))
		return 0xFF9FCFFF;
	float DX = pLocalPlayer->X - BotRec->X;
	float DY = pLocalPlayer->Y - BotRec->Y;
	float DZ = pLocalPlayer->Z - BotRec->Z;
	float DD = sqrt(DX * DX + DY * DY + DZ * DZ);
	if (DD > 100)
		return 0xFFFFCF9F;
	return 0xFFFFFFFF;
}


void WindowSetDY(CSidlScreenWnd* pWindow, long DY)
{
	long T = pWindow->GetLocation().top;
	long B = pWindow->GetLocation().bottom;
	long L = pWindow->GetLocation().left;
	long R = pWindow->GetLocation().right;

	//pWindow->SetLocation({ L , T , R , T + DY });
	//pWindow->SetClientRectChanged(true);
	((CXWnd*)(pWindow))->Move(
		{ L, T, R, T + DY }
	);
}



void WindowUpdate()
{
	static int MaxLines = 0;

	if (!MyWnd || !MyWnd->List)
		return;
	NetShow = MQGetTickCount64() + 100;
	if (MyWnd->IsVisible())
	{
		DWORD White = 0xFFFFFFFF;
		DWORD Color = White;
		map<string, BotInfo>::iterator l;
		BotInfo* BotRec;
		int CurSel = MyWnd->List->GetCurSel();

		int R = 0;

		R = 0;
		for (l = NetMap.begin(); l != NetMap.end(); l++)
		{
			BotRec = &(*l).second;
			if (BotRec->SpawnID == 0)
				continue;

			WndListSetPerc(MyWnd->List, R, 0, BotRec->ManaCur, BotRec->ManaMax);
			WndListSetPerc(MyWnd->List, R, 1, BotRec->EnduCur, BotRec->EnduMax);
			WndListSetPerc(MyWnd->List, R, 2, BotRec->LifeCur, BotRec->LifeMax);

			// Prep State String
			sprintf_s(zOutput, " ");

			if (BotRec->State & STATE_MOUNT)
				zOutput[0] = 'M';
			if (BotRec->State & STATE_WANTAGGRO)
				zOutput[0] = 'A';
			if (BotRec->State & STATE_HAVEAGGRO)
				zOutput[0] = 'T';
			if (BotRec->State & STATE_SIT)
				zOutput[0] = 'S';

			if (BotRec->State & STATE_INVIS && BotRec->State & STATE_ITU)
				zOutput[0] = 'B';
			else if (BotRec->State & STATE_INVIS)
				zOutput[0] = 'I';
			else if (BotRec->State & STATE_ITU)
				zOutput[0] = 'U';
			if (BotRec->State & STATE_STUN)
				zOutput[0] = '-';
			if (BotRec->State & STATE_DEAD)
				zOutput[0] = 'x';

			WndListPrintf(MyWnd->List, R, 3, White, zOutput);

			// Print out name
			Color = GetNameColor(BotRec);
			WndListPrintf(MyWnd->List, R, 4, Color, BotRec->Name);

			// Print out Target - shorten if too long
			char* p = zOutput;
			zOutput[0] = 0;
			if (BotRec->TargetID)
			{
				Evaluate("${Spawn[id %d].Name}", BotRec->TargetID);
				if (strcmp(zOutput, "NULL") == 0)
					sprintf_s(zOutput, "id %d", BotRec->TargetID);
				if (strlen(zOutput) > 12)
					p = &zOutput[strlen(zOutput) - 12];
			}
			WndListPrintf(MyWnd->List, R, 5, White, p);

			// Print out Action field
			if (BotRec->CastID)
				WndListPrintf(MyWnd->List, R, 6, White, GetSpellByID(BotRec->CastID)->Name);
			else if (BotRec->State & STATE_ATTACK)
				WndListPrintf(MyWnd->List, R, 6, White, "Melee");
			else if (BotRec->State & STATE_RANGED)
				WndListPrintf(MyWnd->List, R, 6, White, "Ranged");
			else
				WndListPrintf(MyWnd->List, R, 6, White, "");

			// If we have selected someone from the list pop them up
			if (CurSel == R)
			{
				if (_stricmp(pLocalPC->Name, BotRec->Name) != 0)
				{
					sprintf_s(zOutput, "/bct %s //foreground", BotRec->Name);
					EzCommand(zOutput);
				}
			}

			// Next ...
			R++;
		}

		// If we lost some characters clear the old data
		for (int i = R; i < MaxLines; i++)
		{
			for (int C = 0; C <= 6; C++)
				WndListPrintf(MyWnd->List, i, C, White, "");
		}

		WindowSetDY(MyWnd, 46 + 14 * R);
		MaxLines = R;
	}

	MyWnd->List->SetCurSel(-1);
}


void HideMyWindow()
{
	if (!MyWnd)
		return;
	MyWnd->HideWin();
}

void ShowMyWindow()
{
	if (gGameState != GAMESTATE_INGAME || !pLocalPlayer)
		return;
	if (!MyWnd)
		CreateMyWindow();
	if (!MyWnd)
		return;
	MyWnd->ShowWin();
	WindowUpdate();
}

PLUGIN_API void OnCleanUI()
{
	DestroyMyWindow();
}

PLUGIN_API void OnReloadUI()
{
	if (gGameState == GAMESTATE_INGAME && pLocalPlayer)
	{
		int Show = (MyWnd && MyWnd->IsVisible());
		DestroyMyWindow();
		if (Show)
			ShowMyWindow();
	}
}
