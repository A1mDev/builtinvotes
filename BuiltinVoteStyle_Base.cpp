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

#include "BuiltinVoteStyle_Base.h"

BaseBuiltinVoteStyle::BaseBuiltinVoteStyle() : //m_WatchList(256),
m_hHandle(BAD_HANDLE), m_players(new CBaseBuiltinVotePlayer[256+1])
{
}

BaseBuiltinVoteStyle::~BaseBuiltinVoteStyle()
{
	delete [] m_players;
}

Handle_t BaseBuiltinVoteStyle::GetHandle()
{
	/* Don't create the handle until we need it */
	if (m_hHandle == BAD_HANDLE)
	{
		m_hHandle = g_BuiltinVotes.CreateStyleHandle(this);
	}

	return m_hHandle;
}

CBaseBuiltinVotePlayer *BaseBuiltinVoteStyle::GetVotePlayer(int client)
{
	return &m_players[client];
}

/*
void BaseBuiltinVoteStyle::AddClientToWatch(int client)
{
	m_WatchList.push_back(client);
}

void BaseBuiltinVoteStyle::RemoveClientFromWatch(int client)
{
	m_WatchList.remove(client);
}
*/

void BaseBuiltinVoteStyle::CancelVote(CBaseBuiltinVote *vote)
{
	IBuiltinVoteHandler *bvh = vote->GetHandler();
	if (bvh)
	{
		bvh->OnVoteCancel(vote, BuiltinVoteFail_Generic);
		bvh->OnVoteEnd(vote, BuiltinVoteEnd_Cancelled);
	}
}

void BaseBuiltinVoteStyle::OnClientDisconnected(int client)
{
	/*
	CBaseBuiltinVotePlayer *player = GetVotePlayer(client);
	if (!player->bInVote)
	{
		return;
	}

	player->bInVote = false;
	player->bInExternVote = false;
	*/
}

//static int do_lookup[256];

/*
void BaseBuiltinVoteStyle::ProcessWatchList()
{
	if (!m_WatchList.size())
	{
		return;
	}

	unsigned int total = 0;
	for (FastLink<int>::iterator iter=m_WatchList.begin(); iter!=m_WatchList.end(); ++iter)
	{
		do_lookup[total++] = (*iter);
	}

	int client;
	CBaseBuiltinVotePlayer *player;
	float curtime = gpGlobals->curtime;
	for (unsigned int i=0; i<total; i++)
	{
		client = do_lookup[i];
		player = GetVotePlayer(client);
		if (!player->bInVote || !player->voteHoldTime)
		{
			m_WatchList.remove(client);
			continue;
		}
		if (curtime > player->voteStartTime + player->voteHoldTime)
		{
// Cancel client menu
		}

	}
}

*/

//void BaseBuiltinVoteStyle::ClientPressedKey(int client, unsigned int key_press)
//{
//	CBaseBuiltinVotePlayer *player = GetVotePlayer(client);
//
//	/* First question: Are we in a vote? */
//	/* Whoops, this is legacy code for if a menu is showing */
//	/*
//	if (!player->bInVote)
//	{
//		return;
//	}
//	*/
//
//	bool cancel = false;
//	unsigned int item = 0;
//	BuiltinVoteEndReason end_reason =  BuiltinVoteEnd_Cancelled;
//	vote_states_t &states = player->states;
//
//	/* Save variables */
//	IBuiltinVoteHandler *bvh = states.bvh;
//	IBaseBuiltinVote *vote = states.vote;
//
//	assert(bvh != NULL);
//
//	if (key_press < 1 || key_press > GetMaxItems())
//	{
//		return;
//	}
//
//	item = key_press - 1;
//
//	/* Clear states */
//	player->bInVote = false;
//	/*
//	if (player->voteHoldTime)
//	{
//		RemoveClientFromWatch(client);
//	}
//	*/
//
//	//bvh->OnVoteSelect(vote, client, item);
//
//	// Call moved to OnVoteSelect to fix a bug
//	//vote->ClientPressedKey(client, item);
//
//	s_VoteHandler.OnVoteSelect(vote, client, item);
//
//	/* Only fire end for valid votes */
//	/* *sigh* Another useless thing as I don't track this */
//	/*
//	if (vote)
//	{
//		bvh->OnVoteEnd(vote, end_reason);
//	}
//	*/
//
//}

void BaseBuiltinVoteStyle::SendDisplay(int client, IBaseBuiltinVote *vote)
{
	int clients[1] = { client };
	SendDisplay(clients, 1, vote);
}

