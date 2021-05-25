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
#include "compat_wrappers.h"

#if SOURCE_ENGINE == SE_ORANGEBOXVALVE
#include "BuiltinVoteStyle_TF2.h"
#elif SOURCE_ENGINE == SE_LEFT4DEAD
#include "BuiltinVoteStyle_L4D1.h"
#elif SOURCE_ENGINE == SE_LEFT4DEAD2
#include "BuiltinVoteStyle_L4D2.h"
#endif

/**
 * @file extension.cpp
 * @brief Implement extension code here.
 */

BuiltinVoteManager g_BuiltinVotes;		/**< Global singleton for extension's main interface */

SMEXT_LINK(&g_BuiltinVotes);

BuiltinVoteHandler s_VoteHandler;
CGlobalVars *gpGlobals;
ICvar *icvar;
IPhraseCollection *corePhrases;
IServerGameClients *servergameclients;
IGameEventManager2 *events;
IServerTools *servertools = nullptr;
IServerGameEnts *gameents = nullptr;

bool BuiltinVoteManager::RegisterConCommandBase(ConCommandBase *pVar)
{
	/* Always call META_REGCVAR instead of going through the engine. */
	return META_REGCVAR(pVar);
}

bool BuiltinVoteManager::SDK_OnMetamodLoad(ISmmAPI *ismm, char *error, size_t maxlen, bool late)
{
	GET_V_IFACE_CURRENT(GetEngineFactory, icvar, ICvar, CVAR_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetEngineFactory, events, IGameEventManager2, INTERFACEVERSION_GAMEEVENTSMANAGER2);
	GET_V_IFACE_ANY(GetServerFactory, servergameclients, IServerGameClients, INTERFACEVERSION_SERVERGAMECLIENTS);
	GET_V_IFACE_ANY(GetServerFactory, servertools, IServerTools, VSERVERTOOLS_INTERFACE_VERSION);
	GET_V_IFACE_ANY(GetServerFactory, gameents, IServerGameEnts, INTERFACEVERSION_SERVERGAMEENTS);	
	
	gpGlobals = ismm->GetCGlobals();
	g_pCVar = icvar;
	CONVAR_REGISTER(this);
	
	return true;
}

bool BuiltinVoteManager::SDK_OnLoad(char *error, size_t maxlength, bool late)
{
	if (!CVoteController::InitOffsets(error, maxlength)) {
		return false;
	}
	
	/* Yes, we can be accessed by other extensions in this version. \o/
	 * Whether it works or not has yet to be tested. */
	sharesys->AddInterface(myself, &g_BuiltinVotes);

	HandleAccess access;
	handlesys->InitAccessDefaults(NULL, &access);

	/* Deny cloning to votes */
	access.access[HandleAccess_Clone] = HANDLE_RESTRICT_OWNER|HANDLE_RESTRICT_IDENTITY;
	m_VoteType = handlesys->CreateType("IBaseBuiltinVote", this, 0, NULL, &access, myself->GetIdentity(), NULL);
	
	/* Also deny deletion to styles */
	access.access[HandleAccess_Delete] = HANDLE_RESTRICT_OWNER|HANDLE_RESTRICT_IDENTITY;
	m_StyleType = handlesys->CreateType("IBuiltinVoteStyle", this, 0, NULL, &access, myself->GetIdentity(), NULL);

	// Turns out we need some translation phrases from core.  This is shared with children via extern.
	corePhrases = translator->CreatePhraseCollection();
	corePhrases->AddPhraseFile("core.phrases");

#if SOURCE_ENGINE == SE_ORANGEBOXVALVE
	// TF2 is the only Orange Box game supported
	m_VoteStyle = &g_TF2BuiltinVoteStyle;
#elif SOURCE_ENGINE == SE_LEFT4DEAD
	m_VoteStyle = &g_L4D1BuiltinVoteStyle;
#elif SOURCE_ENGINE == SE_LEFT4DEAD2
	m_VoteStyle = &g_L4D2BuiltinVoteStyle;
#endif

	// Initialize the global vote handler
	s_VoteHandler.OnLoad();

	// Doesn't currently do anything, kept for future use
	g_VoteHelpers.OnLoad();

	sharesys->RegisterLibrary(myself, "builtinvotes");
	return true;
}

