#include "Audio.h"
#include <math.h>

#pragma comment(lib, "lib/fmod_vc.lib")



//Had to create a global variable for this so that the callback could access it
//needs to be clamped between .5 and 1. 1 passes the coefficients unchanged at their maximum value, and anything less lowers their effectiveness
float dynamicFilterMultiplier = 1.0f;
float volume = 0.15f;

//used for calculating the sine and cosine so that the filters will interpolate
static float filterMultiplier = 0.0f;

FMOD_RESULT F_CALLBACK DSPCallback(FMOD_DSP_STATE *dsp_state, float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int *outchannels);
FMOD_RESULT F_CALLBACK DynamicFilterCallback(FMOD_DSP_STATE *dsp_state, float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int *outchannels);

CAudio::CAudio()
{}

CAudio::~CAudio()
{}


//Initialises the FMOD environment, sets up the system object and creates the DSP
//returns true if all succeeds
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

	//Create DSP for dynamic filter
	{
		FMOD_DSP_DESCRIPTION dspdesc;

		memset(&dspdesc, 0, sizeof(dspdesc));

		strncpy_s(dspdesc.name, "dynamic filter", sizeof(dspdesc.name));
		dspdesc.numinputbuffers = 1;
		dspdesc.numoutputbuffers = 1;
		dspdesc.read = DynamicFilterCallback;

		result = m_FmodSystem->createDSP(&dspdesc, &m_dynamicFilter);
		FmodErrorCheck(result);

		if (result != FMOD_OK)
			return false;
	}

	return true;
	
}


// Load an event sound
// takes as a parameter the file name to be loaded
// returns true if successful
bool CAudio::LoadEventSound(char *filename)
{
	result = m_FmodSystem->createSound(filename, FMOD_LOOP_NORMAL, 0, &m_eventSound);
	FmodErrorCheck(result);
	if (result != FMOD_OK) 
		return false;

	return true;
}


// play the event sound
// returns true if successful
// use the dynamic filter
bool CAudio::PlayEventSound()
{	
	if (!switchOffLoop)
	{
		result = m_FmodSystem->playSound(m_eventSound, NULL, false, &m_dynamicFilterChannel);
		FmodErrorCheck(result);
		if (result != FMOD_OK)
			return false;

		m_dynamicFilterChannel->setMode(FMOD_3D);

		m_dynamicFilterChannel->addDSP(0, m_dynamicFilter);
		return true;
	}
	else
	{
		result = m_dynamicFilterChannel->stop();
	}

}


// Load the engine sound for the space ship
// takes as a parameter the file name to be loaded
// returns true if successful
// I created a new function for this because I needed it to loop, and wanted to set it to its own channel
bool CAudio::LoadEngineSound(char * filename)
{
	result = m_FmodSystem->createSound(filename, FMOD_LOOP_NORMAL, 0, &m_engineSound);

	FmodErrorCheck(result);

	if (result != FMOD_OK)
		return false;

	return true;
}

//Plays the engine sound
//sets the mode to 3d and the volume. I set this quite loud because the sample is quiet compared to the music
//returns true if successful
bool CAudio::PlayEngineSound()
{
	result = m_FmodSystem->playSound(m_engineSound, NULL, false, &m_sfxChannel);

	FmodErrorCheck(result);
	
	if (result != FMOD_OK)
		return false;
	
	m_sfxChannel->setMode(FMOD_3D);

	m_sfxChannel->setVolume(50.0f);

	return true;
}

// Load a music stream
// takes as parameter the name of the music file and sets it to loop and play in the music channel
// returns true if successful
bool CAudio::LoadMusicStream(char *filename)
{
	result = m_FmodSystem->createStream(filename, NULL | FMOD_LOOP_NORMAL, 0, &m_music);
	FmodErrorCheck(result);

	if (result != FMOD_OK) 
		return false;
	return true;
}

