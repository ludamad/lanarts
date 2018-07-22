#pragma once
#ifdef BUILD_HEADLESS

#define SDL_Delay(x) 0
#define SDL_Init(x) 0
#define SDL_GetError(x) ""
#define SDL_GameControllerAddMappingsFromFile(x) 0
#define SDL_PollEvent(x) 0
#define SDL_GameControllerGetButton(x, y) 0
#define SDL_GameControllerGetAxis(x, y) 0
#define SDL_JoystickInstanceID(x) 0
#define SDL_GetMouseState(x, y) 0
#define SDL_GameControllerOpen(x) 0
#define SDL_JoystickOpen(x) nullptr
#define SDL_JoystickInstanceID(x) 0
#define SDL_IsGameController(x) 0
#define SDL_NumJoysticks(x) 0
#define SDL_NumJoysticks(x) 0

#define Mix_AllocateChannels(x) 0
#define Mix_CloseAudio(x) 0
#define Mix_FreeChunk(x) 0
#define Mix_FreeMusic(x) 0
#define Mix_HaltMusic(x) 0
#define Mix_LoadMUS(x) 0
#define Mix_LoadWAV_RW(x, y) 0
#define Mix_OpenAudio(x,y,z,w) 0
#define Mix_PlayChannelTimed(x,y,z,w) 0
#define Mix_PlayMusic(x, y) 0
#define Mix_Volume(x,y) 0
#define SDL_CondSignal(x) 0
#define SDL_CondWait(x,y) 0
#define SDL_CondWaitTimeout(x,y,z) 0
#define SDL_CreateCond(x) 0
#define SDL_CreateMutex(x) 0
#define SDL_CreateThread(x,y,z) 0
#define SDL_DestroyCond(x) 0
#define SDL_DestroyMutex(x) 0
#define SDL_GetError(x) 0
#define SDL_LockMutex(x) 0
#define SDL_Quit(x) 0
#define SDL_RWFromFile(x) 0
#define SDL_UnlockMutex(x) 0
#define SDL_WaitThread(x,y) 0

#endif