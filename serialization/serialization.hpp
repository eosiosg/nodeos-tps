//
// Created by zwg on 19-6-6.
//

#ifndef NODEOS_TPS_SERIALIZATION_H
#define NODEOS_TPS_SERIALIZATION_H

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <cstring>
#include <typeinfo>
using namespace std;

#define _REGISTER(T, f1, f2) sz::Reflect::getInstance().addNode(typeid(T).hash_code(), #T, sizeof(T), f1, f2)

#define REGISTER_CALLABLE_CLASS(ClassName, f1, f2) _REGISTER(ClassName, f1, f2)

#define INNER_DEFAULT_REGISTER(T) REGISTER_CALLABLE_CLASS(T, \
        sz::InnerSerializeFunc::DefaultSerializeFuncClass<T>::serialize, \
        sz::InnerSerializeFunc::DefaultSerializeFuncClass<T>::deserialize)

#define __START_REGISTER_NO_CALLABLE_CLASS(ClassName) { \
    ClassName __register__no_callable_class_xwqdqdw; \
    vector<sz::SubNodeType> l; \
    sz::SubNodeType sn;

# define __CALCULATE_OFFSET_AND_ADD(obj, m) \
    sn.offset = (size_t)((char*)(&obj.m) - (char*)(&obj)); \
    sn.hashCode = typeid(obj.m).hash_code(); \
    l.push_back(sn)

#define __END_REGISTER_NO_CALLABLE_CLASS(ClassName) \
    sz::Reflect::getInstance().addNode(typeid(ClassName).hash_code(), #ClassName, sizeof(ClassName), l); \
    }

#define REGISTER_NO_CALLABLE_CLASS_1(ClassName, m) \
    __START_REGISTER_NO_CALLABLE_CLASS(ClassName) \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m); \
    __END_REGISTER_NO_CALLABLE_CLASS(ClassName)

#define REGISTER_NO_CALLABLE_CLASS_2(ClassName, m1, m2) \
    __START_REGISTER_NO_CALLABLE_CLASS(ClassName) \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m1); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m2); \
    __END_REGISTER_NO_CALLABLE_CLASS(ClassName)

#define REGISTER_NO_CALLABLE_CLASS_3(ClassName, m1, m2, m3) \
    __START_REGISTER_NO_CALLABLE_CLASS(ClassName) \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m1); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m2); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m3); \
    __END_REGISTER_NO_CALLABLE_CLASS(ClassName)

#define REGISTER_NO_CALLABLE_CLASS_4(ClassName, m1, m2, m3, m4) \
    __START_REGISTER_NO_CALLABLE_CLASS(ClassName) \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m1); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m2); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m3); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m4); \
    __END_REGISTER_NO_CALLABLE_CLASS(ClassName)

#define REGISTER_NO_CALLABLE_CLASS_5(ClassName, m1, m2, m3, m4, m5) \
    __START_REGISTER_NO_CALLABLE_CLASS(ClassName) \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m1); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m2); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m3); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m4); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m5); \
    __END_REGISTER_NO_CALLABLE_CLASS(ClassName)

#define REGISTER_NO_CALLABLE_CLASS_6(ClassName, m1, m2, m3, m4, m5, m6) \
    __START_REGISTER_NO_CALLABLE_CLASS(ClassName) \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m1); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m2); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m3); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m4); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m5); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m6); \
    __END_REGISTER_NO_CALLABLE_CLASS(ClassName)

#define REGISTER_NO_CALLABLE_CLASS_7(ClassName, m1, m2, m3, m4, m5, m6, m7) \
    __START_REGISTER_NO_CALLABLE_CLASS(ClassName) \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m1); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m2); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m3); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m4); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m5); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m6); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m7); \
    __END_REGISTER_NO_CALLABLE_CLASS(ClassName)

#define REGISTER_NO_CALLABLE_CLASS_8(ClassName, m1, m2, m3, m4, m5, m6, m7, m8) \
    __START_REGISTER_NO_CALLABLE_CLASS(ClassName) \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m1); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m2); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m3); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m4); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m5); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m6); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m7); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m8); \
    __END_REGISTER_NO_CALLABLE_CLASS(ClassName)

#define REGISTER_NO_CALLABLE_CLASS_9(ClassName, m1, m2, m3, m4, m5, m6, m7, m8, m9) \
    __START_REGISTER_NO_CALLABLE_CLASS(ClassName) \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m1); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m2); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m3); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m4); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m5); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m6); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m7); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m8); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m9); \
    __END_REGISTER_NO_CALLABLE_CLASS(ClassName)

#define REGISTER_NO_CALLABLE_CLASS_10(ClassName, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10) \
    __START_REGISTER_NO_CALLABLE_CLASS(ClassName) \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m1); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m2); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m3); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m4); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m5); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m6); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m7); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m8); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m9); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m10); \
    __END_REGISTER_NO_CALLABLE_CLASS(ClassName)

