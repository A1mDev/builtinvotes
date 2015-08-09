/**
 * vim: set ts=4 :
 * =============================================================================
 * Builtin Votes
 * Copyright (C) 2011 Ross Bemrose (Powerlord).  All rights reserved.
 * =============================================================================
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License, version 3.0, as published by the
 * Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * As a special exception, AlliedModders LLC gives you permission to link the
 * code of this program (as well as its derivative works) to "Half-Life 2," the
 * "Source Engine," the "SourcePawn JIT," and any Game MODs that run on software
 * by the Valve Corporation.  You must obey the GNU General Public License in
 * all respects for all other code used.  Additionally, AlliedModders LLC grants
 * this exception to all derivative works.  AlliedModders LLC defines further
 * exceptions, found in LICENSE.txt (as of this writing, version JULY-31-2007),
 * or <http://www.sourcemod.net/license.php>.
 *
 * Version: $Id$
 */

#ifndef _INCLUDE_BUILTINVOTESTYLE_TF2_H
#define _INCLUDE_BUILTINVOTESTYLE_TF2_H

#include "extension.h"
#include "BuiltinVoteStyle_Base.h"

using namespace SourceMod;

#define MAX_VOTE_OPTIONS 5 // Defined by TF2

#define TEAMS_ALL -1

#define TF2_EXTEND										"Extend current Map"
#define TF2_DISABLED									"(Disabled on Server)"

// These are swapped from the order they appear in L4D/L4D2
#define TF2_VOTE_YES_INDEX								0
#define TF2_VOTE_NO_INDEX								1

// User vote to kick user.
#define TRANSLATION_TF2_VOTE_KICK_IDLE_START			"#TF_vote_kick_player_idle"
#define TRANSLATION_TF2_VOTE_KICK_SCAMMING_START		"#TF_vote_kick_player_scamming"
#define TRANSLATION_TF2_VOTE_KICK_CHEATING_START		"#TF_vote_kick_player_cheating"
#define TRANSLATION_TF2_VOTE_KICK_START					"#TF_vote_kick_player_other"
#define TRANSLATION_TF2_VOTE_KICK_PASSED				"#TF_vote_passed_kick_player"

// User vote to restart map.
#define TRANSLATION_TF2_VOTE_RESTART_START				"#TF_vote_restart_game"
#define TRANSLATION_TF2_VOTE_RESTART_PASSED				"#TF_vote_passed_restart_game"

// User vote to change maps.
#define TRANSLATION_TF2_VOTE_CHANGELEVEL_START			"#TF_vote_changelevel"
#define TRANSLATION_TF2_VOTE_CHANGELEVEL_PASSED			"#TF_vote_passed_changelevel"

// User vote to change next level.
#define TRANSLATION_TF2_VOTE_NEXTLEVEL_SINGLE_START		"#TF_vote_nextlevel"
#define TRANSLATION_TF2_VOTE_NEXTLEVEL_MULTIPLE_START	"#TF_vote_nextlevel_choices" // Started by server
#define TRANSLATION_TF2_VOTE_NEXTLEVEL_EXTEND_PASSED	"#TF_vote_passed_nextlevel_extend"
#define TRANSLATION_TF2_VOTE_NEXTLEVEL_PASSED			"#TF_vote_passed_nextlevel"

// User vote to scramble teams.  Can be immediate or end of round.
#define TRANSLATION_TF2_VOTE_SCRAMBLE_IMMEDIATE_START	"#TF_vote_scramble_teams"
#define TRANSLATION_TF2_VOTE_SCRAMBLE_ROUNDEND_START	"#TF_vote_should_scramble_round"
#define TRANSLATION_TF2_VOTE_SCRAMBLE_PASSED 			"#TF_vote_passed_scramble_teams"

// While not a vote string, it works just as well.
#define TRANSLATION_TF2_VOTE_CUSTOM						"#TF_playerid_noteam"

class CTF2BuiltinVote;

class TF2BuiltinVoteStyle : public BaseBuiltinVoteStyle
{
public:
	TF2BuiltinVoteStyle();
public: // BaseBuiltinVoteStyle
	//void SendDisplay(int clients[], unsigned int num_clients, IBaseBuiltinVote *vote);
public: //IBuiltinVoteStyle
	const char *GetStyleName();
	bool CheckVoteType(BuiltinVoteType type);
	IBaseBuiltinVote *CreateVote(IBuiltinVoteHandler *handler, BuiltinVoteType type, IdentityToken_t *pOwner=NULL);
	unsigned int GetMaxItems();
	unsigned int GetApproxMemUsage();
private:
	//void HookCreateMessage(edict_t *pEdict, DIALOG_TYPE type, KeyValues *kv, IServerPluginCallbacks *plugin);
private:
	bool m_bOptionsSent;
};

class CTF2BuiltinVote : public CBaseBuiltinVote
{
public:
	CTF2BuiltinVote(IBuiltinVoteHandler *pHandler, IBuiltinVoteStyle *pStyle, BuiltinVoteType type, IdentityToken_t *pOwner);
	~CTF2BuiltinVote();
public: //IBaseBuiltinVote
	void OnClientCommand(edict_t *pEntity, const CCommand &cmd);
	bool Display(int client);
	bool Display(int clients[], unsigned int num_clients);
	unsigned int GetApproxMemUsage();
public: //CBaseBuiltinVote
	void Cancel_Finally();
	void DisplayVotePass(const char* winner="");
	void DisplayVotePass(const char *translation, const char* winner="");
	void DisplayVoteFail(BuiltinVoteFailReason reason=BuiltinVoteFail_Generic);
	void DisplayVoteFail(int client, BuiltinVoteFailReason reason=BuiltinVoteFail_Generic);
	//void DisplayCallVoteFail(int client, BuiltinCallVoteFailReason reason, short param1=0);
	void ClientSelectedItem(int client, unsigned int item);
	bool UpdateVoteCounts(unsigned int items, CVector<unsigned int> votes, unsigned int totalClients);
protected:
	void InternalDisplayVoteFail(int clients[], unsigned int num_clients, BuiltinVoteFailReason reason);
private:
	bool m_bOptionsSent;
};

extern TF2BuiltinVoteStyle g_TF2BuiltinVoteStyle;
extern IServerGameClients *servergameclients;

#endif //_INCLUDE_BUILTINVOTESTYLE_TF2_H
