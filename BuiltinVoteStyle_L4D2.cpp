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

#include "BuiltinVoteStyle_L4D2.h"

/* This will error out if you try to compile for lower source engines.  Since this class should only be included
 * for recent engines, I don't see that as a problem
 */ 
SH_DECL_HOOK2_void(IServerGameClients, ClientCommand, SH_NOATTRIB, 0, edict_t *, const CCommand &);

L4D2BuiltinVoteStyle g_L4D2BuiltinVoteStyle;

ConVar *sv_alltalk;

/*
void L4D2BuiltinVoteStyle::SendDisplay(int clients[], unsigned int num_clients, IBaseBuiltinVote *vote)
{
	vote->Display(clients, num_clients);
}
*/

L4D2BuiltinVoteStyle::L4D2BuiltinVoteStyle() : L4DBaseBuiltinVoteStyle()
{
}

bool L4D2BuiltinVoteStyle::CheckVoteType(BuiltinVoteType type)
{
	switch(type)
	{
	case BuiltinVoteType_ChgCampaign:
	case BuiltinVoteType_ReturnToLobby:
	case BuiltinVoteType_ChgDifficulty:
	case BuiltinVoteType_Custom_YesNo:
	case BuiltinVoteType_Kick:
	case BuiltinVoteType_Restart:
	case BuiltinVoteType_ChgLevel:
	case BuiltinVoteType_Alltalk:
		return true;
		break;

	default:
		return false;
		break;
	}
}

IBaseBuiltinVote *L4D2BuiltinVoteStyle::CreateVote(IBuiltinVoteHandler *handler, BuiltinVoteType type, IdentityToken_t *pOwner)
{
	if (CheckVoteType(type))
	{
		if (sv_alltalk == NULL)
		{
			sv_alltalk = icvar->FindVar("sv_alltalk");
		}

		return new CL4D2BuiltinVote(handler, this, type, pOwner);
	}
	else
	{
		return NULL;
	}
}

const char *L4D2BuiltinVoteStyle::GetStyleName()
{
	return "l4d2";
}

unsigned int L4D2BuiltinVoteStyle::GetApproxMemUsage()
{
	return sizeof(L4D2BuiltinVoteStyle) + (sizeof(CBaseBuiltinVotePlayer) * 257);
}

CL4D2BuiltinVote::CL4D2BuiltinVote(IBuiltinVoteHandler *pHandler, IBuiltinVoteStyle *pStyle, BuiltinVoteType type, IdentityToken_t *pOwner) :
CL4DBaseBuiltinVote(pHandler, pStyle, type, pOwner)
{
	if (type == BuiltinVoteType_Alltalk)
	{
		if (sv_alltalk->GetBool())
		{
			SetArgument(TRANSLATION_L4D_VOTE_ALLTALK_DISABLE);
		}
		else
		{
			SetArgument(TRANSLATION_L4D_VOTE_ALLTALK_ENABLE);
		}
	}
}

CL4D2BuiltinVote::~CL4D2BuiltinVote()
{
	if (!m_bResultDisplayed)
	{
		SH_REMOVE_HOOK(IServerGameClients, ClientCommand, servergameclients, SH_MEMBER(this, &CL4D2BuiltinVote::OnClientCommand), false);
	}
}

bool CL4D2BuiltinVote::Display(int clients[], unsigned int num_clients)
{
	if (m_bCancelling)
	{
		return false;
	}

	const char *translation;

	switch(m_voteType)
	{
	case BuiltinVoteType_ChgCampaign:
		translation = TRANSLATION_L4D_VOTE_CHANGECAMPAIGN_START;
		break;

	case BuiltinVoteType_ReturnToLobby:
		translation = TRANSLATION_L4D_VOTE_RETURNTOLOBBY_START;
		break;

	case BuiltinVoteType_ChgDifficulty:
		translation = TRANSLATION_L4D_VOTE_CHANGEDIFFICULTY_START;
		break;

	case BuiltinVoteType_Kick:
		translation = TRANSLATION_L4D_VOTE_KICK_START;
		break;

	case BuiltinVoteType_Restart:
		translation = TRANSLATION_L4D_VOTE_RESTART_START;
		break;

	case BuiltinVoteType_ChgLevel:
		translation = TRANSLATION_L4D_VOTE_CHANGELEVEL_START;
		break;

	case BuiltinVoteType_Alltalk:
		translation = TRANSLATION_L4D_VOTE_ALLTALK_START;
		break;

	default:
		translation = TRANSLATION_L4D_VOTE_CUSTOM;
		break;

	}

	int msgId = usermsgs->GetMessageIndex("VoteStart");
	bf_write *bf = usermsgs->StartBitBufMessage(msgId, clients, num_clients, USERMSG_RELIABLE);
	bf->WriteByte(m_team);
	bf->WriteByte(m_initiator);
	bf->WriteString(translation);
	bf->WriteString(m_Argument.c_str());
	if (m_initiator != SERVER_CLIENT_INDEX)
	{
		IGamePlayer *player = playerhelpers->GetGamePlayer(m_initiator);
		if (player != NULL)
		{
			bf->WriteString(player->GetName());
		} else {
			bf->WriteString("");
		}
	} else {
		bf->WriteString("");
	}
	usermsgs->EndMessage();
	
	m_bResultDisplayed = false;
	
	SH_ADD_HOOK(IServerGameClients, ClientCommand, servergameclients, SH_MEMBER(this, &CL4D2BuiltinVote::OnClientCommand), false);

	return true;
}