// Play a music stream in the appropriate music channel
// sets the volume
// returns true if successful
// use the dsp filter
bool CAudio::PlayMusicStream()
{
	result = m_FmodSystem->playSound(m_music, NULL, false, &m_musicChannel);
	FmodErrorCheck(result);

	if (result != FMOD_OK)
		return false;

	result = m_musicChannel->setVolume(m_musicVolume);

	m_musicChannel->addDSP(0, m_dsp);

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

// Update is called once per frame and updates all the moving parts
// Specifically the position of a sound source (in this case the spaceship), its velocity
// And the camera object, which also has position and velocity. See the Update functions in Game.cpp and in Camera.cpp for how velocity was calculated.
// All these vector quantities are converted to FMOD_VECTORs, and the 3D attributes and 3D Listener attributes are updated using these values.
void CAudio::Update(glm::vec3 *shipPosition, glm::vec3 *velocity, glm::vec3 *spherePosition, CCamera * camera)
{
	//Increment multiplier used by the filters
	filterMultiplier += 0.0025f;

	//Convert the glm vectors to FMOD_VECTORs
	ConvertToFMODVector(*shipPosition, &fModPosition);
	ConvertToFMODVector(camera->GetPosition(), &camPos);
	ConvertToFMODVector(*velocity, &fModVelocity);ConvertToFMODVector(camera->cameraVelocity, &camVel);
	ConvertToFMODVector(glm::normalize(camera->GetUpVector()), &camUp);
	ConvertToFMODVector(glm::normalize(camera->GetView()), &camForward);
	ConvertToFMODVector(*spherePosition, &spherePos);

	//Set the 3dAttributes of the sound source (space ship)
	result = m_sfxChannel->set3DAttributes(&fModPosition, &fModVelocity);
	FmodErrorCheck(result);
	if (result != FMOD_OK)
		return;
		
	//velocity not important for sphere object so set to 0,0,0
	FMOD_VECTOR sphereVel = FMOD_VECTOR();

	result = m_dynamicFilterChannel->set3DAttributes(&spherePos, &sphereVel);

	//Set the 3D listener attributes of the camera
	m_FmodSystem->set3DListenerAttributes(0, &camPos, &camVel, &camForward, &camUp);	

	m_musicChannel->setVolume(volume);	

	//Update the system object
	m_FmodSystem->update();
}

//Switches the DSP filter on or off
//the ProcessEvents function in game.cpp calls this when the player presses the 2 key.
void CAudio::ToggleMusicFilter()
{
	// called externally from Game::ProcessEvents
	// toggle the effect on/off
	m_dspActive = !m_dspActive;

	if (m_dspActive) {
		// set the parameter to a low value
		m_dsp->setBypass(false);
		
	}
	else {
		// set the parameter to a high value
		// you could also use m_musicFilter->setBypass(true) instead...
		//m_dsp->setParameterFloat(FMOD_DSP_LOWPASS_CUTOFF, 22000);
		m_dsp->setBypass(true);
	}
}



//Increases the volume when the player hits the + key
void CAudio::IncreaseMusicVolume()
{
	// called externally from Game::ProcessEvents
	// increment the volume
	volume += 0.05f;
	if (volume > 0.15)
		volume = 0.15f;
	m_musicChannel->setVolume(volume);
}

//Decreases the volume when the player hits the - key
void CAudio::DecreaseMusicVolume()
{
	// called externally from Game::ProcessEvents
	// deccrement the volume
	volume -= 0.05f;
	if (volume < 0)
		volume = 0.0f;
	m_musicChannel->setVolume(volume);
}


//Utility function to convert a glm::vec3 vector into an FMOD_VECTOR
// The parameters are the glm::vec3 to be converted and an FMOD_VECTOR to be the destination. These are passed by reference so that the value can be written out.
void CAudio::ConvertToFMODVector(glm::vec3 &vector, FMOD_VECTOR *fmodVector)
{
	fmodVector->x = vector.x;
	fmodVector->y = vector.y;
	fmodVector->z = vector.z;
}

//Example filter coefficients generated in Matlab's Filter designer using the values provided in the coursework tips
//21 coefficients from filter in example, this number is somewhat magic (it's the number of elements in the array), 
//and is used in the callback to control the for loop that applies these coefficients to the signal
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

float coefficients2[]
{
	0.018389323253231483479286367810345836915,
	- 0.009966823982928139577519566216778912349,
	- 0.052116986388947815489469661542898393236,
	- 0.031267730091010012549723739994078641757,
	0.05867424812215452784025515597932098899,
	0.099767863602912176812509414958185516298,
	0.00422498143683301859152567914179599029,
	- 0.12523507876819434114779028277553152293,
	- 0.10630647556499935035834170093949069269,
	0.058085084455391330915396963519015116617,
	0.157149640051922279138096882888930849731,
	0.058085084455391330915396963519015116617,
	- 0.10630647556499935035834170093949069269,
	- 0.12523507876819434114779028277553152293,
	0.00422498143683301859152567914179599029,
	0.099767863602912176812509414958185516298,
	0.05867424812215452784025515597932098899,
	- 0.031267730091010012549723739994078641757,
	- 0.052116986388947815489469661542898393236,
	- 0.009966823982928139577519566216778912349,
	0.018389323253231483479286367810345836915,
};

static int sample_count;

//the circular buffer
static float *buffer = NULL;

// This is the callback function registered to the interpolating dsp based on filters created in MatLab. It takes the dsp_state, the in and out buffers, the length of the in buffer, the number of channels and the number of out channels
// its purpose is to calculate the dsp, which in this case is a band pass filter. it applies the values stored in the circular buffer to those in the inbuffer, and then writes those to the outbuffer
// Due to the inclusion of the coefficient multiplier, it is possible to use the keyboard to control the intensity of the filtered signal.
// The effect is not exactly what I'd hoped, as I wanted to make the filter more or less effective on the overall signal, but this decreases the volume.
FMOD_RESULT F_CALLBACK DSPCallback(FMOD_DSP_STATE *dsp_state, float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int *outchannels)
{
	//Declare the variables used for the outer loops
	unsigned int samp;
	int chan;

	//Create the buffer if it doesn't already exist
	const int buffer_size = 1024;

	if (buffer == NULL)
		buffer = (float*)malloc(buffer_size * sizeof(float) * inchannels);

	//step through the samples
	for (samp = 0; samp < length; samp++)
	{
		//step through the channels
		for (chan = 0; chan < inchannels; chan++)
		{
			//assign the values to the buffer
			//for (int i = 0; i < length; i++)
			buffer[(sample_count * inchannels) % buffer_size + chan] = inbuffer[(samp * inchannels) + chan];

			// don't reach before the the start of the buffer with sample_count-21 below
			if (sample_count < 21)
			{			
				outbuffer[(samp * inchannels) + chan] = inbuffer[(samp * inchannels) + chan]; //signal unchanged up to this point				
			}
			else 
			{
				//calculate the modified values
				float temp = 0.0f;
				float temp2 = 0.0f;

				for (int i = 0; i < 21; i++)
				{
					temp += (dynamicFilterMultiplier * coefficients[i]) * buffer[((sample_count - i) * inchannels) % buffer_size + chan];
					temp2 += (dynamicFilterMultiplier * coefficients2[i]) * buffer[((sample_count - i) * inchannels) % buffer_size + chan];
				}

				
				//volume is multiplied by 25 to get it to be a similar level as unfiltered
				//sin and cos of a global multiplier variable that is incremented in the update function is used to interpolate between filters
				outbuffer[(samp * inchannels) + chan] = temp * (25.0f * volume * (sin(filterMultiplier) * 2.0f - 0.5f)) + (temp2 * (25.0f * volume * (cos(filterMultiplier) * 2.0f - 0.5f)));
			}
		}
		sample_count++;
	}

	return FMOD_OK;	
}


//Decreases the multiplier of the coefficient values used in the DSP function
//This is called when the player hits the 3 key
void CAudio::DecreaseDynamicFilterMultiplier()
{
	dynamicFilterMultiplier -= 0.05f;

	if (dynamicFilterMultiplier < 0.0f)
		dynamicFilterMultiplier = 0.0f;
}


//Increases the multiplier of the coefficient values used in the DSP function
//This is called when the player hits the 4 key
void CAudio::IncreaseDynamicFilterMultiplier()
{
	dynamicFilterMultiplier += 0.05f;

	if (dynamicFilterMultiplier > 1.0f)
		dynamicFilterMultiplier = 1.0f;
}


//This callback function is used by the dynamic filter dsp. If the player hits 1, the event sound will play on a loop. 
FMOD_RESULT F_CALLBACK DynamicFilterCallback(FMOD_DSP_STATE *dsp_state, float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int *outchannels)
{
	//Declare the variables used for the outer loops
	unsigned int samp;
	int chan;

	//Create the buffer if it doesn't already exist
	const int buffer_size = 1024;

	if (buffer == NULL)
		buffer = (float*)malloc(buffer_size * sizeof(float) * inchannels);

	//step through the samples
	for (samp = 0; samp < length; samp++)
	{
		//step through the channels
		for (chan = 0; chan < inchannels; chan++)
		{
			//assign the values to the buffer
			//for (int i = 0; i < length; i++)
			buffer[(sample_count * inchannels) % buffer_size + chan] = inbuffer[(samp * inchannels) + chan];

			// don't reach before the the start of the buffer with sample_count-21 below
			if (sample_count < 21)
			{
				outbuffer[(samp * inchannels) + chan] = inbuffer[(samp * inchannels) + chan]; //signal unchanged up to this point				
			}
			else
			{
				//calculate the modified values
				float temp = 0.0f;

				for (int i = 0; i < 21; i++)
				{
					temp += (coefficients[i]) * buffer[((sample_count - i) * inchannels) % buffer_size + chan];
				}
				//volume is multiplied by 25 to get it to be a similar level as unfiltered
				//
				outbuffer[(samp * inchannels) + chan] = temp * (15.0f * dynamicFilterMultiplier) + inbuffer[(samp * inchannels) + chan] * (1.0f - dynamicFilterMultiplier);
			}
		}
		sample_count++;
	}

	return FMOD_OK;
}

//Creates a wall in FMOD to occlude sound based on the parameters passed in. The position is the centre of the wall.
//width and height are passed in to get the correct dimensions
//this function does not take into account orientation or the third dimension of length.
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