//
// Created by zwg on 19-6-6.
//

#include "serialization.hpp"

namespace sz {
    typedef uint64_t uint64_array_4[4];
    //将len序列化到buffer中，返回消耗buffer的字节数
    size_t serializeLength(size_t len, void* buffer) {
        uint64_t val = len;
        size_t byte = 0;
        do {
            uint8_t b = uint8_t(val) & 0x7f;
            val >>= 7;
            b |= ((val > 0) << 7);
            ((uint8_t*)buffer)[byte] = b;
            byte ++;
        }while(val);
        return byte;
    }

    //解压buffer中len值，返回(len, bufferOffset)
    //bufferOffset返回为0表示解析出错(外部传入的bufferLen和buffer中的内容不相符)
    pair<size_t, size_t> deserializeLength(const void* buffer, size_t bufferLen) {
        auto p = (uint8_t*) buffer;
        size_t offset = 0;
        size_t len = 0;
        size_t temp;
        while(p[offset] > 0x7f) {
            temp = p[offset]&0x7f;
            len += temp<<(7*offset);
            offset ++;
            if(offset >= bufferLen) return make_pair(0, 0);
        }
        temp = p[offset];
        len += temp<<(7*offset);
        offset ++;
        return make_pair(len, offset);
    }

    namespace InnerSerializeFunc {
        template <class T>
        class DefaultSerializeFuncClass {
        public:
            static pair<bool, size_t> serialize(void* dst, const void* src, size_t bufferSize) {
                if(bufferSize < sizeof(T)) return make_pair(false, 0);
                memcpy(dst, src, sizeof(T));
                return make_pair(true, sizeof(T));
            }
            static DataDeserializeFunc deserialize;
        };
        template <class T>
        DataDeserializeFunc DefaultSerializeFuncClass<T>::deserialize = DefaultSerializeFuncClass<T>::serialize;

        class StringSerializeFuncClass {
        public:
            static pair<bool, size_t> serialize(void* dst, const void* src, size_t bufferSize) {
                auto str_len = ((string*)src)->length();
                if(str_len + 8 > bufferSize) return make_pair(false, 0);
                auto skip = serializeLength(str_len, dst);
                memcpy(((char*)dst)+skip, ((string*)src)->data(), str_len);
                return make_pair(true, skip + str_len);
            }
            static pair<bool, size_t> deserialize(void* pDst, const void* pBuffer, size_t bufferLen){
                auto ret = deserializeLength(pBuffer, bufferLen);
                if(ret.second == 0)
                    return make_pair(false, 0);
                auto usedSize = ret.first + ret.second;
                if(usedSize > bufferLen)
                    return make_pair(false, 0);
                auto pStr = ((const char*)pBuffer) + ret.second;
                ((string*) pDst)->clear();
                ((string*) pDst)->append(pStr, pStr + ret.first);
                return make_pair(true, usedSize);
            }
        };
    }

    Reflect Reflect::instance;
    Reflect::Reflect() {
        INNER_DEFAULT_REGISTER(bool);
        INNER_DEFAULT_REGISTER(char);
        INNER_DEFAULT_REGISTER(unsigned char);
        INNER_DEFAULT_REGISTER(short);
        INNER_DEFAULT_REGISTER(unsigned short);
        INNER_DEFAULT_REGISTER(int);
        INNER_DEFAULT_REGISTER(unsigned int);
        INNER_DEFAULT_REGISTER(long);
        INNER_DEFAULT_REGISTER(unsigned long);
        INNER_DEFAULT_REGISTER(long long);
        INNER_DEFAULT_REGISTER(unsigned long long);
        INNER_DEFAULT_REGISTER(float);
        INNER_DEFAULT_REGISTER(double);
        INNER_DEFAULT_REGISTER(long double);

        REGISTER_CALLABLE_CLASS(string,
                                InnerSerializeFunc::StringSerializeFuncClass::serialize,
                                InnerSerializeFunc::StringSerializeFuncClass::deserialize);

    }
}