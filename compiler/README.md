# JACK COMPILER
- parses `.jack` files and folders and produces `.vm` file for each jack file
- to compile and run
```bash
cd compiler
g++ main.cpp tokenizer.cpp compengineVM.cpp symboltable.cpp vmwriter.cpp -o compiler
./compiler GrammarTest.jack
```