/*
 *  A forked server
 *  by Martin Broadhurst (www.martinbroadhurst.com)
 */

#include <stdio.h>
#include <string.h> /* memset() */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <netdb.h>
#include <iostream>
#include <vector>
#include <pthread.h>
#include <assert.h>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "entry.h"


#define BACKLOG     10  /* Passed to listen() */
using namespace rapidjson;
//this class maintain socket list
class Callee : public CallbackInterface
{
public:
    int socket;
    rocksdb::DB* db;
    // The callback function that Caller will call.
    void cbiCallbackFunction(std::string host)
    {
        //printf("  Callee::cbiCallbackFunction() inside callback\n");
        //std::cout<<host<<","<<socket<<std::endl;
        
        rocksdb::Status status = db->Put(rocksdb::WriteOptions(), host, std::to_string(socket));

    }
};
//void remove_escape(char *string);
void *handle(void *);
void *handlecmd(void *);
void *handlecommand(void *);
rocksdb::DB* db;
rocksdb::Options options;
int main(void)
{
    options.create_if_missing = true;
    rocksdb::Status status = rocksdb::DB::Open(options, DB_PATH, &db);
    assert(status.ok());
    
    int sock,sock1;
    
    struct addrinfo hints, *res,hints1,*res1;
    int reuseaddr = 1; /* True */
    
    /* Get the address info */
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(HOST_IP, PORT, &hints, &res) != 0) {
        perror("getaddrinfo");
        return 1;
    }

    /* Get the address provisioning socket info */
    memset(&hints1, 0, sizeof hints1);
    hints1.ai_family = AF_INET;
    hints1.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(HOST_IP, CMDPORT, &hints1, &res1) != 0) {
        perror("getaddrinfo");
        return 1;
    }
    
    /* Create the socket */
    sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock == -1) {
        perror("socket");
        return 1;
    }
    
    /* Create the socket for provisioning*/
    sock1 = socket(res1->ai_family, res1->ai_socktype, res1->ai_protocol);
    if (sock1 == -1) {
        perror("socket");
        return 1;
    }
    
    /* Enable the socket to reuse the address */
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(int)) == -1) {
        perror("setsockopt");
        return 1;
    }
    
    /* Enable the socket to reuse the address */
    if (setsockopt(sock1, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(int)) == -1) {
        perror("setsockopt");
        return 1;
    }
    
    /* Bind to the address */
    if (bind(sock, res->ai_addr, res->ai_addrlen) == -1) {
        perror("bind");
        return 1;
    }
    
    /* Bind to the address */
    if (bind(sock1, res1->ai_addr, res1->ai_addrlen) == -1) {
        perror("bind1");
        return 1;
    }
    
    /* Listen */
    if (listen(sock, BACKLOG) == -1) {
        perror("listen");
        return 1;
    }

    /* Listen */
    if (listen(sock1, BACKLOG) == -1) {
        perror("listen1");
        return 1;
    }
    
    freeaddrinfo(res);
    freeaddrinfo(res1);
    pthread_t thread;
    int iret = pthread_create( &thread, NULL, handlecmd, (void*) &sock1);
    if(iret)
    {
        fprintf(stderr,"Error - pthread_create() return code: %d\n",iret);
        exit(EXIT_FAILURE);
    }

    /* Main loop */
    while (1) {
        struct sockaddr cli_addr;
        socklen_t clilen;
        
        int newsock = accept(sock, &cli_addr, &clilen);
        
        if (newsock == -1) {
            perror("accept");
            return 0;
        }
        pthread_t thread1;
        int iret1 = pthread_create( &thread1, NULL, handle, (void*) &newsock);
        if(iret1)
        {
            fprintf(stderr,"Error - pthread_create() return code: %d\n",iret1);
            exit(EXIT_FAILURE);
        }
        
    }
    
    return 0;
}

void *handlecmd(void *socket){
    while(1){
        struct sockaddr cli_addr;
        socklen_t clilen;
        int sock = *(int*)socket;
        int newsock = accept(sock, &cli_addr, &clilen);
        if (newsock == -1) {
            perror("accept");
            return 0;
        }
        //printf("cmd connected\n");
        pthread_t thread1;
        int iret1 = pthread_create( &thread1, NULL, handlecommand, (void*) &newsock);
        if(iret1)
        {
            fprintf(stderr,"Error - pthread_create() return code: %d\n",iret1);
            exit(EXIT_FAILURE);
        }
    }
    pthread_exit(NULL);
    return 0;
}

char* to_char(const std::string& string)
{
    char* return_string = new char[string.length() + 1];
    strcpy(return_string, string.c_str());
    
    return return_string;
}

int getsocketid(char* msid){
    //get session id
    std::string val;
    strcat(msid, "_ses");
    rocksdb::Status status = db->Get(rocksdb::ReadOptions(),msid, &val);
    std::cout<<val<<std::endl;
    char* peer=strtok(to_char(val), "#;");
    //printf("peer %s\n",peer);
    status = db->Get(rocksdb::ReadOptions(),peer, &val);
    std::cout<<val<<std::endl;
    
    return atoi(to_char(val));
}