unsigned int CL4D2BuiltinVote::GetApproxMemUsage()
{
	return sizeof(CL4D2BuiltinVote) + GetBaseMemUsage();
}

void CL4D2BuiltinVote::Cancel_Finally()
{
	g_L4D2BuiltinVoteStyle.CancelVote(this);
}

void CL4D2BuiltinVote::DisplayVotePass(const char *winner)
{
	const char *translation;

	switch(m_voteType)
	{
	case BuiltinVoteType_ChgCampaign:
		translation = TRANSLATION_L4D_VOTE_CHANGECAMPAIGN_PASSED;
		break;

	case BuiltinVoteType_ReturnToLobby:
		translation = TRANSLATION_L4D_VOTE_RETURNTOLOBBY_PASSED;
		break;

	case BuiltinVoteType_ChgDifficulty:
		translation = TRANSLATION_L4D_VOTE_CHANGEDIFFICULTY_PASSED;
		break;

	case BuiltinVoteType_Kick:
		translation = TRANSLATION_L4D_VOTE_KICK_PASSED;
		break;

	case BuiltinVoteType_Restart:
		translation = TRANSLATION_L4D_VOTE_RESTART_PASSED;
		break;

	case BuiltinVoteType_ChgLevel:
		translation = TRANSLATION_L4D_VOTE_CHANGELEVEL_PASSED;
		break;

	case BuiltinVoteType_Alltalk:
		translation = TRANSLATION_L4D_VOTE_ALLTALK_PASSED;
		winner = GetArgument();
		break;

	default:
		translation = TRANSLATION_L4D_VOTE_CUSTOM;
		break;

	}

	DisplayVotePass(translation, winner);
}

void CL4D2BuiltinVote::DisplayVotePass(const char *translation, const char *winner)
{
	m_bResultDisplayed = true;

	SH_REMOVE_HOOK(IServerGameClients, ClientCommand, servergameclients, SH_MEMBER(this, &CL4D2BuiltinVote::OnClientCommand), false);

	cell_t clients[256+1];
	unsigned int playersNum = GetAllPlayers(clients);

	int msgId = usermsgs->GetMessageIndex("VotePass");

	bf_write *bf = usermsgs->StartBitBufMessage(msgId, clients, playersNum, USERMSG_RELIABLE);
	bf->WriteByte(m_team);
	bf->WriteString(translation);
	bf->WriteString(winner);
	usermsgs->EndMessage();
}

void CL4D2BuiltinVote::DisplayVoteFail(BuiltinVoteFailReason reason)
{
	cell_t clients[256+1];
	unsigned int playersNum = GetAllPlayers(clients);

	InternalDisplayVoteFail(clients, playersNum, reason);
}

void CL4D2BuiltinVote::DisplayVoteFail(int client, BuiltinVoteFailReason reason)
{
	cell_t clients[] = { client };

	InternalDisplayVoteFail(clients, 1, reason);
}

void CL4D2BuiltinVote::InternalDisplayVoteFail(int clients[], unsigned int num_clients, BuiltinVoteFailReason reason)
{
	m_bResultDisplayed = true;

	SH_REMOVE_HOOK(IServerGameClients, ClientCommand, servergameclients, SH_MEMBER(this, &CL4D2BuiltinVote::OnClientCommand), false);

	int msgId = usermsgs->GetMessageIndex("VoteFail");

	bf_write *bf = usermsgs->StartBitBufMessage(msgId, clients, num_clients, USERMSG_RELIABLE);
	bf->WriteByte(m_team);
	usermsgs->EndMessage();
}

void CL4D2BuiltinVote::ClientSelectedItem(int client, unsigned int item)
{
	//CBaseBuiltinVote::ClientSelectedItem(client, item);

	int msgId = usermsgs->GetMessageIndex("VoteRegistered");

	const cell_t players[] = { client };

	bf_write *bf = usermsgs->StartBitBufMessage(msgId, players, 1, USERMSG_RELIABLE);
	bf->WriteByte(item);
	usermsgs->EndMessage();
}
