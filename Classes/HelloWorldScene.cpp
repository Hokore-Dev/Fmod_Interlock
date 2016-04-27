/*********************************************************************
* Date : 2016.04.26
* Name : Fmod (Cocos2d-X ver 3.X)
* Email : create(pbes0707@gmail.com) / modify(mark4215@naver.com)
* GitHub : https://github.com/haminjun/Fmod_Interlock
* Cocos2d-x and linked the fmod library
***********************************************************************/
#include "HelloWorldScene.h"
#include "FmodAudioPlayer.h"

#if (CC_TARGET_PLATFORM != CC_PLATFORM_WIN32)
#include <dirent.h>
#include <sys/stat.h>
#endif

USING_NS_CC;

Scene* HelloWorld::createScene()
{
    auto scene = Scene::create();
    auto layer = HelloWorld::create();
    scene->addChild(layer);
    return scene;
}

bool HelloWorld::init()
{
    if ( !Layer::init() )
    {
        return false;
    }
    
	// Create Directory User Write Area
	// Win - User\projectName\Music
	createDirectory("Music");

	// Load Music (Copy Write Area)
	FmodAudioPlayer::getInstance()->loadMusic(
		"Music/time.mp3", FmodAudioPlayer::DECOMPRESS_ON_MEMORY,
		[&](bool load, int audioId)
	{
		// Ended Load
		if (!load)
			return;
		float delay = FmodAudioPlayer::getInstance()->playLoadedMusic(audioId);
	});
    
    return true;
}


void HelloWorld::createDirectory(std::string dir)
{
	std::string pathToSave = FileUtils::getInstance()->getWritablePath() + dir;

	// Create the folder if it doesn't exist
#if (CC_TARGET_PLATFORM != CC_PLATFORM_WIN32)
	DIR *pDir = NULL;
	pDir = opendir(pathToSave.c_str());
	if (!pDir)
	{
		mkdir(pathToSave.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
	}
#else
	if ((GetFileAttributesA(pathToSave.c_str())) == INVALID_FILE_ATTRIBUTES)
	{
		CreateDirectoryA(pathToSave.c_str(), 0);
	}
#endif
}
