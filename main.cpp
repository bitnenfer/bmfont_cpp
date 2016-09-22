#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/****************************************************/
/****************************************************/
/****************************************************/
/****************************************************/

/**
 * BMFont Binary Format:
 *	http://www.angelcode.com/products/bmfont/doc/file_format.html#bin
 */

#define BMFONT_BLOCK_TYPE_INFO 1
#define BMFONT_BLOCK_TYPE_COMMON 2
#define BMFONT_BLOCK_TYPE_PAGES 3
#define BMFONT_BLOCK_TYPE_CHARS 4
#define BMFONT_BLOCK_TYPE_KERNINGS 5
#define BMFONT_MAX_PAGES 2
#define BMFONT_MAX_CHARS 255
#define BMFONT_MAX_KERNINGPAIRS 500

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
	bmfont_stream(uint8_t* pBuffer, size_t size) :
		pos(0),
		size(size),
		pByteData(pBuffer)
	{}
	// Unsigned
	uint8_t currentU8()
	{
		return (uint8_t)pByteData[pos];
	}
	uint8_t getU8()
	{
		uint8_t value = currentU8();
		pos += 1;
		return value;
	}
	uint16_t currentU16()
	{
		return (uint16_t)((pByteData[pos + 1] << 8) | pByteData[pos]);
	}
	uint16_t getU16()
	{
		uint16_t value = currentU16();
		pos += 2;
		return value;
	}
	uint32_t currentU32()
	{
		return (uint32_t)((pByteData[pos + 3] << 24) | (pByteData[pos + 2] << 16) | (pByteData[pos + 1] << 8) | pByteData[pos]);
	}
	uint32_t getU32()
	{
		uint32_t value = currentU32();
		pos += 4;
		return value;
	}
	// Signed
	int8_t currentS8()
	{
		return (int8_t)pByteData[pos];
	}
	int8_t getS8()
	{
		int8_t value = currentU8();
		pos += 1;
		return value;
	}
	int16_t currentS16()
	{
		return (int16_t)((pByteData[pos + 1] << 8) | pByteData[pos + 0]);
	}
	int16_t getS16()
	{
		int16_t value = currentU16();
		pos += 2;
		return value;
	}
	int32_t currentS32()
	{
		return (int32_t)((pByteData[pos + 3] << 24) | (pByteData[pos + 2] << 16) | (pByteData[pos + 1] << 8) | pByteData[pos]);
	}
	int32_t getS32()
	{
		int32_t value = currentU32();
		pos += 4;
		return value;
	}
	uint8_t* getPtr()
	{
		return &pByteData[pos];
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
	uint8_t* pByteData;
};
struct bmfont_page
{
	char name[32];
	uint8_t length;
};
struct bmfont
{
	bmfont_info info;
	bmfont_common common;
	bmfont_page pages[BMFONT_MAX_PAGES];
	bmfont_char chars[BMFONT_MAX_CHARS];
};
struct bmfont_with_kp
{
	bmfont_info info;
	bmfont_common common;
	bmfont_page pages[BMFONT_MAX_PAGES];
	bmfont_char chars[BMFONT_MAX_CHARS];
	bmfont_kerningpairs kerningPairs[BMFONT_MAX_KERNINGPAIRS];
};
bool GetBMFontData(const char* pBinary, size_t fileSize, bmfont* pBMFont)
{
	if (!((pBinary[0] == 'B' && pBinary[1] == 'M' &&
		pBinary[2] == 'F' && pBinary[3] == 3)))
		return false;

	bmfont_stream stream((uint8_t*)pBinary, fileSize);
	stream.offsetBy(4);
	while (!stream.isEOF())
	{
		uint8_t blockID = stream.getU8();
		int32_t blockSize = stream.getU32();
		switch (blockID)
		{
			case BMFONT_BLOCK_TYPE_INFO:
			{
				pBMFont->info.fontSize = stream.getS16();
				pBMFont->info.bitField = stream.getU8();
				pBMFont->info.charSet = stream.getU8();
				pBMFont->info.stretchH = stream.getU16();
				pBMFont->info.aa = stream.getU8();
				pBMFont->info.paddingUp = stream.getU8();
				pBMFont->info.paddingRight = stream.getU8();
				pBMFont->info.paddingDown = stream.getU8();
				pBMFont->info.paddingLeft = stream.getU8();
				pBMFont->info.spacingHoriz = stream.getU8();
				pBMFont->info.spacingVert = stream.getU8();
				pBMFont->info.outline = stream.getU8();
				const char* name = (const char*)stream.getPtr();
				stream.offsetBy(strlen(name) + 1);
				break;
			}
			case BMFONT_BLOCK_TYPE_COMMON:
			{
				pBMFont->common.lineHeight = stream.getU16();
				pBMFont->common.base = stream.getU16();
				pBMFont->common.scaleW = stream.getU16();
				pBMFont->common.scaleH = stream.getU16();
				pBMFont->common.pages = stream.getU16();
				pBMFont->common.bitField = stream.getU8();
				pBMFont->common.alphaChnl = stream.getU8();
				pBMFont->common.redChnl = stream.getU8();
				pBMFont->common.greenChnl = stream.getU8();
				pBMFont->common.blueChnl = stream.getU8();
				break;
			}
			case BMFONT_BLOCK_TYPE_PAGES:
			{
				uint16_t pageCount = pBMFont->common.pages;
				for (uint16_t index = 0; index < pageCount; ++index)
				{
					const char* pageName = (const char*)stream.getPtr();
					uint8_t len = 0;
					while (pageName[len] != 0)
					{
						pBMFont->pages[index].name[len] = pageName[len++];
					}
					pBMFont->pages[index].length = len;
					stream.offsetBy(len + 1);
				}
				break;
			}
			case BMFONT_BLOCK_TYPE_CHARS:
			{
				int32_t charCount = blockSize / 20;
				if (charCount > BMFONT_MAX_CHARS)
					return false;

				for (int32_t index = 0; index < charCount; ++index)
				{
					pBMFont->chars[index].id = stream.getU32();
					pBMFont->chars[index].x = stream.getU16();
					pBMFont->chars[index].y = stream.getU16();
					pBMFont->chars[index].width = stream.getU16();
					pBMFont->chars[index].height = stream.getU16();
					pBMFont->chars[index].xoffset = stream.getS16();
					pBMFont->chars[index].yoffset = stream.getS16();
					pBMFont->chars[index].xadvance = stream.getS16();
					pBMFont->chars[index].page = stream.getU8();
					pBMFont->chars[index].chnl = stream.getU8();
				}
				return true;
			}
			default:
				return false;
		}
	}
	return true;
}
bool GetBMFontDataWithKerningPairs(const char* pBinary, size_t fileSize, bmfont_with_kp* pBMFont)
{
	if (!((pBinary[0] == 'B' && pBinary[1] == 'M' &&
		pBinary[2] == 'F' && pBinary[3] == 3)))
		return false;

	bmfont_stream stream((uint8_t*)pBinary, fileSize);
	stream.offsetBy(4);
	while (!stream.isEOF())
	{
		uint8_t blockID = stream.getU8();
		int32_t blockSize = stream.getU32();
		switch (blockID)
		{
			case BMFONT_BLOCK_TYPE_INFO:
			{
				pBMFont->info.fontSize = stream.getS16();
				pBMFont->info.bitField = stream.getU8();
				pBMFont->info.charSet = stream.getU8();
				pBMFont->info.stretchH = stream.getU16();
				pBMFont->info.aa = stream.getU8();
				pBMFont->info.paddingUp = stream.getU8();
				pBMFont->info.paddingRight = stream.getU8();
				pBMFont->info.paddingDown = stream.getU8();
				pBMFont->info.paddingLeft = stream.getU8();
				pBMFont->info.spacingHoriz = stream.getU8();
				pBMFont->info.spacingVert = stream.getU8();
				pBMFont->info.outline = stream.getU8();
				const char* name = (const char*)stream.getPtr();
				stream.offsetBy(strlen(name) + 1);
				break;
			}
			case BMFONT_BLOCK_TYPE_COMMON:
			{
				pBMFont->common.lineHeight = stream.getU16();
				pBMFont->common.base = stream.getU16();
				pBMFont->common.scaleW = stream.getU16();
				pBMFont->common.scaleH = stream.getU16();
				pBMFont->common.pages = stream.getU16();
				pBMFont->common.bitField = stream.getU8();
				pBMFont->common.alphaChnl = stream.getU8();
				pBMFont->common.redChnl = stream.getU8();
				pBMFont->common.greenChnl = stream.getU8();
				pBMFont->common.blueChnl = stream.getU8();
				break;
			}
			case BMFONT_BLOCK_TYPE_PAGES:
			{
				uint16_t pageCount = pBMFont->common.pages;
				for (uint16_t index = 0; index < pageCount; ++index)
				{
					const char* pageName = (const char*)stream.getPtr();
					uint8_t len = 0;
					while (pageName[len] != 0)
					{
						pBMFont->pages[index].name[len] = pageName[len++];
					}
					pBMFont->pages[index].length = len;
					stream.offsetBy(len + 1);
				}
				break;
			}
			case BMFONT_BLOCK_TYPE_CHARS:
			{
				int32_t charCount = blockSize / 20;
				if (charCount > BMFONT_MAX_CHARS)
					return false;

				for (int32_t index = 0; index < charCount; ++index)
				{
					pBMFont->chars[index].id = stream.getU32();
					pBMFont->chars[index].x = stream.getU16();
					pBMFont->chars[index].y = stream.getU16();
					pBMFont->chars[index].width = stream.getU16();
					pBMFont->chars[index].height = stream.getU16();
					pBMFont->chars[index].xoffset = stream.getS16();
					pBMFont->chars[index].yoffset = stream.getS16();
					pBMFont->chars[index].xadvance = stream.getS16();
					pBMFont->chars[index].page = stream.getU8();
					pBMFont->chars[index].chnl = stream.getU8();
				}
				break;
			}
			case BMFONT_BLOCK_TYPE_KERNINGS:
			{
				int32_t kerningPairsCount = blockSize / 10;
				if (kerningPairsCount > BMFONT_MAX_KERNINGPAIRS)
					return false;
				for (int32_t index = 0; index < kerningPairsCount; ++index)
				{
					pBMFont->kerningPairs[index].first = stream.getU32();
					pBMFont->kerningPairs[index].second = stream.getU32();
					pBMFont->kerningPairs[index].amount = stream.getS16();
				}
				return true;
			}
			default:
				return false;
		}
	}
	return true;
}

/****************************************************/
/****************************************************/
/****************************************************/
/****************************************************/
char* GetFileData(const char* pPath, size_t* pSize);
int main()
{
	size_t fileSize = 0;
	char* data = GetFileData("comicsans.fnt", &fileSize);
	bmfont comicsans = {};
	bmfont_with_kp yugothic = {};

	if (GetBMFontData(data, fileSize, &comicsans))
	{
		printf("Loaded BMFont data correctly. ComicSans\n");
	}
	if (data != NULL)
		free(data);
	
	data = NULL;
	data = GetFileData("yugothic.fnt", &fileSize);
	if (GetBMFontDataWithKerningPairs(data, fileSize, &yugothic))
	{
		printf("Loaded BMFont data correctly. Yu Gothic\n");
	}
	if (data != NULL)
		free(data);		

	return 0;
}

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