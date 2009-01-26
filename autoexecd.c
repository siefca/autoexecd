/*   autoexecd -- automaticaly executes user's starting scripts at boot time
     version 1.0 , release 5
     
     Copyright (c) 1999 Pawe³ Wilk 

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with this program; if not, write to the Free Software
     Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

     Pawe³ Wilk can be contacted via e-mail: <siefca@gnu.org>
     
     The latest version of autoexecd should be accessible at:
     ftp://dione.ids.pl/~siewca/open_source/
*/


#include <pwd.h>
#include <dirent.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <getopt.h>
#include <stdio.h>

#include "version.c"
#include "ad_messages.h"

#define	PATHNAME_SIZE	1024
#define	USERNAME_SIZE	33

typedef char boolean;

const int DEFAULT_DELAY_AFTER = 2;
const int DEFAULT_DELAY_BETWEEN = 5;
const int DEFAULT_NICE_VAL = 5;
const int DEFAULT_UMASK_VAL = 0007;
const int DEFAULT_PASS_AFTER = 80;
const boolean DEFAULT_FORCED_EXEC = 0;

const char DEFAULT_PROCESS [] = "getty";  /* who are we waiting for?  */
const char A_FILENAME [] = ".autoexec";   /* user's script filename   */
const char A_OUT_FILENAME [] = "autoexec/stdout";/* std. streams' redirections */
const char A_ERR_FILENAME [] = "autoexec/stderr"; 
const char A_CONFIGFILE [] = "/etc/autoexecd.conf"; /* configuration file */

FILE* ad_configfile;

int delay_after, delay_between, nice_value, pass_after, umask_value;
boolean dir_found, forced_exec;

char ad_waited_proc [PATHNAME_SIZE];

boolean _ad_bad_user (uid_t);
int _ad_findword (const char *);
void _ad_load_config (void);
void _ad_waitfortty (void);
int _ad_anyitem (struct dirent*);
void _ad_exegi_monumentum (void);
void _ad_ignore_signals (void);
void _ad_sig_dfl (void);
boolean _ad_contains (char*, char*);

/* MAIN */

int main (int argc, char **argv, char **envp)
{
 char opti, username [USERNAME_SIZE];
 pid_t forked_val = 0;
 int i, j;
  
 if (getuid() || geteuid())
 {
  fprintf (stderr, MSG1);
  exit (2);
 }
  
 _ad_load_config ();        		/* loading configuration */
 
 umask (umask_value);
 
 while ((opti = getopt(argc,argv,"sV")) != EOF)
 {
  if (opti == 'V')
  {
   printf ("\nautoexecd --  version:\t\t%s-%s (pathlevel %s)", VERSION, RELEASE, PATCHLEVEL);
   printf ("\n              author:\t\tPawe³ Wilk <siewca@dione.ids.pl>");
   printf ("\n              copyright:\t%s\n", COPYRIGHT); 
   break;
  }
    
  if (opti == 's')
  {
    printf (MSG2);
    
    if (forced_exec && !pass_after) 
      printf (MSG3);
    else
    {
     printf (MSG4);
     if (pass_after) printf (MSG5);
     else printf (MSG6);
     printf (MSG7, ad_waited_proc);
     printf (MSG8);
     if (pass_after) printf (MSG9, pass_after);
     else printf (MSG10);
      if (forced_exec)  printf (MSG11);
      else printf (MSG12);
    }

    printf (MSG13);    
    if (delay_after) printf (MSG13B, delay_after);
    else printf (MSG13C);
    printf (MSG14);
   
    j=0;
   
    rewind (ad_configfile);
    if (!_ad_findword (INPUTMSG1))
    { 
      printf (MSG15);
      while (fscanf (ad_configfile, "%32s , ", username) != EOF)
      {
        if (!strncasecmp (username, INPUTMSG2, 14)) break; 
        else if (!strncasecmp (username, "#", 1)) {_ad_findword("\n"); continue;}; 
        j += (strlen (username) + 5);
        
        if (j >= 45) {
                       printf ("\n");
                       j = 0;
                     }
		      
        printf ("\t%s", username);
      }
    }
    else printf (".");
   
    printf ("\n\n");
   
    if (nice_value)
     printf (MSG16, nice_value);
    if (delay_between)
     printf (MSG17, delay_between);
    if (umask_value)
     printf (MSG18, umask_value);

    exit (0);
  } /* if */
  
 } /* options */
 
 if ((forked_val = fork()) == -1)
 {
   perror (MSG_FORK_FAIL);
   exit (1);
 }
 
 if (forked_val) exit (0); /* good bye parent */
 
 /* child code here */
 
 chdir ("/");		/* let him unmount some filesystems...		      */
 setsid ();  		/* group leader, cutted off from control terminal     */
  
 _ad_waitfortty ();	/* waiting for getty - that means the system is ready */
 sleep ((unsigned int) delay_after);   /* going to sleep for a while...       */
 
 _ad_ignore_signals ();                /* signals holding		      */
 _ad_exegi_monumentum ();              /* general function		      */
 fclose (ad_configfile);
 sleep ((unsigned int) 5);
 exit (0);
}

