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

#ifndef _INCLUDE_BUILTINVOTESTYLE_L4D_BASE_H
#define _INCLUDE_BUILTINVOTESTYLE_L4D_BASE_H

#include "extension.h"
#include "BuiltinVoteStyle_Base.h"

using namespace SourceMod;

// User vote to kick user.
#define TRANSLATION_L4D_VOTE_KICK_START					"#L4D_vote_kick_player"
#define TRANSLATION_L4D_VOTE_KICK_PASSED				"#L4D_vote_passed_kick_player"

// User vote to restart map.
#define TRANSLATION_L4D_VOTE_RESTART_START				"#L4D_vote_restart_game"
#define TRANSLATION_L4D_VOTE_RESTART_PASSED				"#L4D_vote_passed_restart_game"

// User vote to change maps.
#define TRANSLATION_L4D_VOTE_CHANGECAMPAIGN_START		"#L4D_vote_mission_change"
#define TRANSLATION_L4D_VOTE_CHANGECAMPAIGN_PASSED		"#L4D_vote_passed_mission_change"
#define TRANSLATION_L4D_VOTE_CHANGELEVEL_START			"#L4D_vote_chapter_change"
#define TRANSLATION_L4D_VOTE_CHANGELEVEL_PASSED			"#L4D_vote_passed_chapter_change"

// User vote to return to lobby.
#define TRANSLATION_L4D_VOTE_RETURNTOLOBBY_START		"#L4D_vote_return_to_lobby"
#define TRANSLATION_L4D_VOTE_RETURNTOLOBBY_PASSED		"#L4D_vote_passed_return_to_lobby"

// User vote to change difficulty.
#define TRANSLATION_L4D_VOTE_CHANGEDIFFICULTY_START		"#L4D_vote_change_difficulty"
#define TRANSLATION_L4D_VOTE_CHANGEDIFFICULTY_PASSED	"#L4D_vote_passed_change_difficulty"

// User vote to change alltalk.
#define TRANSLATION_L4D_VOTE_ALLTALK_START				"#L4D_vote_alltalk_change"
#define TRANSLATION_L4D_VOTE_ALLTALK_PASSED				"#L4D_vote_passed_alltalk_change"
#define TRANSLATION_L4D_VOTE_ALLTALK_ENABLE				"#L4D_vote_alltalk_enable"
#define TRANSLATION_L4D_VOTE_ALLTALK_DISABLE			"#L4D_vote_alltalk_disable"

// While not a vote string, it works just as well.
#define TRANSLATION_L4D_VOTE_CUSTOM						"#L4D_TargetID_Player"

class CL4DBaseBuiltinVote;

class L4DBaseBuiltinVoteStyle : public BaseBuiltinVoteStyle
{
public: // BaseBuiltinVoteStyle
	//virtual void SendDisplay(int clients[], unsigned int num_clients, IBaseBuiltinVote *vote) =0;
public: //IBuiltinVoteStyle
	virtual const char *GetStyleName() =0;
	virtual bool CheckVoteType(BuiltinVoteType type) =0;
	virtual IBaseBuiltinVote *CreateVote(IBuiltinVoteHandler *handler, BuiltinVoteType type, IdentityToken_t *pOwner=NULL) =0;
	unsigned int GetMaxItems();
	virtual unsigned int GetApproxMemUsage() =0;
};

class CL4DBaseBuiltinVote : public CBaseBuiltinVote
{
public:
	CL4DBaseBuiltinVote(IBuiltinVoteHandler *pHandler, IBuiltinVoteStyle *pStyle, BuiltinVoteType type, IdentityToken_t *pOwner);
public: //IBaseBuiltinVote
	void OnClientCommand(edict_t *pEntity, const CCommand &cmd);
	virtual bool Display(int clients[], unsigned int num_clients) =0;
	virtual unsigned int GetApproxMemUsage() =0;
	virtual bool UpdateVoteCounts(unsigned int items, CVector<unsigned int> votes, unsigned int totalClients);
public: // CBaseBuiltinVote
	virtual void Cancel_Finally() =0;
	virtual void DisplayVotePass(const char *winner="") =0;
	virtual void DisplayVotePass(const char *translation, const char *winner="") =0;
	virtual void DisplayVoteFail(BuiltinVoteFailReason reason=BuiltinVoteFail_Generic) =0;
	virtual void DisplayVoteFail(int client, BuiltinVoteFailReason reason=BuiltinVoteFail_Generic) =0;
	//virtual void DisplayCallVoteFail(int client, BuiltinCallVoteFailReason reason, short param1) =0;
	virtual void ClientSelectedItem(int client, unsigned int item) =0;
};


//extern L4DBuiltinVoteStyle g_L4DBuiltinVoteStyle;
extern IServerGameClients *servergameclients;

#endif //_INCLUDE_BUILTINVOTESTYLE_L4D_BASE_H
