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

#include "natives.h"

VoteNativeHelpers g_VoteHelpers;
EmptyBuiltinVoteHandler s_EmptyBuiltinVoteHandler;
static unsigned int *s_CurSelectPosition = NULL;

/**
 * GLOBAL CLASS FOR HELPERS
 */

void VoteNativeHelpers::OnLoad()
{
}

void VoteNativeHelpers::OnUnload()
{
	while (!m_FreeVoteHandlers.empty())
	{
		delete m_FreeVoteHandlers.front();
		m_FreeVoteHandlers.pop();
	}
}

CBuiltinVoteHandler *VoteNativeHelpers::GetVoteHandler(IPluginFunction *pFunction, int flags)
{
	CBuiltinVoteHandler *handler;
	if (m_FreeVoteHandlers.empty())
	{
		handler = new CBuiltinVoteHandler(pFunction, flags);
	} else {
		handler = m_FreeVoteHandlers.front();
		m_FreeVoteHandlers.pop();
		handler->m_pBasic = pFunction;
		handler->m_Flags = flags;
		handler->m_pVoteResults = NULL;
	}
	return handler;
}

void VoteNativeHelpers::FreeVoteHandler(CBuiltinVoteHandler *handler)
{
	m_FreeVoteHandlers.push(handler);
}

/**
 * VOTE HANDLER WRAPPER
 */

CBuiltinVoteHandler::CBuiltinVoteHandler(IPluginFunction *pBasic, int flags) :
	m_pBasic(pBasic), m_Flags(flags), m_pVoteResults(NULL)
{
	/* :TODO: We can probably cache this ahead of time */
}

/*
void CBuiltinVoteHandler::OnVoteStart(IBaseBuiltinVote *vote)
{
	if ((m_Flags & (int)BuiltinVoteAction_Start) == (int)BuiltinVoteAction_Start)
	{
		DoAction(vote, BuiltinVoteAction_Start, 0, 0);
	}
}

void CBuiltinVoteHandler::OnVoteDisplay(IBaseBuiltinVote *vote, int client)
{
	if ((m_Flags & (int)BuiltinVoteAction_Display) == (int)BuiltinVoteAction_Display)
	{
		DoAction(vote, BuiltinVoteAction_Display, client, 0);
	}
}
*/

void CBuiltinVoteHandler::OnVoteSelect(IBaseBuiltinVote *vote, int client, unsigned int item)
{
	/* Save our old position first. */
	unsigned int *old_pos = s_CurSelectPosition;

	unsigned int old_reply = playerhelpers->SetReplyTo(SM_REPLY_CHAT);
	DoAction(vote, BuiltinVoteAction_Select, client, item);
	playerhelpers->SetReplyTo(old_reply);

	s_CurSelectPosition = old_pos;
}

void CBuiltinVoteHandler::OnVoteEnd(IBaseBuiltinVote *vote, BuiltinVoteEndReason reason)
{
	DoAction(vote, BuiltinVoteAction_End, reason, 0);
}

void CBuiltinVoteHandler::OnVoteDestroy(IBaseBuiltinVote *vote)
{
	g_VoteHelpers.FreeVoteHandler(this);
}

void CBuiltinVoteHandler::OnVoteStart(IBaseBuiltinVote *vote)
{
	DoAction(vote, BuiltinVoteAction_Start, 0, 0);
}

void CBuiltinVoteHandler::OnVoteCancel(IBaseBuiltinVote *vote, BuiltinVoteFailReason reason)
{
	DoAction(vote, BuiltinVoteAction_Cancel, reason, 0);
}

cell_t CBuiltinVoteHandler::DoAction(IBaseBuiltinVote *vote, BuiltinVoteAction action, cell_t param1, cell_t param2, cell_t def_res)
{
	cell_t res = def_res;
	m_pBasic->PushCell(vote->GetHandle());
	m_pBasic->PushCell((cell_t)action);
	m_pBasic->PushCell(param1);
	m_pBasic->PushCell(param2);
	m_pBasic->Execute(&res);
	return res;
}

