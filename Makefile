all: reset smtp_server smtp_client

smtp_server:
	cc smtp_server.c -o ss

smtp_client:
	cc smtp_client.c -o sc

clean: clean_sc clean_ss

clean_sc:
	rm sc

clean_ss:
	rm ss

reset:
	reset
