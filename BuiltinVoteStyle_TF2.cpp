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

#include "BuiltinVoteStyle_TF2.h"

/* This will error out if you try to compile for lower source engines.  Since this class should only be included
 * for recent engines, I don't see that as a problem
 */ 
SH_DECL_HOOK2_void(IServerGameClients, ClientCommand, SH_NOATTRIB, 0, edict_t *, const CCommand &);

TF2BuiltinVoteStyle g_TF2BuiltinVoteStyle;

TF2BuiltinVoteStyle::TF2BuiltinVoteStyle() : m_bOptionsSent(false)
{
}

bool TF2BuiltinVoteStyle::CheckVoteType(BuiltinVoteType type)
{
	switch(type)
	{
	case BuiltinVoteType_Kick:
	case BuiltinVoteType_Restart:
	case BuiltinVoteType_ChgLevel:
	case BuiltinVoteType_Custom_YesNo:
	case BuiltinVoteType_KickIdle:
	case BuiltinVoteType_KickScamming:
	case BuiltinVoteType_KickCheating:
	case BuiltinVoteType_NextLevel:
	case BuiltinVoteType_NextLevelMult:
	case BuiltinVoteType_ScrambleNow:
	case BuiltinVoteType_ScrambleEnd:
	case BuiltinVoteType_Custom_Mult:
		return true;
		break;
	default:
		return false;
		break;
	}

}

IBaseBuiltinVote *TF2BuiltinVoteStyle::CreateVote(IBuiltinVoteHandler *handler, BuiltinVoteType type, IdentityToken_t *pOwner)
{
	if (CheckVoteType(type))
	{
		return new CTF2BuiltinVote(handler, this, type, pOwner);
	}
	else
	{
		return NULL;
	}
}

const char *TF2BuiltinVoteStyle::GetStyleName()
{
	return "tf2";
}

unsigned int TF2BuiltinVoteStyle::GetMaxItems()
{
	return 5;
}

unsigned int TF2BuiltinVoteStyle::GetApproxMemUsage()
{
	return sizeof(TF2BuiltinVoteStyle) + (sizeof(CBaseBuiltinVotePlayer) * 257);
}

CTF2BuiltinVote::CTF2BuiltinVote(IBuiltinVoteHandler *pHandler, IBuiltinVoteStyle *pStyle, BuiltinVoteType type, IdentityToken_t *pOwner) :
CBaseBuiltinVote(pHandler, pStyle, type, pOwner), m_bOptionsSent(false)
{
}

CTF2BuiltinVote::~CTF2BuiltinVote()
{
	if (!m_bResultDisplayed)
	{
		SH_REMOVE_HOOK(IServerGameClients, ClientCommand, servergameclients, SH_MEMBER(this, &CTF2BuiltinVote::OnClientCommand), false);
	}
}

void CTF2BuiltinVote::Cancel_Finally()
{
	g_TF2BuiltinVoteStyle.CancelVote(this);
}

bool CTF2BuiltinVote::Display(int client)
{
	if (m_bCancelling)
	{
		return false;
	}

	int clients[1] = { client };

	return Display(clients, 1);
}

