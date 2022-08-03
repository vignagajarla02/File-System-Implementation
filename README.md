# File-System-Implementation

Project Description:

  You need to write a program called runScan that takes two arguments, an input file that contains the disk image and an output directory where your 
  output files will be stored. The tasks of your program are as follows:
  First, you need to reconstruct all jpg files (both undeleted and deleted ones) from a given disk image. To do this, you need to scan all inodes that 
  represent regular files and check if the first data block of the inode contains the jpg magic numbers: FF D8 FF E0 or FF D8 FF E1 or FF D8 FF E8. 
  (For more about file signatures, you can visit this page (Links to an external site.) ). For example, if you read the first data block of a file and 
  put it in char buffer [1024] , then this code will identify whether it is a jpg file or not:

              int is_jpg = 0;
              if (buffer[0] == (char)0xff &&
                  buffer[1] == (char)0xd8 &&
                  buffer[2] == (char)0xff &&
                  (buffer[3] == (char)0xe0 ||
                   buffer[3] == (char)0xe1 ||
                   buffer[3] == (char)0xe8)) {
                 is_jpg = 1;
              }
  Once you identify an inode that represents a jpg file, you should copy the content of that file to an output file (stored in your 'output/' directory), 
  using the inode number as the file name. For example, if you detect that inode number 18 is a jpg file, you should create a file 'output/file-18.jpg' 
  which will contain the exact data reachable from inode 18. (See the Example section below for more).
 
  The second part of your task is to find out the filenames of those inodes that represent the jpg files. Note that filenames are not stored in inodes, 
  but in directory data blocks. Thus, after you get the inode numbers of the jpg files, you should scan all directory data blocks to find the corresponding 
  filenames. After you get the filename of a jpg file, you should again copy the content of that file to an output file, but this time, using the actual 
  filename. For example, if inode number 18 is 'bank-uwcu.jpg', you should create a file 'output/bank-uwcu.jpg' which will be identical to 
  'output/file-18.jpg'.
  
  In summary, in your final output directory, for each jpg file, there should be a file with an inode number as the filename, and another file with the
  same filename as the actual one. When your program starts, your program should create the specified output directory. If the directory already exists, 
  print an error message (any message) and exit the program. You can know if a directory exists by using the opendir(name) (Links to an external site.) 
  system call.
  
  How to Run:
    1) Download all the provided files.
    2) Run the Makefile by typing "Make" in your projecct path.
    3) image-01 is a disk image that needs to be tranformed into jpg images. To do so, run "./runscan image-01 [output folder]" and replace [output folder]
      with desired output folder name that does not already exist in your directory.
    4) Once running that command, you will be able to view the decrytped images!
    
 Code Copyright: Vigna Gajarla and Lilly Boyd