/* function definitions */

void _ad_load_config (void)
{
  int retik, linia, czas;
  char buf [PATHNAME_SIZE];

  if (access (A_CONFIGFILE, R_OK) == -1)
    {
      perror (MSG_ERR_OP_CONF);
      exit (9);
    }
      
  ad_configfile = fopen (A_CONFIGFILE, "r");
  
  delay_after = DEFAULT_DELAY_AFTER;
  delay_between = DEFAULT_DELAY_BETWEEN;
  strncpy (ad_waited_proc, DEFAULT_PROCESS, PATHNAME_SIZE);
  nice_value = DEFAULT_NICE_VAL;
  umask_value = DEFAULT_UMASK_VAL;
  pass_after = DEFAULT_PASS_AFTER;
  forced_exec = DEFAULT_FORCED_EXEC;
  
  if (ad_configfile == NULL) 
  {
     perror (MSG_ERR_OP_CONF);
     fprintf (stderr, MSG_ERR_DFL);
     fflush (stderr);
     return;
  }
  
  clearerr (ad_configfile);
  
  /*  delay after start */
  
  if (!_ad_findword (INPUTMSG3)) 
   if ((retik = fscanf (ad_configfile, "%5d", &czas)) != EOF && retik != 0)
     delay_after = czas; 
    
  if (czas < 0 || czas > 30000) 
    delay_after = DEFAULT_DELAY_AFTER;
  rewind (ad_configfile);
  
  /* delay between actions */
  
  if (!_ad_findword (INPUTMSG4)) 
   if ((retik = fscanf (ad_configfile, "%5d", &czas)) != EOF && retik != 0)
     delay_between = czas; 
    
  if (delay_between < 0 || delay_between > 30000) 
    delay_between = DEFAULT_DELAY_BETWEEN;
  rewind (ad_configfile);
  
  /* waiting for process */
    
  if (!_ad_findword (INPUTMSG5)) 
   {
     if ((retik = fscanf (ad_configfile, "%1024s", buf)) != EOF && retik != 0)
       strncpy (ad_waited_proc, buf, PATHNAME_SIZE);
     else strncpy (ad_waited_proc, DEFAULT_PROCESS, PATHNAME_SIZE);
   }
  rewind (ad_configfile);
       
  /* nice value */
  
  if (!_ad_findword (INPUTMSG6)) 
   if ((retik = fscanf (ad_configfile, "%2d", &czas)) != EOF && retik != 0)
     nice_value = czas;
    
  if (nice_value < 0 || nice_value > 19) 
    nice_value = DEFAULT_NICE_VAL;
  rewind (ad_configfile);
  
  /* umask */

  if (!_ad_findword (INPUTMSG9)) 
   if ((retik = fscanf (ad_configfile, "%4o", &czas)) != EOF && retik != 0)
     umask_value = czas;
    
  if (umask_value < 0000 || umask_value > 07777) 
    umask_value = DEFAULT_UMASK_VAL;
  rewind (ad_configfile);

  /* time of waiting for specified process */
  
  if (!_ad_findword (INPUTMSG7)) 
   if ((retik = fscanf (ad_configfile, "%2d", &czas)) != EOF && retik != 0)
     pass_after = czas;
    
  if (pass_after < 0 || pass_after > 30000) 
    pass_after = DEFAULT_NICE_VAL;
  rewind (ad_configfile);
  
  /* what should we do if time has come and no process detected */
  
  if (!_ad_findword (INPUTMSG8)) forced_exec = 1;
  rewind (ad_configfile);  
  
  /* ok, done */ 
}

int _ad_findword (const char* wtf)
{
  char zn;
  int counter, dl;
  const char* ptr = wtf;
  
  counter = dl = strlen (wtf);
  
  while (counter)
  {
    if ((zn = fgetc (ad_configfile)) == EOF) break;
    
    if (zn == '#')
    {
      while ((zn = fgetc (ad_configfile)) != EOF && zn != '\n');
      counter = dl;
      ptr  = wtf;
    }
    
    if (!strncasecmp (&zn, ptr, 1))
    {
      --counter;
      ++ptr;
    }  
    else 
    {
      counter = dl;
      ptr = wtf;
    }
  };
  
  clearerr (ad_configfile);
  if (counter) return (1);
  return (0);  
}


boolean _ad_contains (char* tch, char* smp)
{
#ifndef STRSTR_METHOD
  int c = 1, ma_x = strlen (smp);
  char *ptr_a = tch, *ptr_b = smp;
  
  while (*ptr_a && *ptr_b)
   if (*ptr_a++ == *ptr_b++)
    if (c == ma_x) return 1;
    else ++c;
   else {
          c = 1;
	  ptr_b = smp;
	} 
#else
 if (strstr (tch, smp) != NULL) return 1;
#endif
 return 0;
}

