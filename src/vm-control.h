#ifndef __VM_CONTROL_H
#define __VM_CONTROL_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <libvirt/libvirt.h>
#include <openssl/md5.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include "db-control-sqlite.h"
#include "naegling-main.h"



/*
 * Function to create a virtual machine using libvirt and hardcoded xml file.
 * Parameters:
 *	1-domain: Virtual machine domain name.
 *	2-template: Cluster template name.
 *	3-vdisk_path: Path to the virtual image.
 *	4-uuid: Domain UUID
 *	5-mac: Bridge network MAC address.
 *	6-bridge_network_interface. 
 *	7-naegling_mac: virNaegling network MAC address.
 * 	8-hypervisor: URI of the hipervisor(driver:///system). 		 
 *	9-ram_memory: Ram memory size.
 *	10-cpu_quantity: CPU cores quantity.
 *	11-vnc_port: vnc port used for graphical access.
 * Returns:
 *	0 Machine successfully created
 *	-1 Error creating machine
 */
int create_master_vm(const char *domain,const char *template,const char *vdisk_path,const char *uuid,const char *mac,const char *bridge_network_interface,const char *naegling_mac,const char *hypervisor, const char *ram_memory, const char * cpu_quantity,const char * vnc_port);



/*
 * Function to create a diskless virtual machine using libvirt and hardcoded xml file.
 * Parameters:
 *	1-domain: Virtual machine domain name.
 *	2-uuid: Domain UUID.
 *	3-mac: Bridge network MAC address.
 *	4-bridge_network_interface. 
 * 	5-hypervisor: URI of the hipervisor(driver:///system). 		 
 *	6-ram_memory: Ram memory size.
 *	7-cpu_quantity: CPU cores quantity.
 *	8-vnc_port: vnc port used for graphical access.
 * Returns:
 *	0 Machine successfully created
 *	-1 Error creating machine
 */
int create_diskless_slave_vm(const char *domain,const char *uuid,const char *mac,const char *bridge_network_interface,const char *hypervisor, const char *ram_memory, const char *cpu_quantity,const char *vnc_port);



/*
 * Function to check the status of the virtual machine
 * Parameters:
 * 	1-domain: Virtual machine domain name
 * 	2-hypervisor: URI of the hipervisor(driver:///system). 		 
 * Returns:
 * 	-1 error
 *	0 inactive
 * 	1 running
 */
int vm_status(const char *domain,const char *hypervisor);



/*
 * Function to start the virtual master machine
 * Parameters:
 *      1-domain: Virtual machine domain name
 *      2-hypervisor: URI of the hipervisor(driver:///system).
 *      3-mac: MAC address.
 * Returns:
 *      -1 error
 *      0 success
 */
int vm_start_master_virtual_node(const char *domain, const char *hypervisor, const char *mac,const char *cluster_mac,const char *cluster_ip);



/*
 * Function to stop the virtual master machine
 * Parameters:
 *      1-domain: Virtual machine domain name
 *      2-hypervisor: URI of the hipervisor(driver:///system).
 * 	3-mac: MAC address.
 * Returns:
 *      -1 error
 *      0 success
 */
int vm_stop_master_virtual_node(const char *domain, const char *hypervisor, const char *mac);



/*
 * Function to start the virtual diskless slave machine
 * Parameters:
 *      1-domain: Virtual machine domain name
 *      2-hypervisor: URI of the hipervisor(driver:///system).
 *      3-mac: MAC address.
 *	4-master_mac: MAC address of this diskless node master machine.
 * Returns:
 *      -1 error
 *      0 success
 */
int vm_start_slave_virtual_diskless_node(const char *domain,const char *hypervisor,const char *mac,const char *master_mac);



/*
 * Function to start the virtual machine
 * Parameters:
 *      1-domain: Virtual machine domain name
 *      2-hypervisor: URI of the hipervisor(driver:///system).
 * Returns:
 * 	-1 error
 * 	0 success
 */
int vm_start(const char *domain,const char *hypervisor);



/*
 * Function to stop the virtual machine
 * Parameters:
 *      1-domain: Virtual machine domain name
 *      2-hypervisor: URI of the hipervisor(driver:///system).
 * Returns:
 * 	-1 error
 * 	0 success
 */
int vm_stop(const char *domain,const char *hypervisor);



/*
 * Function to create the naegling virtual network 
 * Returns:
 *      -1: error creating network
 *      0: network created and started successfully.
 */
