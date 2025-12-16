#include "RaylibHelper.hpp"

RaylibHelper::RaylibHelper(AssetManager& assetManager)
{
    m_baseTexture = GenerateBaseTexture();
    m_baseModel = GenerateBaseModel();
    m_assetManager = &assetManager;
}

RaylibHelper::~RaylibHelper()
{
	CleanUp();
    UnloadModel(m_baseModel);
    UnloadTexture(m_baseTexture);
}

Texture2D RaylibHelper::GetTexture(std::string GUID)
{
	m_assetManager->LoadAsync(GUID);
	auto& entry = m_textures[GUID];

	if (entry.refCount == 0)
	{
		std::shared_ptr<IResource> res = m_assetManager->TryGet(GUID);
		if (res == nullptr)
		{
            return m_baseTexture;
		}
		entry.texture = GenerateTexture(res);
	}

	entry.refCount++;
	return entry.texture;
}

Model RaylibHelper::GetModel(std::string GUID, std::string name)
{
    m_assetManager->LoadAsync(GUID);
    auto& entry = m_models[name];

    if (entry.refCount == 0)
    {
        std::shared_ptr<IResource> res = m_assetManager->TryGet(GUID);
        if (res == nullptr)
        {
            return m_baseModel;
        }
        if (res->GetResourceType() != ResourceType::Mesh)
        {
            return m_baseModel;
        }
        auto mesh = std::dynamic_pointer_cast<MeshObj>(res);
        entry.guid = GUID;
        entry.model = ConvertAttribToModel(mesh->GetAttrib(), mesh->GetShapes());
    }

    entry.refCount++;
    return entry.model;
}

void RaylibHelper::ReleaseTexture(std::string GUID)
{
    auto it = m_textures.find(GUID);
    if (it == m_textures.end())
    {
        return;
    }

    if (--it->second.refCount == 0)
    {
        UnloadTexture(it->second.texture);
        m_textures.erase(it);
        m_assetManager->Unload(GUID); //maybe
    }
}

void RaylibHelper::ReleaseModel(std::string name)
{
    auto it = m_models.find(name);
    if (it == m_models.end())
    {
        return;
    }

    if (--it->second.refCount == 0)
    {
        UnloadModel(it->second.model);
        m_assetManager->Unload(it->second.guid); //maybe (yes)
        m_models.erase(it);

    }
}


void RaylibHelper::ForceUnloadTexture(std::string GUID)
{
    auto it = m_textures.find(GUID);
    if (it == m_textures.end())
    {
        return;
    }

    UnloadTexture(it->second.texture);
    m_textures.erase(it);
    m_assetManager->Unload(GUID); //maybe
}

void RaylibHelper::ForceUnloadModel(std::string name)
{
    auto it = m_models.find(name);
    if (it == m_models.end())
    {
        return;
    }

    UnloadModel(it->second.model);
    m_assetManager->Unload(it->second.guid); //maybe (yes)
    m_models.erase(it);

}

void RaylibHelper::CleanUp()
{
	//Unload all models
	for (auto& it : m_models)
	{
		UnloadModel(it.second.model);
	}

	//Unload all textures
	for (auto& it : m_textures)
	{
		UnloadTexture(it.second.texture);
	}

	m_models.clear();
	m_textures.clear();
}


Texture2D RaylibHelper::GenerateTexture(std::shared_ptr<IResource>& baseRes)
{
    ResourceType type = baseRes->GetResourceType();

    if (type == ResourceType::TexturePng)
    {
        auto pngRes = std::dynamic_pointer_cast<TexturePng>(baseRes);

        if (!pngRes)
        {
            std::cerr << "Error: Resource for GUID " << baseRes->GetGUID() << " is not a TexturePng.\n";
        }
        else
        {
            Texture2D texture{};
            const unsigned char* pixels = pngRes->GetTexture();

            int width = pngRes->GetWidth();
            int height = pngRes->GetHeight();

            Image img{};
            img.data = (void*)pixels;
            img.width = width;
            img.height = height;
            img.mipmaps = 1;
            img.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
            texture = LoadTextureFromImage(img);

            UnloadImage(img);
            return texture;
        }
    }
    else if (type == ResourceType::ProgressiveTexturePng)
    {
        auto pngRes = std::dynamic_pointer_cast<ProgressiveTexturePng>(baseRes);

        if (!pngRes)
        {
            std::cerr << "Error: Resource for GUID " << baseRes->GetGUID() << " is not a TexturePng.\n";
        }
        else
        {
            Texture2D texture{};
            const unsigned char* pixels = pngRes->GetTexture();

            int width = pngRes->GetWidth();
            int height = pngRes->GetHeight();

            Image img{};
            img.data = (void*)pixels;
            img.width = width;
            img.height = height;
            img.mipmaps = 1;
            img.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
            texture = LoadTextureFromImage(img);

            UnloadImage(img);
            return texture;
        }
    }
	return m_baseTexture;
}

