//
//  diameter.h test
//  diameter
//
//  Created by hidayat on 10/14/15.
//  Copyright © 2015 hidayat. All rights reserved.
//

#ifndef diameter_h
#define diameter_h

#include "avp.h"
//#include <functional>
#include <string>
#define ORIGIN_HOST    "gy812.cbtevspap12.xl.co.id"
#define ORIGIN_REALM   "gy812.tc.xl.co.id"
#define HOST_IP "10.195.84.157"
#define DB_PATH "./csdb"
#define PORT    "13136" /* Port to listen on */
#define CMDPORT    "2345" /* Port to listen on */
class diameter{

public:
    char* h;
    char* b;
    int len;
    int curr;
    std::string host;
    
    char version;
    char cflags;
    char* ccode;
    char* appId;
    char* hbh;
    char* e2e;
    
    diameter(char* h,char* b,int l);
    void compose(char* res);
    void dump();
    void populateHeader();
    avp getAVP(int acode,int vcode);
    avp copyAVP(int acode,int vcode);
 
    // call function with one extrar (int by value) last parameter
//    template < typename FN, typename... ARGS >
//    void mylibfun_add_tail( int a, int b, FN&& fn, ARGS&&... args )
//    {
//        if( a<b )//cek successful cea here
//        {
//            const int extra_param = a + b ;
//            // call function with an additional int as the last argument
//            std::bind( std::forward<FN>(fn), std::forward<ARGS>(args)..., extra_param,host )() ;
//        }
//    }
    
    //todo
    //create destructor to delete h,b in heap
};

#endif /* diameter_h */
