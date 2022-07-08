#include <Windows.h>
#include <iostream>
#include "MemMan.h"
#include "csgo.hpp"
#include "Console.h"

MemMan MemClass;
namespace con = JadedHoboConsole;

using namespace hazedumper::netvars;
using namespace hazedumper::signatures;

struct variables
{
	uintptr_t localPlayer;
	uintptr_t gameModule;
	uintptr_t engineModule;
	uintptr_t glowObject;
} val;

struct ClrRender
{
	BYTE red, green, blue;
};
ClrRender clrEnemy;
ClrRender clrTeam;

struct GlowStruct
{
	BYTE buffer[8];
	float red;
	float green;
	float blue;
	float alpha;
	BYTE buffer2[16];
	bool renderWhenOccluded;
	bool renderWhenUnOccluded;
	bool fullBloom;
	BYTE buffer3[5];
	int glowStyle;
};

GlowStruct SetGlowColor(GlowStruct Glow, uintptr_t entity)
{
	bool defusing = MemClass.readMem<bool>(entity + m_bIsDefusing);
	if (defusing)
	{
		Glow.red = 1.0f;
		Glow.green = 1.0f;
		Glow.blue = 1.0f;
	}
	else
	{
		int health = MemClass.readMem<int>(entity + m_iHealth);
		Glow.red = health * -0.01 + 1;
		Glow.green = health * 0.01;
	}
	Glow.alpha = 1.0f;
	Glow.renderWhenOccluded = true;
	Glow.renderWhenUnOccluded = false;
	return Glow;
}

void SetTeamGlow(uintptr_t entity, int glowIndex)
{
	GlowStruct TGlow;
	TGlow = MemClass.readMem<GlowStruct>(val.glowObject + (glowIndex * 0x38));

	TGlow.blue = 1.0f;
	TGlow.alpha = 1.0f;
	TGlow.renderWhenOccluded = true;
	TGlow.renderWhenUnOccluded = false;
	MemClass.writeMem<GlowStruct>(val.glowObject + (glowIndex * 0x38), TGlow);
}

void SetEnemyGlow(uintptr_t entity, int glowIndex)
{
	GlowStruct EGlow;
	EGlow = MemClass.readMem<GlowStruct>(val.glowObject + (glowIndex * 0x38));
	EGlow = SetGlowColor(EGlow, entity);
	MemClass.writeMem<GlowStruct>(val.glowObject + (glowIndex * 0x38), EGlow);
}

void HandleGlow()
{
	val.glowObject = MemClass.readMem<uintptr_t>(val.gameModule + dwGlowObjectManager);
	int myTeam = MemClass.readMem<int>(val.localPlayer + m_iTeamNum);

	for (short int i = 0; i < 64; i++)
	{
		uintptr_t entity = MemClass.readMem<uintptr_t>(val.gameModule + dwEntityList + i * 0x10);
		if (entity != NULL)
		{
			int glowIndx = MemClass.readMem<int>(entity + m_iGlowIndex);
			int entityTeam = MemClass.readMem<int>(entity + m_iTeamNum);

			if (myTeam == entityTeam)
			{
				MemClass.writeMem<ClrRender>(entity + m_clrRender, clrTeam);
				SetTeamGlow(entity, glowIndx);
			}
			else
			{
				MemClass.writeMem<ClrRender>(entity + m_clrRender, clrEnemy);
				SetEnemyGlow(entity, glowIndx);
			}
		}
	}
}

void SetBrightness()
{
	clrTeam.red = 0;
	clrTeam.blue = 255;
	clrTeam.green = 0;

	clrEnemy.red = 255;
	clrEnemy.blue = 0;
	clrEnemy.green = 0;

	float brightness = 5.0f;

	int ptr = MemClass.readMem<int>(val.engineModule + model_ambient_min);
	int xorptr = *(int*)&brightness ^ ptr;
	MemClass.writeMem<int>(val.engineModule + model_ambient_min, xorptr);
}

int main()
{
	SetConsoleTitleA("");
	int procID = MemClass.getProcess(L"csgo.exe");
	val.gameModule = MemClass.getModule(procID, L"client.dll");
	val.engineModule = MemClass.getModule(procID, L"engine.dll");

	//output
	std::cout << "[" << con::fg_green << "F2" << con::fg_white << "]" << con::fg_magenta << " Glow-Hack\n\n" << con::fg_white;

	std::cout << "[" << con::fg_green << "+" << con::fg_white << "]" << con::fg_magenta << " Build: July 8\n" << con::fg_white;
	std::cout << "[" << con::fg_green << "+" << con::fg_white << "]" << con::fg_magenta << " Made by hj.#0010\n" << con::fg_white;
	std::cout << con::fg_white << "[" << con::fg_green << "+" << con::fg_white << "]" << con::fg_magenta << " I see a line of cars and they're all painted black\n" << con::fg_white;

	val.localPlayer = MemClass.readMem<uintptr_t>(val.gameModule + dwLocalPlayer);

	if (val.localPlayer == NULL)
		while (val.localPlayer == NULL)
			val.localPlayer = MemClass.readMem<uintptr_t>(val.gameModule + dwLocalPlayer);

	SetBrightness();

	while (true)
	{
		if (GetKeyState(VK_F2) & 1)
			HandleGlow();

		Sleep(1);
	}
	return 0;
}