void CBuiltinVoteHandler::OnVoteResults(IBaseBuiltinVote *vote, const menu_vote_result_t *results)
{
	if (!m_pVoteResults)
	{
		/* Call VoteAction_VoteEnd instead.  See if there are any extra winners. */
		unsigned int num_items = 1;
		for (unsigned int i=1; i<results->num_items; i++)
		{
			if (results->item_list[i].count != results->item_list[0].count)
			{
				break;
			}
			num_items++;
		}

		/* See if we need to pick a random winner. */
		unsigned int winning_item;
		if (num_items > 1)
		{
			/* Yes, we do. */
			srand(time(NULL));
			winning_item = rand() % num_items;
			winning_item = results->item_list[winning_item].item;
		} else {
			/* No, take the first. */
			winning_item = results->item_list[0].item;
		}

		unsigned int total_votes = results->num_votes;
		unsigned int winning_votes = results->item_list[0].count;

		DoAction(vote, BuiltinVoteAction_VoteEnd, winning_item, (total_votes << 16) | (winning_votes & 0xFFFF));
	} else {
		IPluginContext *pContext = m_pVoteResults->GetParentContext();
		bool no_call = false;
		int err;

		/* First array */
		cell_t client_array_address = -1;
		cell_t *client_array_base = NULL;
		cell_t client_array_size = results->num_clients + (results->num_clients * 2);
		if (client_array_size)
		{
			if ((err = pContext->HeapAlloc(client_array_size, &client_array_address, &client_array_base))
				!= SP_ERROR_NONE)
			{
				//g_DbgReporter.GenerateError(pContext, m_fnVoteResult, err, "Vote callback could not allocate %d bytes for client list.", client_array_size * sizeof(cell_t));
				no_call = true;
			} else {
				cell_t target_offs = sizeof(cell_t) * results->num_clients;
				cell_t *cur_index = client_array_base;
				cell_t *cur_array;
				for (unsigned int i=0; i<results->num_clients; i++)
				{
					/* Copy the array index */
					*cur_index = target_offs;
					/* Get the current array address */
					cur_array = (cell_t *)((char *)cur_index + target_offs);
					/* Store information */
					cur_array[0] = results->client_list[i].client;
					cur_array[1] = results->client_list[i].item;
					/* Adjust for the new target by subtracting one indirection
					 * and adding one array.
					 */
					target_offs += (sizeof(cell_t) * 2) - sizeof(cell_t);
					cur_index++;
				}
			}
		}

		/* Second array */
		cell_t item_array_address = -1;
		cell_t *item_array_base = NULL;
		cell_t item_array_size = results->num_items + (results->num_items * 2);
		if (item_array_size)
		{
			if ((err = pContext->HeapAlloc(item_array_size, &item_array_address, &item_array_base))
				!= SP_ERROR_NONE)
			{
				//g_DbgReporter.GenerateError(pContext, m_fnVoteResult, err, "Menu callback could not allocate %d bytes for item list.", item_array_size);
				no_call = true;
			} else {
				cell_t target_offs = sizeof(cell_t) * results->num_items;
				cell_t *cur_index = item_array_base;
				cell_t *cur_array;
				for (unsigned int i=0; i<results->num_items; i++)
				{
					/* Copy the array index */
					*cur_index = target_offs;
					/* Get the current array address */
					cur_array = (cell_t *)((char *)cur_index + target_offs);
					/* Store information */
					cur_array[0] = results->item_list[i].item;
					cur_array[1] = results->item_list[i].count;
					/* Adjust for the new target by subtracting one indirection
					 * and adding one array.
					 */
					target_offs += (sizeof(cell_t) * 2) - sizeof(cell_t);
					cur_index++;
				}
			}
		}

		/* Finally, push everything */
		if (!no_call)
		{
			m_pVoteResults->PushCell(vote->GetHandle());
			m_pVoteResults->PushCell(results->num_votes);
			m_pVoteResults->PushCell(results->num_clients);
			m_pVoteResults->PushCell(client_array_address);
			m_pVoteResults->PushCell(results->num_items);
			m_pVoteResults->PushCell(item_array_address);
			m_pVoteResults->Execute(NULL);
		}

		/* Free what we allocated, in reverse order as required */
		if (item_array_address != -1)
		{
			pContext->HeapPop(item_array_address);
		}
		if (client_array_address != -1)
		{
			pContext->HeapPop(client_array_address);
		}
	}
}

