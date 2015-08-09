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

#include <string.h>
#include <stdlib.h>
#include "BuiltinVoteHandler.h"
#include <ITranslator.h>
#include <IPlayerHelpers.h>
#include <icvar.h>
#include <convar.h>

#define VOTE_DELAY_TIME 3

float g_next_vote = 0.0f;

ConVar *sm_vote_hintbox;
ConVar *sm_vote_chat;
ConVar *sm_vote_console;
ConVar *sm_vote_client_console;
ConVar *sm_vote_delay;

#define VOTE_NOT_VOTING -2
#define VOTE_PENDING -1

void OnVoteDelayChange(IConVar *cvar, const char *value, float flOldValue)
{
	/* See if the new vote delay isn't something we need to account for */
	if (sm_vote_delay->GetFloat() < 1.0f)
	{
		g_next_vote = 0.0f;
		return;
	}

	/* If there was never a last vote, ignore this change */
	if (g_next_vote < 0.1f)
	{
		return;
	}

	/* Subtract the original value, then add the new one. */
	g_next_vote -= (float)atof(value);
	g_next_vote += sm_vote_delay->GetFloat();
}

/* Wrapper for translator->FormatString for error handling.
 * Adapted from SourceMod Translator.cpp's CoreTranslate.
 */
bool Translate(char *buffer, 
			   size_t maxlength,
			   const char *format,
			   unsigned int numparams,
			   size_t *pOutLength,
			   ...)
{
	va_list ap;
	unsigned int i;
	const char *fail_phrase;
	void *params[MAX_TRANSLATE_PARAMS];

	if (numparams > MAX_TRANSLATE_PARAMS)
	{
		assert(false);
		return false;
	}

	va_start(ap, pOutLength);
	for (i = 0; i < numparams; i++)
	{
		params[i] = va_arg(ap, void *);
	}
	va_end(ap);

	if (!corePhrases->FormatString(buffer,
		maxlength, 
		format,
		params,
		numparams,
		pOutLength,
		&fail_phrase))
	{
		if (fail_phrase != NULL)
		{
#ifdef _DEBUG
			smutils->LogMessage(myself, "Could not find phrase: %s", fail_phrase);
#endif
			smutils->LogError(myself, "Could not find phrase: %s", fail_phrase);
		}
		else
		{
#ifdef _DEBUG
			fail_phrase = (const char*)params[0];
			int client = *(int *)params[1];
			if (client > 0)
			{
				IGamePlayer *pPlayer = playerhelpers->GetGamePlayer(client);
				unsigned int language = translator->GetClientLanguage(client);
				Translation pTrans;
				TransError error = corePhrases->FindTranslation(fail_phrase, language, &pTrans);
				const char *errorString;

				switch(error)
				{
				case Trans_Okay:
					errorString = "No Error";
					break;

				case Trans_BadLanguage:
					errorString = "Invalid Language ID";
					break;

				case Trans_BadPhrase:
					errorString = "Invalid Phrase";
					break;

				case Trans_BadPhraseLanguage:
					errorString = "Phrase not available in language";
					break;

				case Trans_BadPhraseFile:
					errorString = "Phrase file was unreadable";
					break;

				}

				smutils->LogMessage(myself, "Unknown fatal error while translating phrase \"%s\" for client \"%s\" using language %d", fail_phrase, pPlayer->GetName(), language);
				smutils->LogMessage(myself, "FindTranslation returned: %s.", errorString);
			}
			else
			{
				smutils->LogMessage(myself, "Unknown fatal error while translating phrase \"%s\" for server", fail_phrase);
			}
#endif
			smutils->LogError(myself, "Unknown fatal error while translating a phrase.");
		}

		return false;
	}

	return true;
}

unsigned int BuiltinVoteHandler::GetRemainingVoteDelay()
{
	if (g_next_vote <= gpGlobals->curtime)
	{
		return 0;
	}

	return (unsigned int)(g_next_vote - gpGlobals->curtime);
}

void BuiltinVoteHandler::OnLoad()
{
	sm_vote_hintbox = icvar->FindVar("sm_vote_progress_hintbox");
	sm_vote_chat = icvar->FindVar("sm_vote_progress_chat");
	sm_vote_console = icvar->FindVar("sm_vote_progress_console");
	sm_vote_client_console = icvar->FindVar("sm_vote_progress_client_console");
	sm_vote_delay = icvar->FindVar("sm_vote_delay");

	// Install a callback for sm_vote_delay
	sm_vote_delay->InstallChangeCallback(OnVoteDelayChange);

	playerhelpers->AddClientListener(this);
}

