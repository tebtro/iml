#if !defined(IML_STRING_HASH_H)
#define IML_STRING_HASH_H



#include "iml_types.h"
#include "iml_string.h"


//
// @note: meiyan hash
//
inline u32 meiyan_hash(const String str) {
    Profile_Zone();
    
    auto count = str.count;
    auto key   = str.data;
    
    typedef uint32_t* P;
    uint32_t h = 0x811c9dc5;
    while (count >= 8) {
        //h = (h ^ ((((*(P)key) << 5) | ((*(P)key) >> 27)) ^ *(P)(key + 4))) * 0xad3e7;
        h = (h ^ ((_rotl(*(P)key, 5)) ^ *(P)(key + 4))) * 0xad3e7;
        count -= 8;
        key += 8;
    }
#define tmp h = (h ^ *(uint16_t*)key) * 0xad3e7; key += 2;
    if (count & 4) { tmp tmp }
    if (count & 2) { tmp }
    if (count & 1) { h = (h ^ *key) * 0xad3e7; }
#undef tmp
    return h ^ (h >> 16);
}


force_inline u32 string_hash(const String str) {
    return meiyan_hash(str);
}



#endif // IML_STRING_HASH_H
