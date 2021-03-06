#include <sys/mman.h>      // For mmap(2)
#include <sys/stat.h>      // For stat(2)
#include <unistd.h>        // For everything else
#include <fcntl.h>         // O_RDONLY
#include <stdio.h>         // printf!
#include <CoreFoundation/CoreFoundation.h>

#include <mach-o/loader.h> // struct mach_header
#include "machlib.h"

/**
  *
  * A simple program to home in on XNU's system call table.
  * Coded specifically for iOS kernels. Seeks XNU version string
  * and signature of beginning of system call table. Then dumps
  * all system calls. Can work on the kernel proper, or the kernel cache.
  *
  * Can be easily adapted to OS X kernels as well (with changes for 64-bit
  * and endian-ness. Likewise adaptable for /dev/kmem, if you patched your iOS.
  *
  * System call names auto-generated from iOS's <sys/syscall.h>
  * (/Developer/Platforms/iPhoneOS.platform/DeviceSupport/Latest/Symbols/usr/include/sys)
  * 
  * can also be generated from OS X's <sys/syscall.h>, with minor tweaks (e.g. include
  *  ledger, pid_shutdown_sockets, etc..)
  *
  * Note, that just because a syscall is present, doesn't imply it's implemented -
  *  System calls can either point to nosys, or can be stubs returning an error code, 
  *  as is the case with audit syscalls (350-359), among others.
  * 
  * Tested on iOS 2.0 through 6.0
  *
  * Coded by Jonathan Levin
  *
  **/


#