#define REGISTER_NO_CALLABLE_CLASS_11(ClassName, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11) \
    __START_REGISTER_NO_CALLABLE_CLASS(ClassName) \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m1); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m2); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m3); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m4); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m5); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m6); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m7); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m8); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m9); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m10); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m11); \
    __END_REGISTER_NO_CALLABLE_CLASS(ClassName)

#define REGISTER_NO_CALLABLE_CLASS_12(ClassName, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12) \
    __START_REGISTER_NO_CALLABLE_CLASS(ClassName) \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m1); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m2); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m3); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m4); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m5); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m6); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m7); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m8); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m9); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m10); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m11); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m12); \
    __END_REGISTER_NO_CALLABLE_CLASS(ClassName)

#define REGISTER_NO_CALLABLE_CLASS_13(ClassName, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13) \
    __START_REGISTER_NO_CALLABLE_CLASS(ClassName) \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m1); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m2); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m3); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m4); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m5); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m6); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m7); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m8); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m9); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m10); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m11); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m12); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m13); \
    __END_REGISTER_NO_CALLABLE_CLASS(ClassName)

#define REGISTER_NO_CALLABLE_CLASS_14(ClassName, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14) \
    __START_REGISTER_NO_CALLABLE_CLASS(ClassName) \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m1); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m2); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m3); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m4); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m5); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m6); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m7); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m8); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m9); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m10); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m11); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m12); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m13); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m14); \
    __END_REGISTER_NO_CALLABLE_CLASS(ClassName)

#define REGISTER_NO_CALLABLE_CLASS_15(ClassName, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15) \
    __START_REGISTER_NO_CALLABLE_CLASS(ClassName) \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m1); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m2); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m3); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m4); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m5); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m6); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m7); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m8); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m9); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m10); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m11); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m12); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m13); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m14); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m15); \
    __END_REGISTER_NO_CALLABLE_CLASS(ClassName)

#define REGISTER_NO_CALLABLE_CLASS_16(ClassName, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16) \
    __START_REGISTER_NO_CALLABLE_CLASS(ClassName) \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m1); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m2); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m3); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m4); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m5); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m6); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m7); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m8); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m9); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m10); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m11); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m12); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m13); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m14); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m15); \
    __CALCULATE_OFFSET_AND_ADD(__register__no_callable_class_xwqdqdw, m16); \
    __END_REGISTER_NO_CALLABLE_CLASS(ClassName)

namespace sz {
    typedef pair<bool, size_t> (*DataSerializeFunc)(void* pDstBuffer, const void* pData, size_t bufferSize);
    typedef pair<bool, size_t> (*DataDeserializeFunc)(void* pDstData, const void* pBuffer, size_t bufferLength);
    //将len序列化到buffer中，返回消耗buffer的字节数
    size_t serializeLength(size_t len, void* buffer);
    pair<size_t, size_t> deserializeLength(const void* buffer, size_t bufferLen);

    enum NodeType {
        CALLABLE, //Stand for that this node has stored the pointer of GetSizeFunc and SerilizeFunc.
        NO_CALLABLE //Stand for that program need reverse call.
    };

    struct SerilizeFuncType {
        DataSerializeFunc serilizeData;
        DataDeserializeFunc deserializeData;
        SerilizeFuncType():serilizeData(nullptr), deserializeData(nullptr) {}
    };

    struct SubNodeType {
        size_t offset;
        size_t hashCode;
    };

    struct Node {
        NodeType type; // callable(means can directly call function) or no callable
        string name; // ClassName, such as int, char, string, vector<int> or any other user defined class.
        size_t bytes; // bytes of the object of class.
        SerilizeFuncType func; // used when type == CALLABLE
        vector<SubNodeType> list; // used when type == NO_CALLABLE
        Node() = delete;
        Node(const NodeType& t, const string& name, const size_t& bytes,
                const DataSerializeFunc& f1, const DataDeserializeFunc& f2)
            :type(t), name(name), bytes(bytes) {
            func.serilizeData = f1;
            func.deserializeData = f2;
        }
        Node(const NodeType& t, const string& name, const size_t& bytes, const vector<SubNodeType>& l)
            :type(t), name(name), bytes(bytes), list(l) {}
    };

