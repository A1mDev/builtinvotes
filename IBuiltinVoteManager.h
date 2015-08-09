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

#ifndef _INCLUDE_BUILTINVOTEMANAGER_H
#define _INCLUDE_BUILTINVOTEMANAGER_H

#include <IShareSys.h>
#include <IHandleSys.h>
#include <sh_vector.h>

#define SMINTERFACE_BUILTINVOTES_NAME		"IBuiltinVoteManager"
#define SMINTERFACE_BUILTINVOTES_VERSION	1

#define SERVER_CLIENT_INDEX 99 // Defined by TF2, may be same in L4D/L4D2

#define BUILTINVOTES_VOTE_NO				0
#define BUILTINVOTES_VOTE_YES				1

//#define MAX_TRANSLATE_PARAMS				32

using namespace SourceHook;

namespace SourceMod
{
	class IBaseBuiltinVote;
	class IBuiltinVoteHandler;

	struct vote_states_t
	{
		unsigned int apiVers;		/**< Must be filled with API version */
		IBaseBuiltinVote *vote;		/**< Vote pointer, or NULL if there is only a display */
		IBuiltinVoteHandler *bvh;	/**< Vote callbacks handler */
	};

	/**
	 * @brief Contains information about a vote result.
	 */
	struct menu_vote_result_t
	{
		unsigned int num_clients;		/**< Number of clients the menu was displayed to */
		unsigned int num_votes;			/**< Number of votes received */
		struct menu_client_vote_t
		{
			int client;					/**< Client index */
			int item;					/**< Item # (or -1 for none) */
		} *client_list;					/**< Array of size num_clients */
		unsigned int num_items;			/**< Number of items voted for */
		struct menu_item_vote_t
		{
			unsigned int item;			/**< Item index */
			unsigned int count;			/**< Number of votes */
		} *item_list;					/**< Array of size num_items, sorted by count, 
											 descending */
	};

	/**
	 * @brief Reasons a vote can end.
	 */
	enum BuiltinVoteEndReason
	{
		BuiltinVoteEnd_Done = -1,			/**< Voting finished */
		BuiltinVoteEnd_Cancelled = -2,		/**< Voting was cancelled */
	};

	/**
	 * @brief Vote types. These are mapped to translation strings and pass strings by VoteStart and VotePass handlers
	 */
	enum BuiltinVoteType
	{
		BuiltinVoteType_ChgCampaign = 0,		/**< L4D/L4D2: Yes/No, argument is campaign name */
		BuiltinVoteType_ReturnToLobby = 1,		/**< L4D/L4D2: Yes/No, argument ignored */
		BuiltinVoteType_ChgDifficulty = 2,		/**< L4D/L4D2: Yes/No, argument is difficulty level */
		BuiltinVoteType_Custom_YesNo = 3,		/**< Yes/No, argument is vote text. */
		BuiltinVoteType_Kick = 4,				/**< Yes/No, argument is player userid */
		BuiltinVoteType_Restart = 5,			/**< Yes/No, argument ignored */
		BuiltinVoteType_ChgLevel = 6,			/**< Yes/No, argument is level number in L4D/L4D2 or map name in TF2 */
		BuiltinVoteType_KickIdle = 7,			/**< TF2: Yes/No, argument is player userid */
		BuiltinVoteType_KickScamming = 8,		/**< TF2: Yes/No, argument is player userid */
		BuiltinVoteType_KickCheating = 9,		/**< TF2: Yes/No, argument is player userid */
		BuiltinVoteType_NextLevel = 10,			/**< TF2: Yes/No, argument is map name */
		BuiltinVoteType_NextLevelMult = 11,		/**< TF2: Multiple-choice, argument ignored */
		BuiltinVoteType_ScrambleNow = 12,		/**< TF2: Yes/No, argument ignored */
		BuiltinVoteType_ScrambleEnd = 13,		/**< TF2: Yes/No, argument ignored */
		BuiltinVoteType_Custom_Mult = 14,		/**< TF2: Multiple-choice, argument is vote text. */
		BuiltinVoteType_Alltalk = 15,			/**< L4D2: Yes/No, argument is ignored and set automagically based on the current sv_alltalk setting */
	};

