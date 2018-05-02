#pragma once
#include <windows.h>									// Header File For The Windows Library
#include "./include/fmod_studio/fmod.hpp"
#include "./include/fmod_studio/fmod_errors.h"
#include "CircleBuffer.h"
#include "Common.h"
#include "Camera.h"

class CAudio
{
public:
	CAudio();
	~CAudio();
	bool Initialise();
	bool LoadEventSound(char *filename);
	bool PlayEventSound();
	bool LoadEngineSound(char * filename);
	bool PlayEngineSound();
	bool LoadMusicStream(char *filename);
	bool PlayMusicStream();
	void Update(glm::vec3 *position, glm::vec3 *velocity, glm::vec3 *spherePosition, CCamera * camera);
	void CreateWall(glm::vec3 &position, float width, float height);
	void ToggleMusicFilter();
	void IncreaseMusicVolume();
	void DecreaseMusicVolume();
	void IncreaseDynamicFilterMultiplier();
	void DecreaseDynamicFilterMultiplier();

	bool switchOffLoop;

private:
		

	void FmodErrorCheck(FMOD_RESULT result);
	void ConvertToFMODVector(glm::vec3 &vector, FMOD_VECTOR *fmodVector);

	FMOD_RESULT result;
	FMOD::System *m_FmodSystem;	// the global variable for talking to FMOD

	FMOD::Sound *m_eventSound;
	FMOD::Sound *m_engineSound;
	FMOD::Channel *m_sfxChannel;
	FMOD::Channel *m_dynamicFilterChannel;

	FMOD::Sound *m_music;
	FMOD::DSP *m_dsp;
	FMOD::DSP *m_dynamicFilter;
	bool m_dspActive = false;
	FMOD::Channel* m_musicChannel;
	FMOD::DSP *m_musicDSPHead;
	FMOD::DSP *m_musicDSPHeadInput;
	float m_musicVolume = 0.2f;
	

	FMOD_VECTOR fModPosition;
	FMOD_VECTOR fModVelocity;
	FMOD_VECTOR camPos;
	FMOD_VECTOR camVel;
	FMOD_VECTOR camUp;
	FMOD_VECTOR camForward;
	FMOD_VECTOR spherePos;
};
