#include <dirent.h>
#include "Common.hpp"
#include "Watcher.hpp"
#include "Sender.hpp"
//-------------------------------------------------------------------------------------------------

string IntToStr(int v)
{
 char a[42];
 sprintf(a,"%d",v);
 return a;
}
//-------------------------------------------------------------------------------------------------

string FloatToStr(float v)
{
 char a[42];
 sprintf(a,"%.3f",v);
 return a;
}
//-------------------------------------------------------------------------------------------------

string ULLToStr(ULL v)
{
 char a[64];
 sprintf(a,"%llu",v);
 return a;
}
//-------------------------------------------------------------------------------------------------

void SplitString(const string& str,vector<string>& vec,const char* step)
{
 char* chp=strdupa(str.c_str());
 char seps[]=" ,'\"\t\n";
 char* token;
 if(step==NULL)
    step=seps;
 char* temp;
 token=strtok_r(chp,step,&temp);
 while(token!=NULL )
      {
       vec.push_back(token);
       token=strtok_r(NULL,step,&temp);
      }//end while
}
//-------------------------------------------------------------------------------------------------

string GetCurrentTime(const unsigned char split)
{
 time_t now=time(NULL);
 struct tm tnow_o;
 localtime_r(&now,&tnow_o);
 struct tm* tnow=&tnow_o;
 char str[128];
 if(split==0)
    sprintf(str,"%04u%02u%02u%02u%02u%02u",1900+tnow->tm_year,tnow->tm_mon+1,tnow->tm_mday,
            tnow->tm_hour,tnow->tm_min,tnow->tm_sec);
 else
    sprintf(str,"%04u%c%02u%c%02u%c%02u%c%02u%c%02u",1900+tnow->tm_year,split,tnow->tm_mon+1,split,tnow->tm_mday,
            split,tnow->tm_hour,split,tnow->tm_min,split,tnow->tm_sec);
 return string(str);
}
//-------------------------------------------------------------------------------------------------

string Timet2String(time_t timet)
{
 struct tm tnow_o;
 localtime_r(&timet,&tnow_o);
 struct tm* tnow=&tnow_o;
 char str[128];
 sprintf(str,"%04u%02u%02u%02u%02u%02u",1900+tnow->tm_year,tnow->tm_mon+1,tnow->tm_mday,
         tnow->tm_hour,tnow->tm_min,tnow->tm_sec);
 return str;
}
//-------------------------------------------------------------------------------------------------

bool FilePutContent(const string& filename,const string& content)
{    
 FILE* fp=fopen(filename.c_str(),"wb");
 if(fp==NULL)
    return false;
 int wl=fwrite(content.c_str(),1,content.size(),fp);
 fclose(fp);
 return wl==int(content.size());
} 
//-------------------------------------------------------------------------------------------------

bool FileGetContent(const string& filename,string& content)
{
 content="";
 FILE* fp=fopen(filename.c_str(),"rb");
 if(fp==NULL)
    return false;
 fseek(fp,0,SEEK_END);
 unsigned int fl=ftell(fp);
 if(fl==0)
   {
    fclose(fp);
    return false;
   }
 fseek(fp,0,SEEK_SET);
 char* buffer=new char[fl+1];
 unsigned int rfl=fread(buffer,1,fl,fp);
 if(rfl!=fl)
   {
    delete[] buffer;
    fclose(fp);
    return false;
   }
 buffer[fl]=0;
 content.resize(fl);
 copy(buffer,buffer+fl,content.begin());
 delete[] buffer;
 fclose(fp);
 return true;
}
//-------------------------------------------------------------------------------------------------

off_t GetFileSize(FILE* fp)
{
 int fd=fileno(fp);
 struct stat st;
 memset(&st,0,sizeof(st));
 fstat(fd,&st);
 return st.st_size;
}
//-------------------------------------------------------------------------------------------------

off_t GetFileSize(const string& filename)
{
 struct stat st;
 memset(&st,0,sizeof(st));
 stat(filename.c_str(),&st);
 return st.st_size;
}
//-------------------------------------------------------------------------------------------------