bool CBuiltinVoteHandler::OnSetHandlerOption(const char *option, const void *data)
{
	if (strcmp(option, "set_vote_results_handler") == 0)
	{
		void **array = (void **)data;
		m_pVoteResults = (IPluginFunction *)array[0];
		m_fnVoteResult = *(cell_t *)((cell_t *)array[1]);
		return true;
	}

	return false;
}

/***********************************
 **** NATIVE DEFINITIONS ***********
 ***********************************/

cell_t CreateBuiltinVote(IPluginContext *pContext, const cell_t *params)
{
	IBuiltinVoteStyle *style = g_BuiltinVotes.GetStyle();
	IPluginFunction *pFunction;

	if ((pFunction=pContext->GetFunctionById(params[1])) == NULL)
	{
		return pContext->ThrowNativeError("Function id %x is invalid", params[1]);
	}

	CBuiltinVoteHandler *handler = g_VoteHelpers.GetVoteHandler(pFunction, params[3]);
	BuiltinVoteType type = (BuiltinVoteType)params[2];

	IBaseBuiltinVote *vote = style->CreateVote(handler, type, pContext->GetIdentity());

	Handle_t hndl = vote->GetHandle();
	if (!hndl)
	{
		vote->Destroy();
		return BAD_HANDLE;
	}

	return hndl;
}

cell_t DisplayBuiltinVote(IPluginContext *pContext, const cell_t *params)
{
	if (g_BuiltinVotes.IsVoteInProgress())
	{
		return pContext->ThrowNativeError("A vote is already in progress");
	}

	Handle_t hndl = (Handle_t)params[1];
	HandleError err;
	IBaseBuiltinVote *vote;

	if ((err=g_BuiltinVotes.ReadVoteHandle(params[1], &vote)) != HandleError_None)
	{
		return pContext->ThrowNativeError("Vote handle %x is invalid (error %d)", hndl, err);
	}

	cell_t *addr;
	pContext->LocalToPhysAddr(params[2], &addr);

	cell_t flags = 0;
	if (params[0] >= 5)
	{
		flags = params[5];
	}

	if (!g_BuiltinVotes.StartVote(vote, params[3], addr, params[4], flags))
	{
		return 0;
	}

	return 1;
}

cell_t AddBuiltinVoteItem(IPluginContext *pContext, const cell_t *params)
{
	Handle_t hndl = (Handle_t)params[1];
	HandleError err;
	IBaseBuiltinVote *vote;

	if ((err=g_BuiltinVotes.ReadVoteHandle(params[1], &vote)) != HandleError_None)
	{
		return pContext->ThrowNativeError("Vote handle %x is invalid (error %d)", hndl, err);
	}

	char *info;
	char *display;

	pContext->LocalToString(params[2], &info);
	pContext->LocalToString(params[3], &display);

	return vote->AppendItem(info, display) ? 1 : 0;
}

cell_t InsertBuiltinVoteItem(IPluginContext *pContext, const cell_t *params)
{
	Handle_t hndl = (Handle_t)params[1];
	HandleError err;
	IBaseBuiltinVote *vote;

	if ((err=g_BuiltinVotes.ReadVoteHandle(params[1], &vote)) != HandleError_None)
	{
		return pContext->ThrowNativeError("Vote handle %x is invalid (error %d)", hndl, err);
	}

	char *info;
	char *display;

	pContext->LocalToString(params[3], &info);
	pContext->LocalToString(params[4], &display);

	return vote->InsertItem(params[2], info, display) ? 1 : 0;
}

cell_t RemoveBuiltinVoteItem(IPluginContext *pContext, const cell_t *params)
{
	Handle_t hndl = (Handle_t)params[1];
	HandleError err;
	IBaseBuiltinVote *vote;

	if ((err=g_BuiltinVotes.ReadVoteHandle(params[1], &vote)) != HandleError_None)
	{
		return pContext->ThrowNativeError("Vote handle %x is invalid (error %d)", hndl, err);
	}

	return vote->RemoveItem(params[2]) ? 1 : 0;
}

