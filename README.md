# This project was so that I could find out who was spending the most time on the school server using system programming.
This is the man page of my program.

## NAME
logtime -- print various statistics of logtime of one or more users
SYNOPSIS
### logtime [options] username ...
## DESCRIPTION
Without options, logtime prints one line for each username argument, containing the total time
that the user has spent logged into the system since record-keeping was last started. If no
usernames are listed, it displays this time for the current user. If there is no wtmp file, it prints a
message on the standard error stream that there is no record-keeping. Otherwise, for each user to
be reported, it displays the username followed by the total login time, accurate to the second, in
days, hours, minutes, and seconds. If the total time is less than a day, the days field is omitted.
If it is less than an hour, the hours and days are omitted, and if it’s less than a minute, only the
seconds are displayed. If a username is given but there are no logins for the user, "0 seconds" is
listed for that username. If any value is zero, the units for that value are not displayed.
The definition of total login time for this command includes:
• all time in completed login sessions, meaning those that have logouts associated with the
logins, and
• all time in login sessions that were terminated without a corresponding logout because there
was a shutdown or a system reboot.
The command excludes in the total reported time any time for which the login session is in
progress at the time the command is invoked.
All times are in whole, non-negative integers. Usernames are not sorted alphabetically.

## OUTPUT
Output units are day(s) for the number of days, hour(s) for the number of hours, min(s) for
the number of minutes, and sec(s) for the number of seconds. For example:
boromir 1 day 9 hours 39 mins 51 secs
frodo 14 hours 1 sec
gandalf 22 days 42 mins

In the second case, it is an implicit time of 14 hours, 0 minutes and 1 second. In the third, it is
22 days, 0 hours, 42 minutes and 0 seconds.


## OPTIONS
The behavior of the command can be modified with the following option:
-a
Show the log times for all users that have entries in the file _PATH_WTMP.

-f <file>
Use <file> instead of _PATH_WTMP.

## EXIT STATUS
0 If it succeeded.
1 If it failed.