bool FileExists(const string& filename)
{
 struct stat st;
 memset(&st,0,sizeof(st));
 if(stat(filename.c_str(),&st)!=0)
    return false;
 return true;
}
//-------------------------------------------------------------------------------------------------

void Trim(string& str,bool trimstring)
{
 while(!str.empty()&&(str[0]=='\r'||str[0]=='\t'||str[0]==' '||str[0]=='\n'))
       str.erase(0,1);
 while(!str.empty()&&
       (str[str.size()-1]=='\r'||str[str.size()-1]=='\t'||
        str[str.size()-1]==' '||str[str.size()-1]=='\n')
      )
       str.erase(str.size()-1,1);
 if(trimstring)
   {
    while(!str.empty()&&(str[0]=='\''||str[0]=='"'))
          str.erase(0,1);
    while(!str.empty()&&(str[str.size()-1]=='\''||str[str.size()-1]=='"'))
       str.erase(str.size()-1,1);
   }
}
//-------------------------------------------------------------------------------------------------

bool SetNBSocket(int fd)
{
 int flag=fcntl(fd,F_GETFL);
 if(flag<0)
   {
    printf("%s\tERROR: fcntl failed to get flag %d\n",GetCurrentTime().c_str(),errno);
    return false;
   }
 flag|=O_NONBLOCK;
 if(fcntl(fd,F_SETFL,flag)<0)
   {
    printf("%s\tERROR: fcntl failed to set flag %d\n",GetCurrentTime().c_str(),errno);
    return false;
   }
 return true;
}
//-------------------------------------------------------------------------------------------------

bool SetCESocket(int fd)
{
 int flag=fcntl(fd,F_GETFD);
 if(flag<0)
   {
    printf("%s\tERROR: fcntl failed to get flag %d\n",GetCurrentTime().c_str(),errno);
    return false;
   }
 flag|=FD_CLOEXEC;
 if(fcntl(fd,F_SETFD,flag)<0)
   {
    printf("%s\tERROR: fcntl failed to set flag %d\n",GetCurrentTime().c_str(),errno);
    return false;
   }
 return true;
}
//-------------------------------------------------------------------------------------------------

int CreateAndConnectUnixSocket(const string& usfn)
{
 int fd=socket(AF_UNIX,SOCK_STREAM,0);
 if(fd==-1)
    return fd;
 struct sockaddr_un address;
 address.sun_family=AF_UNIX;
 strcpy(address.sun_path,usfn.c_str());
 if(connect(fd,(struct sockaddr *)&address,sizeof(address))!=0)
   {
    close(fd);
    return -1;
   }
 return fd; 
}
//-------------------------------------------------------------------------------------------------

int CreateServerUnixSocket(const string& usfn)
{
 int fd=0;
 struct sockaddr_un un;
 memset(&un,0,sizeof(un));
 un.sun_family=AF_UNIX;
 unlink(usfn.c_str());
 strcpy(un.sun_path,usfn.c_str());
 if((fd=socket(AF_UNIX, SOCK_STREAM,0))<0)
   {
    printf("%s\tfailed to create unix socket(%d) %s\n",
           GetCurrentTime().c_str(),errno,usfn.c_str());
    return -1;
   }
 SetNBSocket(fd);
 SetCESocket(fd);
 int reuseaddr=1;
 int tcp_nodelay=1;
 setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,(const void*)&reuseaddr,sizeof(int));
 setsockopt(fd,IPPROTO_TCP,TCP_NODELAY,&tcp_nodelay,sizeof(tcp_nodelay));
 if(bind(fd,(struct sockaddr *)&un,sizeof(un))<0)
   {
    printf("%s\tfailed to bind(%d) %s\n",
           GetCurrentTime().c_str(),errno,usfn.c_str());
    return -2;
   }
 if(listen(fd,LISTEN_QUEUE_LENGTH)<0)
   {
    printf("%s\tfailed to listen(%d) %s\n",
           GetCurrentTime().c_str(),errno,usfn.c_str());
    return -3;
   }
 return fd;
}
//-------------------------------------------------------------------------------------------------

