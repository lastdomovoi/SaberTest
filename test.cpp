/////////////////////////////////////////////////////////////////////////////
// Test app for Saber Interactive
// Dmitry Belousov

/////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include "List.hpp"

/////////////////////////////////////////////////////////////////////////////
// using namespace std;
using std::cout;
using std::endl;
using std::vector;
using std::string;
using std::right;
using std::setw;

/////////////////////////////////////////////////////////////////////////////
static void PrintBinaryTest();
static void PrintBinary(int _iVal);

static void RemoveDupsTest();
static void RemoveDups(char* _pStr);

static void SerializeTest();

/////////////////////////////////////////////////////////////////////////////
template <typename T, size_t size>
size_t countof(T const (&arr)[size])
{
    return size;
}

/////////////////////////////////////////////////////////////////////////////
int main()
{
	PrintBinaryTest();

	RemoveDupsTest();	

	SerializeTest();

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// 1. Напишите функцию, которая принимает на вход знаковое целое число
//    и печатает его двоичное представление, не используя библиотечных
//    классов или функций. 
void PrintBinaryTest()
{
	cout << "* Print Binary Test" << endl;

	int iTestArray[] = { 0, -1, 255, -255, INT_MAX, -INT_MAX };
	for (int i = 0; i < countof(iTestArray); ++i)
	{
		PrintBinary(iTestArray[i]);
	}

	cout << endl;
}

/////////////////////////////////////////////////////////////////////////////
void PrintBinary(int _iVal)
{
	string strResult;
	unsigned int uiBitSize = sizeof(_iVal) * 8;
	int iPos = 0;

	for (unsigned int uiMask = (1 << (uiBitSize - 1));
	     uiMask > 0;
	     uiMask >>= 1)
	{
		strResult += (uiMask & _iVal) ? "1" : "0";
		if (0 == ++iPos % 4)
		{
			strResult += " ";
		}
	}

	cout << "Binary respresentation of " << right << setw(11) << _iVal << ": "
		 << strResult << endl;
}

/////////////////////////////////////////////////////////////////////////////
// 2. Напишите функцию, удаляющую последовательно дублирующиеся символы
//    в строке: ("AAA BBB ААА" -> "A B А")
void RemoveDupsTest()
{
	cout << "* RemoveDups() Test" << endl;

	const char* szTestArray[] =
	{
		"AAA BBB AAA",
		"1111111111",
		"1 1 2 2 3 3   4 4   ",
		"0",
		""
	};

	for (int i = 0; i < countof(szTestArray); ++i)
	{
  		char* psz = strdup(szTestArray[i]);
 	 	if (!psz)
 	 	{
 	 		cout << "Error: strdup failed (err: " << errno << ")" << endl;
 		}
 		else
 		{
			RemoveDups(psz);
 		}
	}

	cout << endl;
}

/////////////////////////////////////////////////////////////////////////////
void RemoveDups(char* _pszStr)
{
	if (!_pszStr)
	{
		cout << "Error: invalid RemoveDups() parameter: _pszStr == NULL" << endl;
		return;
	}

	cout << "Src string: \'" << _pszStr << "\'" << endl;

	for (char *pSrc = _pszStr, *pDest = _pszStr; ; ++pSrc)
	{
		if (*pDest != *pSrc)
		{
			pDest++;
			*pDest = *pSrc;
		}

		if ('\0' == *pSrc)
		{
			break;
		}
	}

	cout << "RemoveDups() = \'" << _pszStr << "\'" << endl;
}

/////////////////////////////////////////////////////////////////////////////
// 3. Реализуйте функции сериализации и десериализации двусвязного списка
//    в бинарном формате в файл.
//    struct ListNode {
//        ListNode *      prev;
//        ListNode *      next;
//        ListNode *      rand; // указатель на произвольный элемент данного списка либо NULL
//        std::string     data;
//    };
//
//    class List {
//    public:
//        void Serialize   (FILE * file);  // сохранение в файл (файл открыт с помощью fopen(path, "wb"))
//        void Deserialize (FILE * file);  // загрузка из файла (файл открыт с помощью fopen(path, "rb"))
//
//    private:
//        ListNode *      head;
//        ListNode *      tail;
//        int       count;
//    };
//
//    Примечание: сериализация подразумевает сохранение и восстановление полной
//    структуры списка, включая взаимное соотношение его элементов между собой.
void SerializeTest()
{
	cout << "* SerializeTest" << endl;

	List list;
	ListNode* pNode1 = list.AddNode("String String");
	ListNode* pNode2 = list.AddNode("0123456789");
	ListNode* pNode3 = list.AddNode("");
	assert(pNode1 && pNode2 && pNode3);

	// set rand links
	List::SetRand(pNode1, pNode3);
	List::SetRand(pNode2, pNode3);
	List::SetRand(pNode3, pNode3);

	list.DumpList();

	const char* szFile = "serialized_list";
	int iRet = list.Serialize(szFile);
	if (0 == iRet)
	{
		list.Deserialize(szFile);
		list.DumpList();
	}

	cout << endl;
}
