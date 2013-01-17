/*********************************************************
 * vlsm.c  --- Library of VLSM Solver program
 * Copyright (C) 2011 Nelson Ka Hei Chan <khcha.n.el@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "vlsm.h"

void
ipv4tostr (ipv4str_t      ipstr,
           const ipv4_t   addr)
{
  snprintf(ipstr,IPV4_STRLEN,"%d.%d.%d.%d",addr[0],addr[1],addr[2],addr[3]);
}

int
strtoipv4 (ipv4_t           addr,
           const ipv4str_t  ipstr)
{
  int x,i;
  char tmp_str[4];
  sprintf(tmp_str,"%s","");
  /* validate ipstr */
  x=0;
  for (i=0;i<=strlen(ipstr);i++) {
    if (ipstr[i] == '.' || ipstr[i] == ',' || ipstr[i] == '\0') {
      addr[x] = (unsigned char)atoi(tmp_str);
      sprintf(tmp_str,"%s","");
      x++;
    } else if (isdigit(ipstr[i]) == 0) {
      makeipv4(addr,0,0,0,0);
      return 0;
    } else {
      if (x == 4) return 0;  /* fail because we have > 3 dots */
      sprintf(tmp_str,"%s%c",tmp_str,ipstr[i]);
      if (strlen(tmp_str) > 3 ) {
        makeipv4(addr,0,0,0,0);
        return 0;
      }
    }
  }
  if (x != 4 ) {
    makeipv4(addr,0,0,0,0);
    return 0;
  }
  return 1;
}


void
print_network (network_t * network)
{
  /* format: addr/smask [uhosts] dmask first_addr last_addr broadcast  */
  ipv4str_t tmp_str;
  ipv4_t tmp_addr;
  ipv4_t dmask;
  ipv4cpy(tmp_addr,network->addr);

  /* "address" */
  ipv4tostr(tmp_str,network->addr);
  printf("%s",tmp_str);
  /* "/dmask " */
  printf("/%d ",network->mask);
  /* "dmask" */
  masktodot(dmask,network->mask);
  ipv4tostr (tmp_str,dmask);
  printf("(%s) | ",tmp_str);
  /* "first_addr" */
  calfirst(tmp_addr,network->addr,network->mask);
  ipv4tostr(tmp_str,tmp_addr);
  printf("%s | ",tmp_str);
  /* "last_addr" */
  callast(tmp_addr,network->addr,network->mask);
  ipv4tostr(tmp_str,tmp_addr);
  printf("%s | ",tmp_str);
  /* "broadcast\n" */
  calbroadcast(tmp_addr,network->addr,network->mask);
  ipv4tostr(tmp_str,tmp_addr);
  printf("%s ",tmp_str);
  /* "[uhosts]" */
  printf("[%ld]\n",caluhosts(network->mask));
}


void
makeipv4 (ipv4_t           addr,
          unsigned char    a,
          unsigned char    b,
          unsigned char    c,
          unsigned char    d )
{
  addr[0]=a;
  addr[1]=b;
  addr[2]=c;
  addr[3]=d;
}


unsigned long
caluhosts (unsigned char mask)
{
  /*if (mask < 8 || mask > 30) return 0;*/
  if (mask > 32 ) return 0;
  else return (unsigned long)(pow(2,IPV4_BITLEN-mask) -2);
}

unsigned long
calahosts (unsigned char mask)
{
  /*if (mask < 8 || mask > 30) return 0;*/
  if (mask > 32) return 0;
  else return (unsigned long) ( pow(2,IPV4_BITLEN-mask));
}


/*
 * This function determine and return the value of net mask for the subnet
 * with nhosts return 0 if fail to address
 */
unsigned char
calmask (unsigned long    nhosts,
         unsigned char    given_mask)
{
  unsigned char mask=0;
  /* check whether it is possible to address nhosts in the subnet */
  if ( (unsigned long)(pow(2,IPV4_BITLEN-given_mask)-2) < nhosts
    || nhosts == 0)
  {
    return 0;
  }

  /* determine the required net mask value */
  unsigned char mtable_size = IPV4_BITLEN-given_mask+1;
  int i;
  unsigned long tmp;
  for (i=0;i<mtable_size;i++) {
    tmp = (unsigned long)pow(2,i);
    if ((tmp)  > 2 && tmp -2  >= nhosts) {
      mask = given_mask + (mtable_size - i - 1);
      break;
    }
  }
  return mask;
}

void
calbroadcast (ipv4_t          broadcast,
              ipv4_t          network_addr,
              unsigned char   mask)
{
  ipv4cpy(broadcast, network_addr);
  ipv4add(broadcast, calahosts(mask)-1);
}


