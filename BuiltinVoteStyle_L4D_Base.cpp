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

#include "BuiltinVoteStyle_L4D_Base.h"

//L4DBuiltinVoteStyle g_L4DBuiltinVoteStyle;

unsigned int L4DBaseBuiltinVoteStyle::GetMaxItems()
{
	return 2;
}


CL4DBaseBuiltinVote::CL4DBaseBuiltinVote(IBuiltinVoteHandler *pHandler, IBuiltinVoteStyle *pStyle, BuiltinVoteType type, IdentityToken_t *pOwner) :
CBaseBuiltinVote(pHandler, pStyle, type, pOwner)
{
}

bool CL4DBaseBuiltinVote::UpdateVoteCounts(unsigned int items, CVector<unsigned int> votes, unsigned int totalClients)
{
	IGameEvent *changeEvent = events->CreateEvent("vote_changed");
	changeEvent->SetInt("yesVotes", votes.at(BUILTINVOTES_VOTE_YES));
	changeEvent->SetInt("noVotes", votes.at(BUILTINVOTES_VOTE_NO));
	changeEvent->SetInt("potentialVotes", totalClients);
	events->FireEvent(changeEvent);

	return true;
}

void CL4DBaseBuiltinVote::OnClientCommand(edict_t *pEntity, const CCommand &cmd)
{
	int client = gamehelpers->IndexOfEdict(pEntity);
	const char *cmdname = cmd.Arg(0);

	if (strcmp(cmdname, "Vote") == 0)
	{
		const char *voteString = cmd.Arg(1);
		int key_press;

		if (strcmp(voteString, "Yes") == 0)
		{
			key_press = BUILTINVOTES_VOTE_YES;
		} else {
			key_press = BUILTINVOTES_VOTE_NO;
		}

		s_VoteHandler.OnVoteSelect(this, client, key_press);
		//ClientPressedKey(client, key_press);

		RETURN_META(MRES_SUPERCEDE);
	}
}