void BuiltinVoteManager::SDK_OnAllLoaded()
{
	// Load natives
	sharesys->AddNatives(myself, g_Natives);
}

void BuiltinVoteManager::SDK_OnUnload()
{
	// Shut down the global vote handler
	s_VoteHandler.OnUnload();

	// Shut down all the individual vote handlers
	g_VoteHelpers.OnUnload();

	// Destroy the phrases collection
	corePhrases->Destroy();

	handlesys->RemoveType(m_VoteType, myself->GetIdentity());
}

void BuiltinVoteManager::OnCoreMapStart(edict_t *pEdictList, int edictCount, int clientMax)
{
	// Tell the vote handler we're changing levels
	s_VoteHandler.OnMapStart();
	
	//find the created vote_controller
	CVoteController::InitVoteController();
}

void BuiltinVoteManager::OnCoreMapEnd()
{
	//I don't know at what point this class will be destroyed (vote_controller), let's do so
	CVoteController::ResetVoteController();
}

void BuiltinVoteManager::OnHandleDestroy(HandleType_t type, void *object)
{
	if (type == m_VoteType) {
		if (IsVoteInProgress()) {
			CancelVoting();
			META_CONPRINT("[BuiltinVotes] Vote handle destroyed - vote cancel!\n");
		}

		IBaseBuiltinVote *vote = (IBaseBuiltinVote *)object;
		vote->Destroy(false);
	} else if (type == m_StyleType) {
		/* Do nothing */
	}
}

bool BuiltinVoteManager::GetHandleApproxSize(HandleType_t type, void *object, unsigned int *pSize)
{
	if (type == m_VoteType)
	{
		*pSize = ((IBaseBuiltinVote *)object)->GetApproxMemUsage();
		return true;
	}
	else
	{
		*pSize = ((IBuiltinVoteStyle *)object)->GetApproxMemUsage();
		return true;
	}

	return false;
}

Handle_t BuiltinVoteManager::CreateVoteHandle(IBaseBuiltinVote *vote, IdentityToken_t *pOwner)
{
	if (m_VoteType == NO_HANDLE_TYPE)
	{
		return BAD_HANDLE;
	}

	return handlesys->CreateHandle(m_VoteType, vote, pOwner, myself->GetIdentity(), NULL);
}

Handle_t BuiltinVoteManager::CreateStyleHandle(IBuiltinVoteStyle *style)
{
	if (m_StyleType == NO_HANDLE_TYPE)
	{
		return BAD_HANDLE;
	}

	return handlesys->CreateHandle(m_StyleType, style, myself->GetIdentity(), myself->GetIdentity(), NULL);
}

HandleError BuiltinVoteManager::ReadVoteHandle(Handle_t handle, IBaseBuiltinVote **vote)
{
	HandleSecurity sec;

	sec.pIdentity = myself->GetIdentity();
	sec.pOwner = myself->GetIdentity();

	return handlesys->ReadHandle(handle, m_VoteType, &sec, (void **)vote);
}

bool BuiltinVoteManager::StartVote(IBaseBuiltinVote *vote, unsigned int num_clients, int clients[], unsigned int max_time, unsigned int flags)
{
	return s_VoteHandler.StartVote(vote, num_clients, clients, max_time, flags);
}

bool BuiltinVoteManager::IsVoteInProgress()
{
	return s_VoteHandler.IsVoteInProgress();
}

void BuiltinVoteManager::CancelVoting()
{
	s_VoteHandler.CancelVoting();
}

unsigned int BuiltinVoteManager::GetRemainingVoteDelay()
{
	return s_VoteHandler.GetRemainingVoteDelay();
}

bool BuiltinVoteManager::IsClientInVotePool(int client)
{
	return s_VoteHandler.IsClientInVotePool(client);
}

bool BuiltinVoteManager::RedrawClientVote(int client)
{
	return RedrawClientVote2(client, true);
}

bool BuiltinVoteManager::RedrawClientVote2(int client, bool revote)
{
	return s_VoteHandler.RedrawToClient(client, revote);
}

IBuiltinVoteStyle *BuiltinVoteManager::GetStyle()
{
	return m_VoteStyle;
}
