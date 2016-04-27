#include "FmodAudioPlayer.h"

FmodAudioPlayer* FmodAudioPlayer::instance = nullptr;

FmodAudioPlayer::FmodAudioPlayer()
	:currMusicId(0)
{

}
FmodAudioPlayer::~FmodAudioPlayer()
{
	cocos2d::Director::getInstance()->getScheduler()->unschedule("FmodUpdate", this);

	if (fmodSounds != nullptr)
	{
		for (auto v : *fmodSounds)
		{
			v.second->channel->stop();
			v.second->sound->release();
			CC_SAFE_DELETE(v.second);
		}
		CC_SAFE_DELETE(fmodSounds);
	}

	result = channelGroup->release();
	FMOD_ERROR_CHECK(result);

	result = fmodSystem->close();
	FMOD_ERROR_CHECK(result);

	result = fmodSystem->release();
	FMOD_ERROR_CHECK(result);
}


FmodAudioPlayer* FmodAudioPlayer::getInstance()
{
	if (instance == nullptr)
	{
		instance = new FmodAudioPlayer();
		instance->init();
	}
	return instance;
}
void FmodAudioPlayer::uncache()
{
	channelGroup->stop();
	if (fmodSounds != nullptr)
	{
		for (auto v : *fmodSounds)
		{
			v.second->channel->stop();
			v.second->sound->release();
			CC_SAFE_DELETE(v.second);
		}
		fmodSounds->clear();
	}
}
int FmodAudioPlayer::ERROR_CHECK(FMOD_RESULT result)
{
	if (result != FMOD_OK)
	{
		return -1;
	}
	return 0;
}

void FmodAudioPlayer::init()
{
	fmodSounds = new std::unordered_map<unsigned int, FmodObj*>();

	FMOD::ChannelGroup *masterChannelGroup;

	result = FMOD::System_Create(&fmodSystem);  // Create the main system object.
	FMOD_ERROR_CHECK(result) return;

	result = fmodSystem->init(FMOD_CHANNEL_COUNT, FMOD_INIT_NORMAL, 0); // Initialize FMOD. FMOD_INIT_STREAM_FROM_UPDATE
	FMOD_ERROR_CHECK(result) return;

	result = fmodSystem->createChannelGroup("CG", &channelGroup);
	FMOD_ERROR_CHECK(result) return;

	result = fmodSystem->getMasterChannelGroup(&masterChannelGroup);
	FMOD_ERROR_CHECK(result) return;

	result = masterChannelGroup->addGroup(channelGroup);
	FMOD_ERROR_CHECK(result) return;

	cocos2d::Director::getInstance()->getScheduler()->schedule(
		std::bind(&FmodAudioPlayer::update, this, std::placeholders::_1),
		this, 1 / 40.f, false, "FmodUpdate");
}
void FmodAudioPlayer::update(float dt)
{
	if (fmodSounds->empty())
		return;

	fmodSystem->update();
}




int FmodAudioPlayer::loadMusic(std::string path, LOAD_TYPE type, std::function<void(bool, int)> listener)
{
	std::string writeablePath = cocos2d::FileUtils::getInstance()->getWritablePath() + path;

	if (!cocos2d::FileUtils::getInstance()->isFileExist(writeablePath))
	{
		std::string assetPath = cocos2d::FileUtils::getInstance()->fullPathForFilename(path);
		if (assetPath == "")
			return -1;

		ssize_t size = 0;
		unsigned char* data = cocos2d::CCFileUtils::sharedFileUtils()->getFileData(assetPath, "r", &size);

		FILE* target = fopen(writeablePath.c_str(), "wb");
		fwrite(data, size, 1, target);

		delete[] data;
		fclose(target);
	}

	FMOD::Sound* sound;
	FmodObj* f = new FmodObj;
	switch (type)
	{
	case CREATE_STREAM:
		result = fmodSystem->createStream(writeablePath.c_str(), FMOD_CREATESTREAM, 0, &sound);
		FMOD_ERROR_CHECK(result) return -1;
		f->sound = sound;
		fmodSounds->insert(std::pair<int, FmodObj*>(currMusicId, f));

		return currMusicId++;
		break;

	case DECOMPRESS_ON_MEMORY:
			result = fmodSystem->createStream(writeablePath.c_str(), FMOD_CREATESAMPLE, 0, &sound);
			bool load;
			if (ERROR_CHECK(result) == -1)
				load = false;
			else
			{
				f->sound = sound;
				fmodSounds->insert(std::pair<int, FmodObj*>(currMusicId, f));
				load = true;
			}

			if (listener != nullptr)
			{
				listener(load, load ? currMusicId++ : -1);
			}
		break;
	}
}
double FmodAudioPlayer::playLoadedMusic(int audioId, bool loop)
{
	std::chrono::system_clock::time_point start = std::chrono::system_clock::now();

	std::unordered_map<unsigned int, FmodObj*>::iterator it =
		fmodSounds->find(audioId);
	if (it == fmodSounds->end())
		return -1;

	FmodObj* obj = it->second;

	result = fmodSystem->playSound(obj->sound, channelGroup, false, &obj->channel);
	FMOD_ERROR_CHECK(result) return -1;

	obj->channel->setChannelGroup(channelGroup);
	result = obj->channel->setLoopCount(loop ? -1 : 0);
	FMOD_ERROR_CHECK(result) return -1;


	fmodSystem->update();
	std::chrono::duration<double> sec = std::chrono::system_clock::now() - start;
	return sec.count();
}

