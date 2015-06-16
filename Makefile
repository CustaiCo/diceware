all: diceware

diceware:
	@echo CC -o $@
	@${CC} -o $@ diceware.c

clean:
	@echo cleaning
	@rm -f diceware

.PHONY: all options clean
