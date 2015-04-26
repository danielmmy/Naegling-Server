#include "vm-control.h"
#include "naegling-com.h"

char *LINE_DELIMITER="#";
int create_master_vm(const char *domain,const char *template,const char *vdisk_path,const char *uuid,const char *mac,const char *bridge_network_interface,const char *naegling_mac,const char *hypervisor, const char *ram_memory, const char * cpu_quantity,const char * vnc_port){
	char *xml_config=(char *)malloc(MAX_XML_FILE_SIZE*sizeof(char));
	sprintf(xml_config,"<domain type='kvm'>\n"
		   "  <name>%s</name>\n"
		   "  <uuid>%s</uuid>\n"
		   "  <memory>%s</memory>\n"
		   "  <currentMemory>%s</currentMemory>\n"
		   "  <vcpu>%s</vcpu>\n"
	 	   "  <os>\n"
		   "    <type>hvm</type>\n"
		   "    <boot dev='hd'/>\n"
		   "  </os>\n"
		   "  <features>\n"
		   "    <acpi/>\n"
		   "  </features>\n"
		   "  <clock offset='utc'/>\n"
		   "  <on_poweroff>destroy</on_poweroff>\n"
		   "  <on_reboot>restart</on_reboot>\n"
		   "  <on_crash>destroy</on_crash>\n"
		   "  <devices>\n"
		   "    <emulator>/usr/bin/kvm</emulator>\n"
		   "    <disk type='block' device='disk'>\n"
		   "      <source dev='%s'/>\n"
		   "      <target dev='hda' bus='ide'/>\n"
		   "    </disk>\n"
                   "    <interface type='network'>\n"
                   "      <mac address='%s'/>\n"
                   "      <source network='virNaegling'/>\n"
                   "    </interface>\n"
		   "    <interface type='bridge'>\n"
		   "      <mac address='%s'/>\n"
		   "      <source bridge='%s'/>\n"
		   "    </interface>\n"
		   "	<graphics type='vnc' port='%s' autoport='no' listen='0.0.0.0' />"
    		   "    <video>"
    		   "      <model type='cirrus' vram='9216' heads='1'/>"
   		   "    </video>"
		   "  </devices>\n"
		   "</domain>"
,domain,uuid,ram_memory,ram_memory,cpu_quantity,vdisk_path,naegling_mac,mac,bridge_network_interface,vnc_port);
	virConnectPtr conn=virConnectOpen(hypervisor);
	if(!conn){
		bail("connection failed.");
		return -1;
	}
	virDomainPtr dom=virDomainDefineXML(conn,xml_config);
	if(!dom){
		bail("Domain definition failed.");
		return -1;
	}else{
		virDomainFree(dom);
	}

	int retval=virConnectClose(conn);
	if(retval==-1)
		bail("close connection failed");
	free(xml_config);
	insert_into_device_table(vdisk_path,domain,COPYING);
	pid_t pid=fork();
	
	/*
	 * Do not wait for child process creating zombie process.
	 */
	signal(SIGCHLD, SIG_IGN);
	if(pid==0){
		char *template_path=(char *)malloc((strlen(template)+strlen(NAEGLING_TEMPLATES_DIRECTORY)+1)*sizeof(char));
		template_path[0]='\0';
		strcpy(template_path,NAEGLING_TEMPLATES_DIRECTORY);
		strcat(template_path,template);
		int retval=copy_large_file(template_path,vdisk_path);
		update_device_status(vdisk_path,DEVICE_READY);
		free(template_path);
		naegling_log("File copy successful.");
		exit(0);
	}
	return 0;
}

