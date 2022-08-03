#include <stdio.h>
#include "ext2_fs.h"
#include "read_ext2.h"
#include <dirent.h>
#include <string.h>
#include <assert.h>

// Lilly Boyd and Vigna Gajarla

/*
- create output directory (argv[2]), if it already exists throw error and exit, use  opendir(name)
- make fd, read fd
- read super block and group desc (code is given to us)
- for loop through inode blocks to get inode info, find out if it is a file or directory
	- only continue if it is a file????
- then check each inodes first data block (inode->i_block[0]??, put this into char buffer????)for magic numbers
	- then set jpeg value to 1 if numbers match and 0 otherwise

YOU HAVE NOW IDENTIFIED IF INODE IS A JPEG OR NOT

- copy contents of file to output file (figure out how to do this, copying a data block)
	- create a file with inode number (file-18.jpg), you will have to use FS calls (look in textbook)

- look through all data blocks to find the name of the jpg files
	- first 2 entries are "." and ".."
	- after this, look through each dir entry and see if the inode number from that entry (a new struct that
	you will create) matches the "i" of the for loop
	- use code given to extract the actual file name and then create it and copy the contents
*/

int main(int argc, char **argv)
{

	if (argc != 3)
	{
		printf("expected usage: ./runScan inputfile outputfile\n");
		exit(0);
	}

	// creating output directory
	char *outputName = argv[2];
	DIR *dir = opendir(outputName);
	if (dir != NULL)
	{
		printf("ERROR: Directory already exists.\n");
		exit(1);
	}

	int check = mkdir(outputName, 0777);
	if (check != 0)
	{
		printf("ERROR: Unable to create directory.\n");
		exit(1);
	}

	// making and reading fd
	int fd;
	fd = open(argv[1], O_RDONLY); /* open disk image */
	ext2_read_init(fd);

	struct ext2_super_block super;
	struct ext2_group_desc group;

	read_super_block(fd, 0, &super);
	read_group_desc(fd, 0, &group);
	off_t start_inode_table = locate_inode_table(0, &group);

	// loop through inode table and inodes
	for (unsigned int j = 0; j < num_groups; j++)
	{
		for (unsigned int i = 0; i < super.s_inodes_per_group; i++)
		{

			struct ext2_inode *inode = malloc(sizeof(struct ext2_inode));
			read_inode(fd, 0, start_inode_table, i, inode);

			unsigned int i_blocks = inode->i_blocks / (2 << super.s_log_block_size);

			uint SIZE = inode->i_size;

			if (S_ISREG(inode->i_mode))
			{
				// check if first data block of inode contains magic nums
				char buffer[1024];
				lseek(fd, BLOCK_OFFSET(inode->i_block[0]), SEEK_SET);
				read(fd, buffer, 1024);

				// checking the magic numbers
				int is_jpg = 0;
				if (buffer[0] == (char)0xff &&
					buffer[1] == (char)0xd8 &&
					buffer[2] == (char)0xff &&
					(buffer[3] == (char)0xe0 ||
					 buffer[3] == (char)0xe1 ||
					 buffer[3] == (char)0xe8))
				{
					is_jpg = 1;
				}

				if (!is_jpg)
				{
					continue;
				}

				// creating the file path
				char filePath[256];
				sprintf(filePath, "%s/file-%i.jpg", outputName, i);

				// copying contents into file
				int new_file = open(filePath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
				// open failed
				if (new_file == -1)
				{
					printf("Error: could not open file: %s", filePath);
				}
				// print i_block numbers
				for (unsigned int i = 0; i < EXT2_N_BLOCKS; i++)
				{
					if (i < EXT2_NDIR_BLOCKS)
					{ /* direct blocks */
						char *buffer[1024];
						lseek(fd, BLOCK_OFFSET(inode->i_block[i]), SEEK_SET);
						read(fd, buffer, 1024);
						if (SIZE == 0)
						{
							break;
						}
						else if (SIZE < 1024)
						{
							write(new_file, buffer, SIZE);
							SIZE -= SIZE;
							break;
						}
						else
						{
							write(new_file, buffer, 1024);
							SIZE -= 1024;
						}
					}
					else if (i == EXT2_IND_BLOCK)
					{ /* single indirect block */
						uint buffer[1024 / 4];
						lseek(fd, BLOCK_OFFSET(inode->i_block[i]), SEEK_SET);
						read(fd, (char *)buffer, 1024);

						for (uint j = 0; j < 1024 / 4; j++)
						{
							char dataBuffer[1024];
							if (j <= i_blocks)
							{
								lseek(fd, BLOCK_OFFSET(buffer[j]), SEEK_SET);
								read(fd, dataBuffer, 1024);
								if (SIZE == 0)
								{
									break;
								}
								else if (SIZE < 1024)
								{
									write(new_file, dataBuffer, SIZE);
									SIZE -= SIZE;
									break;
								}
								else
								{
									write(new_file, dataBuffer, 1024);
									SIZE -= 1024;
								}
							}
							else
							{
								break;
							}
						}
					}
					else if (i == EXT2_DIND_BLOCK)
					{ /* double indirect block */
						uint buffer[1024 / 4];
						lseek(fd, BLOCK_OFFSET(inode->i_block[i]), SEEK_SET);
						read(fd, (char *)buffer, 1024);

						for (uint j = 0; j < 1024 / 4; j++)
						{
							uint dataBuffer[1024];
							if (j <= i_blocks)
							{
								lseek(fd, BLOCK_OFFSET(buffer[j]), SEEK_SET);
								read(fd, (char *)dataBuffer, 1024);
								for (uint k = 0; k < 1024 / 4; k++)
								{
									char doubleBuffer[1024];
									if (k <= i_blocks)
									{
										lseek(fd, BLOCK_OFFSET(dataBuffer[k]), SEEK_SET);
										read(fd, doubleBuffer, 1024);
										if (SIZE == 0)
										{
											break;
										}
										else if (SIZE < 1024)
										{
											write(new_file, doubleBuffer, SIZE);
											SIZE -= SIZE;
											break;
										}
										else
										{
											write(new_file, doubleBuffer, 1024);
											SIZE -= 1024;
										}
									}
									else
									{
										break;
									}
								}
							}
							else
								break;
						}
					}
				}
			}
			free(inode);
		}
	}

	for (unsigned int j = 0; j < num_groups; j++)
	{
		for (unsigned int i = 0; i < super.s_inodes_per_group; i++)
		{
			struct ext2_inode *inode = malloc(sizeof(struct ext2_inode));
			read_inode(fd, 0, start_inode_table, i, inode);

			if (S_ISDIR(inode->i_mode))
			{
				char buffer[1024];
				lseek(fd, BLOCK_OFFSET(inode->i_block[0]), SEEK_SET);
				read(fd, buffer, 1024);

				int offset = 0;
				struct ext2_dir_entry *dentry = (struct ext2_dir_entry *)&(buffer[offset]);
				int name_len = dentry->name_len & 0xFF;
				for (uint x = 0; x < inode->i_block[0]; x++)
				{
					dentry = (struct ext2_dir_entry *)&(buffer[offset]);
					name_len = dentry->name_len & 0xFF; // convert 2 bytes to 4 bytes properly
					if (name_len != 0)
					{
						char name[EXT2_NAME_LEN];
						strncpy(name, dentry->name, name_len);
						name[name_len] = '\0';
						// calculate offset, make sure name_len is 4 byte aligned
						if (name_len % 4 != 0)
							offset += 8 + name_len + (4 - (name_len % 4));
						// name_len is already aligned
						else
							offset += 8 + name_len;
						// now checking if file exists
						char filePath[256];
						sprintf(filePath, "%s/file-%i.jpg", outputName, dentry->inode);
						FILE *inodeFileName;
						inodeFileName = fopen(filePath, "r");
						if (inodeFileName == NULL)
						{
							continue;
						}
						else
						{
							char copyBuffer[1024];
							char nameWithPath[256];
							sprintf(nameWithPath, "%s/%s", outputName, name);
							FILE *actualName = fopen(nameWithPath, "w");
							while (!feof(inodeFileName))
							{
								int length = fread(copyBuffer, 1, 1024, inodeFileName);
								fwrite(copyBuffer, 1, length, actualName);
							}
							fclose(inodeFileName);
							fclose(actualName);
						}
					}
				}
			}
			free(inode);
		}
	}
}
