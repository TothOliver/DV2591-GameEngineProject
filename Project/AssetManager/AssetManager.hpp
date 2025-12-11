#pragma once
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <queue>
#include <condition_variable>
#include "IResource.hpp"
#include "ResourceFactory.hpp"

struct PackageEntry {
    ResourceType type;
    uint32_t offset;
    uint32_t size;
};

struct AssetManagerDebugInfo
{
    size_t memoryUsed = 0;
    size_t memoryLimit = 0;
    size_t loadedResourceCount = 0;
    size_t asyncQueuedJobs = 0;
    size_t asyncActiveJobs = 0;
    size_t totalEvictions = 0;
};

class AssetManager {
public:
    AssetManager(size_t memoryLimitBytes, const std::string& packagePath);
    ~AssetManager();
    std::shared_ptr<IResource> Load(const std::string& guid);
    void Unload(const std::string& guid);

    void LoadAsync(const std::string& guid);
    bool IsLoaded(const std::string& guid) const;
    std::shared_ptr<IResource> TryGet(const std::string& guid);

    void DumpLoadedResources() const;

    void GetDebugInfo(AssetManagerDebugInfo& outinfo) const;

private:
    std::string m_packagePath;
    size_t m_memoryLimit;
    size_t m_memoryUsed = 0;

    mutable std::shared_mutex m_registryMutex;
    mutable std::mutex m_loadedMutex;
    mutable std::mutex m_memoryMutex;
    mutable std::mutex m_jobQueueMutex;

    std::unordered_map<std::string, std::shared_ptr<IResource>> m_loaded;
    std::unordered_map<std::string, PackageEntry> m_registry;

    struct LoadJob{
        std::string guid;
    };

    std::queue<LoadJob> m_jobQueue;
    std::condition_variable m_jobAvailable;
    std::unordered_set<std::string> m_inAction;

    std::thread m_worker;
    bool m_stopWorker = false;
    size_t m_totalEvictions = 0;

    void WorkerLoop();
    std::vector<uint8_t> ReadFromPackage(const PackageEntry& entry);
    void EvictIfNeeded(size_t neededMemory);
    bool PackageParser();
    void EraseJob(const std::string& guid);

};