cell_t RemoveAllBuiltinVoteItems(IPluginContext *pContext, const cell_t *params)
{
	Handle_t hndl = (Handle_t)params[1];
	HandleError err;
	IBaseBuiltinVote *vote;

	if ((err=g_BuiltinVotes.ReadVoteHandle(params[1], &vote)) != HandleError_None)
	{
		return pContext->ThrowNativeError("Vote handle %x is invalid (error %d)", hndl, err);
	}

	vote->RemoveAllItems();

	return 1;
}

cell_t GetBuiltinVoteItem(IPluginContext *pContext, const cell_t *params)
{
	Handle_t hndl = (Handle_t)params[1];
	HandleError err;
	IBaseBuiltinVote *vote;

	if ((err=g_BuiltinVotes.ReadVoteHandle(params[1], &vote)) != HandleError_None)
	{
		return pContext->ThrowNativeError("Vote handle %x is invalid (error %d)", hndl, err);
	}

	const char *info;
	const char *display;

	if ((info=vote->GetItemName(params[2])) == NULL)
	{
		return 0;
	}

	display=vote->GetItemDisplay(params[2]);

	pContext->StringToLocalUTF8(params[3], params[4], info, NULL);
	pContext->StringToLocalUTF8(params[5], params[6], display ? display : "", NULL);

	return 1;
}

cell_t GetBuiltinVoteItemCount(IPluginContext *pContext, const cell_t *params)
{
	Handle_t hndl = (Handle_t)params[1];
	HandleError err;
	IBaseBuiltinVote *vote;

	if ((err=g_BuiltinVotes.ReadVoteHandle(params[1], &vote)) != HandleError_None)
	{
		return pContext->ThrowNativeError("Vote handle %x is invalid (error %d)", hndl, err);
	}

	return vote->GetItemCount();
}

cell_t SetBuiltinVoteArgument(IPluginContext *pContext, const cell_t *params)
{
	Handle_t hndl = (Handle_t)params[1];
	HandleError err;
	IBaseBuiltinVote *vote;

	if ((err=g_BuiltinVotes.ReadVoteHandle(params[1], &vote)) != HandleError_None)
	{
		return pContext->ThrowNativeError("Vote handle %x is invalid (error %d)", hndl, err);
	}

	smutils->SetGlobalTarget(SOURCEMOD_SERVER_LANGUAGE);

	char buffer[1024];
	smutils->FormatString(buffer, sizeof(buffer), pContext, params, 2);

	vote->SetArgument(buffer);

	return 1;
}

cell_t GetBuiltinVoteArgument(IPluginContext *pContext, const cell_t *params)
{
	Handle_t hndl = (Handle_t)params[1];
	HandleError err;
	IBaseBuiltinVote *vote;

	if ((err=g_BuiltinVotes.ReadVoteHandle(params[1], &vote)) != HandleError_None)
	{
		return pContext->ThrowNativeError("Vote handle %x is invalid (error %d)", hndl, err);
	}

	size_t written;
	const char *title = vote->GetArgument();
	pContext->StringToLocalUTF8(params[2], params[3], title, &written);

	return (cell_t)written;
}


cell_t IsBuiltinVoteInProgress(IPluginContext *pContext, const cell_t *params)
{
	return g_BuiltinVotes.IsVoteInProgress() ? 1 : 0;
}


/*
cell_t GetBuiltinVoteStyle(IPluginContext *pContext, const cell_t *params)
{
	Handle_t hndl = (Handle_t)params[1];
	HandleError err;
	IBaseBuiltinVote *vote;

	if ((err=g_BuiltinVotes.ReadVoteHandle(params[1], &vote)) != HandleError_None)
	{
		return pContext->ThrowNativeError("Vote handle %x is invalid (error %d)", hndl, err);
	}

	return vote->GetDrawStyle()->GetHandle();
}
*/

/*
cell_t GetBuiltinVoteStyleHandle(IPluginContext *pContext, const cell_t *params)
{
	if (!m_VoteStyle)
	{
		return BAD_HANDLE;
	}

	return m_VoteStyle->GetHandle();
}
*/

cell_t GetBuiltinVoteMaxItems(IPluginContext *pContext, const cell_t *params)
{
	return g_BuiltinVotes.GetStyle()->GetMaxItems();
}

