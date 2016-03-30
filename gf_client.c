#include <stdio.h>
#include <stdlib.h>
#include <time.h> 
#include "glfs.h"
#include "glfs-handles.h"
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>


#define  L (1024 * 1024 *1024)

void  peek_stat(struct stat *sb);


int is_dir(char *path)
{
	struct stat st;
	stat(path, &st);
 	peek_stat(&st);	
	if(S_ISDIR(st.st_mode)){
		return 1;
	}else{
		return 0;
	}

}
int glfs_is_dir(glfs_t *fs, char *path)
{
	struct stat st;
	glfs_lstat(fs, path, &st);
	if(S_ISDIR(st.st_mode)){
		return 1;
	}else{
		return 0;
	}	
}

int endwith(const char *s, char c)
{
	if(s[strlen(s) - 1] == c){
		return 1;
	}else{
		return 0;
	}
}

char *str_contact(const char *strA,const char *strB)
{
	char *s;
	s = (char *) malloc (strlen(strA) + strlen(strB) + 1);
	if(!s){
		fprintf(stderr, "Malloc Failed : %s\n", strerror(errno));
		exit(0);
	}
	strcpy(s, strA);
	strcat(s, strB);
	return s;	
}




void
peek_stat (struct stat *sb)
{
        printf ("Dumping stat information:\n");
        printf ("File type:                ");

        switch (sb->st_mode & S_IFMT) {
                case S_IFBLK:  printf ("block device\n");            break;
                case S_IFCHR:  printf ("character device\n");        break;
                case S_IFDIR:  printf ("directory\n");               break;
                case S_IFIFO:  printf ("FIFO/pipe\n");               break;
                case S_IFLNK:  printf ("symlink\n");                 break;
                case S_IFREG:  printf ("regular file\n");            break;
                case S_IFSOCK: printf ("socket\n");                  break;
                default:       printf ("unknown?\n");                break;
        }

        printf ("I-node number:            %ld\n", (long) sb->st_ino);

        printf ("Mode:                     %lo (octal)\n",
                (unsigned long) sb->st_mode);

        printf ("Link count:               %ld\n", (long) sb->st_nlink);
        printf ("Ownership:                UID=%ld   GID=%ld\n",
                (long) sb->st_uid, (long) sb->st_gid);

        printf ("Preferred I/O block size: %ld bytes\n",
                (long) sb->st_blksize);
        printf ("File size:                %lld bytes\n",
                (long long) sb->st_size);
        printf ("Blocks allocated:         %lld\n",
                (long long) sb->st_blocks);
//	time_t rawtime;
//	time(&rawtime);
	
	printf ("Last status change:       %s", ctime(&sb->st_ctime));
	printf ("Last file access:         %s", ctime(&sb->st_atime));
	//printf ("The Current Local Time : %s\n", ctime(&rawtime));
        printf ("Last file modification:   %s", ctime(&sb->st_mtime));

        return;
}


void ls_cmd(glfs_t *fs, glfs_fd_t *fd, const char *path)
{
//      const char *path = argv[4];
//	glfs_t  *fs = NULL;
//	glfs_fd_t *fd = NULL;
//	fs = glfs_new(volname);
//      glfs_set_volfile_server(fs, "rmda", serverIP, 24007);
//      glfs_set_logging(fs, "/dev/null", 3); 
//      glfs_init(fs);
        if(path == NULL){
		int ret = -1;
		char *respath=NULL;
		char pathbuf[4096];
		respath = glfs_getcwd(fs, pathbuf, 4096);
		path = respath;	
	}     
        char buf[512];
	struct stat  sb;
        struct dirent *filename = NULL; 
        fd = glfs_opendir(fs, path);
	if (!fd){
		fprintf(stderr, "%s: %s\n",path,  strerror(errno));
		exit(0); 
	}   
	fprintf(stderr, "Entries:\n");
	char *filepath;
	while ((filename = glfs_readdir(fd))!=NULL){
		fprintf(stderr, "%s\n", filename->d_name);
		filepath = str_contact(path,filename->d_name);
		fprintf(stdout, "filepath: %s\n", filepath);
		int ret = -1;
		ret = glfs_lstat(fs, filepath, &sb);
		if(ret != 0){
			fprintf(stderr, "glfs_lstat(%s): %s\n", filepath, strerror(errno));
		}
		peek_stat(&sb);
								
	}
	
/*	while (glfs_readdir_r (fd, (struct dirent *) buf, &entry),entry) {
		fprintf(stderr, "%s: %lu\n", entry->d_name, entry->d_reclen);
		int ret = -1;
		ret = glfs_lstat(fs, entry->d_name, &sb);
		if(ret){
			fprintf(stderr, "lstat (%s): (%d)  %s\n", entry->d_name, ret, strerror(errno));
			exit(0);
		} 
		peek_stat(&sb); 
	} 
*/  
	glfs_closedir(fd);
//	glfs_fini(fs);
}