	/**
	 * @brief Reasons a vote can fail or be cancelled.
	 *
	 * These numbers are shared with the next enum, but are split based on purpose
	 * Specifically, this set can be sent to VoteFailed.
	 */
	enum BuiltinVoteFailReason
	{
		BuiltinVoteFail_Generic = 0,		/**< Vote was generically cancelled. */
		BuiltinVoteFail_Loses = 3,			/**< No votes outnumbered Yes votes */
		BuiltinVoteFail_NotEnoughVotes = 4,	/**< Vote did not receive enough votes. */
	};

	/**
	 * @brief Reasons a vote can not be called.
	 *
	 * These numbers are shared with the previous enum, but are split based on purpose
	 * Specifically, this set can be sent to CallVoteFailed.
	 */
	enum BuiltinCallVoteFailReason
	{
		BuiltinCallVoteFail_PlayersLoading = 1,		/**< Cannot call vote while players are loading, argument ignored */
		BuiltinCallVoteFail_Recent = 2,				/**< Player called a vote recently, argument is number of seconds until player can next call a vote */
		BuiltinCallVoteFail_IssueDisabled = 5,		/**< Server has disabled that issue, argument ignored */
		BuiltinCallVoteFail_MapNotFound = 6,		/**< Map does not exist, argument ignored */
		BuiltinCallVoteFail_MapNotSpecified = 7,	/**< Player must specify a map, argument ignored */
		BuiltinCallVoteFail_RecentlyFailed = 8,		/**< This vote failed recently and players must wait to call it again, argument ignored */
		BuiltinCallVoteFail_WrongTeam = 9,			/**< Player's team cannot call this vote, argument ignored */
		BuiltinCallVoteFail_WaitingForPlayers = 10,	/**< Cannot call vote during Waiting For Players, argument ignored */
		BuiltinCallVoteFail_Unknown = 11,			/**< This reason is currently bugged, do not use, argument unknown */
		BuiltinCallVoteFail_KickAdmin = 12,			/**< Cannot kick server admin, argument ignored */
		BuiltinCallVoteFail_ScramblePending = 13,	/**< Scramble is pending; intended for scramble votes, argument ignored */
		BuiltinCallVoteFail_Spectator = 14,			/**< Spectators cannot call votes, argument ignored */
		BuiltinCallVoteFail_NextLevelSet = 15,		/**< Next level already set, argument ignored */
	};

	#define VOTE_TIME_FOREVER			0		/**< Vote should be displayed as long as possible */
	#define VOTEFLAG_NO_REVOTES			(1<<0)	/**< Players cannot change their votes */

	class IBuiltinVoteStyle
	{
	public:
		/**
		 * @brief Returns the style API version.
		 *
		 * @return				API version.
		 */
		virtual unsigned int GetStyleAPIVersion()
		{
			return SMINTERFACE_BUILTINVOTES_VERSION;
		}

		/**
		 * @brief Returns the name of the vote style.
		 *
		 * @return				String containing the style name.
		 */
		virtual const char *GetStyleName() =0;

		/**
		 * @brief Check if this type of vote is supported.
		 *
		 * This should be done before creating the vote object.
		 *
		 * @param type			BuiltinVoteType type
		 *
		 * @return				true if this vote type is supported, false if it isn't.
		 */
		virtual bool CheckVoteType(BuiltinVoteType type) =0;

		/**
		 * @brief Creates an IBaseBuiltinVote object of this style.
		 *
		 * Note: the object should be freed using IBaseBuiltinVote::Destroy
		 *
		 * @param handler		IBuiltinVoteHandler pointer.
		 * @param type			Vote type.  This is game dependent, and games will reject
		 *						types they don't support.
		 * @param pOwner		Optional IdentityToken_t owner for handle
		 *						creation.
		 * @return				An IBaseBuiltinVote pointer.
		 */
		virtual IBaseBuiltinVote *CreateVote(IBuiltinVoteHandler *handler, BuiltinVoteType type, IdentityToken_t *pOwner=NULL) =0;


		/**
		 * @brief Prepare and display vote to users.
		 *
		 * @param clients		Clients indexes to display to.
		 * @param num_clients	Number of clients to send menu to.
		 * @param vote			Vote object
		 * @param bvh			Vote handler
		 *
		 * @return				True if vote could be started, false if not
		 */
		virtual bool DoClientVote(int clients[], unsigned int num_clients, IBaseBuiltinVote *vote, IBuiltinVoteHandler *bvh) =0;

