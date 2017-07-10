What is it?
-----------

A kind of `tee` that writes stdin to a rotated log file(s).

I got tired of the absence of lightweight log rotation utility.
`logrotate` requires a whole infrastructure around itself.
`bash`-only piping requires a more than one-liner, plus `bash` has to
do the read-write loop, which doesn't feel optimal.
Or, worse, bash scripts restart the main program.

Too late I discovered [multilog](http://cr.yp.to/daemontools/multilog.html)
in [this great SuperUser answer](https://superuser.com/a/291397/32412).

Examples
--------

    verbosecommand | logrotee /var/log/verbosecommand.log \
    > /dev/null
or

    verbosecommand | logrotee \
      --compress "bzip2 {}" --compress-suffix .bz2 \
      --null --chunk 2M \
      /var/log/verbosecommand.log

If the output chunk file + `<suffix>` already exists, add an unique sequential suffix, currently `.1`, `.2`, `.3`, you get the idea.

Usage
-----
`logrotee [-z|--compress <compress-command>] [-s|--compress-suffix <suffix>] [-n|--null] [--chunk|-k <size>] <name>`

* `<name>`: Log file name. If the file exists, its name will be appended a number, like: `/var/log/verbosecommand.log.1`, `/var/log/verbosecommand.log.2`, and so on.
If `--compress-suffix "bz2"` is given, will check for `/var/log/verbosecommand.log.bz2`, `/var/log/verbosecommand.log.1.bz2` and so on.
* `-z`, `--compress` `<compress-command>`	    Optional parameter - command to run on a complete chunk file, typically packer.
 `{}` is substituted with a full file path.
 It's the packer's responsibility to delete the original chunk file.
* `-s`, `--compress-suffix` `<suffix>`	    File suffix to append to check for <name> existence, if any.
* `-n`, `--null`      Don't print the stdin to stdout. Not sure if it gains anything compared to `>/dev/null`.
* `-k`, `--chunk` `<chunk-size>` Chunk file size in bytes, after which it's closed and maybe compressed. Default is ~10Mb.

Planned:
--------
* `-d`, `--dates`  Use dates instead of sequential numbers for rotation suffix.
* Rotate by time instead of size.
* `-m`, `--max-files` `<amount>`     Maximum amount of rotated files. Delete the oldest chunk files when over this limit.
 * Does not check if the files were not created by this instance of `logrotee`.
* `-z` shortcut for gzip/bz2.
* Handle signals.
* Parse k/M/etc suffixes in file sizes.
* Handle out-of-space situation.
* Do not launch a compressor until the previous exits.
* Looks like some people need to prepend log lines with timestamps.
