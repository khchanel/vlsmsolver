/*********************************************************
 * vlsm.h  --- Library header of VLSM Solver program
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

#ifdef __cplusplus
extern "C" {
#endif 

#ifndef VLSM_H
#define VLSM_H
#define VLSM_VERSION "v1.2.1"

#define IPV4_DOTLEN 4
#define IPV4_BITLEN 32
#define IPV4_STRLEN 16

typedef   unsigned char     ipv4_t        [IPV4_DOTLEN];
typedef   char              ipv4str_t     [IPV4_STRLEN];
typedef   char              networkstr_t  [20];  


typedef struct 
{
  ipv4_t            addr;
  unsigned char     mask;
} network_t;

/**** Function prototypes ****/

/**
 * convert (ipv4_t)addr into (ipv4str_t) ipstr
 */
void                  ipv4tostr           (ipv4str_t          ipstr,
                                           const ipv4_t        addr);
                                           
                                                                                           
/**
 * convert ipv4str_t into ipv4_t
 */
int                   strtoipv4           (ipv4_t             addr,
                                           const ipv4str_t    ipstr);
                                           
                                                                                       
/**
 * print network information to stdout
 */
void                  print_network       (network_t        * network);



/* initialize @addr with the given ip {a,b,c,d}
 */
void                  makeipv4            (ipv4_t             addr,
                                           unsigned char      a,
                                           unsigned char      b,
                                           unsigned char      c,
                                           unsigned char      d);
                                               
/**
 * calculate and return the number of usable hosts of a network with @mask 
 * Return value: number of usable hosts
 */
unsigned long         caluhosts           (unsigned char      mask);



/**
 * calculate and RETURN the number of addressable hosts (include net addr & broadcast)
 * of a network with @mask
 */
unsigned long         calahosts           (unsigned char      mask);



/**
 * calculate and RETURN the subnet mask in slash form (without the slash)
 * that is required to address @nhosts number of hosts in the network
 * @mask is used to determine whether or not it is possible to address @nhosts
 * in the base network. pass 0 to @mask to make it ignore such consideration
 */
unsigned char         calmask             (unsigned long      nhosts,
                                           unsigned char      mask);
                                           
                                           
                                               
/**
 * calcuate the broadcast address of the given network (@network_addr & @mask)
 * and store to @broadcast
 */
void                  calbroadcast        (ipv4_t             broadcast,
                                           ipv4_t             network_addr,
                                           unsigned char      mask);
                                           
                                           
                                           
/**
 * calcuate the first address of the given network (@network_addr & @mask)
 * and store to @first_host_addr
 */                                              
void                  calfirst            (ipv4_t             first_host_addr,
                                           ipv4_t             network_addr,
                                           unsigned char      mask);
                                           
                                           
                                           
/**
 * calcuate the last address of the given network (@network_addr & @mask)
 * and store to @last_host_addr
 */                                                            
void                  callast             (ipv4_t             last_host_addr,
                                           ipv4_t             network_addr,
                                           unsigned char      mask);
                                           
                                           

/**
 * advance @addr by @nhosts. 
 * e.g. when @addr={192,168,1,0}, @nhosts=512; @addr will become {192,168,3,0}
 * Return 0 if fail, non-zero if successful
 */                                            
int                   ipv4add             (ipv4_t             addr,
                                           unsigned long      nhosts);
                                           
                                           
                             
/**
 * copy content of @src to @dst 
 */
void                  ipv4cpy             (ipv4_t             dst,
                                           const ipv4_t       src);
                                           
                                           

/**
 * convert net mask @smask in slash form/(24) to @dmask dot form {255,255,255,0}
 */
void                  masktodot           (ipv4_t             dmask,
                                           unsigned char      smask);
                                           
                                           
                                           
/**
 * convert net mask @dmask (dot form) to slash form and return it
 * Return: value of net mask in slash form, 0 for invalid mask
 */                                            
unsigned char         masktoslash         (ipv4_t             dmask);


/**
 * find the network address of an ipv4 address 
 */
void                  ipv4tonet           (ipv4_t                 netaddr,
                                           const ipv4_t           addr,
                                           const unsigned char    smask);


/**
 * initialize @network with @addr & @mask, assume @network has been allocated
 */
void                  makenetwork         (network_t            * network,
                                           const ipv4_t           addr,
                                           const unsigned char    mask);
                                           
                                           
                                           
/**
 * perform subneting calculation with given parameters
 * store an array of network_t to @subnets
 * assuming @subnets has been allocated with memory sizeof(network_t)*arrlen
 * Return: 
 *   >=0 : Successful
 *   -1  : invalid net_mask
 *   -2  : too many or no host to address for the given network
 */
int                   vlsm                (network_t            * subnets,
                                           const ipv4_t           net_addr,
                                           const unsigned char    net_mask,
                                           const unsigned long  * nhosts_arr,
                                           const int              arrlen);

#endif

#ifdef __cplusplus
}
#endif
