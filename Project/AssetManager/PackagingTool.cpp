#include "AssetManager/PackagingTool.hpp"
#include <iostream>
#include <cstdint>
#include <fstream>
#include <sstream>


bool PackagingTool::buildPackage(const std::string& mappingFile, const std::string& outputFile)
{
	std::vector<AssetMetaData> metadata;
	std::vector<std::vector<uint8_t>> data;

	if(!readMappingFile(mappingFile, metadata, data)){
		std::cerr << "Error: readMappingFile failed" << std::endl;
		return false;
	}

	if(!writePackage(outputFile, metadata, data)){
		std::cerr << "Error: writePackage failed" << std::endl;
		return false;
	}
	return true;
}

bool PackagingTool::readMappingFile(const std::string& path, 
	std::vector<AssetMetaData>& assetData, std::vector<std::vector<uint8_t>>& outData)
{
	std::ifstream file(path);
	if(!file){
		std::cerr << "Error: Could not open map file" << path << std::endl;
		return false;
	}

	std::string line;

	while (std::getline(file, line))
	{
		if (line.empty()) continue;

		std::stringstream ss(line);
		std::string guid, filename, typeString; // order in the asset text file

		std::getline(ss, guid, ',');
		std::getline(ss, filename, ',');
		std::getline(ss, typeString, ',');

		AssetMetaData metaData;

		metaData.resourceType = parseType(typeString); //converts string into type
		metaData.guid = guid;
		metaData.filename = filename;


		std::vector<uint8_t> bytes;

		if (!loadAssetFile(filename, bytes)) {
			std::cerr<< "Error in packaging tool: not loading assetfile:"<< filename << std::endl;
			return false;
		}

		metaData.uncomp_size = static_cast<uint32_t>(bytes.size());

		assetData.push_back(metaData);
		outData.push_back(std::move(bytes));
	}

	return true;
}

bool PackagingTool::writePackage(const std::string& outputPath, 
	std::vector<AssetMetaData>& md, const std::vector<std::vector<uint8_t>>& data)
{
	if(md.size() != data.size()){
		std::cerr << "Error: metadata and data size mismatch" << std::endl;
        return false;
	}

	std::ofstream out(outputPath, std::ios::binary);
	if(!out){
		std::cerr << "Error: Could not stream outputPath" << outputPath << std::endl;
		return false;
	}
	std::string header = "{\"assets\":[";

	int currentOffset = 0;

	for(int i = 0; i < md.size(); ++i)
	{
		md[i].offset = currentOffset;

		header += 
			"{\"guid\": \"" + md[i].guid + "\", "
			"\"type\": " + std::to_string((int)md[i].resourceType) + ", "
			"\"offset\": " + std::to_string(md[i].offset) + ", "
			"\"size\": " + std::to_string(md[i].uncomp_size) + " }";

		if(i < md.size() - 1)
			header += ",";

		currentOffset += md[i].uncomp_size;
	}
	header += "] }";

	uint32_t headerSize = static_cast<uint32_t>(header.size());
	out.write(reinterpret_cast<const char*>(&headerSize), sizeof(headerSize));
	out.write(header.data(), header.size());

	for(size_t i = 0; i < data.size(); ++i) 
	{
		out.write(reinterpret_cast<const char*>(data[i].data()), data[i].size());
		if(!out){
			std::cerr << "Error: failed to write ..." << std::endl;
			return false;
		}
	}

	return true;
}

bool PackagingTool::loadAssetFile(const std::string& path, std::vector<uint8_t>& outBytes)
{
	std::ifstream file(path, std::ios::binary);
	if(!file){
		std::cerr << "Error: Could not open asset file: " << path << std::endl;
		return false;
	}

	file.seekg(0, std::ios::end);
	std::streamsize size = file.tellg();
	if(size < 0){
		std::cerr << "Error: Invalid file size: " << path << std::endl;
		return false;
	}
	
	outBytes.resize(static_cast<size_t>(size));
	file.seekg(0, std::ios::beg);

	if(!file.read(reinterpret_cast<char*>(outBytes.data()), size)){
		std::cerr << "Error: Failed to read file data: " << path << std::endl;
		return false;
	}

	return true;
}

ResourceType PackagingTool::parseType(const std::string& type)
{
	ResourceType resourceType;
	if(type == "TexturePng") {
		resourceType = ResourceType::TexturePng;
	}
	else if (type == "ProgressiveTexturePng") {
		resourceType = ResourceType::ProgressiveTexturePng;
	}
	else if(type == "Mesh"){
		resourceType = ResourceType::Mesh;
	}
	else{
		std::cerr << "Error: Unknown asset type: " << type << std::endl;
		resourceType = ResourceType::Unknown;
	}

	return resourceType;
}
