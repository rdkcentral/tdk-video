/opt/TDK/tdk_sample {
/{usr/,}lib/libapparmor*			rm,
/etc/ld.so.*				r,
/opt/TDK/tdk_sample				r,
/bin/ls					r,
/tmp/foobar				rw,
}
