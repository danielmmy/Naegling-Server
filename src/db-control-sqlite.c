#include "db-control-sqlite.h"


int create_db(){
	printf("Generating database...\n");
	int retval;
	/*
	 * Create database
	 */
	sqlite3 *handle;
	retval=sqlite3_open(DATABASE_PATH,&handle);
	if(retval==SQLITE_OK){
		create_device_table(handle);
		create_template_table(handle);
		create_dhcp_table(handle);
		create_cluster_network_table(handle);
		create_job_status_table(handle);
		retval=sqlite3_close(handle);
		naegling_log("Database generated successfully.");
	}else{
		bail("Error creating database sqlite3.");
		return -1;
	}
	/*
	 * End
	 */

}

int db_exists(){
        struct stat status;
	int retval=stat(DATABASE_PATH,&status);
	return (retval)?0:1;
}

void create_device_table(sqlite3 *handle){	
	char *query;
	int retval;
	query=(char*)malloc(QUERY_SIZE);
	strcpy(query,"CREATE TABLE IF NOT EXISTS device_table(path TEXT PRIMARY KEY, domain TEXT, status INTEGER)");
	retval=sqlite3_exec(handle,query,0,0,0);
	free(query);
}

void create_template_table(sqlite3 *handle){	
	char *query;
	int retval;
	query=(char*)malloc(QUERY_SIZE);
	strcpy(query,"CREATE TABLE IF NOT EXISTS template_table(name TEXT PRIMARY KEY, path TEXT , md5 TEXT)");
	retval=sqlite3_exec(handle,query,0,0,0);
	free(query);
}


void create_dhcp_table(sqlite3 *handle){
        char *query;
        int retval;
        query=(char*)malloc(QUERY_SIZE);
        strcpy(query,"CREATE TABLE IF NOT EXISTS dhcp_table(ip TEXT PRIMARY KEY, mac TEXT , domain TEXT)");
        retval=sqlite3_exec(handle,query,0,0,0);
        free(query);
}

void create_cluster_network_table(sqlite3 *handle){
        char *query;
        int retval;
        query=(char*)malloc(QUERY_SIZE);
        strcpy(query,"CREATE TABLE IF NOT EXISTS cluster_network_table(ip TEXT PRIMARY KEY,  domain TEXT,mac TEXT)");
        retval=sqlite3_exec(handle,query,0,0,0);
        free(query);
}

void create_job_status_table(sqlite3 *handle){
        char *query;
        int retval;
        query=(char*)malloc(QUERY_SIZE);
        strcpy(query,"CREATE TABLE IF NOT EXISTS job_status_table(name TEXT ,  master_domain TEXT ,status INTEGER)");
        retval=sqlite3_exec(handle,query,0,0,0);
        free(query);
}




void insert_into_device_table(const char *path,const char *domain, int status){
        int retval;
	sqlite3 *handle;
        retval=sqlite3_open_v2(DATABASE_PATH,&handle,SQLITE_OPEN_READWRITE,NULL);
        if(retval==SQLITE_OK){
		sqlite3_stmt *stmt;
		char *query="INSERT INTO device_table(path,domain,status) VALUES(?,?,?)";
		sqlite3_prepare_v2(handle,query,strlen(query)+1,&stmt,NULL);
		retval=sqlite3_bind_text(stmt,1,path,-1,SQLITE_STATIC);
		if(retval)
			bail("Error appending value 1(path) to query\n");
		retval=sqlite3_bind_text(stmt,2,domain,-1,SQLITE_STATIC);
		if(retval)
			bail("Error appending value 2(domain) to query\n");
		retval=sqlite3_bind_int(stmt,3,status);
		if(retval)
			bail("Error appending value 3(status)  to query\n");
		retval=sqlite3_step(stmt);
		if(retval==SQLITE_DONE)
			naegling_log("Row successfully inserted");
		else
			bail("Error executing query");
		sqlite3_finalize(stmt);
                sqlite3_close(handle);
        }else{
                bail("Error openning database sqlite3.");
	}
}

