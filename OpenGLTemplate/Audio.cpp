#include "Audio.h"
#pragma comment(lib, "lib/fmod_vc.lib")

FMOD_RESULT F_CALLBACK DSPCallback(FMOD_DSP_STATE *dsp_state, float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int *outchannels);

CAudio::CAudio()
{}

CAudio::~CAudio()
{}

bool CAudio::Initialise()
{
	// Create an FMOD system
	result = FMOD::System_Create(&m_FmodSystem);
	FmodErrorCheck(result);
	if (result != FMOD_OK) 
		return false;

	// Initialise the system
	result = m_FmodSystem->init(32, FMOD_INIT_3D_RIGHTHANDED, 0);
	FmodErrorCheck(result);
	if (result != FMOD_OK) 
		return false;

	////DSP Coursework: Set 3D settings, tried a few values, but 1, 1, 1 seemed to work well.
	result = m_FmodSystem->set3DSettings(1.0f, 1.0f, 1.0f);
	FmodErrorCheck(result);
	if (result != FMOD_OK)
		return false;

	//Create DSP effect
	{
		FMOD_DSP_DESCRIPTION dspdesc;
		memset(&dspdesc, 0, sizeof(dspdesc));

		strncpy_s(dspdesc.name, "band pass filter", sizeof(dspdesc.name));
		dspdesc.numinputbuffers = 1;
		dspdesc.numoutputbuffers = 1;
		dspdesc.read = DSPCallback;

		result = m_FmodSystem->createDSP(&dspdesc, &m_dsp);
		FmodErrorCheck(result);

		if (result != FMOD_OK)
			return false;
	}

	return true;
	
}


// Load an event sound
bool CAudio::LoadEventSound(char *filename)
{
	result = m_FmodSystem->createSound(filename, NULL, 0, &m_eventSound);
	FmodErrorCheck(result);
	if (result != FMOD_OK) 
		return false;

	return true;
	

}

// Play an event sound
bool CAudio::PlayEventSound()
{
	result = m_FmodSystem->playSound(m_eventSound, NULL, false, NULL);
	FmodErrorCheck(result);
	if (result != FMOD_OK)
		return false;
	return true;
}

bool CAudio::LoadEngineSound(char * filename)
{
	result = m_FmodSystem->createSound(filename, FMOD_LOOP_NORMAL, 0, &m_engineSound);

	FmodErrorCheck(result);

	if (result != FMOD_OK)
		return false;

	return true;
}

bool CAudio::PlayEngineSound()
{
	result = m_FmodSystem->playSound(m_engineSound, NULL, false, &m_sfxChannel);

	FmodErrorCheck(result);
	
	if (result != FMOD_OK)
		return false;
	
	m_sfxChannel->setMode(FMOD_3D);

	m_sfxChannel->setVolume(25.0f);

	return true;
}

// Load a music stream
bool CAudio::LoadMusicStream(char *filename)
{
	result = m_FmodSystem->createStream(filename, NULL | FMOD_LOOP_NORMAL, 0, &m_music);
	FmodErrorCheck(result);

	if (result != FMOD_OK) 
		return false;
	return true;
	

}

// Play a music stream
bool CAudio::PlayMusicStream()
{
	result = m_FmodSystem->playSound(m_music, NULL, false, &m_musicChannel);
	FmodErrorCheck(result);

	if (result != FMOD_OK)
		return false;

	result = m_musicChannel->setVolume(m_musicVolume);

	m_musicChannel->addDSP(0, m_dsp);

	//FmodErrorCheck(result);
	//if (result != FMOD_OK)
	//	return false;

	////connect the music filter to the music stream
	////get the DSP head and its input
	//m_musicChannel->getDSP(FMOD_CHANNELCONTROL_DSP_HEAD, &m_musicDSPHead);
	//m_musicDSPHead->getInput(0, &m_musicDSPHeadInput, NULL);
	////Disconnect them
	//m_musicDSPHead->disconnectFrom(m_musicDSPHeadInput);
	////Add input to the music head from the filter
	//result = m_musicDSPHead->addInput(m_dsp);
	//FmodErrorCheck(result);
	//if (result != FMOD_OK)
	//	return false;

	////Add input to the filter head music DSP head input
	//result = m_dsp->addInput(m_musicDSPHeadInput);
	//FmodErrorCheck(result);
	//if (result != FMOD_OK)
	//	return false;

	////set the DSP object to be active
	//m_dsp->setActive(true);
	//m_dspActive = false;

	return true;
}

// Check for error
void CAudio::FmodErrorCheck(FMOD_RESULT result)
{						
	if (result != FMOD_OK) {
		const char *errorString = FMOD_ErrorString(result);
		// MessageBox(NULL, errorString, "FMOD Error", MB_OK);
		// Warning: error message commented out -- if headphones not plugged into computer in lab, error occurs
	}
}