void *handlecommand(void *sock){
    int newsock = *(int*)sock;
    int bytes;
    char cClientMessage[32];
    char result[1024];
    int prompt=write(newsock, "ocs>", 4);
    while((bytes = recv(newsock, cClientMessage, sizeof(cClientMessage), 0)) > 0)
    {
        char* chars_array = strtok(cClientMessage, "#:");
        int i=0;
        char* params[4];
        while(chars_array)
        {
            //        MessageBox(NULL, subchar_array, NULL, NULL);
            std::cout << chars_array << '\n';
            params[i]=chars_array;
            i++;
            chars_array = strtok(NULL, "#:");
        }
        if(memcmp( params[0], "quit", strlen( "quit") ) == 0){
            close(newsock);
        
        }else if( memcmp( params[0], "setslice", strlen( "setslice") ) == 0) {
            //char result[1024];
            bzero(result, 1024);
            std::string val;
            //cek if default exist the copy to msid
            std::string valdef;
            rocksdb::Status status = db->Put(rocksdb::WriteOptions(),"slice", params[1]);
            std::cout<<val<<std::endl;
            char* value = to_char(val);
            strcat(result, value);
            strcat(result, "OK\nocs>");
            int res=write(newsock, result, strlen(result));
        }else if( memcmp( params[0], "add", strlen( "add") ) == 0 &&memcmp( params[1], "msid", strlen( "msid") ) == 0 ) {
            //char result[1024];
            bzero(result, 1024);
            std::string val;
            //cek if default exist the copy to msid
            std::string valdef;
            rocksdb::Status status = db->Get(rocksdb::ReadOptions(),"default", &valdef);
            if(valdef==""){
                valdef="{\"rg\":[]}";
            }
            status = db->Put(rocksdb::WriteOptions(),params[2], valdef);
            std::cout<<val<<std::endl;
            char* value = to_char(val);
            strcat(result, value);
            strcat(result, "OK\nocs>");
            int res=write(newsock, result, strlen(result));
        }else if( memcmp( params[0], "del", strlen( "del") ) == 0 &&memcmp( params[1], "msid", strlen( "msid") ) == 0 ) {
            //char result[1024];
            bzero(result, 1024);
            rocksdb::Status status = db->Delete(rocksdb::WriteOptions(),params[2]);
            char* info="_usage";
            char rarinfo[strlen(params[2])+strlen(info)];
            strcpy(rarinfo,params[2]); // copy string one into the result.
            strcat(rarinfo,info); // append string two to the result.
            status = db->Delete(rocksdb::WriteOptions(),rarinfo);
            strcat(result, "OK\nocs>");
            int res=write(newsock, result, strlen(result));
        }else if (memcmp( params[0], "rar", strlen( "rar") ) == 0){
            //printf("send rar to %s \n",params[1]);
            //getrar here
            entry e=entry();
            e.db=db;
            char* msg;
            diameter reply=e.createRAR(params[1]);
            char resp[reply.len+4];
            char* r=resp;
            reply.compose(r);
            
            //find socket here
            int socketmsid=getsocketid(params[1]);
            
            int w = write(socketmsid,resp,reply.len+4);
            if(w<=0){
                //fail write
                msg="rar is failed";
            }else{
                msg="rar is sent";
            }
            bzero(result, 1024);
            strcat(result, msg);
            strcat(result, "\nocs>");
            int res=write(newsock, result, strlen(result));
        }else if( memcmp( params[0], "show", strlen( "show") ) == 0 &&memcmp( params[1], "msid", strlen( "msid") ) == 0 ) {
            //char result[1024];
            char* info="_usage";
            char rarinfo[strlen(params[2])+strlen(info)];
            strcpy(rarinfo,params[2]); // copy string one into the result.
            strcat(rarinfo,info); // append string two to the result.
            
            bzero(result, 1024);
            std::string val;
            rocksdb::Status status = db->Get(rocksdb::ReadOptions(),params[2], &val);
            std::cout<<val<<std::endl;
            std::string valuse;
            status = db->Get(rocksdb::ReadOptions(),rarinfo, &valuse);
            std::cout<<valuse<<std::endl;
            char* value = to_char(val);
            strcat(result, value);
            value = to_char(valuse);
            strcat(result, value);
            strcat(result, "\nocs>");
            int res=write(newsock, result, strlen(result));
        }else if( memcmp( params[0], "delac", strlen( "delac") ) == 0) {
            //            char* msid=params[2];
            //            remove_escape(msid);
            //            printf("show msid %s\n",msid);
            char* info="_usage";
            char rarinfo[strlen(params[1])+strlen(info)];
            strcpy(rarinfo,params[1]); // copy string one into the result.
            strcat(rarinfo,info); // append string two to the result.
            
            std::string val,val1;
            rocksdb::Status status = db->Get(rocksdb::ReadOptions(),rarinfo, &val);
            std::cout<<val<<std::endl;
            char json[val.size()+1];//as 1 char space for null is also required
            strcpy(json, val.c_str());
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
                      //     itr->name.GetString(), itr->value.GetInt());
                    if(strcmp(rgkey, params[2]) == 0){
                        a.Erase(&c);
                    }
                }
                
            }
            //printf("Updated json:\n");
            
            // Convert JSON document to string
            StringBuffer strbuf;
            Writer<StringBuffer> writer(strbuf);
            dom.Accept(writer);
            // string str = buffer.GetString();
            //printf("--\n%s\n--\n", strbuf.GetString());
            status = db->Put(rocksdb::WriteOptions(),rarinfo, strbuf.GetString());
            //char result[1024];
            bzero(result, 1024);
            strcat(result, to_char(strbuf.GetString()));
            strcat(result, "\nocs>");
            int res=write(newsock, result, strlen(result));
            
        }else if( memcmp( params[0], "delrg", strlen( "delrg") ) == 0) {
//            char* msid=params[2];
//            remove_escape(msid);
//            printf("show msid %s\n",msid);
            std::string val,val1;
            rocksdb::Status status = db->Get(rocksdb::ReadOptions(),params[1], &val);
            std::cout<<val<<std::endl;
            char json[val.size()+1];//as 1 char space for null is also required
            strcpy(json, val.c_str());
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
                      //     itr->name.GetString(), itr->value.GetInt());
                    if(strcmp(rgkey, params[2]) == 0){
                        a.Erase(&c);
                    }
                }
                    
            }
            //printf("Updated json:\n");
            
            // Convert JSON document to string
            StringBuffer strbuf;
            Writer<StringBuffer> writer(strbuf);
            dom.Accept(writer);
            // string str = buffer.GetString();
            //printf("--\n%s\n--\n", strbuf.GetString());
            status = db->Put(rocksdb::WriteOptions(),params[1], strbuf.GetString());
            //char result[1024];
            bzero(result, 1024);
            strcat(result, to_char(strbuf.GetString()));
            strcat(result, "\nocs>");
            int res=write(newsock, result, strlen(result));

        }else if( memcmp( params[0], "addrg", strlen( "addrg") ) == 0) {
            std::string val,val1;
            rocksdb::Status status = db->Get(rocksdb::ReadOptions(),params[1], &val);
            //std::cout<<val<<std::endl;
            char json[val.size()+1];//as 1 char space for null is also required
            strcpy(json, val.c_str());
            Document dom;
            //printf("Original json:\n%s\n", json);
            char buffer[sizeof(json)];
            memcpy(buffer, json, sizeof(json));
            if (dom.ParseInsitu<0>(buffer).HasParseError())
                printf("error parsing\n");
            Value& a = dom["rg"];
            assert(a.IsArray());
            Document::AllocatorType& allocator = dom.GetAllocator();
            rapidjson::Value objValue;
            objValue.SetObject();
            Value key,q;
            objValue.AddMember(key.SetString(params[2], strlen(params[2])),q.SetInt(atoi(params[3])), allocator);
            a.PushBack(objValue, allocator);
            //printf("Updated json:\n");
            // Convert JSON document to string
            StringBuffer strbuf;
            Writer<StringBuffer> writer(strbuf);
            dom.Accept(writer);
            // string str = buffer.GetString();
            //printf("--\n%s\n--\n", strbuf.GetString());
            status = db->Put(rocksdb::WriteOptions(),params[1], strbuf.GetString());
            char result[1024];
            bzero(result, 1024);
            strcat(result, to_char(strbuf.GetString()));
            strcat(result, "\nocs>");
            int res=write(newsock, result, strlen(result));
        }
    }
    
    if(bytes == 0)
    {
        //socket was gracefully closed
    }
    else if(bytes < 0)
    {
        //socket error occurred
    }
    pthread_exit(NULL);
    return 0;
}

