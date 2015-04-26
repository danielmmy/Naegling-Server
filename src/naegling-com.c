#include "naegling-com.h"

char file_path[MAX_FILE_PATH_SIZE];
void bail(const char *on_what){
	openlog("naegling_error.log",LOG_PID|LOG_CONS,LOG_USER);
	syslog(LOG_INFO,"Error: %s\n",on_what);
	closelog();
}

void naegling_log(const char *message){
        openlog("naegling_naegling.log",LOG_PID|LOG_CONS,LOG_USER);
        syslog(LOG_INFO,"Log: %s\n",message);
        closelog();

}

void listen_for_remote_message(){
	int server_fd;//, client_sock, c;
        struct sockaddr_in server;// , client;

        //Create socket
        server_fd = socket(AF_INET , SOCK_STREAM , 0);
        if (server_fd == -1){
                bail("Could not create socket");
        }
        naegling_log("Socket created");

        //Prepare the sockaddr_in structure
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = INADDR_ANY;
        server.sin_port = htons(NAEGLING_SERVER_PORT );

        //Bind
        if( bind(server_fd,(struct sockaddr *)&server , sizeof(server)) < 0){
                bail("bind failed. Error");
		fprintf(stderr,"Error bind(2)\n");
		exit(1);
        }
       	naegling_log("bind done");

        //Listen
        listen(server_fd , SOMAXCONN);

	for(;;){
		int session_fd=accept(server_fd,0,0);
		if(session_fd==-1) {
	        	if(errno==EINTR) 
				continue;
		        bail("failed to accept connection");
			exit(1);
    		}
		signal(SIGCHLD, SIG_IGN);//Beware! Zombies Ahead!
	    	pid_t pid=fork();
		if (pid==-1) {
        		bail("failed to create child process");
			exit(1);
		}else if(pid==0){
			close(server_fd);
			connection_handler(session_fd);
			close(session_fd);
			_Exit(0);
		}else{
			close(session_fd);
		}
	}

#if 0
        //Accept and incoming connection
       naegling_log("Waiting for incoming connections...");
        c = sizeof(struct sockaddr_in);
        pthread_t thread_id;
		while((client_sock = accept(socket_desc,(struct sockaddr *)&client,(socklen_t*)&c))){
                	naegling_log("Connection accepted");
                	if(pthread_create(&thread_id,NULL,connection_handler,(void*)&client_sock)<0)
                        	bail("could not create thread");
                	naegling_log("Handler assigned");
        	}

        	if (client_sock < 0)
                	bail("accept failed");
#endif
}



