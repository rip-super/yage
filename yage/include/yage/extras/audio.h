#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace yage
{
    struct AudioParams
    {
        bool loop = false;
        float volume = 1.0f;
        float pitch = 1.0f;
        float fade_in = 0.0f;
        float fade_out = 0.0f;
    };

    class Audio
    {
    public:
        Audio();
        ~Audio();

        void Load(const std::string &id, const std::string &path, int pool_size = 4);

        void Play(const std::string &id, const AudioParams &params = {});
        void Stop(const std::string &id);
        void Pause(const std::string &id);
        void Resume(const std::string &id);

        bool IsPlaying(const std::string &id) const;

        void SetVolume(const std::string &id, float volume);
        void SetMasterVolume(float volume);

        void StopAll();

    private:
        struct SoundPool
        {
            std::string path;
            std::vector<void *> instances;
            float volume = 1.0f;
            int robin = 0;
        };

        void *context = nullptr;
        std::unordered_map<std::string, SoundPool> pools;
        float master_volume = 1.0f;
    };
}