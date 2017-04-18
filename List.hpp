/////////////////////////////////////////////////////////////////////////////
// ListNode class (not a thread-safe implementation)
// List class (not a thread-safe implementation)

/////////////////////////////////////////////////////////////////////////////
#pragma once

//#define NDEBUG

#include <iostream>
#include <string>
#include <map>
#include <cassert>
#include <cstdio>
#include <cstring>
// #include <cstdint> // requires c++11 support

using std::cout;
using std::endl;
using std::map;

/////////////////////////////////////////////////////////////////////////////
bool isempty(const char* str)
{
	return (!str || 0 == strlen(str));
}

/////////////////////////////////////////////////////////////////////////////
struct Header
{
	unsigned int m_nMarker;
	unsigned int m_nFormatVersion;
	unsigned int m_nNodesCount;
};

/////////////////////////////////////////////////////////////////////////////
struct ListNode
{
	ListNode* prev;
	ListNode* next;
	// Указатель на произвольный элемент данного списка либо NULL
	ListNode* rand;
	std::string data;
};

/////////////////////////////////////////////////////////////////////////////
class List
{
public:
	// Сохранение в файл (файл открыт с помощью fopen(path, "wb"))
	void Serialize(FILE* file);
	// Загрузка из файла (файл открыт с помощью fopen(path, "rb"))
	void Deserialize(FILE* file);

private:
	ListNode* head; // better use prefix m_
	ListNode* tail; // better use prefix m_
	int count; // better use prefix m_

public:
	List();
	~List();
	void Clear();
	void DumpList();
	void InitNode(ListNode* _pNode);
	ListNode* AddNode(const char* _szData);
	static void SetRand(ListNode* _pNode, ListNode* _pRand);
	int Serialize(const char* _szFilePath);
	int Deserialize(const char* _szFilePath);

private:
	static const unsigned int m_nMarker;
	static const unsigned int m_nFormatVersion;
	FILE* m_pFile;
	map<ListNode*, unsigned int> m_mapNodeToId;

private:
	int SerializeHeader(FILE* _pFile);
	int SerializeNode(FILE* _pFile, ListNode* _pNode);
	int DeserializeHeader(FILE* _pFile, int& _iCount);
	int DeserializeNode(FILE* _pFile, char*& _pszTempBuf, size_t& _sizeTempBuf);
	int CreateNodesIndexes();
	unsigned int GetNodeId(ListNode* _pNode);
	int FixupNodeIndexes();
};

/////////////////////////////////////////////////////////////////////////////
const unsigned int List::m_nMarker = 0x11223344;
const unsigned int List::m_nFormatVersion = 1;

/////////////////////////////////////////////////////////////////////////////
List::List()
	: head(NULL)
	, tail(NULL)
	, count(0)
	, m_pFile(NULL)
{
}

/////////////////////////////////////////////////////////////////////////////
List::~List()
{
	Clear();
}

/////////////////////////////////////////////////////////////////////////////
void List::Clear()
{
	ListNode* pNext = head;
	while (pNext)
	{
		ListNode* pNode = pNext;
		pNext = pNode->next;
		delete pNode;
	}

	head = NULL;
	tail = NULL;
	count = 0;

	if (m_pFile) fclose(m_pFile), m_pFile = NULL;
	m_mapNodeToId.clear();
}

/////////////////////////////////////////////////////////////////////////////
void List::DumpList()
{
	cout << "List dump:" << endl
		 << "  head: " << head << endl
		 << "  tail: " << tail << endl
		 << "  count: " << count << endl;

	int i = 0;
	for (ListNode* pNode = head; pNode; pNode = pNode->next, ++i)
	{
		cout << "Node #" << i << " (" << pNode << "):" << endl
		     << "  prev: " << pNode->prev << endl
		     << "  next: " << pNode->next << endl
		     << "  rand: " << pNode->rand << endl
		     << "  data: \'" << pNode->data << "\'" << endl;
	}
}

/////////////////////////////////////////////////////////////////////////////
void List::InitNode(ListNode* _pNode)
{
	assert(_pNode);

	_pNode->data.clear();
	_pNode->prev = NULL;
	_pNode->next = NULL;
	_pNode->rand = NULL;
}

/////////////////////////////////////////////////////////////////////////////
ListNode* List::AddNode(const char* _szData)
{
	if (!_szData)	{
		cout << "Error: data string ptr is NULL";
		return NULL;
	}

	ListNode* pNode = new ListNode;
	assert(pNode);

	InitNode(pNode);

	pNode->data = _szData;

	// cout << "data: \'" << pNode->data << "\'" << endl;

	if (!head)
	{
		head = pNode;
	}

	if (!tail)
	{
		tail = pNode;
	}
	else
	{
		tail->next = pNode;
		pNode->prev = tail;
		tail = pNode;
	}

	++count;

	// cout << "head: " << head << endl;
	// cout << "tail: " << tail << endl;
	// cout << "count: " << count << endl;

	// cout << count << " nodes added" << endl;
	return pNode;
}

