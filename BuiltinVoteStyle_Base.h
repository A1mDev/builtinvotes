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

#ifndef _INCLUDE_BUILTINVOTE_BASE_H
#define _INCLUDE_BUILTINVOTE_BASE_H

#include <IBuiltinVoteManager.h>
#include <IPlayerHelpers.h>
#include <sh_string.h>
#include <sh_vector.h>
#include "extension.h"
#include "sm_memtable.h"

using namespace SourceMod;
using namespace SourceHook;

class CItem
{
public:
	CItem()
	{
		infoString = -1;
		displayString = -1;
		access = 0;
	}
public:
	int infoString;
	int displayString;
	unsigned int access;
};

class CBaseBuiltinVotePlayer
{
public:
	CBaseBuiltinVotePlayer()
		//: bInVote(false), bAutoIgnore(false), bInExternVote(false)
	{
	}
public:
	vote_states_t states;
	//bool bInVote;
	//bool bAutoIgnore;
	//float voteStartTime;
	//unsigned int voteHoldTime;
	//bool bInExternVote;
};

class CBaseBuiltinVote;

class BaseBuiltinVoteStyle :
	public IBuiltinVoteStyle,
	public IClientListener
{
public:
	BaseBuiltinVoteStyle();
	~BaseBuiltinVoteStyle();
public: //IBuiltinVoteStyle
	Handle_t GetHandle();
public: //IClientListener
	void OnClientDisconnected(int client);
public: //what derived must implement
	CBaseBuiltinVotePlayer *GetVotePlayer(int client);
	bool DoClientVote(int client, IBaseBuiltinVote *vote, IBuiltinVoteHandler *bvh);
	bool DoClientVote(int clients[], unsigned int num_clients, IBaseBuiltinVote *vote, IBuiltinVoteHandler *bvh);
	void SendDisplay(int client, IBaseBuiltinVote *vote);
	void SendDisplay(int clients[], unsigned int num_clients, IBaseBuiltinVote *vote);
public: //what derived may implement
	/*
	virtual bool DoClientVote(int client,
		CBaseBuiltinVote *vote,
		unsigned int first_item,
		IBuiltinVoteHandler *bvh,
		unsigned int time);
		*/
	//virtual bool DoClientVote(int client, CBaseBuiltinVote *vote, IBuiltinVoteHandler *bvh, unsigned int time);
	//virtual void AddClientToWatch(int client);
	//virtual void RemoveClientFromWatch(int client);
	//virtual void ProcessWatchList();
public: //helpers
	void CancelVote(CBaseBuiltinVote *vote);
	//void ClientPressedKey(int client, unsigned int key_press);
protected:
	bool RedoClientVote(int client);
protected:
//	FastLink<int> m_WatchList;
	Handle_t m_hHandle;
	CBaseBuiltinVotePlayer *m_players;
};


class CBaseBuiltinVote : public IBaseBuiltinVote
{
public:
	CBaseBuiltinVote(IBuiltinVoteHandler *pHandler, IBuiltinVoteStyle *pStyle, BuiltinVoteType type, IdentityToken_t *pOwner);
	virtual ~CBaseBuiltinVote();
public:
	bool AppendItem(const char *info, const char *display=0);
	bool InsertItem(unsigned int position, const char *info, const char *display=0);
	bool RemoveItem(unsigned int position);
	void RemoveAllItems();
	virtual const char *GetItemName(unsigned int position);
	virtual const char *GetItemDisplay(unsigned int position);
	virtual unsigned int GetItemCount();
	virtual IBuiltinVoteStyle *GetDrawStyle();
	virtual void SetArgument(const char *message);
	virtual const char *GetArgument();
	virtual BuiltinVoteType GetVoteType();
	virtual void Cancel();
	virtual void Destroy(bool releaseHandle);
	virtual void Cancel_Finally() =0;
	virtual Handle_t GetHandle();
	virtual unsigned int GetVoteOptionFlags();
	virtual void SetVoteOptionFlags(unsigned int flags);
	virtual void SetTeam(short team);
	virtual short GetTeam();
	virtual void SetInitiator(unsigned char client);
	virtual unsigned char GetInitiator();
	virtual void DisplayVotePass(const char* winner="") =0;
	virtual void DisplayVoteFail(BuiltinVoteFailReason reason=BuiltinVoteFail_Generic) =0;
	virtual void DisplayVoteFail(int client, BuiltinVoteFailReason reason=BuiltinVoteFail_Generic) =0;
	//virtual void DisplayCallVoteFail(int client, BuiltinCallVoteFailReason reason, short param1) =0;
	virtual void ClientSelectedItem(int client, unsigned int item) =0;
	virtual IBuiltinVoteHandler *GetHandler();
	virtual bool Display(int clients[], unsigned int num_clients) =0;
	bool Display(int client);
	unsigned int GetBaseMemUsage();
protected:
	unsigned int GetAllPlayers(cell_t clients[]);
	bool DoClientVote(int clients[], unsigned int num_clients);
private:
	void InternalDelete();
protected:
	String m_Argument;
	IBuiltinVoteStyle *m_pStyle;
	BaseStringTable m_Strings;
	CVector<CItem> m_items;
	bool m_bShouldDelete;
	bool m_bCancelling;
	IdentityToken_t *m_pOwner;
	bool m_bDeleting;
	bool m_bWillFreeHandle;
	Handle_t m_hHandle;
	IBuiltinVoteHandler *m_pHandler;
	unsigned int m_nFlags;
	BuiltinVoteType m_voteType;
	byte m_initiator;
	short m_team;
	bool m_bResultDisplayed;
};

extern BuiltinVoteManager g_BuiltinVotes;
extern IGameEventManager2 *events;

#endif //_INCLUDE_BUILTINVOTE_BASE_H
