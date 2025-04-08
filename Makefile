build-tests:
	echo "Building tests..."
	gcc -o dist/lexer.tests tests/lexer.tests.c src/lexer.c -I ./lib -Wall -Wall -Wextra -pedantic -lm


run-tests:
	./dist/lexer.tests
