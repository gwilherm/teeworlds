#include "bloc.h"

/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <new>
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include <game/mapitems.h>

#include "character.h"
#include "laser.h"
#include "projectile.h"

// Character, "physical" player's part
CBloc::CBloc(CGameWorld *pWorld, vec2 pos, CPlayer *Owner)
: CEntity(pWorld, CGameWorld::ENTTYPE_BLOC)
{
	m_Pos = pos;

	for(int i = 0; i < 36; i++)
	{
		m_IDLine[i] = Server()->SnapNewID();
	}

	m_Core.Reset();
	m_Core.Init(&GameServer()->m_World.m_Core, GameServer()->Collision());
	m_Core.m_Pos = m_Pos;
	GameServer()->m_World.m_Core.m_apBlocs[Owner->GetCID()] = &m_Core;

	m_ReckoningTick = 0;
	mem_zero(&m_SendCore, sizeof(m_SendCore));
	mem_zero(&m_ReckoningCore, sizeof(m_ReckoningCore));

	GameServer()->m_World.InsertEntity(this);
}


void CBloc::Reset() {
    GameWorld()->DestroyEntity(this);
}

void CBloc::Tick()
{
	m_Core.Tick(true);

	return;
}

void CBloc::TickDefered()
{
	// advance the dummy
	{
		CWorldCore TempWorld;
		m_ReckoningCore.Init(&TempWorld, GameServer()->Collision());
		m_ReckoningCore.Tick(false);
		m_ReckoningCore.Move();
		m_ReckoningCore.Quantize();
	}

	//lastsentcore
	vec2 StartPos = m_Core.m_Pos;
	vec2 StartVel = m_Core.m_Vel;
	bool StuckBefore = GameServer()->Collision()->TestBox(m_Core.m_Pos, vec2(28.0f, 28.0f));

	m_Core.Move();
	bool StuckAfterMove = GameServer()->Collision()->TestBox(m_Core.m_Pos, vec2(28.0f, 28.0f));
	m_Core.Quantize();
	bool StuckAfterQuant = GameServer()->Collision()->TestBox(m_Core.m_Pos, vec2(28.0f, 28.0f));
	m_Pos = m_Core.m_Pos;

	if(!StuckBefore && (StuckAfterMove || StuckAfterQuant))
	{
		// Hackish solution to get rid of strict-aliasing warning
		union
		{
			float f;
			unsigned u;
		}StartPosX, StartPosY, StartVelX, StartVelY;

		StartPosX.f = StartPos.x;
		StartPosY.f = StartPos.y;
		StartVelX.f = StartVel.x;
		StartVelY.f = StartVel.y;

		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "STUCK!!! %d %d %d %f %f %f %f %x %x %x %x",
			StuckBefore,
			StuckAfterMove,
			StuckAfterQuant,
			StartPos.x, StartPos.y,
			StartVel.x, StartVel.y,
			StartPosX.u, StartPosY.u,
			StartVelX.u, StartVelY.u);
		GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);
	}

	// update the m_SendCore if needed
	{
		CNetObj_Character Predicted;
		CNetObj_Character Current;
		mem_zero(&Predicted, sizeof(Predicted));
		mem_zero(&Current, sizeof(Current));
		m_ReckoningCore.Write(&Predicted);
		m_Core.Write(&Current);

		// only allow dead reackoning for a top of 3 seconds
		if(m_ReckoningTick+Server()->TickSpeed()*3 < Server()->Tick() || mem_comp(&Predicted, &Current, sizeof(CNetObj_Character)) != 0)
		{
			m_ReckoningTick = Server()->Tick();
			m_SendCore = m_Core;
			m_ReckoningCore = m_Core;
		}
	}
}

void CBloc::TickPaused()
{
	++m_ReckoningTick;
}

void CBloc::Draw()
{

/*	CNetObj_Laser *pObj = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(NETOBJTYPE_LASER, m_IDLine[0], sizeof(CNetObj_Laser)));
	if(!pObj)
		return;
	pObj->m_FromX = (int)m_Pos.x;
	pObj->m_FromY = (int)m_Pos.y+50;
	pObj->m_X = (int)m_Pos.x;
	pObj->m_Y = (int)m_Pos.y;
	pObj->m_StartTick = Server()->Tick();

	pObj = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(NETOBJTYPE_LASER, m_IDLine[1], sizeof(CNetObj_Laser)));
	if(!pObj)
		return;
	pObj->m_FromX = (int)m_Pos.x;
	pObj->m_FromY = (int)m_Pos.y;
	pObj->m_X = (int)m_Pos.x+50;
	pObj->m_Y = (int)m_Pos.y;
	pObj->m_StartTick = Server()->Tick();

	pObj = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(NETOBJTYPE_LASER, m_IDLine[2], sizeof(CNetObj_Laser)));
	if(!pObj)
		return;
	pObj->m_FromX = (int)m_Pos.x+50;
	pObj->m_FromY = (int)m_Pos.y+50;
	pObj->m_X = (int)m_Pos.x+50;
	pObj->m_Y = (int)m_Pos.y;
	pObj->m_StartTick = Server()->Tick();

	pObj = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(NETOBJTYPE_LASER, m_IDLine[3], sizeof(CNetObj_Laser)));
	if(!pObj)
		return;
	pObj->m_FromX = (int)m_Pos.x+50;
	pObj->m_FromY = (int)m_Pos.y+50;
	pObj->m_X = (int)m_Pos.x;
	pObj->m_Y = (int)m_Pos.y+50;
	pObj->m_StartTick = Server()->Tick();*/

	CNetObj_Projectile *pObj;
	int i = 0;
	for(int x = 0; x < 6; x++)
	{
		for(int y = 0; y < 6; y++)
		{
			pObj = static_cast<CNetObj_Projectile *>(Server()->SnapNewItem(NETOBJTYPE_PROJECTILE, m_IDLine[i++], sizeof(CNetObj_Projectile)));
			if(!pObj)
				return;

			pObj->m_Type = WEAPON_SHOTGUN;
			pObj->m_X = (int)m_Pos.x-x*10;
			pObj->m_Y = (int)m_Pos.y-y*10;
			pObj->m_StartTick = Server()->Tick();
		}
	}

}

void CBloc::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;

	/*CNetObj_Character *pCharacter = static_cast<CNetObj_Character *>(Server()->SnapNewItem(NETOBJTYPE_CHARACTER, 8, sizeof(CNetObj_Character)));
	if(!pCharacter)
		return;

	// write down the m_Core
	if(!m_ReckoningTick || GameServer()->m_World.m_Paused)
	{
		// no dead reckoning when paused because the client doesn't know
		// how far to perform the reckoning
		pCharacter->m_Tick = 0;
		m_Core.Write(pCharacter);
	}
	else
	{
		pCharacter->m_Tick = m_ReckoningTick;
		m_SendCore.Write(pCharacter);
	}*/

	Draw();
}