void CAudio::Update(glm::vec3 *position, glm::vec3 *velocity, CCamera * camera)
{
	ConvertToFMODVector(*position, &fModPosition);
	ConvertToFMODVector(camera->GetPosition(), &camPos);
	ConvertToFMODVector(*velocity, &fModVelocity);
	
	FMOD_VECTOR camVel = FMOD_VECTOR();
	FMOD_VECTOR camUp = FMOD_VECTOR();
	FMOD_VECTOR camForward = FMOD_VECTOR();

	ConvertToFMODVector(camera->cameraVelocity, &camVel);
	ConvertToFMODVector(glm::normalize(camera->GetUpVector()), &camUp);
	ConvertToFMODVector(glm::normalize(camera->GetView()), &camForward);

	FMOD_VECTOR zerovelocity = FMOD_VECTOR();
	//result = m_sfxChannel->set3DAttributes(&fModPosition, &zerovelocity);

	result = m_sfxChannel->set3DAttributes(&fModPosition, &fModVelocity);
	FmodErrorCheck(result);
	if (result != FMOD_OK)
		return;

	m_FmodSystem->set3DListenerAttributes(0, &camPos, &camVel, &camForward, &camUp);
	

	m_FmodSystem->update();

}

void CAudio::ToggleMusicFilter()
{
	// called externally from Game::ProcessEvents
	// toggle the effect on/off
	m_dspActive = !m_dspActive;
	if (m_dspActive) {
		// set the parameter to a low value
		m_dsp->setActive(true);
	}
	else {
		// set the parameter to a high value
		// you could also use m_musicFilter->setBypass(true) instead...
		//m_dsp->setParameterFloat(FMOD_DSP_LOWPASS_CUTOFF, 22000);
		m_dsp->setActive(false);
	}
}

void CAudio::IncreaseMusicVolume()
{
	// called externally from Game::ProcessEvents
	// increment the volume
	m_musicVolume += 0.05f;
	if (m_musicVolume > 1)
		m_musicVolume = 1.0f;
	m_musicChannel->setVolume(m_musicVolume);
}

void CAudio::DecreaseMusicVolume()
{
	// called externally from Game::ProcessEvents
	// deccrement the volume
	m_musicVolume -= 0.05f;
	if (m_musicVolume < 0)
		m_musicVolume = 0.0f;
	m_musicChannel->setVolume(m_musicVolume);
}


void CAudio::ConvertToFMODVector(glm::vec3 &vector, FMOD_VECTOR *fmodVector)
{
	fmodVector->x = vector.x;
	fmodVector->y = vector.y;
	fmodVector->z = vector.z;
}

typedef struct
{
	float *buffer;
	float volume_linear;
	int   length_samples;
	int   channels;
} mydsp_data_t;

//21 coefficients
float coefficients[] = 
{
	0.024700733165523316298139988589355198201,
	0.03921345697309289862753090005753620062,
	0.033333889473331913821940730713322409429,
	- 0.00215538428278549184374801583885528089,
	- 0.057157599868397759856897266672604018822,
	- 0.102784946127722390252223760853667045012,
	- 0.106798445972705380313527712132781744003,
	- 0.055462089052768517860592112356243887916,
	0.033348271107052122885772149629701743834,
	0.116703793751943687984073960706155048683,
	0.150642964971482051783624456220422871411,
	0.116703793751943687984073960706155048683,
	0.033348271107052122885772149629701743834,
	- 0.055462089052768517860592112356243887916,
	- 0.106798445972705380313527712132781744003,
	- 0.102784946127722390252223760853667045012,
	- 0.057157599868397759856897266672604018822,
	- 0.00215538428278549184374801583885528089,
	0.033333889473331913821940730713322409429,
	0.03921345697309289862753090005753620062,
	0.024700733165523316298139988589355198201,
};

static CircleBuffer *cbuffer;
static int cbufflength = 1024;
static int sample_count;