		/**
		 * @brief Returns the maximum number of items.
		 *
		 * Vote implementations must return >= 2.  Styles with only 1 or 0
		 * items are not valid.
		 *
		 * @return				Number of items.
		 */
		virtual unsigned int GetMaxItems() =0;

		/**
		 * @brief Returns a Handle for the IBuiltinVoteStyle object
		 *
		 * @return				Handle_t pointing to this object.
		 */
		virtual Handle_t GetHandle() =0;

		/**
		 * @brief For the Handle system, returns approximate memory usage.
		 *
		 * @return				Approximate number of bytes being used.
		 */
		virtual unsigned int GetApproxMemUsage() =0;
	};

	class IBaseBuiltinVote
	{
	public:
		/**
		 * @brief Appends an item to the end of a vote.
		 *
		 * @param info			Item information string.
		 * @param display		Item display string.
		 * @return				True on success, false on item limit reached.
		 */
		virtual bool AppendItem(const char *info, const char *display=0) =0;

		/**
		 * @brief Inserts an item into the vote before a certain position; 
		 * the new item will be at the given position and all next items 
		 * pushed forward.
		 *
		 * @param position		Position, starting from 0.
		 * @param info			Item information string.
		 * @param display		Item display string.
		 * @return				True on success, false on invalid menu position
		 */
		virtual bool InsertItem(unsigned int position, const char *info, const char *display=0) =0;

		/**
		 * @brief Removes an item from the vote.
		 *
		 * @param position		Position, starting from 0.
		 * @return				True on success, false on invalid menu position.
		 */
		virtual bool RemoveItem(unsigned int position) =0;

		/**
		 * @brief Removes all items from the vote.
		 */
		virtual void RemoveAllItems() =0;

		/**
		 * @brief Returns an item's name.
		 *
		 * @param position		Position, starting from 0.
		 * @return				Info string pointer, or NULL if position was invalid.
		 */
		virtual const char *GetItemName(unsigned int position) =0;

		/**
		 * @brief Returns an item's display string.
		 *
		 * @param position		Position, starting from 0.
		 * @return				Display string pointer, or NULL if position was invalid.
		 */
		virtual const char *GetItemDisplay(unsigned int position) =0;

		/**
		 * @brief Returns the number of items.
		 *
		 * @return				Number of items in the menu.
		 */
		virtual unsigned int GetItemCount() =0;

		/**
		 * @brief Return the vote style.
		 *
		 * @return				Vote style.
		 */
		virtual IBuiltinVoteStyle *GetDrawStyle() =0;

		/**
		 * @brief Sets the vote's display title/message.
		 * Only valid for Custom votes
		 *
		 * @param message		Message (format options allowed).
		 */
		virtual void SetArgument(const char *message) =0;

		/**
		 * @brief Returns the vote's display/title message.
		 *
		 * @return				Message string.
		 */
		virtual const char *GetArgument() =0;

		/**
		 * @brief Returns the vote's type
		 */
		virtual BuiltinVoteType GetVoteType() =0;

		/**
		 * @brief Set the team for the vote. -1 = all teams
		 *
		 * @param team			Team Index, game dependent.
		 */
		virtual void SetTeam(short team) =0;

		/**
		 * @brief Returns the team this vote is for. -1 for all teams.
		 *
		 * @return				Team Index, game dependent.
		 */
		virtual short GetTeam() =0;

		/**
		 * @brief Set who initiated the vote
		 *
		 * @param initiator		Player index or 99 for server.
		 */
		virtual void SetInitiator(unsigned char client) =0;

		/**
		 * @brief Set who initiated the vote
		 *
		 * @param initiator		Player index or 99 for server.
		 */
		virtual unsigned char GetInitiator() =0;

		/**
		 * @brief Sends the vote to a client.
		 *
		 * @param client		Client index to display to.
		 * @param time			Time to hold menu for.
		 * @param alt_handler	Alternate IMenuHandler.
		 * @return				True on success, false otherwise.
		 */
		virtual bool Display(int client) =0;

		/**
		 * @brief Sends the vote to clients.
		 *
		 * @param clients		Clients indexes to display to.
		 * @param num_clients	Number of clients to send menu to.
		 * @param time			Time to hold menu for.
		 * @param alt_handler	Alternate IMenuHandler.
		 * @return				True on success, false otherwise.
		 */
		virtual bool Display(int clients[], unsigned int num_clients) =0;

