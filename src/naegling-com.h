#ifndef __NAEGLING_COM_H
#define __NAEGLING_COM_H

#include <stdio.h>
#include <unistd.h>
#include <syslog.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "naegling-main.h"


enum {
	START_NODE=1,
	STOP_NODE,
	CREATE_MASTER_NODE,
	CREATE_SLAVE_NODE,
	NODE_STATUS,
	TEMPLATE_STATUS,
	START_MASTER_VIRTUAL_NODE,
	STOP_MASTER_VIRTUAL_NODE,
	START_NEW_CLUSTER,
	ADD_WORKING_NODE,
	REMOVE_WORKING_NODE,
	GET_CLUSTER_STATUS,
	REQUEST_CLUSTER_IP,
	REQUEST_TEMPLATE_TRANSFER,
	REQUEST_JOB_TRANSFER,	
	EXECUTE_JOB,
	DOWNLOAD_JOB_FILE,
	DELETE_NODE,
	EDIT_NODE
};

/*
 * Function to log error using syslog. ident->naegling_error.log
 * PARAMETERS:
 * 	1-on_what: string with motive from failure. 
 */
void bail(const char *on_what);



/*
 * Function to log using syslog. ident->naegling_naegling.log
 * PARAMETERS: 
 *      1-message: string with the log message. 
 */
void naegling_log(const char *message);




/*
 * Function to create a virtual machine using libvirt and hardcoded xml file on a remote host.
 * PARAMETERS:
 *	1-ip: Remote host ip.
 *      2-hypervisor: URI of the hipervisor(driver:///system).           
 *      3-vdisk_path: Path to the virtual image.
 *      4-name: Virtual machine domain name.
 *      5-memory: Ram memory size.
 *      6-cpu: CPU cores quantity.
 */
void listen_for_remote_message();



/*
 * Function to deal with multiple connection with pthreads
 * PARAMETERS:
 * 	1-socket_desc: client socket descriptor(integer)
 */
void connection_handler(int session_fd);



/*
 * Function to count the number of fields from a message
 */
int get_field_count(char *message);


/*
 * Function to split the message between its fields.
 * PARAMETERS:
 *	1-message: String with message.
 *	2-message_fields: bidimensional array to store the fiels. 
 */
void get_message_fields(char *message,char **message_fields);


/*
 * Function to prepare for file transfer
 * PARAMETERS:
 *	1-file_name
 * RETURNS:
 *	-1 error
 *	0 success.
 */
int prepare_template_transfer(const char *file_name);




/*
 * Function to prepare for job file transfer
 * PARAMETERS:
 *	1-master_domain: domain name from job's master node
 *	2-master_hypervisor: job's master node hypervisor
 *	3-master_mac:job's master node mac address
 *      4-file_name
 *	5-sock: connection socket
 * RETURNS:
 *      -1 error
 *      0 success.
 */
int prepare_job_file_transfer(const char *master_domain,const char *master_hypervisor, const char *master_mac, const char * job_name,const char *file_name,const int sock);


/*
 * Function to transfer files between server and gui modules
 */
//void* file_transfer(void* arg);




//void *job_file_transfer(void *arg);



/*
 * Function to send comands to a cluster.
 * PARAMETERS:
 *	1-cluster_ip_addr: Cluster master node IP address.
 *	2-GUI_message: Message to be send. Fields must be separeted by '#'
 */
int send_message_to_cluster(const char *cluster_ip_addr, const char *GUI_message);

/*
 * Returns the cluster status
 * PARAMETERS:
 *	-1 error
 *	0 cluster ready
 *	1 cluster not ready
 */
int get_cluster_status(const char *master_mac);



/*
 * Function to get the master virtual machine cluster_ip 
 * PARAMETERS:
 *	1-mac: Cluster interface MAC address.
 * RETURNS:
 *	NULL: error.
 *	const char *: Virtual machine's IP address.
 */
char* get_cluster_ip(const char *mac);


/*
 * Adds a new diskless working node to cluster
 * PARAMETERS:
 *	1-domain: working node's domain.
 *	2-mac: working node's MAC address.
 *	3-master_domain: cluster master node's MAC address.
 *      4-master_hypervisor: master hypervisor type
 *      5-master_mac: master's virNaegling MAC address.
 * RETURNS
 *	-1 on error
 *	0 success.
 */
int add_working_node(const char *domain,const char *mac,const char *master_domain,const char *master_hypervisor,const char *master_mac);



/*
 * Removes a diskless working node from the cluster
 * PARAMETERS:
 *      1-domain: working node's domain.
 *      2-master_domain: cluster master node's MAC address.
 *	3-master_hypervisor: master hypervisor type
 *	4-master_mac: master's virNaegling MAC address.
 * RETURNS
 *      -1 on error
 *      0 success.
 */
int remove_working_node(const char *domain,const char *master_domain,const char *master_hypervisor,const char *master_mac);




/*
 * Send execution message to cluster
 * PARAMETERS:
 *      1-master_domain: cluster master node's MAC address.
 *	2-job_name
 *	3-script_name
 * RETURNS
 *      -1 on error
 *      0 success.
 */
int run_job_script(char * master_domain, char *job_name, char *script_name);





int download_cluster_file(const char * master_domain, const char *path,const int GUI_sock);
void *server_gui_job_file_transfer(void *arg);

#endif
