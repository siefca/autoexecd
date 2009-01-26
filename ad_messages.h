#ifndef AD_MESSAGES_H
#define AD_MESSAGES_H

			    /* daemon messages */

#define MSG1	"Your UID must be 0 to run it\n"
#define MSG2	"\nautoexecd: current configuration (read from configuration file)\n---------\n"
#define MSG3	"Autoexecd will start his default actions immediately.\n"
#define MSG4	"Autoexecd will "
#define MSG5	"wait for "
#define MSG6	"look at "
#define MSG7	"process called %s.\n"
#define	MSG8	"If the specified process "
#define	MSG9	"won't show up in %d second(s) then "
#define MSG10	"won't be present then "
#define MSG11	"the daemon will\nstart its default actions.\n"
#define MSG12	"the daemon will\nstop.\n"
#define	MSG13	"\nDEFAULT ACTIONS :\n"
#define	MSG13B	"Daemon will wait %d second(s). After that it will start executing\nusers' .autoexec files."
#define MSG13C	"Daemon will start executing users' .autoexec files.\n"
#define MSG14	"\nIt WON'T work for users with UID=0 "
#define	MSG15	"and for users listed below:\n\n"
#define MSG16	"\nNice value any of executed processes will be changed up to %d.\n"
#define MSG17	"Autoexecd will wait %d second(s) between running .autoexecs.\n"
#define MSG18	"Autoexecd's (and it's child-processes) umask will be set to %04o.\n"

			    /* config-file fields */

#define	INPUTMSG1	"disabled-for"
#define	INPUTMSG2	"eof-disabled-for"
#define INPUTMSG3	"delay after:"
#define INPUTMSG4	"delay between:"
#define	INPUTMSG5	"wait for process:"
#define	INPUTMSG6	"nice value:"
#define	INPUTMSG7	"pass after:"
#define	INPUTMSG8	"forced execute"
#define INPUTMSG9	"umask:"

			    /* daemon error messages */

#define	MSG_FORK_FAIL	"autoexecd: fork failed"
#define	MSG_ERR_OP_CONF	"autoexecd: can't open configuration file for reading"
#define	MSG_ERR_DFL	"autoexecd: using default configuration"

#endif