char *syscall_names[] = { "syscall", "exit", "fork", "read", "write", "open", "close", "wait4", "8  old creat", "link", "unlink", "11  old execv", "chdir", "fchdir", "mknod", "chmod", "chown", "17  old break", "getfsstat", "19  old lseek", "getpid", "21  old mount", "22  old umount", "setuid", "getuid", "geteuid", "ptrace", "recvmsg", "sendmsg", "recvfrom", "accept", "getpeername", "getsockname", "access", "chflags", "fchflags", "sync", "kill", "38  old stat", "getppid", "40  old lstat", "dup", "pipe", "getegid", "profil", "45  old ktrace", "sigaction", "getgid", "sigprocmask", "getlogin", "setlogin", "acct", "sigpending", "sigaltstack", "ioctl", "reboot", "revoke", "symlink", "readlink", "execve", "umask", "chroot", "62  old fstat", "63  used internally , reserved", "64  old getpagesize", "msync", "vfork", "67  old vread", "68  old vwrite", "69  old sbrk", "70  old sstk", "71  old mmap", "72  old vadvise", "munmap", "mprotect", "madvise", "76  old vhangup", "77  old vlimit", "mincore", "getgroups", "setgroups", "getpgrp", "setpgid", "setitimer", "84  old wait", "swapon", "getitimer", "87  old gethostname", "88  old sethostname", "getdtablesize", "dup2", "91  old getdopt", "fcntl", "select", "94  old setdopt", "fsync", "setpriority", "socket", "connect", "99  old accept", "getpriority", "101  old send", "102  old recv", "103  old sigreturn", "bind", "setsockopt", "listen", "107  old vtimes", "108  old sigvec", "109  old sigblock", "110  old sigsetmask", "sigsuspend", "112  old sigstack", "113  old recvmsg", "114  old sendmsg", "115  old vtrace", "gettimeofday", "getrusage", "getsockopt", "119  old resuba", "readv", "writev", "settimeofday", "fchown", "fchmod", "125  old recvfrom", "setreuid", "setregid", "rename", "129  old truncate", "130  old ftruncate", "flock", "mkfifo", "sendto", "shutdown", "socketpair", "mkdir", "rmdir", "utimes", "futimes", "adjtime", "141  old getpeername", "gethostuuid", "143  old sethostid", "144  old getrlimit", "145  old setrlimit", "146  old killpg", "setsid", "148  old setquota", "149  old qquota", "150  old getsockname", "getpgid", "setprivexec", "pread", "pwrite", "nfssvc", "156  old getdirentries", "statfs", "fstatfs", "unmount", "160  old async_daemon", "getfh", "162  old getdomainname", "163  old setdomainname", "164", "quotactl", "166  old exportfs", "mount", "168  old ustat", "csops", "170  old table", "171  old wait3", "172  old rpause", "waitid", "174  old getdents", "175  old gc_control", "add_profil", "177", "178", "179", "kdebug_trace", "setgid", "setegid", "seteuid", "sigreturn", "chud", "186", "fdatasync", "stat", "fstat", "lstat", "pathconf", "fpathconf", "193", "getrlimit", "setrlimit", "getdirentries", "mmap", "198  __syscall", "lseek", "truncate", "ftruncate", "__sysctl", "mlock", "munlock", "undelete", "ATsocket", "ATgetmsg", "ATputmsg", "ATPsndreq", "ATPsndrsp", "ATPgetreq", "ATPgetrsp", "213  Reserved for AppleTalk", "214", "215", "mkcomplex", "statv", "lstatv", "fstatv", "getattrlist", "setattrlist", "getdirentriesattr", "exchangedata", "224  old checkuseraccess / fsgetpath ( which moved to 427 )", "searchfs", "delete", "copyfile", "fgetattrlist", "fsetattrlist", "poll", "watchevent", "waitevent", "modwatch", "getxattr", "fgetxattr", "setxattr", "fsetxattr", "removexattr", "fremovexattr", "listxattr", "flistxattr", "fsctl", "initgroups", "posix_spawn", "ffsctl", "246", "nfsclnt", "fhopen", "249", "minherit", "semsys", "msgsys", "shmsys", "semctl", "semget", "semop", "257", "msgctl", "msgget", "msgsnd", "msgrcv", "shmat", "shmctl", "shmdt", "shmget", "shm_open", "shm_unlink", "sem_open", "sem_close", "sem_unlink", "sem_wait", "sem_trywait", "sem_post", "sem_getvalue", "sem_init", "sem_destroy", "open_extended", "umask_extended", "stat_extended", "lstat_extended", "fstat_extended", "chmod_extended", "fchmod_extended", "access_extended", "settid", "gettid", "setsgroups", "getsgroups", "setwgroups", "getwgroups", "mkfifo_extended", "mkdir_extended", "identitysvc", "shared_region_check_np", "shared_region_map_np", "vm_pressure_monitor", "psynch_rw_longrdlock", "psynch_rw_yieldwrlock", "psynch_rw_downgrade", "psynch_rw_upgrade", "psynch_mutexwait", "psynch_mutexdrop", "psynch_cvbroad", "psynch_cvsignal", "psynch_cvwait", "psynch_rw_rdlock", "psynch_rw_wrlock", "psynch_rw_unlock", "psynch_rw_unlock2", "getsid", "settid_with_pid", "psynch_cvclrprepost", "aio_fsync", "aio_return", "aio_suspend", "aio_cancel", "aio_error", "aio_read", "aio_write", "lio_listio", "321  old __pthread_cond_wait", "iopolicysys", "process_policy", "mlockall", "munlockall", "326", "issetugid", "__pthread_kill", "__pthread_sigmask", "__sigwait", "__disable_threadsignal", "__pthread_markcancel", "__pthread_canceled", "__semwait_signal", "335  old utrace", "proc_info", "sendfile", "stat64", "fstat64", "lstat64", "stat64_extended", "lstat64_extended", "fstat64_extended", "getdirentries64", "statfs64", "fstatfs64", "getfsstat64", "__pthread_chdir", "__pthread_fchdir", "audit", "auditon", "352", "getauid", "setauid", "getaudit", "setaudit", "getaudit_addr", "setaudit_addr", "auditctl", "bsdthread_create", "bsdthread_terminate", "kqueue", "kevent", "lchown", "stack_snapshot", "bsdthread_register", "workq_open", "workq_kernreturn", "kevent64", "__old_semwait_signal", "__old_semwait_signal_nocancel", "thread_selfid", "ledger", "374", "375", "376", "377", "378", "379", "__mac_execve", "__mac_syscall", "__mac_get_file", "__mac_set_file", "__mac_get_link", "__mac_set_link", "__mac_get_proc", "__mac_set_proc", "__mac_get_fd", "__mac_set_fd", "__mac_get_pid", "__mac_get_lcid", "__mac_get_lctx", "__mac_set_lctx", "setlcid", "getlcid", "read_nocancel", "write_nocancel", "open_nocancel", "close_nocancel", "wait4_nocancel", "recvmsg_nocancel", "sendmsg_nocancel", "recvfrom_nocancel", "accept_nocancel", "msync_nocancel", "fcntl_nocancel", "select_nocancel", "fsync_nocancel", "connect_nocancel", "sigsuspend_nocancel", "readv_nocancel", "writev_nocancel", "sendto_nocancel", "pread_nocancel", "pwrite_nocancel", "waitid_nocancel", "poll_nocancel", "msgsnd_nocancel", "msgrcv_nocancel", "sem_wait_nocancel", "aio_suspend_nocancel", "__sigwait_nocancel", "__semwait_signal_nocancel", "__mac_mount", "__mac_get_mount", "__mac_getfsstat", "fsgetpath", "audit_session_self", "audit_session_join", "fileport_makeport", "fileport_makefd", "audit_session_port","pid_suspend", "pid_resume", "pid_hibernate", "pid_shutdown_sockets", "437  old shared_region_slide_np", "shared_region_map_and_slide_np" , 
 "kas_info", "memorystatus_control", "guarded_open_np","guarded_close_np" }; 