void BaseBuiltinVoteStyle::SendDisplay(int clients[], unsigned int num_clients, IBaseBuiltinVote *vote)
{
	vote->Display(clients, num_clients);
}

bool BaseBuiltinVoteStyle::DoClientVote(int client, IBaseBuiltinVote *vote, IBuiltinVoteHandler *bvh)
{
	int clients[1] = { client };
	return DoClientVote(clients, 1, vote, bvh);
}

bool BaseBuiltinVoteStyle::DoClientVote(int clients[], unsigned int num_clients, IBaseBuiltinVote *vote, IBuiltinVoteHandler *bvh)
{
	unsigned int totalPlayers = 0;
	int realClients[256+1];

	for (unsigned int i=0; i<num_clients;i++)
	{
		IGamePlayer *pPlayer = playerhelpers->GetGamePlayer(clients[i]);

		CBaseBuiltinVotePlayer *player = GetVotePlayer(clients[i]);
		vote_states_t &states = player->states;

		if (!pPlayer || pPlayer->IsFakeClient() || !pPlayer->IsInGame())
		{
			states.vote = NULL;
			states.bvh = NULL;
			continue;
		}

		states.vote = vote;
		states.bvh = bvh;
		states.apiVers = SMINTERFACE_BUILTINVOTES_VERSION;

		realClients[totalPlayers++] = clients[i];
	}

	if (totalPlayers > 0)
	{
		vote->Display(realClients, totalPlayers);
		//SendDisplay(realClients, totalPlayers, vote);
		return true;
	} else {
		return false;
	}
}

bool BaseBuiltinVoteStyle::RedoClientVote(int client)
{
	CBaseBuiltinVotePlayer *player = GetVotePlayer(client);
	vote_states_t &states = player->states;

//	player->bAutoIgnore = true;
	IBaseBuiltinVote *vote = states.vote;
	if (!vote)
	{
//		player->bAutoIgnore = false;
		return false;
	}

	SendDisplay(client, vote);

//	player->bAutoIgnore = false;

	return true;
}


CBaseBuiltinVote::CBaseBuiltinVote(IBuiltinVoteHandler *pHandler, IBuiltinVoteStyle *pStyle, BuiltinVoteType type, IdentityToken_t *pOwner) :
m_pStyle(pStyle), m_Strings(512), m_bShouldDelete(false), m_bCancelling(false), m_pOwner(pOwner), m_bDeleting(false),
m_bWillFreeHandle(false), m_hHandle(BAD_HANDLE), m_pHandler(pHandler), m_voteType(type), m_initiator(SERVER_CLIENT_INDEX), m_team(-1)
{
	if (type != BuiltinVoteType_Custom_Mult && type != BuiltinVoteType_NextLevelMult)
	{
		CItem noItem;

		noItem.infoString = m_Strings.AddString("No");
		noItem.displayString = m_Strings.AddString("No");
		m_items.push_back(noItem);

		CItem yesItem;
		
		yesItem.infoString = m_Strings.AddString("Yes");
		yesItem.displayString = m_Strings.AddString("Yes");
		m_items.push_back(yesItem);
	}
}

CBaseBuiltinVote::~CBaseBuiltinVote()
{
}

Handle_t CBaseBuiltinVote::GetHandle()
{
	if (!m_hHandle)
	{
		m_hHandle = g_BuiltinVotes.CreateVoteHandle(this, m_pOwner);
	}

	return m_hHandle;
}

bool CBaseBuiltinVote::AppendItem(const char *info, const char *display)
{
	if ((m_voteType != BuiltinVoteType_Custom_Mult && m_voteType != BuiltinVoteType_NextLevelMult) || m_items.size() >= m_pStyle->GetMaxItems())
	{
		return false;
	}

	CItem item;
	
	item.infoString = m_Strings.AddString(info);
	if (display)
	{
		item.displayString = m_Strings.AddString(display);
	}
	m_items.push_back(item);

	return true;
}

bool CBaseBuiltinVote::InsertItem(unsigned int position, const char *info, const char *display)
{
	if ((m_voteType != BuiltinVoteType_Custom_Mult && m_voteType != BuiltinVoteType_NextLevelMult) || m_items.size() >= m_pStyle->GetMaxItems())
	{
		return false;
	}

	CItem item;
	
	item.infoString = m_Strings.AddString(info);
	if (display)
	{
		item.displayString = m_Strings.AddString(display);
	}
	CVector<CItem>::iterator iter = m_items.iterAt(position);
	m_items.insert(iter, item);

	return true;	
}