bool CTF2BuiltinVote::Display(int clients[], unsigned int num_clients)
{
	if (m_bCancelling)
	{
		return false;
	}

	// Send options if they haven't been sent yet.
	if (!m_bOptionsSent)
	{
		const char *prefix = "option";

		IGameEvent *optionsEvent = events->CreateEvent("vote_options");
		
		unsigned int maxCount = GetItemCount();
		for (unsigned int i=0; i < maxCount; i++)
		{
			char option[7+1];
			// I hate string concatenation in C/C++
			snprintf(option, sizeof(option), "%s%d", prefix, i+1);

			const char *display;
			display = GetItemDisplay(i);
			optionsEvent->SetString(option, display);
		}
		optionsEvent->SetInt("count", maxCount);

		events->FireEvent(optionsEvent);
		m_bOptionsSent = true;
	}

	int msgId = usermsgs->GetMessageIndex("VoteStart");
	const char *translation;
	bool bYesNo = true;

	switch (m_voteType)
	{
	
	case BuiltinVoteType_Kick:
		translation = TRANSLATION_TF2_VOTE_KICK_START;
		break;

	case BuiltinVoteType_Restart:
		translation = TRANSLATION_TF2_VOTE_RESTART_START;
		break;

	case BuiltinVoteType_ChgLevel:
		translation = TRANSLATION_TF2_VOTE_CHANGELEVEL_START;
		break;

	case BuiltinVoteType_KickIdle:
		translation = TRANSLATION_TF2_VOTE_KICK_IDLE_START;
		break;

	case BuiltinVoteType_KickScamming:
		translation = TRANSLATION_TF2_VOTE_KICK_SCAMMING_START;
		break;

	case BuiltinVoteType_KickCheating:
		translation = TRANSLATION_TF2_VOTE_KICK_CHEATING_START;
		break;

	case BuiltinVoteType_NextLevel:
		translation = TRANSLATION_TF2_VOTE_NEXTLEVEL_SINGLE_START;
		break;

	case BuiltinVoteType_NextLevelMult:
		translation = TRANSLATION_TF2_VOTE_NEXTLEVEL_MULTIPLE_START;
		bYesNo = false;
		break;

	case BuiltinVoteType_ScrambleNow:
		translation = TRANSLATION_TF2_VOTE_SCRAMBLE_IMMEDIATE_START;
		break;

	case BuiltinVoteType_ScrambleEnd:
		translation = TRANSLATION_TF2_VOTE_SCRAMBLE_ROUNDEND_START;
		break;

	case BuiltinVoteType_Custom_Mult:
		bYesNo = false;
	default:
		translation = TRANSLATION_TF2_VOTE_CUSTOM;
		break;

	}

	bf_write *bf = usermsgs->StartBitBufMessage(msgId, clients, num_clients, USERMSG_RELIABLE);
	bf->WriteByte(GetTeam()); // Automagically converted to -1 here.  Or at least it'd better be.
	bf->WriteByte(GetInitiator());
	bf->WriteString(translation);
	bf->WriteString(GetArgument());
	bf->WriteOneBit(bYesNo);
	usermsgs->EndMessage();

	m_bResultDisplayed = false;

	SH_ADD_HOOK(IServerGameClients, ClientCommand, servergameclients, SH_MEMBER(this, &CTF2BuiltinVote::OnClientCommand), false);

	return true;
}

void CTF2BuiltinVote::DisplayVotePass(const char* winner)
{
	const char *translation;

	switch (m_voteType)
	{
	
	case BuiltinVoteType_Kick:
	case BuiltinVoteType_KickIdle:
	case BuiltinVoteType_KickScamming:
	case BuiltinVoteType_KickCheating:
		translation = TRANSLATION_TF2_VOTE_KICK_PASSED;
		break;

	case BuiltinVoteType_Restart:
		translation = TRANSLATION_TF2_VOTE_RESTART_PASSED;
		break;

	case BuiltinVoteType_ChgLevel:
		translation = TRANSLATION_TF2_VOTE_CHANGELEVEL_PASSED;
		break;

	case BuiltinVoteType_NextLevel:
		translation = TRANSLATION_TF2_VOTE_NEXTLEVEL_PASSED;
		break;

	case BuiltinVoteType_NextLevelMult:
		if (stricmp(winner, TF2_EXTEND) == 0)
		{
			translation = TRANSLATION_TF2_VOTE_NEXTLEVEL_EXTEND_PASSED;
		} 
		else
		{
			translation = TRANSLATION_TF2_VOTE_NEXTLEVEL_PASSED;
		}
		break;

	case BuiltinVoteType_ScrambleNow:
	case BuiltinVoteType_ScrambleEnd:
		translation = TRANSLATION_TF2_VOTE_SCRAMBLE_PASSED;
		break;

	default:
		translation = TRANSLATION_TF2_VOTE_CUSTOM;
		break;

	}

	DisplayVotePass(translation, winner);
}


void CTF2BuiltinVote::DisplayVotePass(const char *translation, const char* winner)
{
	m_bResultDisplayed = true;
	SH_REMOVE_HOOK(IServerGameClients, ClientCommand, servergameclients, SH_MEMBER(this, &CTF2BuiltinVote::OnClientCommand), false);

	// Display the vote pass message
	cell_t clients[256+1];
	unsigned int playersNum = GetAllPlayers(clients);

	int msgId = usermsgs->GetMessageIndex("VotePass");

	bf_write *bf = usermsgs->StartBitBufMessage(msgId, clients, playersNum, USERMSG_RELIABLE);
	bf->WriteByte(GetTeam());
	bf->WriteString(translation);
	bf->WriteString(winner);
	usermsgs->EndMessage();

	m_bOptionsSent = false;
}