const char * mach_syscall_name_table[128] = {
/* 0 */		"kern_invalid",
/* 1 */		"kern_invalid",
/* 2 */		"kern_invalid",
/* 3 */		"kern_invalid",
/* 4 */		"kern_invalid",
/* 5 */		"kern_invalid",
/* 6 */		"kern_invalid",
/* 7 */		"kern_invalid",
/* 8 */		"kern_invalid",
/* 9 */		"kern_invalid",
/* 10 */	"_kernelrpc_mach_vm_allocate_trap", // OS X : "kern_invalid",
/* 11 */	"_kernelrpc_vm_allocate_trap", // OS X : "kern_invalid",
/* 12 */	"_kernelrpc_mach_vm_deallocate_trap", // OS X: "kern_invalid",
/* 13 */	"_kernelrpc_vm_deallocate_trap" , // "kern_invalid",
/* 14 */	"_kernelrpc_mach_vm_protect_trap", //"kern_invalid",
/* 15 */	"_kernelrpc_vm_protect_trap", // kern_invalid",
/* 16 */	"_kernelrpc_mach_port_allocate_trap", //"kern_invalid",
/* 17 */	"_kernelrpc_mach_port_destroy_trap" ,//"kern_invalid",
/* 18 */	"_kernelrpc_mach_port_deallocate_trap", // "kern_invalid",
/* 19 */	"_kernelrpc_mach_port_mod_refs_trap", //"kern_invalid",
/* 20 */	"_kernelrpc_mach_port_move_member_trap", //"kern_invalid",
/* 21 */	"_kernelrpc_mach_port_insert_right_trap", //"kern_invalid",
/* 22 */	"_kernelrpc_mach_port_insert_member_trap", // "kern_invalid",
/* 23 */	"_kernelrpc_mach_port_extract_member_trap", // "kern_invalid",
/* 24 */	"kern_invalid",
/* 25 */	"kern_invalid",
/* 26 */	"mach_reply_port",
/* 27 */	"thread_self_trap",
/* 28 */	"task_self_trap",
/* 29 */	"host_self_trap",
/* 30 */	"kern_invalid",
/* 31 */	"mach_msg_trap",
/* 32 */	"mach_msg_overwrite_trap",
/* 33 */	"semaphore_signal_trap",
/* 34 */	"semaphore_signal_all_trap",
/* 35 */	"semaphore_signal_thread_trap",
/* 36 */	"semaphore_wait_trap",
/* 37 */	"semaphore_wait_signal_trap",
/* 38 */	"semaphore_timedwait_trap",
/* 39 */	"semaphore_timedwait_signal_trap",
/* 40 */	"kern_invalid",
/* 41 */	"kern_invalid",
/* 42 */	"kern_invalid",
/* 43 */	"map_fd",
/* 44 */	"task_name_for_pid",
/* 45 */ 	"task_for_pid",
/* 46 */	"pid_for_task",
/* 47 */	"kern_invalid",
/* 48 */	"macx_swapon",
/* 49 */	"macx_swapoff",
/* 50 */	"kern_invalid",
/* 51 */	"macx_triggers",
/* 52 */	"macx_backing_store_suspend",
/* 53 */	"macx_backing_store_recovery",
/* 54 */	"kern_invalid",
/* 55 */	"kern_invalid",
/* 56 */	"kern_invalid",
/* 57 */	"kern_invalid",
/* 58 */	"pfz_exit",
/* 59 */ 	"swtch_pri",
/* 60 */	"swtch",
/* 61 */	"thread_switch",
/* 62 */	"clock_sleep_trap",
/* 63 */	"kern_invalid",
/* traps 64 - 95 reserved (debo) */
/* 64 */	"kern_invalid",
/* 65 */	"kern_invalid",
/* 66 */	"kern_invalid",
/* 67 */	"kern_invalid",
/* 68 */	"kern_invalid",
/* 69 */	"kern_invalid",
/* 70 */	"kern_invalid",
/* 71 */	"kern_invalid",
/* 72 */	"kern_invalid",
/* 73 */	"kern_invalid",
/* 74 */	"kern_invalid",
/* 75 */	"kern_invalid",
/* 76 */	"kern_invalid",
/* 77 */	"kern_invalid",
/* 78 */	"kern_invalid",
/* 79 */	"kern_invalid",
/* 80 */	"kern_invalid",
/* 81 */	"kern_invalid",
/* 82 */	"kern_invalid",
/* 83 */	"kern_invalid",
/* 84 */	"kern_invalid",
/* 85 */	"kern_invalid",
/* 86 */	"kern_invalid",
/* 87 */	"kern_invalid",
/* 88 */	"kern_invalid",
/* 89 */	"mach_timebase_info_trap",
/* 90 */	"mach_wait_until_trap",
/* 91 */	"mk_timer_create_trap",
/* 92 */	"mk_timer_destroy_trap",
/* 93 */	"mk_timer_arm_trap",
/* 94 */	"mk_timer_cancel_trap",
/* 95 */	"kern_invalid",
/* traps 64 - 95 reserved (debo) */
/* 96 */	"kern_invalid",
/* 97 */	"kern_invalid",
/* 98 */	"kern_invalid",
/* 99 */	"kern_invalid",
/* traps 100-107 reserved for iokit (esb) */ 
/* 100 */	"kern_invalid",
/* 100 */	//"iokit_user_client_trap",
/* 101 */	"kern_invalid",
/* 102 */	"kern_invalid",
/* 103 */	"kern_invalid",
/* 104 */	"kern_invalid",
/* 105 */	"kern_invalid",
/* 106 */	"kern_invalid",
/* 107 */	"kern_invalid",
/* traps 108-127 unused */			
/* 108 */	"kern_invalid",
/* 109 */	"kern_invalid",
/* 110 */	"kern_invalid",
/* 111 */	"kern_invalid",
/* 112 */	"kern_invalid",
/* 113 */	"kern_invalid",
/* 114 */	"kern_invalid",
/* 115 */	"kern_invalid",
/* 116 */	"kern_invalid",
/* 117 */	"kern_invalid",
/* 118 */	"kern_invalid",
/* 119 */	"kern_invalid",
/* 120 */	"kern_invalid",
/* 121 */	"kern_invalid",
/* 122 */	"kern_invalid",
/* 123 */	"kern_invalid",
/* 124 */	"kern_invalid",
/* 125 */	"kern_invalid",
/* 126 */	"kern_invalid",
/* 127 */	"kern_invalid",
};


