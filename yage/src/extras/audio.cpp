#include "yage/extras/audio.h"
#include <miniaudio.h>
#include <iostream>
#include <filesystem>

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

namespace yage
{
    static std::string resolveAudioPath(const std::string &path)
    {
        if (std::filesystem::path(path).is_absolute() || std::filesystem::exists(path))
            return path;

#ifdef __APPLE__
        char buf[1024];
        uint32_t size = sizeof(buf);
        if (_NSGetExecutablePath(buf, &size) == 0)
        {
            auto exeDir = std::filesystem::canonical(buf).parent_path();
            auto candidate = exeDir / path;
            if (std::filesystem::exists(candidate))
                return candidate.string();
        }
#else
        auto exeDir = std::filesystem::canonical("/proc/self/exe").parent_path();
        auto candidate = exeDir / path;
        if (std::filesystem::exists(candidate))
            return candidate.string();
#endif
        return path;
    }

    Audio::Audio()
    {
        ma_engine *engine = new ma_engine;
        if (ma_engine_init(nullptr, engine) != MA_SUCCESS)
        {
            std::cerr << "Failed to initialize miniaudio engine\n";
            delete engine;
            return;
        }
        context = engine;
    }

    Audio::~Audio()
    {
        StopAll();

        for (auto &[id, pool] : pools)
            for (void *ptr : pool.instances)
            {
                ma_sound *s = (ma_sound *)ptr;
                ma_sound_uninit(s);
                delete s;
            }
        pools.clear();

        if (context)
        {
            ma_engine_uninit((ma_engine *)context);
            delete (ma_engine *)context;
        }
    }

    void Audio::Load(const std::string &id, const std::string &path, int pool_size)
    {
        if (!context)
            return;

        ma_engine *engine = (ma_engine *)context;
        std::string resolved = resolveAudioPath(path);

        if (pools.count(id))
        {
            for (void *ptr : pools[id].instances)
            {
                ma_sound *s = (ma_sound *)ptr;
                ma_sound_uninit(s);
                delete s;
            }
            pools.erase(id);
        }

        SoundPool pool;
        pool.path = resolved;
        pool.robin = 0;

        for (int i = 0; i < pool_size; ++i)
        {
            ma_sound *sound = new ma_sound;
            if (ma_sound_init_from_file(engine, resolved.c_str(), 0, nullptr, nullptr, sound) != MA_SUCCESS)
            {
                std::cerr << "Failed to load audio file: " << resolved << "\n";
                delete sound;
                for (void *ptr : pool.instances)
                {
                    ma_sound_uninit((ma_sound *)ptr);
                    delete (ma_sound *)ptr;
                }
                return;
            }
            pool.instances.push_back(sound);
        }

        pools[id] = std::move(pool);
    }

    void Audio::Play(const std::string &id, const AudioParams &params)
    {
        auto it = pools.find(id);
        if (it == pools.end())
            return;

        SoundPool &pool = it->second;
        if (pool.instances.empty())
            return;

        int start = pool.robin;
        ma_sound *sound = nullptr;
        for (int i = 0; i < (int)pool.instances.size(); ++i)
        {
            int idx = (start + i) % pool.instances.size();
            ma_sound *candidate = (ma_sound *)pool.instances[idx];
            if (!ma_sound_is_playing(candidate))
            {
                sound = candidate;
                pool.robin = (idx + 1) % pool.instances.size();
                break;
            }
        }

        if (!sound)
        {
            sound = (ma_sound *)pool.instances[pool.robin];
            ma_sound_stop(sound);
            ma_sound_seek_to_pcm_frame(sound, 0);
            pool.robin = (pool.robin + 1) % pool.instances.size();
        }

        pool.volume = params.volume;
        ma_sound_set_volume(sound, params.volume);
        ma_sound_set_pitch(sound, params.pitch);
        ma_sound_set_looping(sound, params.loop);

        if (params.fade_in > 0.0f)
            ma_sound_set_fade_in_milliseconds(sound, 0.0f, params.volume, (ma_uint64)(params.fade_in * 1000));

        ma_sound_seek_to_pcm_frame(sound, 0);
        ma_sound_start(sound);
    }

    void Audio::Stop(const std::string &id)
    {
        auto it = pools.find(id);
        if (it == pools.end())
            return;

        for (void *ptr : it->second.instances)
        {
            ma_sound *sound = (ma_sound *)ptr;
            ma_sound_stop(sound);
            ma_sound_seek_to_pcm_frame(sound, 0);
        }
    }

    void Audio::Pause(const std::string &id)
    {
        auto it = pools.find(id);
        if (it == pools.end())
            return;

        for (void *ptr : it->second.instances)
            ma_sound_stop((ma_sound *)ptr);
    }

    void Audio::Resume(const std::string &id)
    {
        auto it = pools.find(id);
        if (it == pools.end())
            return;

        for (void *ptr : it->second.instances)
        {
            ma_sound *sound = (ma_sound *)ptr;
            if (!ma_sound_is_playing(sound))
                ma_sound_start(sound);
        }
    }

    bool Audio::IsPlaying(const std::string &id) const
    {
        auto it = pools.find(id);
        if (it == pools.end())
            return false;

        for (void *ptr : it->second.instances)
            if (ma_sound_is_playing((ma_sound *)ptr))
                return true;
        return false;
    }

    void Audio::SetVolume(const std::string &id, float volume)
    {
        auto it = pools.find(id);
        if (it == pools.end())
            return;

        it->second.volume = volume;
        for (void *ptr : it->second.instances)
            ma_sound_set_volume((ma_sound *)ptr, volume);
    }

    void Audio::SetMasterVolume(float volume)
    {
        master_volume = volume;
        ma_engine_set_volume((ma_engine *)context, volume);
    }

    void Audio::StopAll()
    {
        for (auto &[id, pool] : pools)
            for (void *ptr : pool.instances)
            {
                ma_sound *sound = (ma_sound *)ptr;
                ma_sound_stop(sound);
                ma_sound_seek_to_pcm_frame(sound, 0);
            }
    }
}