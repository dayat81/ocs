//
//  logic.cpp
//  diameter
//
//  Created by hidayat on 10/15/15.
//  Copyright © 2015 hidayat. All rights reserved.
//

#include <stdio.h>
#include <iostream>
#include "logic.h"
#include "avputil.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <list>

using namespace rapidjson;
logic::logic(){
    //
}

void logic::getResult(diameter d,avp* &allavp,int &l,int &total){
    avputil util=avputil();
    
    //read avp
    avp ori_host=d.getAVP(264, 0);
    //printf("ori len %i \n",ori_host.len);
    if(ori_host.len>0){
        std::cout<<util.decodeAsString(ori_host)<<std::endl;
    }
    
    char f=0x40;
    std::string ori ="vmclient.myrealm.example";
    //printf("size : %i\n",ori.size());
    avp o=util.encodeString(264,0,f,ori);
    //o.dump();
    //printf("\n");
    avp id_t=util.encodeInt32(450, 0, 0x40, 1);
    //id_t.dump();
    //printf("\n");
    avp id_d=util.encodeString(444, 0, 0x40, "628119105569");
    //id_d.dump();
    avp* listavp[2]={&id_t,&id_d};
    avp sid=util.encodeAVP(443, 0, 0x40, listavp, 2);
    
    avp id_t1=util.encodeInt32(450, 0, 0x40, 0);
    avp id_d1=util.encodeString(444, 0, 0x40, "51010628119105569");
    avp* listavp1[2]={&id_t1,&id_d1};
    avp sid1=util.encodeAVP(443, 0, 0x40, listavp1, 2);
    
    //sid.dump();
    //printf("\n");
    total=sid.len+o.len+sid1.len;
    l=3;
    allavp=new avp[l];
    allavp[0]=o;
    allavp[1]=sid;
    allavp[2]=sid1;
}