void *handle(void *sock){
    int newsock = *(int*)sock;
    entry e=entry();
    Callee callee;
    callee.socket=newsock;
    callee.db=db;
    e.connectCallback(&callee);
    e.db=db;
    char* h=new char[4];
    int n;
    while((n=read(newsock,h,4))>0){
        //char* h=head;
        
        int32_t l =(((0x00 & 0xff) << 24) | ((*(h+1) & 0xff) << 16)| ((*(h+2) & 0xff) << 8) | ((*(h+3) & 0xff)))-4;
        //printf("len: %zu\n",l);
        
        //char body[l];
        char* b=new char[l];
        n = read(newsock,b,l);
        //char* b=new char[l];
        diameter d=diameter(h,b,l);
        
        diameter reply=e.process(d);
        delete b;
        if(reply.len>0){    //isrequest, need answer
            //char resp[reply.len+4];
            char* r=new char[reply.len+4];
            reply.compose(r);
            delete reply.h;
            delete reply.b;
            int w = write(newsock,r,reply.len+4);
            if(w<=0){
                //fail write
            }
            delete r;
        }
    }
    delete h;
    pthread_exit(NULL);
  if(n == 0)
  {
      //socket was gracefully closed
  }
  else if(n < 0)
  {
      //socket error occurred
  }
    return 0;
}
