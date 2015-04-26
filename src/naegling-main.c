#include "naegling-main.h"
const char *NAEGLING_PATH={"/opt/naegling"};
const char *NAEGLING_TEMPLATE_PATH={"/opt/naegling/templates"};
const char *NAEGLING_JOBS_PATH={"/opt/naegling/jobs"};
const char *DATABASE_PATH={"/opt/naegling/naegling.db"};
const char *NAEGLING_TEMPLATES_FILE_LIST={"/opt/naegling/templates.dat"};
const char *MESSAGE_DELIMITER={"#"};
const char *NAEGLING_TEMPLATES_DIRECTORY={"/opt/naegling/templates/"};
const char *VIR_NAEGLING_NETWORK={"192.168.125."};
int FILE_TRANSFER_AVAILABLE=1;
sem_t sem;




int main (int argc, char *argv[]){

	/*
	 * Initialize semaphore
         */
	sem_init(&sem,0,1);


	/*
         * Naegling virtual network creation
         */
        int retval=start_virtual_network_naegling();
        if(retval==-1){
                naegling_log("Could not start naegling virtual network. Trying to create....");
                retval=create_virtual_network_naegling();
                if(retval){
                        bail("Could not start naegling virtual network.");
                }else{
                        naegling_log("Naegling virtual network started.");
                }
        }else{
                naegling_log("Naegling virtual network started.");
        }
        /*
         * End of naegling virtual network creation
         */


        /*
         * Directory creation
         */
	/*
	 * Check if naegling directory exists, if not create
	 */
	struct stat status;
	retval=stat(NAEGLING_PATH,&status);
	if(retval==-1){
		naegling_log("Creating naegling directory...");
		retval=mkdir(NAEGLING_PATH,S_IRWXU|S_IRWXG|S_IROTH);
		if(retval==-1){
			bail("Error creating naegling directory.");
			fprintf(stderr,"Could not start naegling daemon.\n");
			return -1;
		}
	}else{
		naegling_log("Naegling directory exists.");
	}

	/*
         * Check if naegling templates directory exists, if not create
         */
	retval=stat(NAEGLING_TEMPLATE_PATH,&status);
	if(retval==-1){
		naegling_log("Creating naegling templates directory...");
                retval=mkdir(NAEGLING_TEMPLATE_PATH,S_IRWXU|S_IRWXG|S_IROTH);
                if(retval==-1){
                        bail("Error creating naegling template directory.");
			fprintf(stderr,"Could not start naegling daemon.\n");
                        return -1;
                }
	}else{
		naegling_log("Naegling template directory exists.");
	}
	/*
	 * End of directory creation
	 */



        /*
         * Check if naegling jobs directory exists, if not create
         */
        retval=stat(NAEGLING_JOBS_PATH,&status);
        if(retval==-1){
                naegling_log("Creating naegling JOBS directory...");
                retval=mkdir(NAEGLING_JOBS_PATH,S_IRWXU|S_IRWXG|S_IROTH);
                if(retval==-1){
                        bail("Error creating naegling jobs directory.");
                        fprintf(stderr,"Could not start naegling daemon.\n");
                        return -1;
                }
        }else{
                naegling_log("Naegling jobs directory exists.");
        }
        /*
         * End of directory creation
         */



	/*
	 * Database creation
	 */
	if(!db_exists()){
		create_db();
	}
	/*
 	 * End of Database creation
	 */


	/*
	 * Creating the daemon
	 */
	 pid_t pid, sid;
        
        /*
	 * Fork off the parent process 
	 */
        pid = fork();
        if (pid < 0) {
		bail("Error forking process.");
		fprintf(stderr,"Could not start naegling daemon.\n");
                return -1;
        }

        /* 
	 * If we got a good PID, then we can exit the parent process.
	 */
        if (pid > 0) {
		naegling_log("Process forked successfully.");
                return 0;
        }

        /* Change the file mode mask */
        umask(0);

	/* Create a new SID for the child process */
        sid = setsid();
        if (sid < 0) {
                bail("Error creating child process SID.");
		fprintf(stderr,"Could not start naegling daemon.\n");
                return -1;
        }

	/* Change the current working directory */
        if ((chdir("/")) < 0) {
                bail("Error changing working directory.");
		fprintf(stderr,"Could not start naegling daemon.\n");
		return -1;
        }


	fprintf(stdout,"Naegling daemon started successfully.\n");

#if 0
FIXME
Can not close file descriptor because libvirt uses it.
	/* Close out the standard file descriptors */
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
#endif
	/*
	 * End of daemon creation
	 */


	/*Main thread deals with the routines of naegling*/
	listen_for_remote_message();

	return 0;
}
