#include "SpineResource.h"

#include "IwResManager.h"
#include "IwTextParserITX.h"

#include <iostream>
#include <fstream>
#include <vector>

#include "IwDebug.h"
#include "Debug.h"

IW_MANAGED_IMPLEMENT_FACTORY(SpineResource);

void SpineResource::Serialise() {
	IW_CALLSTACK_SELF;
	
	CIwResource::Serialise();
	SerialiseString(m_sJsonFile);
	SerialiseString(m_sJsonData);
	SerialiseString(m_sAtlasFile);
	SerialiseString(m_sAtlasData);
}

bool SpineResource::ParseAttribute(CIwTextParserITX* pParser, const char* pAttrName) {
	IW_CALLSTACK_SELF;
		
	CIwStringL buf;
	if (!strcmp(pAttrName, "json")) {
		pParser->ReadString(buf);
		m_sJsonFile = buf.c_str();
		m_sJsonData = GetFileData(m_sJsonFile);
		
	} else if (!strcmp(pAttrName, "atlas")) {
		pParser->ReadString(buf);
		m_sAtlasFile = buf.c_str();
		m_sAtlasData = GetFileData(m_sAtlasFile);
	} else {
		return CIwManaged::ParseAttribute(pParser, pAttrName);
	}
	return true;
}

void SpineResource::ParseClose(CIwTextParserITX* pParser) {
	IW_CALLSTACK_SELF;

#ifndef IW_DEBUG
    IwAssertMsg(MYAPP, false, ("Project not built with IW_DEBUG"));
#else
    IwAssertMsgTrap(MYAPP, IwGetResManager()->GetCurrentGroup(), ("No current resource group - don't know what to do with created object"), return);
    IwGetResManager()->GetCurrentGroup()->AddRes(SPINERESOURCE_TYPE, this);
#endif
}

void SpineResource::SerialiseString(std::string& s) {
	unsigned int length = s.length();
	IwSerialiseUInt32(length);
	for (unsigned int i = 0; i < length; i++) {
		if (IwSerialiseIsReading()) {
			char c;
			IwSerialiseChar(c);
			s.push_back(c);
		} else if (IwSerialiseIsWriting()) {
			char c = s.at(i);
			IwSerialiseChar(c);
		} else {
			IwAssertMsg(MYAPP, false, ("Neither reading nor writing? Seems wrong..."));
		}
	}
}

std::string SpineResource::GetFileData(const std::string& filename) {
	IW_CALLSTACK_SELF;
	if (filename.empty()) {
		IwAssertMsg(MYAPP, !filename.empty(), ("Cannot read file with empty name."));
		return "";
	}
	
	std::ifstream ifs(filename.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
	std::ifstream::pos_type size = ifs.tellg();

	if (size > 0) {
		ifs.seekg(0, std::ios::beg);
		std::vector<char> bytes(size);
		ifs.read(&bytes[0], size);
		return std::string(&bytes[0], size);
	} else {
		IwAssertMsg(MYAPP, false, ("File does not exist or is empty: %s", filename.c_str()));
	}
	return "";
}