void connection_handler(int session_fd){
        int i;
	int function_retval;
        int sock = session_fd;
        int read_size;
        int write_size;
        char *message , client_message[TCP_MESSAGE_MAX_SIZE];
	
	read_size=read(sock,client_message,TCP_MESSAGE_MAX_SIZE);
	if(read_size<0){
		bail("recv(2)");
		return;
	}
	client_message[read_size] = '\0';
	naegling_log(client_message);
        char **message_fields;
        int count=get_field_count(client_message);
        message_fields=(char **)malloc(count*sizeof(char *));
        for(i=0;i<count;i++)
        	message_fields[i]=(char *)malloc(256*(sizeof(char)));
        get_message_fields(client_message,message_fields);
        if(message_fields[0]!=NULL){
        	int option=-1;
                char* ptr;
                option=strtol(message_fields[0],&ptr,10);
                if(option<=0 && ptr==message_fields[0])
                	bail("Invalid message format.");
                else{
                	switch(option){
                        	case START_NODE:
                                	naegling_log("Trying to start node...");
                                        function_retval=vm_start(message_fields[1],message_fields[2]);
                                        if(function_retval==0)
                                        	naegling_log("Node successfully started.");
                                        else
                                        	bail("Error starting node.");
                                        break;
				case STOP_NODE:
                                	function_retval=vm_stop(message_fields[1],message_fields[2]);
                                        break;
				case CREATE_MASTER_NODE:
                                	function_retval=create_master_vm(message_fields[1],message_fields[2],message_fields[3],message_fields[4],message_fields[5],message_fields[6], message_fields[7],message_fields[8],message_fields[9],message_fields[10],message_fields[11]);
                                        break;
				case CREATE_SLAVE_NODE:
                                	function_retval=create_diskless_slave_vm(message_fields[1],message_fields[2],message_fields[3],message_fields[4],message_fields[5],message_fields[6], message_fields[7],message_fields[8]);
                                        break;
				case NODE_STATUS:
                                	function_retval=vm_status(message_fields[1],message_fields[2]);
                                        break;
				case TEMPLATE_STATUS:
                                	function_retval=template_exists(message_fields[1]);
                                        break;
				case START_MASTER_VIRTUAL_NODE:
                                	function_retval=vm_start_master_virtual_node(message_fields[1],message_fields[2],message_fields[3],message_fields[4],message_fields[5]);
                                        break;
				case STOP_MASTER_VIRTUAL_NODE:
					function_retval=vm_stop_master_virtual_node(message_fields[1],message_fields[2],message_fields[3]);
                                        break;
				case ADD_WORKING_NODE:
                                	function_retval=add_working_node(message_fields[1],message_fields[2],message_fields[3],message_fields[4],message_fields[5]);
                                        break;
				case REMOVE_WORKING_NODE:
                                	function_retval=remove_working_node(message_fields[1],message_fields[2],message_fields[3],message_fields[4]);
                                        break;
				case GET_CLUSTER_STATUS:
                                	function_retval=get_cluster_status(message_fields[1]);
                                        break;
				case REQUEST_CLUSTER_IP:
					function_retval=2;
                                        char *ip=get_cluster_ip(message_fields[1]);
                                        strcpy(client_message,ip);
                                        free(ip);
                                        break;
				case REQUEST_TEMPLATE_TRANSFER:
                                	if(FILE_TRANSFER_AVAILABLE){
                                        	FILE_TRANSFER_AVAILABLE=0;
                                                function_retval=prepare_template_transfer(message_fields[1]);
                                                if(!function_retval)
                                                	insert_into_template_table(message_fields[1],file_path,message_fields[2]);
						else{
                                                	FILE_TRANSFER_AVAILABLE=1;
                                                }
					}else{
                                        	function_retval=1;
                                        }
                                        break;
				case REQUEST_JOB_TRANSFER:
					function_retval=prepare_job_file_transfer(message_fields[1],message_fields[2],message_fields[3],message_fields[4],message_fields[5],sock);
                                        break;
				case EXECUTE_JOB:
                                	function_retval=run_job_script(message_fields[1],message_fields[2],message_fields[3]);
                                        if(!function_retval)
                                        	insert_into_job_status_table(message_fields[2],message_fields[1],JOB_EXECUTING);
                                        break;
				case DOWNLOAD_JOB_FILE:
                                        download_cluster_file(message_fields[1],message_fields[2],sock);
                                        break;
				case DELETE_NODE:
                                	function_retval=undefine_vm(message_fields[1],message_fields[2]);
                                        break;
				case EDIT_NODE:
					function_retval=edit_vm(message_fields[1],message_fields[2],message_fields[3],message_fields[4],message_fields[5]);
                                        break;
                                default:
                                	bail("Invalid message format.");
                        }
		}

                char *ret_message=(char *)malloc(UDP_MESSAGE_MAX_SIZE*(sizeof(char)));
                switch(function_retval){
                	case 0:
                        	strcpy(ret_message,"0#");
                                break;
                        case 1:
                        	strcpy(ret_message,"1#");
                                break;
                        case 2:/*send customized message*/
                        	strcpy(ret_message,client_message);
                                break;
                        case -1:
				strcpy(ret_message,"-1#");
                                break;
			case -5:
				strcpy(ret_message,"-5#");
				break;
                        default:
                        	strcpy(ret_message,"-1#");
		}

		if(strcmp(ret_message,"-5#")){
                	write_size=write(sock,ret_message,strlen(ret_message));
	        	if(write_size<0)
        	        	bail("write(2)");
               		free(ret_message);
		}
	}else{
        	bail("Invalid message format.");
        }
	for(i=0;i<count;i++)
        	free(message_fields[i]);
        free(message_fields);
        close(sock);
}




int get_field_count(char *message){
	int count=0;
	while(*message){
		if(strchr(MESSAGE_DELIMITER,*message))
			++count;
		++message;
	}
	return ++count;
}


void get_message_fields(char *message,char **message_fields){
	int i;
	char *str;
	char *rest;
	str=strtok_r(message,MESSAGE_DELIMITER,&rest);
	i=0;
	while(str){
		strcpy(message_fields[i++],str);
		str=strtok_r(NULL,MESSAGE_DELIMITER,&rest);
	}
}