void insert_into_template_table(const char *name,const char *path, const char *md5){
        int retval;
	sqlite3 *handle;
        retval=sqlite3_open_v2(DATABASE_PATH,&handle,SQLITE_OPEN_READWRITE,NULL);
        if(retval==SQLITE_OK){
		sqlite3_stmt *stmt;
		char *query="INSERT INTO template_table(name,path,md5) VALUES(?,?,?)";
		sqlite3_prepare_v2(handle,query,strlen(query)+1,&stmt,NULL);
		retval=sqlite3_bind_text(stmt,1,name,-1,SQLITE_STATIC);
		if(retval)
			bail("Error appending value 1(name) to query.");
		retval=sqlite3_bind_text(stmt,2,path,-1,SQLITE_STATIC);
		if(retval)
			bail("Error appending value 2(path) to query.");
		retval=sqlite3_bind_text(stmt,3,md5,-1,SQLITE_STATIC);
		if(retval)
			bail("Error appending value 3(mac)  to query.");
		retval=sqlite3_step(stmt);
		if(retval==SQLITE_DONE)
			naegling_log("Row successfully inserted.");
		else
			bail("Error executing query.");
		sqlite3_finalize(stmt);
                sqlite3_close(handle);
        }else{
                bail("Error openning database sqlite3.");
	}
}


char * insert_into_dhcp_table( const char *mac, const char *domain){
        int retval;
        int end_ip=get_free_ip();
        char *ret_ip=(char*)malloc(sizeof(char)*MAX_IP_SIZE);
        if(end_ip<255&&end_ip>0){
                sprintf(ret_ip,"%s%d",VIR_NAEGLING_NETWORK,end_ip);
                sqlite3 *handle;
                retval=sqlite3_open_v2(DATABASE_PATH,&handle,SQLITE_OPEN_READWRITE,NULL);
                if(retval==SQLITE_OK){
                        sqlite3_stmt *stmt;
                        char *query="INSERT INTO dhcp_table(ip,mac,domain) VALUES(?,?,?)";
                        sqlite3_prepare_v2(handle,query,strlen(query)+1,&stmt,NULL);
                        retval=sqlite3_bind_int(stmt,1,end_ip);
                        if(retval)
                                bail("Error appending value 1(ip) to query.");
                        retval=sqlite3_bind_text(stmt,2,mac,-1,SQLITE_STATIC);
                        if(retval)
                                bail("Error appending value 2(mac) to query.");
                        retval=sqlite3_bind_text(stmt,3,domain,-1,SQLITE_STATIC);
                        if(retval)
                                bail("Error appending value 3(domain)  to query.");
                        retval=sqlite3_step(stmt);
                        if(retval==SQLITE_DONE)
                                naegling_log("Row successfully inserted.");
                        else
                                bail("Error executing query.");
                        sqlite3_finalize(stmt);
                        sqlite3_close(handle);
                }else{
                        bail("Error openning database sqlite3.");
                }
        }else{
                strcpy(ret_ip,"-1");
        }
        return ret_ip;
}



void insert_into_cluster_network_table( const char *ip,  const char *domain,const char *mac){
        int retval;
        sqlite3 *handle;
        retval=sqlite3_open_v2(DATABASE_PATH,&handle,SQLITE_OPEN_READWRITE,NULL);
        if(retval==SQLITE_OK){
		sqlite3_stmt *stmt;
                char *query="INSERT INTO cluster_network_table(ip,domain,mac) VALUES(?,?,?)";
                sqlite3_prepare_v2(handle,query,strlen(query)+1,&stmt,NULL);
                retval=sqlite3_bind_text(stmt,1,ip,-1,SQLITE_STATIC);
                if(retval)
			bail("Error appending value 1(ip) to query.");
                retval=sqlite3_bind_text(stmt,2,domain,-1,SQLITE_STATIC);
                if(retval)
                        bail("Error appending value 2(domain)  to query.");
		retval=sqlite3_bind_text(stmt,3,mac,-1,SQLITE_STATIC);
                if(retval)
                        bail("Error appending value 3(mac)  to query.");	
                retval=sqlite3_step(stmt);
                if(retval==SQLITE_DONE)
                        naegling_log("Row successfully inserted.");
                else
                        bail("Error executing query.");
                sqlite3_finalize(stmt);
                sqlite3_close(handle);
	}else{
                bail("Error openning database sqlite3.");
        }
}


