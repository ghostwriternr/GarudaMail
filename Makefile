all: reset server_abc server_xyz client

server_abc:
	cc server_abc.c -o sa

server_xyz:
	cc server_xyz.c -o sx

client:
	cc client.c -o c

clean: clean_c clean_sa clean_sx reset

clean_c:
	rm c

clean_sa:
	rm sa

clean_sx:
	rm sx

reset:
	reset
