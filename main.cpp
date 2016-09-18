#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/**
 * BMFont Binary Format:
 *	http://www.angelcode.com/products/bmfont/doc/file_format.html#bin
 */

#define BMFONT_BLOCK_TYPE_INFO 1
#define BMFONT_BLOCK_TYPE_COMMON 2
#define BMFONT_BLOCK_TYPE_PAGES 3
#define BMFONT_BLOCK_TYPE_CHARS 4
#define BMFONT_BLOCK_TYPE_KERNINGS 5
#define BMFONT_MAX_PAGES 5
#define BMFONT_MAX_CHARS 1024
#define BMFONT_MAX_KERNINGPAIRS 1024

struct bmfont_info
{
	int16_t fontSize;
	uint8_t bitField;
	uint8_t charSet;
	uint16_t stretchH;
	uint8_t aa;
	uint8_t paddingUp;
	uint8_t paddingRight;
	uint8_t paddingDown;
	uint8_t paddingLeft;
	uint8_t spacingHoriz;
	uint8_t spacingVert;
	uint8_t outline;
};
struct bmfont_common
{
	uint16_t lineHeight;
	uint16_t base;
	uint16_t scaleW;
	uint16_t scaleH;
	uint16_t pages;
	uint8_t bitField;
	uint8_t alphaChnl;
	uint8_t redChnl;
	uint8_t greenChnl;
	uint8_t blueChnl;
};
struct bmfont_page
{
	const char* pPageName;
	size_t length;
}; 
struct bmfont_char
{
	uint32_t id;
	uint16_t x;
	uint16_t y;
	uint16_t width;
	uint16_t height;
	int16_t xoffset;
	int16_t yoffset;
	int16_t xadvance;
	uint8_t page;
	uint8_t chnl;
};
struct bmfont_kerningpairs
{
	uint32_t first;
	uint32_t second;
	int16_t amount;
};
struct bmfont_file
{
	bmfont_info* pInfo;
	const char* pFontName;
	bmfont_common* pCommon;
	const char* pages[BMFONT_MAX_PAGES];
	bmfont_char* chars[BMFONT_MAX_CHARS];
	bmfont_kerningpairs* kerningPairs[BMFONT_MAX_KERNINGPAIRS];
};
struct bmfont_stream
{
	bmfont_stream(const char* pBuffer, size_t size) :
		pos(0),
		size(size),
		pByteData(pBuffer)
	{}
	template<class T>
	T current()
	{
		return (T)*(&pByteData[pos]);
	}
	template<class T>
	T get()
	{
		T value = current<T>();
		pos += sizeof(T);
		return value;
	}
	template<class T>
	T* currentPtr()
	{
		return (T*)(&pByteData[pos]);
	}
	template<class T>
	T* getPtr()
	{
		T* ptr = currentPtr<T>();
		pos += sizeof(T);
		return ptr;
	}
	void offsetBy(size_t byteCount)
	{
		pos += byteCount;
	}
	bool isEOF()
	{
		return pos >= size;
	}
private:
	size_t pos;
	size_t size;
	const char* pByteData;
};

char* GetFileData(const char* pPath, size_t* pSize)
{
	char* pData = NULL;
	FILE* pFile = NULL;
#if defined(_MSC_VER)
	fopen_s(&pFile, pPath, "rb");
#else
	pFile = fopen(pPath, "rb");
#endif
	if (pFile == NULL)
	{
		*pSize = 0;
		return NULL;
	}
	fseek(pFile, 0, SEEK_END);
	*pSize = ftell(pFile);
	fseek(pFile, 0, SEEK_SET);
	pData = (char*)malloc(*pSize);
	fread(pData, *pSize, 1, pFile);
	fclose(pFile);
	return pData;
}
bool GetBMFontFile(const char* pBinary, size_t fileSize, bmfont_file* pBMFont)
{
	if (!((pBinary[0] == 'B' && pBinary[1] == 'M' &&
		pBinary[2] == 'F' && pBinary[3] == 3)))
		return false;

	bmfont_stream stream(pBinary, fileSize);
	stream.offsetBy(4);
	while (!stream.isEOF())
	{
		uint8_t blockID = stream.get<uint8_t>();
		int32_t blockSize = *stream.getPtr<int32_t>();
		switch (blockID)
		{
		case BMFONT_BLOCK_TYPE_INFO:
			pBMFont->pInfo = stream.getPtr<bmfont_info>();
			pBMFont->pFontName = stream.getPtr<const char>();
			stream.offsetBy(strlen(pBMFont->pFontName));
			break;
		case BMFONT_BLOCK_TYPE_COMMON:
			pBMFont->pCommon = stream.currentPtr<bmfont_common>();
			stream.offsetBy(blockSize);
			break;
		case BMFONT_BLOCK_TYPE_PAGES:
		{
			if (pBMFont->pCommon == NULL)
				break;
			uint16_t pageCount = pBMFont->pCommon->pages;
			for (uint16_t index = 0; index < pageCount; ++index)
			{
				const char* pageName = stream.getPtr<const char>();
				stream.offsetBy(strlen(pageName));
				pBMFont->pages[index] = pageName;
			}
			break;
		}
		case BMFONT_BLOCK_TYPE_CHARS:
		{
			int32_t charCount = blockSize / sizeof(bmfont_char);
			for (int32_t index = 0; index < charCount; ++index)
			{
				bmfont_char* pBMChar = stream.getPtr<bmfont_char>();
				if (pBMChar->id > BMFONT_MAX_CHARS || pBMChar->id < 0)
				{
					return false;
				}
				pBMFont->chars[pBMChar->id] = pBMChar;
			}
			break;
		}
		case BMFONT_BLOCK_TYPE_KERNINGS:
		{
			int32_t kerningPairsCount = blockSize / 10;
			if (kerningPairsCount > BMFONT_MAX_KERNINGPAIRS)
			{
				return false;
			}
			for (int32_t index = 0; index < kerningPairsCount; ++index)
			{
				pBMFont->kerningPairs[index] = stream.currentPtr<bmfont_kerningpairs>();
				stream.offsetBy(10);
			}
			break;
		}
		default:
			return false;
		}
	}
	return true;
}

int main()
{
	size_t fileSize = 0;
	char* data = GetFileData("comicsans.fnt", &fileSize);
	bmfont_file bmfile = {};

	if (GetBMFontFile(data, fileSize, &bmfile))
	{
		printf("Loaded BMFont data correctly.");
	}
	if (data != NULL)
		free(data);
	return 0;
}