void mkdir_cmd(glfs_t *fs, const char *mkdirname)
{
        int ret = -1;
        char *respath = NULL;
        char pathbuf[4096];
        respath = glfs_getcwd(fs, pathbuf, 4096);
        fprintf(stderr, "getcwd() = %s\n", respath);

        ret = glfs_mkdir(fs, mkdirname, 0755);
        if(ret){
                fprintf(stderr, "mkdir(%s): %s \n", mkdirname, strerror(errno));
                exit(0);
        }
	fprintf(stdout, "mkdir(%s) success!\n", mkdirname);

}


void put_cmd(glfs_t *fs, glfs_fd_t *fd, const char *filename, const char *destfilename)
{
/*	glfs_t *fs = NULL;
	glfs_fd_t *fd = NULL;
	fs = glfs_new(volname);
	glfs_set_volfile_server(fs, "rdma", serverIP, 24007);
	glfs_set_logging(fs, "/dev/null",3);
	glfs_init(fs);
*/

//	const char *filename=argv[4];
	if(destfilename == NULL){
		destfilename = filename;
	}
	FILE *fp;
	fd  = glfs_creat(fs, filename, O_RDWR, 0644);

	if((fp=fopen(filename, "r"))==NULL){
		fprintf(stderr, "Can not open the file %s", filename);
		exit(0);
	}else{
		char *s;
		s = (char *)malloc(sizeof(char) * L);
		//printf("hello\n");
		while(1){
			int ret = fread(s, 1, L, fp);
			//printf("content:%s\n", s);
			if (ret < L){
				glfs_write(fd, s, strlen(s), 0);
				break;
			}else{
				glfs_write(fd, s, L, 0);
			}
		}
		glfs_fsync(fd);
		glfs_close(fd);
		fclose(fp);
	}
//	glfs_fini(fs);
			
}

void put_cmdr(glfs_t *fs, glfs_fd_t *fd, const char *dirname, const char *destdirname)
{
	fd = glfs_opendir(fs, destdirname);
	if(!fd){
		mkdir_cmd(fs, destdirname);
	}
	char *path;
	path = (char *)malloc(512);
       // char path[512];
        memset(path,0, sizeof(path));
        printf("%s \n", dirname);
	path = str_contact(path, dirname);
	struct dirent *filename;
	//printf("hello1 %s\n", path);
	DIR *dp = opendir(path);
	//printf("hello3\n");
	while((filename = readdir(dp)) !=NULL){
		//printf("hello2\n");
//		memset(path, 0, sizeof(path));
		//path = str_contact(path, dirname);
		char * file_source_path;
		file_source_path = (char *)malloc (512);
		memset(file_source_path,0,sizeof(file_source_path));
		if(!(endwith(dirname, '/'))){
			file_source_path = str_contact(file_source_path, dirname);
			file_source_path = str_contact(dirname, "/");
		}else{
			file_source_path = str_contact(file_source_path, dirname);
		}
		char *file_dest_path;
		file_dest_path = (char *)malloc(512);
		memset(file_dest_path, 0, sizeof(file_dest_path));
		if(!endwith(destdirname, '/')){
			file_dest_path = str_contact(file_dest_path, destdirname);
			file_dest_path = str_contact(destdirname, "/");	
		}else{
			file_dest_path = str_contact(file_dest_path, destdirname);
		}
		file_source_path = str_contact(file_source_path, filename->d_name);
		file_dest_path = str_contact(file_dest_path, filename->d_name);
		//printf("file_source_path: %s\n", file_source_path);
		//printf("file_dest_path: %s \n", file_dest_path);
		if(is_dir(file_source_path)){
			if(!(endwith(file_source_path, '.'))){
				//fprintf(stdout, "hi {%s}\n", file_source_path);
				put_cmdr(fs, fd, file_source_path, file_dest_path);
			}
		}else{
			//fprintf(stdout, "hello{ %s} \n", file_source_path);
			put_cmd(fs, fd, file_source_path, file_dest_path);
			fprintf(stderr, "Copy %s to  %s success!\n", file_source_path, file_dest_path); 
		}
	}
	closedir(dp);
		
}

