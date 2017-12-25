# Atop-logger
A simple user activities logger
log every user activities on console includes the output with syslog

# Software dependency
syslogd on Linux

# How to use
build then copy the library to your libraries diectory
export LD_PRELOAD to path of the library diectory
or
appened "/path/to/your/lib" into the file, ld.so.preload

If everything go right, syslogd will log your activites(except for "cd" or others commands proceeded by shell) in its log file.

Please feel free to share your idea or comments to this project, this is just a prototype after all