bool VisitPath(const string& path,int& dir_count,Watcher* watcher)
{
 struct dirent** list=NULL;
 int nEntries=scandir(path.c_str(),&list,NULL,NULL);
 if(nEntries<0)
    return false;
 dir_count++;
 if(watcher!=NULL)
   {
    if(path.empty()==false&&path[path.size()-1]=='/')
       watcher->OnScanDir(path);
    else
       watcher->OnScanDir(path+"/");
   }
 bool ret=true;
 for(int i=0;i<nEntries;i++)
    {
     if(strcmp(list[i]->d_name,".")==0||
        strcmp(list[i]->d_name,"..")==0)
       {
        free(list[i]);
        list[i]=NULL;
        continue;
       }
     if(list[i]->d_type==DT_DIR)
       {
        static char buf[4096];
        if(path[path.size()-1]!='/')
           snprintf(buf,4095,"%s/%s",path.c_str(),list[i]->d_name);
        else
           snprintf(buf,4095,"%s%s",path.c_str(),list[i]->d_name);
        if(VisitPath(buf,dir_count,watcher)==false)
           ret=false;
       }
     free(list[i]);
     list[i]=NULL;
    }//end for i
 free(list);
 return ret;
}
//-------------------------------------------------------------------------------------------------

bool ListFileFromPath(const string& path,vector<FNSortHelper>& files,const string& ext)
{
 struct dirent** list=NULL;
 int nEntries=scandir(path.c_str(),&list,NULL,NULL);
 if(nEntries<0)
    return false;
 for(int i=0;i<nEntries;++i)
    {
     if(strcmp(list[i]->d_name,".")==0||
        strcmp(list[i]->d_name,"..")==0)
       {
        free(list[i]);
        list[i]=NULL;
        continue;
       }
     if(list[i]->d_type==DT_DIR)
       {
        free(list[i]);
        list[i]=NULL;
        continue;
       }
     FNSortHelper helper;
     helper.filefullname=path+list[i]->d_name;
     helper.FromString(list[i]->d_name);
     if(ext.empty())
        files.push_back(helper);
     else if(helper.filename.size()>=ext.size()&&
             helper.filename.substr(helper.filename.size()-ext.size())==ext)
        files.push_back(helper);
     free(list[i]);
     list[i]=NULL;
    }//end for i
 free(list);
 return true;
}
//-------------------------------------------------------------------------------------------------

void ToLower(string& str)
{transform(str.begin(),str.end(),str.begin(),static_cast<int(*)(int)>(std::tolower));}
//-------------------------------------------------------------------------------------------------

struct timeval BeginTiming()
{
 struct timeval bt;
 gettimeofday(&bt,NULL);
 return bt;
}
//-------------------------------------------------------------------------------------------------

double EndTiming(const struct timeval& bt)
{
 struct timeval et;
 double timeuse=0;
 gettimeofday(&et,NULL);
 timeuse=et.tv_sec-bt.tv_sec+double(et.tv_usec-bt.tv_usec)/1000000;
 return timeuse;
}
//-------------------------------------------------------------------------------------------------

char* EventType2String(int event_type)
{
 char* et=NULL;
 switch(event_type)
       {
        case IN_CREATE:
               et="create";
               break;
        case IN_DELETE:
               et="delete";
               break;
        case IN_DELETE_SELF:
               et="delete_self";
               break;
        case IN_MODIFY:
               et="modifying";
               break;
        case IN_CLOSE_WRITE:
               et="modified";
               break;
        case IN_MOVE_SELF:
               et="move_self";
               break;
        case IN_MOVED_FROM:
               et="moved_from";
               break;
        case IN_MOVED_TO:
               et="moved_to";
               break;
        default:
               et="unknown";
               break;
       };//end switch
  return et;
}
//-------------------------------------------------------------------------------------------------