int create_virtual_network_naegling();



/*
 * Function to start the naegling virtual network 
 * Returns:
 *      -1: error starting network
 *      0: network started successfully.
 */
int start_virtual_network_naegling();



/*
 * Function to stop the naegling virtual network 
 * Returns:
 *      -1: error stopping network
 *      0: network stopped successfully.
 */
int stop_virtual_network_naegling();



/*
 * Function to get the naegling virtual network ip
 * PARAMETERS:
 *	1-mac: Domain's naegling virtual network MAC address.
 * Returns:
 *      char * with vir naegling IP. Use free to release resource.
 *      NULL: IP not found.
 */
char * get_vir_naegling_ip(const char *mac);



/*
 * Function to calculate the MD5 hash of a file using libssl.
 * PARAMETERS:
 *      1-file_path
 * 	2-result: Char pointer to store the result. Memory needs to be allocated. EX.:char* result=(char*)malloc(sizeof(char)*MAX_MD5_SIZE);
 */
void getMD5(const char* file_path, char* result);



/*
 * Function to copy large files locally
 * PARAMETERS:
 *	1-src_path: path to the source file.
 *	2-dest_path: path to the destination file.
 * RETURNS:
 *	-1: error.
 *	>=0: Size of the file copied in MegaBytes.
 */
int copy_large_file(const char* src_path, const char* dest_path);



/*
 * Function to add a mac to the libvirt(dnsmasq) dhcp
 * PARAMETERS:
 *	1-ip: IP address.
 *	2-domain: Virtual machine's domain name.
 *	3-mac: Virtual machine's MAC address.
 * RETURNS:
 *	String with the MAC address if successful.
 *	String with -1 if all available IPs are being used.
 *	NULL if there is an error. 
 */
const char * add_mac_to_dhcp(const char *ip,const char *domain,const char *mac);



/*
 * Function to remove a mac from the libvirt(dnsmasq) dhcp
 * PARAMETERS:
 *      1-domain: Virtual machine's domain name.
 *      2-mac: Virtual machine's MAC address.
 * RETURNS:
 *      0 if successful.
 *      -1 if there is an error. 
 */
int remove_mac_from_dhcp(const char *ip,const char *domain,const char *mac);



/*
 * Function to undefine a domain.
 * PARAMETERS:
 *      1-domain: Virtual machine's domain name.
 *      2-hypervisor: URI of the hipervisor(driver:///system).
 * RETURNS:
 *      0 if successful.
 *      -1 if there is an error. 
 */
int undefine_vm(const char *domain,const char *hypervisor);



/**
 * Function to insert a string between first ocurrency of 'before' and 'after' replacing anything in between.
 * PARAMETERS:
 *	1-before: String marker before the text to be inserted.
 *	2-after: String marker after the text to be inserted.
 * 	3-text: original text.
 *	4-insert_text: string to be inserted 
 * RETURNS:
 *	0 if successful.
 *	-1 if there is an error.
 */
int insert_string(const char * before, const char *after,char **text,const char* insert_text);



/*
 * Function to edit a domain.
 * PARAMETERS:
 *      1-domain: Virtual machine's domain name.
 *      2-hypervisor: URI of the hipervisor(driver:///system).
 *	3-memory
 *	4-cpu
 *	5-vnc_port
 * RETURNS:
 *      0 if successful.
 *      -1 if there is an error. 
 */
int edit_vm(const char *domain,const char *hypervisor,const char *memory, const char *cpu, const char *vnc_port);



/*
 * get_virNaegling_ip auxiliary function. Deals with string manipulation
 * PARAMETERS:
 *      1-ip: address of pointer to store result(char *).
 *      2-xml: virNaegling xml definition where to look for ip.
 *      3-mac: domain mac address
 *      4-domain: Virtual machine's domain name.
 * RETURNS:
 *      0 if successful.
 *      -1 if there is an error. 
 */
int get_virNaegling_ip_aux(char **ip,char *xml,const char *mac,const char *domain);



/*
 * Query libvirt network 'virNaegling' for ip associated with MAC address.
 * PARAMETERS:
 *      1-domain: Virtual machine's domain name.
 *      2-hypervisor: URI of the hipervisor(driver:///system).
 *      3-mac
 * RETURNS:
 *      char * if successful. Needs to be freed.
 *      NULL if there is an error. 
 */
char *get_virNaegling_ip(const char *domain,const char *hypervisor,const char *mac);

#endif
