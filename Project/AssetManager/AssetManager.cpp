#include "AssetManager.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>

AssetManager::AssetManager(size_t memoryLimitBytes, const std::string& packagePath)
    : m_memoryLimit(memoryLimitBytes), m_packagePath(packagePath)
{
    PackageParser();

    m_worker = std::thread(&AssetManager::WorkerLoop, this);
}

AssetManager::~AssetManager()
{
    {
        std::scoped_lock lock(m_jobQueueMutex);
        m_stopWorker = true;
    }

    m_jobAvailable.notify_all();

    if(m_worker.joinable()){
        m_worker.join();
    }

    // Unloading
    {
        std::scoped_lock lock(m_loadedMutex);

        for (auto& [guid, resource] : m_loaded)
        {
            resource->Unload();
        }

        m_loaded.clear();
        m_memoryUsed = 0;
    }
}

std::shared_ptr<IResource> AssetManager::Load(const std::string& guid)
{
    {
        std::scoped_lock lock(m_loadedMutex);
        auto it = m_loaded.find(guid);
        if (it != m_loaded.end())
            return it->second;
    }

    PackageEntry entry;
    {
        std::shared_lock lock(m_registryMutex);
        auto regIt = m_registry.find(guid);
        if (regIt == m_registry.end()) 
            return nullptr;
        entry = regIt->second;
    }

    std::vector<uint8_t> data = ReadFromPackage(entry);
    if (data.empty()) return nullptr;

    std::shared_ptr<IResource> resource = ResourceFactory::Create(guid, entry.type);
    if (!resource) return nullptr;

    if (!resource->Load(data)) return nullptr;

    {
        std::scoped_lock lock(m_loadedMutex);
        EvictIfNeeded(resource->GetSize());
        m_memoryUsed += resource->GetSize();
        m_loaded[guid] = resource;
    }
    return resource;
}

void AssetManager::Unload(const std::string& guid)
{
    std::scoped_lock lock(m_loadedMutex);
    auto it = m_loaded.find(guid);
    if (it == m_loaded.end())
        return;

    m_memoryUsed -= it->second->GetSize();
    it->second->Unload();
    m_loaded.erase(it);
}

void AssetManager::LoadAsync(const std::string& guid)
{
    {
        std::scoped_lock lock(m_loadedMutex);
        auto it = m_loaded.find(guid);
        if (it != m_loaded.end())
            return;
    }

    {
        std::scoped_lock lock(m_jobQueueMutex);
        auto actionIt = m_inAction.find(guid);
        if (actionIt != m_inAction.end())
            return;

        auto regIt = m_registry.find(guid);
        if (regIt == m_registry.end()) {
            std::cerr << "ResourceManager::LoadAsync Unknown GUID: " << guid << "\n";
            return;
        }

        m_inAction.insert(guid);

        LoadJob job;
        job.guid = guid;
        m_jobQueue.push(job);

        m_jobAvailable.notify_one();
    }

}

bool AssetManager::IsLoaded(const std::string& guid) const
{
    std::scoped_lock lock(m_loadedMutex);
    return m_loaded.find(guid) != m_loaded.end();
}

std::shared_ptr<IResource> AssetManager::TryGet(const std::string& guid)
{
    std::scoped_lock lock(m_loadedMutex);
    auto it = m_loaded.find(guid);
    return (it != m_loaded.end()) ? it->second : nullptr;
}

void AssetManager::DumpLoadedResources() const
{
    std::cout << "Loaded resources:\n";
    for (auto& [guid, res] : m_loaded) 
    {
        std::cout << " - " << guid << " | Size: " << res->GetSize() << " bytes\n";
    }
}

std::vector<uint8_t> AssetManager::ReadFromPackage(const PackageEntry& entry)
{
    std::ifstream file(m_packagePath, std::ios::binary);
    if (!file) return {};

    file.seekg(entry.offset, std::ios::beg);

    std::vector<uint8_t> buffer(entry.size);
    file.read(reinterpret_cast<char*>(buffer.data()), entry.size);

    return buffer;
}

void AssetManager::EvictIfNeeded(size_t neededMemory)
{
    while (m_memoryUsed + neededMemory > m_memoryLimit && !m_loaded.empty()) 
    {
        auto it = m_loaded.begin();
        m_memoryUsed -= it->second->GetSize();
        it->second->Unload();
        m_loaded.erase(it);
        std::cerr << "AssetManager: No space! Evicting resources.\n";
    }

    if (m_memoryUsed + neededMemory > m_memoryLimit) 
    {
        std::cerr << "AssetManager: Memory limit exceeded! Cannot load resource.\n";
    }
}

