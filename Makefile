build-tests:
	echo "Building tests..."
	gcc -o dist/lexer.tests tests/lexer.tests.c src/lexer.c -I ./lib -Wall -Wall -Wextra -pedantic -lm
	gcc -o dist/parser.tests tests/parser.tests.c src/parser.c src/lexer.c -I ./lib -Wall -Wall -Wextra -pedantic -lm

build-benchmarks:
	echo "Building benchmarks..."
	gcc -o dist/fixturegen benchmark/fixtures/fixturegen.c -O2 -I ./lib -Wall -Wall -Wextra -pedantic -lm
	gcc -o dist/lexer.benchmarks benchmark/lexer.benchmark.c -O2 benchmark/benchmark.c src/lexer.c -I ./lib -Wall -Wall -Wextra -pedantic -lm

run-tests:
	./dist/lexer.tests
	./dist/parser.tests

run-benchmarks:
	./dist/fixturegen ./benchmark/fixtures/small.lisp 10
	./dist/fixturegen ./benchmark/fixtures/medium.lisp 100
	./dist/fixturegen ./benchmark/fixtures/large.lisp 1000
	./dist/lexer.benchmarks