void insert_into_job_status_table( const char *name,  const char *master_domain, int status){
        int retval;
        sqlite3 *handle;
        retval=sqlite3_open_v2(DATABASE_PATH,&handle,SQLITE_OPEN_READWRITE,NULL);
        if(retval==SQLITE_OK){
                sqlite3_stmt *stmt;
                char *query="INSERT INTO job_status_table(name,master_domain,status) VALUES(?,?,?)";
                sqlite3_prepare_v2(handle,query,strlen(query)+1,&stmt,NULL);
                retval=sqlite3_bind_text(stmt,1,name,-1,SQLITE_STATIC);
                if(retval)
                        bail("Error appending value 1(name) to query.");
                retval=sqlite3_bind_text(stmt,2,master_domain,-1,SQLITE_STATIC);
                if(retval)
                        bail("Error appending value 2(master_domain)  to query.");
                retval=sqlite3_bind_int(stmt,3,status);
                if(retval)
                        bail("Error appending value 3(status)  to query.");
                retval=sqlite3_step(stmt);
                if(retval==SQLITE_DONE)
                        naegling_log("Row successfully inserted.");
                else
                        bail("Error executing query.");
                sqlite3_finalize(stmt);
                sqlite3_close(handle);
        }else{
                bail("Error openning database sqlite3.");
        }
}



int get_table_count(const char *table){
        int retval;
        int rows=0;
        sqlite3 *handle;
	retval=sqlite3_open_v2(DATABASE_PATH,&handle,SQLITE_OPEN_READONLY,NULL);
        if(retval==SQLITE_OK){
                char count[QUERY_SIZE];
		sprintf(count,"SELECT COUNT() FROM %s",table);
                sqlite3_stmt *stmt;
                retval=sqlite3_prepare_v2(handle,count,strlen(count)+1,&stmt,NULL);
		if(retval){
			bail("SQL error.");
		}else{
			retval=sqlite3_step(stmt);
			rows=sqlite3_column_int(stmt,0);
		}
		sqlite3_finalize(stmt);
                sqlite3_close(handle);
	}else{
                bail("Error openning database for reading sqlite3.");
        }
	return rows;
}


int template_exists(const char *name){
	sqlite3 *handle;
	int retval=sqlite3_open_v2(DATABASE_PATH,&handle,SQLITE_OPEN_READONLY,NULL);
	int exist=-1;
        if(retval==SQLITE_OK){
                char query[QUERY_SIZE];
		sprintf(query,"SELECT * FROM template_table where name='%s'",name);
                sqlite3_stmt *stmt;
                retval=sqlite3_prepare_v2(handle,query,strlen(query)+1,&stmt,NULL);
		if(retval){
			bail("SQL error: template_exists query error");
		}else{
			retval=sqlite3_step(stmt);
			if(retval==SQLITE_DONE)
				exist=0;
			else if(retval==SQLITE_ROW)
				exist=1;
		}
		sqlite3_finalize(stmt);
		sqlite3_close(handle);
	}else{
                bail("Error openning database for reading sqlite3_open_v2.");	
        }
	return exist;
}



int get_device_status(const char *domain){
	char *path=(char *)malloc(QUERY_SIZE*sizeof(char));
	path[0]='\0';
	strcpy(path,"/var/lib/libvirt/images/");
	strcat(path,domain);
	strcat(path,".img");
        sqlite3 *handle;
        int retval=sqlite3_open_v2(DATABASE_PATH,&handle,SQLITE_OPEN_READONLY,NULL);
	int status=-1;
        if(retval==SQLITE_OK){
                char query[QUERY_SIZE];
                sprintf(query,"SELECT status FROM device_table where path='%s'",path);
                sqlite3_stmt *stmt;
                retval=sqlite3_prepare_v2(handle,query,strlen(query)+1,&stmt,NULL);
                if(retval){
                        bail("SQL error: get_device_status query error");
                }else{
			retval=sqlite3_step(stmt);
			if(retval==SQLITE_ROW){
				status=(int)sqlite3_column_int(stmt,0);
			}else if(retval==SQLITE_DONE){
				status=DEVICE_READY;
			}
                                
                }
		sqlite3_finalize(stmt);
		sqlite3_close(handle);
        }else{
                bail("Error openning database for reading sqlite3_open_v2.");
        }
	free(path);
	return status;
}