int create_diskless_slave_vm(const char *domain,const char *uuid,const char *mac,const char *bridge_network_interface,const char *hypervisor, const char *ram_memory, const char * cpu_quantity,const char * vnc_port){
	char *xml_config=((char*)malloc(MAX_XML_FILE_SIZE*sizeof(char)));
	sprintf(xml_config,"<domain type='kvm'>\n"
		   "  <name>%s</name>\n"
		   "  <uuid>%s</uuid>\n"
		   "  <memory>%s</memory>\n"
		   "  <currentMemory>%s</currentMemory>\n"
		   "  <vcpu>%s</vcpu>\n"
	 	   "  <os>\n"
		   "    <type>hvm</type>\n"
		   "    <boot dev='network'/>\n"
		   "  </os>\n"
		   "  <features>\n"
		   "    <acpi/>\n"
		   "  </features>\n"
		   "  <clock offset='utc'/>\n"
		   "  <on_poweroff>destroy</on_poweroff>\n"
		   "  <on_reboot>restart</on_reboot>\n"
		   "  <on_crash>destroy</on_crash>\n"
		   "  <devices>\n"
		   "    <emulator>/usr/bin/kvm</emulator>\n"
		   "    <interface type='bridge'>\n"
		   "      <mac address='%s'/>\n"
		   "      <source bridge='%s'/>\n"
		   "    </interface>\n"
		   "	<graphics type='vnc' port='%s' autoport='no' listen='0.0.0.0' />"
    		   "    <video>"
    		   "      <model type='cirrus' vram='9216' heads='1'/>"
   		   "    </video>"
		   "  </devices>\n"
		   "</domain>"
,domain,uuid,ram_memory,ram_memory,cpu_quantity,mac,bridge_network_interface,vnc_port);
	virConnectPtr conn=virConnectOpen(hypervisor);
	if(!conn){
		bail("connection failed.");
		return -1;
	}
	virDomainPtr dom=virDomainDefineXML(conn,xml_config);
	if(!dom){
		bail("Domain definition failed.");
		return -1;
	}else
		virDomainFree(dom);

	int retval=virConnectClose(conn);
	if(retval==-1)
		bail("close connection failed.");
	free(xml_config);
	return 0;
}

int vm_status(const char *domain,const char *hypervisor){
	virConnectPtr conn=virConnectOpen(hypervisor);
	if(!conn){
		bail("connection failed.");
		return -1;
	}
	virDomainPtr dom=virDomainLookupByName(conn,domain);
	if(!dom){
                bail("Domain lookup failed.");
                return -1;
        }
	int retval=virDomainIsActive(dom);
	virDomainFree(dom);
	virConnectClose(conn);
	return retval;
}


int vm_start_master_virtual_node(const char *domain,const char *hypervisor,const char *mac,const char *cluster_mac,const char *cluster_ip){
        int retval=-1;
	char *ip=insert_into_dhcp_table(mac,domain);
	insert_into_cluster_network_table(cluster_ip,domain,cluster_mac);
	if(ip){
		add_mac_to_dhcp(ip,domain,mac);
		retval=vm_start(domain,hypervisor);
		free(ip);
	}
        return retval;
}

int vm_stop_master_virtual_node(const char *domain, const char *hypervisor, const char *mac){
	//char *ip=get_ip_by_mac(mac);
	char *ip=get_virNaegling_ip(domain,hypervisor,mac);
	int retval=-1;
	if(ip){
		remove_mac_from_dhcp(ip,domain,mac);		
		free(ip);
	}
	delete_from_dhcp_table_by_domain(domain);
	delete_from_cluster_network_table_by_domain(domain);
	retval=vm_stop(domain,hypervisor);	
	return retval;
}

int vm_start_slave_virtual_diskless_node(const char *domain,const char *hypervisor,const char *mac,const char *master_mac){
	int retval=-1;
	//char *ip=get_ip_by_mac(master_mac);
        char *ip=get_virNaegling_ip(domain,hypervisor,mac);
	char *message=(char *)malloc(sizeof(char)*UDP_MESSAGE_MAX_SIZE);
	if(ip){
		sprintf(message,"%d%s%s%s%s",ADD_WORKING_NODE,MESSAGE_DELIMITER,domain,MESSAGE_DELIMITER,mac);
		retval=send_message_to_cluster(ip,message);	
		if(!retval){
			retval=vm_start(domain,hypervisor);
		}

		free(ip);
	}
	free(message);
	return retval;
}


int vm_start(const char *domain,const char *hypervisor){
	int status=get_device_status(domain);
	int retval=-1;
	if(status==DEVICE_READY){
		virConnectPtr conn=virConnectOpen(hypervisor);
		if(!conn){
			bail("connection failed.");
		}
		virDomainPtr dom=virDomainLookupByName(conn,domain);
		if(!dom){
		        bail("Domain lookup failed.");
        	}
		retval=virDomainCreate(dom);
		virDomainFree(dom);
		virConnectClose(conn);
	}
	return retval;
}

int vm_stop(const char *domain,const char *hypervisor){
	virConnectPtr conn=virConnectOpen(hypervisor);
	if(!conn){
		bail("connection failed.");
		return -1;
	}
	virDomainPtr dom=virDomainLookupByName(conn,domain);
	if(!dom){
                bail("Domain lookup failed.");
                return -1;
        }
	int retval=virDomainDestroy(dom);
	virDomainFree(dom);
	virConnectClose(conn);
	return retval;
}