int prepare_template_transfer(const char *file_name){
#if 0
        file_path[0]='\0';
        strcpy(file_path,NAEGLING_TEMPLATES_DIRECTORY);
        strcat(file_path,file_name);
        remove(file_path);
        pthread_t tid;
        pthread_create(&tid,NULL,file_transfer,file_path);
	return 0;
#endif
}



int send_message_to_cluster2(const char *cluster_ip_addr, const char *GUI_message){
        int retval;
        int z;
        int len_inet;
        struct sockaddr_in addr_cluster;
        int addr_size;
        int sock;
        char message[TCP_MESSAGE_MAX_SIZE];

        naegling_log(GUI_message);


        /*creating socket address*/
        memset(&addr_cluster,0,sizeof addr_cluster);
        addr_cluster.sin_family = AF_INET;
        addr_cluster.sin_port = htons(NAEGLING_CLUSTER_PORT);
        addr_cluster.sin_addr.s_addr =inet_addr(cluster_ip_addr);
        if ( addr_cluster.sin_addr.s_addr == INADDR_NONE){
                bail("bad address.");
                return -1;
        }
        len_inet = sizeof addr_cluster;

        /*creating TCP socket*/
        sock=socket(PF_INET,SOCK_STREAM,0);
        if (sock==-1){
                bail("socket(2)");
                return -1;
        }

        /*connect to server*/
        z=connect(sock,(struct sockaddr *)&addr_cluster,len_inet);
        if(z==-1){
                close(sock);
                bail("connect(2)");
                return -1;
        }

        /*sending message*/
        retval=write(sock,GUI_message,strlen(GUI_message));
        if(retval<0){
                close(sock);
                bail("sending message to server.");
                return -1;
        }

        return sock;
}




int prepare_job_file_transfer(const char *master_domain,const char *master_hypervisor, const char *master_mac,const char *job_name,const char *file_name, const int sock){
	int retval=-1;
	int cluster_sock;
	int block_sz;
	char *buff=(char *)malloc(sizeof(char)*TCP_MESSAGE_MAX_SIZE);//buffer
	char *message=(char *)malloc(sizeof(char)*256);

	char *ip=get_virNaegling_ip(master_domain,master_hypervisor,master_mac);
	if(*ip){
		sprintf(buff,"%d%s%s%s%s",REQUEST_JOB_TRANSFER,MESSAGE_DELIMITER,job_name,MESSAGE_DELIMITER,file_name);
		cluster_sock=send_message_to_cluster2(ip,buff);
		if(cluster_sock>0){
                        bzero(buff,TCP_MESSAGE_MAX_SIZE);
			//Send the message to cluster
			block_sz=write(cluster_sock , buff ,block_sz);
			bzero(buff,TCP_MESSAGE_MAX_SIZE);
			block_sz=read(cluster_sock,buff,TCP_MESSAGE_MAX_SIZE);
			shutdown(cluster_sock,SHUT_RD);
			buff[block_sz]='\0';
			naegling_log(buff);
			if(buff[0]=='0'){
				block_sz=write(sock,buff,strlen(buff));
				shutdown(sock, SHUT_WR);
				bzero(buff,TCP_MESSAGE_MAX_SIZE);
				while((block_sz = recv(sock, buff,TCP_MESSAGE_MAX_SIZE,0)) > 0){
					sprintf(message,"Received %d",block_sz);
					naegling_log(message);
					block_sz=write(cluster_sock,buff,block_sz);
					sprintf(message,"Sent %d",block_sz);
                                        naegling_log(message);
					bzero(buff,TCP_MESSAGE_MAX_SIZE);
				}
			}
			retval=0;
			naegling_log("Closing cluster socket");
			close(cluster_sock);
		}
		
	}
	return retval;
}