void update_device_status(const char *path, int status){
        int retval;
        sqlite3 *handle;
        retval=sqlite3_open_v2(DATABASE_PATH,&handle,SQLITE_OPEN_READWRITE,NULL);
        if(retval==SQLITE_OK){
                char query[QUERY_SIZE];
		sprintf(query,"UPDATE device_table SET status=%d WHERE path='%s'",status,path);
		retval=sqlite3_exec(handle,query,0,0,0);
                sqlite3_close(handle);
        }else{
                bail("Error openning database sqlite3.");
        }
}


void update_job_status(const char *name, const char * master_domain, int status){
        int retval;
        sqlite3 *handle;
        retval=sqlite3_open_v2(DATABASE_PATH,&handle,SQLITE_OPEN_READWRITE,NULL);
        if(retval==SQLITE_OK){
                char query[QUERY_SIZE];
                sprintf(query,"UPDATE job_status_table SET status=%d WHERE name='%s' AND master_domain='%s'",status,name,master_domain);
                retval=sqlite3_exec(handle,query,0,0,0);
                sqlite3_close(handle);
        }else{
                bail("Error openning database sqlite3.");
        }
}



int get_free_ip(){
        int count=get_table_count("dhcp_table");
        int ip=0;
        if(count==0){
                ip=2;
        }else if(count>0){
                int i=0,j;
                int *list;
                list=(int*)malloc(sizeof(int)*count);
                sqlite3 *handle;
                int retval=sqlite3_open_v2(DATABASE_PATH,&handle,SQLITE_OPEN_READONLY,NULL);
                if(retval==SQLITE_OK){
                        char *query={"SELECT ip FROM dhcp_table"};
                        sqlite3_stmt *stmt;
                        retval=sqlite3_prepare_v2(handle,query,strlen(query)+1,&stmt,NULL);
                        if(retval){
                                bail("SQL error: get_used_ip_list query error.");
                        }else{
                                i=0;
                                while((retval=sqlite3_step(stmt))==SQLITE_ROW){
                                        list[i++]=(int)sqlite3_column_int(stmt,0);
                                }
                        }
                        sqlite3_finalize(stmt);
                        sqlite3_close(handle);
                        qsort(list,count,sizeof(int),comp);
                        for(i=2,j=0;j<count;i++,j++){
                                if(list[j]!=i){
                                        ip=i;
                                        break;
                                }
                        }
                        if(j==count){
                                ip=i;
                        }
                }else{
                        bail("Error openning database for reading sqlite3_open_v2.");
                }
        }
        return ip;
}


int comp(const void *a,const void *b){
	const int *ia=(const int *)a;
	const int *ib=(const int *)b;
	return *ia - *ib;
}



char* get_ip_by_mac(const char *mac){
        sqlite3 *handle;
        int retval=sqlite3_open_v2(DATABASE_PATH,&handle,SQLITE_OPEN_READONLY,NULL);
        int end_ip=0;
        char *ip=(char*)malloc(sizeof(char)*MAX_IP_SIZE);
        if(retval==SQLITE_OK){
                char count[QUERY_SIZE];
                sprintf(count,"SELECT ip FROM dhcp_table WHERE mac='%s'",mac);
                sqlite3_stmt *stmt;
                retval=sqlite3_prepare_v2(handle,count,strlen(count)+1,&stmt,NULL);
                if(retval){
                        bail("SQL error: get_ip_by_mac");
                }else{
                        retval=sqlite3_step(stmt);
                        if(retval==SQLITE_ROW)
                                end_ip=(int)sqlite3_column_int(stmt,0);
                }
                sqlite3_finalize(stmt);
                sqlite3_close(handle);
        }else{
                bail("Error openning database for read.");
        }
        if (end_ip>0 && end_ip<255){
                sprintf(ip,"%s%d",VIR_NAEGLING_NETWORK,end_ip);
        }else{
                ip=NULL;
        }
        return ip;
}

