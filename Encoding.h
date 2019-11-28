#pragma once
#include <cstdint>
#include <string>
#include <string_view>
#include <stdexcept>

namespace Encoding
{
    enum class EncodingKind
    {
		Error=0, ASCII, Utf8, Utf16, Utf32
	};


    struct ASCII
    {
        using CharType = char;
        using ConstCharType = const char;
		using CharPtrType = char*;
		using ConstCharPtrType = const char*;

        static size_t Validate(std::string_view source)
        {
            return static_cast<uint32_t>(source.at(0)) <= 0x07F ? 1 : 0;
        }

        static size_t Decode(std::string_view source,uint32_t* pCodePoint)
        {
            uint32_t codePoint = static_cast<uint8_t>(source.at(0));
            if(codePoint<=0x7F)
            {
                *pCodePoint = codePoint;
                return 1;
            }else { throw std::invalid_argument("source");}
            
        }

        static size_t Encode(uint32_t codePoint, std::string_view dest)
        {
            
			if (codePoint <= 0x7F) {
				const_cast<char&>(dest.at(0)) = (0xFF & codePoint);
				return 1;
			}else { throw std::invalid_argument("codePoint"); }
			
        }
    };


    struct Utf8
    {
        using CharType = char;
        using ConstCharType = const char;
		using CharPtrType = char*;
		using ConstCharPtrType = const char*;

    private: 
        static unsigned char GetRange(unsigned char c)
        {
            static constexpr unsigned char type[] =
			{
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,
				0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,
				0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
				0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
				8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
				10,3,3,3,3,3,3,3,3,3,3,3,3,4,3,3, 11,6,6,6,5,8,8,8,8,8,8,8,8,8,8,8,
			};
			return type[c];
        }
    
    public:
        static size_t Validate(std::string_view source)
        {           
			int pos = 0;
			char c = source.at(pos++);
			size_t result = 1;
			bool checkFlag = true;

			auto Copy = [&]() { c = source.at(pos++); ++result; };
			auto Trans = [&](int mask) { checkFlag &= ((GetRange(static_cast<unsigned char>(c))& mask) != 0); };
			auto Tail = [&]() { Copy(); Trans(0x70); };

			if (!(c & 0x80)) { return 1; }

			unsigned char type = GetRange(static_cast<unsigned char>(c));

			switch (type)
			{
				case 2: Tail(); if (checkFlag)return result; break;
				case 3: Tail(); Tail(); if (checkFlag)return result; break;
				case 4: Copy(); Trans(0x50); Tail(); if (checkFlag)return result; break;
				case 5: Copy(); Trans(0x10); Tail(); Tail(); if (checkFlag)return result; break;
				case 6: Tail(); Tail(); Tail(); if (checkFlag)return result; break;
				case 10: Copy(); Trans(0x20); Tail(); if (checkFlag)return result; break;
				case 11: Copy(); Trans(0x60); Tail(); Tail(); if (checkFlag)return result; break;
				default: return 0;
			}			
        }

        static size_t Decode(std::string_view source, uint32_t* pCodePoint)
        {         
			int pos = 0;
			char c = source.at(pos++);
			size_t result = 1;
			bool checkFlag = true;

			auto Copy = [&]()
				{ c = source.at(pos++); ++result; *pCodePoint = (*pCodePoint << 6) | (static_cast<unsigned char>(c) & 0x3Fu); };
			auto Trans = [&](int mask) { checkFlag &= ((GetRange(static_cast<unsigned char>(c))& mask) != 0); };
			auto Tail = [&]() { Copy(); Trans(0x70); };

			if (!(c & 0x80)) {
				*pCodePoint = static_cast<unsigned char>(c);
				return 1;
			}
			unsigned char type = GetRange(static_cast<unsigned char>(c));

			if (type >= 32) { *pCodePoint = 0; }
			else { *pCodePoint = (0xFFu >> type)& static_cast<unsigned char>(c); }

			switch (type)
			{
				case 2: Tail(); if (checkFlag)return result; break;
				case 3: Tail(); Tail(); if (checkFlag)return result; break;
				case 4: Copy(); Trans(0x50); Tail(); if (checkFlag)return result; break;
				case 5: Copy(); Trans(0x10); Tail(); Tail(); if (checkFlag)return result; break;
				case 6: Tail(); Tail(); Tail(); if (checkFlag)return result; break;
				case 10: Copy(); Trans(0x20); Tail(); if (checkFlag)return result; break;
				case 11: Copy(); Trans(0x60); Tail(); Tail(); if (checkFlag)return result; break;
				default: throw std::invalid_argument("source");
			}
	
        }