FMOD_RESULT F_CALLBACK DSPCallback(FMOD_DSP_STATE *dsp_state, float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int *outchannels)
{
	//unsigned int samp;
	//int chan;
	//FMOD::DSP *thisdsp = (FMOD::DSP *)dsp_state->instance;

	//const int buffer_size = 1024;
	//int delay = 512;
	//static float *buffer = NULL;

	//if (buffer == NULL)
	//	buffer = (float*)malloc(buffer_size * sizeof(float) * inchannels);

	//for (samp = 0; samp < length; samp++)
	//{
	//	for (chan = 0; chan < inchannels; chan++)
	//	{
	//		buffer[(sample_count * inchannels) % buffer_size + chan] = inbuffer[(samp * inchannels) + chan];

	//		if (sample_count < 4) // don't reach before the the start of the buffer with sample_count-3 below
	//			outbuffer[(samp * inchannels) + chan] = 0;
	//		else {
	//			// this is a simple averaging filter with 4 coefficients
	//			outbuffer[(samp * inchannels) + chan] = 0.25f * buffer[(sample_count * inchannels) % buffer_size + chan];
	//			outbuffer[(samp * inchannels) + chan] += 0.25f * buffer[((sample_count - 1) * inchannels) % buffer_size + chan];
	//			outbuffer[(samp * inchannels) + chan] += 0.25f * buffer[((sample_count - 2) * inchannels) % buffer_size + chan];
	//			outbuffer[(samp * inchannels) + chan] += 0.25f * buffer[((sample_count - 3) * inchannels) % buffer_size + chan];
	//		}
	//	}
	//	sample_count++;
	//}

	//return FMOD_OK;
	//instantiate the circle buffer if not already done
	//if (cbuffer == NULL)
	//	cbuffer = new CircleBuffer(cbufflength * sizeof(float) * inchannels);	
	//
	//mydsp_data_t *data = (mydsp_data_t *)dsp_state->plugindata;
	//
	//int cbufferLen = cbuffer->getBufferLength();

	//for (int i = 0; i < length; i++)
	//{
	//	for (int j = 0; j < inchannels; j++)
	//	{
	//		cbuffer->put(inbuffer[(i * inchannels) + j]);

	//		//convolution
	//		float temp = 0;

	//		for (int k = 0; k < cbufferLen; k++)
	//		{
	//			temp = cbuffer->getValueAtIndex((i * cbufferLen - 1) % cbufferLen + j) * coefficients[k];
	//		}

	//		data->buffer[(i * *outchannels) + j] = outbuffer[(i * *outchannels) + j] = 0.5f * (inbuffer[(i * inchannels) + j] * data->volume_linear + temp * (1.0 - data->volume_linear));
	//	}
	//}

	//data->channels = inchannels;

	//return FMOD_OK;
	mydsp_data_t *data = (mydsp_data_t *)dsp_state->plugindata;

	/*
	This loop assumes inchannels = outchannels, which it will be if the DSP is created with '0'
	as the number of channels in FMOD_DSP_DESCRIPTION.
	Specifying an actual channel count will mean you have to take care of any number of channels coming in,
	but outputting the number of channels specified. Generally it is best to keep the channel
	count at 0 for maximum compatibility.
	*/
	for (unsigned int samp = 0; samp < length; samp++)
	{
		/*
		Feel free to unroll this.
		*/
		for (int chan = 0; chan < *outchannels; chan++)
		{
			/*
			This DSP filter just halves the volume!
			Input is modified, and sent to output.
			*/
			data->buffer[(samp * *outchannels) + chan] = outbuffer[(samp * inchannels) + chan] = inbuffer[(samp * inchannels) + chan] * data->volume_linear;
		}
	}

	data->channels = inchannels;

	return FMOD_OK;
}

//FMOD_RESULT F_CALLBACK DSPCreateCallback(FMOD_DSP_STATE *dsp_state)
//{
//	unsigned int blocksize;
//	FMOD_RESULT result;
//
//	result = dsp_state->functions->getblocksize(dsp_state, &blocksize);
//}
FMOD_RESULT F_CALLBACK myDSPCreateCallback(FMOD_DSP_STATE *dsp_state)
{
	unsigned int blocksize;
	FMOD_RESULT result;

	result = dsp_state->functions->getblocksize(dsp_state, &blocksize);
	ERRCHECK(result);

	mydsp_data_t *data = (mydsp_data_t *)calloc(sizeof(mydsp_data_t), 1);
	if (!data)
	{
		return FMOD_ERR_MEMORY;
	}
	dsp_state->plugindata = data;
	data->volume_linear = 1.0f;
	data->length_samples = blocksize;

	data->buffer = (float *)malloc(blocksize * 8 * sizeof(float));      // *8 = maximum size allowing room for 7.1.   Could ask dsp_state->functions->getspeakermode for the right speakermode to get real speaker count.
	if (!data->buffer)
	{
		return FMOD_ERR_MEMORY;
	}

	return FMOD_OK;
}

FMOD_RESULT F_CALLBACK myDSPReleaseCallback(FMOD_DSP_STATE *dsp_state)
{
	if (dsp_state->plugindata)
	{
		mydsp_data_t *data = (mydsp_data_t *)dsp_state->plugindata;

		if (data->buffer)
		{
			free(data->buffer);
		}

		free(data);
	}

	return FMOD_OK;
}


void  CAudio::CreateWall(glm::vec3 &position, float width, float height)
{
	FMOD::Geometry *geometry;
	result = m_FmodSystem->createGeometry(1, 4, &geometry);
	FmodErrorCheck(result);
	if (result != FMOD_OK) {
		return;
	}

	float halfWidth = width / 2;
	FMOD_VECTOR wallPoly[4];
	wallPoly[0] = { -halfWidth, 0.0f, 0.0f };
	wallPoly[1] = { -halfWidth, height, 0.0f };
	wallPoly[2] = { halfWidth, height, 0.0f };
	wallPoly[3] = { halfWidth, 0.0f, 0.0f };
	int polyIndex = 0;
	result = geometry->addPolygon(1.0f, 1.0f, TRUE, 4, wallPoly, &polyIndex);
	FmodErrorCheck(result);
	if (result != FMOD_OK) {
		return;
	}

	FMOD_VECTOR wallPosition;
	ConvertToFMODVector(position, &wallPosition);
	geometry->setPosition(&wallPosition);
	geometry->setActive(TRUE);
}