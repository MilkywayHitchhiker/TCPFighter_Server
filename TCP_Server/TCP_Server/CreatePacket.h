#pragma once
#include"Packet.h"

void	Pack_CreateMyCharacter (Packet *pack, DWORD dwSessionID, BYTE byDirection, short shX, short shY, char chHP);

void	Pack_CreateOtherCharacter (Packet *pack, DWORD dwSessionID, BYTE byDirection, short shX, short shY, char chHP);

void	Pack_DeleteCharacter (Packet *pack, DWORD dwSessionID);

void	Pack_MoveStart (Packet *pack, DWORD dwSessionID, BYTE byDir, short shX, short shY);

void	Pack_MoveStop (Packet *pack, DWORD dwSessionID, BYTE byDir, short shX, short shY);

void	Pack_Attack1 (Packet *pack, DWORD dwSessionID, BYTE byDir, short shX, short shY);

void	Pack_Attack2 (Packet *pack, DWORD dwSessionID, BYTE byDir, short shX, short shY);

void	Pack_Attack3 (Packet *pack, DWORD dwSessionID, BYTE byDir, short shX, short shY);

void	Pack_Damage (Packet *pack, DWORD dwAttackID, DWORD dwDamageID, short shHP);

void	Pack_Sync (Packet *pack, DWORD dwSessionID,short shX, short shY );