		static size_t Encode(uint32_t codePoint, std::string_view dest)
		{		
			if (codePoint <= 0x7F) {
				const_cast<char&>(dest.at(0)) = (static_cast<char>(codePoint & 0xFF));
				return 1;
			} else if (codePoint <= 0x7FF) {
				const_cast<char&>(dest.at(0)) = static_cast<char>(0xC0 | ((codePoint >> 6) & 0xFF));
				const_cast<char&>(dest.at(1)) = static_cast<char>(0x80 | ((codePoint & 0x3F)));
				return 2;
			} else if (codePoint <= 0xFFFF) {
				const_cast<char&>(dest.at(0)) = static_cast<char>(0xE0 | ((codePoint >> 12) & 0xFF));
				const_cast<char&>(dest.at(1)) = static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F));
				const_cast<char&>(dest.at(2)) = static_cast<char>(0x80 | (codePoint & 0x3F));
				return 3;
			} else if (codePoint <= 0x10FFFF) {
				const_cast<char&>(dest.at(0)) = static_cast<char>(0xF0 | ((codePoint >> 18) & 0xFF));
				const_cast<char&>(dest.at(1)) = static_cast<char>(0x80 | ((codePoint >> 12) & 0x3F));
				const_cast<char&>(dest.at(2)) = static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F));
				const_cast<char&>(dest.at(3)) = static_cast<char>(0x80 | (codePoint & 0x3F));
				return 4;
			} else { 
                throw std::invalid_argument("codePoint");
            }

		}       
    };


    struct Utf16
    {
        using CharType = char16_t;
        using ConstCharType = const char16_t;
		using CharPtrType = char16_t*;
		using ConstCharPtrType = const char16_t*;
    
    	static size_t Validate(std::u16string_view source)
		{			
			int pos = 0;
			char16_t c = source.at(pos++);
			if (c < 0xD800 || c > 0xDFFF) { 
                return 1; 
            } else if (c <= 0xDBFF) {
				c = source.at(pos++);
				if (c >= 0xDC00 && c <= 0xDFFF) { return 2; }
			}else{
				return 0;
            }		
		}

		static size_t Decode(std::u16string_view source, uint32_t* pCodePoint)
		{
			int pos = 0;
			char16_t c = source.at(pos++);
			if (c < 0xD800 || c > 0xDFFF) {
				*pCodePoint = static_cast<unsigned>(c);
				return 1;
			} else if (c <= 0xDBFF) {
				*pCodePoint = (static_cast<unsigned>(c) & 0x3FF) << 10;
				c = source.at(pos++);
				*pCodePoint |= (static_cast<unsigned>(c) & 0x3FF);
				*pCodePoint += 0x10000;
				if (c >= 0xDC00 && c <= 0xDFFF)return 2;
			} else {
				throw std::invalid_argument("source");
            }			
        }

        static size_t Encode(uint32_t codePoint, std::u16string_view dest)
		{		
			if (codePoint <= 0xFFFF) {
				if ((codePoint < 0xD800 || codePoint > 0xDFFF)) {
					const_cast<char16_t&>(dest.at(0)) = static_cast<char16_t>(codePoint);
					return 1;
				}
			} else if (codePoint <= 0x10FFFF) {
				uint32_t v = codePoint - 0x10000;
				const_cast<char16_t&>(dest.at(0)) = (v >> 10) | 0xD800;
				const_cast<char16_t&>(dest.at(1)) = (v & 0x3FF) | 0xDC00;
				return 2;
			}else{ throw std::invalid_argument("codePoint"); }		
		}
    };


    struct Utf32
    {
        using CharType = char32_t;
        using ConstCharType = const char32_t;
		using CharPtrType = char32_t*;
		using ConstCharPtrType = const char32_t*;

        static size_t Validate(std::u32string_view source)
        {
		    return source.at(0) <= 0x10FFFF ? 1 : 0;
		}

		static size_t Decode(std::u32string_view source, uint32_t* pCodePoint)
		{			
			char32_t c = source.at(0);
            if(c<=0x10FFFF){
			    *pCodePoint = static_cast<uint32_t>(c);
                return 1;
            }else{
                throw std::invalid_argument("source");
            }
            
		}

        static size_t Encode( uint32_t codePoint, std::u32string_view dest)
		{
            if(codePoint<=0x10FFFF){
			    const_cast<char32_t&>(dest.at(0)) = codePoint;
                return 1;
            }else { throw std::invalid_argument("codePoint"); }
		}        

    };


    EncodingKind DetectBom(const char bytes[4])
    {
        uint32_t bom = static_cast<unsigned>(bytes[0] | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24));
        if(bom == 0x0000FEFF ){ return EncodingKind::Utf32;}
        else if((bom & 0xFFFF) == 0xFEFF) { return EncodingKind::Utf16;}
        else if((bom & 0xFFFFFF) == 0xBFBBEF){ return EncodingKind::Utf8;}
        else{ return EncodingKind::Error;}
    }


    struct ConvertInfo
    {
        size_t CodePointsNum;
		size_t SourceCodeUnitsNum;
		size_t DestCodeUnitsNum;
    };


    struct Convert 
    {
        template<typename Coding>
		static size_t GetCodePointsNum(
			std::basic_string_view<typename Coding::CharType> source
		){
			size_t codePointsNum = 0;
            size_t sumDecodedCodeUnitsNum = 0;

            uint32_t codePoint;
            const size_t sourceCodeUnitsNum = source.size();       
            while (sumDecodedCodeUnitsNum < sourceCodeUnitsNum )
            {
                const size_t decodedCodeUnitsNum = Coding::Decode(source, &codePoint);
                source.remove_prefix(decodedCodeUnitsNum);
                sumDecodedCodeUnitsNum += decodedCodeUnitsNum;
				++codePointsNum;
            }
            return codePointsNum;
		}

        template<typename Coding>
        static size_t GetCodeUnitsNum(const uint32_t* pCodePoints, size_t codePointsNum=1)
        {
            typename Coding::CharType buffer[4] = { 0 };
			size_t sumCodeUnitsNum = 0;
			for (size_t i = 0; i < codePointsNum; ++i) {
				const size_t codeUnitsNum = Coding::Encode(pCodePoints[i], buffer);
				sumCodeUnitsNum += codeUnitsNum;
				memset(buffer, 0, 4 * sizeof(typename Coding::CharType));
			}
			return sumCodeUnitsNum;
        }

        template<typename SourceCoding, typename DestCoding>
			static ConvertInfo TransCode(
				std::basic_string_view<typename SourceCoding::CharType> source,
				std::basic_string_view<typename DestCoding::CharType> dest
			)
		{				
			uint32_t codePoint;
			const uint32_t decodedSourceCodeUnitsNum = SourceCoding::Decode(source, &codePoint);
			const uint32_t encodedDestCodeUnitsNum = DestCoding::Encode(codePoint, dest);
            return { 1,decodedSourceCodeUnitsNum,encodedDestCodeUnitsNum };           	
		}

        template<typename SourceCoding, typename DestCoding>
		static ConvertInfo TransCodes(
			std::basic_string_view<typename SourceCoding::CharType> source,
			std::basic_string_view<typename DestCoding::CharType> dest
		){
            size_t sourceCodeUnitsNum = source.size();
			size_t codePointsNum = 0;
			size_t sumDecodedSourceCodeUnitsNum = 0;
			size_t sumEncodedDestCodeUnitsNum = 0;

			uint32_t codePoint;
 			while (sumDecodedSourceCodeUnitsNum < sourceCodeUnitsNum)
			{
				const size_t decodedSourceCodeUnitsNum = SourceCoding::Decode(source, &codePoint);
				source.remove_prefix(decodedSourceCodeUnitsNum);
				sumDecodedSourceCodeUnitsNum += decodedSourceCodeUnitsNum;
				const size_t encodedDestCodingUnitsNum = DestCoding::Encode(codePoint,dest);
				dest.remove_prefix(encodedDestCodingUnitsNum);
				sumEncodedDestCodeUnitsNum += encodedDestCodingUnitsNum;
				++codePointsNum;
			} 
            return { codePointsNum,sumDecodedSourceCodeUnitsNum,sumEncodedDestCodeUnitsNum };
        }
    };
}