void BuiltinVoteHandler::OnUnload()
{
	playerhelpers->RemoveClientListener(this);
}

void BuiltinVoteHandler::OnMapStart()
{
	g_next_vote = 0.0f;

	CancelVoting();
}

unsigned int BuiltinVoteHandler::GetBuiltinVoteAPIVersion()
{
	return m_pHandler->GetBuiltinVoteAPIVersion();
}

void BuiltinVoteHandler::OnClientDisconnected(int client)
{
	if (!IsVoteInProgress())
	{
		return;
	}

	/* Wipe out their vote if they had one.  We have to make sure the the the
	 * newly connected client is not allowed to vote.
	 */
	int item;
	if ((item = m_ClientVotes[client]) >= VOTE_PENDING)
	{
		if (item >= 0)
		{
			assert((unsigned)item < m_Items);
			assert(m_Votes[item] > 0);
			m_Votes[item]--;
		}
		m_ClientVotes[client] = VOTE_NOT_VOTING;
	}
}

bool BuiltinVoteHandler::IsVoteInProgress()
{
	return (m_pCurVote != NULL);
}

bool BuiltinVoteHandler::StartVote(IBaseBuiltinVote *vote,
								   unsigned int num_clients,
								   int clients[],
								   unsigned int max_time,
								   unsigned int flags)
{
	if (!InitializeVoting(vote, vote->GetHandler(), max_time, flags))
	{
		return false;
	}

	/* Note: we can use game time and not universal time because
	 * if we're voting then players are in-game.
	 */

	float fVoteDelay = sm_vote_delay->GetFloat();
	if (fVoteDelay < 1.0)
	{
		g_next_vote = 0.0;
	}
	else
	{
		/* This little trick break for infinite votes!
		 * However, we just ignore that since those 1) shouldn't exist and
		 * 2) people must be checking IsVoteInProgress() beforehand anyway.
		 */
		g_next_vote = gpGlobals->curtime + fVoteDelay + (float)max_time;
	}

	m_fStartTime = gpGlobals->curtime;
	m_nVoteTime = max_time;
	m_TimeLeft = max_time;

	unsigned int clientCount = 0;

	for (unsigned int i=0; i<num_clients; i++)
	{
		if (clients[i] < 1 || clients[i] > 256)
		{
			continue;
		}
		
		m_ClientVotes[clients[i]] = VOTE_PENDING;
		clientCount++;
		// NOTE: Good idea would be to send the vote message out just once
		// vote->Display(clients[i], max_time, this);
	}

	m_Clients = clientCount;

	m_pCurVote->UpdateVoteCounts(m_Items, m_Votes, clientCount); // Same thing as BuildVoteLeaders, but for L4D/L4D2 internals

	vote->GetDrawStyle()->DoClientVote(clients, num_clients, vote, vote->GetHandler());
	//vote->Display(clients, num_clients);

	StartVoting();

	return true;
}

bool BuiltinVoteHandler::IsClientInVotePool(int client)
{
	if (client < 1
		|| client > playerhelpers->GetMaxClients()
		|| m_pCurVote == NULL)
	{
		return false;
	}

	return (m_ClientVotes[client] > VOTE_NOT_VOTING);
}


bool BuiltinVoteHandler::GetClientVoteChoice(int client, unsigned int *pItem)
{
	if (!IsClientInVotePool(client)
		|| m_ClientVotes[client] == VOTE_PENDING)
	{
		return false;
	}

	*pItem = m_ClientVotes[client];

	return true;
}

bool BuiltinVoteHandler::RedrawToClient(int client, bool revotes)
{
	if (!IsClientInVotePool(client))
	{
		return false;
	}

	if (m_ClientVotes[client] >= 0)
	{
		if ((m_VoteFlags & VOTEFLAG_NO_REVOTES) == VOTEFLAG_NO_REVOTES || !revotes || m_nVoteTime <= VOTE_DELAY_TIME)
		{
			return false;
		}
		assert((unsigned)m_ClientVotes[client] < m_Items);
		assert(m_Votes[m_ClientVotes[client]] > 0);

		// Display the fail screen for a second.
		m_pCurVote->DisplayVoteFail(client, BuiltinVoteFail_Generic);

		m_Clients++;
		m_Votes[m_ClientVotes[client]]--;
		m_ClientVotes[client] = VOTE_PENDING;
		m_Revoting[client] = true;
		m_NumVotes--;
	}

	RedrawTimer *redrawTimer = new RedrawTimer(client, m_pCurVote);

	timersys->CreateTimer(redrawTimer, VOTE_DELAY_TIME, NULL, TIMER_FLAG_NO_MAPCHANGE);

	return true;
	//return m_pCurVote->Display(client);
}