    class Reflect {
        map<size_t, Node> reflectTable;
        static Reflect instance;
        Reflect();
        Reflect(const Reflect&) = delete;
        Reflect&operator==(const Reflect&) = delete;
    public:
        static Reflect& getInstance() {
            return instance;
        }
        void addNode(const size_t& hashCode, const string& name, const size_t& bytes,
                const DataSerializeFunc& f1, const DataDeserializeFunc& f2) {
            if(!reflectTable.insert(make_pair(hashCode, Node(CALLABLE, name, bytes, f1, f2))).second)
                cerr << "Warning: insert reflectTable failed of " << name << "." << endl;
        }
        void addNode(const size_t& hashCode, const string& name, const size_t& bytes, const vector<SubNodeType>& l) {
            if(!reflectTable.insert(make_pair(hashCode, Node(NO_CALLABLE, name, bytes, l))).second)
                cerr << "Warning: insert reflectTable failed of " << name << "." << endl;
        }
        pair<bool, const Node*> get(size_t hash) {
            auto iter = reflectTable.find(hash);
            if(iter == reflectTable.end()) {
                cerr << "Warning: type not register in table" << endl;
                return make_pair(false, nullptr);
            }
            return make_pair(true, &iter->second);
        }
    };

    struct Buffer {
        void* buffer;
        size_t curPos;
        size_t maxSize;
    };

    template <class T, class HeaderType>
    class Serialization {
        Buffer buffer;
        bool __pack(const void*src, const size_t& hashCode) {
            Reflect& table = Reflect::getInstance();
            auto rs = table.get(hashCode);
            if(!rs.first) return false;
            auto node = *rs.second;
            if(node.type == CALLABLE) {
                if(node.func.serilizeData == nullptr) {
                    cerr << "serialize function of " << node.name << " not registered." << endl;
                    return false;
                }
                auto ret = node.func.serilizeData((char*)buffer.buffer + buffer.curPos, src, buffer.maxSize - buffer.curPos);
                if(!ret.first) return false;
                buffer.curPos += ret.second;
                return true;
            } else if(node.type == NO_CALLABLE) {
                for(auto& it: node.list)
                    if(!__pack(((char*)src) + it.offset, it.hashCode))
                        return false;
                return true;
            } else {
                cerr << "Unknown NodeType." << endl;
                return false;
            }
        }
        inline void clearBuffer() {
            buffer.buffer = nullptr;
            buffer.curPos = 0;
            buffer.maxSize = 0;
        }
        inline pair<bool, size_t> __pack_2(const void*src, const size_t& hashCode) {
            if(src == nullptr) return make_pair(false, 0);
            auto ret = make_pair(__pack(src, hashCode), buffer.curPos);
            clearBuffer();
            return ret;
        }
    public:
        Serialization() {
            clearBuffer();
        }
        inline bool setBuffer(void* pBuff, const size_t& maxSize) {
            if(pBuff == nullptr) return false;
            buffer.buffer = pBuff;
            buffer.maxSize = maxSize;
            buffer.curPos = 0;
            return true;
        }
        inline pair<bool, size_t> pack(const T& src, const HeaderType& headerCode) {
            uint8_t code = uint8_t(headerCode); //暂不支持>127个类的情况
            if(buffer.maxSize < sizeof(code))
                return make_pair(false, 0);
            ((uint8_t *)buffer.buffer)[0] = code;
            buffer.curPos = sizeof(code);
            return __pack_2(&src, typeid(src).hash_code());
        }
    };

    template <class T, class HeaderType>
    class Deserialization {
        const char * pBuffer;
        size_t curPos;
        size_t maxLen;
        bool __unpack(size_t hash, void* pData) {
            auto& table = sz::Reflect::getInstance();
            auto rs = table.get(hash);
            if(!rs.first) return false;
            auto node = *rs.second;
            if(node.type == CALLABLE) {
                if(node.func.deserializeData == nullptr) {
                    cerr << "deserialize function of " << node.name << " not registered." << endl;
                    return false;
                }
                auto ret = node.func.deserializeData(pData, pBuffer + curPos, maxLen - curPos);
                if(!ret.first) return false;
                curPos += ret.second;
                return true;
            } else if(node.type == NO_CALLABLE) {
                for(auto& it: node.list)
                    if(!__unpack(it.hashCode, (char*)pData + it.offset))
                        return false;
                return true;
            } else {
                cerr << "Unknown NodeType." << endl;
                return false;
            }
        }
    public:
        bool setBuffer(const void* pBuff, const size_t& len) {
            if(pBuff == nullptr or len < 1) return false;
            pBuffer = (const char*)pBuff;
            curPos = 0;
            maxLen = len;
            return true;
        }
        pair<bool, size_t> unpack(T& data, HeaderType& headerCode) {
            auto hash = typeid(T).hash_code();
            memcpy(&headerCode, pBuffer, sizeof(uint8_t));
            memset(((char*)&headerCode) + sizeof(uint8_t), 0, sizeof(HeaderType) - sizeof(uint8_t));
            curPos ++;
            auto ret = make_pair(__unpack(hash, &data), curPos);
            pBuffer = nullptr;
            curPos = 0;
            maxLen = 0;
            return ret;
        }
    };
}

#endif //NODEOS_TPS_SERIALIZATION_H