void
calfirst (ipv4_t           first_host_addr,
          ipv4_t           network_addr,
          unsigned char    mask )
{
  ipv4cpy(first_host_addr,network_addr);
  ipv4add(first_host_addr,1);
}


void
callast ( ipv4_t           last_host_addr,
          ipv4_t           network_addr,
          unsigned char    mask)
{
  ipv4cpy(last_host_addr,network_addr);
  ipv4add(last_host_addr,caluhosts(mask));
}


/**
 * advance given ipv4 address by nhosts
 * return 0 if fail, non-zero if successful
 */
int
ipv4add ( ipv4_t          addr,
          unsigned long   nhosts)

{
  int i;
  unsigned long max=0;
  unsigned long x=nhosts;

  /* check whether it is possible to advance addr by nhosts
   * add current number of hosts addressed to nhosts for easier arithmetics later
   * remarks: 10.0.1.4  = 10*2^24 + 0*2^16 + 1*2^8 + 4*2^0 hosts addressed
   */
  for (i=0;i<IPV4_DOTLEN;i++) {
    max += (255-addr[i]) * pow(2,(3-i)*8);
    nhosts += addr[i] * pow(2,(3-i)*8);
  }
  if (x > max) return 0;

  /* carry out calculations
   * remarks: imagine addr to be {0,0,0,0} after we get the new nhosts
   * so we could simply calculate by / and % ...eh just read the code
   */
  for (i=0;i<IPV4_DOTLEN;i++) {
    x = (unsigned long)(pow(2,(3-i)*8));
    addr[i] = nhosts / x;
    nhosts = nhosts % x;
  }
  return 1;
}


/* copy ipv4 address src to address dst */
void
ipv4cpy ( ipv4_t        dst,
          const ipv4_t  src)
{
  int i;
  for (i=0;i<IPV4_DOTLEN;i++) {
    dst[i] = src[i];
  }
}


/**
 * convert net mask in slash form (e.g. /25) to dot form (255.255.255.128)
 * behaviour is undefined if smask > 32
 */
void
masktodot ( ipv4_t          dmask,
            unsigned char   smask)

{
  ipv4_t addr = {0,0,0,0};
  ipv4add (addr, pow(2,32)-pow(2,32-smask) ); /* 2^32 - 2^host_bit */
  ipv4cpy (dmask,addr);
}


/* convert mask in dot form to slash form. invalid dmask is treated as 0 */
unsigned char
masktoslash ( ipv4_t dmask )
{
  int i,j;
  unsigned char map[9] = {0,128,192,224,240,248,252,254,255};
  unsigned char smask=0;
  for (i=0;i<IPV4_DOTLEN;i++) {
    for(j=0;j<9;j++) {
      if (dmask[i] == map[j] ) {
        smask += j;
      }
    }
  }
  return smask;
}

/**
 * get the network address of a given address/mask
 */
void ipv4tonet (ipv4_t                netaddr,
                const ipv4_t          addr,
                const unsigned char   smask)
{
  int i;
  ipv4_t dmask;

  masktodot (dmask, smask);
  for (i=0;i<IPV4_DOTLEN;i++) {
    netaddr[i] = addr[i] & dmask[i]; // AND-ing operations
  }
}


void
makenetwork ( network_t               * network,
              const                     ipv4_t addr,
              const unsigned char       mask)
{
  ipv4cpy(network->addr,addr);
  network->mask = mask;
}


/* address an array of "network" of representing subnets by VLSM
 * assume "*subnets" has "sizeof(network_t)*arrlen "
 * return :
 *     >=0: successful & the number of subnets
 *     -1: invalid net_mask
 *     -2: too many or no host to address for the given network
 *TODO: detect(ignore?) cases where request no. of host to subnet is 0
 */
int
vlsm ( network_t              * subnets,
       const ipv4_t             net_addr,
       const unsigned char      net_mask,
       const unsigned long    * nhosts_arr,
       const int                arrlen )
{
  int i;

  /* some checking */
  {
    if (arrlen == 0 ) return 0;
    if (net_mask > 30 || net_mask <= 0) return -1;
    unsigned long sum=0;
    for (i=0;i<arrlen;i++) {
      sum  += nhosts_arr[i];
    }
    if (calmask(sum,net_mask) == 0 ) return -2;
  }

  /* Process */
  makenetwork(&subnets[0],net_addr, calmask(nhosts_arr[0],net_mask) );
  for (i=1;i<arrlen;i++) {
    unsigned char sub_mask = calmask(nhosts_arr[i],net_mask);
    ipv4_t        sub_addr;
    ipv4cpy(sub_addr,subnets[i-1].addr);
    ipv4add(sub_addr,calahosts(subnets[i-1].mask));

    makenetwork(&subnets[i],sub_addr, sub_mask);
  }
  return i;
}

