/*********************************************************************
* Date : 2016.04.26
* Name : Fmod (Cocos2d-X ver 3.X)
* Email : create(pbes0707@gmail.com) / modify(mark4215@naver.com)
* GitHub : https://github.com/haminjun/Fmod_Interlock
* Cocos2d-x and linked the fmod library
***********************************************************************/
#ifndef __FMOD_AUDIO_PLAYER__
#define __FMOD_AUDIO_PLAYER__

#include "inc/fmod_errors.h"
#include "inc/fmod.hpp"
#include "cocos2d.h"

#define FMOD_ERROR_CHECK(result) \
	if (ERROR_CHECK(result) == -1)

class FmodAudioPlayer
{
public:
	enum LOAD_TYPE{
		CREATE_STREAM,		
		DECOMPRESS_ON_MEMORY
	};
	typedef struct _fmodObj{	
		FMOD::Sound* sound;	
		FMOD::Channel* channel;	
	}FmodObj;
private:
	FmodAudioPlayer();
	~FmodAudioPlayer();

	/*
	@brief : fmod update
	*/
	void update(float dt);

	/*
	@brief : FMOD Error Check
	* See the error return value of the internal fmod
	* @return fail : -1
	*/
	int ERROR_CHECK(FMOD_RESULT result);

	unsigned int								currMusicId;	// Current Sound Count
	FMOD_RESULT									result;			// Fmod Error Check		
	FMOD::System*								fmodSystem;		// Fmod		System
	FMOD::ChannelGroup*							channelGroup;	// Channel	Group
	std::unordered_map<unsigned int, FmodObj*>* fmodSounds;		// Sound	List

public:
	static FmodAudioPlayer* instance;
	static FmodAudioPlayer* getInstance();

	void init();
	void uncache();

	/*
	@brief : load Music (type standard : CREATE_STREAM / listener : nullptr )
	* load Music path/ type/ callback
	* @return fail : -1
	*/
	int loadMusic(std::string path, LOAD_TYPE type = CREATE_STREAM, std::function<void(bool, int)> listener = nullptr);

	/*
	@brief : play loaded music (loop standard : false)
	* @return fail : -1
	*/
	double playLoadedMusic(int audioId, bool loop = false);

	/*
	@brief : stop loaded music (releaseData standard : false)
	*/
	void stopLoadedMusic(int audioId, bool releaseData = false);

	/*
	@brief : setPosition Music Time
	* The unit value is in seconds
	*/
	void setPosition(int audioId, float value);

	/*
	@brief : setPosition Music Time
	* The unit value is in seconds
	*/
	float getPosition(int audioId);

	/*
	@brief : change music cyclically
	*/
	void setFlanger(int audioId);

	/*
	@brief : change music LowPass
	*/
	void setLowPass(int audioId);

	/*
	@brief : pause music all
	*/
	void pauseAll();

	/*
	@brief : resume music all
	*/
	void resumeAll();

	/*
	@brief : fadeIn Music (endCallback standard : nullptr)
	*/
	void FadeIn(int audioId,  float time, std::function<void()> endCallback = nullptr);

	/*
	@brief : fadeOut Music (endCallback standard : nullptr)
	*/
	void FadeOut(int audioId, float time, std::function<void()> endCallback = nullptr);
};

#endif