int create_virtual_network_naegling(){
        int retval;
        char *xml={
                "<network>"
                " <name>virNaegling</name>"
                " <bridge name=\"virNaegling\" />"
                " <forward/>"
                " <ip address=\"192.168.125.1\" netmask=\"255.255.255.0\">"
                "   <dhcp>"
                "     <range start=\"192.168.125.2\" end=\"192.168.125.254\" />"
                "   </dhcp>"
                " </ip>"
                "</network>"
        };
        virConnectPtr conn=virConnectOpen("qemu:///system");
        if(!conn){
               bail("connection failed.");
                return -1;
        }
        virNetworkPtr net=virNetworkDefineXML(conn,xml);
	/*free(xml);*/
        if(net!=NULL){
                virNetworkCreate(net);
                virNetworkFree(net);
                virConnectClose(conn);
                return 0;
        }else{
                return -1;
        }
}




int start_virtual_network_naegling(){
        virConnectPtr conn=virConnectOpen("qemu:///system");
        if(!conn){
                bail("connection failed.");
                return -1;
        }
        virNetworkPtr net=virNetworkLookupByName(conn,"virNaegling");
        if(net!=NULL){
                virNetworkCreate(net);
                virNetworkFree(net);
                virConnectClose(conn);
                return 0;
        }else{
                return -1;
        }

}


int stop_virtual_network_naegling(){
        virConnectPtr conn=virConnectOpen("qemu:///system");
        if(!conn){
                bail("connection failed.");
                return -1;
        }
        virNetworkPtr net=virNetworkLookupByName(conn,"virNaegling");
        if(net!=NULL){
                virNetworkDestroy(net);
                virNetworkFree(net);
                virConnectClose(conn);
                return 0;
        }else{
                return -1;
        }

}


char * get_vir_naegling_ip(const char *mac){
        char *ip_output;
	size_t ip_bytes=MAX_IP_SIZE;
	char *query;
        ip_output=(char *)malloc(((MAX_IP_SIZE)+1)*sizeof (char));
        query=(char *)malloc(((MAX_QUERY_SIZE)+1)*sizeof (char));
        sprintf(query,"cat /var/lib/libvirt/dnsmasq/virNaegling.leases | grep %s | cut -d' ' -f 3",mac);
        FILE *ip=popen(query,"r");
        if(!ip){
                bail("Error using popen().");
		return NULL;
	}
        int bytes_read=getline(&ip_output,&ip_bytes,ip);
        int i=0;
        while(ip_output[i]!='\n')
               ++i;
        ip_output[i]='\0';
	free(query);
        return ip_output;
}

void getMD5(const char* file_path,char* result){
	char *file_buffer;
	int fd=open(file_path,O_RDONLY);
	if(fd<0)
		return;
	struct stat statbuf;
    	if(fstat(fd, &statbuf) < 0)
		return;
    	
	unsigned char* res=(unsigned char *)malloc((MD5_DIGEST_LENGTH)*sizeof(unsigned char));
	
	file_buffer = mmap(0,statbuf.st_size ,PROT_READ, MAP_SHARED, fd, 0);
    	MD5((unsigned char*) file_buffer, statbuf.st_size, res);
	int i;
	char buff[3];
    	for(i=0; i <MD5_DIGEST_LENGTH; i++){
		sprintf(buff,"%02x",res[i]);
		buff[2]='\0';
		strcat(result,buff);
	}
	free(res);
}

int copy_large_file(const char *src_path, const  char *dest_path){
        int src;
        int dest;
        struct stat stat_buf;
        off_t offset=0;
        int retval=0;
        int bytes;

        src=open(src_path,O_RDONLY);
        fstat(src,&stat_buf);
        dest=open(dest_path,O_WRONLY|O_CREAT,stat_buf.st_mode);
        do{
                bytes=sendfile(dest,src,&offset,stat_buf.st_size);
                if(bytes!=-1){
                        retval+=bytes/1048576;
                }else{
                        bail("Error usind senfile().");
                        return -1;
                }
        }while(bytes!=0);
        close(dest);
        close(src);
        return retval;
}

const char * add_mac_to_dhcp(const char *ip,const char *domain,const char *mac){
	int retval=-1;
	char *xml=(char *)malloc(sizeof(char)*256);
	virConnectPtr conn=virConnectOpen("qemu:///system");
        if(conn){
       		virNetworkPtr net=virNetworkLookupByName(conn,"virNaegling");
        	if(net!=NULL){
                	sprintf(xml,"<host mac='%s' name='%s' ip='%s' />",mac,domain,ip);
			virNetworkUpdate(net,VIR_NETWORK_UPDATE_COMMAND_ADD_LAST,VIR_NETWORK_SECTION_IP_DHCP_HOST,-1,xml,VIR_NETWORK_UPDATE_AFFECT_CURRENT );
                	virNetworkFree(net);
        	}else{
			bail("Could not find virNaegling network.");
		}
               	virConnectClose(conn);
	}else{
		bail("connection failed.");
	}
	return ip;
}