		/**
		 * @brief Destroys the vote and frees all associated resources.
		 *
		 * @param releaseHandle	If true, the Handle will be released
		 *						in the destructor.  This should be set
		 *						to true except for IHandleTypeDispatch
		 *						destructors.
		 */
		virtual void Destroy(bool releaseHandle=true) =0;

		/**
		 * @brief Cancels the vote on all client's displays.  While the vote is
		 * being cancelled, the vote may not be re-displayed to any clients.
		 * If a vote is currently active, it will be cancelled as well.
		 *
		 * @return				Number of votes cancelled.
		 */
		virtual void Cancel() =0;

		/**
		 * @brief Returns the vote's Handle.  The Handle is automatically
		 * removed when the vote is destroyed.
		 *
		 * @return				Handle_t handle value.
		 */
		virtual Handle_t GetHandle() =0;
		
		/**
		 * @brief Returns vote option flags.
		 *
		 * @return				Vote option flags.
		 */
		virtual unsigned int GetVoteOptionFlags() =0;

		/**
		 * @brief Sets vote option flags.
		 *
		 * @param flags			Vote option flags.
		 */
		virtual void SetVoteOptionFlags(unsigned int flags) =0;

		/**
		 * @brief Display the vote passed screen.
		 *
		 * @param winner		Vote winner.
		 */
		virtual void DisplayVotePass(const char *winner="") =0;

		/**
		 * @brief Display the vote passed screen.
		 *
		 * @param winner		Vote winner.
		 */
		virtual void DisplayVotePass(const char *translation, const char* winner) =0;

		/**
		 * @brief Display the vote failed screen.
		 *
		 * @param reason		BuiltinVoteFailReason with the correct reason. Only used for OB.
		 */
		virtual void DisplayVoteFail(BuiltinVoteFailReason reason=BuiltinVoteFail_Generic) =0;

		/**
		 * @brief Display the vote failed screen.
		 *
		 * @param client		Client to display screen to.
		 * @param reason		BuiltinVoteFailReason with the correct reason. Only used for OB.
		 */
		virtual void DisplayVoteFail(int client, BuiltinVoteFailReason reason=BuiltinVoteFail_Generic) =0;

		/**
		 * @brief Display the callvote failed screen to a single user.
		 *
		 * @param client		Client whose vote can not start
		 * @param reason		BuiltinVoteFailReason with the correct reason.
		 * @param param1		Only used for BuiltinCallVoteFail_Recent.
		 */
		//virtual void DisplayCallVoteFail(int client, BuiltinCallVoteFailReason reason, short param1=0) =0;

		/**
		 * @brief The client chose an item. Game-specific logic.
		 * 
		 * @param client		Client who cast the vote
		 * @param item			The item they chose
		 */
		virtual void ClientSelectedItem(int client, unsigned int item) =0;

		/**
		 * @brief Update vote counts for games that support it (L4D/L4D2, noop on TF2)
		 *
		 * @param items			Number of items
		 * @param votes			Votes vector
		 * @param totalClients	Number of clients in vote
		 * @return				true if supported by game, false if not
		 */
		virtual bool UpdateVoteCounts(unsigned int items, CVector<unsigned int> votes, unsigned int totalClients) =0;

		/**
		 * @brief Returns the vote's handler.
		 *
		 * @return				IBuiltinVoteHandler of the menu.
		 */
		virtual IBuiltinVoteHandler *GetHandler() =0;

		/**
		 * @brief For the Handle system, returns approximate memory usage.
		 *
		 * @return				Approximate number of bytes being used.
		 */
		virtual unsigned int GetApproxMemUsage() =0;
	};


	class IBuiltinVoteHandler
	{
	public:
		/**
		 * @brief Returns the vote api verison.
		 *
		 * @return				BuiltinVotes API version.
		 */
		virtual unsigned int GetBuiltinVoteAPIVersion()
		{
			return SMINTERFACE_BUILTINVOTES_VERSION;
		}

		/**
		 * @brief Called when an item is selected.
		 *
		 * @param vote			Vote pointer.
		 * @param client		Client that selected the item.
		 * @param item			Item number.
		 */
		virtual void OnVoteSelect(IBaseBuiltinVote *vote, int client, unsigned int item)
		{
		}

		/**
		 * @brief A display/selection cycle has ended.
		 *
		 * @param vote			Vote pointer.
		 * @param reason		BuiltinVoteEndReason reason.
		 */
		virtual void OnVoteEnd(IBaseBuiltinVote *vote, BuiltinVoteEndReason reason)
		{
		}