#define XNUSIG "SourceCache/xnu/xnu-"

#define SYS_MAXSYSCALL   443
#define SIG1 "\x00\x00\x00\x00"  "\x00\x00\x00\x00"  "\x01\x00\x00\x00"  "\x00\x00\x00\x00"  "\x01\x00\x00\x00"

#define SIG1_SUF "\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x04\x00\x00\x00" 

#define SIG2 "\x00\x00\x00\x00" \
             "\x00\x00\x00\x00" \
	     "\x01\x00\x00\x00" \
	     "\x1C\x00\x00\x00" \
             "\x00\x00\x00\x00"


void dumpMachTraps(char *mach)
{
        if (mach) printf ("Kern invalid should be %p. Ignoring those\n", *((int *) &mach[4]));
	int i;
        for (i = 0; i < 128; i++)
	{
	  int thumb = 0;
	  int addr = * ((int *) (mach + 4 + 8*i));

	  if (addr == *((int *) (mach + 4))) continue;
	  if ((addr % 4) == 1) { addr--; thumb++; }
	  if ((addr % 4) == -3) { addr--; thumb++; }
	  if (addr % 4) { thumb = "?"; }
	  
	  printf ("%3d %-40s %x %s\n", i, mach_syscall_name_table[i], addr, (thumb? "T": "-"));

	} // end for < 128 .. 

} // dumpMachTraps
   

