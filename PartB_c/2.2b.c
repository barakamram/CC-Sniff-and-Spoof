#include <unistd.h>//for close()
#include<stdio.h>
#include<stdlib.h> 
#include<string.h> 
#include<arpa/inet.h> // for inet_ntoa()
#include<net/ethernet.h> //ethernet header
#include<netinet/ip.h>	//ip header



unsigned short in_cksum (unsigned short *buf, int length)
{
   unsigned short *w = buf;
   int nleft = length;
   int sum = 0;
   unsigned short temp=0;

   /*
    * The algorithm uses a 32 bit accumulator (sum), adds
    * sequential 16 bit words to it, and at the end, folds back all 
    * the carry bits from the top 16 bits into the lower 16 bits.
    */
   while (nleft > 1)  {
       sum += *w++;
       nleft -= 2;
   }

   /* treat the odd byte at the end, if any */
   if (nleft == 1) {
        *(u_char *)(&temp) = *(u_char *)w ;
        sum += temp;
   }

   /* add back carry outs from top 16 bits to low 16 bits */
   sum = (sum >> 16) + (sum & 0xffff);  // add hi 16 to low 16 
   sum += (sum >> 16);                  // add carry 
   return (unsigned short)(~sum);
}

/* ICMP Header  */
struct icmpheader {
  unsigned char icmp_type; // ICMP message type
  unsigned char icmp_code; // Error code
  unsigned short int icmp_chksum; //Checksum for ICMP Header and data
  unsigned short int icmp_id;     //Used for identifying request
  unsigned short int icmp_seq;    //Sequence number
};



//////////////////////

void send_raw_ip_packet(struct iphdr* ip)
{
    struct sockaddr_in dest_info;
    int enable = 1;
    
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);//create raw socket
    if(sock<0){
    	perror("root privileges required ");
    	exit(1);
    	}

    if(setsockopt(sock, IPPROTO_IP, IP_HDRINCL,&enable, sizeof(enable))==-1)//socket option
      {
      	perror("setsockopt");
        exit(1);
        }
                     
    //destination info
    dest_info.sin_family = AF_INET;//ip v4
    dest_info.sin_addr.s_addr = ip->daddr;
    //send the packet
    if(sendto(sock, ip, ntohs(ip->tot_len), 0, 
           (struct sockaddr *)&dest_info, sizeof(dest_info))==-1)
           {
           perror("sendto");
             exit(1);
           }
    printf("Spoofed ICMP echo request sent\n");
    close(sock);
}


int main() {
   char buffer[1500];

   memset(buffer, 0, 1500);
   
    //fill ip header
   struct iphdr *ip = (struct iphdr *) buffer;
   ip->version = 4;
   ip->ihl = 5;//
   ip->ttl = 44;
   ip->saddr = inet_addr("2.2.2.2");//spoofed src
   ip->daddr = inet_addr("10.9.0.6");//spoofed dest
   ip->protocol = IPPROTO_ICMP; 
   ip->tot_len = htons(sizeof(struct iphdr) + 
                       sizeof(struct icmpheader));
   
   //fill icmp header
   struct icmpheader *icmp = (struct icmpheader *) 
                             (buffer + sizeof(struct iphdr));
   icmp->icmp_type = 8; //ICMP Type: 8 is request, 0 is reply.

   // Calculate the checksum for integrity
   icmp->icmp_chksum = 0;//init
   icmp->icmp_chksum = in_cksum((unsigned short *)icmp, 
                                 sizeof(struct icmpheader));

  
   //send the packet
   send_raw_ip_packet (ip);

   return 0;
}
 