#if 0
void* file_transfer(void* arg){
 
        /* Defining Variables */
        int sock;
        int nsock;
        int num;
        int sin_size;
        struct sockaddr_in addr_local; /* client addr */
        struct sockaddr_in addr_remote; /* server addr */
        char buff[TCP_MESSAGE_MAX_SIZE]; // Receiver buffer

        /* Get the Socket file descriptor */
        if((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1 ){
                bail("[Image-Server] ERROR: Failed to obtain Socket Descriptor.");
                return;
        }else
                naegling_log("[Image-Server] Obtaining socket descriptor successfully.");

        /* Fill the client socket address struct */
        addr_local.sin_family = AF_INET; // Protocol Family
        addr_local.sin_port = htons(NAEGLING_FILE_TRANSFER_PORT); // Port number
        addr_local.sin_addr.s_addr = INADDR_ANY; // AutoFill local address
        bzero(&(addr_local.sin_zero), 8); // Flush the rest of struct

        /* Bind a special Port */
        if( bind(sock, (struct sockaddr*)&addr_local, sizeof(struct sockaddr)) == -1 ){
                bail("[Image-Server] ERROR: Failed to bind Port.");
                return;
        }else
                naegling_log("[Image-Server] Binded tcp port sucessfully.");

        /* Listen remote connect/calling */
        if(listen(sock,SOMAXCONN) == -1){
                bail("[Image-Server] ERROR: Failed to listen Port.");
                return;
        }
        else
                naegling_log("[Image-Server] Listening the port successfully.");

        int success = 0;
        int block_sz=TCP_MESSAGE_MAX_SIZE;
        while(success == 0 && block_sz==TCP_MESSAGE_MAX_SIZE){
                sin_size = sizeof(struct sockaddr_in);

                /* Wait a connection, and obtain a new socket file despriptor for single connection */
                if ((nsock = accept(sock, (struct sockaddr *)&addr_remote, &sin_size)) == -1){
                    bail("[Image-Server] ERROR: Obtaining new Socket Despcritor.");
                        return;
                }else
                        naegling_log("[Image-Server] Server has got connected.");

                /*Receive File from Client */
                FILE *fp = fopen(file_path, "ab");
                if(fp == NULL)
                        naegling_log("[Image-Server] File Cannot be opened file on server.");
                else{
                        bzero(buff, TCP_MESSAGE_MAX_SIZE);
                        block_sz = 0;
                        while((block_sz = recv(nsock, buff,TCP_MESSAGE_MAX_SIZE,MSG_WAITALL)) > 0){
                            int write_sz = fwrite(buff, sizeof(char), block_sz, fp);
                                if(write_sz < block_sz){
                                        bail("[Image-Server] File write failed on server.");
                                }
                                bzero(buff, TCP_MESSAGE_MAX_SIZE);
                                if (block_sz == 0 || block_sz != 512){
                                        break;
                                }
                        }
                        if(block_sz < 0){
                                if (errno == EAGAIN){
                                        naegling_log("[Image-Server] recv() timed out.");
                                }else{
                                        bail("[Image-Server] recv() failed.");
                                        return;
                                }
                        }
                        naegling_log("[Image-Server] Ok received from client!");
                        fflush(fp);
                        fclose(fp);
                }

            close(nsock);
            naegling_log("[Image-Server] Connection with Client closed. Server will wait now...");
        }
        close(sock);
	FILE_TRANSFER_AVAILABLE=1;
}
#endif

#if 0
void *job_file_transfer(void *arg){
	char *ip=(char *)arg;
	struct sockaddr_in addr_cluster;
	int len_inet_cluster;
	int sock_cluster;
        int sock;
        int nsock;
        int num;
        int sin_size;
        struct sockaddr_in addr_local; /* client addr */
        struct sockaddr_in addr_remote; /* server addr */
        char buff[TCP_MESSAGE_MAX_SIZE]; // Receiver buffer

        /* Get the Socket file descriptor */
        if((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1 ){
                bail("job_file_transfer: ERROR: Failed to obtain Socket Descriptor.");
                return;
        }else
                naegling_log("job_file_transfer: Obtaining socket descriptor successfully.");

        /* Fill the client socket address struct */
        addr_local.sin_family = AF_INET; // Protocol Family
        addr_local.sin_port = htons(NAEGLING_FILE_TRANSFER_PORT); // Port number
        addr_local.sin_addr.s_addr = INADDR_ANY; // AutoFill local address
        bzero(&(addr_local.sin_zero), 8); // Flush the rest of struct

        /* Bind a special Port */
        if( bind(sock, (struct sockaddr*)&addr_local, sizeof(struct sockaddr)) == -1 ){
                bail("job_file_transfer: ERROR: Failed to bind Port.");
                return;
        }else
                naegling_log("job_file_transfer: Binded tcp port sucessfully.");

        /* Listen remote connect/calling */
        if(listen(sock,SOMAXCONN) == -1){
                bail("job_file_transfer: ERROR: Failed to listen Port.");
                return;
        }
        else
                naegling_log("job_file_transfer: Listening the port successfully.");

        int success = 0;
        int block_sz=TCP_MESSAGE_MAX_SIZE;
        while(success == 0 && block_sz==TCP_MESSAGE_MAX_SIZE){
                sin_size = sizeof(struct sockaddr_in);

                /* Wait a connection, and obtain a new socket file despriptor for single connection */
                if ((nsock = accept(sock, (struct sockaddr *)&addr_remote, &sin_size)) == -1){
			bail("job_file_transfer: ERROR: Obtaining new Socket Despcritor.");
                        return;
                }else{
                        naegling_log("job_file_transfer: Server has got connected.");
		}

                /*Receive File from Client */
                bzero(buff, TCP_MESSAGE_MAX_SIZE);
                block_sz = 0;
                while((block_sz = recv(nsock, buff,TCP_MESSAGE_MAX_SIZE,MSG_WAITALL)) > 0){
			sock_cluster=socket(PF_INET,SOCK_STREAM,0);
		        if(sock_cluster==-1){
		                bail("job_file_transfer: Error acquiring socket");
		                return;
		        }
		        memset(&addr_cluster,0,sizeof addr_cluster);
		        addr_cluster.sin_family=AF_INET;
		        addr_cluster.sin_port=htons(NAEGLING_CLUSTER_FILE_TRANSFER_PORT);
		        addr_cluster.sin_addr.s_addr=inet_addr(ip);
		        if(addr_cluster.sin_addr.s_addr==INADDR_NONE){
		                bail("job_file_transfer: Bad address.");
		                return;
		        }
			len_inet_cluster=sizeof(addr_cluster);
			int z=connect(sock_cluster,(struct sockaddr*)&addr_cluster,len_inet_cluster);
			if(z==-1){
				bail("job_file_transfer: Error on connect(2)");
				return;
			}
			z=write(sock_cluster,buff,block_sz);
			if(z==-1){
				bail("job_file_transfer: write(2)");
				return;
			}
			close(sock_cluster);

                        bzero(buff, TCP_MESSAGE_MAX_SIZE);
                        if (block_sz == 0 || block_sz != 512){
                        	break;
                        }
               }
               if(block_sz < 0){
			if (errno == EAGAIN){
				naegling_log("job_file_transfer: recv() timed out.");
                        }else{
                                bail("job_file_transfer:  recv() failed.");
                        	return;
                        }
               }
               naegling_log("job_file_transfer: Ok received from client!");
                

            close(nsock);
            naegling_log("job_file_transfer: Connection with Client closed. Server will wait now...");
        }
        close(sock);
        FILE_TRANSFER_AVAILABLE=1;

}
#endif






int send_message_to_cluster(const char *cluster_ip_addr, const char *GUI_message){
	int retval;
        int z;
        int len_inet;
        struct sockaddr_in addr_cluster;
        int addr_size;
        int sock;
        char message[TCP_MESSAGE_MAX_SIZE];

	naegling_log(GUI_message);


        /*creating socket address*/
        memset(&addr_cluster,0,sizeof addr_cluster);
        addr_cluster.sin_family = AF_INET;
        addr_cluster.sin_port = htons(NAEGLING_CLUSTER_PORT);
        addr_cluster.sin_addr.s_addr =inet_addr(cluster_ip_addr);
        if ( addr_cluster.sin_addr.s_addr == INADDR_NONE){
                bail("bad address.");
                return -1;
        }
        len_inet = sizeof addr_cluster;

        /*creating TCP socket*/
        sock=socket(PF_INET,SOCK_STREAM,0);
        if (sock==-1){
                bail("socket(2)");
                return -1;
        }

        /*connect to server*/
        z=connect(sock,(struct sockaddr *)&addr_cluster,len_inet);
        if(z==-1){
		close(sock);
                bail("connect(2)");
                return -1;
        }

        /*sending message*/
        retval=write(sock,GUI_message,strlen(GUI_message));
        if(retval<0){
		close(sock);
                bail("sending message to server.");
                return -1;
        }
	close(sock);
        return 0;
}


int get_cluster_status(const char *master_mac){
	char *ip=get_ip_by_mac(master_mac);
	char *message=(char *)malloc(sizeof(char)*UDP_MESSAGE_MAX_SIZE);
	sprintf(message,"%d",GET_CLUSTER_STATUS);
	int retval=send_message_to_cluster(ip,message);	
	free(message);
	free(ip);
	return retval;
}



char * get_cluster_ip(const char *mac){
//	int retval=-1;
	char *ip=get_cluster_ip_by_mac(mac);
	//char *domain=get_cluster_domain_by_mac(mac);
	//char *vir_naegling_ip=get_ip_by_domain(domain);

	
	//if(ip&&domain&&vir_naegling_ip){
	//	char *message=(char *)malloc(sizeof(char)*UDP_MESSAGE_MAX_SIZE);
	///	sprintf(message,"%d%s%s",START_NEW_CLUSTER,MESSAGE_DELIMITER,ip);
        //	retval=send_message_to_cluster(vir_naegling_ip,message);
//
//		free(message);
//		        free(ip);
//		        free(domain);
//		        free(vir_naegling_ip);

//	}else{
//		if(ip)
//			free(ip);
//		if(domain)
//			free(domain);
//		if(vir_naegling_ip)
//			free(vir_naegling_ip);
//	}


	return ip;
}


int add_working_node(const char *domain,const char *mac,const char *master_domain, const char *master_hypervisor,const char *master_mac){
	int retval=-1;
	char *message=(char *)malloc(sizeof(char)*TCP_MESSAGE_MAX_SIZE);
	sprintf(message,"%d%s%s%s%s",ADD_WORKING_NODE,MESSAGE_DELIMITER,domain,MESSAGE_DELIMITER,mac);
	char *ip=get_virNaegling_ip(master_domain,master_hypervisor,master_mac);
	if(ip!=NULL){
		retval=send_message_to_cluster(ip,message);
		free(ip);
	}
	free(message);
	return retval;
}


int remove_working_node(const char *domain,const char *master_domain, const char *master_hypervisor,const char *master_mac){
        int retval=-1;
        char *message=(char *)malloc(sizeof(char)*UDP_MESSAGE_MAX_SIZE);
        sprintf(message,"%d%s%s",REMOVE_WORKING_NODE,MESSAGE_DELIMITER,domain);
	char *ip=get_virNaegling_ip(master_domain,master_hypervisor,master_mac);
	naegling_log(message);
        if(ip!=NULL){
                retval=send_message_to_cluster(ip,message);
                free(ip);
        }
        free(message);
        return retval;
}

int run_job_script(char * master_domain, char *job_name, char *script_name){
	int retval=-1;
	char *message=(char *)malloc(sizeof(char)*UDP_MESSAGE_MAX_SIZE);
	sprintf(message,"%d%s%s%s%s",EXECUTE_JOB,MESSAGE_DELIMITER,job_name,MESSAGE_DELIMITER,script_name);
	char *ip=get_ip_by_domain(master_domain);
	if(ip!=NULL){
		retval=send_message_to_cluster(ip,message);
                free(ip);
	}
	free(message);
	return retval;
}

int download_cluster_file(const char * master_domain, const char *path, const int GUI_sock){
        int cluster_sock=-1;
	int read_size;
	int write_size;
	int retval;
        char *message=(char *)malloc(sizeof(char)*TCP_MESSAGE_MAX_SIZE);
        sprintf(message,"%d%s%s",DOWNLOAD_JOB_FILE,MESSAGE_DELIMITER,path);
        char *cluster_ip_addr=get_ip_by_domain(master_domain);
        if(cluster_ip_addr!=NULL){
		int z;
        	int len_inet;
        	struct sockaddr_in addr_cluster;
        	int addr_size;
        	int cluster_sock;



	        /*creating socket address*/
        	memset(&addr_cluster,0,sizeof addr_cluster);
	        addr_cluster.sin_family = AF_INET;
	        addr_cluster.sin_port = htons(NAEGLING_CLUSTER_PORT);
       		addr_cluster.sin_addr.s_addr =inet_addr(cluster_ip_addr);
	        if ( addr_cluster.sin_addr.s_addr == INADDR_NONE){
        	        bail("bad address.");
               		return -1;
        	}
        	len_inet = sizeof addr_cluster;

        	/*creating TCP socket*/
	        cluster_sock=socket(PF_INET,SOCK_STREAM,0);
        	if (cluster_sock==-1){
                	bail("socket(2)");
                	return -1;
       		}

	        /*connect to cluster*/
	        z=connect(cluster_sock,(struct sockaddr *)&addr_cluster,len_inet);
       		if(z==-1){
                	bail("connect(2)");
                	return -1;
        	}

	        /*sending message*/
        	retval=write(cluster_sock,message,strlen(message));
		shutdown(cluster_sock,SHUT_WR);
        
		//transfer file
		while((read_size = recv(cluster_sock,message,TCP_MESSAGE_MAX_SIZE,0))>0){
	                //Send file to client
                	write_size=write(GUI_sock , message ,read_size);
        	        memset(message, 0, TCP_MESSAGE_MAX_SIZE);
	        }

        	//Close cluster_socket
        	shutdown(cluster_sock,2);
	}
	
	free(cluster_ip_addr);
        return -5;
}



void *server_gui_job_file_transfer(void *arg){
	char *ip=(char *)arg;
	struct sockaddr_in addr_cluster;
	int len_inet_cluster;
	int sock_cluster;
        int sock;
        int nsock;
        int num;
        int sin_size;
        struct sockaddr_in addr_local; /* client addr */
        struct sockaddr_in addr_remote; /* server addr */
        char buff[TCP_MESSAGE_MAX_SIZE]; // Receiver buffer

        /* Get the Socket file descriptor */
        if((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1 ){
                bail("job_file_transfer: ERROR: Failed to obtain Socket Descriptor.");
                return;
        }else
                naegling_log("job_file_transfer: Obtaining socket descriptor successfully.");

        /* Fill the client socket address struct */
        addr_local.sin_family = AF_INET; // Protocol Family
        addr_local.sin_port = htons(9293); // Port number
        addr_local.sin_addr.s_addr = INADDR_ANY; // AutoFill local address
        bzero(&(addr_local.sin_zero), 8); // Flush the rest of struct

        /* Bind a special Port */
        if( bind(sock, (struct sockaddr*)&addr_local, sizeof(struct sockaddr)) == -1 ){
                bail("job_file_transfer: ERROR: Failed to bind Port.");
                return;
        }else
                naegling_log("job_file_transfer: Binded tcp port sucessfully.");

        /* Listen remote connect/calling */
        if(listen(sock,SOMAXCONN) == -1){
                bail("job_file_transfer: ERROR: Failed to listen Port.");
                return;
        }
        else
                naegling_log("job_file_transfer: Listening the port successfully.");

        sin_size = sizeof(struct sockaddr_in);

        /* Wait a connection, and obtain a new socket file despriptor for single connection */
        if ((nsock = accept(sock, (struct sockaddr *)&addr_remote, &sin_size)) == -1){
		bail("job_file_transfer: ERROR: Obtaining new Socket Despcritor.");
                return;
        }else{
                naegling_log("job_file_transfer: Server has got connected.");
	}

        /*Receive File from Client */
        bzero(buff, TCP_MESSAGE_MAX_SIZE);
        int block_sz = 0;
        if((block_sz = recv(nsock, buff,TCP_MESSAGE_MAX_SIZE,MSG_WAITALL)) > 0){
		sock_cluster=socket(PF_INET,SOCK_STREAM,0);
		if(sock_cluster==-1){
			bail("job_file_transfer: Error acquiring socket");
			return;
		}
		memset(&addr_cluster,0,sizeof addr_cluster);
		addr_cluster.sin_family=AF_INET;
		addr_cluster.sin_port=htons(9294);
		addr_cluster.sin_addr.s_addr=inet_addr(ip);
		if(addr_cluster.sin_addr.s_addr==INADDR_NONE){
			bail("job_file_transfer: Bad address.");
		        return;
		}
		len_inet_cluster=sizeof(addr_cluster);
		int z=connect(sock_cluster,(struct sockaddr*)&addr_cluster,len_inet_cluster);
		if(z==-1){
			bail("job_file_transfer: Error on connect(2)");
			return;
		}
		z=write(sock_cluster,buff,block_sz);
		if(z==-1){
			bail("job_file_transfer: write(2)");
			return;
		}
		close(sock_cluster);

	}else{
        	bail("job_file_transfer:  recv() failed.");
                return;
        }
        naegling_log("job_file_transfer: Ok received from client!");
                

        close(nsock);
        naegling_log("job_file_transfer: Connection with Client closed. Server will wait now...");
        
        close(sock);
        FILE_TRANSFER_AVAILABLE=1;
}