bool BuiltinVoteHandler::InitializeVoting(IBaseBuiltinVote *vote,
										  IBuiltinVoteHandler *handler,
										  unsigned int time,
										  unsigned int flags)

{
	if (IsVoteInProgress())
	{
		return false;
	}

	InternalReset();

	/* Mark all clients as not voting */
	for (int i=1; i<=gpGlobals->maxClients; i++)
	{
		m_ClientVotes[i] = VOTE_NOT_VOTING;
		m_Revoting[i] = false;
	}

	m_Items = vote->GetItemCount();

	if (m_Votes.size() < (size_t)m_Items)
	{
		/* Only clear the items we need to... */
		size_t size = m_Votes.size();
		for (size_t i=0; i<size; i++)
		{
			m_Votes[i] = 0;
		}
		m_Votes.resize(m_Items, 0);
	}
	else
	{
		for (unsigned int i=0; i<m_Items; i++)
		{
			m_Votes[i] = 0;
		}
	}

	m_bWasCancelled = false;
	m_pCurVote = vote;
	m_VoteTime = time;
	m_VoteFlags = flags;
	m_pHandler = handler;

	return true;
}

void BuiltinVoteHandler::StartVoting()
{
	if (!m_pCurVote)
	{
		return;
	}

	m_bStarted = true;

	m_pHandler->OnVoteStart(m_pCurVote);

	m_displayTimer = timersys->CreateTimer(this, 1.0, NULL, TIMER_FLAG_REPEAT|TIMER_FLAG_NO_MAPCHANGE);

	/* By now we know how many clients were set.
	 * If there are none, we should end IMMEDIATELY.
	 */
	if (m_Clients == 0)
	{
		EndVoting();
	}

	m_TotalClients = m_Clients;
}

void BuiltinVoteHandler::DecrementPlayerCount()
{
	assert(m_Clients > 0);

	m_Clients--;

	if (m_bStarted && m_Clients == 0)
	{
		EndVoting();
	}
}

int SortVoteItems(const void *item1, const void *item2)
{
	return ((menu_vote_result_t::menu_item_vote_t *)item2)->count
		- ((menu_vote_result_t::menu_item_vote_t *)item1)->count;
}

void BuiltinVoteHandler::EndVoting()
{
	/* Set when the next delay ends.  We ignore cancellation because a vote
	 * was, at one point, displayed, which is all that counts.  However, we
	 * do recalculate the time just in case the vote had no time limit.
	 */
	float fVoteDelay = sm_vote_delay->GetFloat();
	if (fVoteDelay < 1.0)
	{
		g_next_vote = 0.0;
	}
	else
	{
		g_next_vote = gpGlobals->curtime + fVoteDelay;
	}

	if (m_displayTimer)
	{
		timersys->KillTimer(m_displayTimer);
	}

	if (m_bCancelled)
	{
		/* If we were cancelled, don't bother tabulating anything.
		 * Reset just in case someone tries to redraw, which means
		 * we need to save our states.
		 */
		IBaseBuiltinVote *vote = m_pCurVote;
		IBuiltinVoteHandler *handler = m_pHandler;
		InternalReset();
		handler->OnVoteCancel(vote, BuiltinVoteFail_Generic);
		handler->OnVoteEnd(vote, BuiltinVoteEnd_Cancelled);
		return;
	}

	menu_vote_result_t results;
	menu_vote_result_t::menu_client_vote_t client_vote[256];
	menu_vote_result_t::menu_item_vote_t item_vote[256];

	memset(&results, 0, sizeof(results));

	/* Build the item list */
	for (unsigned int i=0; i<m_Items; i++)
	{
		if (m_Votes[i] > 0)
		{
			item_vote[results.num_items].count = m_Votes[i];
			item_vote[results.num_items].item = i;
			results.num_votes += m_Votes[i];
			results.num_items++;
		}
	}
	results.item_list = item_vote;

	if (!results.num_votes)
	{
		IBaseBuiltinVote *vote = m_pCurVote;
		IBuiltinVoteHandler *handler = m_pHandler;
		InternalReset();
		handler->OnVoteCancel(vote, BuiltinVoteFail_NotEnoughVotes);
		handler->OnVoteEnd(vote, BuiltinVoteEnd_Cancelled);
		return;
	}

	/* Build the client list */
	for (int i=1; i<=gpGlobals->maxClients; i++)
	{
		if (m_ClientVotes[i] >= VOTE_PENDING)
		{
			client_vote[results.num_clients].client = i;
			client_vote[results.num_clients].item = m_ClientVotes[i];
			results.num_clients++;
		}
	}
	results.client_list = client_vote;

	/* Sort the item list descending like we promised */
	qsort(item_vote,
		results.num_items,
		sizeof(menu_vote_result_t::menu_item_vote_t),
		SortVoteItems);

	/* Save states, then clear what we've saved.
	 * This makes us re-entrant, which is always the safe way to go.
	 */
	IBaseBuiltinVote *vote = m_pCurVote;
	IBuiltinVoteHandler *handler = m_pHandler;
	InternalReset();

	/* Send vote info */
	handler->OnVoteResults(vote, &results);
	handler->OnVoteEnd(vote, BuiltinVoteEnd_Done);
}