bool CBaseBuiltinVote::RemoveItem(unsigned int position)
{
	if ((m_voteType != BuiltinVoteType_Custom_Mult && m_voteType != BuiltinVoteType_NextLevelMult) || position >= m_items.size())
	{
		return false;
	}

	m_items.erase(m_items.iterAt(position));

	if (m_items.size() == 0)
	{
		m_Strings.Reset();
	}

	return true;
}

void CBaseBuiltinVote::RemoveAllItems()
{
	if (m_voteType != BuiltinVoteType_Custom_Mult && m_voteType != BuiltinVoteType_NextLevelMult)
	{
		return;
	}

	m_items.clear();
	m_Strings.Reset();
}

const char *CBaseBuiltinVote::GetItemName(unsigned int position)
{
	if (position >= m_items.size())
	{
		return NULL;
	}

	return m_Strings.GetString(m_items[position].infoString);
}

const char *CBaseBuiltinVote::GetItemDisplay(unsigned int position)
{
	if (position >= m_items.size())
	{
		return NULL;
	}

	return m_Strings.GetString(m_items[position].displayString);
}

unsigned int CBaseBuiltinVote::GetItemCount()
{
	return m_items.size();
}

IBuiltinVoteStyle *CBaseBuiltinVote::GetDrawStyle()
{
	return m_pStyle;
}

void CBaseBuiltinVote::SetArgument(const char *message)
{
	m_Argument.assign(message);
}

const char *CBaseBuiltinVote::GetArgument()
{
	return m_Argument.c_str();
}

void CBaseBuiltinVote::Cancel()
{
	// Doesn't really do cancelling.  Use BuiltinVoteHandler's CancelVoting instead, which calls this internally
	/*

	if (m_bCancelling)
	{
		return;
	}

	m_bCancelling = true;

	if (!m_bResultDisplayed)
	{
		m_pHandler->OnVoteCancel(this, BuiltinVoteFail_Generic);
		m_pHandler->OnVoteEnd(this, BuiltinVoteEnd_Cancelled);
	}

	m_bCancelling = false;

	*/

	if (m_bShouldDelete)
	{
		InternalDelete();
	}
}

void CBaseBuiltinVote::Destroy(bool releaseHandle)
{
	/* Check if we shouldn't be here */
	if (m_bDeleting)
	{
		return;
	}

	/* Save the destruction hint about our handle */
	m_bWillFreeHandle = releaseHandle;

	/* Now actually do stuff */
	if (!m_bCancelling || m_bShouldDelete)
	{
		Cancel();
		InternalDelete();
	} else {
		m_bShouldDelete = true;
	}
}

void CBaseBuiltinVote::InternalDelete()
{
	if (m_bWillFreeHandle && m_hHandle != BAD_HANDLE)
	{
		Handle_t hndl = m_hHandle;
		HandleSecurity sec;

		sec.pOwner = m_pOwner;
		sec.pIdentity = myself->GetIdentity();

		m_hHandle = BAD_HANDLE;
		m_bDeleting = true;
		handlesys->FreeHandle(hndl, &sec);
	}

	m_pHandler->OnVoteDestroy(this);

	delete this;
}

unsigned int CBaseBuiltinVote::GetVoteOptionFlags()
{
	return m_nFlags;
}

void CBaseBuiltinVote::SetVoteOptionFlags(unsigned int flags)
{
	m_nFlags = flags;
}

IBuiltinVoteHandler *CBaseBuiltinVote::GetHandler()
{
	return m_pHandler;
}

unsigned int CBaseBuiltinVote::GetBaseMemUsage()
{
	return m_Argument.size()
		+ m_Strings.GetMemTable()->GetMemUsage()
		+ (m_items.size() * sizeof(CItem));
}

BuiltinVoteType CBaseBuiltinVote::GetVoteType()
{
	return m_voteType;
}

void CBaseBuiltinVote::SetTeam(short team)
{
	m_team = team;
}

short CBaseBuiltinVote::GetTeam()
{
	return m_team;
}

void CBaseBuiltinVote::SetInitiator(unsigned char client)
{
	m_initiator = client;
}

unsigned char CBaseBuiltinVote::GetInitiator()
{
	return m_initiator;
}

unsigned int CBaseBuiltinVote::GetAllPlayers(cell_t clients[])
{
	unsigned int playersNum = 0;

	int maxclients = playerhelpers->GetMaxClients();

	for (int i = 1; i <= maxclients; i++)
	{
		if (playerhelpers->GetGamePlayer(i)->IsInGame())
		{
			clients[playersNum++] = i;
		}
	}
	return playersNum;
}

bool CBaseBuiltinVote::Display(int client)
{
	if (m_bCancelling)
	{
		return false;
	}

	int clients[1] = { client };

	return Display(clients, 1);
}