		/**
		 * @brief Called when the vote object is destroyed.
		 *
		 * @param menu			Vote pointer.
		 */
		virtual void OnVoteDestroy(IBaseBuiltinVote *vote)
		{
		}

		/**
		 * @brief Called when a vote has been started and displayed to 
		 * clients.  This is called after OnVoteStart() and OnVoteDisplay(),
		 * but before OnVoteSelect().
		 *
		 * @param vote			Vote pointer.
		 */
		virtual void OnVoteStart(IBaseBuiltinVote *vote)
		{
		}

		/**
		 * @brief Called when a vote ends.  This is automatically called by the 
		 * wrapper, and never needs to called from a style implementation.  
		 *
		 * This function does not replace OnVoteEnd(), nor does it have the 
		 * same meaning as OnVoteEnd(), meaning you should not destroy a vote
		 * while it is in this function.
		 *
		 * @param vote			Vote pointer.
		 * @param results		Vote results.
		 */
		virtual void OnVoteResults(IBaseBuiltinVote *vote, const menu_vote_result_t *results)
		{
		}

		/**
		 * @brief Called when a vote is cancelled.  If this is called, then 
		 * OnVoteResults() will not be called.  In both cases, OnVoteEnd will 
		 * always be called.
		 *
		 * @param vote			Vote pointer.
		 * @param reason		BuiltinVoteFailReason reason.
		 */
		virtual void OnVoteCancel(IBaseBuiltinVote *vote, BuiltinVoteFailReason reason)
		{
		}

		/**
		 * @brief Call to set private handler stuff.
		 *
		 * @param option		Option name.
		 * @param data			Private data.
		 * @return				True if set, false if invalid or unrecognized.
		 */
		virtual bool OnSetHandlerOption(const char *option, const void *data)
		{
			return false;
		}

	};

	class IBuiltinVoteManager : public SMInterface
	{
	public:
		virtual const char *GetInterfaceName()
		{
			return SMINTERFACE_BUILTINVOTES_NAME;
		}
		virtual unsigned int GetInterfaceVersion()
		{
			return SMINTERFACE_BUILTINVOTES_VERSION;
		}
	public:

		/**
		 * @brief Displays a vote.
		 *
		 * @param menu			IBaseBuiltinVote pointer.
		 * @param num_clients	Number of clients to display to.
		 * @param clients		Client index array.
		 * @param max_time		Maximum time to hold menu for.
		 * @param flags			Vote flags.
		 * @return				True on success, false if a vote is in progress.
		 */
		virtual bool StartVote(IBaseBuiltinVote *vote,
								unsigned int num_clients,
								int clients[],
								unsigned int max_time,
								unsigned int flags=0) =0;

		/**
		 * @brief Returns whether or not a vote is in progress.
		 *
		 * @return				True if a vote is in progress, false otherwise.
		 */
		virtual bool IsVoteInProgress() =0;

		/**
		 * @brief Cancels the vote in progress.  This calls IBaseBuiltinVote::Cancel().
		 */
		virtual void CancelVoting() =0;

		/**
		 * @brief Returns the remaining vote delay from the last menu.  This delay is 
		 * a suggestion for all public votes, and is not enforced.
		 *
		 * @return				Number of seconds to wait.
		 */
		virtual unsigned int GetRemainingVoteDelay() =0;

		/**
		 * @brief Returns whether a client is in the "allowed to vote" pool determined 
		 * by the initial call to StartVote().
		 *
		 * @param client		Client index.
		 * @return				True if client is allowed to vote, false on failure.
		 */
		virtual bool IsClientInVotePool(int client) =0;

		/**
		 * @brief Redraws the current vote menu to a client in the voting pool.
		 *
		 * @param client		Client index.
		 * @return				True on success, false if client is not allowed to vote.
		 */
		virtual bool RedrawClientVote(int client) =0;

		/**
		 * @brief Redraws the current vote menu to a client in the voting pool.
		 *
		 * @param client		Client index.
		 * @param revotes		True to allow revotes, false otherwise.
		 * @return				True on success, false if client is not allowed to vote.
		 */
		virtual bool RedrawClientVote2(int client, bool revotes) =0;
	};

};

#endif //_INCLUDE_BUILTINVOTEMANAGER_H