boolean _ad_bad_user (uid_t ad_userid)
{
  struct passwd* u;
  char username [USERNAME_SIZE];
  int retik;
  
  u = getpwuid (ad_userid);
  if (u == NULL) return 1;
  
        /* users with denied access */
  
  rewind (ad_configfile);
  if (!_ad_findword (INPUTMSG1))
  {
    while ((retik = fscanf (ad_configfile, "%50s , ", username)) != EOF)
    {
      if (!strncasecmp (username, INPUTMSG2, 14)) break; /* end of section */
      else if (!strncasecmp (username, "#", 1)) {_ad_findword("\n"); continue;}; 
    
      if (!strcmp (u->pw_name, username)) return 1;
    };
  }
  return 0;
}

void _ad_waitfortty (void)
{
  struct dirent** nmlist;
  int x;
  
  dir_found = 0;
  
  if (!pass_after) pass_after = 1;
  
  for (x=0; x<=pass_after-1; x++)  
  {
    nice (10);
    scandir ("/proc", &nmlist, _ad_anyitem, NULL);
    nice (-10);
    if (dir_found) return;
    else sleep ((unsigned int) 1);    
  }
  
  if (forced_exec) return;
  else exit (8);
}

int _ad_anyitem (struct dirent *kat)
{
  char filenam [PATHNAME_SIZE], bf [PATHNAME_SIZE];
  FILE* pf;
  
  if (!isdigit(*(kat->d_name))) return -1;
  if ((strlen (kat->d_name) + 14) >= PATHNAME_SIZE) return -1;
  
  sprintf (filenam, "/proc/%s/status", kat->d_name);
  
  if ((pf = fopen (filenam, "r")) == NULL) return -1;
  
  fscanf (pf, "Name:\t%s", bf);
  fclose (pf);
  if (_ad_contains (bf, ad_waited_proc)) dir_found = 1;
  return -1;
}

void _ad_exegi_monumentum (void)
{
  struct passwd *h;
  FILE *standwy, *standerr;
  
  pid_t forked_val;
  char fullpath [PATHNAME_SIZE], czyj [USERNAME_SIZE];
    
  setpwent ();
    
  for (;;)
  {
    h = getpwent ();
    if (h == NULL) break;
    if (h->pw_dir == NULL || *(h->pw_dir) == '\0' || !(h->pw_uid)
        || _ad_bad_user (h->pw_uid)) continue;
    
    sprintf (fullpath, "%s/%s", h->pw_dir, A_FILENAME);
    
    if (access (fullpath, R_OK) == -1) continue;
        
    while ((forked_val = fork()) == -1) sleep ((unsigned int) 10);
    if (forked_val == 0)				/* child */
    {
      _ad_sig_dfl ();
                  
      setgid (h->pw_gid);
      setuid (h->pw_uid);
      chdir (h->pw_dir);
      
      sprintf (fullpath, "./%s", A_OUT_FILENAME);
      standwy = freopen (fullpath, "a+", stdout);
      if (!standwy) standwy = freopen ("/dev/null", "a+", stdout);
      
      sprintf (fullpath, "./%s", A_ERR_FILENAME);
      standerr = freopen (fullpath, "a+", stderr);
      if (!standerr) standerr = freopen ("/dev/null", "a+", stderr);
      
      sprintf (fullpath, "%s/%s", h->pw_dir, A_FILENAME);
      if (h->pw_name) sprintf (czyj, "(%s)", h->pw_name);
      setenv ("AUTOEXEC", "TRUE", 1);	  /* oh, i'm turned on by autoexecd! */
      nice (nice_value);		  

      execl (fullpath, fullpath, "--", czyj, NULL);   /* let's play... */
      
      exit (4); 
    }
    else sleep ((unsigned int) delay_between);
  } /* for */     
  
  endpwent ();
}  

void _ad_ignore_signals (void)
{
#ifdef SIGCHLD
 signal (SIGCHLD, SIG_IGN); 
#endif
#ifdef SIGTTOU
 signal (SIGTTOU, SIG_IGN); 
#endif
#ifdef SIGTTIN
 signal (SIGTTIN, SIG_IGN);
#endif
#ifdef SIGTSTP
 signal (SIGTSTP, SIG_IGN);
#endif
#ifdef SIGHUP
  signal (SIGHUP, SIG_IGN);
#endif
}

void _ad_sig_dfl (void)
{
#ifdef SIGCHLD
 signal (SIGCHLD, SIG_DFL); 
#endif
#ifdef SIGTTOU
 signal (SIGTTOU, SIG_DFL); 
#endif
#ifdef SIGTTIN
 signal (SIGTTIN, SIG_DFL);
#endif
#ifdef SIGTSTP
 signal (SIGTSTP, SIG_DFL);
#endif
#ifdef SIGHUP
  signal (SIGHUP, SIG_DFL);
#endif
}