Texture2D RaylibHelper::GenerateBaseTexture()
{
    Image img = GenImageColor(1, 1, WHITE);
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);
    return tex;
}

Model RaylibHelper::GenerateBaseModel()
{
    Mesh cube = GenMeshCube(1.0f, 1.0f, 1.0f);
    Model model = LoadModelFromMesh(cube);
    return model;
}

void RaylibHelper::RequestProgressiveTexture(const std::string& baseGuid, int maxLOD)
{
    // Register state immediately (do NOT depend on TryGet)
    ProgressiveLODState& state = m_progressiveLODs[baseGuid];
    state.baseGuid = baseGuid;
    state.timer = 0.0f;
    state.delay = 2.0f;
    state.maxLOD = maxLOD;
    state.active = true;
    state.nextGuid.clear();
    state.pendingUnload.clear();

    // Start loading base texture
    m_assetManager->LoadAsync(baseGuid);

    // Force GPU texture creation when resource arrives
    GetTexture(baseGuid);
}



void RaylibHelper::Update(float dt)
{
    for (auto& [guid, state] : m_progressiveLODs)
    {
        if (!state.active)
            continue;

        state.timer += dt;
        if (state.timer < state.delay)
            continue;

        auto baseRes = m_assetManager->TryGet(state.baseGuid);
        if (!baseRes)
            continue;

        auto baseTex = std::dynamic_pointer_cast<ProgressiveTexturePng>(baseRes);
        if (!baseTex)
            continue;

        // Initialize LOD metadata once
        if (state.nextGuid.empty())
        {
            baseTex->SetLODInfo(state.maxLOD);

            if (!baseTex->HasHigherLOD())
            {
                state.active = false;
                continue;
            }

            state.nextGuid = baseTex->GetNextLODGuid();
            m_assetManager->LoadAsync(state.nextGuid);
            state.timer = 0.0f;
            continue;
        }

        // Check if next LOD resource is ready
        auto nextRes = m_assetManager->TryGet(state.nextGuid);
        if (!nextRes)
            continue;

        auto nextTex = std::dynamic_pointer_cast<ProgressiveTexturePng>(nextRes);
        if (!nextTex)
            continue;

        // SAFE deep copy of pixel data (no dangling pointers)
        const size_t size =
            static_cast<size_t>(nextTex->GetWidth()) *
            static_cast<size_t>(nextTex->GetHeight()) * 4;

        std::vector<uint8_t> pixelCopy(size);
        memcpy(pixelCopy.data(), nextTex->GetImageData(), size);

        // Load + promote
        if (!baseTex->LoadHigherLOD(pixelCopy, nextTex->GetWidth(), nextTex->GetHeight()))
            continue;

        baseTex->TryUpgrade();

        // Update GPU texture safely
        auto& entry = m_textures[state.baseGuid];

        if (entry.texture.id != 0)
        {
            if (entry.texture.width != baseTex->GetWidth() ||
                entry.texture.height != baseTex->GetHeight())
            {
                UnloadTexture(entry.texture);
                entry.texture = GenerateTexture(baseRes);
            }
            else
            {
                UpdateTexture(entry.texture, baseTex->GetTexture());
            }
        }

        // Delay unload to avoid race conditions
        state.pendingUnload = state.nextGuid;

        // Prepare next LOD or finish
        if (baseTex->HasHigherLOD())
        {
            state.nextGuid = baseTex->GetNextLODGuid();
            m_assetManager->LoadAsync(state.nextGuid);
            state.timer = 0.0f;
        }
        else
        {
            state.active = false;
        }

        // Safe cleanup
        if (!state.pendingUnload.empty())
        {
            m_assetManager->Unload(state.pendingUnload);
            state.pendingUnload.clear();
        }
    }
}



