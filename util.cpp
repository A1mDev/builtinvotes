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
 
#include "extension.h"
#include "util.h"

CBaseEntity* UTIL_FindEntityByClassname(const char *classname)
{
	CBaseEntity *pEntity = (CBaseEntity *)servertools->FirstEntity();
	
	while (pEntity) {
		if (strcmp(gamehelpers->GetEntityClassname(pEntity), classname) == 0) {
			return pEntity;
		}
		
		pEntity = (CBaseEntity *)servertools->NextEntity(pEntity);
	}

	return NULL;
}

void UTIL_ShowError(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	
	g_pSM->LogError(myself, fmt, ap);
	rootconsole->ConsolePrint(fmt, ap);
	
	va_end(ap);
}