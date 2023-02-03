# Description
MQ2NetBots provides linked MQ2EQBC clients a method of sharing status information and statistics. It also adds a new Top-Level Object ${NetBots}.

# Commands
/netbots on/off Toggles NetBots on or off.<BR>
/netbots grab=on/off Toggles receiving status updates from other MQ2NetBot clients connected to EQBCS.<BR>
/netbots send=on/off Toggles MQ2NetBot clients sending of status updates to EQBCS on or off.<BR>
/netbots ext=on/off Toggles MQ2NetBot clients sending of extended status updates to EQBCS on or off (such as Buffs, Gems, etc. - see TLO information below for all affected entries).<BR>

# INI Example
SERVER_CHARACTER.INI

[MQ2NetBots]<BR>
Stat=1<BR>
Grab=1<BR>
Send=1<BR>
Extended=1<BR>

# INI Settings Definition
SERVER_CHARACTER.INI is SERVER=ShortServerName and CHARACTER=YourCharName.<BR> Example for character called Bubba on the Povar server: Povar\_Bubba.ini

All settings are in the [MQ2NetBots] section.


Stat This enables (set to 1) or disables (set to 0) mq2netbots send and receive data and processing. Setting this to 0 effectively disables the plugin. Using the /netbots on or /netbots off commands toggle this from in game.<BR><BR>
Grab Whether to listen for NetBots data. If this particular character needs to send information, but doesn't care about information from other clients, you can set this to 0 so it doesn't process incoming data. Set to 1 to process the data. Using the /netbots grab=on or /netbots grab=off commands toggle this from in game.<BR><BR>
Send Whether to send NetBots data to other clients via eqbc. If this character needs to receive other netbots client info, but doesn't need to send it, you can set this to 0 to disable it. Set to 1 to enable. Using the /netbots send=on or /netbots send=off commands toggle this from in game.<BR><BR>
Extended Whether to send additional detailed data to other netbots clients via eqbc, such as Gems, Buffs, Durations, and endurance information. Set to 0 to disable, 1 to enable. Using the /netbots ext=on or /netbots ext=off commands toggle this from in game.


#Macro Data

Examples

/echo ${NetBots[MY\_TOON].CurrentHPs}<BR>
/echo ${NetBots[MY\_TOON].Buff[4]}

#Top-Level Object

${NetBots} Returns information about your client.<BR>
${NetBots[Name]} Returns information about the connected toon Name.

#Data Types - NetBots

${NetBots.Client} :String Returns currently broadcasting MQ2NetBot clients connect to EQBCS.<BR>
${NetBots.Counts} :Int Returns count of broadcasting MQ2NetBot clients connect to EQBCS.<BR>
${NetBots.Enable} : Bool Returns TRUE/FALSE based on plugin status.<BR>
${NetBots.Listen} :Bool Returns TRUE/FALSE based on grab parameter status.<BR>
${NetBots.Output} :Bool Returns TRUE/FALSE based on send parameter status.<BR>

#Data Types - NetBots Clients