// Useless because we don't track this per-user
/*
void BuiltinVoteHandler::OnVoteStart(IBaseBuiltinVote *vote)
{
	m_Clients++;
}

void BuiltinVoteHandler::OnVoteEnd(IBaseBuiltinVote *vote, BuiltinVoteEndReason reason)
{
	DecrementPlayerCount();
}
*/

// This version of Cancel can't be called, as users can't cancel the vote once shown to them
/*
void BuiltinVoteHandler::OnVoteCancel(SourceMod::IBaseBuiltinVote *vote, int client, SourceMod::BuiltinVoteFailReason reason)
{
	m_pHandler->OnVoteCancel(menu, client, reason);
}
*/

/*
void BuiltinVoteHandler::OnVoteDisplay(IBaseBuiltinVote *vote, int client)
{
	m_ClientVotes[client] = VOTE_PENDING;
	m_pHandler->OnVoteDisplay(vote, client);
}
*/

void BuiltinVoteHandler::OnVoteSelect(IBaseBuiltinVote *vote, int client, unsigned int item)
{
	if (IsVoteInProgress() && m_ClientVotes[client] == VOTE_PENDING)
	{
		/* Check by our item count, NOT the vote array size */
		if (item < m_Items)
		{
			vote->ClientSelectedItem(client, item);
			m_ClientVotes[client] = item;
			m_Votes[item]++;
			m_NumVotes++;

			if (sm_vote_chat->GetBool() || sm_vote_console->GetBool())
			{
				static char buffer[1024];
				const char *choice;
				choice = vote->GetItemDisplay(item);
			
				if (sm_vote_console->GetBool())
				{
					int target = SOURCEMOD_SERVER_LANGUAGE;
					
#ifdef _DEBUG
					smutils->LogMessage(myself, "Sending vote cast to server console.");
#endif
					Translate(buffer, sizeof(buffer), "[BV] %T", 4, NULL, "Voted For", &target, playerhelpers->GetGamePlayer(client)->GetName(), choice);
					engine->LogPrint(buffer);
				}

				if (sm_vote_chat->GetBool() || sm_vote_client_console->GetBool())
				{
					int maxclients = playerhelpers->GetMaxClients();
					for (int i=1; i<=maxclients; i++)
					{
						IGamePlayer *pPlayer = playerhelpers->GetGamePlayer(i);
						assert(pPlayer);

						if(pPlayer->IsInGame() && !pPlayer->IsFakeClient())
						{
							if (m_Revoting[client])
							{
								Translate(buffer, sizeof(buffer), "[BV] %T", 4, NULL, "Changed Vote", &i, playerhelpers->GetGamePlayer(client)->GetName(), choice);
							}
							else
							{
								Translate(buffer, sizeof(buffer), "[BV] %T", 4, NULL, "Voted For", &i, playerhelpers->GetGamePlayer(client)->GetName(), choice);
							}

							if (sm_vote_chat->GetBool())
							{
#ifdef _DEBUG
								smutils->LogMessage(myself, "Sending vote cast to chat for %s.", pPlayer->GetName());
#endif
								gamehelpers->TextMsg(i, TEXTMSG_DEST_CHAT, buffer);
							}

							if (sm_vote_client_console->GetBool())
							{
#ifdef _DEBUG
								smutils->LogMessage(myself, "Sending vote cast to client console for %s.", pPlayer->GetName());
#endif
								engine->ClientPrintf(pPlayer->GetEdict(), buffer);
							}
						}
					}
				}
			}
			vote->UpdateVoteCounts(m_Items, m_Votes, m_TotalClients); // Same thing as BuildVoteLeaders, but for L4D/L4D2 internals
			BuildVoteLeaders();
			DrawHintProgress();
		}

		m_pHandler->OnVoteSelect(vote, client, item);
		DecrementPlayerCount(); // Added here when EndVote got removed
	}
}

