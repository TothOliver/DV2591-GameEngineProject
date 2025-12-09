#pragma once
#include <string>
#include <vector>
#include "AssetManager/ResourceTypeEnum.h"

struct AssetMetaData
{
	std::string guid;
	std::string filename;
	ResourceType resourceType;

	size_t uncomp_size = 0;

	size_t comp_size = 0;

	size_t offset = 0;

};

class PackagingTool
{
public:
	bool buildPackage(const std::string& mappingFile, const std::string& outputFile);

private:
	bool readMappingFile(const std::string& path,std::vector<AssetMetaData>& assetData, std::vector<std::vector<uint8_t>>& outData);
	bool writePackage(const std::string& outputPath, std::vector<AssetMetaData>& metadata, const std::vector<std::vector<uint8_t>>& data);
	bool loadAssetFile(const std::string& path, std::vector<uint8_t>& outBytes);
	
	ResourceType parseType(const std::string& type);

};