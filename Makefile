build-tests:
	echo "Building tests..."
	gcc -o dist/lexer.tests tests/lexer.tests.c src/lexer.c -I ./lib -Wall -Wall -Wextra -pedantic -lm
	gcc -o dist/parser.tests tests/parser.tests.c src/parser.c src/lexer.c -I ./lib -Wall -Wall -Wextra -pedantic -lm

build-benchmarks:
	echo "Building benchmarks..."
	gcc -o dist/fixturegen benchmark/fixtures/fixturegen.c -O3 -I ./lib -Wall -Wall -Wextra -pedantic -lm
	gcc -o dist/lexer.benchmarks benchmark/lexer.benchmark.c -O3 benchmark/benchmark.c src/lexer.c -I ./lib -Wall -Wall -Wextra -pedantic -lm
	gcc -o dist/parser.benchmarks benchmark/parser.benchmark.c -O3 benchmark/benchmark.c src/lexer.c src/parser.c -I ./lib -Wall -Wall -Wextra -pedantic -lm

	./dist/fixturegen ./benchmark/fixtures/small.lisp 100
	./dist/fixturegen ./benchmark/fixtures/medium.lisp 10000
	./dist/fixturegen ./benchmark/fixtures/large.lisp 100000

run-tests:
	./dist/lexer.tests
	./dist/parser.tests

run-benchmarks:
	./dist/lexer.benchmarks
	./dist/parser.benchmarks
