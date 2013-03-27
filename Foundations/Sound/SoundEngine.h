#ifndef __SOUNDENGINE_H__
#define __SOUNDENGINE_H__

#include <map>
#include <string>

#include "IwSound.h"
#include "IwSoundInst.h"
#include "IwResGroup.h"

class SoundEngine {
private:
	static CIwResGroup* s_pSoundResources;

public:
	static void Initialize();
	static void Terminate();

	static void PlayMusicFileLoop(const std::string& filename);
	static void PlayMusicFile(const std::string& filename);
	static void StopMusicFile();

	static void PlaySoundEffect(const std::string& soundname);

	static void Update();
};

#endif