int String2EventType(const string& str)
{
 if(str.empty())
    return 0;
 if(str=="create")
    return IN_CREATE;
 if(str=="delete")
    return IN_DELETE;
 if(str=="delete_self")
    return IN_DELETE_SELF;
 if(str=="modifying")
    return IN_MODIFY;
 if(str=="modified")
    return IN_CLOSE_WRITE;
 if(str=="move_self")
    return IN_MOVE_SELF;
 if(str=="moved_from")
    return IN_MOVED_FROM;
 if(str=="moved_to")
    return IN_MOVED_TO;
 return 0;
}
//-------------------------------------------------------------------------------------------------

string ExtractFilename(const string& fullfilename)
{
 int pos=fullfilename.rfind('/');
 if(pos==int(string::npos))
    return fullfilename;
 if(pos>=int(fullfilename.size()-1))
    return "";
 return fullfilename.substr(pos+1);
}
//-------------------------------------------------------------------------------------------------

string GetParentPath(string path)
{
 if(path.empty())
    return "";
 while(!path.empty()&&path[path.size()-1]=='/')
       path.erase(path.size()-1,1);
 if(path.empty())
    return "";
 int pos=path.rfind('/');
 if(pos==int(string::npos))
    return "./";
 return path.substr(0,pos+1);
}
//-------------------------------------------------------------------------------------------------

bool UnixSocketBlockSend(int fd,const char* buffer,unsigned int total_size)
{
 unsigned int rv=send(fd,buffer,total_size,0);
 if(rv!=total_size)
    return false;
 return true;
}
//-------------------------------------------------------------------------------------------------

bool NonblockSend(int fd,const char* buffer,unsigned int total_size)
{
 unsigned int ss=total_size;
 unsigned int cp=0;
 int retry=0;
 while(ss>0)
      {
       int rv=send(fd,buffer+cp,ss,0);
       if(rv>0)
         {
          ss-=rv;
          cp+=rv;
         }
       else
         {
          if((errno==EAGAIN||errno==EINTR||errno==ENOBUFS)&&retry<1024)
            {
             retry++;
             continue;
            }
          else
            {
             printf("%s\tERROR: nonblock send failed %d\n",GetCurrentTime().c_str(),errno);
             break;
            }
         }
      }//end while
 if(ss==0)
    return true;
 return false;
}
//-------------------------------------------------------------------------------------------------

string StateToString(int state)
{
 switch(state)
       {
        case STATE_NULL:
             return "STATE_NULL";
        case STATE_TO_MASTER_WAIT_SLAVE_OK:
             return "STATE_TO_MASTER_WAIT_SLAVE_OK";
        case STATE_MASTER:
             return "STATE_MASTER";
        case STATE_MASTER_TO_SLAVE1:
             return "STATE_MASTER_TO_SLAVE1";
        case STATE_MASTER_TO_SLAVE2:
             return "STATE_MASTER_TO_SLAVE2";
        case STATE_SLAVE:
             return "STATE_SLAVE";
        default:
             break;
       }//end switch
 return "";
}
//-------------------------------------------------------------------------------------------------

string BuildHTMLResult(const Conf& conf,int state,
                       const WatcherStatus& ws,const SenderStatus& ss)
{
 string content;
 content+=StateToString(state)+"<br>";
 content+="<br>";
 content+="watch_wd_number:"+ULLToStr(ws.watch_wd_number)+"<br>";
 content+="safe:"+ws.safe+"<br>";
 content+="current_watcher_write_rlog_filename:"+ws.current_watcher_write_rlog_filename+"<br>";
 content+="current_watcher_write_rlog_offset:"+IntToStr(ws.current_watcher_write_rlog_offset)+"<br>";
 content+="<br>";
 content+="sent_file_number:"+ULLToStr(ss.sent_file_number)+"<br>";
 content+="current_sender_read_rlog_filename:"+ss.current_sender_read_rlog_filename+"<br>";
 content+="current_sender_read_rlog_offset:"+IntToStr(ss.current_sender_read_rlog_offset)+"<br>";
 return content;
}
//-------------------------------------------------------------------------------------------------