bool AssetManager::PackageParser()
{

    std::ifstream file(m_packagePath, std::ios::binary);
    if (!file) {
        std::cerr << "AssetManager: Failed to open package: " << m_packagePath << "\n";
        return false;
    }

    uint32_t headerSize = 0;
    file.read(reinterpret_cast<char*>(&headerSize), sizeof(headerSize));
    if (!file) {
        std::cerr << "AssetManager: Failed to read header size\n";
        return false;
    }

    std::string headerJson(headerSize, '\0');
    file.read(&headerJson[0], headerSize);
    if (!file) {
        std::cerr << "AssetManager: Failed to read header data\n";
        return false;
    }

    size_t dataStartOffset = sizeof(uint32_t) + headerSize;

    size_t pos = headerJson.find("\"assets\":[");
    if (pos == std::string::npos) {
        std::cerr << "AssetManager: Invalid header format\n";
        return false;
    }

    pos = headerJson.find('[', pos);
    size_t endPos = headerJson.rfind(']');

    size_t current = pos + 1;
    while (current < endPos) {

        size_t objStart = headerJson.find('{', current);
        if (objStart == std::string::npos || objStart >= endPos) break;

        size_t objEnd = headerJson.find('}', objStart);
        if (objEnd == std::string::npos) break;

        std::string obj = headerJson.substr(objStart, objEnd - objStart + 1);

        PackageEntry entry;
        std::string guid;

        size_t guidPos = obj.find("\"guid\":");
        if (guidPos != std::string::npos) {
            size_t guidStart = obj.find('"', guidPos + 7) + 1;
            size_t guidEnd = obj.find('"', guidStart);
            guid = obj.substr(guidStart, guidEnd - guidStart);
        }

        size_t typePos = obj.find("\"type\":");
        if (typePos != std::string::npos) {
            size_t typeStart = typePos + 7;
            while (typeStart < obj.size() && (obj[typeStart] == ' ' || obj[typeStart] == ':')) typeStart++;
            size_t typeEnd = obj.find_first_of(",}", typeStart);
            int typeInt = std::stoi(obj.substr(typeStart, typeEnd - typeStart));
            entry.type = static_cast<ResourceType>(typeInt);
        }

        size_t offsetPos = obj.find("\"offset\":");
        if (offsetPos != std::string::npos) {
            size_t offsetStart = offsetPos + 9;
            while (offsetStart < obj.size() && (obj[offsetStart] == ' ' || obj[offsetStart] == ':')) offsetStart++;
            size_t offsetEnd = obj.find_first_of(",}", offsetStart);
            entry.offset = std::stoull(obj.substr(offsetStart, offsetEnd - offsetStart));
            entry.offset += dataStartOffset;
        }

        size_t sizePos = obj.find("\"size\":");
        if (sizePos != std::string::npos) {
            size_t sizeStart = sizePos + 7;
            while (sizeStart < obj.size() && (obj[sizeStart] == ' ' || obj[sizeStart] == ':')) sizeStart++;
            size_t sizeEnd = obj.find_first_of(",}", sizeStart);    
            entry.size = std::stoull(obj.substr(sizeStart, sizeEnd - sizeStart));
        }

        if (!guid.empty()) {
            m_registry[guid] = entry;
        }

        current = objEnd + 1;
    }

    std::cout << "AssetManager: Loaded " << m_registry.size() << " assets from package\n";
    
    return true;
}

void AssetManager::WorkerLoop()
{
    while (true){
        LoadJob job;
        {
            std::unique_lock<std::mutex> lock(m_jobQueueMutex);

            while(m_jobQueue.empty() && !m_stopWorker){
                m_jobAvailable.wait(lock);
            }

            if(m_stopWorker && m_jobQueue.empty()){
                return;
            }

            job = m_jobQueue.front();
            m_jobQueue.pop();
        }
        
        auto regIt = m_registry.find(job.guid);
        if(regIt == m_registry.end()){
            std::cerr << "WorkerLoop Error: GUID not found in registry: " << job.guid << std::endl;
            EraseJob(job.guid);
            continue;
        }

        const PackageEntry& entry = regIt->second;
        std::vector<uint8_t> data = ReadFromPackage(entry);
        if(data.empty()){
            std::cerr << "WorkerLoop Error: Failed to read data for GUID: " << job.guid << std::endl;
            EraseJob(job.guid);
            continue;
        }

        std::shared_ptr<IResource> resource = ResourceFactory::Create(job.guid, entry.type);
        if(!resource){
            std::cerr << "WorkerLoop Error: Unsupported resource type for GUID: " << job.guid << std::endl;
            EraseJob(job.guid);
            continue;
        }

        if(!resource->Load(data)){
            std::cerr << "WorkerLoop Error: Resource->Load() failed for GUID: " << job.guid << "\n";
            EraseJob(job.guid);
            continue;
        }

        {
            std::scoped_lock lock(m_loadedMutex);
            EvictIfNeeded(resource->GetSize());
            m_memoryUsed += resource->GetSize();
            m_loaded[job.guid] = resource;
        }

        EraseJob(job.guid);
    }
}

void AssetManager::EraseJob(const std::string& guid)
{
    {
        std::scoped_lock lock(m_jobQueueMutex);
        m_inAction.erase(guid);
    }
}