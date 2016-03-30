gf_client:   gf_client.c
	gcc -O2 -o gf_client gf_client.c -L /usr/local/lib  -lgfapi -I ~/glusterfs/api/src -g