void get_cmd(glfs_t *fs , glfs_fd_t *fd, const char *filename, const char *destdirname)
{
	
//	glfs_t *fs = NULL;
		
		
//	const char *filename = argv[4];
//	const char *destdirname = argv[5];
	char *destfilename;
	destfilename = (char *)malloc(512);
	char *destdirname_d;
	destdirname_d = (char *)malloc(512);
	if(endwith(destdirname, '/')){
		destfilename = filename;
		destdirname_d = destdirname;
	}else{
		int i;
		for(i = strlen(destdirname) - 1; i>=0; i--){
			if(destdirname[i] == '/'){
				break;
			}
		}
		int j;
		for (j = i + 1;  j <= strlen (destdirname) - 1; j++){
			printf("%c", destdirname[j]); 
		}
		printf("\n jhehehe\n");
		strncpy(destfilename, destdirname+i+1, strlen(destdirname)  - i);
		//destfilename[strlen(destdirname) - i] = '\0';
		printf("%s\n", destfilename);
		//char *split = "/";
		//char *p =
		strncpy(destdirname_d, destdirname, i + 1); 	
	}
	
	fd = glfs_open(fs, filename, O_RDWR);
	if(fd == NULL){
		fprintf(stderr, "Cannot find source file %s\n", filename);
		exit(0);
	}else{
		char *s;
		s = (char *)malloc(sizeof(char) * L);
		//printf("hello\n");
		FILE *fp;
		char *destfilename_f = str_contact(destdirname_d, destfilename);
		//strcpy(destfilename_d, destdirname_d);
		//strcat(destfilename_d, destfilename);
		printf("destfilename, %s\n", destfilename_f);


		if((fp=fopen(destfilename_f, "w")) == NULL){
			fprintf(stderr, "Cannot Open the file %s\n", destfilename_f);
			exit(0);
		}
		while(1){
			int ret = glfs_read(fd, s, L, 0);
			printf("Hi \n");
			printf("%d %s\n",ret,s);
			if(ret < L){
				fwrite(s,1,strlen(s),fp);
				break;
			}else{
				fwrite(s,1,L,fp);
			}
		}
		fclose(fp);
		glfs_close(fd);
	}
//	glfs_fini(fs);	
}

void get_cmdr(glfs_t *fs, glfs_fd_t *fd, const char *srcdirname, const char *destdirname)
{
	if(!opendir(destdirname)){
		if(mkdir(destdirname, 0777)){
			fprintf(stderr, "mkdir (%s):%s \n", destdirname, strerror(errno));
			exit(0);
		}
	}
	char *path;
	path = (char *)malloc(512);
	memset(path, 0, sizeof(path));
	path = str_contact(path, srcdirname);
	fd  = glfs_opendir(fs, path);
	struct dirent *filename;
	if(!fd){
		fprintf(stderr, "%s:  %s\n", path, strerror(errno));
		exit(0);
	}
	while((filename = glfs_readdir(fd))!=NULL){
                char * file_source_path;
                file_source_path = (char *)malloc (512);
                memset(file_source_path,0,sizeof(file_source_path));
                if(!(endwith(srcdirname, '/'))){
                        file_source_path = str_contact(file_source_path,srcdirname);
                        file_source_path = str_contact(srcdirname, "/");
                }else{
                        file_source_path = str_contact(file_source_path, srcdirname);
                }
                char *file_dest_path;
                file_dest_path = (char *)malloc(512);
                memset(file_dest_path, 0, sizeof(file_dest_path));
                if(!endwith(destdirname, '/')){
                        file_dest_path = str_contact(file_dest_path, destdirname);
                        file_dest_path = str_contact(destdirname, "/");
                }else{
                        file_dest_path = str_contact(file_dest_path, destdirname);
                }
                file_source_path = str_contact(file_source_path, filename->d_name);
                file_dest_path = str_contact(file_dest_path, filename->d_name);
		fprintf(stdout, "file_source_path:%s\n", file_source_path);
		fprintf(stdout, "file_dest_path:%s\n", file_dest_path);
		if(glfs_is_dir(fs,file_source_path)){
			printf("hello!\n");
			if(!endwith(file_source_path, '.')){
				get_cmdr(fs, fd, file_source_path, file_dest_path);
			}
		}else{
			get_cmd(fs, fd, file_source_path, file_dest_path);
			fprintf(stdout, "Copy %s to %s success!\n", file_source_path, file_dest_path);
		}
		
	}
}