cell_t GetBuiltinVoteOptionFlags(IPluginContext *pContext, const cell_t *params)
{
	Handle_t hndl = (Handle_t)params[1];
	HandleError err;
	IBaseBuiltinVote *vote;

	if ((err=g_BuiltinVotes.ReadVoteHandle(params[1], &vote)) != HandleError_None)
	{
		return pContext->ThrowNativeError("Vote handle %x is invalid (error %d)", hndl, err);
	}

	return vote->GetVoteOptionFlags();
}

cell_t SetBuiltinVoteOptionFlags(IPluginContext *pContext, const cell_t *params)
{
	Handle_t hndl = (Handle_t)params[1];
	HandleError err;
	IBaseBuiltinVote *vote;

	if ((err=g_BuiltinVotes.ReadVoteHandle(params[1], &vote)) != HandleError_None)
	{
		return pContext->ThrowNativeError("Vote handle %x is invalid (error %d)", hndl, err);
	}

	vote->SetVoteOptionFlags(params[2]);

	return 1;
}

cell_t CancelBuiltinVote(IPluginContext *pContext, const cell_t *params)
{
	if (!g_BuiltinVotes.IsVoteInProgress())
	{
		return pContext->ThrowNativeError("No vote is in progress");
	}

	g_BuiltinVotes.CancelVoting();

	return 1;
}

cell_t SetBuiltinVoteResultCallback(IPluginContext *pContext, const cell_t *params)
{
	Handle_t hndl = (Handle_t)params[1];
	HandleError err;
	IBaseBuiltinVote *vote;

	if ((err=g_BuiltinVotes.ReadVoteHandle(params[1], &vote)) != HandleError_None)
	{
		return pContext->ThrowNativeError("Vote handle %x is invalid (error %d)", hndl, err);
	}

	IPluginFunction *pFunction = pContext->GetFunctionById(params[2]);
	if (!pFunction)
	{
		return pContext->ThrowNativeError("Invalid function %x", params[2]);
	}

	void *array[2];
	array[0] = pFunction;
	array[1] = (void *)&params[2];

	IBuiltinVoteHandler *pHandler = vote->GetHandler();
	if (!pHandler->OnSetHandlerOption("set_vote_results_handler", (const void *)array))
	{
		return pContext->ThrowNativeError("The given vote does not support this option");
	}

	return 1;

}

cell_t CheckBuiltinVoteDelay(IPluginContext *pContext, const cell_t *params)
{
	return g_BuiltinVotes.GetRemainingVoteDelay();
}

cell_t GetBuiltinVoteSelectionPosition(IPluginContext *pContext, const cell_t *params)
{
	if (!s_CurSelectPosition)
	{
		return pContext->ThrowNativeError("Can only be called from inside a VoteAction_Select callback");
	}

	return *s_CurSelectPosition;
}

cell_t IsClientInBuiltinVotePool(IPluginContext *pContext, const cell_t *params)
{
	int client;
	IGamePlayer *pPlayer;

	client = params[1];
	if ((pPlayer = playerhelpers->GetGamePlayer(client)) == NULL)
	{
		return pContext->ThrowNativeError("Invalid client index %d", client);
	}

	if (!g_BuiltinVotes.IsVoteInProgress())
	{
		return pContext->ThrowNativeError("No vote is in progress");
	}

	return g_BuiltinVotes.IsClientInVotePool(client) ? 1 : 0;
}

cell_t RedrawClientBuiltinVote(IPluginContext *pContext, const cell_t *params)
{
	int client;
	IGamePlayer *pPlayer;

	client = params[1];
	if ((pPlayer = playerhelpers->GetGamePlayer(client)) == NULL)
	{
		return pContext->ThrowNativeError("Invalid client index %d", client);
	}

	if (!g_BuiltinVotes.IsVoteInProgress())
	{
		return pContext->ThrowNativeError("No vote is in progress");
	}

	if (!g_BuiltinVotes.IsClientInVotePool(client))
	{
		return pContext->ThrowNativeError("Client is not in the voting pool");
	}

	bool revote = true;
	if (params[0] >= 2 && !params[2])
	{
		revote = false;
	}

	return g_BuiltinVotes.RedrawClientVote2(client, revote) ? 1 : 0;
}