extern int g_list;
extern int g_ListSymbols;
int doKext (char *mmapped)
{

  return 1;

} // doKext



void
printDictionaryAsXML(CFMutableDictionaryRef dict)
{
    CFDataRef xml = CFPropertyListCreateXMLData(kCFAllocatorDefault,
                                                (CFPropertyListRef)dict);
    if (xml) {
        write(1, CFDataGetBytePtr(xml), CFDataGetLength(xml));
	printf("done\n");
        CFRelease(xml);
    }
	printf("..\n");
}



void main (int argc, char **argv)
{

   int fd;
   char *mmapped;
   int rc;
   struct stat stbuf;
   int filesize;
   char *filename = argv[1];
   struct mach_header *mh;
   int i,j ;
   int magic;
   char *sysent = NULL;
   char *mach = NULL;
   char *xnuSig = NULL;
   int showUNIX = 1, showMach = 1;
   int suppressEnosys = 1;
   int kexts = 0;

   if (!filename) { fprintf (stderr,"Usage: joker _filename_\n", argv[0]);
		    fprintf (stderr," _filename_ should be a decrypted iOS kernelcache. Tested on 3.x-4.x-5.x-6.0\n"); 
		    fprintf (stderr, "Stable version (no symbolification/sysctl/etc here yet)\n"); exit(0);}


   rc = stat(filename, &stbuf);

   if (rc == -1) { perror ("stat"); exit (1); }

   filesize = stbuf.st_size;

   fd = open (argv[1], O_RDONLY);
   if (fd < 0) { perror ("open"); exit(2);}

   mmapped = mmap(NULL,
             filesize,  // size_t len,
             PROT_READ, // int prot,
             MAP_SHARED | MAP_FILE,  // int flags,
             fd,        // int fd,
             0);        // off_t offset);

   if (!mmapped) { perror ("mmap"); exit(3);}

   
   processFile(mmapped);

   
   mh =  (struct mach_header *) (mmapped);
  
   switch (mh->magic)
	{
		case 0xFEEDFACE:
			/* Good, this is a Mach-O */

			if (mh->cputype == 12) /* ARM */
			 {
			   // This is an ARM binary. Good.
			   printf ("This is an ARM binary. Applying iOS kernel signatures\n");
			 }
			break;

		case 0xbebafeca:	
			fprintf (stderr, "This is an Intel FAT binary, but I can't handle these yet\n");
			exit(5);
		default:
			fprintf(stderr, "I have no idea how to handle a file with a magic of %p\n", magic); exit(6);

	}
   

   printf ("Entry point is 0x%llx..", getEntryPoint());


   for  (i = 0;
         i < filesize-50;
         i++)
	{
	   
	   if (!xnuSig && memcmp(&mmapped[i], XNUSIG, strlen(XNUSIG)) == 0)
		{
		char buf[80];
		  xnuSig = mmapped + i + strlen(XNUSIG);
		memset(buf, '\0', 80);
		strncpy (buf, xnuSig, 40);

		// The signature we get is from a panic, with the full path to the
                // xnu sources. Remove the "/" following the XNU version. Because the
                // memory is mmap(2)ed read only, we have to copy this first.

		char *temp = strstr(buf, "/");
		if (temp) {
		  *temp = '\0';
		}

		xnuSig = buf;
		
		printf ("..This appears to be XNU %s\n", xnuSig);


		}

	   if (memcmp(&mmapped[i], SIG1, 20) == 0)
		{
	  	    if (memcmp(&mmapped[i+24], SIG1_SUF, 16) == 0)
			{
		          printf ("Sysent offset in file/memory (for patching purposes):  %p/%p\n",i-8,findAddressOfOffset(i -8));  
			  sysent = mmapped + i - 24 ; 
	//		  if (xnuSig) break;
			}
		}

	if (showMach)
	{
	   if (! mach &&
               (memcmp(&mmapped[i], &mmapped[i+40], 40 ) == 0) &&
	       (memcmp(&mmapped[i], &mmapped[i+32], 32 ) == 0) &&
	       (memcmp(&mmapped[i], &mmapped[i+24], 24 ) == 0) &&
	       (memcmp(&mmapped[i], &mmapped[i+16], 16) == 0) &&
	       (memcmp(&mmapped[i], &mmapped[i+24], 24) == 0) &&
	       (memcmp(&mmapped[i], &mmapped[i+8], 8 ) == 0) &&
	       (  (!*((int *) &mmapped[i])) &&  *((int *) &mmapped[i+4]))
	      )  
	      {
		  printf ("mach_trap_table offset in file/memory (for patching purposes): 0x%x/%p\n", i,findAddressOfOffset(i));
		  mach = &mmapped[i];
		  dumpMachTraps (mach);
		}

	   } // end showMach
	} // end for i..


    if (!xnuSig) { fprintf (stderr, "This doesn't seem to be a kernel!\n"); exit (7);}


	if (showUNIX)
	{
    	 if (!sysent) { fprintf (stderr, "Unable to find sysent!\n"); exit(8);}
	 if (memcmp(&mmapped[i], "syscall\0exit", 12) == 0)
	  {
	   //	syscall_names = &mmapped[i];

		printf ("Syscall names are @%x\n", i);
	  }

    if (suppressEnosys)
	{
	  int enosys = * ((int *) (sysent + 20 + 24*4));
	  printf ("Suppressing enosys (%p)\n", enosys);

	}

    for (i = 0; i < SYS_MAXSYSCALL  ; i++)
	{
	  int suppress = 0;
	  int thumb = 0;
	  int addr = * ((int *) (sysent + 20 + 24*i));
	
	  
	  if (addr == *((int *)(sysent + 20 + 24 * 8)))
		suppress =1;
	

	  if ((addr % 4) == 1) { addr--; thumb++; }
	  if ((addr % 4) == -3) { addr--; thumb++; }

  	  if (!suppress)
	    printf ("%d. %-20s %x %s\n", i,syscall_names[i], addr, (thumb? "T": "-"));

	  // skip to next post null byte - unfortunately wont work due to optimizations
	  // putting some of the system call name strings elsewhere (in their first appearance
          // in the binary)

	  //  for (; *syscall_names; syscall_names++);
	  //  syscall_names++;
	}
	  } // showUNIX

	// Do KEXTs

	// To do the kexts, we load the dictionary of PRELINK_INFO
	{
		char *kextPrelinkInfo = (char *) malloc(1000000);
		CFDictionaryRef	dict;
		char *kextNamePtr;
		char *kextLoadAddr;
		char kextName[256];
		char loadAddr[16];
		char *temp = kextPrelinkInfo;

		extern char *g_SegName;

		g_SegName = "__PRELINK_INFO";

		void *seg = MachOGetSection("__PRELINK_INFO");

		
	
		kextPrelinkInfo = (char *) (mmapped + MachOGetSegmentOffset(seg));

		temp = kextPrelinkInfo;
		kextNamePtr = strstr(temp,"CFBundleName</key>");

		// This is EXTREMELY quick and dirty, but I can't find a way to load a CFDictionary
		// directly from XML data, so it will do for now..

		while (kextNamePtr) {
			temp = strstr(kextNamePtr, "</string>");
			// overflow, etc..
			memset(kextName, '\0', 256);
			strncpy (kextName, kextNamePtr + 26, temp - kextNamePtr - 26);


			printf("Kext: %s ", kextName);
			temp += 15;

	//		_PrelinkExecutableLoadAddr
			kextNamePtr = strstr(temp, "CFBundleIdentifier");
			if (kextNamePtr)
			{
				temp = strstr(kextNamePtr,"</string>");
				memset(kextName,'\0',256);
				strncpy(kextName, kextNamePtr + 32, temp - kextNamePtr - 32);
				
				printf ("(%s)\n", kextName);
			}
			kextNamePtr = strstr(temp,"CFBundleName</key>"); 

			kexts++;
	
			

			
		
		}

		
	}
	
	printf("Got %d kexts. done\n", kexts);

}