/////////////////////////////////////////////////////////////////////////////
void List::SetRand(ListNode* _pNode, ListNode* _pRand)
{
	assert(_pNode && _pRand);

	_pNode->rand = _pRand;
}

/////////////////////////////////////////////////////////////////////////////
// Serialize methods
/////////////////////////////////////////////////////////////////////////////
int List::Serialize(const char* _szFilePath)
{
	if (isempty(_szFilePath))
	{
		cout << "Error: serialization file name is empty" << endl;
		return -1;
	}

	cout << "Serializing to \'" << _szFilePath << "\'" << endl;

	if (m_pFile)
	{
		fclose(m_pFile), m_pFile = NULL;
	}
	m_pFile = fopen(_szFilePath, "wb");
	if (!m_pFile)
	{
		cout << "Error: fopen(" << _szFilePath << ") failed" << endl;
		return -1;
	}

	Serialize(m_pFile);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// Should return void according to test requirements
void List::Serialize(FILE* file)
{
	int ret = 0;

	assert(file);

	try
	{
		// create relation table

		// serialize header
		int iRet = SerializeHeader(file);
		if (iRet)
		{
			throw -1;
		}

		iRet = CreateNodesIndexes();
		if (iRet)
		{
			throw -1;
		}

		// serialize nodes
		for (ListNode* pNode = head; pNode; pNode = pNode->next)
		{
			iRet = SerializeNode(file, pNode);
			if (iRet)
			{
				throw -1;
			}
		}

		// index is not needed anymore
		m_mapNodeToId.clear();
	}
	catch(int ex)
	{
	}
}

/////////////////////////////////////////////////////////////////////////////
int List::SerializeHeader(FILE* _pFile)
{
	Header header;
	header.m_nMarker = m_nMarker;
	header.m_nFormatVersion = m_nFormatVersion;
	header.m_nNodesCount = count;

	size_t sizeToWrite = sizeof(header);
	fwrite(static_cast<const void*>(&header), sizeToWrite, 1, _pFile);
	if (ferror(_pFile))
	{
		cout << "Error: failed to write serialization header" << endl;
		return -1;
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
int List::SerializeNode(FILE* _pFile, ListNode* _pNode)
{
	// Serialization frame:
	// +0: rand index (unsigned int)
	// +4: size of data field (unsigned int size)
	// +8: char string (size of data)

	assert(_pFile);
	assert(_pNode);

	// store rand index
	unsigned int nRandId = GetNodeId(_pNode->rand);
	size_t sizeToWrite = sizeof(nRandId);
	fwrite(static_cast<const void*>(&nRandId), sizeToWrite, 1, _pFile);
	if (ferror(_pFile))
	{
		cout << "Error: failed to write node rand index" << endl;
		return -1;
	}

	// store size
	unsigned int nDataSize = _pNode->data.size();
	sizeToWrite = sizeof(nDataSize);
	fwrite(static_cast<const void*>(&nDataSize), sizeToWrite, 1, _pFile);
	if (ferror(_pFile))
	{
		cout << "Error: failed to write node data" << endl;
		return -1;
	}

	// store data
	sizeToWrite = nDataSize;
	fwrite(static_cast<const void*>(_pNode->data.c_str()), sizeToWrite, 1, _pFile);
	if (ferror(_pFile))
	{
		cout << "Error: failed to write node data" << endl;
		return -1;
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// Deserialize methods
/////////////////////////////////////////////////////////////////////////////
int List::Deserialize(const char* _szFilePath)
{
	cout << "Deserializing from \'" << _szFilePath << "\'" << endl;

	if (isempty(_szFilePath))
	{
		cout << "Error: serialization file name is empty" << endl;
		return -1;
	}

	Clear();

	m_pFile = fopen(_szFilePath, "rb");
	if (!m_pFile)
	{
		cout << "Error: fopen(" << _szFilePath << ") failed" << endl;
		return -1;
	}

	Deserialize(m_pFile);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// Should return void according to test requirements
void List::Deserialize(FILE* file)
{
	int ret = 0;
	char* pszTempBuf = NULL;
	size_t sizeTempBuf = 0;

	assert(file);

	try
	{
		// deserialize header
		int iCount = 0;
		int iRet = DeserializeHeader(file, iCount);
		if (0 != iRet)
		{
			throw -1;
		}

		if (0 == iCount)
		{
			// no nodes present
			throw 0;
		}

		// reuse it to store rand indexes
		m_mapNodeToId.clear();

		// deserialize nodes
		for (int i = 0; i < iCount; ++i)
		{
			iRet = DeserializeNode(file, pszTempBuf, sizeTempBuf);
			if (iRet)
			{
				throw -1;
			}
		}

		// replace all rand node indexes by pointers
		iRet = FixupNodeIndexes();
		if (iRet)
		{
			throw -1;
		}

		ret = 0;
	}
	catch(int ex)
	{
		ret = ex;
	}

	if(0 != ret)
	{
		Clear();
	}

	if (pszTempBuf) free(pszTempBuf), pszTempBuf = NULL;
}

/////////////////////////////////////////////////////////////////////////////
int List::DeserializeHeader(FILE* _pFile, int& _iCount)
{
	Header header;

	size_t sizeToRead = sizeof(header);
	size_t sizeCount = 1;
	size_t sizeCountRead = fread(static_cast<void*>(&header),
		sizeToRead, sizeCount, _pFile);
	if (sizeCount != sizeCountRead)
	{
		cout << "Error: failed to read marker" << endl;
		return -1;
	}

	cout << "Header" << endl
	     << "  marker: " << header.m_nMarker << endl
	     << "  format: " << header.m_nFormatVersion << endl
	     << "  nodes : " << header.m_nNodesCount << endl;

	if (m_nMarker != header.m_nMarker)
	{
		cout << "Invalid serialization file" << endl;
		return -1;
	}

	if (m_nFormatVersion != header.m_nFormatVersion)
	{
		cout << "Unsupported serialization file format" << endl;
		return -1;
	}

	_iCount = header.m_nNodesCount;

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
int List::DeserializeNode(
	FILE* _pFile,
	char*& _pszTempBuf,
	size_t& _sizeTempBuf)
{
	int ret = 0;
	// Serialization frame:
	// +0: size of data field (int32_t size)
	// +4: char string (size of data)

	try
	{
		assert(_pFile);

		unsigned int nRandId;

		// read rand id
		size_t sizeToRead = sizeof(nRandId);
		size_t sizeCount = 1;
		size_t sizeCountRead = fread(static_cast<void*>(&nRandId),
			sizeToRead, sizeCount, _pFile);
		if (sizeCount != sizeCountRead)
		{
			cout << "Error: failed to read node rand index" << endl;
			throw -1;
		}

		unsigned int nDataSize = 0;

		// read size
		sizeToRead = sizeof(nDataSize);
		sizeCount = 1;
		sizeCountRead = fread(static_cast<void*>(&nDataSize),
			sizeToRead, sizeCount, _pFile);
		if (sizeCount != sizeCountRead)
		{
			cout << "Error: failed to read node data size" << endl;
			throw -1;
		}

		// provide buffer of enough size
		if (_sizeTempBuf < nDataSize + 1)
		{
			char* ptr = static_cast<char*>(realloc(_pszTempBuf, nDataSize + 1));
			assert (ptr);

			_pszTempBuf = ptr;
			_sizeTempBuf = nDataSize + 1;
		}

		if (nDataSize > 0)
		{
			// read data
			sizeToRead = nDataSize;
			sizeCount = 1;
			sizeCountRead = fread(static_cast<void*>(_pszTempBuf),
				sizeToRead, sizeCount, _pFile);
			if (sizeCount != sizeCountRead)
			{
				cout << "Error: failed to read node data size" << endl;
				throw -1;
			}
		}

		_pszTempBuf[nDataSize] = '\0';

		ListNode* pNode = AddNode(_pszTempBuf);
		assert(pNode);

		m_mapNodeToId[pNode] = nRandId;

		ret = 0;
	}
	catch (int ex)
	{
		ret = ex;
	}

	return ret;
}

/////////////////////////////////////////////////////////////////////////////
// Nodes indexation
/////////////////////////////////////////////////////////////////////////////
int List::CreateNodesIndexes()
{
	m_mapNodeToId.clear();

	unsigned int id = 0;
	for (ListNode* pNode = head; pNode; pNode = pNode->next, ++id)
	{
		m_mapNodeToId[pNode] = id;
	}

	// for (map<ListNode*,int>::iterator it = m_mapNodeToId.begin(); it != m_mapNodeToId.end(); ++it)
	// {
	// 	cout << it->first << " => " << it->second << '\n';
	// }

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
unsigned int List::GetNodeId(ListNode* _pNode)
{
	assert(_pNode);

	return m_mapNodeToId.at(_pNode);
}

/////////////////////////////////////////////////////////////////////////////
int List::FixupNodeIndexes()
{
	int ret = 0;
	map<unsigned int, ListNode*> mapIndexes;

	try
	{
		unsigned int id = 0;
		for (ListNode* pNode = head; pNode; pNode = pNode->next, ++id)
		{
			mapIndexes[id] = pNode;
		}

		for (ListNode* pNode = head; pNode; pNode = pNode->next)
		{
			unsigned int id = m_mapNodeToId.at(pNode);
			pNode->rand = mapIndexes[id];
		}

		ret = 0;
	}
	catch (int ex)
	{
		cout << "Error: exception" << endl;
		ret = ex;
	}

	mapIndexes.clear();
	m_mapNodeToId.clear();

	return ret;
}
