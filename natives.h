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

#ifndef _INCLUDE_SOURCEMOD_NATIVES_PROPER_H_
#define _INCLUDE_SOURCEMOD_NATIVES_PROPER_H_

#include "extension.h"
#include <sh_stack.h>

enum BuiltinVoteAction
{
	BuiltinVoteAction_Start = (1<<0),		/**< A vote display/sequence has started */
	BuiltinVoteAction_End = (1<<1),			/**< A vote display has finished. */
	BuiltinVoteAction_Select = (1<<2),		/**< An item was selected (param1=client, param2=item) */
	BuiltinVoteAction_Cancel = (1<<3),		/**< A vote sequence has been cancelled (nothing passed) */
	BuiltinVoteAction_VoteEnd = (1<<4),		/**< A vote sequence has ended (param1=chosen item). */
};

class CBuiltinVoteHandler;

class VoteNativeHelpers
{
public:
	virtual void OnLoad();
	virtual void OnUnload();
	CBuiltinVoteHandler *GetVoteHandler(IPluginFunction *pFunction, int flags);
	void FreeVoteHandler(CBuiltinVoteHandler *handler);
private:
	CStack<CBuiltinVoteHandler *> m_FreeVoteHandlers;
};

class CBuiltinVoteHandler : public IBuiltinVoteHandler
{
	friend class VoteNativeHelpers;
public:
	CBuiltinVoteHandler(IPluginFunction *pBasic, int flags);
public:
	void OnVoteStart(IBaseBuiltinVote *vote);
	//void OnVoteDisplay(IBaseBuiltinVote *vote, int client);
	void OnVoteSelect(IBaseBuiltinVote *vote, int client, unsigned int item);
	void OnVoteEnd(IBaseBuiltinVote *vote, BuiltinVoteEndReason reason);
	void OnVoteDestroy(IBaseBuiltinVote *vote);
	//void OnVoteVoteStart(IBaseBuiltinVote *vote);
	void OnVoteResults(IBaseBuiltinVote *vote, const menu_vote_result_t *results);
	void OnVoteCancel(IBaseBuiltinVote *vote, BuiltinVoteFailReason reason);
	bool OnSetHandlerOption(const char *option, const void *data);
private:
	cell_t DoAction(IBaseBuiltinVote *vote, BuiltinVoteAction action, cell_t param1, cell_t param2, cell_t def_res=0);
private:
	IPluginFunction *m_pBasic;
	int m_Flags;
	IPluginFunction *m_pVoteResults;
	cell_t m_fnVoteResult;
};

class EmptyBuiltinVoteHandler : public IBuiltinVoteHandler
{
public:
};

/***********************************
 **** NATIVE DEFINITIONS ***********
 ***********************************/
cell_t CreateBuiltinVote(IPluginContext *pContext, const cell_t *params);
cell_t DisplayBuiltinVote(IPluginContext *pContext, const cell_t *params);
cell_t AddBuiltinVoteItem(IPluginContext *pContext, const cell_t *params);
cell_t InsertBuiltinVoteItem(IPluginContext *pContext, const cell_t *params);
cell_t RemoveBuiltinVoteItem(IPluginContext *pContext, const cell_t *params);
cell_t RemoveAllBuiltinVoteItems(IPluginContext *pContext, const cell_t *params);
cell_t GetBuiltinVoteItem(IPluginContext *pContext, const cell_t *params);
cell_t GetBuiltinVoteItemCount(IPluginContext *pContext, const cell_t *params);
cell_t SetBuiltinVoteArgument(IPluginContext *pContext, const cell_t *params);
cell_t GetBuiltinVoteArgument(IPluginContext *pContext, const cell_t *params);
cell_t IsBuiltinVoteInProgress(IPluginContext *pContext, const cell_t *params);
//cell_t GetBuiltinVoteStyle(IPluginContext *pContext, const cell_t *params);
//cell_t GetBuiltinVoteStyleHandle(IPluginContext *pContext, const cell_t *params);
cell_t GetBuiltinVoteMaxItems(IPluginContext *pContext, const cell_t *params);
cell_t GetBuiltinVoteOptionFlags(IPluginContext *pContext, const cell_t *params);
cell_t SetBuiltinVoteOptionFlags(IPluginContext *pContext, const cell_t *params);
cell_t CancelBuiltinVote(IPluginContext *pContext, const cell_t *params);
cell_t SetBuiltinVoteResultCallback(IPluginContext *pContext, const cell_t *params);
cell_t CheckBuiltinVoteDelay(IPluginContext *pContext, const cell_t *params);
cell_t GetBuiltinVoteSelectionPosition(IPluginContext *pContext, const cell_t *params);
cell_t IsClientInBuiltinVotePool(IPluginContext *pContext, const cell_t *params);
cell_t RedrawClientBuiltinVote(IPluginContext *pContext, const cell_t *params);
cell_t GetBuiltinVoteType(IPluginContext *pContext, const cell_t *params);
cell_t SetBuiltinVoteTeam(IPluginContext *pContext, const cell_t *params);
cell_t GetBuiltinVoteTeam(IPluginContext *pContext, const cell_t *params);
cell_t SetBuiltinVoteInitiator(IPluginContext *pContext, const cell_t *params);
cell_t GetBuiltinVoteInitiator(IPluginContext *pContext, const cell_t *params);
cell_t DisplayBuiltinVotePass(IPluginContext *pContext, const cell_t *params);
cell_t DisplayBuiltinVotePass2(IPluginContext *pContext, const cell_t *params);
cell_t DisplayBuiltinVoteFail(IPluginContext *pContext, const cell_t *params);
cell_t Game_IsVoteInProgress(IPluginContext *pContext, const cell_t *params);
cell_t BuiltinVote_IsVoteInProgress(IPluginContext *pContext, const cell_t *params);

