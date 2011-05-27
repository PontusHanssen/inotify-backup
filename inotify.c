#include <dirent.h>
#include <sys/inotify.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

void check_dir(char *search_dir, int fd);
void check_dir(char *search_dir, int fd)
{
	DIR *dir;
        struct dirent *ent;
        dir = opendir (search_dir);
        if (dir != NULL)
	{

	        /* print all the files and directories within directory */
	        while ((ent = readdir (dir)) != NULL)
		{
	        	if(ent->d_type & DT_DIR)
		        {
				if(ent->d_name[0] != '.' || strlen(ent->d_name)>2)
				{
				/*
				printf("Letar i mappen %s\n", search_dir);
				printf("Hittade den nya mappen %s\n", ent->d_name);
				*/

				char *directory = malloc(strlen(search_dir) + strlen(ent->d_name) + 2);
				strcpy(directory,search_dir);
				strcat(directory,"/");
				strcat(directory,ent->d_name);

				inotify_add_watch(fd,directory,IN_MODIFY | IN_CREATE | IN_DELETE);
				check_dir(directory, fd);
				}
		        }                
		}
	}
	closedir (dir);
	}


int main()

{
	#define EVENT_SIZE (sizeof (struct inotify_event))
	#define BUF_LEN (1024 * (EVENT_SIZE + 16))

	char buf[BUF_LEN];
	char *dir = "/home/persson/tmp";
	int fd, wd, len, i=0;
	fd = inotify_init();
	if(fd<0)
		perror("inotify_init");
	
	wd = inotify_add_watch(fd, dir, IN_MODIFY | IN_CREATE | IN_DELETE);
	check_dir(dir, fd);
	if(wd<0)
		perror("inotify_add_watch");
while(1)
{
	len = read(fd, buf, BUF_LEN);
	if(len<0)
	{
		if(errno == EINTR)
		{
		}
		else
		{
			perror("read");
		}
	}
	else if(!len)
		printf("BUF_LEN too small?\n");
	
	while(i<len)
	{
		struct inotify_event *event;
		event = (struct inotify_event *) &buf[i];
		if(event->len)
		{
			if(event->mask & IN_CREATE)
			{
				if(event->mask & IN_ISDIR)
				{
					printf("Mappen %s skapades.\n", event->name);
					char *new_dir = malloc(strlen(dir) + strlen(event->name) + 2);
                                	strcpy(new_dir,dir);
                                	strcat(new_dir,"/");
                                	strcat(new_dir,event->name);
					check_dir(new_dir,fd);
				}
				else
				{
					printf("Filen %s skapades.\n", event->name);
				}
			}
			else if(event->mask & IN_MODIFY)
			{
				printf("Filen %s ändrades.\n", event->name);
			}
			else if(event->mask & IN_DELETE)
			{
				if(event->mask & IN_ISDIR)
				{
					printf("Mappen %s togs bort.\n", event->name);
					inotify_rm_watch(fd, event->wd);
				}
				else
				{
					printf("Filen %s togs bort.\n", event->name);
				}
			}
		}
		/*printf("wd=%d mask=%u cookie=%u len=%u\n", event->wd, event->mask, event->cookie, event->len);

		if(event->len)
			printf("name=%s\n", event->name);
		*/
		i += EVENT_SIZE + event->len;
	}
	i=0;
}
	return 0;
}

	
	
	