void cd_cmd(glfs_t *fs, const char *dir)
{
	int ret = -1;
	ret = glfs_chdir(fs, dir);
	if(ret){
		fprintf(stderr, "changetodir(%s): %s\n", dir, strerror(errno));
		exit(0);
	}
}

		




int main(int argc, char *argv[])
{
	if(argc < 2){
		printf("Usage: %s", argv[0]); 
	}
	const char *volname = argv[1];
	const char *serverIP= argv[2];
	glfs_t *fs = NULL;
	glfs_fd_t *fd = NULL;
	fs = glfs_new(volname);
	if(!fs){
		fprintf(stderr, "glfs_new: Return NULL\n");
		exit(0);
	}
	glfs_set_volfile_server(fs,"rdma", serverIP, 24007);
	glfs_set_logging(fs, "/dev/null", 3);
	glfs_init(fs);
	
	const char *cmd = argv[3];
	
	//const char *path = argv[4];
	if(strcmp(cmd,"ls") == 0){
		 const char *path = argv[4];
		 ls_cmd(fs, fd, path);
	}
	else if(strcmp(cmd,"put") == 0){
		const char *pra = argv[4];
		if(strcmp(pra, "-r") == 0){
			const char *dirname = argv[5];
			const char *destdirname = argv[6];
			put_cmdr(fs, fd, dirname, destdirname);
			
		}else{
			const char *filename = argv[4];
			const char *destdirname = argv[5];
			put_cmd(fs, fd, filename, destdirname);
		}		

/*		glfs_t *fs = NULL;
		glfs_fd_t *fd = NULL;
		fs = glfs_new(volname);
		glfs_set_volfile_server(fs, "rdma", serverIP, 24007);
		glfs_set_logging (fs, "/dev/null", 3);
		glfs_init(fs);
		const char *pra = argv[4];
		if (strcmp(pra, "-r") == 0){
			const char *dirname = argv[4];
			
		}else{
			const char *filename=argv[4];
			
			FILE *fp;
			fd  = glfs_creat(fs, filename, O_RDWR, 0644);
		 	

	
			if((fp=fopen(filename, "r"))==NULL){
				fprintf(stderr, "Can not open the file %s", filename);
				exit(0);
			}else{
				char *s;
				s = (char *)malloc(sizeof(char) * L);
				printf("hello\n"); 
			        while(1){
					int ret = fread(s, 1, L, fp);
					printf("content:%s\n", s);
					if (ret < L){
						glfs_write(fd, s, strlen(s), 0);
						break;
					}else{
						glfs_write(fd, s, L, 0);
					}		
				}
				glfs_fsync(fd);
				glfs_close(fd);
				fclose(fp);	
			}
		
		}
		glfs_fini(fs);				
*/		
	}
	else if(strcmp(cmd, "get") == 0){
		const char *pra = argv[4];
		
		if(strcmp(pra, "-r") == 0){
			const char *srcdirname = argv[5];
			const char *destdirname = argv[6];
			get_cmdr(fs,fd, srcdirname, destdirname); 
		}else{
			const char *filename=argv[4]; 
			const char *destdirname = argv[5];
			get_cmd(fs, fd, filename, destdirname);	
		}
	}
	else if(strcmp(cmd,"mkdir") == 0){
	//	const char *parentdir = argv[4];
		const char *mkdirname = argv[4];
		fprintf(stderr, "argv[3] = %s\n", argv[3]);
		mkdir_cmd(fs, mkdirname);

	}
	glfs_fini(fs);			 
	return 0;
}
		
