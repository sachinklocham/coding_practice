#include <string>
#include <dirent.h> 
#include <sys/stat.h>
#include <time.h>

#define DEL_2HR_OLD_FILE 7200

unsigned long int timestamp(std::string filename)
{
    struct stat filestat;
    stat(filename.c_str(),&filestat);
    unsigned long int t_stamp_sec = filestat.st_mtime;	
    printf("File modify time %s\n",ctime(&filestat.st_mtime));
    //printf("File modify time in sec : %lu\n",filestat.st_mtime);
    return t_stamp_sec;
}

int list_dir_subdir_files(std::string dir_name,unsigned long int delete_before_ts) 
{ 
	struct dirent *de; // Pointer for directory entry 
	if(dir_name.find("Audio")!= std::string::npos){//skipping Audio directory
		return 0;
	} 
	if(dir_name.find("Recordings")!= std::string::npos){//skipping Recordings directory
		return 0;
	} 
	if(dir_name.find(".jpeg")!= std::string::npos){//finding timestamp of jpeg files
            if(delete_before_ts > timestamp(dir_name)){
	    	//printf("Removing jpeg file: %s\n", dir_name.c_str());	
		remove(dir_name.c_str());	
	    }	
        }
	DIR *dr = opendir(dir_name.c_str()); 
	if (dr == NULL) // opendir returns NULL if couldn't open directory 
	{ 
		return 0; 
	} 
	//printf("::%s\n", dir_name.c_str()); 
	while ((de = readdir(dr)) != NULL){ 
		if(de->d_name[0]!='.' ){
			list_dir_subdir_files(dir_name+"/"+de->d_name, delete_before_ts);//recursively creating path and checking sub-directiories
		}
	}
	closedir(dr);	 
	return 0; 
}

int main(){
	unsigned long int del_before_sec;
	del_before_sec = time(NULL) - DEL_2HR_OLD_FILE;
	//list_dir_subdir_files("/media/user",del_before_sec);
	//list_dir_subdir_files("/home/user/nfs_share/SDCARD",del_before_sec);
	list_dir_subdir_files("/mnt/sdcard0",del_before_sec);
return 0;
}






