/*********************************************************************
* Date : 2016.04.26
* Name : Fmod (Cocos2d-X ver 3.X)
* Email : create(pbes0707@gmail.com) / modify(mark4215@naver.com)
* GitHub : https://github.com/haminjun/Joystick_Lib
* Cocos2d-x and linked the fmod library
***********************************************************************/
#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include "cocos2d.h"
#include <sstream>

class HelloWorld : public cocos2d::Layer
{
public:
    static cocos2d::Scene* createScene();

    virtual bool init();

	/*
	@brief : Copy User Write Area
	*/
	void createDirectory(std::string dir);

    CREATE_FUNC(HelloWorld);
};

#endif