void FmodAudioPlayer::stopLoadedMusic(int audioId, bool releaseData)
{
	fmodSystem->update();

	std::unordered_map<unsigned int, FmodObj*>::iterator it =
		fmodSounds->find(audioId);
	if (it == fmodSounds->end())
		return;
	
	FmodObj* v = it->second;
	cocos2d::Director::getInstance()->getScheduler()->unscheduleAllForTarget(v);
	
	v->channel->stop();

	if (releaseData)
	{
		v->sound->release();
		fmodSounds->erase(it);
		CC_SAFE_DELETE(v);
	}

}

void FmodAudioPlayer::pauseAll()
{
	channelGroup->setPaused(true);
}
void FmodAudioPlayer::resumeAll()
{
	channelGroup->setPaused(false);
}



void FmodAudioPlayer::setPosition(int audioId, float value)
{
	std::unordered_map<unsigned int, FmodObj*>::iterator it =
		fmodSounds->find(audioId);
	if (it == fmodSounds->end())
		return;
	
	FmodObj* v = it->second;
	v->channel->setPosition( ((unsigned int)value * 1000), FMOD_TIMEUNIT_MS );

	fmodSystem->update();
}

float FmodAudioPlayer::getPosition(int audioId)
{
	std::unordered_map<unsigned int, FmodObj*>::iterator it =
		fmodSounds->find(audioId);
	if (it == fmodSounds->end())
		return -1;

	FmodObj* v = it->second;
	unsigned int pos;
	v->channel->getPosition(&pos, FMOD_TIMEUNIT_MS);

	return pos / 1000.f;
}


void FmodAudioPlayer::setFlanger(int audioId)
{
	std::unordered_map<unsigned int, FmodObj*>::iterator it =
		fmodSounds->find(audioId);
	if (it == fmodSounds->end())
		return;

	FmodObj* v = it->second;
	FMOD::DSP *flanger;
	fmodSystem->createDSPByType(FMOD_DSP_TYPE_FLANGE, &flanger);
	flanger->setParameterFloat(FMOD_DSP_FLANGE_MIX, 60);
	flanger->setParameterFloat(FMOD_DSP_FLANGE_DEPTH, 0.5f);
	flanger->setParameterFloat(FMOD_DSP_FLANGE_RATE, 8.f);
	v->channel->addDSP(0, flanger);
}

void FmodAudioPlayer::setLowPass(int audioId)
{
	std::unordered_map<unsigned int, FmodObj*>::iterator it =
		fmodSounds->find(audioId);
	if (it == fmodSounds->end())
		return;

	FmodObj* v = it->second;
	FMOD::DSP *lowPass;
	fmodSystem->createDSPByType(FMOD_DSP_TYPE_LOWPASS, &lowPass);
	lowPass->setParameterFloat(FMOD_DSP_LOWPASS_CUTOFF, 6000);
	lowPass->setParameterFloat(FMOD_DSP_LOWPASS_RESONANCE, 3.f);
	v->channel->addDSP(0, lowPass);

}

void FmodAudioPlayer::FadeIn(int audioId, float time, std::function<void()> endCallback)
{
	std::unordered_map<unsigned int, FmodObj*>::iterator it =
		fmodSounds->find(audioId);
	if (it == fmodSounds->end())
		return;

	FmodObj* v = it->second;
	v->channel->setVolume(0.f);
	cocos2d::Director::getInstance()->getScheduler()->schedule(
		[=](float dt)
	{
		float interval = dt / time;
		float volume;
		v->channel->getVolume(&volume);

		if (volume + interval > 1.0f)
		{
			v->channel->setVolume(1.0f);
			if(endCallback)
				endCallback();
			cocos2d::Director::getInstance()->getScheduler()->unschedule("fadeIn" , v);
		}
		else
			v->channel->setVolume(volume + interval);

	}, v, 1 / 60.f, false, "fadeIn");

}
void FmodAudioPlayer::FadeOut(int audioId, float time, std::function<void()> endCallback)
{
	std::unordered_map<unsigned int, FmodObj*>::iterator it =
		fmodSounds->find(audioId);
	if (it == fmodSounds->end())
		return;

	FmodObj* v = it->second;
	v->channel->setVolume(1.f);
	cocos2d::Director::getInstance()->getScheduler()->schedule(
		[=](float dt)
	{
		float interval = dt / time;
		float volume;
		v->channel->getVolume(&volume);

		if (volume - interval < 0.f)
		{
			v->channel->setVolume(0.f);
			if (endCallback)
				endCallback();
			cocos2d::Director::getInstance()->getScheduler()->unschedule("fadeOut", v);
		}
		else
			v->channel->setVolume(volume - interval);

	}, v, 1 / 60.f, false, "fadeOut");
}


