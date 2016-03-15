#ifndef BLOC_H
#define BLOC_H

#include <game/server/entity.h>
#include <game/generated/server_data.h>
#include <game/generated/protocol.h>

#include <game/gamecore.h>

class CPlayer;

class CBloc : public CEntity
{

public:
	//character's size
	static const int ms_PhysSize = 28;

	CBloc(CGameWorld *pWorld, vec2 pos, CPlayer *Owner);

	virtual void Reset();
	virtual void Tick();
	virtual void TickDefered();
	virtual void TickPaused();
	virtual void Snap(int SnappingClient);

	void Draw();
	bool Remove();

private:
	int m_IDLine[36];

	int m_ReloadTimer;

	// the player core for the physics
	CCharacterCore m_Core;

	// info for dead reckoning
	int m_ReckoningTick; // tick that we are performing dead reckoning From
	CCharacterCore m_SendCore; // core that we should send
	CCharacterCore m_ReckoningCore; // the dead reckoning core

};


#endif // BLOC_H
