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

#ifndef _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_
#define _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_

/**
 * @file extension.h
 * @brief Sample extension code header.
 */

#include "smsdk_ext.h"
#include <itoolentity.h>
#include <IGameHelpers.h>
#include <IBuiltinVoteManager.h>
#include <BuiltinVoteHandler.h>
#include "igameevents.h"
#include "natives.h"
#include "icvar.h"
#include "util.h"
#include "CVoteController.h"

/**
 * @brief Sample implementation of the SDK Extension.
 * Note: Uncomment one of the pre-defined virtual functions in order to use it.
 */
class BuiltinVoteManager : public SDKExtension,
	public IBuiltinVoteManager,
	public IHandleTypeDispatch,
	public IConCommandBaseAccessor
{
	friend class BuiltinVoteHandler;
	friend class CBaseBuiltinVote;
	friend class BaseBuiltinVoteStyle;

public:
	/**
	 * @brief This is called after the initial loading sequence has been processed.
	 *
	 * @param error		Error message buffer.
	 * @param maxlength	Size of error message buffer.
	 * @param late		Whether or not the module was loaded after map load.
	 * @return			True to succeed loading, false to fail.
	 */
	virtual bool SDK_OnLoad(char *error, size_t maxlength, bool late);
	
	/**
	 * @brief This is called right before the extension is unloaded.
	 */
	virtual void SDK_OnUnload();

	/**
	 * @brief This is called once all known extensions have been loaded.
	 * Note: It is is a good idea to add natives here, if any are provided.
	 */
	virtual void SDK_OnAllLoaded();

	/**
	 * @brief Called when the pause state is changed.
	 */
	//virtual void SDK_OnPauseChange(bool paused);

	/**
	 * @brief this is called when Core wants to know if your extension is working.
	 *
	 * @param error		Error message buffer.
	 * @param maxlength	Size of error message buffer.
	 * @return			True if working, false otherwise.
	 */
	//virtual bool QueryRunning(char *error, size_t maxlength);

public:
#if defined SMEXT_CONF_METAMOD
	/**
	 * @brief Called when Metamod is attached, before the extension version is called.
	 *
	 * @param error			Error buffer.
	 * @param maxlength		Maximum size of error buffer.
	 * @param late			Whether or not Metamod considers this a late load.
	 * @return				True to succeed, false to fail.
	 */
	virtual bool SDK_OnMetamodLoad(ISmmAPI *ismm, char *error, size_t maxlength, bool late);

	/**
	 * @brief Called when Metamod is detaching, after the extension version is called.
	 * NOTE: By default this is blocked unless sent from SourceMod.
	 *
	 * @param error			Error buffer.
	 * @param maxlength		Maximum size of error buffer.
	 * @return				True to succeed, false to fail.
	 */
	//virtual bool SDK_OnMetamodUnload(char *error, size_t maxlength);

	/**
	 * @brief Called when Metamod's pause state is changing.
	 * NOTE: By default this is blocked unless sent from SourceMod.
	 *
	 * @param paused		Pause state being set.
	 * @param error			Error buffer.
	 * @param maxlength		Maximum size of error buffer.
	 * @return				True to succeed, false to fail.
	 */
	//virtual bool SDK_OnMetamodPauseChange(bool paused, char *error, size_t maxlength);

	/**
	 * @brief Called on server activation before plugins receive the OnServerLoad forward. 
	 *
	 * @param pEdictList	Edicts list.
	 * @param edictCount	Number of edicts in the list.
	 * @param clientMax		Maximum number of clients allowed in the server.
	 */
	virtual void OnCoreMapStart(edict_t *pEdictList, int edictCount, int clientMax);
	
	/**
	 * @brief Called on level shutdown
	 *
	 */
	virtual void OnCoreMapEnd();
#endif

public:  // IConCommandBaseAccessor
	virtual bool RegisterConCommandBase(ConCommandBase *pVar);

public: //IBuiltinVoteManager
	virtual const char *GetInterfaceName()
	{
		return SMINTERFACE_BUILTINVOTES_NAME;
	}
	virtual unsigned int GetInterfaceVersion()
	{
		return SMINTERFACE_BUILTINVOTES_VERSION;
	}
public:
	IBuiltinVoteStyle *GetStyle();
	bool StartVote(IBaseBuiltinVote *vote,
		unsigned int num_clients,
		int clients[],
		unsigned int max_time,
		unsigned int flags=0);
		bool IsVoteInProgress();
	void CancelVoting();
	unsigned int GetRemainingVoteDelay();
	bool IsClientInVotePool(int client);
	bool RedrawClientVote(int client);
	bool RedrawClientVote2(int client, bool revote);
public: //IHandleTypeDispatch
	void OnHandleDestroy(HandleType_t type, void *object);
	bool GetHandleApproxSize(HandleType_t type, void *object, unsigned int *pSize);
public:
	HandleError ReadVoteHandle(Handle_t handle, IBaseBuiltinVote **vote);
protected:
	Handle_t CreateVoteHandle(IBaseBuiltinVote *vote, IdentityToken_t *pOwner);
	Handle_t CreateStyleHandle(IBuiltinVoteStyle *style);
private:
	HandleType_t m_StyleType;
	HandleType_t m_VoteType;
	IBuiltinVoteStyle *m_VoteStyle;

};

extern BuiltinVoteManager g_BuiltinVotes;
extern CGlobalVars *gpGlobals;
extern BuiltinVoteHandler s_VoteHandler;
//extern IBuiltinVoteStyle *m_VoteStyle;
extern ICvar *icvar;
extern IServerTools *servertools;
extern IServerGameEnts *gameents;

#endif //_INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_
