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

#ifndef _INCLUDE_BUILTINVOTEHANDLER_H
#define _INCLUDE_BUILTINVOTEHANDLER_H

#include <sh_vector.h>
#include "icvar.h"
#include "convar.h"
#include <IBuiltinVoteManager.h>
#include "smsdk_ext.h"

// We used to need these because we cannot access smsdk_ext directly
// #include <ITranslator.h>
// #include <IPlayerHelpers.h>
// #include <IGameHelpers.h>
// #include <ITimerSystem.h>

using namespace SourceHook;
using namespace SourceMod;

class RedrawTimer :
	public ITimedEvent
{
public:
	RedrawTimer(int client, IBaseBuiltinVote *vote);
	//~RedrawTimer();
public: //ITimedEvent
	ResultType OnTimer(ITimer *pTimer, void *pData);
	void OnTimerEnd(ITimer *pTimer, void *pData);
private:
	int m_client;
	IBaseBuiltinVote *m_pVote;
};

class BuiltinVoteHandler :
	public IBuiltinVoteHandler,
	public IClientListener,
	public ITimedEvent
{
public: //SMGlobalClass (faked from extension.cpp)
	void OnLoad();
	void OnUnload();
	void OnMapStart();
public: //IClientListener
	void OnClientDisconnected(int client);
public: //IBuiltinVoteHandler
	unsigned int GetBuiltinVoteAPIVersion();
	//void OnVoteStart(IBaseBuiltinVote *vote);
	//void OnVoteDisplay(IBaseBuiltinVote *vote, int client);
	void OnVoteSelect(IBaseBuiltinVote *vote, int client, unsigned int item);
	//void OnVoteEnd(IBaseBuiltinVote *vote, BuiltinVoteEndReason reason);
public: //ITimedEvent
	ResultType OnTimer(ITimer *pTimer, void *pData);
	void OnTimerEnd(ITimer *pTimer, void *pData);
public:
	bool StartVote(IBaseBuiltinVote *vote,
		unsigned int num_clients,
		int clients[],
		unsigned int max_time,
		unsigned int flags=0);
	bool IsVoteInProgress();
	void CancelVoting();
	IBaseBuiltinVote *GetCurrentVote();
	bool IsCancelling();
	bool WasCancelled();
	unsigned int GetRemainingVoteDelay();
	bool IsClientInVotePool(int client);
	bool GetClientVoteChoice(int client, unsigned int *pItem);
	bool RedrawToClient(int client, bool revote);
private:
	void Reset(IBuiltinVoteHandler *vh);
	void DecrementPlayerCount();
	void EndVoting();
	void InternalReset();
	bool InitializeVoting(IBaseBuiltinVote *vote,
		IBuiltinVoteHandler *handler,
		unsigned int time,
		unsigned int flags);
	void StartVoting();
	void DrawHintProgress();
	void BuildVoteLeaders();
private:
	IBuiltinVoteHandler *m_pHandler;
	unsigned int m_Clients;
	unsigned int m_TotalClients;
	unsigned int m_Items;
	CVector<unsigned int> m_Votes;
	IBaseBuiltinVote *m_pCurVote;
	bool m_bStarted;
	bool m_bCancelled;
	bool m_bWasCancelled;
	unsigned int m_NumVotes;
	unsigned int m_VoteTime;
	unsigned int m_VoteFlags;
	float m_fStartTime;
	unsigned int m_nVoteTime;
	unsigned int m_TimeLeft;
	int m_ClientVotes[256+1];
	bool m_Revoting[256+1];
	char m_leaderList[1024];
	ITimer *m_displayTimer;
};


// These are hacks since you can't have multiple entry points in an extension like SourceMod does (with its SMGlobalClass)

// Defined in smsdk_ext.cpp
//extern IPlayerManager *playerhelpers;
//extern ITimerSystem *timersys;
//extern IGameHelpers *gamehelpers;
//extern IVEngineServer *engine;
//extern ITranslator *translator;

// Defined in extension.cpp
extern CGlobalVars *gpGlobals;
extern ICvar *icvar;
extern IPhraseCollection *corePhrases;
extern BuiltinVoteHandler s_VoteHandler;

#endif //_INCLUDE_BUILTINVOTEHANDLER_H
