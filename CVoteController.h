/**
 * vim: set ts=4 :
 * =============================================================================
 * Builtin Votes
 * Copyright (C) 2021 A1m` (A1mDev).  All rights reserved.
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
 
#ifndef _INCLUDE_BUILTINVOTES_CVOTECONTROLLER_H_
#define _INCLUDE_BUILTINVOTES_CVOTECONTROLLER_H_

class CVoteController
{
public:
	/*	
	* @brief		Get the desired offset to work with the vote_controller
	*
	* @return		true if managed to get an offset, false otherwise
	*/
	static bool GetVoteControllerOffsets(char* error, size_t maxlength);
	
	/*	
	* @brief		Lookup for a vote_controller by class name, and save its CBaseHandle.
	*
	* @return		pointer to vote_controller, or NULL if not found
	*/
	static CVoteController* FindVoteController();
	
	/*	
	* @brief		Returns the already found pointer to vote_controller, or trying to lookup it again
	*
	* @returns		pointer to vote_controller, or NULL
	*/
	static CVoteController* GetVoteController();
	
	/*	
	* @brief		Check if there is a game vote in progress
	*
	* @returns		true if game vote in progress, false otherwise
	*/
	static bool Game_IsVoteInProgress();
	
#if SOURCE_ENGINE == SE_LEFT4DEAD2	
	/*	
	* @brief		Returns the number of the team in which the vote is progress
	* @remarks		Team number: 1 - spectators, 2 - survivors, 3 - infected
	* @remarks		It hardly works for a team of spectators
	*
	* @returns		-1 if no vote in progress or vote is for everyone, 
	*					otherwise the command number
	*/
	// L4D2 only.
	static int Game_GetVoteTeam();
#endif

private:
	static CBaseHandle s_hVoteController;
	
	static int offset_m_activeIssueIndex;
	
#if SOURCE_ENGINE == SE_LEFT4DEAD2
	static int offset_m_onlyTeamToVote;
#endif
};

#endif //_INCLUDE_BUILTINVOTES_CVOTECONTROLLER_H_