${NetBots[Name].Name} :String Name of Name.<BR>
${NetBots[Name].Zone} :Int Zone ID of Name.<BR>
${NetBots[Name].Instance} :Int Instance ID of Name.<BR>
${NetBots[Name].ID} :Int Spawn ID of Name.<BR>
${NetBots[Name].Class} :Class Class of Name.<BR>
${NetBots[Name].Level} :Int Level of Name.<BR>
${NetBots[Name].PctExp} :Float Percent Experience of Name.<BR>
${NetBots[Name].PctAAExp} :Float Percent AA Experience of Name.<BR>
${NetBots[Name].PctGroupLeaderExp} :Float Percent Group Leader Experience of Name. EMU servers only.<BR>
${NetBots[Name].CurrentHPs} Current Hitpoints of Name.<BR>
${NetBots[Name].MaxHPs} :Int Total Hitpoints of Name<BR>
${NetBots[Name].PctHPs} :Int Current Hitpoints percentage of Name.<BR>
${NetBots[Name].CurrentEndurance} :Int Current Endurace of Name. Requires Extended=1 in the ini.<BR>
${NetBots[Name].MaxEndurance} :Int Total Endurance of Name. Requires Extended=1 in the ini.<BR>
${NetBots[Name].PctEndurace} :Int Current Endurance percentage of Name. Requires Extended=1 in the ini.<BR>
${NetBots[Name].CurrentMana} :Int Current Mana of Name.<BR>
${NetBots[Name].MaxMana} :Int Total Mana of Name.<BR>
${NetBots[Name].PctMana} :Int Current Mana percentage of Name.<BR>
${NetBots[Name].PetID} :Int Spawn ID of Name's pet.<BR>
${NetBots[Name].PetHP} :Int Hitpoints of Name's pet.<BR>
${NetBots[Name].TargetID} :Int Spawn ID of Name's target.<BR>
${NetBots[Name].TargetHP} :Int Hitpoints of Name's target.<BR>
${NetBots[Name].Casting} :Spell Spell Name is casting.<BR>
${NetBots[Name].State} :String State of Name (STUN STAND SIT DUCK BIND FEIGN DEAD UNKNOWN).<BR>
${NetBots[Name].Attacking} :Bool Is Name Attacking?<BR>
${NetBots[Name].AFK} :Bool Is Name AFK?<BR>
${NetBots[Name].Binding} :Bool Is Name kneeling?<BR>
${NetBots[Name].Ducking} :Bool Is Name ducking?<BR>
${NetBots[Name].Feigning} :Bool Is Name feigning?<BR>
${NetBots[Name].Grouped} :Bool Is Name in a group? (not necessarily your own group)<BR>
${NetBots[Name].Invis} :Bool Is Name invis?<BR>
${NetBots[Name].Levitating} :Bool Is Name levitating?<BR>
${NetBots[Name].LFG} :Bool Is Name LFG?<BR>
${NetBots[Name].Mounted} :Bool Is Name on a mount?<BR>
${NetBots[Name].Moving} :Bool Is Name moving?<BR>
${NetBots[Name].Detrimentals} :Int Total of detrimental counts for Name.<BR>
${NetBots[Name].Detrimental} :String A string list of all detrimental types affecting Name.<BR>
${NetBots[Name].Raid} :Bool Is Name in a raid?<BR>
${NetBots[Name].Sitting} :Bool Is Name sitting?<BR>
${NetBots[Name].Standing} :Bool Is Name standing?<BR>
${NetBots[Name].Stunned} :Bool Is Name stunned?<BR>
${NetBots[Name].FreeBuffSlots} :Int Total free buff slots Name has.<BR>
${NetBots[Name].InZone} :Bool Is Name in the same zone as you?<BR>
${NetBots[Name].InGroup} :Bool Is Name in your group?<BR>
${NetBots[Name].Leader} :String Is Name the group leader?<BR>
${NetBots[Name].Note} :String Is Name the group leader?<BR>
${NetBots[Name].Updated} :Int Timestamp of last NetBots update from Name.<BR>
${NetBots[Name].Gem} :String All spells Name has memorized. Requires Extended=1 in the ini.<BR>
${NetBots[Name].Gem[#]}} :Spell Spell Name has memorized in Gem[#]. Requires Extended=1 in the ini.<BR>
${NetBots[Name].Buff} :String All buffs Name currently has. Requires Extended=1 in the ini.<BR>
${NetBots[Name].Buff[#]} :Spell Name of the buff Name has in buff slot [#]. Requires Extended=1 in the ini.<BR>
${NetBots[Name].Buff[spellname]} :Spell pSpellType of the buff Name has if he has spellname buff. Requires Extended=1 in the ini.<BR>
${NetBots[Name].Duration} :String Duration remaining of all buffs Name has. Requires Extended=1 in the ini.<BR>
${NetBots[Name].Duration[#]} :Int Duration of the buff on Name in slot [#]. Requires Extended=1 in the ini.<BR>
${NetBots[Name].ShortBuff} :String All the short duration buffs Name has.<BR>
${NetBots[Name].ShortBuff[#]} :Spell Name of the shortbuff Name has in slot [#].<BR>
${NetBots[Name].ShortBuff[spellname]} :Spell pSpellType of the short duration buff Name has if he has spellname short duration buff.<BR>
${NetBots[Name].PetBuff} :String Name of all the pet buffs Name's pet has.<BR>
${NetBots[Name].PetBuff[#]} :Spell Name of the pet buff in slot [#] Name's pet has.<BR>
${NetBots[Name].PetBuff[spellname]} :Spell pSpellType of the pet buff Name has if his pet has spellname buff.<BR>
${NetBots[Name].Stacks[#]} :Bool Returns true if the buff ID number [#] will stack on Name.<BR>
${NetBots[Name].TotalAA} :Int Total AAs Name has.<BR>
${NetBots[Name].UsedAA} :Int Total spend AAs of Name.<BR>
${NetBots[Name].UnusedAA} :Int Total unspent AAs of Name.<BR>
${NetBots[Name].CombatState} :Int Combat State of Name.<BR>
${NetBots[Name].Counters} :Int Cursed,etc. counters for Name.<BR>
${NetBots[Name].Cursed} :Int Cursed counters for Name.<BR>
${NetBots[Name].Diseased} :Int Diseased counters for Name.<BR>
${NetBots[Name].Poisoned} :Int Poisoned counters for Name.<BR>
${NetBots[Name].Corrupted} :Int Corrupted counters for Name.<BR>
${NetBots[Name].EnduDrain} :Int Endurance drain counters for Name.<BR>
${NetBots[Name].LifeDrain} :Int HP drain counters for Name.<BR>
${NetBots[Name].ManaDrain} :Int Mana drain counters for Name.<BR>
${NetBots[Name].Blinded} :Int Blinded counters for Name.<BR>
${NetBots[Name].CastingLevel} :Int CastingLevel counters for Name.<BR>
${NetBots[Name].Charmed} :Int Charmed counters for Name.<BR>
${NetBots[Name].Feared} :Int Feared counters for Name.<BR>
${NetBots[Name].Healing} :Int Healing counters for Name.<BR>
${NetBots[Name].Invulnerable} :Int Invulnerable counters for Name.<BR>
${NetBots[Name].Mesmerized} :Int Mesmerized counters for Name.<BR>
${NetBots[Name].Rooted} :Int Rooted counters for Name.<BR>
${NetBots[Name].Silenced} :Int Silenced counters for Name.<BR>
${NetBots[Name].Slowed} :Int Slowed counters for Name.<BR>
${NetBots[Name].Snared} :Int Snared counters for Name.<BR>
${NetBots[Name].SpellCost} :Int SpellCost counters for Name.<BR>
${NetBots[Name].SpellSlowed} :Int SpellSlowed counters for Name.<BR>
${NetBots[Name].SpellDamage} :Int SpellDamage counters for Name.<BR>
${NetBots[Name].Trigger} :Int Trigger counters for Name.<BR>
${NetBots[Name].Resistance} :Int Resistance counters for Name.<BR>
${NetBots[Name].NoCure} :Int NoCure counters for Name.<BR>
${NetBots[Name].Location} :String The Y,X,Z location of Name.<BR>
${NetBots[Name].Heading} :String The Heading of Name.<BR>
${NetBots[Name].MacroName} :String The running macro of Name, empty string if none running.<BR>
${NetBots[Name].MacroState} :Int The macro state for Name. 0=No macro running, 1=Running, 2=Paused<BR>
${NetBots[Name].NavigationActive} :Bool If running a MQ2Nav path for Name.<BR>
${NetBots[Name].NavigationPaused} :Bool If a MQ2Nav path is paused for Name.<BR>
