#ifndef __DB_CONTROL_SQLITE_H
#define __DB_CONTROL_SQLITE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pwd.h>
#include "naegling-main.h"

#define QUERY_SIZE 256


enum{
        DEVICE_READY,
        COPYING,
};

enum {
	JOB_STOPED,
	JOB_EXECUTING,
	JOB_DONE
};
	

/*
 * Function to create database.
 * RETURNS:
 * 	0 Successfully created database.
 *	-1 Error creating database.
 */
int create_db();



/*
 * Function to check wether the database exists.
 * RETURNS:
 * 	0 Database does not exist.
 *	1 Database exists.
 */
int db_exists();




/*
 * Function to create the devices table called by create_db function;
 * PARAMETERS:
 * 	1-handle: Pointer to the sqlite3 database.
 */
void create_device_table(sqlite3 *handle);




/*
 * Function to create the templates table called by create_db function;
 * PARAMETERS:
 *      1-handle: Pointer to the sqlite3 database.
 */
void create_template_table(sqlite3 *handle);



/*
 * Function to create the dhcp table called by create_db function;
 * PARAMETERS:
 *      1-handle: Pointer to the sqlite3 database.
 */
void create_dhcp_table(sqlite3 *handle);



/*
 * Function to create the cluster network table called by create_db function;
 * PARAMETERS:
 *      1-handle: Pointer to the sqlite3 database.
 */
void create_cluster_network_table(sqlite3 *handle);


/*
 * Function to create the job status table called by create_db function;
 * PARAMETERS:
 *      1-handle: Pointer to the sqlite3 database.
 */
void create_job_status_table(sqlite3 *handle);

/*
 * Function to insert row into device_table;
 * PARAMETERS:
 *	1-path: device path
 * 	2-domain: device's domain 
 *	3-status: device status 0-ready, 1-copying
 */
void insert_into_device_table(const char *path,const char *domain, int status);




/*
 * Function to insert row into template_table;
 * PARAMETERS:
 *	1-name: Template name
 * 	2-path: Template path 
 *	3-md5: template md5sum
 */
void insert_template_table(const char *name,const char *path, const char *md5);



/*
 * Function to insert row into dhcp_table;
 * PARAMETERS:
 *	1-mac: MAC address
 *      2-domain: device's domain 
 * RETURNS:
 *	string with the ip address.
 *	string with "-1" on error.
 */
char* insert_into_dhcp_table(const char *mac, const char *domain);



/*
 * Function to insert row into dhcp_table;
 * PARAMETERS:
 *	1-ip:IP address used by domain in this network
 *      2-domain: device's domain 
 *      3-mac: MAC address
 */
void insert_into_cluster_network_table(const char *ip, const char *domain,const char *mac);



/*
 * Function to insert row into job_status_table;
 * PARAMETERS:
 *      1-name:Job's name.
 *      2-master_domain: Job's master domain name.
 *	3-status: Job's status .
 */
void insert_into_job_status_table( const char *name,  const char *master_domain, int status);

/*
 * Function to read database for master nodes count
 * PARAMETERS:
 *	1-table: Table name in database.
 * RETURNS:
 *	int number of rows in table.
 */
int get_table_count(const char *table);

/*
 * Function to verify if the template exists locally.
 * PARAMETERS:
 *	1-name: template name.
 * RETURNS:
 *	1-if exists.
 *	0-if does not exist.
 *	-1-Error.
 */
int template_exists(const char *name);


/*
 * Function to verify the status of a device.
 * PARAMETERS:
 *	1-domain: Device's domain name.
 * RETURNS:
 *	-1: Query error.
 *	DEVICE_READY
 *	COPYING
 */
int get_device_status(const char* domain);



/*
 * Updates the device status
 * PARAMETERS:
 *	1-path:device's path.
 *	2-status: device's status to be set.
 */
void update_device_status(const char *path, int status);


/*
 * Updates job_status_table status
 * PARAMETERS:
 *	1-name:Job's name.
 *	2-master_domain: domain of the job's master node.
 *	3-status: job's status to be set.
 */
void update_job_status(const char *name, const char * master_domain, int status);



/*
 * Function to return first free IP address from database.
 * RETURNS:
 *	int with the ip. can be higher than 254.
 *	0 error.
 */
int get_free_ip();


/*
 * Integer comparison used by qsort
 */
int comp(const void *a,const void *b);


/*
 * Function to find the IP address based on the MAC address.
 * PARAMETERS:
 *	1-mac: MAC address.
 * RETURNS:
 *	String with the ip or NULL when it is not found. Must be released.
 */
char * get_ip_by_mac(const char *mac);



/*
 * Function to find the IP address based on the domain.
 * PARAMETERS:
 *      1-mac: MAC address.
 * RETURNS:
 *      String with the ip or NULL when it is not found. Must be released.
 */
char * get_ip_by_domain(const char *domain);



/*
 * Function to find the cluster IP address based on the MAC address.
 * PARAMETERS:
 *      1-mac: MAC address.
 * RETURNS:
 *      String with the ip or NULL when it is not found. Must be released.
 */
char * get_cluster_ip_by_mac(const char *mac);



/*
 * Function to find the cluster domain address based on the MAC address.
 * PARAMETERS:
 *      1-mac: MAC address.
 * RETURNS:
 *      String with the domain or NULL when it is not found. Must be released.
 */
char * get_cluster_domain_by_mac(const char *mac);



/*
 * Function to delete all entries in database given a domain
 * PARAMETERS:
 *	1-domain:Virtual machine domain name.
 */
void delete_from_dhcp_table_by_domain(const char *domain);




/*
 * Function to delete all entries in database given a domain
 * PARAMETERS:
 *      1-domain:Virtual machine domain name.
 */
void delete_from_cluster_network_table_by_domain(const char *domain);


#endif