void logic::getCCA(diameter d,avp* &allavp,int &l,int &total){
    avputil util=avputil();
    avp cca_sessid=d.copyAVP(263, 0);
    std::string sessidval="";
    avp sessid=d.getAVP(263, 0);
    if(sessid.len>0){
        sessidval=util.decodeAsString(sessid);
    }
    avp cca_req_type=d.copyAVP(416, 0);
    int req_type=0;
    avp reqtype=d.getAVP(416, 0);
    if (reqtype.len>0) {
        req_type=util.decodeAsInt(reqtype);
    }
    avp cca_req_num=d.copyAVP(415, 0);
    
    char f=0x40;
    avp o=util.encodeString(264,0,f,ORIGIN_HOST);
    avp realm=util.encodeString(296,0,f,ORIGIN_REALM);
    avp authappid=util.encodeInt32(258, 0, f, 4);
    avp rc=util.encodeInt32(268, 0, f, 2001);
    avp flid=util.encodeInt32(629, 10415, 0xc0, 1);
    avp fl=util.encodeInt32(630, 10415, 0xc0, 3);
    avp vid=util.encodeInt32(266, 0, f, 10415);
    avp* list_fl[3]={&vid,&flid,&fl};
    //avp sf=util.encodeAVP(628, 10415, 0xc0, list_fl, 3);
    
    total=cca_sessid.len+o.len+realm.len+cca_req_type.len+cca_req_num.len+rc.len+authappid.len;
    l=7;
    std::list<avp> ListMSCC;
    //avp msccresp=avp(0,0);
    //if(req_type==1){//initial
        //read avp msid
        bool exit=false;
        std::string msidstring="";
        while(!exit){
            avp msid=d.getAVP(443, 0);
            //printf("msid len %i \n",msid.len);
            if(msid.len>0){
                avp msidtype=util.getAVP(450, 0, msid);
                //printf("msidtype len %i \n",msidtype.len);
                if(msidtype.len>0){
                    int type=util.decodeAsInt(msidtype);
                    //printf("decoded : %i\n",type);
                    if(type==0){
                        exit=true;
                        avp msiddata=util.getAVP(444, 0, msid);
                        msidstring=util.decodeAsString(msiddata);
                    }
                }
            }else{//avp not found
                exit=true;
            }
        }
        std::cout<<msidstring<<std::endl;
        std::string msidusageinfo=msidstring;
        msidusageinfo.append("_usage");
        //store sessid,msid
        
    rocksdb::Status status;
//    if (req_type==1) {
//        status = db->Put(rocksdb::WriteOptions(), sessidval, msidstring);
//        //status = db->Put(rocksdb::WriteOptions(), msidsesinfo.append("_sess"), sessidval);
//    }
    
        //status = db->Put(rocksdb::WriteOptions(), msidrarinfo.append("_rarinfo"), "{\"addacg\":[],\"delacg\":[]}");
        std::string val;
        status = db->Get(rocksdb::ReadOptions(), msidstring, &val);
        bool profilefound=false;
        if(val==""){
            //look for default profile
            status = db->Get(rocksdb::ReadOptions(), "default", &val);
            if(val!=""){
                profilefound=true;
            }
        }else{
            profilefound=true;
        }
    //std::cout<<"quota"<<val<<std::endl;
        if(profilefound){
            //if(a.Size()>0){
                //cek mscc avp in ccr with iteration
                bool all=false;
                int rgnum = 0,totalnum=0,quota = 0;
                //int64_t totalnum;
                while (!all) {
                    avp mscc=d.getAVP(456, 0);
                    if (mscc.len>0) {
                        avp rsu=util.getAVP(437, 0, mscc);
                        avp usu=util.getAVP(446, 0, mscc);
                        avp rg=util.getAVP(432, 0, mscc);
                        if(rg.len>0){
                            rgnum=util.decodeAsInt(rg);
                            printf("rg:%i\n",rgnum);
                        }
                        std::string s = std::to_string(rgnum);
                        char const *pchar = s.c_str();
                        Document domrg;
                        domrg.Parse(val.c_str());
                        const Value& a = domrg["rg"];
                        assert(a.IsArray());
                        for (rapidjson::SizeType i = 0; i < a.Size(); i++)
                        {
                            const Value& c = a[i];
                            for (Value::ConstMemberIterator itr = c.MemberBegin();
                                 itr != c.MemberEnd(); ++itr)
                            {
                                const char* rgkey=itr->name.GetString();
                                //printf("Type of member %s is %i\n",
                                //      itr->name.GetString(), itr->value.GetInt());
                                if(strcmp(rgkey, pchar) == 0){
                                    quota=itr->value.GetInt();
                                }
                            }
                            
                        }
                        //get prev usage in database
                        std::string valusage;
                        status = db->Get(rocksdb::ReadOptions(), msidusageinfo, &valusage);
                        //std::cout<<"valusage -"<<valusage<<"-"<<std::endl;
                        if (valusage!="") {
                            Document domrgusage;
                            domrgusage.Parse(valusage.c_str());
                            const Value& ausage = domrgusage["rg"];
                            assert(ausage.IsArray());
                            for (rapidjson::SizeType i = 0; i < ausage.Size(); i++)
                            {
                                const Value& c = ausage[i];
                                for (Value::ConstMemberIterator itr = c.MemberBegin();
                                     itr != c.MemberEnd(); ++itr)
                                {
                                    const char* rgkey=itr->name.GetString();
                                    //printf("Type of member %s is %i\n",
                                    //     itr->name.GetString(), itr->value.GetInt());
                                    if(strcmp(rgkey, pchar) == 0){
                                        totalnum=itr->value.GetInt();
                                    }
                                }
                                
                            }
                        }
                        //cek usage report
                        if(usu.len>0){
                            avp total=util.getAVP(421, 0, usu);
                            if(total.len>0){
                                printf("usage %i\n", totalnum);
                                totalnum=totalnum+util.decodeAsInt(total);
                                //get prev usage in database
                                std::string valusage;
                                status = db->Get(rocksdb::ReadOptions(), msidusageinfo, &valusage);
                                //std::cout<<"valusage -"<<valusage<<"-"<<std::endl;
                                if (valusage!="") {
                                    //get & delete old &create new
                                    
                                    char json[valusage.size()+1];//as 1 char space for null is also required
                                    strcpy(json, valusage.c_str());
                                    Document dom;
                                    //printf("Original json:\n%s\n", json);
                                    char buffer[sizeof(json)];
                                    memcpy(buffer, json, sizeof(json));
                                    if (dom.ParseInsitu<0>(buffer).HasParseError())
                                        printf("error parsing\n");
                                    Value& a = dom["rg"];
                                    assert(a.IsArray());
                                    for (rapidjson::SizeType i = 0; i < a.Size(); i++)
                                    {
                                        const Value& c = a[i];
                                        for (Value::ConstMemberIterator itr = c.MemberBegin();
                                             itr != c.MemberEnd(); ++itr)
                                        {
                                            const char* rgkey=itr->name.GetString();
                                            //printf("Type of member %s is %i\n",
                                               //    itr->name.GetString(), itr->value.GetInt());
                                            if(strcmp(rgkey, pchar) == 0){
                                                //totalnum=totalnum+(itr->value.GetInt());
                                                a.Erase(&c);
                                            }
                                        }
                                        
                                    }
                                    Document::AllocatorType& allocator = dom.GetAllocator();
                                    rapidjson::Value objValue;
                                    objValue.SetObject();
                                    Value key,q;
                                    objValue.AddMember(key.SetString(pchar, strlen(pchar)),q.SetInt(totalnum), allocator);
                                    a.PushBack(objValue, allocator);
                                    //printf("Updated json:\n");
                                    
                                    // Convert JSON document to string
                                    StringBuffer strbuf;
                                    Writer<StringBuffer> writer(strbuf);
                                    dom.Accept(writer);
                                    // string str = buffer.GetString();
                                    //printf("--\n%s\n--\n", strbuf.GetString());
                                    status = db->Put(rocksdb::WriteOptions(),msidusageinfo, strbuf.GetString());
                                }else{
                                    //create new
                                    printf("create new usage\n");
                                    std::string valdef="{\"rg\":[{\"";
                                    valdef.append(pchar);
                                    valdef.append("\":");
                                    std::string st = std::to_string(totalnum);
                                    char const *pchart = st.c_str();
                                    valdef.append(pchart);
                                    valdef.append("}]}");
                                    status = db->Put(rocksdb::WriteOptions(),msidusageinfo, valdef);
                                }
                            }
                        }
                        if(rsu.len>-1){
                            printf("rsu\n");
                            //cek quota-usage for granting
                            printf("quota: %i, usage: %i\n",quota,totalnum);
                            int grant=1000*quota-totalnum;
                            printf("grant: %i\n",grant);
                            if(grant>0){
                                //create octet avp
                                avp grantvol = util.encodeInt64(421, 0, f, grant);
                                //grantvol.dump();
                                //printf("\n");
                                avp* listavp[1]={&grantvol};
                                avp gsu=util.encodeAVP(431, 0, f, listavp, 1);
                                //gsu.dump();
                                avp rgrespon=util.encodeInt32(432, 0, f, rgnum);
				avp sidrespon=util.encodeInt32(439, 0, f, rgnum);
                                avp rcmscc=util.encodeInt32(268, 0, f, 2001);
                                avp* listavp1[4]={&gsu,&rgrespon,&sidrespon,&rcmscc};
                                avp msccresp=util.encodeAVP(456, 0, f, listavp1, 4);
                                //msccresp.dump();
                                printf("\n");
                                l++;
                                total=total+msccresp.len;
                                ListMSCC.push_front(msccresp);
                                //add mscc
                            }
                        }
                        
                    }else{
                        all=true;
                    }
                }
//                avp* acg=new avp[a.Size()];
//                for (SizeType i = 0; i < a.Size(); i++){ // Uses SizeType instead of size_t
//                    //printf("a[%d] = %s\n", i, a[i].GetString());   //map to charging-rule-name-avp
//                    avp temp=util.encodeString(1004, 10415, 0xC0, a[i].GetString());
//                    temp.dump();
//                    //printf("\n");
//                    *acg=temp;
//                    acg++;
//                }
//                acg=acg-a.Size();
//                cr_install=util.encodeAVP(1001, 10415, 0xC0, acg, a.Size());
            //}
        }
    allavp=new avp[l];
    allavp[0]=cca_sessid;
    allavp[1]=o;
    allavp[2]=realm;
    allavp[3]=cca_req_type;
    allavp[4]=cca_req_num;
    //allavp[5]=authappid;
    allavp[5]=rc;
    allavp[6]=authappid;
    int i=7;
    for (std::list<avp>::iterator it = ListMSCC.begin(); it != ListMSCC.end(); it++){
        allavp[i]=*it;
        i++;
    }
    //}
//    if (req_type==3){ //terminate
//        //get msid by sessid
////        std::string val;
////        rocksdb::Status status = db->Get(rocksdb::ReadOptions(), sessidval, &val);
////        status = db->Delete(rocksdb::WriteOptions(),val.append("_sess"));
//        //delete sessid
//        status = db->Delete(rocksdb::WriteOptions(),sessidval);
//    }
 
}


