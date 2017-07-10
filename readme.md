What is it?
===========

A kind of `tee` that writes stdin to a rotated log file(s).

EXAMPLES
========

    verbosecommand | logrotee /var/log/verbosecommand.log \
    > /dev/null
or

    verbosecommand | logrotee /var/log/verbosecommand.log \
      --compress "bzip2 {}" --compress-suffix .bz2 \
      --null --chunk 2M

If the output chunk file + <suffix> already exists, add an unique sequential suffix, currently `.1`, `.2`, `.3`, you get the idea.

USAGE
=====
`logrotee <name> [-z|--compress <compress-command>] [-s|--compress-suffix <suffix>] [-n|--null] [--chunk|-k <size>]`

`<name>`: Chunk file name. If the file exists, will be appended a number, like: `/var/log/verbosecommand.log.1`, `/var/log/verbosecommand.log.2`, and so on.
If `--compress-suffix "bz2"` is given, will check for `/var/log/verbosecommand.log.bz2`, `/var/log/verbosecommand.log.1.bz2` and so on.

`-z`, `--compress` `<compress-command>`	    Optional parameter - command to run on a complete chunk file, typically packer.
{} is substituted with a full file path.
It's the packer's responsibility to delete the original chunk file.
`-s`, `--compress-suffix` `<suffix>`	    File suffix to append to check for <name> existence, if any.
`-n`, `--null`      Don't print the file to stdout.

Planned:
=======
`-d`, `--dates`  Use dates instead of sequential numbers for rotation suffix.
`-m`, `--max-files` `<amount>`     Maximum amount of rotated files. Delete the oldest chunk files when over this limit.
Does not check if the files were not created by this instance of `logrotee`.

Handle signals.

Parse k/M/etc suffixes in file sizes.