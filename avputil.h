//
//  avputil.h
//  diameter
//
//  Created by hidayat on 10/14/15.
//  Copyright © 2015 hidayat. All rights reserved.
//

#ifndef avputil_h
#define avputil_h

#include "avp.h"
#include <string>

class avputil{
public:
    avputil();
    
    std::string decodeAsString(avp a);
    int decodeAsInt(avp a);
    //int64_t decodeAsInt64(avp a);
    
    avp getAVP(int acode,int vcode,avp a);
avp getAVP(int acode,int vcode,avp a,int i);
    
    avp encodeString(int acode,int vcode,char flags,std::string value);
    avp encodeInt32(int acode,int vcode,char flags,int value);
    avp encodeInt64(int acode,int vcode,char flags,int64_t value);
    avp encodeIP(int acode,int vcode,char flags,unsigned int value[]);
    //avp encodeIP(int acode,int vcode,char flags,char* val);
    avp encodeAVP(int acode,int vcode,char flags,avp* list[],int l);
    avp encodeAVP(int acode,int vcode,char flags,avp* list,int l);
};

#endif /* avputil_h */
