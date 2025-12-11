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
        m_models.erase(it);
        //m_assetManager->Unload(GUID); //maybe
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
    m_models.erase(it);
    //m_assetManager->Unload(GUID); //maybe
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