void BuiltinVoteHandler::InternalReset()
{
	m_Clients = 0;
	m_Items = 0;
	m_bStarted = false;
	m_pCurVote = NULL;
	m_NumVotes = 0;
	m_bCancelled = false;
	m_pHandler = NULL;
	m_leaderList[0] = '\0';
	
	m_TotalClients = 0;

	if (m_displayTimer)
	{
		timersys->KillTimer(m_displayTimer);
	}
	m_displayTimer = NULL;
}

void BuiltinVoteHandler::CancelVoting()
{
	if (m_bCancelled || !m_pCurVote)
	{
		return;
	}
	m_bCancelled = true;
	m_bWasCancelled = true;

	m_pCurVote->Cancel();
	EndVoting();
}

IBaseBuiltinVote *BuiltinVoteHandler::GetCurrentVote()
{
	return m_pCurVote;
}

bool BuiltinVoteHandler::IsCancelling()
{
	return m_bCancelled;
}

bool BuiltinVoteHandler::WasCancelled()
{
	return m_bWasCancelled;
}

void BuiltinVoteHandler::DrawHintProgress()
{
	if (!sm_vote_hintbox->GetBool())
	{
		return;
	}

	static char buffer[1024];

	float timeRemaining = (m_fStartTime + m_nVoteTime) - gpGlobals->curtime;
	if (timeRemaining < 0)
	{
		timeRemaining = 0.0;
	}

	int iTimeRemaining = RoundFloatToInt(timeRemaining);
	
	int maxclients = playerhelpers->GetMaxClients();
	for (int i=1; i<=maxclients; i++)
	{
		IGamePlayer *pPlayer = playerhelpers->GetGamePlayer(i);
		if (pPlayer->IsInGame() && !pPlayer->IsFakeClient())
		{
#ifdef _DEBUG
			smutils->LogMessage(myself, "Sending vote cast to hintbox for %s.", pPlayer->GetName());
#endif

			Translate(buffer, sizeof(buffer), "%T%s", 6, NULL, "Vote Count", &i, &m_NumVotes, &m_TotalClients, &iTimeRemaining, &m_leaderList);
			gamehelpers->HintTextMsg(i, buffer);
		}
	}
}

void BuiltinVoteHandler::BuildVoteLeaders()
{
	if (m_NumVotes == 0 || !sm_vote_hintbox->GetBool())
	{
		return;
	}

	menu_vote_result_t results;
	menu_vote_result_t::menu_item_vote_t item_vote[256];

	memset(&results, 0, sizeof(results));

	/* Build the item list */
	for (unsigned int i=0; i<m_Items; i++)
	{
		if (m_Votes[i] > 0)
		{
			item_vote[results.num_items].count = m_Votes[i];
			item_vote[results.num_items].item = i;
			results.num_votes += m_Votes[i];
			results.num_items++;
		}
	}
	results.item_list = item_vote;
	assert(results.num_votes);

	/* Sort the item list descending */
	qsort(item_vote,
		results.num_items,
		sizeof(menu_vote_result_t::menu_item_vote_t),
		SortVoteItems);

	/* Take the top 3 (if applicable) and draw them */
	int len = 0;
	for (unsigned int i=0; i<results.num_items && i<3; i++)
	{
		int curItem = results.item_list[i].item;
		const char *choice;
		choice = m_pCurVote->GetItemDisplay(curItem);
		len += smutils->Format(m_leaderList + len, sizeof(m_leaderList) - len, "\n%i. %s: (%i)", i+1, choice, results.item_list[i].count);
	}
}

ResultType BuiltinVoteHandler::OnTimer(ITimer *pTimer, void *pData)
{
	DrawHintProgress();

	if (--m_TimeLeft == 0)
	{
		if (m_displayTimer != NULL)
		{
			m_displayTimer = NULL;
			EndVoting();
		}
		return Pl_Stop;
	}

	return Pl_Continue;
}

void BuiltinVoteHandler::OnTimerEnd(ITimer *pTimer, void *pData)
{
	m_displayTimer = NULL;
}

RedrawTimer::RedrawTimer(int client, IBaseBuiltinVote *vote) :
m_client(client), m_pVote(vote)
{
}

ResultType RedrawTimer::OnTimer(ITimer *pTimer, void *pData)
{
	if (s_VoteHandler.IsVoteInProgress() && !s_VoteHandler.IsCancelling() && !s_VoteHandler.WasCancelled())
	{
		m_pVote->Display(m_client);
	}
	return Pl_Stop;
}

void RedrawTimer::OnTimerEnd(ITimer *pTimer, void *pData)
{
	delete this;
}
