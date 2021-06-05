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

#include "BuiltinVoteStyle_L4D1.h"

/* This will error out if you try to compile for lower source engines.  Since this class should only be included
 * for recent engines, I don't see that as a problem
 */ 
SH_DECL_HOOK2_void(IServerGameClients, ClientCommand, SH_NOATTRIB, 0, edict_t *, const CCommand &);

L4D1BuiltinVoteStyle g_L4D1BuiltinVoteStyle;

/*
void L4D1BuiltinVoteStyle::SendDisplay(int clients[], unsigned int num_clients, IBaseBuiltinVote *vote)
{
	vote->Display(clients, num_clients);
}
*/

bool L4D1BuiltinVoteStyle::CheckVoteType(BuiltinVoteType type)
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
		return true;
		break;

	default:
		return false;
		break;
	}
}

IBaseBuiltinVote *L4D1BuiltinVoteStyle::CreateVote(IBuiltinVoteHandler *handler, BuiltinVoteType type, IdentityToken_t *pOwner)
{
	if (CheckVoteType(type))
	{
		return new CL4D1BuiltinVote(handler, this, type, pOwner);
	}
	else
	{
		return NULL;
	}
}

const char *L4D1BuiltinVoteStyle::GetStyleName()
{
	return "l4d1";
}

unsigned int L4D1BuiltinVoteStyle::GetApproxMemUsage()
{
	return sizeof(L4D1BuiltinVoteStyle) + (sizeof(CBaseBuiltinVotePlayer) * 257);
}

CL4D1BuiltinVote::CL4D1BuiltinVote(IBuiltinVoteHandler *pHandler, IBuiltinVoteStyle *pStyle, BuiltinVoteType type, IdentityToken_t *pOwner) :
CL4DBaseBuiltinVote(pHandler, pStyle, type, pOwner)
{
}

CL4D1BuiltinVote::~CL4D1BuiltinVote()
{
	if (!m_bResultDisplayed)
	{
		SH_REMOVE_HOOK(IServerGameClients, ClientCommand, servergameclients, SH_MEMBER(this, &CL4D1BuiltinVote::OnClientCommand), false);
	}
}

bool CL4D1BuiltinVote::Display(int clients[], unsigned int num_clients)
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

	default:
		translation = TRANSLATION_L4D_VOTE_CUSTOM;
		break;

	}


	IGameEvent *startEvent = events->CreateEvent("vote_started");
	startEvent->SetInt("team", m_team);
	startEvent->SetInt("initiator", m_initiator);
	startEvent->SetString("issue", translation);
	startEvent->SetString("param1", GetArgument());

	events->FireEvent(startEvent);
	
	m_bResultDisplayed = false;
	
	SH_ADD_HOOK(IServerGameClients, ClientCommand, servergameclients, SH_MEMBER(this, &CL4D1BuiltinVote::OnClientCommand), false);

	return true;
}

unsigned int CL4D1BuiltinVote::GetApproxMemUsage()
{
	return sizeof(CL4D1BuiltinVote) + GetBaseMemUsage();
}

void CL4D1BuiltinVote::Cancel_Finally()
{
	g_L4D1BuiltinVoteStyle.CancelVote(this);
}

void CL4D1BuiltinVote::VoteEnded()
{
	IGameEvent *endEvent = events->CreateEvent("vote_ended");
	events->FireEvent(endEvent);
}

void CL4D1BuiltinVote::DisplayVotePass(const char *winner)
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

	default:
		translation = TRANSLATION_L4D_VOTE_CUSTOM;
		break;

	}

	DisplayVotePass(translation, winner);
}

void CL4D1BuiltinVote::DisplayVotePass(const char *translation, const char* winner)
{
	m_bResultDisplayed = true;
	
	SH_REMOVE_HOOK(IServerGameClients, ClientCommand, servergameclients, SH_MEMBER(this, &CL4D1BuiltinVote::OnClientCommand), false);

	VoteEnded();

	IGameEvent *passEvent = events->CreateEvent("vote_passed");
	passEvent->SetString("details", translation);
	passEvent->SetString("param1", winner);
	passEvent->SetInt("team", m_team);
	events->FireEvent(passEvent);
}

void CL4D1BuiltinVote::DisplayVoteFail(BuiltinVoteFailReason reason)
{
	m_bResultDisplayed = true;
	
	SH_REMOVE_HOOK(IServerGameClients, ClientCommand, servergameclients, SH_MEMBER(this, &CL4D1BuiltinVote::OnClientCommand), false);

	VoteEnded();
	
	IGameEvent *failEvent = events->CreateEvent("vote_failed");
	failEvent->SetInt("team", m_team);
	events->FireEvent(failEvent);

}

void CL4D1BuiltinVote::DisplayVoteFail(int client, BuiltinVoteFailReason reason)
{
	//NoOp on L4D
	return;
}

void CL4D1BuiltinVote::ClientSelectedItem(int client, unsigned int item)
{
	//CBaseBuiltinVote::ClientSelectedItem(client, item);

	if (item <= BUILTINVOTES_VOTE_YES)
	{
		IGameEvent *castEvent;
		switch (item)
		{
		case BUILTINVOTES_VOTE_NO:
			castEvent = events->CreateEvent("vote_cast_no");
			break;

		case BUILTINVOTES_VOTE_YES:
			castEvent = events->CreateEvent("vote_cast_yes");
			break;

		}

		if (castEvent != NULL)
		{
			castEvent->SetInt("team", m_team);
			castEvent->SetInt("entityid", m_initiator);
			events->FireEvent(castEvent);
		}

	}
}
