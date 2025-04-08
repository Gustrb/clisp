build-tests:
	echo "Building tests..."
	gcc -o dist/lexer.tests tests/lexer.tests.c src/lexer.c -I ./lib -Wall -Wall -Wextra -pedantic -lm

build-benchmarks:
	echo "Building benchmarks..."
	gcc -o dist/lexer.benchmarks benchmark/lexer.benchmark.c benchmark/benchmark.c src/lexer.c -I ./lib -Wall -Wall -Wextra -pedantic -lm


run-tests:
	./dist/lexer.tests

run-benchmarks:
	./dist/lexer.benchmarks