cell_t GetBuiltinVoteType(IPluginContext *pContext, const cell_t *params)
{
	Handle_t hndl = (Handle_t)params[1];
	HandleError err;
	IBaseBuiltinVote *vote;

	if ((err=g_BuiltinVotes.ReadVoteHandle(params[1], &vote)) != HandleError_None)
	{
		return pContext->ThrowNativeError("Vote handle %x is invalid (error %d)", hndl, err);
	}

	return vote->GetVoteType();
}

cell_t SetBuiltinVoteTeam(IPluginContext *pContext, const cell_t *params)
{
	Handle_t hndl = (Handle_t)params[1];
	HandleError err;
	IBaseBuiltinVote *vote;

	if ((err=g_BuiltinVotes.ReadVoteHandle(params[1], &vote)) != HandleError_None)
	{
		return pContext->ThrowNativeError("Vote handle %x is invalid (error %d)", hndl, err);
	}

	vote->SetTeam(params[2]);

	return 1;
}

cell_t GetBuiltinVoteTeam(IPluginContext *pContext, const cell_t *params)
{
	Handle_t hndl = (Handle_t)params[1];
	HandleError err;
	IBaseBuiltinVote *vote;

	if ((err=g_BuiltinVotes.ReadVoteHandle(params[1], &vote)) != HandleError_None)
	{
		return pContext->ThrowNativeError("Vote handle %x is invalid (error %d)", hndl, err);
	}

	return vote->GetTeam();
}

cell_t SetBuiltinVoteInitiator(IPluginContext *pContext, const cell_t *params)
{
	Handle_t hndl = (Handle_t)params[1];
	HandleError err;
	IBaseBuiltinVote *vote;

	if ((err=g_BuiltinVotes.ReadVoteHandle(params[1], &vote)) != HandleError_None)
	{
		return pContext->ThrowNativeError("Vote handle %x is invalid (error %d)", hndl, err);
	}

	vote->SetInitiator(params[2]);
	return 1;
}

cell_t GetBuiltinVoteInitiator(IPluginContext *pContext, const cell_t *params)
{
	Handle_t hndl = (Handle_t)params[1];
	HandleError err;
	IBaseBuiltinVote *vote;

	if ((err=g_BuiltinVotes.ReadVoteHandle(params[1], &vote)) != HandleError_None)
	{
		return pContext->ThrowNativeError("Vote handle %x is invalid (error %d)", hndl, err);
	}

	return vote->GetInitiator();
}

cell_t DisplayBuiltinVotePass(IPluginContext *pContext, const cell_t *params)
{
	Handle_t hndl = (Handle_t)params[1];
	HandleError err;
	IBaseBuiltinVote *vote;

	if ((err=g_BuiltinVotes.ReadVoteHandle(params[1], &vote)) != HandleError_None)
	{
		return pContext->ThrowNativeError("Vote handle %x is invalid (error %d)", hndl, err);
	}

	char *winner;

	pContext->LocalToString(params[2], &winner);

	vote->DisplayVotePass(winner);

	return 1;
}

cell_t DisplayBuiltinVotePass2(IPluginContext *pContext, const cell_t *params)
{
	Handle_t hndl = (Handle_t)params[1];
	HandleError err;
	IBaseBuiltinVote *vote;

	if ((err=g_BuiltinVotes.ReadVoteHandle(params[1], &vote)) != HandleError_None)
	{
		return pContext->ThrowNativeError("Vote handle %x is invalid (error %d)", hndl, err);
	}

	char *translation;
	char *winner;

	pContext->LocalToString(params[2], &translation);
	pContext->LocalToString(params[3], &winner);

	vote->DisplayVotePass(translation, winner);

	return 1;
}

cell_t DisplayBuiltinVoteFail(IPluginContext *pContext, const cell_t *params)
{
	Handle_t hndl = (Handle_t)params[1];
	HandleError err;
	IBaseBuiltinVote *vote;

	if ((err=g_BuiltinVotes.ReadVoteHandle(params[1], &vote)) != HandleError_None)
	{
		return pContext->ThrowNativeError("Vote handle %x is invalid (error %d)", hndl, err);
	}

	BuiltinVoteFailReason reason = (BuiltinVoteFailReason) params[2];

	vote->DisplayVoteFail(reason);

	return 1;
}
