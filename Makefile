all: reset server_abc server_xyz server_standard_xyz client client_standard

server_abc:
	cc server_abc.c -o sa

server_xyz:
	cc server_xyz.c -o sx

server_standard_xyz:
	cc server_standard_xyz.c -o ssx

client:
	cc client.c -o c

client_standard:
	cc client_standard.c -o cs

clean: clean_c clean_cs clean_sa clean_sx clean_ssx reset

clean_c:
	rm c

clean_cs:
	rm cs

clean_sa:
	rm sa

clean_sx:
	rm sx

clean_ssx:
	rm ssx

reset:
	reset