const sp_nativeinfo_t g_Natives[] = 
{
	{"CreateBuiltinVote",				CreateBuiltinVote},
	{"DisplayBuiltinVote",				DisplayBuiltinVote},
	{"AddBuiltinVoteItem",				AddBuiltinVoteItem},
	{"InsertBuiltinVoteItem",			InsertBuiltinVoteItem},
	{"RemoveBuiltinVoteItem",			RemoveBuiltinVoteItem},
	{"RemoveAllBuiltinVoteItems",		RemoveAllBuiltinVoteItems},
	{"GetBuiltinVoteItem",				GetBuiltinVoteItem},
	{"GetBuiltinVoteItemCount",			GetBuiltinVoteItemCount},
	{"SetBuiltinVoteArgument",			SetBuiltinVoteArgument},
	{"GetBuiltinVoteArgument",			GetBuiltinVoteArgument},
	{"IsBuiltinVoteInProgress",			IsBuiltinVoteInProgress},
	{"Game_IsVoteInProgress",			Game_IsVoteInProgress},
	{"BuiltinVote_IsVoteInProgress",	BuiltinVote_IsVoteInProgress},
//	{"GetBuiltinVoteStyle",				GetBuiltinVoteStyle},
//	{"GetBuiltinVoteStyleHandle",		GetBuiltinVoteStyleHandle},
	{"GetBuiltinVoteMaxItems",			GetBuiltinVoteMaxItems},
	{"GetBuiltinVoteOptionFlags",		GetBuiltinVoteOptionFlags},
	{"SetBuiltinVoteOptionFlags",		SetBuiltinVoteOptionFlags},
	{"CancelBuiltinVote",				CancelBuiltinVote},
	{"SetBuiltinVoteResultCallback",	SetBuiltinVoteResultCallback},
	{"CheckBuiltinVoteDelay",			CheckBuiltinVoteDelay},
//	{"GetBuiltinVoteSelectionPosition",	GetBuiltinVoteSelectionPosition},
	{"IsClientInBuiltinVotePool",		IsClientInBuiltinVotePool},
	{"RedrawClientBuiltinVote",			RedrawClientBuiltinVote},
	{"GetBuiltinVoteType",				GetBuiltinVoteType},
	{"SetBuiltinVoteTeam",				SetBuiltinVoteTeam},
	{"GetBuiltinVoteTeam",				GetBuiltinVoteTeam},
	{"SetBuiltinVoteInitiator",			SetBuiltinVoteInitiator},
	{"GetBuiltinVoteInitiator",			GetBuiltinVoteInitiator},
	{"DisplayBuiltinVotePass",			DisplayBuiltinVotePass},
	{"DisplayBuiltinVotePass2",			DisplayBuiltinVotePass2},
	{"DisplayBuiltinVoteFail",			DisplayBuiltinVoteFail},
	{NULL,								NULL},
};

extern VoteNativeHelpers g_VoteHelpers;

#endif // _INCLUDE_SOURCEMOD_NATIVES_PROPER_H_
