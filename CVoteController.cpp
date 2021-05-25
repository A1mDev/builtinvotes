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

#include "extension.h"
#include "CVoteController.h"

CBaseEntity* CVoteController::pEntity = nullptr;
int CVoteController::offset_m_activeIssueIndex = 0;

bool CVoteController::InitOffsets(char* error, size_t maxlength)
{
	sm_sendprop_info_t info;
	
	if (!gamehelpers->FindSendPropInfo("CVoteController", "m_activeIssueIndex", &info)) {
		snprintf(error, maxlength, "Unable to find SendProp \"CVoteController::m_activeIssueIndex\"");
		return false;
	}
	
	offset_m_activeIssueIndex = info.actual_offset;
		
	return true;
}

void CVoteController::InitVoteController()
{
	pEntity = FindEntityByClassname("vote_controller");
	
	if (pEntity == nullptr) {
		g_pSM->LogError(myself, "Couldn't find the vote_controller");
		rootconsole->ConsolePrint("Couldn't find the vote_controller");
	}
}

void CVoteController::ResetVoteController()
{
	pEntity = nullptr;
}

bool CVoteController::Game_IsVoteInProgress()
{
	if (pEntity == nullptr) {
		g_pSM->LogError(myself, "Native Game_IsVoteInProgress(). Couldn't find the vote_controller");
		rootconsole->ConsolePrint("Native Game_IsVoteInProgress(). Couldn't find the vote_controller");
		return false;
	}
	
	int m_activeIssueIndex = (*(int *)((byte *)pEntity + CVoteController::offset_m_activeIssueIndex));
	return (m_activeIssueIndex > INVALID_ISSUE);
}