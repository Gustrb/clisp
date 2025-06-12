build-tests:
	echo "Building tests..."
	gcc -o dist/lexer.tests tests/lexer.tests.c src/lexer.c -O3 -I ./lib -Wall -Wall -Wextra -pedantic -lm
	gcc -o dist/parser.tests tests/parser.tests.c src/parser.c src/lexer.c -O3 -I ./lib -Wall -Wall -Wextra -pedantic -lm
	gcc -o dist/serialize-deserialize.tests tests/serialize-deserialize.tests.c src/serialize.c src/parser.c src/lexer.c -O3 -DPARSER_TESTS -I ./lib -Wall -Wall -Wextra -pedantic -lm
	gcc -o dist/alloc.tests tests/alloc.tests.c src/alloc.c -DALLOC_TESTS -O3 -I ./lib -Wall -Wall -Wextra -pedantic -lm

	gcc -o dist/serial-over-the-wire.server tests/serial-over-the-wire/server.c src/serialize.c src/parser.c src/lexer.c -DPARSER_TESTS -I ./lib -Wall -Wall -Wextra -pedantic -lm
	gcc -o dist/serial-over-the-wire.client tests/serial-over-the-wire/client.c src/serialize.c src/parser.c src/lexer.c -DPARSER_TESTS -I ./lib -Wall -Wall -Wextra -pedantic -lm

build-benchmarks:
	echo "Building benchmarks..."
	gcc -o dist/fixturegen benchmark/fixtures/fixturegen.c -O3 -I ./lib -Wall -Wall -Wextra -pedantic -lm
	gcc -o dist/lexer.benchmarks benchmark/lexer.benchmark.c -O3 benchmark/benchmark.c src/lexer.c src/io.c -I ./lib -Wall -Wall -Wextra -pedantic -lm
	gcc -o dist/parser.benchmarks benchmark/parser.benchmark.c -O3 benchmark/benchmark.c src/lexer.c src/parser.c src/io.c -I ./lib -Wall -Wall -Wextra -pedantic -lm

	./dist/fixturegen ./benchmark/fixtures/small.lisp 100
	./dist/fixturegen ./benchmark/fixtures/medium.lisp 10000
	./dist/fixturegen ./benchmark/fixtures/large.lisp 100000

run-tests:
	./dist/lexer.tests
	./dist/parser.tests
	./dist/serialize-deserialize.tests
	./dist/alloc.tests

	./dist/serial-over-the-wire.server&
	sleep 1
	./dist/serial-over-the-wire.client
	sleep 1

run-benchmarks:
	./dist/lexer.benchmarks
	./dist/parser.benchmarks

build-plain:
	echo "Building plain..."
	gcc -o dist/plain.singlethread src/plain/single-thread/main.c src/lexer.c src/parser.c src/io.c benchmark/benchmark.c -O3 -I ./lib -Wall -Wall -Wextra -pedantic -lm
	gcc -o dist/plain.threaded src/plain/threaded/main.c src/lexer.c src/parser.c src/io.c benchmark/benchmark.c -O3 -I ./lib -Wall -Wall -Wextra -pedantic -lm -lpthread

run-plain:
	mkdir -p ./benchmark/fixtures/data

	./dist/fixturegen ./benchmark/fixtures/data/small.lisp 1000
	./dist/fixturegen ./benchmark/fixtures/data/medium.lisp 10000
	./dist/fixturegen ./benchmark/fixtures/data/large.lisp 100000
	FILES=$$(find ./benchmark/fixtures/data -type f -name "*.lisp" | paste -sd ' ' -); \
	time ./dist/plain.singlethread $$FILES; \
	time ./dist/plain.threaded $$FILES;
