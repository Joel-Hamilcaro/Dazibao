CC=gcc
CFLAGS=-g -Wall

run: bin/client.o bin/client_manager.o bin/donnee.o bin/noeud.o bin/list.o bin/voisin.o bin/calculator.o bin/sha224-256.o bin/tvl.o bin/paquet.o
	$(CC) $(CFLAGS) -o $@ $^

bin/noeud.o: src/noeud.c src/noeud.h
	$(CC) $(CFLAGS) -c -o $@ $<

bin/list.o: src/list.c src/list.h
	$(CC) $(CFLAGS) -c -o $@ $<

bin/donnee.o: src/donnee.c src/donnee.h src/calculator.h src/list.h
	$(CC) $(CFLAGS) -c -o $@ $<

bin/voisin.o: src/voisin.c src/voisin.h
	$(CC) $(CFLAGS) -c -o $@ $<

bin/sha224-256.o : src/rfc6234/sha224-256.c src/rfc6234/sha.h src/rfc6234/sha-private.h
	$(CC) $(CFLAGS) -c -o $@ $<

bin/client_manager.o: src/client_manager.c src/client_manager.h src/paquet.h src/list.h
	$(CC) $(CFLAGS) -c -o $@ $<

bin/calculator.o : src/calculator.c src/calculator.h src/rfc6234/sha.h src/donnee.h src/list.h
	$(CC) $(CFLAGS) -c -o $@ $<

bin/tvl.o: src/tvl.c src/tvl.h
	$(CC) $(CFLAGS) -c -o $@ $<

bin/paquet.o: src/paquet.c src/paquet.h src/tvl.h src/list.h
	$(CC) $(CFLAGS) -c -o $@ $<

bin/client.o: src/client.c src/client.h src/client_manager.h src/noeud.h src/list.h src/donnee.h src/voisin.h src/tvl.h src/paquet.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf *~ bin/*.o run