char* get_cluster_ip_by_mac(const char *mac){
        
        sqlite3 *handle;
        int retval=sqlite3_open_v2(DATABASE_PATH,&handle,SQLITE_OPEN_READONLY,NULL);
        char *ip=NULL;
        if(retval==SQLITE_OK){
                char count[QUERY_SIZE];
                sprintf(count,"SELECT ip FROM cluster_network_table WHERE mac='%s'",mac);
                sqlite3_stmt *stmt;
                retval=sqlite3_prepare_v2(handle,count,strlen(count)+1,&stmt,NULL);
                if(retval){
                        bail("SQL error: get_ip_by_mac");
                }else{
                        retval=sqlite3_step(stmt);
                        if(retval==SQLITE_ROW){
                                ip=(char *)malloc(sizeof(char) *MAX_DOMAIN_SIZE);
                                strcpy(ip,(char *)sqlite3_column_text(stmt,0));
                        }
                }
                sqlite3_finalize(stmt);
                sqlite3_close(handle);
        }else{
                bail("Error openning database for read.");
        }
        return ip;
}


char* get_ip_by_domain(const char *domain){
        sqlite3 *handle;
        int retval=sqlite3_open_v2(DATABASE_PATH,&handle,SQLITE_OPEN_READONLY,NULL);
        int end_ip=0;
        char *ip=(char*)malloc(sizeof(char)*MAX_IP_SIZE);
        if(retval==SQLITE_OK){
                char count[QUERY_SIZE];
                sprintf(count,"SELECT ip FROM dhcp_table WHERE domain='%s'",domain);
                sqlite3_stmt *stmt;
                retval=sqlite3_prepare_v2(handle,count,strlen(count)+1,&stmt,NULL);
                if(retval){
                        bail("SQL error: get_ip_by_mac");
                }else{
                        retval=sqlite3_step(stmt);
                        if(retval==SQLITE_ROW)
                                end_ip=(int)sqlite3_column_int(stmt,0);
                }
                sqlite3_finalize(stmt);
                sqlite3_close(handle);
        }else{
                bail("Error openning database for read.");
        }
        if (end_ip>0 && end_ip<255){
                sprintf(ip,"%s%d",VIR_NAEGLING_NETWORK,end_ip);
        }else{
                ip=NULL;
        }
        return ip;
}


char* get_cluster_domain_by_mac(const char *mac){
        sqlite3 *handle;
        int retval=sqlite3_open_v2(DATABASE_PATH,&handle,SQLITE_OPEN_READONLY,NULL);
        char *domain=NULL;
        if(retval==SQLITE_OK){
		char count[QUERY_SIZE];
                sprintf(count,"SELECT domain FROM cluster_network_table WHERE mac='%s'",mac);
                sqlite3_stmt *stmt;
                retval=sqlite3_prepare_v2(handle,count,strlen(count)+1,&stmt,NULL);
                if(retval){
                        bail("SQL error: get_ip_by_mac");
                }else{
                        retval=sqlite3_step(stmt);
                        if(retval==SQLITE_ROW){
				domain=(char *)malloc(sizeof(char) *MAX_DOMAIN_SIZE);
                                strcpy(domain,(char *)sqlite3_column_text(stmt,0));
			}
                }
                sqlite3_finalize(stmt);
                sqlite3_close(handle);
        }else{
                bail("Error openning database for read.");
        }
        return domain;
}





void delete_from_dhcp_table_by_domain(const char *domain){
	sqlite3 *handle;
        int retval=sqlite3_open_v2(DATABASE_PATH,&handle,SQLITE_OPEN_READWRITE,NULL);
        if(retval==SQLITE_OK){
                char query[QUERY_SIZE];
                sprintf(query,"DELETE FROM dhcp_table where domain='%s'",domain);
		sqlite3_exec(handle,query,0,0,0);
                sqlite3_close(handle);
        }else{
                bail("Error openning database for reading sqlite3_open_v2.");
        }
}


void delete_from_cluster_network_table_by_domain(const char *domain){
        sqlite3 *handle;
        int retval=sqlite3_open_v2(DATABASE_PATH,&handle,SQLITE_OPEN_READWRITE,NULL);
        if(retval==SQLITE_OK){
                char query[QUERY_SIZE];
                sprintf(query,"DELETE FROM cluster_network_table where domain='%s'",domain);
                sqlite3_exec(handle,query,0,0,0);
                sqlite3_close(handle);
        }else{
                bail("Error openning database for reading sqlite3_open_v2.");
        }
}