int remove_mac_from_dhcp(const char *ip,const char *domain,const char *mac){
        int retval=-1;
        char *xml=(char *)malloc(sizeof(char)*256);
        virConnectPtr conn=virConnectOpen("qemu:///system");
        if(conn){
                virNetworkPtr net=virNetworkLookupByName(conn,"virNaegling");
                if(net!=NULL){
                        sprintf(xml,"<host mac='%s' name='%s' ip='%s' />",mac,domain,ip);
                        retval=virNetworkUpdate(net,VIR_NETWORK_UPDATE_COMMAND_DELETE,VIR_NETWORK_SECTION_IP_DHCP_HOST,-1,xml,VIR_NETWORK_UPDATE_AFFECT_CURRENT );
                        virNetworkFree(net);
                }else{
                        bail("Could not find virNaegling network.");
                }
                virConnectClose(conn);
        }else{
                bail("connection failed.");
        }
        return retval;
}


int undefine_vm(const char *domain,const char *hypervisor){
        virConnectPtr conn=virConnectOpen(hypervisor);
        if(!conn){
                bail("connection failed.");
                return -1;
        }
        virDomainPtr dom=virDomainLookupByName(conn,domain);
        if(!dom){
                bail("Domain lookup failed.");
                return -1;
        }
	int retval=virDomainUndefine(dom);
        virDomainFree(dom);
        virConnectClose(conn);
        return retval;
}

int insert_string(const char * before, const char *after,char **text,const char* insert_text){

	char *start=strstr(*text,before);
	char *end=strstr(*text,after);

	if(start&&end){
		start+=strlen(before);
		char *tmp=(char*)malloc(sizeof(char) *(strlen(*text)+strlen(insert_text)-(end-start)));
		strncpy(tmp,*text,(start)-(*text));
		tmp[(start)-(*text)]='\0';
		strcat(tmp,insert_text);
		strcpy(*text,end);
		strcat(tmp,*text);
		free(*text);
		*text=tmp;
		return 0;
	}
	return -1;
	

}

int edit_vm(const char *domain,const char *hypervisor,const char *memory, const char *cpu, const char *vnc_port){
	int retval=-1;
	virConnectPtr conn=virConnectOpen(hypervisor);
	if(!conn){
		bail("connection failed.");
		return retval;
	}
	virDomainPtr dom=virDomainLookupByName(conn,domain);
	if(!dom){
                bail("Domain lookup failed.");
                return retval;
        }
	char *xml=virDomainGetXMLDesc(dom,VIR_DOMAIN_XML_INACTIVE);
	insert_string("<memory unit='KiB'>","</memory>",&xml,memory);
	insert_string("<currentMemory unit='KiB'>","</currentMemory>",&xml,memory);
	insert_string("<vcpu placement='static'>","</vcpu>",&xml,cpu);
	insert_string("<graphics type='vnc' port='","' autoport='no' listen='0.0.0.0'>",&xml,vnc_port);
	virDomainFree(dom);
	dom=virDomainDefineXML(conn,xml);
	if(dom)
		retval=0;

	virDomainFree(dom);
	virConnectClose(conn);
	free(xml);
	return retval;
}

int get_virNaegling_ip_aux(char **ip,char *xml,const char *mac,const char *domain){
        int i;
        char *str;
        char *rest;
        char line[256];
        sprintf(line,"<host mac='%s' name='%s' ip='",mac,domain);
        str=strtok_r(xml,"\n",&rest);
        char *start;
        while(str){
                str=strtok_r(NULL,"\n",&rest);
                if((start=strstr(str,line))!=NULL)
                        break;
        }
        if(start){
                memcpy(line,start+strlen(line),strlen(str)-(start-str));
                for(i=0;i<strlen(line);++i)
                        if(line[i]=='\''){
                                line[i]='\0';
                                break;
                        }
                strcpy(*ip,line);
                return 0;
        }

        return -1;


}

char * get_virNaegling_ip(const char *domain,const char *hypervisor,const char *mac){
        int retval;
        virConnectPtr conn=virConnectOpen(hypervisor);
        /*if(!conn){
                bail("connection failed.");
                return -1;
        }*/
        virNetworkPtr dom=virNetworkLookupByName(conn,"virNaegling");
        /*if(!dom){
                bail("Domain lookup failed.");
                return -1;
        }*/
        char *xml=virNetworkGetXMLDesc(dom,0);
        char *ip=(char*)malloc(sizeof(char)*20);
        retval=get_virNaegling_ip_aux(&ip,xml,mac,domain);

        virNetworkFree(dom);
        virConnectClose(conn);
        free(xml);
        if(!retval)
                return ip;
        else
                return NULL;
}

