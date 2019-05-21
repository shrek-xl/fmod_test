// fmod_test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <assert.h>
#include "fmod.hpp"
#include <mutex>
#include <thread>

#define FMOD_ASSERT(A) assert(A == FMOD_OK)
const int CHANNEL_NUM = 10;

struct myChannel
{
	bool used;
	int idx;
	FMOD::Channel *channel;

	myChannel()
	{
		used = false;
		idx = -1;
		channel = NULL;
	}
} Channel_array[CHANNEL_NUM];

FMOD::System *gFMOD = NULL;

FMOD::Sound *mainThreadSound = NULL;
FMOD::Sound *subThreadSound = NULL;

std::mutex mMutexFMOD;
std::mutex mMutexGetFreeChannel;

myChannel* GetFreeChannel()
{
	std::lock_guard<std::mutex> lock(mMutexGetFreeChannel);

	for (int i = 0;i < CHANNEL_NUM;i++)
	{
		if (!Channel_array[i].used)
		{
			printf("got channel to play.....\n");
			Channel_array[i].used = true;
			Channel_array[i].idx = i;

			return &Channel_array[i];
		}
	}
	printf("........wait for free channel.\n");
	return NULL;
}

FMOD_RESULT F_CALLBACK custom_channel_call_back(FMOD_CHANNEL *channel, FMOD_CHANNEL_CALLBACKTYPE type, int command, unsigned int commanddata1, unsigned int commanddata2)
{
	void *p = NULL;
	((FMOD::Channel*)channel)->getUserData(&p);

	int index = *(int *)p;

	assert(index >= 0 && index < CHANNEL_NUM);

	printf("channel %d is reset.\n", index);
	
	Channel_array[index].channel = NULL;
	Channel_array[index].idx = -1;
	Channel_array[index].used = false;

	return FMOD_OK;
}

void sound_thread_entry()
{
	FMOD_ASSERT(gFMOD->createSound("swish.wav", 0, 0, &subThreadSound));

	while (true)
	{
		if (myChannel *freeChannel = GetFreeChannel())
		{
			std::lock_guard<std::mutex> lock(mMutexFMOD);

			FMOD_ASSERT(gFMOD->playSound(FMOD_CHANNEL_REUSE, subThreadSound, false, &freeChannel->channel));
			FMOD_ASSERT(freeChannel->channel->setUserData((void*)&freeChannel->idx));
			FMOD_ASSERT(freeChannel->channel->setCallback(FMOD_CHANNEL_CALLBACKTYPE_END, custom_channel_call_back, 0));

// 			FMOD::ChannelGroup *cgp = NULL;
// 			gFMOD->getMasterChannelGroup(&cgp);
// 			float r = rand() % 10 / 10.0;
// 			cgp->setVolume(r);
		}
	}
}

int main()
{
	FMOD_ASSERT(FMOD::System_Create(&gFMOD));
	FMOD_ASSERT(gFMOD->init(CHANNEL_NUM, FMOD_INIT_NORMAL, 0));

	FMOD_ASSERT(gFMOD->createSound("wave.mp3", 0, 0, &mainThreadSound));

	std::thread soundThread(sound_thread_entry);

	while (true)
	{
		std::lock_guard<std::mutex> lock(mMutexFMOD);

		if (myChannel *freeChannel = GetFreeChannel())
		{
			FMOD_ASSERT(gFMOD->playSound(FMOD_CHANNEL_REUSE, mainThreadSound, false, &freeChannel->channel));
			FMOD_ASSERT(freeChannel->channel->setUserData((void*)&freeChannel->idx));
			FMOD_ASSERT(freeChannel->channel->setCallback(FMOD_CHANNEL_CALLBACKTYPE_END, custom_channel_call_back, 0));
		}
		gFMOD->update();
	}

	FMOD_ASSERT(gFMOD->close());
	FMOD_ASSERT(gFMOD->release());

	soundThread.join();

    return 0;
}

