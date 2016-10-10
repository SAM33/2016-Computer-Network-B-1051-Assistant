/* 
 *    Author : Sam33
 *    Date   : 2016/10/10
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <stdbool.h>
#define BUFFERSIZE 1024
int main(int argc , char **args)
{
	// parameters
    char maptype[100];
    char outputfile[100];
    memset(maptype,0x0,100);
    memset(outputfile,0x0,100);
    double latitude = 24.9708264;
    double longitude = 121.1882077;
    int zoom=14;
    int sizex=300;
    int sizey=300;
    strcpy(maptype,"roadmap");
    strcpy(outputfile,"download.png");
    char *hostname = "maps.google.com";
    char parameter[100];
    memset(parameter,0x0,100);
    snprintf(parameter,100,"/maps/api/staticmap?center=%lf,%lf&zoom=%d&size=%dx%d&maptype=%s&sensor=true",latitude,longitude,zoom,sizex,sizey,maptype);
    // DNS information
	struct hostent *dsthost = gethostbyname(hostname);
    if(dsthost==NULL){
        printf("Error-->Host not found!\n");
        return -1;
    }else{
        int i=0;
        while(dsthost->h_addr_list[i]!=NULL){
			// list servers from DNS information
            char *str=inet_ntoa(*( struct in_addr*)(dsthost->h_addr_list[i]));
            printf("host(%d):%s\n",(i+1),str);
            i++;
        }
        if(i>0){
			// select first server as target
            char *addr = inet_ntoa(*( struct in_addr*)(dsthost->h_addr_list[0]));
            int dstsocket = socket(AF_INET,SOCK_STREAM,0);
            if(dstsocket<0){
                printf("Error-->socket\n");
                return -1;
            }
			// create network address
            struct sockaddr_in dstaddr;
            memset((void*)&dstaddr,0x0,sizeof(dstaddr));
            dstaddr.sin_addr.s_addr = inet_addr(addr);
            dstaddr.sin_family = AF_INET;
            dstaddr.sin_port = htons(80);
			// connect to server
            int c = connect(dstsocket,(struct sockaddr *)&dstaddr,sizeof(dstaddr));
            if(c<0){
                printf("Error-->connect(%s)\n",addr);
                return -1;
            }
            char sendbuf[BUFFERSIZE];
            memset(sendbuf,0x0,BUFFERSIZE);
			// http://maps.google.com/maps/api/staticmap?center=24.970826,121.188208&zoom=14&size=300x300&maptype=roadmap&sensor=true
			// create HTTP request
            snprintf(sendbuf,BUFFERSIZE,"GET %s HTTP/1.0\r\nUser-Agent: NCU-NWLAB\r\nAccept: */*\r\nHost: maps.google.com\r\nConnection: Drop\r\n\r\n",parameter);
            printf("\n####################\nHTTP {\n%s\n}\n####################\n",sendbuf);
            // send HTTP request
			int s = write(dstsocket,sendbuf,strlen(sendbuf));
            char rcvbuf[BUFFERSIZE];
            int r;
			// open writeback (PNG file)
            FILE *f = fopen(outputfile,"wb");
            if(f==NULL){
				printf("Error-->fopen(%s)\n",outputfile);
                return -1;
            }
			// check HTTP response header
            bool first = false;
            do{
                memset(rcvbuf,0x0,BUFFERSIZE);
				// recv server's data
                r = read(dstsocket,rcvbuf,BUFFERSIZE);
                printf("Info-->recv(%d)\n",r);
                if(!first){
					// find HTTP response
                    char * bufptr  = strstr(rcvbuf,"\r\n\r\n");
					if(bufptr==NULL)
				        continue;
                    first = true;	
                    // shift to end of HTTP response header					
                    int shift = bufptr-rcvbuf+strlen("\r\n\r\n");
					char header[BUFFERSIZE];
					memset(header,0x0,BUFFERSIZE);
					memcpy(header,rcvbuf,shift);
					printf("\n####################\nHTTP {\n%s\n}\n####################\n",header);
					// download map context
                    printf("Info-->fwrite(%d)\n",r-shift);
                    fwrite(rcvbuf+shift,1,r-shift,f);
                    fflush(f);
                }else{
					// download map context
                    printf("Info-->fwrite(%d)\n",r);
                    fwrite(rcvbuf,1,r,f);
                    fflush(f);
                }
            }while(r>0);
            fclose(f);
            
        }
    }
    return 0;
}
