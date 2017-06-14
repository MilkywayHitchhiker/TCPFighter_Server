#pragma once
#include"Packet.h"
#include"Game_Proc.h"

//자신의 캐릭터를 생성
void Create_MyCharactor (Packet *Pack, st_Charactor *pChar);

//다른사람의 캐릭터 생성
void Create_OtherCharactor (Packet *Pack, st_Charactor *pChar);