void CTF2BuiltinVote::DisplayVoteFail(BuiltinVoteFailReason reason)
{
	m_bResultDisplayed = true;
	SH_REMOVE_HOOK(IServerGameClients, ClientCommand, servergameclients, SH_MEMBER(this, &CTF2BuiltinVote::OnClientCommand), false);

	// Display the vote fail message
	cell_t clients[256+1];
	unsigned int playersNum = GetAllPlayers(clients);

	InternalDisplayVoteFail(clients, playersNum, reason);

	m_bOptionsSent = false;
}

void CTF2BuiltinVote::DisplayVoteFail(int client, BuiltinVoteFailReason reason)
{
	// Display the vote fail message
	cell_t clients[] = { client };

	InternalDisplayVoteFail(clients, 1, reason);
}

void CTF2BuiltinVote::InternalDisplayVoteFail(int clients[], unsigned int num_clients, BuiltinVoteFailReason reason)
{
	int msgId = usermsgs->GetMessageIndex("VoteFailed");

	bf_write *bf = usermsgs->StartBitBufMessage(msgId, clients, num_clients, USERMSG_RELIABLE);
	bf->WriteByte(GetTeam());
	bf->WriteByte(reason);
	usermsgs->EndMessage();
}

/*
void CTF2BuiltinVote::DisplayCallVoteFail(int client, BuiltinCallVoteFailReason reason, short param1)
{
	// Display the call vote failed message
	const cell_t clients[1] = { client };

	int msgId = usermsgs->GetMessageIndex("CallVoteFailed");

	bf_write *bf = usermsgs->StartBitBufMessage(msgId, clients, 1, USERMSG_RELIABLE);
	bf->WriteByte(reason);
	bf->WriteShort(param1);
	usermsgs->EndMessage();
}
*/

void CTF2BuiltinVote::ClientSelectedItem(int client, unsigned int item)
{
	if (m_voteType != BuiltinVoteType_Custom_Mult && m_voteType != BuiltinVoteType_NextLevelMult)
	{
		if (item == BUILTINVOTES_VOTE_NO)
		{
			item = TF2_VOTE_NO_INDEX;
		}
		else if (item == BUILTINVOTES_VOTE_YES)
		{
			item = TF2_VOTE_YES_INDEX;
		}
	}

	//Fire the vote_cast event
	IGameEvent *castEvent = events->CreateEvent("vote_cast");
	castEvent->SetInt("team", GetTeam());
	castEvent->SetInt("entityid", client);
	castEvent->SetInt("vote_option", item);
	events->FireEvent(castEvent);

	//CBaseBuiltinVote::ClientPressedKey(client, key_press);
	//s_VoteHandler.OnVoteSelect(this, client, item);
}

unsigned int CTF2BuiltinVote::GetApproxMemUsage()
{
	return sizeof(CTF2BuiltinVote) + GetBaseMemUsage();
}

bool CTF2BuiltinVote::UpdateVoteCounts(unsigned int items, CVector<unsigned int> votes, unsigned int totalClients)
{
	return false;
}

void CTF2BuiltinVote::OnClientCommand(edict_t *pEntity, const CCommand &cmd)
{
	int client = gamehelpers->IndexOfEdict(pEntity);
	const char *cmdname = cmd.Arg(0);

	if (strcmp(cmdname, "vote") == 0)
	{
		const char *option = cmd.Arg(1);

#ifdef _DEBUG
		if (client > 0)
		{
			IGamePlayer *pPlayer = playerhelpers->GetGamePlayer(client);
			const char *name = pPlayer->GetName();
			smutils->LogMessage(myself, "%s voted for %s", name, option);
		}
#endif
		if (strlen(option) == 7)
		{
			// The seventh character is the actual vote option in TF2 (option1 - option5)
			int arg = atoi(&(option[6]));
			//ClientPressedKey(client, arg);

			int item = arg - 1;

			// Swap Yes and No votes to match expectations
			if (m_voteType != BuiltinVoteType_Custom_Mult && m_voteType != BuiltinVoteType_NextLevelMult)
			{
				if (item == TF2_VOTE_YES_INDEX)
				{
					item = BUILTINVOTES_VOTE_YES;
				}
				else if (item == TF2_VOTE_NO_INDEX)
				{
					item = BUILTINVOTES_VOTE_NO;
				}
			}

			s_VoteHandler.OnVoteSelect(this, client, item);

			RETURN_META(MRES_SUPERCEDE);
		}
	}
}
